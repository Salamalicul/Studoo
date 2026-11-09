#pragma once
// ============================================================================
//  Instruments: los instrumentos que producen sonido a partir de notas.
//
//  Todos reciben "mensajes MIDI" (nota encendida / nota apagada) y escriben
//  sonido en un buffer estéreo:
//    - SamplerInstrument: reproduce un archivo de audio (bombo, voz, etc.),
//      más agudo o más grave según la nota.
//    - SynthInstrument: sintetizador con 2 osciladores, filtro y envolvente.
//    - Bass808Instrument: bajo 808 con caída de tono, glide y distorsión.
//    - PluginInstrument: un instrumento VST3 externo.
//    - MetronomeInstrument: el "clic" del metrónomo.
// ============================================================================
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SampleCache.h"
#include "Snapshot.h"

namespace studoo
{
    class Instrument
    {
    public:
        virtual ~Instrument() = default;
        virtual void prepare (double sampleRate, int maxBlockSize) = 0;
        virtual void setParameters (const ChannelSnap&) {}
        // Suma el sonido a 'out' (2 canales) durante numSamples muestras
        virtual void render (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi, int numSamples) = 0;
        virtual void allNotesOff() = 0;
    };

    // Base que trocea el bloque en cada evento MIDI para que todo suene a tiempo exacto
    class VoiceInstrument : public Instrument
    {
    public:
        void render (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi, int numSamples) override;

    protected:
        virtual void renderVoices (juce::AudioBuffer<float>& out, int start, int num) = 0;
        virtual void noteOn (int note, float velocity) = 0;
        virtual void noteOff (int note) = 0;
    };

    // ------------------------------------------------------------------------
    class SamplerInstrument : public VoiceInstrument
    {
    public:
        void prepare (double sampleRate, int) override { sr = sampleRate; }
        void setParameters (const ChannelSnap&) override;
        void allNotesOff() override;

        // Cambia el sample y devuelve el anterior (para liberarlo fuera del hilo de audio)
        SamplePtr setSample (SamplePtr newSample);
        const juce::String& getSamplePath() const { return samplePath; }
        void setSamplePath (const juce::String& p) { samplePath = p; }

    protected:
        void renderVoices (juce::AudioBuffer<float>&, int start, int num) override;
        void noteOn (int note, float velocity) override;
        void noteOff (int note) override;

    private:
        struct Voice
        {
            bool active = false;
            int note = 0;
            double position = 0.0, increment = 1.0;
            float gain = 1.0f, env = 1.0f;
            bool releasing = false;
            juce::uint32 age = 0;
        };

        std::array<Voice, 16> voices;
        SamplePtr sample;
        juce::String samplePath;
        double sr = 44100.0;
        int root = 60;
        bool oneShot = true;
        juce::uint32 ageCounter = 0;
    };

    // ------------------------------------------------------------------------
    class SynthInstrument : public VoiceInstrument
    {
    public:
        void prepare (double sampleRate, int maxBlockSize) override;
        void setParameters (const ChannelSnap&) override;
        void allNotesOff() override;

    protected:
        void renderVoices (juce::AudioBuffer<float>&, int start, int num) override;
        void noteOn (int note, float velocity) override;
        void noteOff (int note) override;

    private:
        struct Voice
        {
            int note = -1;
            double phase1 = 0.0, phase2 = 0.0, inc1 = 0.0, inc2 = 0.0;
            float velocity = 0.0f;
            juce::ADSR adsr;
            juce::dsp::StateVariableTPTFilter<float> filter;
            juce::uint32 age = 0;
        };

        float oscillator (double phase, double inc) const;
        void updateVoiceFrequency (Voice&);

        std::array<Voice, 12> voices;
        SynthParams params;
        double sr = 44100.0;
        juce::uint32 ageCounter = 0;
    };

    // ------------------------------------------------------------------------
    class Bass808Instrument : public VoiceInstrument
    {
    public:
        void prepare (double sampleRate, int) override { sr = sampleRate; }
        void setParameters (const ChannelSnap&) override;
        void allNotesOff() override;

    protected:
        void renderVoices (juce::AudioBuffer<float>&, int start, int num) override;
        void noteOn (int note, float velocity) override;
        void noteOff (int note) override;

    private:
        Bass808Params params;
        double sr = 44100.0;
        bool active = false, releasing = false;
        int note = -1;
        double freq = 55.0, targetFreq = 55.0, phase = 0.0, timeSinceAttack = 0.0;
        float amp = 0.0f, velocity = 1.0f;
    };

    // ------------------------------------------------------------------------
    class PluginInstrument : public Instrument
    {
    public:
        void prepare (double sampleRate, int maxBlockSize) override;
        void render (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi, int numSamples) override;
        void allNotesOff() override { pendingAllNotesOff = true; }

        void setPlugin (juce::AudioPluginInstance* p) { plugin = p; }
        juce::AudioPluginInstance* getPlugin() const  { return plugin; }

    private:
        juce::AudioPluginInstance* plugin = nullptr;
        juce::AudioBuffer<float> buffer;
        juce::MidiBuffer midiCopy;
        bool pendingAllNotesOff = false;
    };

    // ------------------------------------------------------------------------
    class MetronomeInstrument
    {
    public:
        void prepare (double sampleRate);
        void trigger (bool accent, int sampleOffset);
        void render (juce::AudioBuffer<float>& out, int numSamples);

    private:
        juce::AudioBuffer<float> high, low;
        const juce::AudioBuffer<float>* current = nullptr;
        int position = 0, startOffset = -1;
        const juce::AudioBuffer<float>* pending = nullptr;
    };
}
