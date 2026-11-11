#include "Kits.h"
#include <array>
#include <cmath>

namespace studoo::kits
{
// ============================================================================
//  Lista de kits
// ============================================================================
const std::vector<KitInfo>& all()
{
    static const std::vector<KitInfo> list = {
        { "trap", "Trap", "Bombo con pegada, hi-hats brillantes y un 808 largo y grave.",
          { { "kick", "Bombo" }, { "snare", "Caja" }, { "clap", "Palmada" }, { "hihat", "Hi-hat" },
            { "openhat", "Hi-hat abierto" }, { "rim", "Rim" }, { "perc", "Perc" }, { "snap", "Chasquido" } },
          { true, 2.2f, 0.10f, 0.45f } },

        { "drill", "Drill", "Sonido oscuro y agresivo: caja aguda, hi-hats rápidos y 808 con mucho deslizamiento.",
          { { "kick", "Bombo" }, { "snare", "Caja" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
            { "perc", "Perc" }, { "rim", "Rim" } },
          { true, 2.6f, 0.16f, 0.6f } },

        { "boombap", "Boom Bap (hip-hop clásico)", "Batería redonda y polvorienta de los 90: bombo grueso y caja gorda.",
          { { "kick", "Bombo" }, { "snare", "Caja" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
            { "rim", "Rim" }, { "shaker", "Shaker" }, { "crash", "Platillo" } },
          {} },

        { "reggaeton", "Reggaetón", "Todo lo necesario para el ritmo dembow: caja seca, timbal, bongó y un 808 corto.",
          { { "kick", "Bombo" }, { "snare", "Caja (dembow)" }, { "clap", "Palmada" }, { "hihat", "Hi-hat" },
            { "tom", "Timbal" }, { "perc", "Bongó" }, { "shaker", "Shaker" } },
          { true, 1.0f, 0.05f, 0.25f } },

        { "house", "House / Electrónica", "Inspirado en las cajas de ritmos 909: bombo largo, palmada grande y hi-hats abiertos.",
          { { "kick", "Bombo" }, { "clap", "Palmada" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
            { "crash", "Ride" }, { "shaker", "Shaker" }, { "cowbell", "Cencerro" } },
          {} },

        { "lofi", "Lo-fi", "Sonidos suaves, apagados y con \"polvo\" de disco antiguo. Ideal para beats relajados.",
          { { "kick", "Bombo" }, { "snare", "Caja" }, { "hihat", "Hi-hat" }, { "rim", "Rim" },
            { "shaker", "Shaker" }, { "snap", "Chasquido" } },
          {} },
    };
    return list;
}

const KitInfo* find (const juce::String& kitId)
{
    for (auto& k : all())
        if (kitId == k.id)
            return &k;
    return nullptr;
}

juce::String samplePath (const KitInfo& kit, const SoundInfo& sound)
{
    return juce::String ("builtin:") + kit.id + "/" + sound.id;
}

juce::String roleOf (const juce::String& path)
{
    if (! path.startsWith ("builtin:"))
        return {};
    auto rest = path.fromFirstOccurrenceOf ("builtin:", false, false);
    return rest.containsChar ('/') ? rest.fromFirstOccurrenceOf ("/", false, false) : rest;
}

// ============================================================================
//  Síntesis de los sonidos
// ============================================================================
namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    struct OnePoleHP
    {
        double a = 0.0, prevIn = 0.0, prevOut = 0.0;
        OnePoleHP (double cutoff, double sr) { const double rc = 1.0 / (twoPi * cutoff); a = rc / (rc + 1.0 / sr); }
        double process (double x) { prevOut = a * (prevOut + x - prevIn); prevIn = x; return prevOut; }
    };

    struct OnePoleLP
    {
        double a = 0.0, state = 0.0;
        OnePoleLP (double cutoff, double sr) { const double dt = 1.0 / sr, rc = 1.0 / (twoPi * cutoff); a = dt / (rc + dt); }
        double process (double x) { state += a * (x - state); return state; }
    };

    struct BandPass
    {
        juce::IIRFilter f;
        BandPass (double freq, double q, double sr) { f.setCoefficients (juce::IIRCoefficients::makeBandPass (sr, juce::jmin (freq, sr * 0.45), q)); }
        double process (double x) { return f.processSingleSampleRaw ((float) x); }
    };

    struct Gen
    {
        double sr;
        juce::AudioBuffer<float> buffer;
        float* out = nullptr;
        int n = 0;
        juce::Random random { 4242 };

        Gen (double sampleRate, double seconds) : sr (sampleRate)
        {
            n = juce::jmax (1, (int) (seconds * sr));
            buffer.setSize (1, n);
            buffer.clear();
            out = buffer.getWritePointer (0);
        }

        double noise() { return random.nextDouble() * 2.0 - 1.0; }
        double t (int i) const { return i / sr; }
    };

    // Las 6 frecuencias "metálicas" de los platillos de la Roland 808
    constexpr double metalFreqs[] = { 205.3, 304.4, 369.6, 522.7, 540.0, 800.0 };

    void kick (Gen& g, double f0, double f1, double pitchDecay, double ampDecay, double click, double drive)
    {
        double phase = 0.0;
        const double norm = std::tanh (drive);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double f = f1 + (f0 - f1) * std::exp (-t * pitchDecay);
            phase += twoPi * f / g.sr;
            const double body = std::sin (phase) * std::exp (-t * ampDecay);
            const double c = g.noise() * std::exp (-t * 350.0) * click;
            g.out[i] = (float) (std::tanh ((body + c) * drive) / norm);
        }
    }

    void snare (Gen& g, double toneHz, double toneDecay, double noiseDecay, double hp, double centre, double noiseAmt, double toneAmt)
    {
        OnePoleHP h (hp, g.sr);
        BandPass l (centre, 0.55, g.sr);
        OnePoleLP soft1 (centre * 1.8, g.sr), soft2 (centre * 2.2, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double tone = (std::sin (twoPi * toneHz * t) + 0.5 * std::sin (twoPi * toneHz * 1.52 * t)) * std::exp (-t * toneDecay) * toneAmt;
            const double rattle = soft2.process (soft1.process (l.process (h.process (g.noise())))) * std::exp (-t * noiseDecay) * noiseAmt;
            g.out[i] = (float) std::tanh (tone * 1.3 + rattle * 2.2);
        }
    }

    void clap (Gen& g, double hp, double centre, double tail, double gap)
    {
        OnePoleHP h (hp, g.sr);
        BandPass b (centre, 0.7, g.sr);
        OnePoleLP l (6000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            double env = 0.0;
            for (int k = 0; k < 3; ++k)
            {
                const double s = k * gap;
                if (t >= s && t < s + gap)
                    env = juce::jmax (env, std::exp (-(t - s) * 170.0));
            }
            if (t >= 3.0 * gap)
                env = juce::jmax (env, std::exp (-(t - 3.0 * gap) * tail) * 0.85);
            g.out[i] = (float) (l.process (b.process (h.process (g.noise()))) * env * 2.2);
        }
    }

    void hat (Gen& g, double tune, double decay, double noiseAmt)
    {
        std::array<double, 6> phase {};
        BandPass b (10000.0, 0.8, g.sr);
        OnePoleHP h1 (7000.0, g.sr), h2 (7000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            double metal = 0.0;
            for (size_t k = 0; k < 6; ++k)
            {
                phase[k] += metalFreqs[k] * tune / g.sr;
                if (phase[k] >= 1.0) phase[k] -= 1.0;
                metal += phase[k] < 0.5 ? 1.0 : -1.0;
            }
            const double x = metal / 6.0 + g.noise() * noiseAmt;
            const double env = juce::jmin (1.0, t / 0.0008) * std::exp (-t * decay);
            g.out[i] = (float) (h2.process (h1.process (b.process (x))) * env * 2.5);
        }
    }

    void tom (Gen& g, double f0, double f1, double decay)
    {
        double phase = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double f = f1 + (f0 - f1) * std::exp (-t * 14.0);
            phase += twoPi * f / g.sr;
            g.out[i] = (float) (std::tanh (std::sin (phase) * 1.4 + g.noise() * std::exp (-t * 300.0) * 0.2) * std::exp (-t * decay));
        }
    }

    void rim (Gen& g)
    {
        BandPass b (3200.0, 2.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double x = std::sin (twoPi * 1720.0 * t) * std::exp (-t * 110.0) * 0.6
                           + std::sin (twoPi * 460.0 * t) * std::exp (-t * 80.0) * 0.5
                           + b.process (g.noise()) * std::exp (-t * 220.0) * 1.2;
            g.out[i] = (float) std::tanh (x * 1.5);
        }
    }

    void cowbell (Gen& g)
    {
        BandPass b (900.0, 1.2, g.sr);
        double p1 = 0.0, p2 = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            p1 += 587.0 / g.sr; if (p1 >= 1.0) p1 -= 1.0;
            p2 += 845.0 / g.sr; if (p2 >= 1.0) p2 -= 1.0;
            const double x = (p1 < 0.5 ? 1.0 : -1.0) + (p2 < 0.5 ? 1.0 : -1.0);
            const double env = std::exp (-t * 40.0) * 0.4 + std::exp (-t * 9.0) * 0.6;
            g.out[i] = (float) (b.process (x) * env * 1.6);
        }
    }

    void shaker (Gen& g, double decay)
    {
        OnePoleHP h (5000.0, g.sr);
        OnePoleLP l (13000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double env = t < 0.02 ? t / 0.02 : std::exp (-(t - 0.02) * decay);
            g.out[i] = (float) (l.process (h.process (g.noise())) * env * 1.2);
        }
    }

    void crash (Gen& g, double tune, double decay)
    {
        std::array<double, 6> phase {};
        OnePoleHP h1 (4000.0, g.sr), h2 (4000.0, g.sr);
        OnePoleLP l (14000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            double metal = 0.0;
            for (size_t k = 0; k < 6; ++k)
            {
                phase[k] += metalFreqs[k] * tune / g.sr;
                if (phase[k] >= 1.0) phase[k] -= 1.0;
                metal += phase[k] < 0.5 ? 1.0 : -1.0;
            }
            const double x = metal / 6.0 * 0.6 + g.noise() * 0.7;
            const double env = juce::jmin (1.0, t / 0.002) * std::exp (-t * decay);
            g.out[i] = (float) (l.process (h2.process (h1.process (x))) * env * 1.8);
        }
    }

    void snap (Gen& g)
    {
        BandPass b (2400.0, 1.6, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            g.out[i] = (float) (b.process (g.noise()) * std::exp (-t * 85.0) * 2.6);
        }
    }

    void perc (Gen& g, double f0, double f1, double decay)
    {
        double phase = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double f = f1 + (f0 - f1) * std::exp (-t * 45.0);
            phase += twoPi * f / g.sr;
            g.out[i] = (float) ((std::sin (phase) + 0.15 * std::sin (phase * 2.01)) * std::exp (-t * decay)
                                 + g.noise() * std::exp (-t * 400.0) * 0.15);
        }
    }

    // Efecto "lo-fi": filtro apagado, saturación, menos bits y polvo de disco
    void lofi (Gen& g)
    {
        OnePoleLP l1 (4200.0, g.sr), l2 (5200.0, g.sr);
        juce::Random dust (99);
        for (int i = 0; i < g.n; ++i)
        {
            double x = l2.process (l1.process (g.out[i]));
            x = std::tanh (x * 1.4);
            if (dust.nextInt (6000) == 0)          // algún "crujido" de vinilo
                x += (dust.nextDouble() - 0.5) * 0.06;
            g.out[i] = (float) x;
        }
    }

    void finish (Gen& g)
    {
        const float peak = g.buffer.getMagnitude (0, g.n);
        if (peak > 0.0001f)
            g.buffer.applyGain (0.9f / peak);
        const int fade = juce::jmin (g.n, (int) (0.006 * g.sr));
        g.buffer.applyGainRamp (g.n - fade, fade, 1.0f, 0.0f);
    }
}

juce::AudioBuffer<float> render (const juce::String& kit, const juce::String& sound, double sr)
{
    if (find (kit) == nullptr)
        return {};

    auto is = [&kit] (const char* k) { return kit == k; };
    auto make = [sr] (double seconds) { return Gen (sr, seconds); };

    if (sound == "kick")
    {
        if (is ("trap"))      { auto g = make (0.55); kick (g, 180, 50, 30, 7, 0.35, 2.2); finish (g); return g.buffer; }
        if (is ("drill"))     { auto g = make (0.45); kick (g, 210, 55, 36, 9, 0.5, 3.0); finish (g); return g.buffer; }
        if (is ("boombap"))   { auto g = make (0.6);  kick (g, 140, 48, 22, 6, 0.25, 1.8); finish (g); return g.buffer; }
        if (is ("reggaeton")) { auto g = make (0.5);  kick (g, 170, 52, 30, 8, 0.4, 2.5); finish (g); return g.buffer; }
        if (is ("house"))     { auto g = make (0.7);  kick (g, 165, 50, 26, 5, 0.55, 2.0); finish (g); return g.buffer; }
        if (is ("lofi"))      { auto g = make (0.5);  kick (g, 120, 45, 20, 7, 0.1, 1.4); lofi (g); finish (g); return g.buffer; }
    }
    if (sound == "snare")
    {
        if (is ("trap"))      { auto g = make (0.3);  snare (g, 210, 22, 17, 700, 4200, 0.8, 0.6); finish (g); return g.buffer; }
        if (is ("drill"))     { auto g = make (0.25); snare (g, 260, 26, 21, 900, 5200, 0.9, 0.55); finish (g); return g.buffer; }
        if (is ("boombap"))   { auto g = make (0.38); snare (g, 175, 16, 11, 400, 2600, 0.85, 0.75); finish (g); return g.buffer; }
        if (is ("reggaeton")) { auto g = make (0.28); snare (g, 230, 24, 19, 800, 4800, 0.85, 0.6); finish (g); return g.buffer; }
        if (is ("lofi"))      { auto g = make (0.35); snare (g, 185, 18, 14, 400, 2400, 0.7, 0.7); lofi (g); finish (g); return g.buffer; }
    }
    if (sound == "clap")
    {
        if (is ("trap"))      { auto g = make (0.4);  clap (g, 900, 1300, 14, 0.011); finish (g); return g.buffer; }
        if (is ("reggaeton")) { auto g = make (0.35); clap (g, 1000, 1500, 18, 0.010); finish (g); return g.buffer; }
        if (is ("house"))     { auto g = make (0.55); clap (g, 800, 1200, 9, 0.012); finish (g); return g.buffer; }
    }
    if (sound == "hihat")
    {
        if (is ("trap"))      { auto g = make (0.12); hat (g, 1.0, 55, 0.3); finish (g); return g.buffer; }
        if (is ("drill"))     { auto g = make (0.1);  hat (g, 1.1, 70, 0.25); finish (g); return g.buffer; }
        if (is ("boombap"))   { auto g = make (0.15); hat (g, 0.85, 40, 0.5); finish (g); return g.buffer; }
        if (is ("reggaeton")) { auto g = make (0.12); hat (g, 1.0, 50, 0.35); finish (g); return g.buffer; }
        if (is ("house"))     { auto g = make (0.13); hat (g, 1.05, 45, 0.3); finish (g); return g.buffer; }
        if (is ("lofi"))      { auto g = make (0.15); hat (g, 0.8, 35, 0.5); lofi (g); finish (g); return g.buffer; }
    }
    if (sound == "openhat")
    {
        if (is ("trap"))      { auto g = make (0.6);  hat (g, 1.0, 7, 0.3); finish (g); return g.buffer; }
        if (is ("drill"))     { auto g = make (0.6);  hat (g, 1.1, 6, 0.3); finish (g); return g.buffer; }
        if (is ("boombap"))   { auto g = make (0.7);  hat (g, 0.85, 6, 0.5); finish (g); return g.buffer; }
        if (is ("house"))     { auto g = make (0.8);  hat (g, 1.05, 5, 0.3); finish (g); return g.buffer; }
    }
    if (sound == "rim")
    {
        if (is ("trap") || is ("drill") || is ("boombap")) { auto g = make (0.12); rim (g); finish (g); return g.buffer; }
        if (is ("lofi"))      { auto g = make (0.12); rim (g); lofi (g); finish (g); return g.buffer; }
    }
    if (sound == "perc")
    {
        if (is ("trap"))      { auto g = make (0.25); perc (g, 1100, 820, 22); finish (g); return g.buffer; }
        if (is ("drill"))     { auto g = make (0.2);  perc (g, 1500, 1250, 30); finish (g); return g.buffer; }
        if (is ("reggaeton")) { auto g = make (0.3);  perc (g, 470, 390, 16); finish (g); return g.buffer; }
    }
    if (sound == "tom" && is ("reggaeton"))   { auto g = make (0.45); tom (g, 320, 210, 9); finish (g); return g.buffer; }
    if (sound == "shaker")
    {
        if (is ("lofi"))      { auto g = make (0.18); shaker (g, 30); lofi (g); finish (g); return g.buffer; }
        if (is ("boombap") || is ("reggaeton") || is ("house")) { auto g = make (0.18); shaker (g, 32); finish (g); return g.buffer; }
    }
    if (sound == "crash")
    {
        if (is ("boombap"))   { auto g = make (2.0); crash (g, 1.25, 2.4); finish (g); return g.buffer; }
        if (is ("house"))     { auto g = make (1.4); crash (g, 1.6, 3.8); finish (g); return g.buffer; }
    }
    if (sound == "cowbell" && is ("house"))   { auto g = make (0.45); cowbell (g); finish (g); return g.buffer; }
    if (sound == "snap")
    {
        if (is ("trap"))      { auto g = make (0.15); snap (g); finish (g); return g.buffer; }
        if (is ("lofi"))      { auto g = make (0.15); snap (g); lofi (g); finish (g); return g.buffer; }
    }

    return {};
}
}
