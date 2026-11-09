#include "DrumSounds.h"

namespace studoo
{
namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    // Filtro muy sencillo que deja pasar los agudos (quita graves).
    struct HighPass
    {
        double a = 0.0, prevIn = 0.0, prevOut = 0.0;
        HighPass (double cutoffHz, double sr)
        {
            const double rc = 1.0 / (twoPi * cutoffHz);
            a = rc / (rc + 1.0 / sr);
        }
        double process (double x)
        {
            prevOut = a * (prevOut + x - prevIn);
            prevIn = x;
            return prevOut;
        }
    };

    // Filtro muy sencillo que deja pasar los graves (quita agudos).
    struct LowPass
    {
        double a = 0.0, state = 0.0;
        LowPass (double cutoffHz, double sr)
        {
            const double dt = 1.0 / sr, rc = 1.0 / (twoPi * cutoffHz);
            a = dt / (rc + dt);
        }
        double process (double x) { state += a * (x - state); return state; }
    };
}

juce::AudioBuffer<float> renderDrumSound (DrumSound sound, double sr)
{
    juce::Random random (1234); // semilla fija: el sonido es siempre el mismo

    auto noise = [&random] { return random.nextDouble() * 2.0 - 1.0; };

    double lengthSeconds = 0.3;
    switch (sound)
    {
        case DrumSound::kick:      lengthSeconds = 0.6;  break;
        case DrumSound::snare:     lengthSeconds = 0.3;  break;
        case DrumSound::hihat:     lengthSeconds = 0.09; break;
        case DrumSound::clap:      lengthSeconds = 0.35; break;
        case DrumSound::clickHigh:
        case DrumSound::clickLow:  lengthSeconds = 0.04; break;
    }

    const int numSamples = juce::jmax (1, (int) (lengthSeconds * sr));
    juce::AudioBuffer<float> buffer (1, numSamples);
    auto* out = buffer.getWritePointer (0);

    switch (sound)
    {
        case DrumSound::kick:
        {
            // Un tono grave cuya frecuencia baja rápidamente (de ~155 Hz a 45 Hz)
            double phase = 0.0;
            for (int i = 0; i < numSamples; ++i)
            {
                const double t = i / sr;
                const double freq = 45.0 + 110.0 * std::exp (-t * 28.0);
                phase += twoPi * freq / sr;
                const double body  = std::sin (phase) * std::exp (-t * 5.5);
                const double click = noise() * std::exp (-t * 400.0) * 0.25;
                out[i] = (float) std::tanh ((body + click) * 1.6);
            }
            break;
        }
        case DrumSound::snare:
        {
            // Un tono corto + ruido (las "bordonas" de la caja)
            HighPass hp (1200.0, sr);
            for (int i = 0; i < numSamples; ++i)
            {
                const double t = i / sr;
                const double tone = std::sin (twoPi * 185.0 * t) * std::exp (-t * 22.0) * 0.55;
                const double rattle = hp.process (noise()) * std::exp (-t * 13.0) * 0.8;
                out[i] = (float) std::tanh (tone + rattle);
            }
            break;
        }
        case DrumSound::hihat:
        {
            // Ruido muy agudo y muy corto
            HighPass hp1 (6000.0, sr), hp2 (6000.0, sr);
            for (int i = 0; i < numSamples; ++i)
            {
                const double t = i / sr;
                out[i] = (float) (hp2.process (hp1.process (noise())) * std::exp (-t * 55.0) * 0.9);
            }
            break;
        }
        case DrumSound::clap:
        {
            // Varias ráfagas de ruido muy seguidas, como varias manos aplaudiendo
            HighPass hp (900.0, sr);
            LowPass lp (5000.0, sr);
            for (int i = 0; i < numSamples; ++i)
            {
                const double t = i / sr;
                double env = 0.0;
                for (double start : { 0.0, 0.011, 0.022 })
                    if (t >= start && t < start + 0.011)
                        env = juce::jmax (env, std::exp (-(t - start) * 180.0));
                if (t >= 0.033)
                    env = juce::jmax (env, std::exp (-(t - 0.033) * 16.0) * 0.8);

                out[i] = (float) (lp.process (hp.process (noise())) * env * 1.4);
            }
            break;
        }
        case DrumSound::clickHigh:
        case DrumSound::clickLow:
        {
            const double freq = sound == DrumSound::clickHigh ? 1760.0 : 1100.0;
            for (int i = 0; i < numSamples; ++i)
            {
                const double t = i / sr;
                out[i] = (float) (std::sin (twoPi * freq * t) * std::exp (-t * 140.0) * 0.6);
            }
            break;
        }
    }

    // Pequeño fundido al final para que no haga "clic" al cortarse
    const int fade = juce::jmin (numSamples, (int) (0.005 * sr));
    buffer.applyGainRamp (numSamples - fade, fade, 1.0f, 0.0f);
    return buffer;
}
}
