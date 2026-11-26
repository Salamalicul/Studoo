#pragma once
// ============================================================================
//  Onsets: encuentra dónde empieza cada golpe de un sample (para trocear loops
//  de batería automáticamente, como hacen los programas de música).
// ============================================================================
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace studoo::onsets
{
    // Devuelve las posiciones (0..1 del sample entero) de los golpes entre 'start' y 'end'.
    // La primera siempre es 'start'. Como mucho 'maxSlices'.
    inline std::vector<float> detect (const juce::AudioBuffer<float>& b, double sampleRate, float start, float end, int maxSlices)
    {
        std::vector<float> result;
        const int length = b.getNumSamples();
        if (length < 1024 || sampleRate <= 0.0 || end <= start)
            return result;
        const int hop = 256;
        const int from = (int) (juce::jlimit (0.0f, 1.0f, start) * length), to = (int) (juce::jlimit (0.0f, 1.0f, end) * length);

        // Energía de cada trocito de 256 muestras
        std::vector<float> energy;
        float maxEnergy = 0.0f;
        for (int i = from; i + hop <= to; i += hop)
        {
            float e = 0.0f;
            for (int ch = 0; ch < b.getNumChannels(); ++ch)
            {
                const float* d = b.getReadPointer (ch, i);
                for (int k = 0; k < hop; ++k)
                    e += d[k] * d[k];
            }
            energy.push_back (e);
            maxEnergy = juce::jmax (maxEnergy, e);
        }
        result.push_back (start);
        if (energy.size() < 4 || maxEnergy <= 0.0f)
            return result;

        // Un golpe: la energía sube de golpe (mucho más que la media reciente) y no es muy floja
        const int minGap = juce::jmax (1, (int) (0.08 * sampleRate / hop));   // 80 ms entre golpes
        int last = 0;
        float average = energy[0];
        for (size_t i = 1; i < energy.size() && (int) result.size() < maxSlices; ++i)
        {
            const bool rise = energy[i] > average * 2.5f + maxEnergy * 0.002f && energy[i] > maxEnergy * 0.02f;
            if (rise && (int) i - last >= minGap)
            {
                result.push_back ((float) (from + (int) i * hop) / (float) length);
                last = (int) i;
            }
            average = average * 0.8f + energy[i] * 0.2f;
        }
        return result;
    }
}
