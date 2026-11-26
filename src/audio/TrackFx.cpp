#include "TrackFx.h"
#include "model/Scales.h"

namespace studoo
{
// ============================================================================
//  Puerta de ruido
// ============================================================================
void NoiseGate::process (juce::AudioBuffer<float>& b, int n, float thresholdDb)
{
    if (thresholdDb <= -79.0f)
        return;
    const float threshold = juce::Decibels::decibelsToGain (thresholdDb);
    const float envRelease = (float) std::exp (-1.0 / (0.02 * sr));
    const float open = (float) (1.0 - std::exp (-1.0 / (0.001 * sr)));    // abre en 1 ms
    const float close = (float) (1.0 - std::exp (-1.0 / (0.08 * sr)));    // cierra en 80 ms
    float* l = b.getWritePointer (0);
    float* r = b.getWritePointer (1);
    for (int i = 0; i < n; ++i)
    {
        const float level = juce::jmax (std::abs (l[i]), std::abs (r[i]));
        env = level > env ? level : env * envRelease;
        const float target = env > threshold ? 1.0f : 0.0f;
        gain += (target - gain) * (target > gain ? open : close);
        l[i] *= gain;
        r[i] *= gain;
    }
}

// ============================================================================
//  Autotune
// ============================================================================
namespace
{
    constexpr int analysisSize = 1024;    // a media frecuencia: unos 46 ms de voz
    constexpr int ringSize = 8192;
    constexpr double shiftWindow = 0.04;  // ventana de las lecturas: 40 ms
}

void PitchCorrector::prepare (double sampleRate)
{
    sr = sampleRate;
    analysis.assign ((size_t) analysisSize, 0.0f);
    yin.assign ((size_t) analysisSize / 2, 0.0f);
    analysisPos = sinceDetect = 0;
    ring.setSize (2, ringSize);
    ring.clear();
    writePos = 0;
    phase = 0.0;
    ratio = targetRatio = 1.0;
    detectedHz.store (0.0f);
}

float PitchCorrector::detectPitch()
{
    // YIN: busca el retardo en el que la señal se parece más a sí misma
    const int half = analysisSize / 2;
    const double rate = sr * 0.5;
    const int minTau = juce::jmax (2, (int) (rate / 1000.0));   // hasta 1000 Hz
    const int maxTau = juce::jmin (half - 1, (int) (rate / 70.0));   // desde 70 Hz
    float energy = 0.0f;
    for (int i = 0; i < analysisSize; ++i)
    {
        const float v = analysis[(size_t) ((analysisPos + i) % analysisSize)];
        energy += v * v;
    }
    if (energy / analysisSize < 1.0e-6f)
        return 0.0f;   // silencio

    auto at = [this] (int i) { return analysis[(size_t) ((analysisPos + i) % analysisSize)]; };
    float running = 0.0f;
    yin[0] = 1.0f;
    for (int tau = 1; tau <= maxTau; ++tau)
    {
        float d = 0.0f;
        for (int i = 0; i < half; ++i)
        {
            const float diff = at (i) - at (i + tau);
            d += diff * diff;
        }
        running += d;
        yin[(size_t) tau] = running > 0.0f ? d * (float) tau / running : 1.0f;
    }
    for (int tau = minTau; tau < maxTau; ++tau)
        if (yin[(size_t) tau] < 0.15f)
        {
            while (tau + 1 < maxTau && yin[(size_t) tau + 1] < yin[(size_t) tau])
                ++tau;
            // Interpolación parabólica para afinar el resultado
            const float a = yin[(size_t) tau - 1], b = yin[(size_t) tau], c = yin[(size_t) tau + 1];
            const float denom = a - 2.0f * b + c;
            const float shift = std::abs (denom) > 1.0e-9f ? 0.5f * (a - c) / denom : 0.0f;
            return (float) (rate / ((double) tau + juce::jlimit (-1.0f, 1.0f, shift)));
        }
    return 0.0f;
}

void PitchCorrector::process (juce::AudioBuffer<float>& b, int n, const TrackParams& p)
{
    const bool correcting = p.tuneAmount > 0.001f;
    if (! correcting && ! analyse)
    {
        ratio = targetRatio = 1.0;
        return;
    }

    float* l = b.getWritePointer (0);
    float* r = b.getWritePointer (1);
    float* ringL = ring.getWritePointer (0);
    float* ringR = ring.getWritePointer (1);
    // Cuanto más autotune, más rápido llega a la nota (al 100%: el efecto "robot")
    const double speed = 1.0 - std::exp (-1.0 / ((0.2 * (1.0 - p.tuneAmount) + 0.0005) * sr));
    const double window = shiftWindow * sr;

    for (int i = 0; i < n; ++i)
    {
        // Análisis a media frecuencia de muestreo
        decimateToggle = ! decimateToggle;
        if (decimateToggle)
        {
            analysis[(size_t) analysisPos] = 0.5f * (l[i] + r[i]);
            analysisPos = (analysisPos + 1) % analysisSize;
            if (++sinceDetect >= analysisSize / 2)
            {
                sinceDetect = 0;
                const float hz = detectPitch();
                detectedHz.store (hz);
                if (hz > 0.0f && correcting)
                {
                    const double midi = 69.0 + 12.0 * std::log2 (hz / 440.0);
                    int best = juce::roundToInt (midi);
                    double bestDist = 100.0;
                    for (int cand = (int) std::floor (midi) - 2; cand <= (int) std::ceil (midi) + 2; ++cand)
                        if (scales::contains (p.tuneScale, p.tuneRoot, cand) && std::abs (cand - midi) < bestDist)
                        {
                            bestDist = std::abs (cand - midi);
                            best = cand;
                        }
                    targetRatio = std::pow (2.0, (best - midi) / 12.0);
                }
                else if (hz <= 0.0f)
                {
                    targetRatio = 1.0;
                }
            }
        }

        ringL[writePos] = l[i];
        ringR[writePos] = r[i];
        if (! correcting)
        {
            writePos = (writePos + 1) & (ringSize - 1);
            continue;
        }

        ratio += (targetRatio - ratio) * speed;
        // Dos lecturas separadas media ventana; el retardo cambia a (1 - ratio) muestras por muestra
        phase += (1.0 - ratio) / window;
        phase -= std::floor (phase);
        float outL = 0.0f, outR = 0.0f;
        for (double ph : { phase, std::fmod (phase + 0.5, 1.0) })
        {
            const double delay = 1.0 + ph * window;
            double readPos = (double) writePos - delay;
            if (readPos < 0.0) readPos += ringSize;
            const int i0 = (int) readPos;
            const int i1 = (i0 + 1) & (ringSize - 1);
            const float frac = (float) (readPos - i0);
            const float w = (float) std::sin (juce::MathConstants<double>::pi * ph);   // sen² de las dos suma 1
            const float g = w * w;
            outL += (ringL[i0] + (ringL[i1] - ringL[i0]) * frac) * g;
            outR += (ringR[i0] + (ringR[i1] - ringR[i0]) * frac) * g;
        }
        l[i] = outL;
        r[i] = outR;
        writePos = (writePos + 1) & (ringSize - 1);
    }
}

// ============================================================================
//  EQ paramétrico
// ============================================================================
void ParametricEq::prepare (double sampleRate)
{
    sr = sampleRate;
    reset();
}

void ParametricEq::reset()
{
    for (auto& band : filters)
        for (auto& f : band)
        {
            f.coefficients = new juce::dsp::IIR::Coefficients<float> (1.0f, 0.0f, 1.0f, 0.0f);
            f.reset();
        }
    lastF.fill (-1.0f);
}

void ParametricEq::process (juce::AudioBuffer<float>& b, int n, const TrackParams& p)
{
    for (size_t band = 0; band < (size_t) numEqBands; ++band)
    {
        if (std::abs (p.peqGain[band]) < 0.05f)
            continue;
        if (! juce::approximatelyEqual (lastF[band], p.peqFreq[band]) || ! juce::approximatelyEqual (lastG[band], p.peqGain[band])
            || ! juce::approximatelyEqual (lastQ[band], p.peqQ[band]))
        {
            lastF[band] = p.peqFreq[band];
            lastG[band] = p.peqGain[band];
            lastQ[band] = p.peqQ[band];
            const auto c = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sr, juce::jlimit (20.0f, (float) (sr * 0.45), p.peqFreq[band]),
                                                                                p.peqQ[band], juce::Decibels::decibelsToGain (p.peqGain[band]));
            for (auto& f : filters[band]) *f.coefficients = *c;
        }
        for (int ch = 0; ch < 2; ++ch)
        {
            float* d = b.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
                d[i] = filters[band][(size_t) ch].processSample (d[i]);
        }
    }
}

// ============================================================================
//  Efecto de ritmo (al estilo de los "gross beat")
// ============================================================================
void RhythmFx::prepare (double sampleRate)
{
    sr = sampleRate;
    if (history.getNumSamples() > 0)
        history.setSize (2, (int) (sr * 3.5));
    history.clear();
    writePos = 0;
}

void RhythmFx::ensureMemory()
{
    if (history.getNumSamples() == 0)
    {
        history.setSize (2, (int) (sr * 3.5));   // 2 tiempos a 40 BPM caben de sobra
        history.clear();
        writePos = 0;
    }
}

void RhythmFx::process (juce::AudioBuffer<float>& b, int n, const TrackParams& p, double positionSteps,
                        double samplesPerStep, bool playing)
{
    const int size = history.getNumSamples();
    if (size == 0)
        return;
    float* l = b.getWritePointer (0);
    float* r = b.getWritePointer (1);
    float* hl = history.getWritePointer (0);
    float* hr = history.getWritePointer (1);
    const bool active = p.rhythmMode > 0 && playing && samplesPerStep > 1.0;

    for (int i = 0; i < n; ++i)
    {
        hl[writePos] = l[i];
        hr[writePos] = r[i];
        if (active)
        {
            const double step = positionSteps + i / samplesPerStep;   // posición en pasos (semicorcheas)
            double back = -1.0;   // cuántas muestras atrás hay que leer (-1 = el sonido de ahora)
            float gain = 1.0f;
            switch (p.rhythmMode)
            {
                case 1: { // Half-time: cada 2 tiempos suenan a la mitad de velocidad
                    const double inWindow = std::fmod (step, 8.0) * samplesPerStep;
                    back = inWindow - inWindow * 0.5;
                    break;
                }
                case 2: case 3: { // Tartamudeo: se repite el primer trocito (1/8 o 1/16) de cada tiempo
                    const double slice = p.rhythmMode == 2 ? 2.0 : 1.0;
                    const double inBeat = std::fmod (step, 4.0);
                    back = (inBeat - std::fmod (inBeat, slice)) * samplesPerStep;
                    break;
                }
                case 4: case 5: { // Puerta rítmica: sonido sí, sonido no (a 1/16 o 1/32)
                    const double len = p.rhythmMode == 4 ? 1.0 : 0.5;
                    gain = std::fmod (step, len * 2.0) < len ? 1.0f : 0.0f;
                    break;
                }
                case 6: { // Parada de cinta: en el último tiempo de cada compás el sonido se frena
                    const double inBar = std::fmod (step, 16.0);
                    if (inBar >= 12.0)
                    {
                        const double t = (inBar - 12.0) / 4.0;   // 0..1 dentro del último tiempo
                        back = (t - (t - t * t * 0.5)) * 4.0 * samplesPerStep;   // lee cada vez más despacio
                    }
                    break;
                }
                default: break;
            }
            float wl = l[i], wr = r[i];
            if (back > 0.0 && back < size - 2)
            {
                double pos = writePos - back;
                if (pos < 0.0) pos += size;
                const int i0 = (int) pos;
                const int i1 = (i0 + 1) % size;
                const float frac = (float) (pos - i0);
                wl = hl[i0] + (hl[i1] - hl[i0]) * frac;
                wr = hr[i0] + (hr[i1] - hr[i0]) * frac;
            }
            // Suaviza la puerta para que no haga "clic"
            gateGain += (gain - gateGain) * 0.02f;
            wl *= gateGain;
            wr *= gateGain;
            l[i] = l[i] + (wl - l[i]) * p.rhythmMix;
            r[i] = r[i] + (wr - r[i]) * p.rhythmMix;
        }
        if (++writePos >= size) writePos = 0;
    }
}

// ============================================================================
//  Flanger y phaser
// ============================================================================
void ModulationFx::prepare (double sampleRate, int maxBlock)
{
    sr = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (1, maxBlock), 2 };
    phaser.prepare (spec);
    phaser.setDepth (0.7f);
    phaser.setCentreFrequency (900.0f);
    phaser.setFeedback (0.5f);
    lastRate = lastPhaserMix = -1.0f;
    reset();
}

void ModulationFx::reset()
{
    for (auto& b : flangerBuf) b.fill (0.0f);
    flangerFb = {};
    flangerWrite = 0;
    phaser.reset();
}

void ModulationFx::process (juce::AudioBuffer<float>& b, int n, const TrackParams& p)
{
    if (p.flangerMix > 0.01f)
    {
        // Un eco cortísimo (1-7 ms) que se mueve: el sonido "de avión"
        const double inc = p.modRate / sr;
        const float mix = p.flangerMix * 0.7f;
        for (int i = 0; i < n; ++i)
        {
            const double lfo = 0.5 - 0.5 * std::cos (flangerPhase * juce::MathConstants<double>::twoPi);
            flangerPhase += inc;
            if (flangerPhase >= 1.0) flangerPhase -= 1.0;
            for (int ch = 0; ch < 2; ++ch)
            {
                float* d = b.getWritePointer (ch);
                // el canal derecho va un poco desfasado: más ancho
                const double l2 = ch == 0 ? lfo : 0.5 - 0.5 * std::cos ((flangerPhase + 0.25) * juce::MathConstants<double>::twoPi);
                const double delay = (0.001 + 0.006 * l2) * sr;
                double readPos = flangerWrite - delay;
                while (readPos < 0) readPos += flangerSize;
                const int i0 = (int) readPos;
                const float frac = (float) (readPos - i0);
                auto& buf = flangerBuf[(size_t) ch];
                const float delayed = buf[(size_t) i0] + (buf[(size_t) ((i0 + 1) % flangerSize)] - buf[(size_t) i0]) * frac;
                buf[(size_t) flangerWrite] = d[i] + delayed * 0.6f;
                d[i] = d[i] * (1.0f - mix * 0.5f) + delayed * mix;
            }
            flangerWrite = (flangerWrite + 1) % flangerSize;
        }
    }

    if (p.phaserMix > 0.01f)
    {
        if (! juce::approximatelyEqual (p.modRate, lastRate))
        {
            lastRate = p.modRate;
            phaser.setRate (p.modRate);
        }
        if (! juce::approximatelyEqual (p.phaserMix, lastPhaserMix))
        {
            lastPhaserMix = p.phaserMix;
            phaser.setMix (p.phaserMix * 0.8f);
        }
        juce::dsp::AudioBlock<float> block (b);
        auto sub = block.getSubBlock (0, (size_t) n);
        phaser.process (juce::dsp::ProcessContextReplacing<float> (sub));
    }
}

// ============================================================================
//  Procesador de transitorios
// ============================================================================
void TransientShaper::process (juce::AudioBuffer<float>& b, int n, float attack, float sustain)
{
    if (std::abs (attack) < 0.01f && std::abs (sustain) < 0.01f)
        return;
    auto coef = [this] (double seconds) { return (float) std::exp (-1.0 / (seconds * sr)); };
    const float upFast = coef (0.0005), upSlow = coef (0.015), down = coef (0.08);
    const float up = coef (0.001), downFast = coef (0.04), downSlow = coef (0.3);
    auto follow = [] (float& env, float level, float a, float r) { env = level + (env - level) * (level > env ? a : r); };
    float* l = b.getWritePointer (0);
    float* r = b.getWritePointer (1);
    for (int i = 0; i < n; ++i)
    {
        const float level = juce::jmax (std::abs (l[i]), std::abs (r[i]));
        follow (attFast, level, upFast, down);
        follow (attSlow, level, upSlow, down);
        follow (relFast, level, up, downFast);
        follow (relSlow, level, up, downSlow);
        // Golpe: el seguidor rápido va por delante del lento. Cola: el lento tarda más en bajar.
        const float hit = juce::jlimit (0.0f, 1.0f, ((attFast - attSlow) / (attFast + 1.0e-4f) - 0.3f) / 0.7f);
        const float tail = juce::jlimit (0.0f, 1.0f, ((relSlow - relFast) / (relSlow + 1.0e-4f) - 0.1f) / 0.9f);
        const float gainDb = attack * 12.0f * hit + sustain * 9.0f * tail;
        const float g = juce::Decibels::decibelsToGain (juce::jlimit (-18.0f, 12.0f, gainDb));
        l[i] *= g;
        r[i] *= g;
    }
}

// ============================================================================
//  Compresor multibanda
// ============================================================================
void MultibandComp::prepare (double sampleRate, int maxBlock)
{
    sr = sampleRate;
    const int size = juce::jmax (1, maxBlock);
    low.setSize (2, size);
    mid.setSize (2, size);
    high.setSize (2, size);
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) size, 2 };
    for (auto* f : { &lowCut, &midSplitHigh })
    {
        f->prepare (spec);
        f->setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    }
    lowCut.setCutoffFrequency (200.0f);
    midSplitHigh.setCutoffFrequency (3000.0f);
    for (auto& c : comps)
    {
        c.prepare (spec);
        c.setRatio (3.0f);
    }
    comps[0].setAttack (20.0f); comps[0].setRelease (200.0f);
    comps[1].setAttack (8.0f);  comps[1].setRelease (120.0f);
    comps[2].setAttack (2.0f);  comps[2].setRelease (60.0f);
    lastAmount = -1.0f;
}

void MultibandComp::reset()
{
    lowCut.reset(); midSplitHigh.reset();
    for (auto& c : comps) c.reset();
}

void MultibandComp::process (juce::AudioBuffer<float>& b, int n, float amount)
{
    if (amount < 0.01f || n > low.getNumSamples())
        return;
    if (! juce::approximatelyEqual (amount, lastAmount))
    {
        lastAmount = amount;
        for (auto& c : comps)
            c.setThreshold (-amount * 30.0f);
    }
    // Separa en 3 bandas: graves (< 200 Hz), medios y agudos (> 3 kHz)
    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in = b.getReadPointer (ch);
        float* lo = low.getWritePointer (ch);
        float* mi = mid.getWritePointer (ch);
        float* hi = high.getWritePointer (ch);
        for (int i = 0; i < n; ++i)
        {
            float lp = 0.0f, hp = 0.0f;
            lowCut.processSample (ch, in[i], lp, hp);
            lo[i] = lp;
            float mlp = 0.0f, mhp = 0.0f;
            midSplitHigh.processSample (ch, hp, mlp, mhp);
            mi[i] = mlp;
            hi[i] = mhp;
        }
    }
    const std::array<juce::AudioBuffer<float>*, 3> bands { &low, &mid, &high };
    const float makeup = juce::Decibels::decibelsToGain (amount * 8.0f);
    b.clear (0, n);
    for (size_t k = 0; k < 3; ++k)
    {
        juce::dsp::AudioBlock<float> block (*bands[k]);
        auto sub = block.getSubBlock (0, (size_t) n);
        comps[k].process (juce::dsp::ProcessContextReplacing<float> (sub));
        b.addFrom (0, 0, *bands[k], 0, 0, n, makeup);
        b.addFrom (1, 0, *bands[k], 1, 0, n, makeup);
    }
}

// ============================================================================
//  Vocoder
// ============================================================================
void Vocoder::prepare (double sampleRate)
{
    sr = sampleRate;
    for (int k = 0; k < numBands; ++k)
    {
        // Bandas repartidas de 120 Hz a 7 kHz (como las teclas de un piano: más juntas en los graves)
        const double f = 120.0 * std::pow (7000.0 / 120.0, (double) k / (numBands - 1));
        const auto c = juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, juce::jmin (f, sampleRate * 0.45), 6.0);
        modFilters[(size_t) k].coefficients = c;
        carFilters[(size_t) k].coefficients = new juce::dsp::IIR::Coefficients<float> (*c);
        // las bandas agudas se oyen menos: un poco más de volumen
        bandGain[(size_t) k] = 1.0f + 1.5f * (float) k / (numBands - 1);
    }
    reset();
}

void Vocoder::reset()
{
    for (auto& f : modFilters) f.reset();
    for (auto& f : carFilters) f.reset();
    env.fill (0.0f);
}

void Vocoder::process (juce::AudioBuffer<float>& carrier, const juce::AudioBuffer<float>& modulator, int n, float mix)
{
    if (mix < 0.01f)
        return;
    const float att = (float) std::exp (-1.0 / (0.004 * sr));
    const float rel = (float) std::exp (-1.0 / (0.03 * sr));
    float* l = carrier.getWritePointer (0);
    float* r = carrier.getWritePointer (1);
    const float* ml = modulator.getReadPointer (0);
    const float* mr = modulator.getReadPointer (1);
    for (int i = 0; i < n; ++i)
    {
        const float m = 0.5f * (ml[i] + mr[i]);
        const float c = 0.5f * (l[i] + r[i]);
        float out = 0.0f;
        for (size_t k = 0; k < (size_t) numBands; ++k)
        {
            const float level = std::abs (modFilters[k].processSample (m));
            env[k] = level + (env[k] - level) * (level > env[k] ? att : rel);
            out += carFilters[k].processSample (c) * env[k] * bandGain[k];
        }
        out = std::tanh (out * 6.0f);
        l[i] = l[i] * (1.0f - mix) + out * mix;
        r[i] = r[i] * (1.0f - mix) + out * mix;
    }
}

// ============================================================================
//  Ancho estéreo
// ============================================================================
void widenStereo (juce::AudioBuffer<float>& b, int n, float width)
{
    if (std::abs (width - 1.0f) < 0.01f)
        return;
    float* l = b.getWritePointer (0);
    float* r = b.getWritePointer (1);
    const float w = juce::jlimit (0.0f, 2.0f, width);
    const float norm = 1.0f / juce::jmax (1.0f, 0.5f + 0.5f * w);   // que no suba de volumen al ensanchar
    for (int i = 0; i < n; ++i)
    {
        const float mid = 0.5f * (l[i] + r[i]);
        const float side = 0.5f * (l[i] - r[i]) * w;
        l[i] = (mid + side) * norm;
        r[i] = (mid - side) * norm;
    }
}
}
