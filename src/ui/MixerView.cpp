#include "MixerView.h"
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

    compOn.getToggleStateValue().referTo (track.getPropertyAsValue (ids::compOn, um));
    muteButton.getToggleStateValue().referTo (track.getPropertyAsValue (ids::muted, um));
    soloButton.getToggleStateValue().referTo (track.getPropertyAsValue (ids::solo, um));
    fader.setRange (0.0, 1.25, 0.0);
    fader.setSkewFactorFromMidPoint (0.5);
    fader.setDoubleClickReturnValue (true, 0.8);
    fader.getValueObject().referTo (track.getPropertyAsValue (ids::volume, um));

    soloButton.setVisible (index != 0);
    pluginButton.setButtonText (ctx.engine.getMixerPlugin (index) != nullptr
                                    ? ctx.engine.getMixerPlugin (index)->getName().substring (0, 14)
                                    : txt ("+ Plugin"));
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

    // Cabecera con el nombre
    auto head = nameArea.toFloat().expanded (3.0f, 3.0f);
    g.setColour (master ? Colours::accent.withAlpha (0.18f) : juce::Colours::black.withAlpha (0.25f));
    g.fillRoundedRectangle (head, 6.0f);
    auto track = ctx.project.getMixerTrack (index);
    g.setColour (master ? Colours::accentLight : Colours::text);
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
    area.removeFromTop (-8);
    pluginButton.setBounds (area.removeFromTop (24));
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
    addAndMakeVisible (helpButton);
    addAndMakeVisible (infoLabel);
    infoLabel.setText (txt ("Doble clic en el nombre de una pista para renombrarla. Cada canal elige su pista en el secuenciador."),
                       juce::dontSendNotification);
    infoLabel.setColour (juce::Label::textColourId, Colours::textDim);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&stripHolder, false);
    viewport.setScrollBarsShown (false, true);

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
        s->repaint();
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
    infoLabel.setBounds (h);

    viewport.setBounds (area.reduced (6));
    const int height = juce::jmax (560, viewport.getHeight() - viewport.getScrollBarThickness());
    stripHolder.setSize (stripWidth * numMixerTracks, height);
    for (int i = 0; i < strips.size(); ++i)
        strips[i]->setBounds (i * stripWidth, 0, stripWidth, height);
}
}
