#include "SampleCache.h"
#include "DrumSounds.h"
#include "Kits.h"

namespace studoo
{
SampleCache::SampleCache()
{
    formats.registerBasicFormats(); // WAV, AIFF, FLAC, OGG y MP3
}

bool SampleCache::isAudioFile (const juce::File& f)
{
    return f.hasFileExtension ("wav;mp3;aif;aiff;flac;ogg");
}

SamplePtr SampleCache::get (const juce::String& path)
{
    if (path.isEmpty())
        return nullptr;

    if (auto it = cache.find (path); it != cache.end())
        return it->second;

    auto data = load (path);
    if (data != nullptr)
        cache[path] = data;
    return data;
}

void SampleCache::purgeUnused()
{
    for (auto it = cache.begin(); it != cache.end();)
    {
        if (it->second.use_count() == 1)
            it = cache.erase (it);
        else
            ++it;
    }
}

SamplePtr SampleCache::load (const juce::String& path)
{
    auto data = std::make_shared<SampleData>();

    if (path.startsWith ("builtin:"))
    {
        const auto which = path.fromFirstOccurrenceOf ("builtin:", false, false);

        // Sonido de un kit incluido: "builtin:trap/kick"
        if (which.containsChar ('/'))
        {
            data->sampleRate = 44100.0;
            data->buffer = kits::render (which.upToFirstOccurrenceOf ("/", false, false),
                                         which.fromFirstOccurrenceOf ("/", false, false), data->sampleRate);
            data->name = which;
            return data->buffer.getNumSamples() > 0 ? data : nullptr;
        }

        DrumSound sound = DrumSound::kick;
        if (which == "snare")      sound = DrumSound::snare;
        else if (which == "hihat") sound = DrumSound::hihat;
        else if (which == "clap")  sound = DrumSound::clap;
        else if (which != "kick")  return nullptr;

        data->sampleRate = 44100.0;
        data->buffer = renderDrumSound (sound, data->sampleRate);
        data->name = which;
        return data;
    }

    juce::File file (path);
    if (! file.existsAsFile())
        return nullptr;

    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
    if (reader == nullptr || reader->lengthInSamples <= 0)
        return nullptr;

    // Máximo 10 minutos para no llenar la memoria
    const auto maxSamples = (juce::int64) (reader->sampleRate * 600.0);
    const int numSamples = (int) juce::jmin (reader->lengthInSamples, maxSamples);
    const int numChannels = (int) juce::jlimit (1u, 2u, reader->numChannels);

    data->buffer.setSize (numChannels, numSamples);
    reader->read (&data->buffer, 0, numSamples, 0, true, numChannels > 1);
    data->sampleRate = reader->sampleRate > 0 ? reader->sampleRate : 44100.0;
    data->name = file.getFileNameWithoutExtension();
    return data;
}
}
