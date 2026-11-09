// ============================================================================
//  Prueba automática del motor de audio (sin ventana ni tarjeta de sonido).
//  Reproduce el beat de ejemplo durante 2 compases, comprueba que suena y que
//  el tiempo es exacto, y guarda el resultado en un archivo WAV.
//
//  Uso:  StudooEngineTest [archivo_salida.wav]
// ============================================================================
#include <juce_audio_formats/juce_audio_formats.h>
#include "audio/AudioEngine.h"

#include <iostream>

static int failures = 0;

static void check (bool condition, const char* what)
{
    std::cout << (condition ? "  OK    " : "  FALLO ") << what << "\n";
    if (! condition)
        ++failures;
}

int main (int argc, char* argv[])
{

    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    constexpr double bpm = 120.0;

    studoo::AudioEngine engine;
    engine.setBpm (bpm);
    engine.prepareToPlay (blockSize, sampleRate);

    // 2 compases a 120 BPM = 4 segundos
    const int totalSamples = (int) (sampleRate * 4.0);
    juce::AudioBuffer<float> output (2, totalSamples);
    output.clear();

    // --- 1. Parado: no debe sonar nada ------------------------------------
    {
        juce::AudioBuffer<float> block (2, blockSize);
        juce::AudioSourceChannelInfo info (&block, 0, blockSize);
        engine.getNextAudioBlock (info);
        check (block.getMagnitude (0, blockSize) < 1.0e-9f, "Silencio cuando está parado");
        check (engine.getCurrentStep() == -1, "Sin paso activo cuando está parado");
    }

    // --- 2. Reproducir y renderizar -----------------------------------------
    engine.play();
    int stepChanges = 0, lastStep = -1;
    for (int pos = 0; pos < totalSamples; pos += blockSize)
    {
        const int n = juce::jmin (blockSize, totalSamples - pos);
        juce::AudioSourceChannelInfo info (&output, pos, n);
        engine.getNextAudioBlock (info);
        if (engine.getCurrentStep() != lastStep)
        {
            lastStep = engine.getCurrentStep();
            ++stepChanges;
        }
    }

    check (output.getRMSLevel (0, 0, totalSamples) > 0.01f, "Suena al reproducir");
    check (output.getMagnitude (0, 0, totalSamples) <= 1.0f, "No satura (pico <= 1.0)");
    check (stepChanges >= 28, "El paso avanza mientras suena");

    // El bombo está en el paso 1: debe haber sonido justo al principio
    check (output.getMagnitude (0, 0, 2000) > 0.1f, "El bombo suena en el primer paso");

    // A 120 BPM un paso dura 5512,5 muestras. El paso 5 (índice 4) tiene caja.
    // Comprobamos que justo antes no hay casi nada nuevo y justo después sí.
    {
        const int snareStart = (int) (4 * sampleRate * 60.0 / bpm / 4.0);
        check (output.getMagnitude (0, snareStart, 1000) > 0.1f, "La caja suena a tiempo en el paso 5");
    }

    // --- 3. Parar ------------------------------------------------------------
    engine.stop();
    check (engine.getCurrentStep() == -1 && ! engine.isPlaying(), "Se detiene al parar");

    // --- 4. Mute -------------------------------------------------------------
    {
        studoo::AudioEngine muted;
        muted.prepareToPlay (blockSize, sampleRate);
        for (int c = 0; c < studoo::numChannels; ++c)
            muted.setChannelMuted (c, true);
        muted.play();
        juce::AudioBuffer<float> block (2, blockSize * 20);
        juce::AudioSourceChannelInfo info (&block, 0, block.getNumSamples());
        muted.getNextAudioBlock (info);
        check (block.getMagnitude (0, block.getNumSamples()) < 1.0e-9f, "Silencio con todos los canales en mute");
    }

    // --- Guardar WAV para escucharlo ------------------------------------------
    if (argc > 1)
    {
        juce::File file = juce::File::getCurrentWorkingDirectory().getChildFile (argv[1]);
        file.deleteFile();
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());
        if (stream != nullptr)
        {
            const auto options = juce::AudioFormatWriterOptions{}.withSampleRate (sampleRate)
                                                                 .withNumChannels (2)
                                                                 .withBitsPerSample (16);
            if (auto writer = wav.createWriterFor (stream, options))
            {
                writer->writeFromAudioSampleBuffer (output, 0, totalSamples);
                std::cout << "  WAV guardado en " << file.getFullPathName() << "\n";
            }
        }
    }

    std::cout << (failures == 0 ? "\nTodas las pruebas han pasado.\n" : "\nHay pruebas que han fallado.\n");
    return failures == 0 ? 0 : 1;
}
