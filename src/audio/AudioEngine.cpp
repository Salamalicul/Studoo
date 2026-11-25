#include "AudioEngine.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace studoo
{
AudioEngine::AudioEngine()
{
    writerThread.startThread();
    liveMidi.ensureSize (4096);
}

AudioEngine::~AudioEngine()
{
    stopRecording();
    writerThread.stopThread (2000);

    const juce::ScopedLock sl (lock);
    runtimes.clear();
    for (auto& t : mixer)
        for (int slot = 0; slot < numFxSlots; ++slot)
            t.setPlugin (nullptr, slot);
    channelPlugins.clear();
    for (auto& p : mixerPlugins)
        p.reset();
}

// ============================================================================
//  Tarjeta de sonido
// ============================================================================
void AudioEngine::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    numInputChannels = device->getActiveInputChannels().countNumberOfSetBits();
    prepare (device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
}

void AudioEngine::audioDeviceStopped() {}

void AudioEngine::audioDeviceIOCallbackWithContext (const float* const* inputs, int numInputs,
                                                    float* const* outputs, int numOutputs, int numSamples,
                                                    const juce::AudioIODeviceCallbackContext&)
{
    render (inputs, numInputs, outputs, numOutputs, numSamples);
}

void AudioEngine::prepare (double newSampleRate, int maxBlockSize)
{
    const juce::ScopedLock sl (lock);

    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    blockSize = juce::jlimit (64, 4096, maxBlockSize > 0 ? maxBlockSize : 512);

    for (auto& rt : runtimes)
    {
        rt->buffer.setSize (2, blockSize);
        if (rt->instrument != nullptr)
            rt->instrument->prepare (sampleRate, blockSize);
    }

    auto preparePlugin = [this] (juce::AudioPluginInstance* p)
    {
        p->setPlayHead (this);
        p->setRateAndBufferSizeDetails (sampleRate, blockSize);
        p->prepareToPlay (sampleRate, blockSize);
    };
    for (auto& [uid, p] : channelPlugins)
        preparePlugin (p.get());
    for (auto& p : mixerPlugins)
        if (p != nullptr)
            preparePlugin (p.get());
    for (auto& rt : runtimes)
        if (rt->instrument != nullptr)
            rt->instrument->prepare (sampleRate, blockSize); // tras preparar los plugins

    for (auto& t : mixer)
        t.prepare (sampleRate, blockSize);

    metronome.prepare (sampleRate);
    midiCollector.reset (sampleRate);
    recordScratch.setSize (2, blockSize);
}

// ============================================================================
//  Proyecto -> foto para el hilo de audio
// ============================================================================
std::unique_ptr<AudioEngine::ChannelRuntime> AudioEngine::createRuntime (const ChannelSnap& ch)
{
    auto rt = std::make_unique<ChannelRuntime>();
    rt->uid = ch.uid;
    rt->type = ch.type;
    rt->noteOffAt.fill (-1);
    rt->midi.ensureSize (4096);
    rt->buffer.setSize (2, blockSize);

    switch (ch.type)
    {
        case ChannelType::sampler: rt->instrument = std::make_unique<SamplerInstrument>(); break;
        case ChannelType::synth:   rt->instrument = std::make_unique<SynthInstrument>();   break;
        case ChannelType::bass808: rt->instrument = std::make_unique<Bass808Instrument>(); break;
        case ChannelType::plugin:
        {
            auto pi = std::make_unique<PluginInstrument>();
            if (auto it = channelPlugins.find (ch.uid); it != channelPlugins.end())
                pi->setPlugin (it->second.get());
            rt->instrument = std::move (pi);
            break;
        }
        case ChannelType::audio: break;
    }

    if (rt->instrument != nullptr)
    {
        rt->instrument->prepare (sampleRate, blockSize);
        rt->instrument->setParameters (ch);
    }
    return rt;
}

void AudioEngine::updateFromProject (const juce::ValueTree& state, SampleCache& cache)
{
    std::shared_ptr<const Snapshot> newSnap = Snapshot::build (state);

    // Cargamos los samples ANTES de bloquear (leer un archivo puede tardar)
    std::vector<SamplePtr> samples (newSnap->channels.size());
    for (size_t i = 0; i < newSnap->channels.size(); ++i)
    {
        const auto& ch = newSnap->channels[i];
        if (ch.type == ChannelType::sampler || ch.type == ChannelType::audio)
            samples[i] = cache.get (ch.samplePath);
    }

    // Cosas que se liberan después de soltar el candado
    std::vector<std::unique_ptr<ChannelRuntime>> oldRuntimes;
    std::vector<SamplePtr> oldSamples;
    std::vector<std::unique_ptr<juce::AudioPluginInstance>> oldPlugins;

    {
        const juce::ScopedLock sl (lock);

        std::vector<std::unique_ptr<ChannelRuntime>> newRuntimes;
        newRuntimes.reserve (newSnap->channels.size());

        for (size_t i = 0; i < newSnap->channels.size(); ++i)
        {
            const auto& ch = newSnap->channels[i];
            std::unique_ptr<ChannelRuntime> rt;

            // Reutilizamos el instrumento si ya existía (así no se corta el sonido)
            for (auto& old : runtimes)
                if (old != nullptr && old->uid == ch.uid && old->type == ch.type)
                {
                    rt = std::move (old);
                    break;
                }

            if (rt == nullptr)
                rt = createRuntime (ch);

            if (auto* sampler = dynamic_cast<SamplerInstrument*> (rt->instrument.get()))
            {
                if (sampler->getSamplePath() != ch.samplePath)
                {
                    oldSamples.push_back (sampler->setSample (samples[i]));
                    sampler->setSamplePath (ch.samplePath);
                }
            }
            else if (ch.type == ChannelType::audio && rt->audioSamplePath != ch.samplePath)
            {
                oldSamples.push_back (std::exchange (rt->audioSample, samples[i]));
                rt->audioSamplePath = ch.samplePath;
            }

            if (rt->instrument != nullptr)
                rt->instrument->setParameters (ch);

            newRuntimes.push_back (std::move (rt));
        }

        for (auto& old : runtimes)
            if (old != nullptr)
                oldRuntimes.push_back (std::move (old));
        runtimes.swap (newRuntimes);

        // Plugins de canales que ya no existen
        for (auto it = channelPlugins.begin(); it != channelPlugins.end();)
        {
            const bool used = std::any_of (newSnap->channels.begin(), newSnap->channels.end(), [&] (const ChannelSnap& c)
                                           { return c.uid == it->first && c.type == ChannelType::plugin; });
            if (! used)
            {
                oldPlugins.push_back (std::move (it->second));
                it = channelPlugins.erase (it);
            }
            else
            {
                ++it;
            }
        }

        // Al cambiar entre modo patrón y modo canción, volvemos al principio
        if (snapshot != nullptr && newSnap->songMode != snapshot->songMode)
        {
            position.store (0.0);
            requestedPosition.store (0.0);
        }

        currentBpm.store (newSnap->bpm);
        songModeFlag.store (newSnap->songMode);
        std::swap (snapshot, newSnap);
    }

    for (auto& p : oldPlugins)
        if (onPluginWillBeDeleted)
            onPluginWillBeDeleted (p.get());
    // Al salir de esta función se liberan oldRuntimes, oldSamples, oldPlugins y la foto antigua
}

// ============================================================================
//  Transporte
// ============================================================================
void AudioEngine::play()
{
    playing.store (true);
}

void AudioEngine::stop()
{
    playing.store (false);
    setPlayPosition (0.0);
}

void AudioEngine::setPlayPosition (double steps)
{
    steps = juce::jmax (0.0, steps);
    position.store (steps);
    requestedPosition.store (steps);
}

void AudioEngine::previewNote (int channelUid, int note, float velocity, double seconds)
{
    const auto scope = previewFifo.write (1);
    if (scope.blockSize1 > 0)
        previewEvents[(size_t) scope.startIndex1] = { channelUid, note, velocity, (int) (seconds * sampleRate) };
}

void AudioEngine::playBuffer (SharedBuffer buffer, double bufferSampleRate, float gain)
{
    SharedBuffer old;
    {
        const juce::ScopedLock sl (lock);
        old = std::move (audition);
        audition = std::move (buffer);
        auditionPos = 0.0;
        auditionStep = juce::jlimit (0.125, 8.0, bufferSampleRate / sampleRate);
        auditionGain = juce::jlimit (0.0f, 1.0f, gain);
    }
    // 'old' se libera aquí, fuera del candado (no en el hilo de audio)
}

float AudioEngine::getTrackPeak (int track, int channel) const
{
    return juce::isPositiveAndBelow (track, numMixerTracks) ? mixer[(size_t) track].getPeak (channel) : 0.0f;
}

void AudioEngine::decayMeters()
{
    for (auto& t : mixer)
        t.decayPeaks();
    for (auto& a : channelActivity)
        a.store (a.load() * 0.8f);
}

// ============================================================================
//  Plugins
// ============================================================================
void AudioEngine::setChannelPlugin (int channelUid, std::unique_ptr<juce::AudioPluginInstance> plugin)
{
    if (plugin != nullptr)
    {
        plugin->setPlayHead (this);
        plugin->setRateAndBufferSizeDetails (sampleRate, blockSize);
        plugin->prepareToPlay (sampleRate, blockSize);
    }

    std::unique_ptr<juce::AudioPluginInstance> old;
    {
        const juce::ScopedLock sl (lock);
        auto& slot = channelPlugins[channelUid];
        old = std::move (slot);
        slot = std::move (plugin);

        for (auto& rt : runtimes)
            if (rt->uid == channelUid)
                if (auto* pi = dynamic_cast<PluginInstrument*> (rt->instrument.get()))
                {
                    pi->setPlugin (slot.get());
                    pi->prepare (sampleRate, blockSize);
                }
    }

    if (old != nullptr && onPluginWillBeDeleted)
        onPluginWillBeDeleted (old.get());
}

juce::AudioPluginInstance* AudioEngine::getChannelPlugin (int channelUid)
{
    const juce::ScopedLock sl (lock);
    auto it = channelPlugins.find (channelUid);
    return it != channelPlugins.end() ? it->second.get() : nullptr;
}

void AudioEngine::setMixerPlugin (int track, std::unique_ptr<juce::AudioPluginInstance> plugin, int slot)
{
    if (! juce::isPositiveAndBelow (track, numMixerTracks) || ! juce::isPositiveAndBelow (slot, numFxSlots))
        return;
    const auto index = (size_t) (track * numFxSlots + slot);

    if (plugin != nullptr)
    {
        plugin->setPlayHead (this);
        plugin->setRateAndBufferSizeDetails (sampleRate, blockSize);
        plugin->prepareToPlay (sampleRate, blockSize);
    }

    std::unique_ptr<juce::AudioPluginInstance> old;
    {
        const juce::ScopedLock sl (lock);
        old = std::move (mixerPlugins[index]);
        mixerPlugins[index] = std::move (plugin);
        mixer[(size_t) track].setPlugin (mixerPlugins[index].get(), slot);
    }

    if (old != nullptr && onPluginWillBeDeleted)
        onPluginWillBeDeleted (old.get());
}

void AudioEngine::clearAllPlugins()
{
    std::vector<std::unique_ptr<juce::AudioPluginInstance>> old;
    {
        const juce::ScopedLock sl (lock);
        for (auto& [uid, p] : channelPlugins)
            old.push_back (std::move (p));
        channelPlugins.clear();
        for (auto& rt : runtimes)
            if (auto* pi = dynamic_cast<PluginInstrument*> (rt->instrument.get()))
                pi->setPlugin (nullptr);
        for (int t = 0; t < numMixerTracks; ++t)
            for (int slot = 0; slot < numFxSlots; ++slot)
            {
                mixer[(size_t) t].setPlugin (nullptr, slot);
                auto& p = mixerPlugins[(size_t) (t * numFxSlots + slot)];
                if (p != nullptr)
                    old.push_back (std::move (p));
            }
    }
    for (auto& p : old)
        if (onPluginWillBeDeleted)
            onPluginWillBeDeleted (p.get());
}

juce::AudioPluginInstance* AudioEngine::getMixerPlugin (int track, int slot)
{
    return juce::isPositiveAndBelow (track, numMixerTracks) && juce::isPositiveAndBelow (slot, numFxSlots)
               ? mixerPlugins[(size_t) (track * numFxSlots + slot)].get() : nullptr;
}

juce::Optional<juce::AudioPlayHead::PositionInfo> AudioEngine::getPosition() const
{
    PositionInfo info;
    const double bpm = currentBpm.load();
    const double steps = position.load();
    info.setBpm (bpm);
    info.setTimeSignature (juce::AudioPlayHead::TimeSignature { 4, 4 });
    info.setIsPlaying (playing.load());
    info.setPpqPosition (steps / stepsPerBeat);
    info.setPpqPositionOfLastBarStart (std::floor (steps / stepsPerBar) * 4.0);
    info.setTimeInSeconds (steps * 60.0 / bpm / stepsPerBeat);
    info.setTimeInSamples ((juce::int64) (steps * 60.0 / bpm / stepsPerBeat * sampleRate));
    return info;
}

// ============================================================================
//  Grabación
// ============================================================================
bool AudioEngine::startRecording (const juce::File& file, juce::String& error)
{
    if (numInputChannels <= 0)
    {
        error = juce::String::fromUTF8 ("No hay ninguna entrada de audio activa. Pulsa \"Audio\" y elige tu micrófono "
                                        "en \"Canales de entrada\".");
        return false;
    }

    file.getParentDirectory().createDirectory();
    file.deleteFile();
    std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());
    if (stream == nullptr)
    {
        error = juce::String::fromUTF8 ("No se puede crear el archivo de grabación.");
        return false;
    }

    juce::WavAudioFormat wav;
    auto writer = wav.createWriterFor (stream, juce::AudioFormatWriterOptions{}.withSampleRate (sampleRate)
                                                                               .withNumChannels (2)
                                                                               .withBitsPerSample (24));
    if (writer == nullptr)
    {
        error = juce::String::fromUTF8 ("No se puede crear el archivo WAV.");
        return false;
    }

    auto threaded = std::make_unique<juce::AudioFormatWriter::ThreadedWriter> (writer.release(), writerThread, 65536);

    const juce::ScopedLock sl (lock);
    recordWriter = std::move (threaded);
    recordFile = file;
    recordStartSteps = -1.0;
    recordedSamples = 0;
    recording.store (true);
    return true;
}

AudioEngine::RecordingResult AudioEngine::stopRecording()
{
    RecordingResult result;
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> writer;
    {
        const juce::ScopedLock sl (lock);
        writer = std::move (recordWriter);
        recording.store (false);
        result.file = recordFile;
        result.startSteps = juce::jmax (0.0, recordStartSteps);
        const double samplesPerStep = sampleRate * 60.0 / currentBpm.load() / stepsPerBeat;
        result.lengthSteps = (double) recordedSamples / samplesPerStep;
        result.valid = writer != nullptr && recordedSamples > 0;
    }
    writer.reset(); // termina de escribir el archivo
    return result;
}

// ============================================================================
//  Exportación
// ============================================================================
bool AudioEngine::renderSongToFile (const juce::File& wavFile, double exportRate,
                                    std::function<bool (float)> keepGoing, juce::String& error, double maxSeconds)
{
    const double oldRate = sampleRate;
    const int oldBlock = blockSize;
    constexpr int chunk = 512;

    prepare (exportRate, chunk);

    double songLength = 16.0, bpm = 140.0;
    {
        const juce::ScopedLock sl (lock);
        if (snapshot != nullptr)
        {
            songLength = snapshot->songLength;
            bpm = snapshot->bpm;
        }
        for (auto& [uid, p] : channelPlugins) p->setNonRealtime (true);
        for (auto& p : mixerPlugins)          if (p != nullptr) p->setNonRealtime (true);
    }

    wavFile.deleteFile();
    std::unique_ptr<juce::OutputStream> stream (wavFile.createOutputStream());
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    if (stream != nullptr)
        writer = wav.createWriterFor (stream, juce::AudioFormatWriterOptions{}.withSampleRate (exportRate)
                                                                              .withNumChannels (2)
                                                                              .withBitsPerSample (24));

    bool ok = writer != nullptr;
    if (! ok)
        error = juce::String::fromUTF8 ("No se puede crear el archivo. ¿Tienes permiso para escribir en esa carpeta?");

    if (ok)
    {
        exportingSong.store (true);
        position.store (0.0);
        requestedPosition.store (0.0);
        playing.store (true);

        const double samplesPerStep = exportRate * 60.0 / bpm / stepsPerBeat;
        auto songSamples = (juce::int64) (songLength * samplesPerStep);
        if (maxSeconds > 0.0)
            songSamples = juce::jmin (songSamples, (juce::int64) (maxSeconds * exportRate));
        const auto totalSamples = songSamples + (juce::int64) (exportRate * 2.0); // 2 s de cola (reverb, delay...)

        juce::AudioBuffer<float> out (2, chunk);
        for (juce::int64 done = 0; done < totalSamples; done += chunk)
        {
            if (done >= songSamples && playing.load())
                playing.store (false); // la cola suena sin volver a empezar

            render (nullptr, 0, out.getArrayOfWritePointers(), 2, chunk);
            const int n = (int) juce::jmin ((juce::int64) chunk, totalSamples - done);
            writer->writeFromAudioSampleBuffer (out, 0, n);

            if (keepGoing != nullptr && ! keepGoing ((float) done / (float) totalSamples))
            {
                ok = false;
                error = juce::String::fromUTF8 ("Exportación cancelada.");
                break;
            }
        }

        playing.store (false);
        exportingSong.store (false);
        setPlayPosition (0.0);
    }

    writer.reset();
    if (! ok)
        wavFile.deleteFile();

    {
        const juce::ScopedLock sl (lock);
        for (auto& [uid, p] : channelPlugins) p->setNonRealtime (false);
        for (auto& p : mixerPlugins)          if (p != nullptr) p->setNonRealtime (false);
    }
    prepare (oldRate, oldBlock);
    return ok;
}

// ============================================================================
//  Audio
// ============================================================================
void AudioEngine::render (const float* const* inputs, int numInputs, float* const* outputs, int numOutputs, int numSamples)
{
    for (int ch = 0; ch < numOutputs; ++ch)
        if (outputs[ch] != nullptr)
            juce::FloatVectorOperations::clear (outputs[ch], numSamples);

    for (int done = 0; done < numSamples;)
    {
        const int n = juce::jmin (blockSize, numSamples - done);
        renderChunk (inputs, numInputs, outputs, numOutputs, done, n);
        done += n;
    }
}

void AudioEngine::releaseAllNotes()
{
    for (auto& rt : runtimes)
    {
        for (int note = 0; note < 128; ++note)
            if (rt->noteOffAt[(size_t) note] >= 0)
            {
                rt->midi.addEvent (juce::MidiMessage::noteOff (1, note), 0);
                rt->noteOffAt[(size_t) note] = -1;
            }
        if (rt->instrument != nullptr)
            rt->instrument->allNotesOff();
    }
}

void AudioEngine::scheduleNote (ChannelRuntime& rt, int note, float velocity, int sampleOffset, juce::int64 lengthSamples)
{
    note = juce::jlimit (0, 127, note);
    sampleOffset = juce::jlimit (0, juce::jmax (0, currentChunkSize - 1), sampleOffset);

    if (rt.noteOffAt[(size_t) note] >= 0)
        rt.midi.addEvent (juce::MidiMessage::noteOff (1, note), sampleOffset);

    rt.midi.addEvent (juce::MidiMessage::noteOn (1, note, juce::jlimit (0.01f, 1.0f, velocity)), sampleOffset);
    rt.noteOffAt[(size_t) note] = sampleClock + sampleOffset + juce::jmax ((juce::int64) 1, lengthSamples);
}

void AudioEngine::collectPatternEvents (const Snapshot& snap, const PatternSnap& pattern,
                                        double localFrom, double localTo, double localEnd,
                                        double globalShift, double segmentStart,
                                        int sampleOffset, double samplesPerStep)
{
    const double len = juce::jmax (1.0, pattern.length);
    const int firstRep = (int) std::floor (localFrom / len);
    const int lastRep = (int) std::floor ((localTo - 1.0e-9) / len);

    for (size_t ci = 0; ci < pattern.notesPerChannel.size() && ci < runtimes.size(); ++ci)
    {
        if (snap.channels[ci].muted)
            continue;

        auto& rt = *runtimes[ci];
        for (int rep = firstRep; rep <= lastRep; ++rep)
        {
            const double base = rep * len;
            for (const auto& note : pattern.notesPerChannel[ci])
            {
                // Swing: las semicorcheas "pares" (2, 4, 6...) se retrasan un poco
                double start = note.start;
                if (snap.swing > 0.001f)
                {
                    const double inPair = std::fmod (start, 2.0);
                    if (std::abs (inPair - 1.0) < 0.01)
                        start += snap.swing * 0.5;
                }
                const double t = base + start;
                if (t < localFrom || t >= localTo || t >= localEnd)
                    continue;

                const double noteLength = juce::jmin (note.length, localEnd - t);
                const int offset = sampleOffset + (int) ((t + globalShift - segmentStart) * samplesPerStep);
                scheduleNote (rt, note.pitch, note.velocity, offset, (juce::int64) (noteLength * samplesPerStep));
            }
        }
    }
}

void AudioEngine::collectEvents (const Snapshot& snap, double from, double to, int sampleOffset, double samplesPerStep)
{
    // Metrónomo: un clic en cada tiempo (cada 4 pasos)
    if (snap.metronome && ! exportingSong.load())
    {
        for (double beat = std::ceil (from / stepsPerBeat) * stepsPerBeat; beat < to; beat += stepsPerBeat)
        {
            const bool accent = std::fmod (beat, (double) stepsPerBar) < 0.5;
            metronome.trigger (accent, sampleOffset + (int) ((beat - from) * samplesPerStep));
        }
    }

    const bool song = snap.songMode || exportingSong.load();

    if (! song)
    {
        if (juce::isPositiveAndBelow (snap.currentPatternIndex, (int) snap.patterns.size()))
        {
            const auto& p = snap.patterns[(size_t) snap.currentPatternIndex];
            collectPatternEvents (snap, p, from, to, p.length, 0.0, from, sampleOffset, samplesPerStep);
        }
        return;
    }

    for (const auto& clip : snap.clips)
    {
        if (clip.isAudio || ! juce::isPositiveAndBelow (clip.patternIndex, (int) snap.patterns.size()))
            continue;

        const double a = juce::jmax (from, clip.start);
        const double b = juce::jmin (to, clip.start + clip.length);
        if (a >= b)
            continue;

        collectPatternEvents (snap, snap.patterns[(size_t) clip.patternIndex], a - clip.start, b - clip.start,
                              clip.length, clip.start, from, sampleOffset, samplesPerStep);
    }
}

void AudioEngine::renderAudioClips (const Snapshot& snap, double from, double to, int sampleOffset, double samplesPerStep)
{
    const double secondsPerStep = 60.0 / snap.bpm / stepsPerBeat;

    for (const auto& clip : snap.clips)
    {
        if (! clip.isAudio || ! juce::isPositiveAndBelow (clip.channelIndex, (int) runtimes.size()))
            continue;

        auto& rt = *runtimes[(size_t) clip.channelIndex];
        if (rt.audioSample == nullptr || snap.channels[(size_t) clip.channelIndex].muted)
            continue;

        const double a = juce::jmax (from, clip.start);
        const double b = juce::jmin (to, clip.start + clip.length);
        if (a >= b)
            continue;

        const auto& src = rt.audioSample->buffer;
        const int srcLength = src.getNumSamples();
        const float* srcL = src.getReadPointer (0);
        const float* srcR = src.getNumChannels() > 1 ? src.getReadPointer (1) : srcL;
        float* outL = rt.buffer.getWritePointer (0);
        float* outR = rt.buffer.getWritePointer (1);

        const int first = juce::jmax (0, sampleOffset + (int) ((a - from) * samplesPerStep));
        const int last = juce::jmin (currentChunkSize, sampleOffset + (int) ((b - from) * samplesPerStep));

        const double srcRate = rt.audioSample->sampleRate;
        const bool granular = ! juce::approximatelyEqual (clip.timeRatio, 1.0) || ! juce::approximatelyEqual (clip.pitchRatio, 1.0);

        // Lee el sample en una posición (en muestras), con interpolación
        auto readAt = [&] (double pos, float& l, float& r)
        {
            if (! (pos >= 0.0) || pos >= (double) (srcLength - 1)) { l = r = 0.0f; return false; }
            const int idx = (int) pos;
            const float fr = (float) (pos - idx);
            l = srcL[idx] + (srcL[idx + 1] - srcL[idx]) * fr;
            r = srcR[idx] + (srcR[idx + 1] - srcR[idx]) * fr;
            return true;
        };

        for (int i = first; i < last; ++i)
        {
            const double songTime = from + (i - sampleOffset) / samplesPerStep;
            const double u = ((songTime - clip.start) + clip.offset) * secondsPerStep;   // segundos desde el principio del audio
            float sl = 0.0f, sr = 0.0f;
            if (granular)
            {
                // Estirar sin cambiar el tono (y cambiar el tono sin cambiar la velocidad):
                // granos de 60 ms que se solapan a la mitad, con ventana Hann (suman 1).
                // Cada grano empieza en u·timeRatio y dentro de él se lee a pitchRatio.
                constexpr double grain = 0.06, hop = grain * 0.5;
                if (! (u >= 0.0) || u * clip.timeRatio >= (double) srcLength / srcRate) break;
                const auto k0 = (juce::int64) std::floor (u / hop);
                for (auto k = k0 - 1; k <= k0; ++k)
                {
                    const double local = u - (double) k * hop;
                    if (k < 0 || local < 0.0 || local >= grain)
                        continue;
                    const float w = 0.5f - 0.5f * (float) std::cos (juce::MathConstants<double>::twoPi * local / grain);
                    float gl, gr;
                    if (readAt (((double) k * hop * clip.timeRatio + local * clip.pitchRatio) * srcRate, gl, gr))
                    {
                        sl += gl * w;
                        sr += gr * w;
                    }
                }
            }
            else
            {
                const double srcPos = u * srcRate;
                if (! (srcPos >= 0.0)) continue;               // también descarta NaN
                if (srcPos >= (double) (srcLength - 1)) break;  // antes de convertir a int (sin desbordes)
                readAt (srcPos, sl, sr);
            }
            // Volumen del clip y fundidos (siempre un fundido mínimo para que los cortes no hagan "clic")
            const double inClip = songTime - clip.start;
            constexpr double minFade = 0.03;
            const double fin = juce::jmax (minFade, clip.fadeIn), fout = juce::jmax (minFade, clip.fadeOut);
            const float env = clip.gain * (float) juce::jmin (1.0, inClip / fin, (clip.length - inClip) / fout);
            if (env <= 0.0f) continue;
            outL[i] += sl * env;
            outR[i] += sr * env;
        }
    }
}

void AudioEngine::renderChunk (const float* const* inputs, int numInputs, float* const* outputs, int numOutputs,
                               int outputOffset, int n)
{
    const juce::ScopedLock sl (lock);
    currentChunkSize = n;

    const Snapshot* snap = snapshot.get();
    if (snap == nullptr)
        return;

    // --- Preparar -------------------------------------------------------------------
    for (auto& rt : runtimes)
    {
        rt->midi.clear();
        rt->buffer.clear (0, n);
    }
    for (auto& t : mixer)
        t.getBuffer().clear (0, n);

    const double requested = requestedPosition.exchange (-1.0);
    if (requested >= 0.0)
        releaseAllNotes();

    const bool isPlaying = playing.load();
    if (wasPlaying && ! isPlaying)
        releaseAllNotes();
    wasPlaying = isPlaying;

    // --- Notas de prueba (clic en un canal, en el piano roll...) --------------------------
    {
        const auto scope = previewFifo.read (previewFifo.getNumReady());
        auto handle = [&] (int index)
        {
            const auto& e = previewEvents[(size_t) index];
            for (auto& rt : runtimes)
                if (rt->uid == e.channelUid)
                    scheduleNote (*rt, e.note, e.velocity, 0, e.lengthSamples);
        };
        scope.forEach (handle);
    }

    // --- Teclado MIDI en directo ---------------------------------------------------------
    liveMidi.clear();
    midiCollector.removeNextBlockOfMessages (liveMidi, n);
    if (! liveMidi.isEmpty())
    {
        const int liveUid = liveChannelUid.load();
        for (auto& rt : runtimes)
            if (rt->uid == liveUid)
                rt->midi.addEvents (liveMidi, 0, n, 0);
    }

    // --- Secuenciador: qué notas suenan en este trocito ----------------------------------
    const double samplesPerStep = sampleRate * 60.0 / snap->bpm / stepsPerBeat;
    const bool song = snap->songMode || exportingSong.load();
    double chunkStartPos = position.load();

    // Grabar lo que se toca en el teclado MIDI (solo en modo patrón y sonando)
    if (isPlaying && ! song && midiRecording.load() && ! liveMidi.isEmpty())
    {
        double patternLength = 16.0;
        if (juce::isPositiveAndBelow (snap->currentPatternIndex, (int) snap->patterns.size()))
            patternLength = juce::jmax (1.0, snap->patterns[(size_t) snap->currentPatternIndex].length);
        for (const auto meta : liveMidi)
        {
            const auto m = meta.getMessage();
            if (! m.isNoteOnOrOff())
                continue;
            const double p = std::fmod (chunkStartPos + meta.samplePosition / samplesPerStep, patternLength);
            const RecordedMidi r { m.isNoteOn(), m.getNoteNumber(), m.getFloatVelocity(), p };
            const auto scope = recordFifo.write (1);   // si se llena (no debería), se pierde la nota
            if (scope.blockSize1 > 0)      recordBuffer[(size_t) scope.startIndex1] = r;
            else if (scope.blockSize2 > 0) recordBuffer[(size_t) scope.startIndex2] = r;
        }
    }

    if (isPlaying)
    {
        double loopEnd = song ? snap->songLength : 16.0;
        if (! song && juce::isPositiveAndBelow (snap->currentPatternIndex, (int) snap->patterns.size()))
            loopEnd = snap->patterns[(size_t) snap->currentPatternIndex].length;
        loopEnd = juce::jmax (1.0, loopEnd);

        double pos = chunkStartPos;
        if (pos >= loopEnd)
            pos = std::fmod (pos, loopEnd);
        chunkStartPos = pos;

        double remaining = n / samplesPerStep;
        double consumed = 0.0;
        while (remaining > 1.0e-9)
        {
            const double segEnd = juce::jmin (pos + remaining, loopEnd);
            const int offset = (int) (consumed * samplesPerStep);
            collectEvents (*snap, pos, segEnd, offset, samplesPerStep);
            if (song)
                renderAudioClips (*snap, pos, segEnd, offset, samplesPerStep);

            const double used = segEnd - pos;
            consumed += used;
            remaining -= used;
            pos = (segEnd >= loopEnd - 1.0e-9) ? 0.0 : segEnd;
        }
        position.store (pos);
    }

    // --- Notas que terminan en este trocito ----------------------------------------------
    for (auto& rt : runtimes)
        for (int note = 0; note < 128; ++note)
        {
            const auto off = rt->noteOffAt[(size_t) note];
            if (off >= 0 && off < sampleClock + n)
            {
                rt->midi.addEvent (juce::MidiMessage::noteOff (1, note), (int) juce::jmax ((juce::int64) 0, off - sampleClock));
                rt->noteOffAt[(size_t) note] = -1;
            }
        }

    // --- Automatización (solo en modo canción) -----------------------------------------------
    std::array<TrackParams, numMixerTracks> trackParams = snap->mixer;
    const bool automate = isPlaying && song && ! snap->lanes.empty();
    if (automate)
    {
        for (const auto& lane : snap->lanes)
        {
            if (lane.target.kind != AutomationTarget::Kind::mixer)
                continue;
            auto& tp = trackParams[(size_t) lane.target.index];
            const float v = AutomationTarget::fromNormalised (lane.target.param, lane.valueAt (chunkStartPos));
            switch (lane.target.param)
            {
                case AutomationTarget::Param::volume:    tp.volume = v; break;
                case AutomationTarget::Param::pan:       tp.pan = v; break;
                case AutomationTarget::Param::reverbMix: tp.reverbMix = v; break;
                case AutomationTarget::Param::delayMix:  tp.delayMix = v; break;
                case AutomationTarget::Param::eqLow:     tp.eqLow = v; break;
                case AutomationTarget::Param::eqHigh:    tp.eqHigh = v; break;
                case AutomationTarget::Param::eqMid:     tp.eqMid = v; break;
                case AutomationTarget::Param::trackFilter: tp.filter = v; break;
                case AutomationTarget::Param::distortion:  tp.distortion = v; break;
                case AutomationTarget::Param::chorusMix:   tp.chorusMix = v; break;
                case AutomationTarget::Param::sendLevel:   tp.sendLevel = v; break;
                default: break;
            }
        }
    }

    // --- Instrumentos -> pistas del mezclador -----------------------------------------------
    for (size_t i = 0; i < runtimes.size() && i < snap->channels.size(); ++i)
    {
        auto& rt = *runtimes[i];
        const auto& ch = snap->channels[i];
        float volume = ch.volume, pan = ch.pan;

        if (rt.instrument != nullptr)
        {
            bool instrumentAutomated = false;
            if (automate)
                for (const auto& lane : snap->lanes)
                    instrumentAutomated = instrumentAutomated
                        || (lane.channelIndex == (int) i && AutomationTarget::isInstrumentParam (lane.target.param));
            if (instrumentAutomated)
            {
                using P = AutomationTarget::Param;
                ChannelSnap tmp = ch;
                for (const auto& lane : snap->lanes)
                {
                    if (lane.channelIndex != (int) i)
                        continue;
                    const float v = AutomationTarget::fromNormalised (lane.target.param, lane.valueAt (chunkStartPos));
                    switch (lane.target.param)
                    {
                        case P::cutoff:     tmp.synth.cutoff = v; break;
                        case P::resonance:  tmp.synth.resonance = v; break;
                        case P::lfoRate:    tmp.synth.lfoRate = v; break;
                        case P::lfoAmount:  tmp.synth.lfoAmount = v; break;
                        case P::drive:      tmp.bass.drive = v; break;
                        case P::sampleTune: tmp.sampler.tune = v; break;
                        default: break;
                    }
                }
                rt.instrument->setParameters (tmp);
            }
            else
            {
                rt.instrument->setParameters (ch);
            }
            rt.instrument->render (rt.buffer, rt.midi, n);

            const float level = juce::jmax (rt.buffer.getMagnitude (0, 0, n),
                                            rt.buffer.getNumChannels() > 1 ? rt.buffer.getMagnitude (1, 0, n) : 0.0f);
            auto& activity = channelActivity[(size_t) (ch.uid & 255)];
            if (level > activity.load())
                activity.store (level);
        }

        if (automate)
            for (const auto& lane : snap->lanes)
                if (lane.channelIndex == (int) i)
                {
                    const float v = AutomationTarget::fromNormalised (lane.target.param, lane.valueAt (chunkStartPos));
                    if (lane.target.param == AutomationTarget::Param::volume)   volume = v;
                    else if (lane.target.param == AutomationTarget::Param::pan) pan = v;
                }

        if (ch.muted)
            continue;

        const float angle = (juce::jlimit (-1.0f, 1.0f, pan) + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        const float gl = volume * std::cos (angle) * juce::MathConstants<float>::sqrt2;
        const float gr = volume * std::sin (angle) * juce::MathConstants<float>::sqrt2;
        auto& dest = mixer[(size_t) ch.mixer].getBuffer();
        dest.addFrom (0, 0, rt.buffer, 0, 0, n, gl);
        dest.addFrom (1, 0, rt.buffer, 1, 0, n, gr);
    }

    // --- Sidechain: cada pista puede "agacharse" cuando suena otra (normalmente el bombo) -------
    for (int t = 1; t < numMixerTracks; ++t)
    {
        const auto& tp = trackParams[(size_t) t];
        if (tp.sidechain < 0.01f || tp.sidechainSource == t)
            continue;
        const auto& src = mixer[(size_t) tp.sidechainSource].getBuffer();
        auto& dst = mixer[(size_t) t].getBuffer();
        float& env = sidechainEnv[(size_t) t];
        const float release = (float) std::exp (-1.0 / (0.16 * sampleRate));   // vuelve a subir en ~160 ms
        const float* sl = src.getReadPointer (0);
        const float* sr_ = src.getReadPointer (1);
        float* dl = dst.getWritePointer (0);
        float* dr = dst.getWritePointer (1);
        for (int i = 0; i < n; ++i)
        {
            const float level = juce::jmin (1.0f, juce::jmax (std::abs (sl[i]), std::abs (sr_[i])) * 2.0f);
            env = level > env ? level : env * release;
            const float gain = 1.0f - tp.sidechain * 0.9f * env;
            dl[i] *= gain;
            dr[i] *= gain;
        }
    }

    // --- Mezclador -----------------------------------------------------------------------------
    // Solo: una pista que recibe el envío de una pista en solo también se oye
    std::array<bool, numMixerTracks> soloed {};
    bool anySolo = false;
    const int stem = stemTrack.load();
    if (stem > 0 && stem < numMixerTracks)
    {
        anySolo = true;   // exportar por pistas: solo esta pista
        soloed[(size_t) stem] = true;
    }
    else for (int t = 1; t < numMixerTracks; ++t)
    {
        const auto& tp = trackParams[(size_t) t];
        soloed[(size_t) t] = soloed[(size_t) t] || tp.solo;
        anySolo = anySolo || tp.solo;
        if (soloed[(size_t) t] && tp.sendTo > t && tp.sendLevel > 0.0f)
            soloed[(size_t) tp.sendTo] = true;
    }

    auto& master = mixer[0].getBuffer();
    for (int t = 1; t < numMixerTracks; ++t)
    {
        const auto& tp = trackParams[(size_t) t];
        const bool silenced = tp.mute || (anySolo && ! soloed[(size_t) t]);
        auto& out = mixer[(size_t) t].getBuffer();
        mixer[(size_t) t].process (n, tp, snap->bpm, silenced);

        // Envío: una copia (después del volumen) va a otra pista, que aún no se ha procesado
        if (tp.sendTo > t && tp.sendTo < numMixerTracks && tp.sendLevel > 0.0f)
        {
            auto& dest = mixer[(size_t) tp.sendTo].getBuffer();
            dest.addFrom (0, 0, out, 0, 0, n, tp.sendLevel);
            dest.addFrom (1, 0, out, 1, 0, n, tp.sendLevel);
        }
        if (tp.sendOnly && tp.sendTo > t && t != stem)
            continue;
        master.addFrom (0, 0, out, 0, 0, n);
        master.addFrom (1, 0, out, 1, 0, n);
    }
    if (! (stem > 0 && stem < numMixerTracks))
        mixer[0].process (n, trackParams[0], snap->bpm, trackParams[0].mute);
    metronome.render (master, n);

    // --- Sonido de los juegos de oído (no pasa por el mezclador) ---------------------------
    if (audition != nullptr)
    {
        const auto& src = *audition;
        const int len = src.getNumSamples();
        const float* srcL = src.getNumChannels() > 0 ? src.getReadPointer (0) : nullptr;
        const float* srcR = src.getNumChannels() > 1 ? src.getReadPointer (1) : srcL;
        float* l = master.getWritePointer (0);
        float* r = master.getWritePointer (1);
        int i = 0;
        for (; i < n && srcL != nullptr; ++i)
        {
            const int idx = (int) auditionPos;
            if (idx + 1 >= len)
                break;
            const float frac = (float) (auditionPos - idx);
            l[i] += (srcL[idx] + (srcL[idx + 1] - srcL[idx]) * frac) * auditionGain;
            r[i] += (srcR[idx] + (srcR[idx + 1] - srcR[idx]) * frac) * auditionGain;
            auditionPos += auditionStep;
        }
        if (i < n) // terminó: lo soltamos en el próximo playBuffer (no liberamos memoria aquí)
            auditionPos = (double) len;
    }

    // --- Salida a los altavoces -------------------------------------------------------------------
    // Protección: nunca se envía a los altavoces nada por encima del máximo ni
    // valores no válidos (NaN/infinito, por ejemplo de un plugin estropeado).
    // Si aparece uno, se reinician los efectos para que no se quede "pegado".
    bool invalid = false;
    for (int ch = 0; ch < numOutputs; ++ch)
    {
        if (outputs[ch] == nullptr)
            continue;
        const float* src = master.getReadPointer (ch % 2);
        float* dst = outputs[ch] + outputOffset;
        for (int i = 0; i < n; ++i)
        {
            const float v = src[i];
            if (! std::isfinite (v)) { dst[i] = 0.0f; invalid = true; }
            else                     dst[i] = juce::jlimit (-1.0f, 1.0f, v);
        }
    }
    if (invalid)
        for (auto& t : mixer)
            t.reset();   // sin reservar memoria (estamos en el hilo de audio)

    // --- Grabación ------------------------------------------------------------------------------------
    if (recording.load() && recordWriter != nullptr && isPlaying && song
        && inputs != nullptr && numInputs > 0 && inputs[0] != nullptr)
    {
        if (recordStartSteps < 0.0)
            recordStartSteps = chunkStartPos;

        const float* in[2] = { inputs[0] + outputOffset,
                               (numInputs > 1 && inputs[1] != nullptr ? inputs[1] : inputs[0]) + outputOffset };
        recordWriter->write (in, n);
        recordedSamples += n;
    }

    sampleClock += n;
}

std::vector<AudioEngine::RecordedMidi> AudioEngine::takeRecordedMidi()
{
    std::vector<RecordedMidi> out;
    const auto scope = recordFifo.read (recordFifo.getNumReady());
    scope.forEach ([&] (int i) { out.push_back (recordBuffer[(size_t) i]); });
    return out;
}
}
