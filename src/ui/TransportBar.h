#pragma once
// ============================================================================
//  TransportBar: la barra superior con Reproducir, Parar, BPM, metrónomo,
//  volumen general y ajustes de audio.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>
#include "audio/AudioEngine.h"
#include "HelpButton.h"

namespace studoo
{
    // Botón que dibuja un icono (triángulo de play o cuadrado de stop)
    class IconButton : public juce::Button
    {
    public:
        enum class Icon { play, stop };
        IconButton (Icon iconType, juce::Colour activeColour);
        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    private:
        Icon icon;
        juce::Colour colour;
    };

    class TransportBar : public juce::Component, private juce::Timer
    {
    public:
        explicit TransportBar (AudioEngine& engine);

        void togglePlayStop();
        std::function<void()> onAudioSettings; // lo rellena la ventana principal

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void timerCallback() override;

        AudioEngine& engine;

        IconButton playButton { IconButton::Icon::play, juce::Colour (0xff5fd35a) };
        IconButton stopButton { IconButton::Icon::stop, juce::Colour (0xffff8a1f) };
        juce::Label bpmLabel;
        juce::Slider bpmSlider;
        juce::TextButton metronomeButton;
        juce::Label volumeLabel;
        juce::Slider volumeKnob;
        juce::TextButton audioButton;
        HelpButton helpButton;

        juce::Rectangle<int> logoArea;
    };
}
