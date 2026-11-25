#pragma once
// ============================================================================
//  TrackFxPanel: los "Más efectos" de una pista del mezclador (se abre en una
//  ventanita): envíos 2 y 3, autotune con afinador, puerta de ruido,
//  maximizador, efecto de ritmo y EQ paramétrico de 4 bandas.
// ============================================================================
#include "AppContext.h"
#include "Widgets.h"

namespace studoo
{
    class TrackFxPanel : public juce::Component, private juce::Timer
    {
    public:
        TrackFxPanel (AppContext&, int trackIndex);
        ~TrackFxPanel() override;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void timerCallback() override;
        Knob* knob (const juce::String& label, const juce::String& tip, const juce::Identifier& id,
                    double min, double max, double def, double skewMid = 0.0);
        juce::ComboBox* combo (const juce::String& tip);
        juce::Rectangle<int> eqCurveArea;
        void drawEqCurve (juce::Graphics&);

        AppContext& ctx;
        const int index;
        juce::ValueTree track;
        juce::OwnedArray<juce::Component> owned;
        std::array<juce::ComboBox*, numSends - 1> sendBoxes {};
        std::array<Knob*, numSends - 1> sendKnobs {};
        Knob *tuneAmount = nullptr, *gate = nullptr, *maximize = nullptr, *rhythmMix = nullptr;
        juce::ComboBox *tuneRoot = nullptr, *tuneScale = nullptr, *rhythmMode = nullptr;
        std::array<std::array<Knob*, 3>, numEqBands> eqKnobs {};
        juce::String tunerText = "-";
        float tunerCents = 0.0f;
        bool tunerActive = false;
    };
}

namespace studoo
{
    // Analizador de frecuencias del Master (graves a la izquierda, agudos a la derecha)
    class AnalyzerPanel : public juce::Component, private juce::Timer
    {
    public:
        explicit AnalyzerPanel (AppContext&);
        void paint (juce::Graphics&) override;

    private:
        void timerCallback() override;
        AppContext& ctx;
        static constexpr int order = 12, size = 1 << order;
        juce::dsp::FFT fft { order };
        juce::dsp::WindowingFunction<float> window { (size_t) size, juce::dsp::WindowingFunction<float>::hann };
        std::array<float, size * 2> data {};
        std::array<float, 512> levels {};   // dB suavizados por columna
    };
}
