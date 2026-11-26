#pragma once
// ============================================================================
//  TrackFx: efectos avanzados de cada pista del mezclador
//    - NoiseGate: corta el ruido cuando el sonido es flojo
//    - PitchCorrector: el "autotune" (detecta la nota y la lleva a la escala)
//    - ParametricEq: 4 bandas con frecuencia, ganancia y anchura
//    - RhythmFx: half-time, tartamudeo, puerta rítmica y parada de cinta
//    - ModulationFx: flanger y phaser
//    - TransientShaper: golpes más fuertes o más suaves, cola más larga o corta
//    - MultibandComp: compresor de 3 bandas (graves, medios, agudos)
//    - Vocoder: la voz "de robot" (una pista habla a través del sonido de otra)
//    - widenStereo: hacer el sonido más ancho o más estrecho
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

    class ModulationFx
    {
    public:
        void prepare (double sampleRate, int maxBlock);
        void reset();
        void process (juce::AudioBuffer<float>&, int n, const TrackParams&);

    private:
        double sr = 44100.0;
        static constexpr int flangerSize = 2048;   // unos 40 ms
        std::array<std::array<float, flangerSize>, 2> flangerBuf {};
        int flangerWrite = 0;
        double flangerPhase = 0.0;
        std::array<float, 2> flangerFb {};
        juce::dsp::Phaser<float> phaser;
        float lastRate = -1.0f, lastPhaserMix = -1.0f;
    };

    class TransientShaper
    {
    public:
        void prepare (double sampleRate) { sr = sampleRate; attFast = attSlow = relFast = relSlow = 0.0f; }
        void process (juce::AudioBuffer<float>&, int n, float attack, float sustain);

    private:
        double sr = 44100.0;
        float attFast = 0.0f, attSlow = 0.0f;   // mismo final, distinta subida: detectan el golpe
        float relFast = 0.0f, relSlow = 0.0f;   // misma subida, distinto final: detectan la cola
    };

    class MultibandComp
    {
    public:
        void prepare (double sampleRate, int maxBlock);
        void reset();
        void process (juce::AudioBuffer<float>&, int n, float amount);

    private:
        double sr = 44100.0;
        juce::AudioBuffer<float> low, mid, high;
        juce::dsp::LinkwitzRileyFilter<float> lowCut, midSplitHigh;   // 200 Hz y 3 kHz
        std::array<juce::dsp::Compressor<float>, 3> comps;
        float lastAmount = -1.0f;
    };

    class Vocoder
    {
    public:
        static constexpr int numBands = 16;
        void prepare (double sampleRate);
        void reset();
        // 'carrier' (esta pista, por ejemplo un sinte) toma la forma de 'modulator' (la voz)
        void process (juce::AudioBuffer<float>& carrier, const juce::AudioBuffer<float>& modulator, int n, float mix);

    private:
        double sr = 44100.0;
        std::array<juce::dsp::IIR::Filter<float>, numBands> modFilters, carFilters;
        std::array<float, numBands> env {};
        std::array<float, numBands> bandGain {};
    };

    // 0 = mono, 1 = igual, 2 = muy ancho (medio/lados)
    void widenStereo (juce::AudioBuffer<float>&, int n, float width);
}
