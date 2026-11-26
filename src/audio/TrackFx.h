#pragma once
// ============================================================================
//  TrackFx: efectos avanzados de cada pista del mezclador
//    - NoiseGate: corta el ruido cuando el sonido es flojo
//    - PitchCorrector: el "autotune" (detecta la nota y la lleva a la escala)
//    - ParametricEq: 4 bandas con frecuencia, ganancia y anchura
//    - RhythmFx: half-time, tartamudeo, puerta rítmica y parada de cinta
// ============================================================================
#include <juce_dsp/juce_dsp.h>
#include "Snapshot.h"

namespace studoo
{
    class NoiseGate
    {
    public:
        void prepare (double sampleRate) { sr = sampleRate; env = 0.0f; gain = 1.0f; }
        void process (juce::AudioBuffer<float>&, int n, float thresholdDb);

    private:
        double sr = 44100.0;
        float env = 0.0f, gain = 1.0f;
    };

    class PitchCorrector
    {
    public:
        void prepare (double sampleRate);
        void process (juce::AudioBuffer<float>&, int n, const TrackParams&);
        // Afinador: la última frecuencia detectada (0 = no hay nota clara)
        float getDetectedHz() const { return detectedHz.load(); }
        void setAnalyse (bool on) { analyse = on; }   // detecta aunque el autotune esté apagado

    private:
        float detectPitch();   // YIN sobre el análisis (a media frecuencia de muestreo)

        double sr = 44100.0;
        std::atomic<bool> analyse { false };
        // Análisis
        std::vector<float> analysis;   // ventana mono, a la mitad de muestras
        int analysisPos = 0, sinceDetect = 0;
        bool decimateToggle = false;
        std::vector<float> yin;
        std::atomic<float> detectedHz { 0.0f };
        // Cambio de tono: dos lecturas de un búfer circular que se cruzan
        juce::AudioBuffer<float> ring;
        int writePos = 0;
        double phase = 0.0;
        double ratio = 1.0, targetRatio = 1.0;
    };

    class ParametricEq
    {
    public:
        void prepare (double sampleRate);
        void process (juce::AudioBuffer<float>&, int n, const TrackParams&);
        void reset();

    private:
        double sr = 44100.0;
        std::array<std::array<juce::dsp::IIR::Filter<float>, 2>, numEqBands> filters;
        std::array<float, numEqBands> lastF {}, lastG {}, lastQ {};
    };

    class RhythmFx
    {
    public:
        void prepare (double sampleRate);
        void ensureMemory();   // hilo principal: solo las pistas que lo usan reservan memoria
        bool hasMemory() const { return history.getNumSamples() > 0; }
        void process (juce::AudioBuffer<float>&, int n, const TrackParams&, double positionSteps, double samplesPerStep, bool playing);

    private:
        double sr = 44100.0;
        juce::AudioBuffer<float> history;   // lo que ha sonado (unos 3 segundos)
        int writePos = 0;
        float gateGain = 1.0f;
    };
}
