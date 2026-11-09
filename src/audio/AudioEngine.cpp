#include "AudioEngine.h"

namespace studoo
{
AudioEngine::AudioEngine()
{
    for (auto& row : pattern)
        for (auto& s : row)
            s.store (false);

    for (int c = 0; c < numChannels; ++c)
    {
        channelVolume[(size_t) c].store (0.8f);
        channelMuted[(size_t) c].store (false);
        previewRequested[(size_t) c].store (false);
    }

    loadExamplePattern();
}

void AudioEngine::prepareToPlay (int, double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;

    // Fabricamos los sonidos para la frecuencia de muestreo de la tarjeta
    const DrumSound sounds[numChannels] = { DrumSound::kick, DrumSound::snare,
                                            DrumSound::hihat, DrumSound::clap };
    for (int c = 0; c < numChannels; ++c)
        channelSounds[(size_t) c] = renderDrumSound (sounds[c], sampleRate);

    clickHigh = renderDrumSound (DrumSound::clickHigh, sampleRate);
    clickLow  = renderDrumSound (DrumSound::clickLow,  sampleRate);

    for (auto& v : voices)
        v = {};
    metronomeVoice = {};
    samplesUntilNextStep = 0.0;
    nextStep = 0;
}

// ---------------------------------------------------------------------------
//  Transporte
// ---------------------------------------------------------------------------
void AudioEngine::play()
{
    restartRequested.store (true);
    playing.store (true);
}

void AudioEngine::stop()
{
    playing.store (false);
    currentStep.store (-1);
}

void AudioEngine::setBpm (double newBpm)
{
    bpm.store (juce::jlimit (40.0, 250.0, newBpm));
}

// ---------------------------------------------------------------------------
//  Patrón y canales
// ---------------------------------------------------------------------------
void AudioEngine::setStep (int channel, int step, bool on)
{
    if (juce::isPositiveAndBelow (channel, numChannels) && juce::isPositiveAndBelow (step, numSteps))
        pattern[(size_t) channel][(size_t) step].store (on);
}

bool AudioEngine::getStep (int channel, int step) const
{
    if (juce::isPositiveAndBelow (channel, numChannels) && juce::isPositiveAndBelow (step, numSteps))
        return pattern[(size_t) channel][(size_t) step].load();
    return false;
}

void AudioEngine::clearPattern()
{
    for (auto& row : pattern)
        for (auto& s : row)
            s.store (false);
}

void AudioEngine::loadExamplePattern()
{
    clearPattern();
    // Un ritmo sencillo de hip-hop / trap
    for (int s : { 0, 6, 8, 11 })           setStep (0, s, true); // bombo
    for (int s : { 4, 12 })                 setStep (1, s, true); // caja
    for (int s = 0; s < numSteps; s += 2)   setStep (2, s, true); // hi-hat
    setStep (2, 15, true);
    setStep (3, 12, true);                                         // palmada
}

juce::String AudioEngine::getChannelName (int channel)
{
    switch (channel)
    {
        case 0:  return "Bombo";
        case 1:  return "Caja";
        case 2:  return "Hi-hat";
        case 3:  return "Palmada";
        default: return {};
    }
}

void AudioEngine::setChannelVolume (int channel, float gain)
{
    if (juce::isPositiveAndBelow (channel, numChannels))
        channelVolume[(size_t) channel].store (juce::jlimit (0.0f, 1.0f, gain));
}

float AudioEngine::getChannelVolume (int channel) const
{
    return juce::isPositiveAndBelow (channel, numChannels) ? channelVolume[(size_t) channel].load() : 0.0f;
}

void AudioEngine::setChannelMuted (int channel, bool muted)
{
    if (juce::isPositiveAndBelow (channel, numChannels))
        channelMuted[(size_t) channel].store (muted);
}

bool AudioEngine::isChannelMuted (int channel) const
{
    return juce::isPositiveAndBelow (channel, numChannels) && channelMuted[(size_t) channel].load();
}

void AudioEngine::previewChannel (int channel)
{
    if (juce::isPositiveAndBelow (channel, numChannels))
        previewRequested[(size_t) channel].store (true);
}

// ---------------------------------------------------------------------------
//  Audio (se ejecuta en el hilo de audio, cientos de veces por segundo)
// ---------------------------------------------------------------------------
void AudioEngine::startVoice (Voice& v, const juce::AudioBuffer<float>& sound, float gain)
{
    v.sound = &sound;
    v.position = 0;
    v.gain = gain;
}

void AudioEngine::triggerStep (int step)
{
    for (int c = 0; c < numChannels; ++c)
        if (pattern[(size_t) c][(size_t) step].load() && ! channelMuted[(size_t) c].load())
            startVoice (voices[(size_t) c], channelSounds[(size_t) c], 1.0f);

    // Metrónomo: un "clic" en cada tiempo (cada 4 pasos), más agudo en el primero
    if (metronomeOn.load() && step % 4 == 0)
        startVoice (metronomeVoice, step == 0 ? clickHigh : clickLow, 0.7f);
}

void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& info)
{
    info.clearActiveBufferRegion();
    auto* buffer = info.buffer;
    if (buffer == nullptr || buffer->getNumChannels() == 0)
        return;

    // Sonidos de prueba pedidos desde la interfaz
    for (int c = 0; c < numChannels; ++c)
        if (previewRequested[(size_t) c].exchange (false))
            startVoice (voices[(size_t) c], channelSounds[(size_t) c], 1.0f);

    if (restartRequested.exchange (false))
    {
        nextStep = 0;
        samplesUntilNextStep = 0.0;
    }

    const bool isRunning = playing.load();
    // Un paso es una semicorchea: 1/4 de un tiempo
    const double samplesPerStep = sampleRate * 60.0 / bpm.load() / 4.0;
    const float master = masterGain.load();

    std::array<float, numChannels> gains;
    for (int c = 0; c < numChannels; ++c)
        gains[(size_t) c] = channelVolume[(size_t) c].load();

    auto* left  = buffer->getWritePointer (0, info.startSample);
    auto* right = buffer->getNumChannels() > 1 ? buffer->getWritePointer (1, info.startSample) : nullptr;

    for (int i = 0; i < info.numSamples; ++i)
    {
        if (isRunning)
        {
            if (samplesUntilNextStep <= 0.0)
            {
                triggerStep (nextStep);
                currentStep.store (nextStep);
                nextStep = (nextStep + 1) % numSteps;
                samplesUntilNextStep += samplesPerStep;
            }
            samplesUntilNextStep -= 1.0;
        }

        float mix = 0.0f;
        for (int c = 0; c < numChannels; ++c)
        {
            auto& v = voices[(size_t) c];
            if (v.sound != nullptr)
            {
                mix += v.sound->getSample (0, v.position) * v.gain * gains[(size_t) c];
                if (++v.position >= v.sound->getNumSamples())
                    v.sound = nullptr;
            }
        }

        if (metronomeVoice.sound != nullptr)
        {
            mix += metronomeVoice.sound->getSample (0, metronomeVoice.position) * metronomeVoice.gain;
            if (++metronomeVoice.position >= metronomeVoice.sound->getNumSamples())
                metronomeVoice.sound = nullptr;
        }

        // Limitador suave: evita que el sonido "rompa" si se suma demasiado
        const float out = std::tanh (mix * master);
        left[i] = out;
        if (right != nullptr)
            right[i] = out;
    }

    // Si hay más de 2 salidas, copiamos el mismo sonido
    for (int ch = 2; ch < buffer->getNumChannels(); ++ch)
        buffer->copyFrom (ch, info.startSample, *buffer, 0, info.startSample, info.numSamples);
}
}
