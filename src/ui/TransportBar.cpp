#include "TransportBar.h"
#include "HelpTexts.h"
#include "StudooLookAndFeel.h"

namespace studoo
{
// ---------------------------------------------------------------------------
//  IconButton
// ---------------------------------------------------------------------------
IconButton::IconButton (Icon iconType, juce::Colour activeColour)
    : juce::Button ({}), icon (iconType), colour (activeColour)
{
    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);
}

void IconButton::paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    auto area = getLocalBounds().toFloat().reduced (0.5f);
    const bool on = getToggleState();

    g.setColour (on ? colour.withAlpha (0.25f) : Colours::panelLight.brighter (isMouseOver ? 0.12f : 0.0f));
    g.fillRoundedRectangle (area, 4.0f);
    g.setColour (on ? colour : Colours::outline);
    g.drawRoundedRectangle (area, 4.0f, 1.0f);

    auto iconArea = area.withSizeKeepingCentre (area.getHeight() * 0.45f, area.getHeight() * 0.45f);
    if (isButtonDown)
        iconArea = iconArea.reduced (1.0f);

    juce::Path p;
    if (icon == Icon::play)
        p.addTriangle (iconArea.getX(), iconArea.getY(), iconArea.getX(), iconArea.getBottom(),
                       iconArea.getRight(), iconArea.getCentreY());
    else
        p.addRoundedRectangle (iconArea, 1.5f);

    g.setColour (on || isMouseOver ? colour : colour.withMultipliedBrightness (0.8f));
    g.fillPath (p);
}

// ---------------------------------------------------------------------------
//  TransportBar
// ---------------------------------------------------------------------------
static void makeNonFocusing (juce::Component& c)
{
    c.setWantsKeyboardFocus (false);
    c.setMouseClickGrabsKeyboardFocus (false);
}

TransportBar::TransportBar (AudioEngine& e)
    : engine (e), helpButton (txt (help::transportTitle), txt (help::transportText))
{
    addAndMakeVisible (playButton);
    playButton.setTooltip (txt ("Reproducir: empieza a sonar el patrón desde el principio. "
                                "Atajo: barra espaciadora."));
    playButton.onClick = [this] { engine.play(); };

    addAndMakeVisible (stopButton);
    stopButton.setTooltip (txt ("Parar: detiene la música. Atajo: barra espaciadora."));
    stopButton.onClick = [this] { engine.stop(); };

    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setColour (juce::Label::textColourId, Colours::textDim);
    bpmLabel.setJustificationType (juce::Justification::centredRight);

    addAndMakeVisible (bpmSlider);
    bpmSlider.setSliderStyle (juce::Slider::LinearBar);
    bpmSlider.setRange (40.0, 250.0, 1.0);
    bpmSlider.setValue (engine.getBpm(), juce::dontSendNotification);
    bpmSlider.setDoubleClickReturnValue (true, 140.0);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 30);
    bpmSlider.setColour (juce::Slider::textBoxTextColourId, Colours::accent);
    bpmSlider.setTooltip (txt ("BPM (pulsaciones por minuto): la velocidad de tu canción. "
                               "Arrastra a izquierda/derecha para cambiarla. Doble clic: vuelve a 140."));
    bpmSlider.onValueChange = [this] { engine.setBpm (bpmSlider.getValue()); };
    makeNonFocusing (bpmSlider);

    addAndMakeVisible (metronomeButton);
    metronomeButton.setButtonText (txt ("Metrónomo"));
    metronomeButton.setClickingTogglesState (true);
    metronomeButton.setToggleState (engine.isMetronomeEnabled(), juce::dontSendNotification);
    metronomeButton.setTooltip (txt ("Metrónomo: activa un \"clic\" en cada tiempo para ayudarte a "
                                     "seguir el ritmo. Se ilumina en naranja cuando está activo."));
    metronomeButton.onClick = [this] { engine.setMetronomeEnabled (metronomeButton.getToggleState()); };
    makeNonFocusing (metronomeButton);

    addAndMakeVisible (volumeLabel);
    volumeLabel.setText ("Volumen", juce::dontSendNotification);
    volumeLabel.setColour (juce::Label::textColourId, Colours::textDim);
    volumeLabel.setJustificationType (juce::Justification::centredRight);

    addAndMakeVisible (volumeKnob);
    volumeKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    volumeKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volumeKnob.setRange (0.0, 1.0, 0.01);
    volumeKnob.setValue (engine.getMasterVolume(), juce::dontSendNotification);
    volumeKnob.setDoubleClickReturnValue (true, 0.8);
    volumeKnob.setTooltip (txt ("Volumen general: sube o baja el volumen de todo. "
                                "Arrastra hacia arriba o abajo. Doble clic: valor por defecto."));
    volumeKnob.onValueChange = [this] { engine.setMasterVolume ((float) volumeKnob.getValue()); };
    makeNonFocusing (volumeKnob);

    addAndMakeVisible (audioButton);
    audioButton.setButtonText ("Audio");
    audioButton.setTooltip (txt ("Ajustes de audio: elige por dónde suena Studoo (altavoces, auriculares, "
                                 "tarjeta de sonido). En Linux puedes elegir ALSA o JACK."));
    audioButton.onClick = [this] { if (onAudioSettings) onAudioSettings(); };
    makeNonFocusing (audioButton);

    addAndMakeVisible (helpButton);

    startTimerHz (20);
}

void TransportBar::togglePlayStop()
{
    if (engine.isPlaying()) engine.stop();
    else                    engine.play();
}

void TransportBar::timerCallback()
{
    // Encendemos el botón de play mientras suena
    const bool playing = engine.isPlaying();
    if (playButton.getToggleState() != playing)
        playButton.setToggleState (playing, juce::dontSendNotification);
}

void TransportBar::paint (juce::Graphics& g)
{
    g.fillAll (Colours::header);
    g.setColour (Colours::outline);
    g.drawHorizontalLine (getHeight() - 1, 0.0f, (float) getWidth());

    // Logotipo
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.setColour (Colours::accent);
    g.drawText ("STUDOO", logoArea, juce::Justification::centredLeft);
}

void TransportBar::resized()
{
    auto area = getLocalBounds().reduced (12, 9);

    logoArea = area.removeFromLeft (120);
    area.removeFromLeft (8);

    playButton.setBounds (area.removeFromLeft (46));
    area.removeFromLeft (6);
    stopButton.setBounds (area.removeFromLeft (46));
    area.removeFromLeft (24);

    bpmLabel.setBounds (area.removeFromLeft (40));
    area.removeFromLeft (4);
    bpmSlider.setBounds (area.removeFromLeft (110));
    area.removeFromLeft (16);

    metronomeButton.setBounds (area.removeFromLeft (100));

    helpButton.setBounds (area.removeFromRight (30).withSizeKeepingCentre (26, 26));
    area.removeFromRight (10);
    audioButton.setBounds (area.removeFromRight (70));
    area.removeFromRight (20);
    volumeKnob.setBounds (area.removeFromRight (40));
    volumeLabel.setBounds (area.removeFromRight (60));
}
}
