#pragma once
// ============================================================================
//  Kits: los sonidos que vienen incluidos en Studoo.
//
//  Todos los sonidos se FABRICAN con síntesis (matemáticas), como en las cajas
//  de ritmos clásicas (Roland 808 / 909). Por eso no ocupan espacio y no tienen
//  problemas de derechos de autor: son 100% tuyos para usarlos en tu música.
//
//  Cada sonido tiene una ruta del tipo "builtin:trap/kick"
//  (kit "trap", sonido "kick").
//
//  Aquí también están los sonidos de sintetizador ya preparados (presets)
//  y los distintos tipos de 808.
// ============================================================================
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace studoo::kits
{
    struct SoundInfo
    {
        const char* id;   // papel del sonido: kick, snare, clap, hihat, openhat, conga, riser...
        const char* name; // nombre en español (UTF-8)
    };

    struct Bass808Preset
    {
        bool present = false;
        float decay = 1.5f, glide = 0.08f, drive = 0.3f;
    };

    // "Carácter" de un kit: cómo suenan sus bombos, cajas, hi-hats...
    struct Character
    {
        // Bombo: frecuencia inicial y final, rapidez de la caída de tono, duración, golpe, saturación, segundos
        double kF0 = 170, kF1 = 50, kPitchDecay = 30, kAmpDecay = 7, kClick = 0.35, kDrive = 2.2, kLen = 0.55;
        // Caja: tono, caída del tono, caída del ruido, filtro, centro del ruido, cantidad de ruido y de tono, segundos
        double sTone = 210, sToneDecay = 22, sNoiseDecay = 17, sHP = 700, sCentre = 4200, sNoise = 0.8, sToneAmt = 0.6, sLen = 0.3;
        // Palmada
        double cHP = 900, cCentre = 1300, cTail = 14, cGap = 0.011, cLen = 0.4;
        // Platillos: afinación, caída del hi-hat cerrado y abierto, cantidad de ruido
        double hTune = 1.0, hDecay = 55, hOpenDecay = 7, hNoise = 0.3;
        // Percusiones (toms, congas...): afinación, y el sonido "Perc" (tono inicial, final y duración)
        double percTune = 1.0, percF0 = 1100, percF1 = 820, percDecay = 22;
        // Efectos para todo el kit
        double extraDrive = 0.0, room = 0.0, roomSize = 0.5;
        bool lofi = false, gated = false;
    };

    struct KitInfo
    {
        const char* id;
        const char* name;
        const char* description;
        std::vector<SoundInfo> sounds;
        Bass808Preset bass;
        Character character;
        bool isEffects = false; // el kit de efectos (FX) no es una batería
    };

    const std::vector<KitInfo>& all();
    const KitInfo* find (const juce::String& kitId);

    // "builtin:trap/kick"
    juce::String samplePath (const KitInfo&, const SoundInfo&);

    // Papel de un sonido incluido: "builtin:trap/kick" -> "kick", "builtin:hihat" -> "hihat"
    juce::String roleOf (const juce::String& samplePath);

    // Genera el sonido (buffer vacío si no existe)
    juce::AudioBuffer<float> render (const juce::String& kitId, const juce::String& soundId, double sampleRate);

    // ------------------------------------------------------------------------
    //  Sonidos de sintetizador ya preparados
    // ------------------------------------------------------------------------
    struct SynthPreset
    {
        const char* name;
        const char* description;
        int wave; // 0 sierra, 1 cuadrada, 2 seno, 3 triángulo
        float attack, decay, sustain, release, cutoff, resonance, detune;
        int root; // nota en la que suena el canal cuando usas pasos
    };
    const std::vector<SynthPreset>& synthPresets();

    // ------------------------------------------------------------------------
    //  Tipos de 808
    // ------------------------------------------------------------------------
    struct Bass808Named
    {
        const char* name;
        const char* description;
        float decay, glide, drive;
    };
    const std::vector<Bass808Named>& bass808Presets();
}
