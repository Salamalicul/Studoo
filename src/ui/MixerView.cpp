#include "MixerView.h"
#include "TrackFxPanel.h"
#include "HelpTexts.h"

namespace studoo
{
namespace
{
    constexpr int stripWidth = 132;
    constexpr int knobHeight = 46;
}

// ============================================================================
//  MixerStrip
// ============================================================================
MixerStrip::MixerStrip (AppContext& c, int trackIndex) : ctx (c), index (trackIndex)
{
    eqLow = makeKnob ("Graves", txt ("EQ graves (200 Hz): peso y potencia."));
    eqMid = makeKnob ("Medios", txt ("EQ medios (1000 Hz): cuerpo del sonido."));
    eqHigh = makeKnob ("Agudos", txt ("EQ agudos (5000 Hz): brillo y aire."));
    compThreshold = makeKnob ("Umbral", txt ("Compresor: a partir de qué volumen empieza a actuar. Más bajo = comprime más."));
    compRatio = makeKnob ("Ratio", txt ("Compresor: cuánto comprime. 2 = suave, 4 = normal, 10 = muy fuerte."));
    delayMix = makeKnob ("Mezcla", txt ("Delay: volumen del eco."));
    delayTime = makeKnob ("Tiempo", txt ("Delay: tiempo entre ecos, en tiempos de compás (0,75 = corchea con puntillo)."));
    delayFeedback = makeKnob ("Repet.", txt ("Delay: cuántas repeticiones (feedback)."));
    reverbMix = makeKnob ("Mezcla", txt ("Reverb: cuánta reverberación se oye."));
    reverbSize = makeKnob (txt ("Tamaño"), txt ("Reverb: tamaño de la sala."));
    pan = makeKnob ("PAN", txt ("Panorama: coloca el sonido a la izquierda o a la derecha."));
    filter = makeKnob ("Filtro", txt ("Filtro de pista: a la izquierda quita agudos (sonido apagado), a la derecha quita graves (sonido fino). En el centro no hace nada."));
    distortion = makeKnob ("Distor.", txt ("Ensucia y satura el sonido: más fuerte y agresivo."));
    chorus = makeKnob ("Chorus", txt ("Chorus: el sonido se hace más ancho, como si sonaran varios a la vez."));
    sidechain = makeKnob ("Bajada", txt ("Sidechain: cuánto baja esta pista cuando suena la pista de origen (normalmente el bombo). Es el famoso \"bombeo\"."));
    filter->setDisplayFunction ([] (double v)
    {
        if (std::abs (v) < 0.02) return txt ("Sin filtro");
        return v < 0 ? txt ("Quita agudos ") + juce::String ((int) (-v * 100.0)) + "%" : txt ("Quita graves ") + juce::String ((int) (v * 100.0)) + "%";
    });
    for (auto* k : { distortion, chorus, sidechain })
        k->setDisplayFunction ([] (double v) { return juce::String ((int) (v * 100.0)) + "%"; });

    addAndMakeVisible (sidechainSource);
    sidechainSource.setTooltip (txt ("Pista que manda (normalmente el bombo): cuando suena, esta pista baja su volumen."));
    for (int t = 1; t < numMixerTracks; ++t)
        if (t != index)
            sidechainSource.addItem ("Pista " + juce::String (t), t);
    makeNonFocusing (sidechainSource);

    send = makeKnob (txt ("Envío"), txt ("Envío: cuánto sonido de esta pista se manda (copia) a otra pista. "
                                       "Ideal para una pista de efectos, por ejemplo una reverb compartida por varias pistas."));
    send->setDisplayFunction ([] (double v) { return juce::String ((int) std::round (v * 100.0)) + "%"; });

    addAndMakeVisible (sendTarget);
    sendTarget.setTooltip (txt ("A qué pista se manda la copia. Solo se puede mandar a pistas de número mayor "
                                "(pon tus pistas de efectos o \"buses\" al final)."));
    sendTarget.onChange = [this]
    {
        ctx.project.beginTransaction();
        const int target = juce::jmax (0, sendTarget.getSelectedId() - 1);
        auto track = ctx.project.getMixerTrack (index);
        track.setProperty (ids::sendTo, target, ctx.project.um());
        if (target > 0 && (float) track.getProperty (ids::sendLevel, 0.0f) < 0.01f)
            track.setProperty (ids::sendLevel, 0.8f, ctx.project.um());   // para que se oiga al elegirla
        updateSendControls();
    };
    makeNonFocusing (sendTarget);

    addAndMakeVisible (sendOnly);
    sendOnly.setButtonText (txt ("Solo envío"));
    sendOnly.setTooltip (txt ("Activado: esta pista ya no va directa al Master, solo suena a través de la pista de destino. "
                              "Sirve para agrupar (por ejemplo, toda la batería en una pista \"Batería\")."));
    makeNonFocusing (sendOnly);

    addAndMakeVisible (limiterOn);
    limiterOn.setButtonText (txt ("Limitador"));
    limiterOn.setTooltip (txt ("Limitador: nada pasa del máximo. Evita que el sonido se rompa (distorsione) cuando todo suena muy fuerte."));
    makeNonFocusing (limiterOn);

    auto db = [] (double v) { return (v > 0 ? "+" : "") + juce::String (v, 1) + " dB"; };
    for (auto* k : { eqLow, eqMid, eqHigh })
        k->setDisplayFunction (db);
    compThreshold->setDisplayFunction (db);
    compRatio->setDisplayFunction ([] (double v) { return juce::String (v, 1) + ":1"; });
    for (auto* k : { delayMix, delayFeedback, reverbMix, reverbSize })
        k->setDisplayFunction ([] (double v) { return juce::String ((int) (v * 100.0)) + "%"; });
    delayTime->setDisplayFunction ([] (double v) { return juce::String (v, 2) + " tiempos"; });

    addAndMakeVisible (compOn);
    compOn.setButtonText ("On");
    compOn.setTooltip (txt ("Activa el compresor de esta pista."));
    makeNonFocusing (compOn);

    addAndMakeVisible (muteButton);
    muteButton.setButtonText ("M");
    muteButton.setClickingTogglesState (true);
    muteButton.setTooltip (txt ("Silenciar esta pista (mute)."));
    makeNonFocusing (muteButton);

    addAndMakeVisible (soloButton);
    soloButton.setButtonText ("S");
    soloButton.setClickingTogglesState (true);
    soloButton.setTooltip (txt ("Solo: escuchar solo esta pista (y las que también tengan S)."));
    soloButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff4fc3f7));
    makeNonFocusing (soloButton);

    addAndMakeVisible (pluginButton);
    pluginButton.setTooltip (txt ("Añadir, abrir o quitar un efecto VST3 en esta pista."));
    pluginButton.onClick = [this] { if (ctx.showMixerPluginMenu) ctx.showMixerPluginMenu (index); };
    makeNonFocusing (pluginButton);

    addAndMakeVisible (fxButton);
    fxButton.setButtonText ("FX+");
    fxButton.setTooltip (txt ("Más efectos: autotune y afinador, puerta de ruido, maximizador, efectos de ritmo "
                              "(half-time, tartamudeo...), EQ paramétrico y envíos 2 y 3."));
    fxButton.onClick = [this]
    {
        auto panel = std::make_unique<TrackFxPanel> (ctx, index);
        auto* top = getTopLevelComponent();
        juce::CallOutBox::launchAsynchronously (std::move (panel), top->getLocalArea (this, fxButton.getBounds()), top);
    };
    makeNonFocusing (fxButton);

    addAndMakeVisible (fader);
    fader.setSliderStyle (juce::Slider::LinearVertical);
    fader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    fader.setPopupDisplayEnabled (true, true, nullptr, 1000);
    fader.textFromValueFunction = [] (double v) { return v <= 0.0001 ? juce::String ("-inf dB") : juce::String (juce::Decibels::gainToDecibels (v), 1) + " dB"; };
    fader.setTooltip (txt ("Volumen de la pista. Doble clic: valor por defecto."));
    fader.onDragStart = [this] { ctx.project.beginTransaction(); };
    makeNonFocusing (fader);

    bind();
}

Knob* MixerStrip::makeKnob (const juce::String& label, const juce::String& tip)
{
    auto* k = knobs.add (new Knob (label, tip));
    addAndMakeVisible (k);
    return k;
}

void MixerStrip::bind()
{
    auto track = ctx.project.getMixerTrack (index);
    auto* um = ctx.project.um();
    eqLow->bind (track, ids::eqLow, um, -12.0, 12.0, 0.0);
    eqMid->bind (track, ids::eqMid, um, -12.0, 12.0, 0.0);
    eqHigh->bind (track, ids::eqHigh, um, -12.0, 12.0, 0.0);
    compThreshold->bind (track, ids::compThreshold, um, -40.0, 0.0, -18.0);
    compRatio->bind (track, ids::compRatio, um, 1.0, 20.0, 4.0, 4.0);
    delayMix->bind (track, ids::delayMix, um, 0.0, 1.0, 0.0);
    delayTime->bind (track, ids::delayTime, um, 0.25, 2.0, 0.75);
    delayTime->getSlider().setRange (0.25, 2.0, 0.25);
    delayFeedback->bind (track, ids::delayFeedback, um, 0.0, 0.9, 0.35);
    reverbMix->bind (track, ids::reverbMix, um, 0.0, 1.0, 0.0);
    reverbSize->bind (track, ids::reverbSize, um, 0.0, 1.0, 0.6);
    pan->bind (track, ids::pan, um, -1.0, 1.0, 0.0);
    filter->bind (track, ids::trackFilter, um, -1.0, 1.0, 0.0);
    distortion->bind (track, ids::distortion, um, 0.0, 1.0, 0.0);
    chorus->bind (track, ids::chorusMix, um, 0.0, 1.0, 0.0);
    sidechain->bind (track, ids::sidechain, um, 0.0, 1.0, 0.0);
    sidechainSource.getSelectedIdAsValue().referTo (track.getPropertyAsValue (ids::sidechainSource, um));
    if (sidechainSource.getSelectedId() == 0)
        sidechainSource.setSelectedId (index == 1 ? 2 : 1, juce::dontSendNotification);
    if (! track.hasProperty (ids::limiter))
        track.setProperty (ids::limiter, index == 0, nullptr);   // el Master lo trae encendido
    limiterOn.getToggleStateValue().referTo (track.getPropertyAsValue (ids::limiter, um));
    sidechain->setVisible (index != 0);
    sidechainSource.setVisible (index != 0);
    send->bind (track, ids::sendLevel, um, 0.0, 1.0, 0.0);
    sendOnly.getToggleStateValue().referTo (track.getPropertyAsValue (ids::sendOnly, um));
    updateSendControls();

    compOn.getToggleStateValue().referTo (track.getPropertyAsValue (ids::compOn, um));
    muteButton.getToggleStateValue().referTo (track.getPropertyAsValue (ids::muted, um));
    soloButton.getToggleStateValue().referTo (track.getPropertyAsValue (ids::solo, um));
    fader.setRange (0.0, 1.25, 0.0);
    fader.setSkewFactorFromMidPoint (0.5);
    fader.setDoubleClickReturnValue (true, 0.8);
    fader.getValueObject().referTo (track.getPropertyAsValue (ids::volume, um));

    soloButton.setVisible (index != 0);
    juce::StringArray names;
    for (int slot = 0; slot < numFxSlots; ++slot)
        if (auto* p = ctx.engine.getMixerPlugin (index, slot))
            names.add (p->getName());
    pluginButton.setButtonText (names.isEmpty() ? txt ("+ Plugin")
                                : names.size() == 1 ? names[0].substring (0, 14)
                                                    : juce::String (names.size()) + " plugins");
    pluginButton.setTooltip (names.isEmpty() ? txt ("Añade hasta 4 efectos VST3 a esta pista.")
                                             : txt ("Efectos (en orden): ") + names.joinIntoString (", "));
    repaint();
}

void MixerStrip::updateMeter()
{
    const float l = ctx.engine.getTrackPeak (index, 0);
    const float r = ctx.engine.getTrackPeak (index, 1);
    if (! juce::approximatelyEqual (l, meterL) || ! juce::approximatelyEqual (r, meterR))
    {
        meterL = l;
        meterR = r;
        repaint (meterArea);
    }
}

void MixerStrip::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (index == 0 || ! nameArea.contains (e.getPosition()))
        return;
    auto track = ctx.project.getMixerTrack (index);
    askForText (txt ("Renombrar pista"), txt ("Nuevo nombre de la pista:"), track[ids::name].toString(), [this, track] (const juce::String& name) mutable
    {
        ctx.project.beginTransaction();
        track.setProperty (ids::name, name, ctx.project.um());
    });
}

void MixerStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (3.0f);
    const bool master = index == 0;

    Draw::shadow (g, bounds, 8.0f, 8);
    g.setGradientFill (juce::ColourGradient (master ? Colours::panelLight.brighter (0.04f) : Colours::panelLight.darker (0.05f), 0.0f, bounds.getY(),
                                             Colours::panelDark, 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (master ? Colours::accent.withAlpha (0.7f) : Colours::outline);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, master ? 1.4f : 1.0f);

    // Color de la pista: el del primer canal que manda su sonido aquí
    juce::Colour trackColour = Colours::accent;
    bool used = master;
    if (! master)
        for (auto ch : ctx.project.channels())
            if ((int) ch[ids::mixer] == index)
            {
                trackColour = Colours::forChannelType (ch[ids::type].toString());
                used = true;
                break;
            }

    // Cabecera con el nombre
    auto head = nameArea.toFloat().expanded (3.0f, 3.0f);
    g.setColour (master ? Colours::accent.withAlpha (0.18f)
                        : used ? trackColour.withAlpha (0.12f) : juce::Colours::black.withAlpha (0.25f));
    g.fillRoundedRectangle (head, 6.0f);
    if (used && ! master)
    {
        auto bar = head.withHeight (3.0f).reduced (8.0f, 0.0f).translated (0.0f, -2.0f);
        Draw::glow (g, bar, trackColour, 5.0f, 1.5f);
        Draw::colourFill (g, bar, trackColour, 1.5f);
    }
    auto track = ctx.project.getMixerTrack (index);
    g.setColour (master ? Colours::accentLight : used ? Colours::text : Colours::textDim);
    g.setFont (Fonts::bold (13.0f));
    const auto name = master ? juce::String ("MASTER") : juce::String (index) + ". " + track[ids::name].toString();
    g.drawFittedText (name, nameArea, juce::Justification::centred, 1, 0.8f);

    // Títulos de cada efecto, con una línea fina
    auto label = [&] (juce::Component* c, const char* text)
    {
        auto r = juce::Rectangle<int> (8, c->getY() - 12, getWidth() - 16, 11);
        g.setFont (Fonts::bold (9.5f));
        g.setColour (Colours::accent.withAlpha (0.9f));
        g.drawText (txt (text), r, juce::Justification::centredLeft);
        const int textW = juce::GlyphArrangement::getStringWidthInt (Fonts::bold (9.5f), txt (text));
        g.setColour (Colours::outline.withAlpha (0.8f));
        g.drawHorizontalLine (r.getCentreY(), (float) (r.getX() + textW + 6), (float) r.getRight());
    };
    label (eqLow, "EQ");
    label (compThreshold, "COMPRESOR");
    label (delayMix, "DELAY");
    label (reverbMix, "REVERB");
    label (filter, "FILTRO · SUCIO · CHORUS");
    label (index == 0 ? (juce::Component*) &limiterOn : (juce::Component*) sidechain, index == 0 ? "MASTER" : "SIDECHAIN · LIMITADOR");
    if (index != 0)
        label (send, "ENVÍO");

    // Medidor de nivel con LEDs
    auto drawBar = [&] (juce::Rectangle<int> r, float level)
    {
        g.setColour (juce::Colour (0xff07090b));
        g.fillRoundedRectangle (r.toFloat(), 2.5f);
        const float db = juce::Decibels::gainToDecibels (level, -60.0f);
        const float prop = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 63.0f);
        const int segments = juce::jmax (8, r.getHeight() / 5);
        const float segH = (float) r.getHeight() / (float) segments;
        for (int i = 0; i < segments; ++i)
        {
            const float pos = (float) i / (float) segments; // 0 = abajo
            auto seg = juce::Rectangle<float> ((float) r.getX() + 1.0f, (float) r.getBottom() - (float) (i + 1) * segH + 0.6f,
                                               (float) r.getWidth() - 2.0f, segH - 1.2f);
            const juce::Colour c = pos > 0.9f ? juce::Colour (0xffff4040) : pos > 0.72f ? juce::Colour (0xffffc23d) : Colours::green;
            const bool lit = pos < prop;
            g.setColour (lit ? c : c.withAlpha (0.08f));
            g.fillRect (seg);
        }
    };
    auto m = meterArea;
    drawBar (m.removeFromLeft (m.getWidth() / 2 - 1), meterL);
    m.removeFromLeft (2);
    drawBar (m, meterR);
}

void MixerStrip::updateSendControls()
{
    const int target = ctx.project.getMixerTrack (index).getProperty (ids::sendTo, 0);
    const bool valid = target > index && target < numMixerTracks;

    // Los nombres de las pistas pueden cambiar: rehace la lista solo si cambió algo
    juce::StringArray names { txt ("Sin envío") };
    for (int t = index + 1; t < numMixerTracks && index > 0; ++t)
        names.add (txt ("→ ") + juce::String (t) + ". " + ctx.project.getMixerTrack (t)[ids::name].toString());
    bool same = sendTarget.getNumItems() == names.size();
    for (int i = 0; same && i < names.size(); ++i)
        same = sendTarget.getItemText (i) == names[i];
    if (! same)
    {
        sendTarget.clear (juce::dontSendNotification);
        sendTarget.addItem (names[0], 1);
        for (int i = 1; i < names.size(); ++i)
            sendTarget.addItem (names[i], index + i + 1);
    }
    sendTarget.setSelectedId (valid ? target + 1 : 1, juce::dontSendNotification);
    const bool canSend = index > 0 && index < numMixerTracks - 1;
    send->setVisible (index > 0);
    sendTarget.setVisible (index > 0);
    sendOnly.setVisible (index > 0);
    sendTarget.setEnabled (canSend);
    send->setEnabled (valid);
    sendOnly.setEnabled (valid);
}

void MixerStrip::resized()
{
    auto area = getLocalBounds().reduced (6);
    nameArea = area.removeFromTop (22);
    area.removeFromTop (12);

    auto row = [&] (std::initializer_list<juce::Component*> comps)
    {
        auto r = area.removeFromTop (knobHeight);
        const int w = r.getWidth() / 3;
        for (auto* c : comps)
            c->setBounds (r.removeFromLeft (w));
        area.removeFromTop (12);
    };

    row ({ eqLow, eqMid, eqHigh });
    {
        auto r = area.removeFromTop (knobHeight);
        const int w = r.getWidth() / 3;
        compOn.setBounds (r.removeFromLeft (w + 6).withSizeKeepingCentre (w + 6, 24));
        compThreshold->setBounds (r.removeFromLeft (w));
        compRatio->setBounds (r.removeFromLeft (w));
        area.removeFromTop (12);
    }
    row ({ delayMix, delayTime, delayFeedback });
    row ({ reverbMix, reverbSize });
    row ({ filter, distortion, chorus });
    {
        auto r = area.removeFromTop (knobHeight);
        const int w = r.getWidth() / 3;
        if (index != 0)
        {
            sidechain->setBounds (r.removeFromLeft (w));
            auto right = r.reduced (2, 0);
            sidechainSource.setBounds (right.removeFromTop (22));
            limiterOn.setBounds (right.withTrimmedTop (2));
        }
        else
        {
            limiterOn.setBounds (r.withSizeKeepingCentre (r.getWidth(), 26));
        }
        area.removeFromTop (12);
    }
    if (index != 0)
    {
        auto r = area.removeFromTop (knobHeight);
        const int w = r.getWidth() / 3;
        send->setBounds (r.removeFromLeft (w));
        auto right = r.reduced (2, 0);
        sendTarget.setBounds (right.removeFromTop (22));
        sendOnly.setBounds (right.withTrimmedTop (2));
        area.removeFromTop (12);
    }
    else
    {
        area.removeFromTop (knobHeight + 12);   // el Master no envía: deja el hueco para que todo quede alineado
    }
    area.removeFromTop (-8);
    {
        auto r = area.removeFromTop (24);
        fxButton.setBounds (r.removeFromRight (42));
        r.removeFromRight (3);
        pluginButton.setBounds (r);
    }
    area.removeFromTop (6);
    {
        auto r = area.removeFromTop (knobHeight);
        pan->setBounds (r.removeFromLeft (r.getWidth() / 2));
        auto buttons = r.reduced (2, 6);
        muteButton.setBounds (buttons.removeFromLeft (buttons.getWidth() / 2 - 1));
        buttons.removeFromLeft (2);
        soloButton.setBounds (buttons);
    }
    area.removeFromTop (6);
    meterArea = area.removeFromRight (16);
    area.removeFromRight (4);
    fader.setBounds (area);
}

// ============================================================================
//  MixerView
// ============================================================================
MixerView::MixerView (AppContext& c) : ctx (c), helpButton (txt (help::mixerTitle), txt (help::mixerText))
{
    addAndMakeVisible (analyzerButton);
    analyzerButton.setButtonText (txt ("Analizador"));
    analyzerButton.setTooltip (txt ("Muestra las frecuencias del Master: cuántos graves, medios y agudos tiene tu canción."));
    analyzerButton.onClick = [this]
    {
        auto* top = getTopLevelComponent();
        juce::CallOutBox::launchAsynchronously (std::make_unique<AnalyzerPanel> (ctx), top->getLocalArea (this, analyzerButton.getBounds()), top);
    };
    makeNonFocusing (analyzerButton);

    addAndMakeVisible (helpButton);
    addAndMakeVisible (infoLabel);
    infoLabel.setText (txt ("Doble clic en el nombre de una pista para renombrarla. Cada canal elige su pista en el secuenciador."),
                       juce::dontSendNotification);
    infoLabel.setColour (juce::Label::textColourId, Colours::textDim);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&stripHolder, false);
    viewport.setScrollBarsShown (true, true);

    for (int i = 0; i < numMixerTracks; ++i)
        stripHolder.addAndMakeVisible (strips.add (new MixerStrip (ctx, i)));

    lastGeneration = ctx.project.getGeneration();
    ctx.project.addChangeListener (this);
    startTimerHz (30);
}

MixerView::~MixerView()
{
    ctx.project.removeChangeListener (this);
}

void MixerView::changeListenerCallback (juce::ChangeBroadcaster*)
{
    if (lastGeneration != ctx.project.getGeneration())
    {
        lastGeneration = ctx.project.getGeneration();
        for (auto* s : strips)
            s->bind();
    }
    for (auto* s : strips)
    {
        s->updateSendControls();
        s->repaint();
    }
}

void MixerView::timerCallback()
{
    ctx.engine.decayMeters();
    if (isShowing())
        for (auto* s : strips)
            s->updateMeter();
}

void MixerView::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, "Mezclador");
}

void MixerView::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (42);
    auto h = headerArea.reduced (8, 7);
    helpButton.setBounds (h.removeFromLeft (26));
    h.removeFromLeft (120);
    analyzerButton.setBounds (h.removeFromRight (130));
    h.removeFromRight (8);
    infoLabel.setBounds (h);

    viewport.setBounds (area.reduced (6));
    const int height = juce::jmax (750, viewport.getHeight() - viewport.getScrollBarThickness());
    stripHolder.setSize (stripWidth * numMixerTracks, height);
    for (int i = 0; i < strips.size(); ++i)
        strips[i]->setBounds (i * stripWidth, 0, stripWidth, height);
}
}
