#pragma once
// ============================================================================
//  StepSequencer: el secuenciador de patrones (en FL Studio se llama
//  "Channel Rack"). Cada fila es un sonido y cada cuadrado un paso.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>
#include "audio/AudioEngine.h"
#include "HelpButton.h"

namespace studoo
{
    // La cuadrícula de pasos de todas las filas
    class StepGrid : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        explicit StepGrid (AudioEngine& engine);

        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;

        int  getRowHeight() const;
        void setPlayingStep (int step);

    private:
        juce::Rectangle<float> getCellBounds (int channel, int step) const;
        bool cellAt (juce::Point<float> pos, int& channel, int& step) const;
        void applyPaint (const juce::MouseEvent&);

        AudioEngine& engine;
        int playingStep = -1;
        bool paintValue = true;   // al arrastrar: ¿encender o apagar?
        int  paintChannel = -1;   // al arrastrar solo pintamos en la misma fila
    };

    // Botón de "luz" verde para silenciar un canal
    class MuteLight : public juce::Button
    {
    public:
        MuteLight();
        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;
    };

    class StepSequencer : public juce::Component, private juce::Timer
    {
    public:
        explicit StepSequencer (AudioEngine& engine);

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void timerCallback() override;

        struct ChannelControls
        {
            MuteLight muteLight;
            juce::TextButton nameButton;
            juce::Slider volumeKnob;
        };

        AudioEngine& engine;
        juce::OwnedArray<ChannelControls> channels;
        StepGrid grid;
        HelpButton helpButton;
        juce::TextButton exampleButton, clearButton;

        juce::Rectangle<int> headerArea, numbersArea;
    };
}
