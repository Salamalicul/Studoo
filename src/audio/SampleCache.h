#pragma once
// ============================================================================
//  SampleCache: carga archivos de audio (WAV, MP3, AIFF, FLAC, OGG) y los
//  guarda en memoria. Si dos canales usan el mismo sample, solo se carga una vez.
//  Los sonidos incluidos se llaman "builtin:kick", "builtin:snare", etc.
// ============================================================================
#include <juce_audio_formats/juce_audio_formats.h>
#include <map>
#include <memory>

namespace studoo
{
    struct SampleData
    {
        juce::AudioBuffer<float> buffer; // 1 o 2 canales
        double sampleRate = 44100.0;
        juce::String name;
    };

    using SamplePtr = std::shared_ptr<const SampleData>;

    class SampleCache
    {
    public:
        SampleCache();

        // Devuelve el sample (o nullptr si no se puede leer). Solo desde el hilo principal.
        SamplePtr get (const juce::String& path);

        juce::AudioFormatManager& getFormatManager() { return formats; }
        juce::String getSupportedWildcard() const    { return "*.wav;*.mp3;*.aif;*.aiff;*.flac;*.ogg"; }
        static bool isAudioFile (const juce::File&);

        // Libera los samples que ya nadie usa
        void purgeUnused();

    private:
        SamplePtr load (const juce::String& path);

        juce::AudioFormatManager formats;
        std::map<juce::String, SamplePtr> cache;
    };
}
