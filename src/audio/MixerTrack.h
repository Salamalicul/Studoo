#pragma once
// ============================================================================
//  MixerTrack: una pista del mezclador con su cadena de efectos:
//
//     EQ (graves/medios/agudos) -> Compresor -> Delay -> Reverb
//        -> Plugin VST3 (opcional) -> Volumen y panorama -> Medidor
// ============================================================================
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Snapshot.h"

namespace studoo
{
    class MixerTrack
    {
    public:
        void prepare (double sampleRate, int maxBlockSize);
        void reset();

        juce::AudioBuffer<float>& getBuffer() { return buffer; }

        // Procesa 'numSamples' muestras del buffer interno
        void process (int numSamples, const TrackParams& params, double bpm, bool silenced);

        void setPlugin (juce::AudioPluginInstance* p) { plugin = p; }
        juce::AudioPluginInstance* getPlugin() const  { return plugin; }

        // Nivel para el medidor (0..1), lo lee la interfaz
        float getPeak (int channel) const { return peak[(size_t) juce::jlimit (0, 1, channel)].load(); }
        void  decayPeaks();

    private:
        void updateEq (const TrackParams&);

        juce::AudioBuffer<float> buffer, pluginBuffer;
        double sr = 44100.0;

        // EQ: 3 bandas x 2 canales
        std::array<std::array<juce::dsp::IIR::Filter<float>, 2>, 3> eq;
        float lastLow = 1000.0f, lastMid = 1000.0f, lastHigh = 1000.0f;

        juce::dsp::Compressor<float> compressor;

        juce::AudioBuffer<float> delayLine;
        int delayWrite = 0;

        juce::Reverb reverb;
        juce::Reverb::Parameters lastReverb;

        juce::AudioPluginInstance* plugin = nullptr;
        std::array<std::atomic<float>, 2> peak { 0.0f, 0.0f };
    };
}
