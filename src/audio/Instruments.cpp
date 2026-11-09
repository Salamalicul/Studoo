#include "Instruments.h"
#include "DrumSounds.h"

namespace studoo
{
namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    double noteToFrequency (int note) { return 440.0 * std::pow (2.0, (note - 69) / 12.0); }

    // Corrección "PolyBLEP": quita parte de los pitidos raros (aliasing) de las ondas sierra y cuadrada
    float polyBlep (double t, double dt)
    {
        if (t < dt)       { t /= dt;               return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt;    return (float) (t * t + t + t + 1.0); }
        return 0.0f;
    }
}

// ============================================================================
//  VoiceInstrument
// ============================================================================
void VoiceInstrument::render (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi, int numSamples)
{
    int position = 0;
    for (const auto meta : midi)
    {
        const int eventPos = juce::jlimit (0, numSamples, meta.samplePosition);
        if (eventPos > position)
        {
            renderVoices (out, position, eventPos - position);
            position = eventPos;
        }

        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
            noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            noteOff (msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            allNotesOff();
    }

    if (position < numSamples)
        renderVoices (out, position, numSamples - position);
}

// ============================================================================
//  Sampler
// ============================================================================
void SamplerInstrument::setParameters (const ChannelSnap& c)
{
    root = c.root;
    oneShot = c.oneShot;
}

SamplePtr SamplerInstrument::setSample (SamplePtr newSample)
{
    for (auto& v : voices)
        v.active = false;
    std::swap (sample, newSample);
    return newSample;
}

void SamplerInstrument::allNotesOff()
{
    for (auto& v : voices)
        if (v.active)
            v.releasing = true;
}

void SamplerInstrument::noteOn (int note, float velocity)
{
    if (sample == nullptr || sample->buffer.getNumSamples() == 0)
        return;

    // Buscamos una voz libre; si no hay, usamos la más antigua
    Voice* voice = nullptr;
    for (auto& v : voices)
        if (! v.active) { voice = &v; break; }

    if (voice == nullptr)
    {
        voice = &voices[0];
        for (auto& v : voices)
            if (v.age < voice->age)
                voice = &v;
    }

    voice->active = true;
    voice->note = note;
    voice->position = 0.0;
    voice->increment = sample->sampleRate / sr * std::pow (2.0, (note - root) / 12.0);
    voice->gain = velocity;
    voice->env = 1.0f;
    voice->releasing = false;
    voice->age = ++ageCounter;
}

void SamplerInstrument::noteOff (int note)
{
    if (oneShot)
        return; // los sonidos "one-shot" (como la batería) suenan enteros

    for (auto& v : voices)
        if (v.active && v.note == note)
            v.releasing = true;
}

void SamplerInstrument::renderVoices (juce::AudioBuffer<float>& out, int start, int num)
{
    if (sample == nullptr)
        return;

    const auto& src = sample->buffer;
    const int length = src.getNumSamples();
    const float* srcL = src.getReadPointer (0);
    const float* srcR = src.getNumChannels() > 1 ? src.getReadPointer (1) : srcL;
    float* outL = out.getWritePointer (0, start);
    float* outR = out.getWritePointer (1, start);
    const float releaseStep = (float) (1.0 / (0.03 * sr));

    for (auto& v : voices)
    {
        if (! v.active)
            continue;

        for (int i = 0; i < num; ++i)
        {
            const int idx = (int) v.position;
            if (idx + 1 >= length)
            {
                v.active = false;
                break;
            }

            const float frac = (float) (v.position - idx);
            const float l = srcL[idx] + (srcL[idx + 1] - srcL[idx]) * frac;
            const float r = srcR[idx] + (srcR[idx + 1] - srcR[idx]) * frac;
            const float g = v.gain * v.env;
            outL[i] += l * g;
            outR[i] += r * g;

            v.position += v.increment;

            if (v.releasing)
            {
                v.env -= releaseStep;
                if (v.env <= 0.0f)
                {
                    v.active = false;
                    break;
                }
            }
        }
    }
}

// ============================================================================
//  Sintetizador
// ============================================================================
void SynthInstrument::prepare (double sampleRate, int maxBlockSize)
{
    sr = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (1, maxBlockSize), 1 };
    for (auto& v : voices)
    {
        v.adsr.setSampleRate (sampleRate);
        v.filter.prepare (spec);
        v.filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        v.note = -1;
    }
}

void SynthInstrument::setParameters (const ChannelSnap& c)
{
    const bool filterChanged = ! juce::approximatelyEqual (c.synth.cutoff, params.cutoff)
                               || ! juce::approximatelyEqual (c.synth.resonance, params.resonance);
    params = c.synth;

    const juce::ADSR::Parameters adsrParams { juce::jmax (0.001f, params.attack), juce::jmax (0.001f, params.decay),
                                              juce::jlimit (0.0f, 1.0f, params.sustain), juce::jmax (0.005f, params.release) };
    const float cutoff = juce::jlimit (20.0f, (float) (sr * 0.45), params.cutoff);
    const float q = 0.707f + juce::jlimit (0.0f, 1.0f, params.resonance) * 8.0f;

    for (auto& v : voices)
    {
        v.adsr.setParameters (adsrParams);
        if (filterChanged || v.note < 0)
        {
            v.filter.setCutoffFrequency (cutoff);
            v.filter.setResonance (q);
        }
    }
}

void SynthInstrument::allNotesOff()
{
    for (auto& v : voices)
        v.adsr.noteOff();
}

void SynthInstrument::updateVoiceFrequency (Voice& v)
{
    const double f = noteToFrequency (v.note);
    v.inc1 = f / sr;
    v.inc2 = v.inc1 * std::pow (2.0, params.detune / 1200.0);
}

void SynthInstrument::noteOn (int note, float velocity)
{
    Voice* voice = nullptr;
    for (auto& v : voices)
        if (v.note < 0 || ! v.adsr.isActive()) { voice = &v; break; }

    if (voice == nullptr)
    {
        voice = &voices[0];
        for (auto& v : voices)
            if (v.age < voice->age)
                voice = &v;
    }

    voice->note = note;
    voice->velocity = velocity;
    voice->phase1 = 0.0;
    voice->phase2 = 0.37;
    voice->age = ++ageCounter;
    updateVoiceFrequency (*voice);
    voice->filter.reset();
    voice->filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (sr * 0.45), params.cutoff));
    voice->filter.setResonance (0.707f + juce::jlimit (0.0f, 1.0f, params.resonance) * 8.0f);
    voice->adsr.reset();
    voice->adsr.noteOn();
}

void SynthInstrument::noteOff (int note)
{
    for (auto& v : voices)
        if (v.note == note)
            v.adsr.noteOff();
}

float SynthInstrument::oscillator (double t, double dt) const
{
    switch (params.wave)
    {
        case 1: // cuadrada
        {
            float s = t < 0.5 ? 1.0f : -1.0f;
            s += polyBlep (t, dt);
            s -= polyBlep (std::fmod (t + 0.5, 1.0), dt);
            return s;
        }
        case 2: // seno
            return (float) std::sin (twoPi * t);
        case 3: // triángulo
            return (float) (2.0 * std::abs (2.0 * t - 1.0) - 1.0);
        default: // sierra
            return (float) (2.0 * t - 1.0) - polyBlep (t, dt);
    }
}

void SynthInstrument::renderVoices (juce::AudioBuffer<float>& out, int start, int num)
{
    float* outL = out.getWritePointer (0, start);
    float* outR = out.getWritePointer (1, start);

    for (auto& v : voices)
    {
        if (v.note < 0)
            continue;

        if (! v.adsr.isActive())
        {
            v.note = -1;
            continue;
        }

        for (int i = 0; i < num; ++i)
        {
            const float osc = 0.5f * (oscillator (v.phase1, v.inc1) + oscillator (v.phase2, v.inc2));
            v.phase1 += v.inc1; if (v.phase1 >= 1.0) v.phase1 -= 1.0;
            v.phase2 += v.inc2; if (v.phase2 >= 1.0) v.phase2 -= 1.0;

            const float filtered = v.filter.processSample (0, osc);
            const float s = filtered * v.adsr.getNextSample() * v.velocity * 0.3f;
            outL[i] += s;
            outR[i] += s;
        }
    }
}

// ============================================================================
//  808
// ============================================================================
void Bass808Instrument::setParameters (const ChannelSnap& c)
{
    params = c.bass;
}

void Bass808Instrument::allNotesOff()
{
    releasing = true;
}

void Bass808Instrument::noteOn (int newNote, float vel)
{
    const double f = noteToFrequency (newNote);
    const bool legato = active && ! releasing && amp > 0.02f && params.glide > 0.001f;

    if (legato)
    {
        targetFreq = f; // glide: el tono "se desliza" hasta la nueva nota
    }
    else
    {
        freq = targetFreq = f;
        phase = 0.0;
        timeSinceAttack = 0.0;
    }

    note = newNote;
    velocity = vel;
    amp = vel;
    active = true;
    releasing = false;
}

void Bass808Instrument::noteOff (int n)
{
    if (n == note)
        releasing = true;
}

void Bass808Instrument::renderVoices (juce::AudioBuffer<float>& out, int start, int num)
{
    if (! active)
        return;

    float* outL = out.getWritePointer (0, start);
    float* outR = out.getWritePointer (1, start);

    const double glideCoef = params.glide > 0.001f ? 1.0 - std::exp (-1.0 / (params.glide * 0.3 * sr)) : 1.0;
    const float decayCoef = (float) std::exp (std::log (0.001) / (juce::jmax (0.05f, params.decay) * sr));
    const float releaseCoef = (float) std::exp (std::log (0.001) / (0.06 * sr));
    const float k = 1.0f + juce::jlimit (0.0f, 1.0f, params.drive) * 8.0f;
    const float norm = 1.0f / std::tanh (k);
    const double dt = 1.0 / sr;

    for (int i = 0; i < num; ++i)
    {
        freq += (targetFreq - freq) * glideCoef;
        // "Punch": el tono empieza más agudo y cae muy rápido, como un 808 real
        const double punch = 1.0 + 0.8 * std::exp (-timeSinceAttack * 35.0);
        phase += freq * punch * dt;
        if (phase >= 1.0) phase -= 1.0;

        const float s = std::tanh ((float) std::sin (twoPi * phase) * k) * norm;
        const float v = s * amp * 0.8f;
        outL[i] += v;
        outR[i] += v;

        amp *= decayCoef;
        if (releasing)
            amp *= releaseCoef;
        timeSinceAttack += dt;

        if (amp < 0.0001f)
        {
            active = false;
            break;
        }
    }
}

// ============================================================================
//  Plugin VST3
// ============================================================================
void PluginInstrument::prepare (double, int maxBlockSize)
{
    int channels = 2;
    if (plugin != nullptr)
        channels = juce::jmax (2, plugin->getTotalNumInputChannels(), plugin->getTotalNumOutputChannels());
    buffer.setSize (channels, juce::jmax (1, maxBlockSize));
    midiCopy.ensureSize (8192);
}

void PluginInstrument::render (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi, int numSamples)
{
    if (plugin == nullptr)
        return;

    const int channels = juce::jmax (2, plugin->getTotalNumInputChannels(), plugin->getTotalNumOutputChannels());
    if (channels > buffer.getNumChannels() || numSamples > buffer.getNumSamples())
        return;

    midiCopy.clear();
    if (pendingAllNotesOff)
    {
        for (int ch = 1; ch <= 16; ++ch)
            midiCopy.addEvent (juce::MidiMessage::allNotesOff (ch), 0);
        pendingAllNotesOff = false;
    }
    midiCopy.addEvents (midi, 0, numSamples, 0);

    juce::AudioBuffer<float> view (buffer.getArrayOfWritePointers(), channels, numSamples);
    view.clear();

    {
        const juce::ScopedLock sl (plugin->getCallbackLock());
        if (plugin->isSuspended())
            return;
        plugin->processBlock (view, midiCopy);
    }

    const int outs = juce::jmax (1, plugin->getTotalNumOutputChannels());
    out.addFrom (0, 0, view, 0, 0, numSamples);
    out.addFrom (1, 0, view, juce::jmin (1, outs - 1), 0, numSamples);
}

// ============================================================================
//  Metrónomo
// ============================================================================
void MetronomeInstrument::prepare (double sampleRate)
{
    high = renderDrumSound (DrumSound::clickHigh, sampleRate);
    low = renderDrumSound (DrumSound::clickLow, sampleRate);
    current = pending = nullptr;
    startOffset = -1;
}

void MetronomeInstrument::trigger (bool accent, int sampleOffset)
{
    pending = accent ? &high : &low;
    startOffset = sampleOffset;
}

void MetronomeInstrument::render (juce::AudioBuffer<float>& out, int numSamples)
{
    float* l = out.getWritePointer (0);
    float* r = out.getWritePointer (1);

    for (int i = 0; i < numSamples; ++i)
    {
        if (pending != nullptr && i == startOffset)
        {
            current = pending;
            pending = nullptr;
            startOffset = -1;
            position = 0;
        }

        if (current != nullptr)
        {
            const float v = current->getSample (0, position) * 0.6f;
            l[i] += v;
            r[i] += v;
            if (++position >= current->getNumSamples())
                current = nullptr;
        }
    }

    if (pending != nullptr) // el clic cae en un bloque posterior
        startOffset = juce::jmax (0, startOffset - numSamples);
}
}
