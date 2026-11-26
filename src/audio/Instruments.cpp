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
        {
            noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
            extras = {};
        }
        else if (msg.isController())
        {
            const float v = msg.getControllerValue() / 127.0f;
            switch (msg.getControllerNumber())
            {
                case ccPan:   extras.pan = v * 2.0f - 1.0f; break;
                case ccFine:  extras.fine = v * 200.0f - 100.0f; break;
                case ccMod:   extras.mod = v; break;
                case ccSlide: extras.slide = msg.getControllerValue() >= 64; break;
                default: break;
            }
        }
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
    params = c.sampler;
}

std::vector<SamplerInstrument::Zone> SamplerInstrument::setZones (std::vector<Zone> newZones, const juce::String& key)
{
    for (auto& v : voices)
        v.active = false;   // ninguna voz puede seguir leyendo los samples viejos
    std::swap (zones, newZones);
    zonesKey = key;
    return newZones;
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
    // Qué sample suena: el de la zona de esta nota (multisample) o el sample del canal
    const SampleData* data = sample.get();
    int baseNote = root;
    if (! zones.empty())
    {
        const Zone* best = nullptr;
        for (auto& z : zones)
            if (z.sample != nullptr && (best == nullptr
                    || (note >= z.lo && note <= z.hi && ! (note >= best->lo && note <= best->hi))
                    || ((note >= z.lo && note <= z.hi) == (note >= best->lo && note <= best->hi)
                        && std::abs (z.root - note) < std::abs (best->root - note))))
                best = &z;
        if (best != nullptr)
        {
            data = best->sample.get();
            baseNote = best->root;
        }
    }
    if (data == nullptr || data->buffer.getNumSamples() == 0)
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

    // Parte del sample que suena: inicio/final y, si está troceado, el trozo de esta nota
    const double length = (double) data->buffer.getNumSamples();
    double from = juce::jlimit (0.0, length - 1.0, (double) params.start * length);
    double to = juce::jlimit (from + 1.0, length, (double) params.end * length);
    double semitones = params.tune;
    if (! zones.empty())
    {
        semitones += (double) (note - baseNote);   // multisample: sin trozos
    }
    else if (params.numSlicePoints > 1)
    {
        // Trozos en los golpes detectados
        const int n = params.numSlicePoints;
        const int slice = ((note - root) % n + n) % n;
        const double a = juce::jlimit (0.0, length - 1.0, (double) params.slicePoints[(size_t) slice] * length);
        const double b = slice + 1 < n ? (double) params.slicePoints[(size_t) slice + 1] * length : to;
        from = a;
        to = juce::jlimit (from + 1.0, length, b);
    }
    else if (params.slices > 1)
    {
        const int slice = ((note - root) % params.slices + params.slices) % params.slices;
        const double size = (to - from) / params.slices;
        from = from + size * slice;
        to = from + size;
    }
    else
    {
        semitones += (double) (note - root);
    }

    // Nota deslizante: empieza en el tono de la última nota que sonaba y llega a la suya
    double glideFrom = 1.0;
    const double newIncrement = data->sampleRate / sr * std::pow (2.0, semitones / 12.0) * fineRatio (extras.fine);
    if (extras.slide)
    {
        const Voice* last = nullptr;
        for (auto& v : voices)
            if (v.active && &v != voice && (last == nullptr || v.age > last->age))
                last = &v;
        if (last != nullptr && newIncrement > 0.0)
            glideFrom = std::abs (last->increment) * last->glideRatio / newIncrement;
    }

    voice->active = true;
    voice->note = note;
    voice->regionStart = from;
    voice->regionEnd = to;
    voice->data = data;
    voice->increment = newIncrement;
    voice->glideRatio = juce::jlimit (0.125, 8.0, glideFrom);
    panGains (extras.pan, voice->gl, voice->gr);
    voice->position = params.reverse ? to - 1.0 - 1.0e-6 : from;
    if (params.reverse)
        voice->increment = -voice->increment;
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
    float* outL = out.getWritePointer (0, start);
    float* outR = out.getWritePointer (1, start);
    const float releaseStep = (float) (1.0 / (0.03 * sr));
    const double glideCoef = std::exp (-1.0 / (0.02 * sr));   // notas deslizantes: unos 80 ms

    for (auto& v : voices)
    {
        if (! v.active || v.data == nullptr)
            continue;
        const auto& src = v.data->buffer;
        const int length = src.getNumSamples();
        const float* srcL = src.getReadPointer (0);
        const float* srcR = src.getNumChannels() > 1 ? src.getReadPointer (1) : srcL;

        for (int i = 0; i < num; ++i)
        {
            // Fuera de su parte del sample (o del sample entero): la voz termina
            if (v.position < v.regionStart || v.position >= v.regionEnd - 1.0)
            {
                v.active = false;
                break;
            }
            const int idx = (int) v.position;
            if (idx < 0 || idx + 1 >= length)
            {
                v.active = false;
                break;
            }

            const float frac = (float) (v.position - idx);
            const float l = srcL[idx] + (srcL[idx + 1] - srcL[idx]) * frac;
            const float r = srcR[idx] + (srcR[idx + 1] - srcR[idx]) * frac;
            // Pequeño fundido al final de su parte (sin "clics" al trocear o recortar)
            const double remaining = v.increment > 0.0 ? v.regionEnd - v.position : v.position - v.regionStart;
            const float fade = (float) juce::jmin (1.0, remaining / (0.004 * sr * std::abs (v.increment) + 1.0));
            const float g = v.gain * v.env * fade;
            outL[i] += l * g * v.gl;
            outR[i] += r * g * v.gr;

            if (! juce::approximatelyEqual (v.glideRatio, 1.0))
                v.glideRatio = std::abs (v.glideRatio - 1.0) < 1.0e-5 ? 1.0 : 1.0 + (v.glideRatio - 1.0) * glideCoef;
            v.position += v.increment * v.glideRatio;

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
            v.filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (sr * 0.45), cutoff * v.modMul));
            v.filter.setResonance (q);
        }
    }
}

void SynthInstrument::allNotesOff()
{
    for (auto& v : voices)
    {
        v.adsr.noteOff();
        v.released = true;
    }
}

void SynthInstrument::updateVoiceFrequency (Voice& v)
{
    const double f = noteToFrequency (v.note) * v.fine;
    v.inc1 = f / sr;
    v.inc2 = v.inc1 * std::pow (2.0, params.detune / 1200.0);
}

void SynthInstrument::noteOn (int note, float velocity)
{
    // Modo mono (o nota deslizante): si ya suena una nota, la misma voz se desliza a la nueva
    const float glideTime = extras.slide ? juce::jmax (params.glide, 0.08f) : params.glide;
    const float cutoffNow = juce::jlimit (20.0f, (float) (sr * 0.45), params.cutoff);
    if (params.mono || extras.slide)
    {
        Voice* playing = nullptr;
        for (auto& v : voices)
            if (v.note >= 0 && v.adsr.isActive() && (playing == nullptr || v.age > playing->age))
                playing = &v;
        if (playing != nullptr)
        {
            const double oldFreq = noteToFrequency (playing->note) * playing->fine * playing->glideRatio;
            if (params.mono)
                for (auto& v : voices)
                    if (&v != playing && v.note >= 0)
                        v.adsr.noteOff();
            playing->note = note;
            playing->velocity = velocity;
            playing->age = ++ageCounter;
            playing->fine = fineRatio (extras.fine);
            playing->glideTime = glideTime;
            panGains (extras.pan, playing->gl, playing->gr);
            playing->modMul = std::pow (2.0f, extras.mod * 3.0f);
            playing->filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (sr * 0.45), cutoffNow * playing->modMul));
            updateVoiceFrequency (*playing);
            playing->glideRatio = glideTime > 0.001f ? oldFreq / (noteToFrequency (note) * playing->fine) : 1.0;
            if (playing->adsr.isActive() && ! playing->released && ! juce::approximatelyEqual (params.sustain, 0.0f))
                return;
            playing->released = false;
            playing->adsr.noteOn();
            return;
        }
    }

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
    voice->glideRatio = 1.0;
    voice->glideTime = glideTime;
    voice->age = ++ageCounter;
    voice->fine = fineRatio (extras.fine);
    panGains (extras.pan, voice->gl, voice->gr);
    voice->modMul = std::pow (2.0f, extras.mod * 3.0f);
    updateVoiceFrequency (*voice);
    voice->filter.reset();
    voice->filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (sr * 0.45), cutoffNow * voice->modMul));
    voice->filter.setResonance (0.707f + juce::jlimit (0.0f, 1.0f, params.resonance) * 8.0f);
    voice->released = false;
    voice->adsr.reset();
    voice->adsr.noteOn();
}

void SynthInstrument::noteOff (int note)
{
    for (auto& v : voices)
        if (v.note == note)
        {
            v.adsr.noteOff();
            v.released = true;
        }
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
        case 4: // FM: una onda seno que modula a otra (campanas, pianos eléctricos, bajos "DX")
            return (float) std::sin (twoPi * t + params.timbre * 6.0 * std::sin (twoPi * params.fmRatio * t));
        case 5: // Tabla de ondas: el timbre pasa de seno a triángulo, sierra y cuadrada
        {
            const float pos = params.timbre * 3.0f;
            const int a = juce::jmin (2, (int) pos);
            const float f = pos - (float) a;
            auto shape = [&] (int k) -> float
            {
                switch (k)
                {
                    case 0:  return (float) std::sin (twoPi * t);
                    case 1:  return (float) (2.0 * std::abs (2.0 * t - 1.0) - 1.0);
                    case 2:  return (float) (2.0 * t - 1.0) - polyBlep (t, dt);
                    default: return (t < 0.5 ? 1.0f : -1.0f) + polyBlep (t, dt) - polyBlep (std::fmod (t + 0.5, 1.0), dt);
                }
            };
            return shape (a) * (1.0f - f) + shape (a + 1) * f;
        }
        default: // sierra
            return (float) (2.0 * t - 1.0) - polyBlep (t, dt);
    }
}

void SynthInstrument::renderVoices (juce::AudioBuffer<float>& out, int start, int num)
{
    float* outL = out.getWritePointer (0, start);
    float* outR = out.getWritePointer (1, start);

    if (params.lfoAmount < 0.001f)
    {
        renderVoiceRange (outL, outR, num);
        return;
    }

    // Con LFO: el filtro se mueve cada 32 muestras (suficiente para que suene suave)
    const float baseCutoff = juce::jlimit (20.0f, (float) (sr * 0.45), params.cutoff);
    for (int done = 0; done < num;)
    {
        const int n = juce::jmin (32, num - done);
        const float lfo = (float) std::sin (juce::MathConstants<double>::twoPi * lfoPhase);
        const float cutoff = juce::jlimit (20.0f, (float) (sr * 0.45), baseCutoff * std::pow (2.0f, lfo * params.lfoAmount * 3.0f));
        for (auto& v : voices)
            if (v.note >= 0)
                v.filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (sr * 0.45), cutoff * v.modMul));
        renderVoiceRange (outL + done, outR + done, n);
        lfoPhase = std::fmod (lfoPhase + params.lfoRate * n / sr, 1.0);
        done += n;
    }
}

void SynthInstrument::renderVoiceRange (float* outL, float* outR, int num)
{
    for (auto& v : voices)
    {
        if (v.note < 0)
            continue;

        if (! v.adsr.isActive())
        {
            v.note = -1;
            continue;
        }

        const double glideCoef = v.glideTime > 0.001f ? std::exp (-1.0 / (v.glideTime * 0.25 * sr)) : 0.0;
        for (int i = 0; i < num; ++i)
        {
            if (! juce::approximatelyEqual (v.glideRatio, 1.0))
                v.glideRatio = std::abs (v.glideRatio - 1.0) < 1.0e-5 ? 1.0 : 1.0 + (v.glideRatio - 1.0) * glideCoef;
            const double inc1 = v.inc1 * v.glideRatio, inc2 = v.inc2 * v.glideRatio;
            const float osc = 0.5f * (oscillator (v.phase1, inc1) + oscillator (v.phase2, inc2));
            v.phase1 += inc1; if (v.phase1 >= 1.0) v.phase1 -= 1.0;
            v.phase2 += inc2; if (v.phase2 >= 1.0) v.phase2 -= 1.0;

            const float filtered = v.filter.processSample (0, osc);
            const float s = filtered * v.adsr.getNextSample() * v.velocity * 0.3f;
            outL[i] += s * v.gl;
            outR[i] += s * v.gr;
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
    const double f = noteToFrequency (newNote) * fineRatio (extras.fine);
    glideTime = extras.slide ? juce::jmax (params.glide, 0.08f) : params.glide;
    const bool legato = active && (! releasing || extras.slide) && amp > 0.02f && glideTime > 0.001f;
    panGains (extras.pan, gl, gr);

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

    const double glideCoef = glideTime > 0.001f ? 1.0 - std::exp (-1.0 / (glideTime * 0.3 * sr)) : 1.0;
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
        outL[i] += v * gl;
        outR[i] += v * gr;

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
