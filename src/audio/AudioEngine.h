#pragma once
// ============================================================================
//  AudioEngine: el "corazón" que produce el sonido.
//
//  Cómo funciona, paso a paso:
//   1. La tarjeta de sonido pide trocitos de audio (unas 512 muestras, ~10 ms).
//   2. El motor mira qué pasos y notas caen en ese trocito (del patrón actual
//      en modo PATRÓN, o de la playlist en modo CANCIÓN).
//   3. Envía esas notas a cada instrumento, que genera su sonido.
//   4. Cada canal va a una pista del mezclador, que aplica sus efectos.
//   5. Todas las pistas se suman en el Master y salen por los altavoces.
//
//  La interfaz y el audio funcionan en hilos distintos. El motor recibe
//  "fotos" del proyecto (Snapshot) y usa un candado (lock) muy breve para
//  cambiarlas de forma segura.
// ============================================================================
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>

#include "Instruments.h"
#include "MixerTrack.h"
#include "SampleCache.h"
#include "Snapshot.h"

namespace studoo
{
    class AudioEngine : public juce::AudioIODeviceCallback,
                        public juce::AudioPlayHead
    {
    public:
        AudioEngine();
        ~AudioEngine() override;

        // --- Conexión con la tarjeta de sonido --------------------------------
        void audioDeviceAboutToStart (juce::AudioIODevice*) override;
        void audioDeviceStopped() override;
        void audioDeviceIOCallbackWithContext (const float* const* inputs, int numInputs,
                                               float* const* outputs, int numOutputs, int numSamples,
                                               const juce::AudioIODeviceCallbackContext&) override;

        void prepare (double sampleRate, int maxBlockSize);
        double getSampleRate() const { return sampleRate; }
        int getBlockSize() const     { return blockSize; }

        // Genera audio (lo usa la tarjeta de sonido y también la exportación)
        void render (const float* const* inputs, int numInputs, float* const* outputs, int numOutputs, int numSamples);

        // --- Proyecto ----------------------------------------------------------
        // Crea una foto nueva del proyecto y se la pasa al hilo de audio
        void updateFromProject (const juce::ValueTree& projectState, SampleCache&);

        // --- Transporte ----------------------------------------------------------
        void play();
        void stop();
        bool isPlaying() const              { return playing.load(); }
        void setPlayPosition (double steps);
        double getPlayPosition() const      { return position.load(); }
        bool isSongMode() const             { return songModeFlag.load(); }

        // --- Sonar notas desde la interfaz o un teclado MIDI ------------------------
        void previewNote (int channelUid, int note, float velocity, double seconds);
        void setLiveChannel (int channelUid) { liveChannelUid.store (channelUid); }
        juce::MidiMessageCollector& getMidiCollector() { return midiCollector; }

        // --- Medidores del mezclador -------------------------------------------------
        float getTrackPeak (int track, int channel) const;
        void decayMeters();

        // --- Plugins VST3 --------------------------------------------------------------
        void setChannelPlugin (int channelUid, std::unique_ptr<juce::AudioPluginInstance>);
        juce::AudioPluginInstance* getChannelPlugin (int channelUid);
        void setMixerPlugin (int track, std::unique_ptr<juce::AudioPluginInstance>);
        juce::AudioPluginInstance* getMixerPlugin (int track);
        void clearAllPlugins();
        std::function<void (juce::AudioPluginInstance*)> onPluginWillBeDeleted;

        // --- Grabación de audio -------------------------------------------------------
        bool startRecording (const juce::File& file, juce::String& error);
        struct RecordingResult { juce::File file; double startSteps = 0.0, lengthSteps = 0.0; bool valid = false; };
        RecordingResult stopRecording();
        bool isRecording() const { return recording.load(); }
        int getNumInputChannels() const { return numInputChannels; }

        // --- Exportación (sin tarjeta de sonido) ---------------------------------------
        // Debe llamarse con el motor desconectado de la tarjeta de sonido
        bool renderSongToFile (const juce::File& wavFile, double sampleRate,
                               std::function<bool (float progress)> keepGoing, juce::String& error);

        // --- AudioPlayHead (para que los plugins sepan el tempo) ---------------------------
        juce::Optional<PositionInfo> getPosition() const override;

    private:
        struct ChannelRuntime
        {
            int uid = 0;
            ChannelType type = ChannelType::sampler;
            std::unique_ptr<Instrument> instrument;
            SamplePtr audioSample; // para canales de audio grabado
            juce::String audioSamplePath;
            juce::MidiBuffer midi;
            juce::AudioBuffer<float> buffer;
            std::array<juce::int64, 128> noteOffAt;
        };

        struct PreviewEvent { int channelUid = 0, note = 60; float velocity = 0.8f; int lengthSamples = 0; };

        void renderChunk (const float* const* inputs, int numInputs, float* const* outputs, int numOutputs,
                          int outputOffset, int n);
        void scheduleNote (ChannelRuntime&, int note, float velocity, int sampleOffset, juce::int64 lengthSamples);
        void collectEvents (const Snapshot&, double from, double to, int sampleOffset, double samplesPerStep);
        void collectPatternEvents (const Snapshot&, const PatternSnap&, double localFrom, double localTo,
                                   double localEnd, double globalShift, double segmentStart,
                                   int sampleOffset, double samplesPerStep);
        void renderAudioClips (const Snapshot&, double from, double to, int sampleOffset, double samplesPerStep);
        std::unique_ptr<ChannelRuntime> createRuntime (const ChannelSnap&);
        void releaseAllNotes();

        int currentChunkSize = 0;
        bool wasPlaying = false;
        bool lastSongMode = false;
        std::atomic<bool> exportingSong { false };

        juce::CriticalSection lock;
        std::shared_ptr<const Snapshot> snapshot;
        std::vector<std::unique_ptr<ChannelRuntime>> runtimes;
        std::array<MixerTrack, numMixerTracks> mixer;
        MetronomeInstrument metronome;

        std::map<int, std::unique_ptr<juce::AudioPluginInstance>> channelPlugins;
        std::array<std::unique_ptr<juce::AudioPluginInstance>, numMixerTracks> mixerPlugins;

        double sampleRate = 44100.0;
        int blockSize = 512;
        int numInputChannels = 0;
        juce::int64 sampleClock = 0;

        std::atomic<bool> playing { false }, songModeFlag { false };
        std::atomic<double> position { 0.0 };
        std::atomic<double> requestedPosition { -1.0 };
        std::atomic<double> currentBpm { 140.0 };
        std::atomic<int> liveChannelUid { 0 };

        juce::AbstractFifo previewFifo { 64 };
        std::array<PreviewEvent, 64> previewEvents;

        juce::MidiMessageCollector midiCollector;
        juce::MidiBuffer liveMidi;

        // Grabación
        juce::TimeSliceThread writerThread { "Studoo grabación" };
        std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> recordWriter;
        juce::File recordFile;
        std::atomic<bool> recording { false };
        double recordStartSteps = -1.0;
        juce::int64 recordedSamples = 0;
        juce::AudioBuffer<float> recordScratch;
    };
}
