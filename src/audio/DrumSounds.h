#pragma once
// ============================================================================
//  DrumSounds: fabrica los sonidos de batería con matemáticas (síntesis).
//
//  Así la app ya suena sin necesitar ningún archivo de audio. En la fase 2
//  podrás cargar tus propios samples WAV/MP3.
// ============================================================================
#include <juce_audio_basics/juce_audio_basics.h>

namespace studoo
{
    enum class DrumSound
    {
        kick,      // Bombo
        snare,     // Caja
        hihat,     // Hi-hat (charles)
        clap,      // Palmada
        clickHigh, // Metrónomo: primer tiempo del compás
        clickLow   // Metrónomo: resto de tiempos
    };

    // Genera el sonido indicado como una lista de muestras (mono).
    juce::AudioBuffer<float> renderDrumSound (DrumSound sound, double sampleRate);
}
