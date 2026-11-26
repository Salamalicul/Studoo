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

        // --- Modo directo: lanzar patrones en directo (empiezan y paran en el siguiente compás) ---
        static constexpr int numLiveSlots = 16;
        void setLiveMode (bool on)          { liveMode.store (on); }
        bool isLiveMode() const             { return liveMode.load(); }
        void toggleLivePattern (int patternUid);
        // 0 = apagado, 1 = sonando, 2 = va a empezar, 3 = va a parar
        int getLiveState (int patternUid) const;

        // --- Sonar notas desde la interfaz o un teclado MIDI ------------------------
        void previewNote (int channelUid, int note, float velocity, double seconds);
        void setLiveChannel (int channelUid) { liveChannelUid.store (channelUid); }

        // --- Grabar notas del teclado MIDI en el patrón ----------------------------
        struct RecordedMidi { bool noteOn; int note; float velocity; double position; };   // posición en pasos
        void setMidiRecording (bool on) { midiRecording.store (on); }
        bool isMidiRecording() const { return midiRecording.load(); }
        // Hilo principal: recoge lo que se ha tocado desde la última vez
        std::vector<RecordedMidi> takeRecordedMidi();
        juce::MidiMessageCollector& getMidiCollector() { return midiCollector; }

        // --- Escuchar un sonido ya preparado (juegos de oído de Aprende) ----------------
        // Suena por encima de todo lo demás, una vez. Otro sonido corta al anterior.
        using SharedBuffer = std::shared_ptr<const juce::AudioBuffer<float>>;
        void playBuffer (SharedBuffer buffer, double bufferSampleRate, float gain = 0.8f);
        void stopBuffer() { playBuffer (nullptr, 44100.0); }

        // --- Medidores del mezclador -------------------------------------------------
        float getTrackPeak (int track, int channel) const;
        // Nivel de sonido de un canal (para las luces de actividad del secuenciador)
        float getChannelActivity (int channelUid) const { return channelActivity[(size_t) (channelUid & 255)].load(); }
        void decayMeters();

        // --- Plugins VST3 --------------------------------------------------------------
        void setChannelPlugin (int channelUid, std::unique_ptr<juce::AudioPluginInstance>);
        juce::AudioPluginInstance* getChannelPlugin (int channelUid);
        void setMixerPlugin (int track, std::unique_ptr<juce::AudioPluginInstance>, int slot = 0);
        juce::AudioPluginInstance* getMixerPlugin (int track, int slot = 0);
        // Analizador: copia los últimos 'num' sonidos del Master (mezcla de los dos canales)
        static constexpr int scopeSize = 4096;
        void getScope (float* dest, int num) const
        {
            const int end = scopePos.load();
            for (int i = 0; i < num; ++i)
                dest[i] = scope[(size_t) ((end - num + i) & (scopeSize - 1))];
        }
        // Afinador: la nota que suena en una pista (Hz, 0 = ninguna)
        float getDetectedPitch (int track) const { return juce::isPositiveAndBelow (track, numMixerTracks) ? mixer[(size_t) track].getDetectedHz() : 0.0f; }
        void setPitchAnalysis (int track, bool on) { if (juce::isPositiveAndBelow (track, numMixerTracks)) mixer[(size_t) track].setAnalysePitch (on); }
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
        // maxSeconds > 0: solo se exporta ese tiempo (versión gratis)
        // Exportar por pistas: solo suena esta pista del mezclador (sin los efectos del Master). -1 = normal
        void setStemTrack (int track) { stemTrack.store (track); }
        bool renderSongToFile (const juce::File& wavFile, double sampleRate,
                               std::function<bool (float progress)> keepGoing, juce::String& error,
                               double maxSeconds = 0.0);

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
        void scheduleNote (ChannelRuntime&, int note, float velocity, int sampleOffset, juce::int64 lengthSamples,
                           const NoteEvent* extras = nullptr);
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
        std::atomic<bool> liveMode { false };
        struct LiveSlot { int uid = 0; double start = 0.0; bool starting = false, stopping = false; };
        std::array<LiveSlot, numLiveSlots> liveSlots;              // solo el hilo de audio
        std::array<std::atomic<int>, 32> liveCommands {};          // la interfaz pide encender/apagar (uid)
        std::array<std::atomic<int>, numLiveSlots> liveShowUid {}, liveShowState {};   // para que la interfaz lo vea
        double liveClock = 0.0;
        void advanceLive (const Snapshot&, int n, double samplesPerStep, bool isPlaying);

        juce::CriticalSection lock;
        std::shared_ptr<const Snapshot> snapshot;
        std::vector<std::unique_ptr<ChannelRuntime>> runtimes;
        std::array<MixerTrack, numMixerTracks> mixer;
        MetronomeInstrument metronome;

        std::map<int, std::unique_ptr<juce::AudioPluginInstance>> channelPlugins;
        std::array<std::unique_ptr<juce::AudioPluginInstance>, numMixerTracks * numFxSlots> mixerPlugins;   // [pista * huecos + hueco]

        double sampleRate = 44100.0;
        int blockSize = 512;
        int numInputChannels = 0;
        juce::int64 sampleClock = 0;

        std::atomic<bool> playing { false }, songModeFlag { false };
        std::atomic<double> position { 0.0 };
        std::atomic<double> requestedPosition { -1.0 };
        std::atomic<double> currentBpm { 140.0 };
        std::atomic<int> currentStepsPerBar { 16 };
        std::atomic<int> liveChannelUid { 0 };
        std::atomic<bool> midiRecording { false };
        std::atomic<int> stemTrack { -1 };
        // Compensación del retraso de los plugins: una línea de retardo por pista (solo si hace falta)
        static constexpr int pdcSize = 16384;
        std::array<juce::AudioBuffer<float>, numMixerTracks> pdcLines;
        std::array<int, numMixerTracks> pdcWrite {};
        void ensurePdc();
        std::array<float, scopeSize> scope {};
        std::atomic<int> scopePos { 0 };
        static constexpr int recordFifoSize = 1024;
        juce::AbstractFifo recordFifo { recordFifoSize };
        std::array<RecordedMidi, recordFifoSize> recordBuffer {};
        std::array<std::atomic<float>, 256> channelActivity {};
        std::array<float, numMixerTracks> peakEnv {};        // controlador de picos: nivel de cada pista
        std::array<float, numMixerTracks> sidechainEnv {};   // seguidor del sidechain de cada pista

        SharedBuffer audition;            // sonido de los juegos de oído
        double auditionPos = 0.0, auditionStep = 1.0;
        float auditionGain = 0.8f;

        juce::AbstractFifo previewFifo { 64 };
        std::array<PreviewEvent, 64> previewEvents;

        juce::MidiMessageCollector midiCollector;
        juce::MidiBuffer liveMidi;

        // Grabación
        juce::TimeSliceThread writerThread { "Studoo grabacion" };
        std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> recordWriter;
        juce::File recordFile;
        std::atomic<bool> recording { false };
        double recordStartSteps = -1.0;
        juce::int64 recordedSamples = 0;
        juce::AudioBuffer<float> recordScratch;
    };
}
