#pragma once
// ============================================================================
//  Challenges: los retos de "Aprende". Cada reto dice qué hacer en Studoo y
//  tiene una comprobación que mira el proyecto para saber si ya lo hiciste.
//  En una lección se ponen con  [reto:id]
// ============================================================================
#include "AppContext.h"

namespace studoo::learn
{
    struct Challenge
    {
        const char* id;
        const char* title;        // "Four on the floor"
        const char* instruction;  // qué hay que hacer
        const char* hint;         // pista si no te sale
        View view;                // dónde se hace
        std::function<bool (Project&)> check;
    };

    const std::vector<Challenge>& challenges();
    const Challenge* findChallenge (const juce::String& id);
}
