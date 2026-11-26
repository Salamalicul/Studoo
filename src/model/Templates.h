#pragma once
// ============================================================================
//  Templates: plantillas para empezar una canción ya preparada.
//  Cada una pone el tempo, la base del género, la estructura de la canción
//  (intro, estribillo...) con marcadores, nombres de pistas y una mezcla
//  inicial (voz con reverb, sidechain del bajo, Master con maximizador).
// ============================================================================
#include "Project.h"

namespace studoo::templates
{
    struct TemplateInfo
    {
        const char* id;
        const char* name;
        const char* description;
        const char* genre;   // base de género que usa ("" = ninguna)
    };

    const std::vector<TemplateInfo>& all();

    // Cambia el proyecto por la plantilla (el proyecto anterior se pierde: pregunta antes de guardar)
    bool apply (Project&, const juce::String& id);
}
