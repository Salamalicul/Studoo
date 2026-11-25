#include "InstrumentPanel.h"
#include "HelpTexts.h"

namespace studoo
{
// ============================================================================
//  SampleWaveform
// ============================================================================
SampleWaveform::SampleWaveform (AppContext& c, juce::ValueTree ch) : ctx (c), channel (ch)
{
    setTooltip (txt ("La forma de onda del sample. Arrastra la marca verde (inicio) o la roja (final) para recortarlo. "
                     "Las líneas finas son los trozos (chops)."));
    makeNonFocusing (*this);
    channel.addListener (this);
    rebuildPeaks();
}

SampleWaveform::~SampleWaveform() { channel.removeListener (this); }

void SampleWaveform::valueTreePropertyChanged (juce::ValueTree& t, const juce::Identifier& id)
{
    if (t != channel)
        return;
    if (id == ids::sample)
        rebuildPeaks();
    if (id == ids::sample || id == ids::sampleStart || id == ids::sampleEnd || id == ids::slices || id == ids::reverse)
        repaint();
}

void SampleWaveform::rebuildPeaks()
{
    const auto path = channel[ids::sample].toString();
    if (path == peaksPath && ! peaks.empty())
        return;
    peaksPath = path;
    peaks.clear();
    auto sample = path.isNotEmpty() ? ctx.samples.get (path) : nullptr;
    if (sample == nullptr || sample->buffer.getNumSamples() == 0)
        return;

    constexpr int numPeaks = 400;
    const auto& b = sample->buffer;
    const int len = b.getNumSamples();
    peaks.resize (numPeaks, 0.0f);
    float maxPeak = 0.0001f;
    for (int i = 0; i < numPeaks; ++i)
    {
        const int from = (int) ((juce::int64) len * i / numPeaks);
        const int to = juce::jmax (from + 1, (int) ((juce::int64) len * (i + 1) / numPeaks));
        float m = 0.0f;
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            m = juce::jmax (m, b.getMagnitude (ch, from, juce::jmin (to, len) - from));
        peaks[(size_t) i] = m;
        maxPeak = juce::jmax (maxPeak, m);
    }
    for (auto& p : peaks)
        p /= maxPeak;   // normalizado para que se vea bien aunque el sample sea flojo
}

juce::Rectangle<float> SampleWaveform::waveArea() const
{
    return getLocalBounds().toFloat().reduced (2.0f, 4.0f).withTrimmedBottom (12.0f);
}

void SampleWaveform::paint (juce::Graphics& g)
{
    auto area = waveArea();
    g.setColour (juce::Colour (0xff121417));
    g.fillRoundedRectangle (area, 5.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (area, 5.0f, 1.0f);

    const auto colour = Colours::forChannelType (channel[ids::type].toString());
    if (peaks.empty())
    {
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (11.0f));
        g.drawFittedText (txt ("Sin sample"), area.toNearestInt(), juce::Justification::centred, 1);
        return;
    }

    const float start = juce::jlimit (0.0f, 1.0f, (float) channel.getProperty (ids::sampleStart, 0.0f));
    const float end = juce::jlimit (0.0f, 1.0f, (float) channel.getProperty (ids::sampleEnd, 1.0f));
    const auto inner = area.reduced (3.0f, 3.0f);
    const float cy = inner.getCentreY(), half = inner.getHeight() * 0.5f;
    const float step = inner.getWidth() / (float) peaks.size();
    for (size_t i = 0; i < peaks.size(); ++i)
    {
        const float pos = (float) i / (float) peaks.size();
        const bool inside = pos >= start && pos <= end;
        g.setColour (inside ? colour.withAlpha (0.9f) : Colours::textDim.withAlpha (0.25f));
        const float h = juce::jmax (0.5f, peaks[i] * half);
        g.fillRect (inner.getX() + (float) i * step, cy - h, juce::jmax (1.0f, step), h * 2.0f);
    }

    // Trozos (chops)
    const int slices = juce::jlimit (1, 16, (int) channel.getProperty (ids::slices, 1));
    if (slices > 1 && end > start)
    {
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        for (int s = 1; s < slices; ++s)
        {
            const float x = inner.getX() + inner.getWidth() * (start + (end - start) * (float) s / (float) slices);
            g.drawVerticalLine ((int) x, inner.getY(), inner.getBottom());
        }
    }

    // Marcas de inicio (verde) y final (rojo)
    auto marker = [&] (float pos, juce::Colour c)
    {
        const float x = inner.getX() + inner.getWidth() * pos;
        g.setColour (c);
        g.fillRect (x - 1.0f, area.getY(), 2.0f, area.getHeight());
        juce::Path tri;
        tri.addTriangle (x - 5.0f, area.getY(), x + 5.0f, area.getY(), x, area.getY() + 7.0f);
        g.fillPath (tri);
    };
    marker (start, Colours::green);
    marker (end, juce::Colour (0xffef5350));

    g.setColour (Colours::textDim);
    g.setFont (Fonts::regular (10.0f));
    auto caption = getLocalBounds().toFloat().removeFromBottom (13.0f);
    juce::String text = txt ("Arrastra las marcas para recortar");
    if ((bool) channel.getProperty (ids::reverse, false))
        text = txt ("◀ Suena al revés");
    else if (slices > 1)
        text = juce::String (slices) + txt (" trozos: cada nota toca uno");
    g.drawFittedText (text, caption.toNearestInt(), juce::Justification::centred, 1);
}

void SampleWaveform::mouseDown (const juce::MouseEvent& e)
{
    if (peaks.empty())
        return;
    const auto inner = waveArea().reduced (3.0f, 3.0f);
    const float pos = juce::jlimit (0.0f, 1.0f, (e.position.x - inner.getX()) / juce::jmax (1.0f, inner.getWidth()));
    const float start = channel.getProperty (ids::sampleStart, 0.0f);
    const float end = channel.getProperty (ids::sampleEnd, 1.0f);
    draggingStart = std::abs (pos - start) < std::abs (pos - end);
    ctx.project.beginTransaction();
    mouseDrag (e);
}

void SampleWaveform::mouseDrag (const juce::MouseEvent& e)
{
    if (peaks.empty())
        return;
    const auto inner = waveArea().reduced (3.0f, 3.0f);
    const float pos = juce::jlimit (0.0f, 1.0f, (e.position.x - inner.getX()) / juce::jmax (1.0f, inner.getWidth()));
    const float start = channel.getProperty (ids::sampleStart, 0.0f);
    const float end = channel.getProperty (ids::sampleEnd, 1.0f);
    if (draggingStart)
        channel.setProperty (ids::sampleStart, juce::jmin (pos, end - 0.01f), ctx.project.um());
    else
        channel.setProperty (ids::sampleEnd, juce::jmax (pos, start + 0.01f), ctx.project.um());
}

// ============================================================================
//  InstrumentPanel
// ============================================================================
InstrumentPanel::InstrumentPanel (AppContext& c)
    : ctx (c), helpButton (txt (help::instrumentTitle), txt (help::instrumentText))
{
    addAndMakeVisible (helpButton);
    addAndMakeVisible (infoLabel);
    infoLabel.setColour (juce::Label::textColourId, Colours::textDim);
    infoLabel.setJustificationType (juce::Justification::centredLeft);
    infoLabel.setMinimumHorizontalScale (0.7f);
}

Knob* InstrumentPanel::addKnob (const juce::String& label, const juce::String& tooltip, const juce::Identifier& id,
                                double min, double max, double def, double skewMid)
{
    auto* k = new Knob (label, tooltip);
    k->bind (channel, id, ctx.project.um(), min, max, def, skewMid);
    controls.add (k);
    addAndMakeVisible (k);
    return k;
}

void InstrumentPanel::setChannel (juce::ValueTree ch)
{
    channel = ch;
    controls.clear();
    title = {};
    typeName = {};
    infoLabel.setText ({}, juce::dontSendNotification);

    if (! channel.isValid())
    {
        resized();
        repaint();
        return;
    }

    const auto type = channelTypeFromString (channel[ids::type].toString());
    auto secs = [] (double v) { return v < 1.0 ? juce::String ((int) (v * 1000.0)) + " ms" : juce::String (v, 2) + " s"; };

    switch (type)
    {
        case ChannelType::sampler:
        {
            typeName = "Sampler";
            auto* load = new juce::TextButton (txt ("Cargar sample..."));
            load->setTooltip (txt ("Elige un archivo WAV o MP3 para este canal."));
            load->onClick = [this] { if (onLoadSample) onLoadSample (channel); };
            makeNonFocusing (*load);
            controls.add (load);
            addAndMakeVisible (load);

            auto* wave = new SampleWaveform (ctx, channel);
            controls.add (wave);
            addAndMakeVisible (wave);

            auto* oneShot = new juce::ToggleButton (txt ("One-shot (suena entero)"));
            oneShot->setTooltip (txt ("Activado: el sample suena completo aunque la nota sea corta (ideal para batería). "
                                      "Desactivado: deja de sonar al soltar la nota (ideal para melodías)."));
            oneShot->getToggleStateValue().referTo (channel.getPropertyAsValue (ids::oneShot, ctx.project.um()));
            makeNonFocusing (*oneShot);
            controls.add (oneShot);
            addAndMakeVisible (oneShot);

            auto* root = addKnob (txt ("Nota base"), txt ("La nota en la que el sample suena a su tono original."),
                                  ids::root, 24, 96, 60);
            root->getSlider().setRange (24, 96, 1);
            root->setDisplayFunction ([] (double v) { return noteName ((int) v); });

            auto* tune = addKnob ("Tono", txt ("Sube o baja el tono del sample en semitonos (12 = una octava)."),
                                  ids::sampleTune, -24.0, 24.0, 0.0);
            tune->getSlider().setRange (-24.0, 24.0, 1.0);
            tune->setDisplayFunction ([] (double v) { const int s = (int) std::round (v); return (s > 0 ? "+" : "") + juce::String (s) + " st"; });
            auto pct = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + "%"; };
            addKnob ("Inicio", txt ("Desde dónde empieza a sonar el sample. Súbelo para quitar el principio."),
                     ids::sampleStart, 0.0, 1.0, 0.0)->setDisplayFunction (pct);
            addKnob ("Final", txt ("Dónde termina el sample. Bájalo para cortarlo antes."),
                     ids::sampleEnd, 0.0, 1.0, 1.0)->setDisplayFunction (pct);
            auto* slices = addKnob ("Trozos", txt ("Corta el sample en trozos iguales (chops). Cada nota del piano roll toca un trozo distinto: "
                                                   "la nota base es el trozo 1, la siguiente el 2..."),
                                    ids::slices, 1.0, 16.0, 1.0);
            slices->getSlider().setRange (1.0, 16.0, 1.0);
            slices->setDisplayFunction ([] (double v) { const int n = (int) std::round (v); return n <= 1 ? txt ("Entero") : juce::String (n) + " trozos"; });

            auto* reverse = new juce::ToggleButton (txt ("Al revés"));
            reverse->setTooltip (txt ("El sample suena del final al principio (efecto \"reverse\")."));
            reverse->getToggleStateValue().referTo (channel.getPropertyAsValue (ids::reverse, ctx.project.um()));
            makeNonFocusing (*reverse);
            controls.add (reverse);
            addAndMakeVisible (reverse);
            break;
        }
        case ChannelType::synth:
        {
            typeName = "Sintetizador";
            auto* wave = new juce::ComboBox();
            wave->addItemList ({ "Sierra", "Cuadrada", "Seno", txt ("Triángulo") }, 1);
            wave->setTooltip (txt ("Forma de onda: el \"color\" básico del sonido."));
            wave->setSelectedId ((int) channel[ids::wave] + 1, juce::dontSendNotification);
            wave->onChange = [this, wave]
            {
                ctx.project.beginTransaction();
                channel.setProperty (ids::wave, wave->getSelectedId() - 1, ctx.project.um());
            };
            makeNonFocusing (*wave);
            controls.add (wave);
            addAndMakeVisible (wave);

            addKnob ("Ataque", txt ("Lo que tarda la nota en llegar al máximo volumen."), ids::attack, 0.001, 2.0, 0.005, 0.2)
                ->setDisplayFunction (secs);
            addKnob (txt ("Caída"), txt ("Lo que tarda en bajar hasta el nivel de sostenido."), ids::decay, 0.01, 3.0, 0.3, 0.5)
                ->setDisplayFunction (secs);
            addKnob ("Sostenido", txt ("Volumen mientras mantienes la nota."), ids::sustain, 0.0, 1.0, 0.6);
            addKnob (txt ("Relajación"), txt ("Lo que tarda en apagarse al soltar la nota."), ids::release, 0.005, 4.0, 0.3, 0.5)
                ->setDisplayFunction (secs);
            addKnob ("Filtro", txt ("Corta los agudos. Más bajo = sonido más oscuro."), ids::cutoff, 40.0, 18000.0, 5000.0, 1500.0)
                ->setDisplayFunction ([] (double v) { return juce::String ((int) v) + " Hz"; });
            addKnob ("Resonancia", txt ("Realza la zona del filtro (sonido más \"chillón\")."), ids::resonance, 0.0, 1.0, 0.2);
            addKnob ("Desafinar", txt ("Separa los dos osciladores: sonido más ancho."), ids::detune, 0.0, 50.0, 8.0)
                ->setValueSuffix (" cents", 0);
            auto* mono = new juce::ToggleButton ("Mono");
            mono->setTooltip (txt ("Mono: solo suena una nota a la vez. Si dos notas se solapan en el piano roll, "
                                   "el sonido se desliza de una a otra (con la rueda Glide)."));
            mono->getToggleStateValue().referTo (channel.getPropertyAsValue (ids::mono, ctx.project.um()));
            makeNonFocusing (*mono);
            controls.add (mono);
            addAndMakeVisible (mono);
            addKnob ("Glide", txt ("Cuánto tarda en deslizarse de una nota a otra (en modo Mono, con notas solapadas)."),
                     ids::glide, 0.0, 1.0, 0.0, 0.2)->setDisplayFunction ([] (double v) { return juce::String ((int) (v * 1000.0)) + " ms"; });
            addKnob ("LFO vel.", txt ("Velocidad del LFO: lo rápido que el filtro sube y baja solo (el \"wobble\" del dubstep)."),
                     ids::lfoRate, 0.1, 20.0, 4.0, 3.0)
                ->setDisplayFunction ([] (double v) { return juce::String (v, 1) + " Hz"; });
            addKnob ("LFO cant.", txt ("Cuánto mueve el LFO el filtro. A 0 no hace nada."),
                     ids::lfoAmount, 0.0, 1.0, 0.0)
                ->setDisplayFunction ([] (double v) { return juce::String ((int) std::round (v * 100.0)) + "%"; });
            break;
        }
        case ChannelType::bass808:
        {
            typeName = "808";
            addKnob (txt ("Caída"), txt ("Cuánto dura el 808 hasta apagarse."), ids::decay, 0.1, 4.0, 1.5, 1.0)
                ->setDisplayFunction (secs);
            addKnob ("Glide", txt ("Cuánto tarda en deslizarse a la nota siguiente cuando las notas se solapan."),
                     ids::glide, 0.0, 0.5, 0.08)->setDisplayFunction (secs);
            addKnob (txt ("Distorsión"), txt ("Hace el 808 más fuerte y agresivo; se oye mejor en altavoces pequeños."),
                     ids::drive, 0.0, 1.0, 0.3);
            auto* root = addKnob (txt ("Nota pasos"), txt ("La nota que suena cuando usas el 808 en el secuenciador de pasos."),
                                  ids::root, 24, 60, 36);
            root->getSlider().setRange (24, 60, 1);
            root->setDisplayFunction ([] (double v) { return noteName ((int) v); });
            infoLabel.setText (txt ("Consejo: escribe la línea del 808 en el PIANO ROLL (F7). Notas solapadas = glide."),
                               juce::dontSendNotification);
            break;
        }
        case ChannelType::plugin:
        {
            typeName = "Plugin VST3";
            auto* open = new juce::TextButton ("Abrir plugin");
            open->setTooltip (txt ("Muestra los controles del plugin."));
            const int uid = channel[ids::uid];
            const auto name = channel[ids::name].toString();
            open->onClick = [this, uid, name] { PluginWindows::show (ctx.engine.getChannelPlugin (uid), name); };
            makeNonFocusing (*open);
            controls.add (open);
            addAndMakeVisible (open);
            break;
        }
        case ChannelType::audio:
            typeName = txt ("Audio grabado");
            infoLabel.setText (txt ("Este canal contiene una grabación. Se reproduce desde la PLAYLIST en modo CANCIÓN."),
                               juce::dontSendNotification);
            break;
    }

    refresh();
    resized();
}

void InstrumentPanel::refresh()
{
    if (! channel.isValid())
        return;

    title = channel[ids::name].toString();
    const auto type = channelTypeFromString (channel[ids::type].toString());
    if (type == ChannelType::sampler || type == ChannelType::audio)
    {
        const auto path = channel[ids::sample].toString();
        juce::String text = path.startsWith ("builtin:")
                              ? (path.containsChar ('/') && kits::find (path.fromFirstOccurrenceOf ("builtin:", false, false).upToFirstOccurrenceOf ("/", false, false)) != nullptr
                                     ? txt ("Sonido incluido · kit ") + txt (kits::find (path.fromFirstOccurrenceOf ("builtin:", false, false).upToFirstOccurrenceOf ("/", false, false))->name)
                                     : txt ("Sonido incluido en Studoo"))
                          : path.isEmpty()                ? txt ("Sin sample: pulsa \"Cargar sample...\"")
                                                          : juce::File (path).getFileName();
        if (type == ChannelType::audio)
            text = txt ("Grabación: ") + juce::File (path).getFileName();
        infoLabel.setText (text, juce::dontSendNotification);
    }
    else if (type == ChannelType::plugin)
    {
        const bool loaded = ctx.engine.getChannelPlugin (channel[ids::uid]) != nullptr;
        infoLabel.setText (loaded ? txt ("Plugin cargado.") : txt ("El plugin no está cargado (¿está instalado en este ordenador?)."),
                           juce::dontSendNotification);
    }
    repaint();
}

void InstrumentPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    Draw::shadow (g, bounds, 10.0f, 10);
    g.setGradientFill (juce::ColourGradient (Colours::panel.brighter (0.03f), 0.0f, 0.0f, Colours::panelDark, 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 10.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 10.0f, 1.0f);

    const auto colour = Colours::forChannelType (channel[ids::type].toString());
    if (channel.isValid())
    {
        // Franja superior con el color del tipo de canal
        auto stripe = bounds.withHeight (3.0f).reduced (14.0f, 0.0f).translated (0.0f, 1.0f);
        Draw::glow (g, stripe, colour, 6.0f, 1.5f);
        Draw::colourFill (g, stripe, colour, 1.5f);
    }

    auto top = getLocalBounds().reduced (10, 6).removeFromTop (24).withTrimmedLeft (34);
    g.setFont (Fonts::bold (15.0f));
    g.setColour (Colours::text);
    if (channel.isValid())
    {
        g.drawText (title, top, juce::Justification::centredLeft);
        const int w = juce::GlyphArrangement::getStringWidthInt (Fonts::bold (15.0f), title);
        auto chip = juce::Rectangle<float> ((float) (top.getX() + w + 12), (float) top.getY() + 3.0f,
                                            (float) juce::GlyphArrangement::getStringWidthInt (Fonts::bold (11.0f), typeName) + 16.0f, 18.0f);
        g.setColour (colour.withAlpha (0.18f));
        g.fillRoundedRectangle (chip, 9.0f);
        g.setColour (colour.brighter (0.3f));
        g.setFont (Fonts::bold (11.0f));
        g.drawText (typeName, chip, juce::Justification::centred);
    }
    else
    {
        g.drawText (txt ("Selecciona un canal para ver sus ajustes"), top, juce::Justification::centredLeft);
    }
}

void InstrumentPanel::resized()
{
    auto area = getLocalBounds().reduced (10, 6);
    auto top = area.removeFromTop (24);
    helpButton.setBounds (top.removeFromLeft (24).withSizeKeepingCentre (22, 22));
    infoLabel.setBounds (top.removeFromRight (juce::jmax (200, top.getWidth() / 2)));

    area.removeFromTop (4);
    const auto colour = Colours::forChannelType (channel[ids::type].toString());
    for (auto* c : controls)
        if (auto* k = dynamic_cast<Knob*> (c))
            k->getSlider().setColour (juce::Slider::rotarySliderFillColourId, colour);

    // ¿Cuánto ocupa todo menos la forma de onda? Ella usa lo que sobre (y se esconde si no cabe)
    auto widthOf = [] (juce::Component* c)
    {
        if (dynamic_cast<juce::TextButton*> (c) != nullptr) return 130;
        if (auto* tb = dynamic_cast<juce::ToggleButton*> (c))
            return juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (15.0f), tb->getButtonText()) + 44;
        if (dynamic_cast<juce::ComboBox*> (c) != nullptr) return 110;
        if (dynamic_cast<SampleWaveform*> (c) != nullptr) return 0;
        return 60;
    };
    int fixed = 0;
    for (auto* c : controls)
        fixed += widthOf (c) + 6;
    const int waveWidth = juce::jmin (230, area.getWidth() - fixed - 6);

    for (auto* c : controls)
    {
        if (dynamic_cast<SampleWaveform*> (c) != nullptr)
        {
            c->setVisible (waveWidth >= 110);
            if (waveWidth < 110)
                continue;
        }
        int w = 60;
        if (dynamic_cast<juce::TextButton*> (c) != nullptr)   w = 130;
        if (auto* tb = dynamic_cast<juce::ToggleButton*> (c))
            w = juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (15.0f), tb->getButtonText()) + 44;
        if (dynamic_cast<juce::ComboBox*> (c) != nullptr)     w = 110;
        if (dynamic_cast<SampleWaveform*> (c) != nullptr)     w = waveWidth;

        auto slot = area.removeFromLeft (w);
        if (dynamic_cast<Knob*> (c) != nullptr || dynamic_cast<SampleWaveform*> (c) != nullptr)
            c->setBounds (slot);
        else
            c->setBounds (slot.withSizeKeepingCentre (w - 8, 28));
        area.removeFromLeft (6);
    }
}
}
