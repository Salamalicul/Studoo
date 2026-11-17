#pragma once
// ============================================================================
//  GenreBeats: bases de ejemplo de cada género (trap, house, reggaetón...).
//  Cada base trae batería de su kit, bajo (808 o sinte) y acordes, para que
//  puedas cargarla, escucharla y cambiarla para aprender cómo funciona.
// ============================================================================
#include "Project.h"

namespace studoo::genres
{
    struct GenreInfo
    {
        const char* id;       // "house"
        const char* name;     // "House"
        double bpm;
    };

    // Todos los géneros con base, en el orden del menú
    const std::vector<GenreInfo>& all();

    // Nombre bonito de un género ("" si no existe)
    juce::String nameOf (const juce::String& id);

    // Rellena el patrón con la base del género: pone el tempo, crea los canales
    // que falten (o reutiliza los que ya existen) y pone pasos y notas.
    bool load (Project&, juce::ValueTree pattern, const juce::String& id);
}
