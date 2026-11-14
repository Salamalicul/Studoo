#include "Toolbar.h"
#include "HelpTexts.h"
#include <BinaryData.h>

namespace studoo
{
Toolbar::Toolbar (AppContext& c)
    : ctx (c), helpButton (txt (help::toolbarTitle), txt (help::toolbarText))
{
    addAndMakeVisible (fileButton);
    fileButton.setTooltip (txt ("Archivo: nuevo, abrir, guardar, exportar a WAV/MP3, tutorial y atajos de teclado."));
    fileButton.onClick = [this] { if (onFileMenu) onFileMenu(); };

    addAndMakeVisible (undoButton);
    undoButton.setTooltip (txt ("Deshacer el último cambio (Ctrl+Z)."));
    undoButton.onClick = [this] { ctx.project.getUndo().undo(); };

    addAndMakeVisible (redoButton);
    redoButton.setTooltip (txt ("Rehacer lo que deshiciste (Ctrl+Y)."));
    redoButton.onClick = [this] { ctx.project.getUndo().redo(); };

    addAndMakeVisible (modeArea);
    for (auto* b : { &patternModeButton, &songModeButton })
    {
        modeArea.addAndMakeVisible (*b);
        makeNonFocusing (*b);
        b->setClickingTogglesState (false);
    }
    patternModeButton.setButtonText (txt ("PATRÓN"));
    patternModeButton.setTooltip (txt ("Modo PATRÓN: repite solo el patrón que estás editando. "
                                       "Ideal para crear el beat. (Tecla L)"));
    patternModeButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    patternModeButton.onClick = [this] { ctx.project.getState().setProperty (ids::playMode, "pattern", nullptr); };

    songModeButton.setButtonText (txt ("CANCIÓN"));
    songModeButton.setTooltip (txt ("Modo CANCIÓN: reproduce la playlist completa, con todos los patrones "
                                    "colocados en orden. (Tecla L)"));
    songModeButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    songModeButton.onClick = [this] { ctx.project.getState().setProperty (ids::playMode, "song", nullptr); };

    addAndMakeVisible (playButton);
    playButton.setTooltip (txt ("Reproducir (barra espaciadora)."));
    playButton.onClick = [this] { ctx.engine.play(); };

    addAndMakeVisible (stopButton);
    stopButton.setTooltip (txt ("Parar y volver al principio (barra espaciadora)."));
    stopButton.onClick = [this] { ctx.engine.stop(); };

    addAndMakeVisible (recordButton);
    recordButton.setTooltip (txt ("Grabar audio con el micrófono (tecla R). Se graba en la playlist, en modo CANCIÓN. "
                                  "Pulsa otra vez para terminar."));
    recordButton.onClick = [this] { if (onRecord) onRecord(); };

    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setColour (juce::Label::textColourId, Colours::textDim);
    bpmLabel.setFont (Fonts::bold (12.0f));
    bpmLabel.setJustificationType (juce::Justification::centredRight);

    addAndMakeVisible (bpmSlider);
    bpmSlider.setSliderStyle (juce::Slider::LinearBar);
    bpmSlider.setRange (40.0, 250.0, 1.0);
    bpmSlider.setDoubleClickReturnValue (true, 140.0);
    bpmSlider.setColour (juce::Slider::textBoxTextColourId, Colours::accent);
    bpmSlider.setTooltip (txt ("BPM (pulsaciones por minuto): la velocidad de tu canción. Arrastra a izquierda/derecha "
                               "para cambiarla. Doble clic: 140."));
    bpmSlider.onDragStart = [this] { ctx.project.beginTransaction(); };
    makeNonFocusing (bpmSlider);

    addAndMakeVisible (positionLabel);
    positionLabel.setJustificationType (juce::Justification::centred);
    positionLabel.setFont (Fonts::bold (20.0f));
    positionLabel.setColour (juce::Label::textColourId, Colours::accentLight);
    positionLabel.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    positionLabel.setTooltip (txt ("Posición: compás . tiempo"));

    addAndMakeVisible (metronomeButton);
    metronomeButton.setButtonText (txt ("Metrónomo"));
    metronomeButton.setClickingTogglesState (true);
    metronomeButton.setTooltip (txt ("Metrónomo: un \"clic\" en cada tiempo para seguir el ritmo (Ctrl+M)."));
    makeNonFocusing (metronomeButton);

    addAndMakeVisible (volumeKnob);

    addAndMakeVisible (audioButton);
    audioButton.setButtonText ("Audio");
    audioButton.setTooltip (txt ("Ajustes de audio y MIDI: altavoces, auriculares, micrófono y teclados MIDI. "
                                 "En Linux puedes elegir ALSA o JACK."));
    audioButton.onClick = [this] { if (onAudioSettings) onAudioSettings(); };
    makeNonFocusing (audioButton);

    addAndMakeVisible (pluginsButton);
    pluginsButton.setButtonText ("Plugins");
    pluginsButton.setTooltip (txt ("Busca los plugins VST3 instalados en tu ordenador para poder usarlos."));
    pluginsButton.onClick = [this] { if (onPlugins) onPlugins(); };
    makeNonFocusing (pluginsButton);

    addAndMakeVisible (helpButton);

    for (auto* b : { (juce::Component*) &undoButton, (juce::Component*) &redoButton })
        makeNonFocusing (*b);

    ctx.project.addChangeListener (this);
    bindToProject();
    startTimerHz (30);
}

Toolbar::~Toolbar()
{
    ctx.project.removeChangeListener (this);
}

void Toolbar::bindToProject()
{
    auto& state = ctx.project.getState();
    bpmSlider.getValueObject().referTo (state.getPropertyAsValue (ids::bpm, ctx.project.um()));
    metronomeButton.getToggleStateValue().referTo (state.getPropertyAsValue (ids::metronome, nullptr));
    volumeKnob.bind (ctx.project.getMixerTrack (0), ids::volume, ctx.project.um(), 0.0, 1.0, 0.8);
    updateModeButtons();
}

void Toolbar::changeListenerCallback (juce::ChangeBroadcaster*)
{
    updateModeButtons();
}

void Toolbar::updateModeButtons()
{
    const bool song = ctx.project.getState()[ids::playMode].toString() == "song";
    patternModeButton.setToggleState (! song, juce::dontSendNotification);
    songModeButton.setToggleState (song, juce::dontSendNotification);
    undoButton.setEnabled (ctx.project.getUndo().canUndo());
    redoButton.setEnabled (ctx.project.getUndo().canRedo());
}

void Toolbar::togglePlayStop()
{
    if (ctx.engine.isPlaying()) ctx.engine.stop();
    else                        ctx.engine.play();
}

void Toolbar::timerCallback()
{
    const bool playing = ctx.engine.isPlaying();
    if (playButton.getToggleState() != playing)
        playButton.setToggleState (playing, juce::dontSendNotification);

    const bool rec = ctx.engine.isRecording();
    if (recordButton.getToggleState() != rec)
        recordButton.setToggleState (rec, juce::dontSendNotification);

    const double pos = ctx.engine.getPlayPosition();
    const int bar = (int) (pos / stepsPerBar) + 1;
    const int beat = (int) std::fmod (pos, (double) stepsPerBar) / stepsPerBeat + 1;
    positionLabel.setText (juce::String (bar).paddedLeft ('0', 3) + " . " + juce::String (beat), juce::dontSendNotification);

    undoButton.setEnabled (ctx.project.getUndo().canUndo());
    redoButton.setEnabled (ctx.project.getUndo().canRedo());

    // Medidor del volumen general (Master)
    const float l = ctx.engine.getTrackPeak (0, 0), r = ctx.engine.getTrackPeak (0, 1);
    if (std::abs (l - meterL) > 0.003f || std::abs (r - meterR) > 0.003f)
    {
        meterL = l;
        meterR = r;
        repaint (meterArea);
    }
}

void Toolbar::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (Colours::panelLight.darker (0.1f), 0.0f, 0.0f,
                                             Colours::header, 0.0f, (float) getHeight(), false));
    g.fillAll();
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, (float) getWidth());
    // Línea naranja con brillo debajo del logo
    g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.8f), 0.0f, 0.0f,
                                             Colours::accent.withAlpha (0.0f), (float) getWidth() * 0.5f, 0.0f, false));
    g.fillRect (0, getHeight() - 2, getWidth() / 2, 1);

    // Logo
    auto logo = juce::ImageCache::getFromMemory (BinaryData::icon_png, BinaryData::icon_pngSize);
    auto area = logoArea;
    auto iconArea = area.removeFromLeft (logoArea.getHeight()).toFloat();
    Draw::glow (g, iconArea.reduced (6.0f), Colours::accent, 6.0f, 8.0f);
    g.setOpacity (1.0f);
    g.drawImage (logo, iconArea, juce::RectanglePlacement::centred);
    area.removeFromLeft (8);
    g.setFont (Fonts::display (24.0f));
    g.setGradientFill (juce::ColourGradient (Colours::accentLight, 0.0f, (float) area.getY(),
                                             Colours::accentDeep, 0.0f, (float) area.getBottom(), false));
    g.drawText ("STUDOO", area, juce::Justification::centredLeft);

    // Cápsula de reproducción y pantalla de posición
    auto pill = transportArea.toFloat().expanded (5.0f, 4.0f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (pill, 12.0f);
    g.setColour (Colours::outline.withAlpha (0.8f));
    g.drawRoundedRectangle (pill.reduced (0.5f), 12.0f, 1.0f);
    Draw::lcd (g, positionLabel.getBounds().toFloat());
    g.setColour (Colours::textDim.withAlpha (0.7f));
    g.setFont (Fonts::regular (9.0f));
    g.drawText (txt ("COMPÁS . TIEMPO"), positionLabel.getBounds().removeFromBottom (11), juce::Justification::centred);

    // Medidor Master: dos barras LED (izquierda y derecha)
    if (! meterArea.isEmpty())
    {
        auto m = meterArea.toFloat();
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (m, 4.0f);
        g.setColour (Colours::outline.withAlpha (0.8f));
        g.drawRoundedRectangle (m.reduced (0.5f), 4.0f, 1.0f);
        auto inner = m.reduced (3.0f);
        const float barW = (inner.getWidth() - 2.0f) * 0.5f;
        const int segments = 14;
        const float segH = inner.getHeight() / (float) segments;
        for (int ch = 0; ch < 2; ++ch)
        {
            const float level = juce::jlimit (0.0f, 1.0f, std::sqrt (ch == 0 ? meterL : meterR));
            const int lit = juce::roundToInt (level * (float) segments);
            for (int sIdx = 0; sIdx < segments; ++sIdx)
            {
                auto seg = juce::Rectangle<float> (inner.getX() + (float) ch * (barW + 2.0f),
                                                   inner.getBottom() - (float) (sIdx + 1) * segH, barW, segH - 1.0f);
                const juce::Colour c = sIdx >= segments - 2 ? Colours::red
                                     : sIdx >= segments - 5 ? juce::Colour (0xffffd24a) : Colours::green;
                g.setColour (sIdx < lit ? c : c.withAlpha (0.12f));
                g.fillRect (seg);
            }
        }
    }
}

void Toolbar::resized()
{
    auto area = getLocalBounds().reduced (10, 9);
    auto gap = [&area] (int w) { area.removeFromLeft (w); };

    logoArea = area.removeFromLeft (142);
    fileButton.setBounds (area.removeFromLeft (38)); gap (10);
    undoButton.setBounds (area.removeFromLeft (34)); gap (4);
    redoButton.setBounds (area.removeFromLeft (34)); gap (14);

    modeArea.setBounds (area.removeFromLeft (170));
    {
        auto m = modeArea.getLocalBounds();
        patternModeButton.setBounds (m.removeFromLeft (m.getWidth() / 2));
        songModeButton.setBounds (m);
    }
    gap (10);
    gap (4);
    transportArea = area.removeFromLeft (140);
    {
        auto t = transportArea;
        playButton.setBounds (t.removeFromLeft (44)); t.removeFromLeft (4);
        stopButton.setBounds (t.removeFromLeft (44)); t.removeFromLeft (4);
        recordButton.setBounds (t.removeFromLeft (44));
    }
    gap (16);

    bpmLabel.setBounds (area.removeFromLeft (36)); gap (4);
    bpmSlider.setBounds (area.removeFromLeft (84)); gap (10);
    positionLabel.setBounds (area.removeFromLeft (100)); gap (10);
    metronomeButton.setBounds (area.removeFromLeft (96));

    helpButton.setBounds (area.removeFromRight (28).withSizeKeepingCentre (26, 26));
    area.removeFromRight (8);
   #if JUCE_ANDROID
    pluginsButton.setVisible (false); // en los tablets no hay plugins VST3
   #else
    pluginsButton.setBounds (area.removeFromRight (70));
    area.removeFromRight (6);
   #endif
    audioButton.setBounds (area.removeFromRight (60));
    area.removeFromRight (8);
    meterArea = area.removeFromRight (16).withSizeKeepingCentre (16, 40);
    area.removeFromRight (2);
    volumeKnob.setBounds (area.removeFromRight (52).withTrimmedTop (-6).withTrimmedBottom (-6));
}
}
