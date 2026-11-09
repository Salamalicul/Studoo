#pragma once
// ============================================================================
//  AudioEngine: el "corazón" que produce el sonido.
//
//  La tarjeta de sonido le pide constantemente pequeños trozos de audio
//  (por ejemplo 512 muestras, unos 10 milisegundos). El motor cuenta el tiempo
//  muestra a muestra: cuando toca un nuevo paso del secuenciador, dispara los
//  sonidos que estén activados en ese paso.
//
//  La interfaz (botones, cuadrícula...) y el audio funcionan en "hilos"
//  distintos, como dos personas trabajando a la vez. Para que no se pisen,
//  comparten la información mediante variables "atómicas" (std::atomic),
//  que se pueden leer y escribir de forma segura desde los dos lados.
// ============================================================================
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>

#include "DrumSounds.h"

namespace studoo
{
    constexpr int numSteps    = 16; // 16 pasos = 1 compás de 4 tiempos
    constexpr int numChannels = 4;  // Bombo, caja, hi-hat, palmada

    class AudioEngine : public juce::AudioSource
    {
    public:
        AudioEngine();

        // --- Llamadas que hace la tarjeta de sonido -------------------------
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void releaseResources() override {}
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& info) override;

        // --- Transporte (reproducir / parar) --------------------------------
        void play();
        void stop();
        bool isPlaying() const              { return playing.load(); }

        void   setBpm (double newBpm);
        double getBpm() const               { return bpm.load(); }

        void setMetronomeEnabled (bool on)  { metronomeOn.store (on); }
        bool isMetronomeEnabled() const     { return metronomeOn.load(); }

        void  setMasterVolume (float gain)  { masterGain.store (gain); }
        float getMasterVolume() const       { return masterGain.load(); }

        // Paso que está sonando ahora (0..15), o -1 si está parado
        int getCurrentStep() const          { return currentStep.load(); }

        // --- Patrón ----------------------------------------------------------
        void setStep (int channel, int step, bool on);
        bool getStep (int channel, int step) const;
        void clearPattern();
        void loadExamplePattern();

        // --- Canales ---------------------------------------------------------
        static juce::String getChannelName (int channel);
        void  setChannelVolume (int channel, float gain);
        float getChannelVolume (int channel) const;
        void  setChannelMuted (int channel, bool muted);
        bool  isChannelMuted (int channel) const;

        // Hace sonar un canal una vez (al hacer clic en su nombre)
        void previewChannel (int channel);

    private:
        struct Voice
        {
            const juce::AudioBuffer<float>* sound = nullptr;
            int position = 0;
            float gain = 0.0f;
        };

        void triggerStep (int step);
        static void startVoice (Voice& v, const juce::AudioBuffer<float>& sound, float gain);

        // Datos compartidos entre la interfaz y el audio
        std::array<std::array<std::atomic<bool>, numSteps>, numChannels> pattern;
        std::array<std::atomic<float>, numChannels> channelVolume;
        std::array<std::atomic<bool>,  numChannels> channelMuted;
        std::array<std::atomic<bool>,  numChannels> previewRequested;

        std::atomic<bool>   playing { false };
        std::atomic<bool>   restartRequested { false };
        std::atomic<double> bpm { 140.0 };
        std::atomic<bool>   metronomeOn { false };
        std::atomic<float>  masterGain { 0.8f };
        std::atomic<int>    currentStep { -1 };

        // Datos que solo usa el hilo de audio
        double sampleRate = 44100.0;
        double samplesUntilNextStep = 0.0;
        int    nextStep = 0;

        std::array<juce::AudioBuffer<float>, numChannels> channelSounds;
        juce::AudioBuffer<float> clickHigh, clickLow;

        std::array<Voice, numChannels> voices;
        Voice metronomeVoice;
    };
}
