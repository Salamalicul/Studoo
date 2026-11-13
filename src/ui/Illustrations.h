#pragma once
// ============================================================================
//  Illustrations: los dibujos animados del apartado "Aprende".
//  Todo se dibuja con código (no hay vídeos ni imágenes): así ocupan muy poco
//  y se ven nítidos a cualquier tamaño.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>

namespace studoo::illustrations
{
    // Dibuja la ilustración 'id' en 'area'. 'time' son los segundos desde que
    // se empezó a mostrar (para animar). Devuelve false si 'id' no existe.
    bool draw (juce::Graphics&, const juce::String& id, juce::Rectangle<float> area, double time);

    // Alto recomendado para cada ilustración, según el ancho disponible
    float preferredHeight (const juce::String& id, float width);
}
