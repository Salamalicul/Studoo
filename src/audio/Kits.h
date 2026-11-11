#pragma once
// ============================================================================
//  Kits: los kits de batería que vienen incluidos en Studoo.
//
//  Todos los sonidos se FABRICAN con síntesis (matemáticas), como en las cajas
//  de ritmos clásicas (Roland 808 / 909). Por eso no ocupan espacio y no tienen
//  problemas de derechos de autor: son 100% tuyos para usarlos en tu música.
//
//  Cada sonido tiene una ruta del tipo "builtin:trap/kick"
//  (kit "trap", sonido "kick").
// ============================================================================
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace studoo::kits
{
    struct SoundInfo
    {
        const char* id;   // papel del sonido: kick, snare, clap, hihat, openhat...
        const char* name; // nombre en español (UTF-8)
    };

    struct Bass808Preset
    {
        bool present = false;
        float decay = 1.5f, glide = 0.08f, drive = 0.3f;
    };

    struct KitInfo
    {
        const char* id;
        const char* name;
        const char* description;
        std::vector<SoundInfo> sounds;
        Bass808Preset bass;
    };

    const std::vector<KitInfo>& all();
    const KitInfo* find (const juce::String& kitId);

    // "builtin:trap/kick"
    juce::String samplePath (const KitInfo&, const SoundInfo&);

    // Papel de un sonido incluido: "builtin:trap/kick" -> "kick", "builtin:hihat" -> "hihat"
    juce::String roleOf (const juce::String& samplePath);

    // Genera el sonido (buffer vacío si no existe)
    juce::AudioBuffer<float> render (const juce::String& kitId, const juce::String& soundId, double sampleRate);
}
