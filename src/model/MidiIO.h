#pragma once
// ============================================================================
//  MidiIO: pasar las notas de un patrón a un archivo MIDI (.mid) y al revés,
//  para llevar tus melodías a otros programas o traerlas a Studoo.
// ============================================================================
#include "Project.h"

namespace studoo::midiio
{
    // Un archivo MIDI con una pista por canal (pasos del secuenciador y notas del piano roll)
    juce::MidiFile patternToMidi (Project&, juce::ValueTree pattern);

    // Mete en el patrón, en el canal indicado, todas las notas del archivo (como mucho 16 compases).
    // Alarga el patrón si hace falta. Devuelve cuántas notas se han añadido.
    int importIntoPattern (Project&, juce::ValueTree pattern, int channelUid, const juce::MidiFile&);

    // "Recuperar lo que tocaste": notas tocadas en directo (segundos) -> un patrón nuevo con ellas.
    // Coge la última frase (hasta que hubo más de 'gapSeconds' sin tocar) y como mucho 'maxSeconds'.
    struct PlayedEvent { double seconds; int note; float velocity; bool on; };
    juce::ValueTree patternFromPlayed (Project&, int channelUid, const std::vector<PlayedEvent>&, double bpm,
                                       double gapSeconds = 8.0, double maxSeconds = 300.0);
}
