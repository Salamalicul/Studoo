#include "InstrumentPanel.h"
#include "HelpTexts.h"

namespace studoo
{
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
        juce::String text = path.startsWith ("builtin:") ? txt ("Sonido incluido en Studoo")
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

    auto top = getLocalBounds().reduced (10, 6).removeFromTop (24).withTrimmedLeft (34);
    g.setFont (Fonts::bold (15.0f));
    g.setColour (Colours::text);
    if (channel.isValid())
    {
        g.drawText (title, top, juce::Justification::centredLeft);
        const int w = juce::GlyphArrangement::getStringWidthInt (Fonts::bold (15.0f), title);
        auto chip = juce::Rectangle<float> ((float) (top.getX() + w + 12), (float) top.getY() + 3.0f,
                                            (float) juce::GlyphArrangement::getStringWidthInt (Fonts::bold (11.0f), typeName) + 16.0f, 18.0f);
        g.setColour (Colours::accent.withAlpha (0.18f));
        g.fillRoundedRectangle (chip, 9.0f);
        g.setColour (Colours::accentLight);
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
    for (auto* c : controls)
    {
        int w = 60;
        if (dynamic_cast<juce::TextButton*> (c) != nullptr)   w = 130;
        if (dynamic_cast<juce::ToggleButton*> (c) != nullptr) w = 190;
        if (dynamic_cast<juce::ComboBox*> (c) != nullptr)     w = 110;

        auto slot = area.removeFromLeft (w);
        if (dynamic_cast<Knob*> (c) != nullptr)
            c->setBounds (slot);
        else
            c->setBounds (slot.withSizeKeepingCentre (w - 8, 28));
        area.removeFromLeft (6);
    }
}
}
