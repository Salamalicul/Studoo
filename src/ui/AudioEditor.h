#pragma once
// ============================================================================
//  AudioEditor: editor de grabaciones (como un editor de audio sencillo).
//  Arrastra sobre la onda para elegir un trozo y aplícale un efecto; si no
//  eliges nada, se aplica a todo. Al guardar se crea una copia del archivo.
// ============================================================================
#include "AppContext.h"
#include "Widgets.h"

namespace studoo
{
    class AudioEditorPanel : public juce::Component
    {
    public:
        AudioEditorPanel (AppContext&, juce::ValueTree channel);

        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;

        // Lo que hace cada botón (también lo usan las pruebas)
        enum class Op { silence, fadeIn, fadeOut, reverse, normalise, louder, quieter, denoise, crop, remove };
        static void apply (juce::AudioBuffer<float>&, Op, int from, int to);

    private:
        void run (Op);
        void save();
        juce::Rectangle<int> waveArea() const;
        int xToSample (float x) const;
        float sampleToX (int s) const;

        AppContext& ctx;
        juce::ValueTree channel;
        double sampleRate = 44100.0;
        juce::AudioBuffer<float> audio;
        std::vector<juce::AudioBuffer<float>> history;   // para deshacer dentro del editor
        int selStart = 0, selEnd = 0, anchor = 0;
        juce::OwnedArray<juce::TextButton> buttons;
        juce::String status;
    };
}
