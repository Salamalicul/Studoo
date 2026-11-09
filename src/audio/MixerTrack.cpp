#include "MixerTrack.h"

namespace studoo
{
void MixerTrack::prepare (double sampleRate, int maxBlockSize)
{
    sr = sampleRate;
    buffer.setSize (2, maxBlockSize);
    pluginBuffer.setSize (8, maxBlockSize);

    for (auto& band : eq)
        for (auto& f : band)
        {
            f.coefficients = new juce::dsp::IIR::Coefficients<float> (1.0f, 0.0f, 1.0f, 0.0f);
            f.reset();
        }
    lastLow = lastMid = lastHigh = 1000.0f; // fuerza a recalcular

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 2 };
    compressor.prepare (spec);
    compressor.setAttack (10.0f);
    compressor.setRelease (120.0f);

    delayLine.setSize (2, (int) (sampleRate * 4.0) + 1);
    delayLine.clear();
    delayWrite = 0;

    reverb.setSampleRate (sampleRate);
    reverb.reset();
}

void MixerTrack::reset()
{
    for (auto& band : eq)
        for (auto& f : band)
            f.reset();
    compressor.reset();
    delayLine.clear();
    reverb.reset();
}

void MixerTrack::updateEq (const TrackParams& p)
{
    using AC = juce::dsp::IIR::ArrayCoefficients<float>;

    if (! juce::approximatelyEqual (p.eqLow, lastLow))
    {
        lastLow = p.eqLow;
        const auto c = AC::makeLowShelf (sr, 200.0f, 0.707f, juce::Decibels::decibelsToGain (p.eqLow));
        for (auto& f : eq[0]) *f.coefficients = c;
    }
    if (! juce::approximatelyEqual (p.eqMid, lastMid))
    {
        lastMid = p.eqMid;
        const auto c = AC::makePeakFilter (sr, 1000.0f, 0.8f, juce::Decibels::decibelsToGain (p.eqMid));
        for (auto& f : eq[1]) *f.coefficients = c;
    }
    if (! juce::approximatelyEqual (p.eqHigh, lastHigh))
    {
        lastHigh = p.eqHigh;
        const auto c = AC::makeHighShelf (sr, 5000.0f, 0.707f, juce::Decibels::decibelsToGain (p.eqHigh));
        for (auto& f : eq[2]) *f.coefficients = c;
    }
}

void MixerTrack::process (int n, const TrackParams& p, double bpm, bool silenced)
{
    float* l = buffer.getWritePointer (0);
    float* r = buffer.getWritePointer (1);

    // --- EQ ------------------------------------------------------------------
    updateEq (p);
    for (size_t band = 0; band < 3; ++band)
    {
        const float gain = band == 0 ? p.eqLow : band == 1 ? p.eqMid : p.eqHigh;
        if (std::abs (gain) < 0.05f)
            continue;
        for (int i = 0; i < n; ++i)
        {
            l[i] = eq[band][0].processSample (l[i]);
            r[i] = eq[band][1].processSample (r[i]);
        }
    }

    // --- Compresor --------------------------------------------------------------
    if (p.compOn)
    {
        compressor.setThreshold (p.compThreshold);
        compressor.setRatio (juce::jmax (1.0f, p.compRatio));
        juce::dsp::AudioBlock<float> block (buffer);
        auto sub = block.getSubBlock (0, (size_t) n);
        compressor.process (juce::dsp::ProcessContextReplacing<float> (sub));
        // Ganancia de compensación aproximada
        const float makeup = juce::Decibels::decibelsToGain (-p.compThreshold * (1.0f - 1.0f / juce::jmax (1.0f, p.compRatio)) * 0.5f);
        buffer.applyGain (0, n, makeup);
    }

    // --- Delay (eco sincronizado con el BPM) --------------------------------------
    if (p.delayMix > 0.001f)
    {
        const int size = delayLine.getNumSamples();
        const int delaySamples = juce::jlimit (1, size - 1, (int) (p.delayTime * 60.0 / bpm * sr));
        const float fb = juce::jlimit (0.0f, 0.95f, p.delayFeedback);
        float* dl = delayLine.getWritePointer (0);
        float* dr = delayLine.getWritePointer (1);

        for (int i = 0; i < n; ++i)
        {
            int readPos = delayWrite - delaySamples;
            if (readPos < 0) readPos += size;
            const float el = dl[readPos], er = dr[readPos];
            // Eco "ping-pong": rebota entre izquierda y derecha
            dl[delayWrite] = l[i] + er * fb;
            dr[delayWrite] = r[i] + el * fb;
            l[i] += el * p.delayMix;
            r[i] += er * p.delayMix;
            if (++delayWrite >= size) delayWrite = 0;
        }
    }

    // --- Reverb --------------------------------------------------------------------
    if (p.reverbMix > 0.001f)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize = juce::jlimit (0.0f, 1.0f, p.reverbSize);
        rp.damping = 0.5f;
        rp.wetLevel = p.reverbMix * 0.6f;
        rp.dryLevel = 1.0f;
        rp.width = 1.0f;
        if (! juce::approximatelyEqual (rp.roomSize, lastReverb.roomSize)
            || ! juce::approximatelyEqual (rp.wetLevel, lastReverb.wetLevel))
        {
            reverb.setParameters (rp);
            lastReverb = rp;
        }
        reverb.processStereo (l, r, n);
    }

    // --- Plugin de efecto -------------------------------------------------------------
    if (plugin != nullptr)
    {
        const int channels = juce::jmax (2, plugin->getTotalNumInputChannels(), plugin->getTotalNumOutputChannels());
        if (channels <= pluginBuffer.getNumChannels())
        {
            juce::AudioBuffer<float> view (pluginBuffer.getArrayOfWritePointers(), channels, n);
            view.clear();
            view.copyFrom (0, 0, buffer, 0, 0, n);
            view.copyFrom (1, 0, buffer, 1, 0, n);
            juce::MidiBuffer noMidi;
            const juce::ScopedLock sl (plugin->getCallbackLock());
            if (! plugin->isSuspended())
            {
                plugin->processBlock (view, noMidi);
                buffer.copyFrom (0, 0, view, 0, 0, n);
                buffer.copyFrom (1, 0, view, 1, 0, n);
            }
        }
    }

    // --- Volumen y panorama -------------------------------------------------------------
    if (silenced)
    {
        buffer.clear (0, n);
    }
    else
    {
        const float angle = (juce::jlimit (-1.0f, 1.0f, p.pan) + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        const float gl = p.volume * std::cos (angle) * juce::MathConstants<float>::sqrt2;
        const float gr = p.volume * std::sin (angle) * juce::MathConstants<float>::sqrt2;
        buffer.applyGain (0, 0, n, gl);
        buffer.applyGain (1, 0, n, gr);
    }

    // --- Medidor ---------------------------------------------------------------------------
    for (int ch = 0; ch < 2; ++ch)
    {
        const float m = buffer.getMagnitude (ch, 0, n);
        if (m > peak[(size_t) ch].load())
            peak[(size_t) ch].store (m);
    }
}

void MixerTrack::decayPeaks()
{
    for (auto& p : peak)
        p.store (p.load() * 0.85f);
}
}
