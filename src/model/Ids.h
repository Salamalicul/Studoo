#pragma once
// ============================================================================
//  Ids: los nombres de todas las "etiquetas" que usa el proyecto.
//  Un proyecto de Studoo es un árbol de datos (juce::ValueTree), como las
//  carpetas de tu ordenador: PROYECTO > CANALES > CANAL, etc.
// ============================================================================
#include <juce_data_structures/juce_data_structures.h>

namespace studoo::ids
{
#define STUDOO_ID(name) inline const juce::Identifier name { #name };

    // Tipos de nodo
    STUDOO_ID (PROJECT)
    STUDOO_ID (CHANNELS)
    STUDOO_ID (CHANNEL)
    STUDOO_ID (PATTERNS)
    STUDOO_ID (PATTERN)
    STUDOO_ID (STEPSET)
    STUDOO_ID (NOTE)
    STUDOO_ID (PLAYLIST)
    STUDOO_ID (CLIP)
    STUDOO_ID (MIXER)
    STUDOO_ID (TRACK)
    STUDOO_ID (AUTOMATION)
    STUDOO_ID (LANE)
    STUDOO_ID (POINT)

    // Proyecto
    STUDOO_ID (version)
    STUDOO_ID (bpm)
    STUDOO_ID (metronome)
    STUDOO_ID (playMode)
    STUDOO_ID (nextUid)
    STUDOO_ID (currentPattern)
    STUDOO_ID (selectedChannel)
    STUDOO_ID (selectedLane)

    // Comunes
    STUDOO_ID (uid)
    STUDOO_ID (name)
    STUDOO_ID (colour)

    // Canal
    STUDOO_ID (type)
    STUDOO_ID (volume)
    STUDOO_ID (pan)
    STUDOO_ID (muted)
    STUDOO_ID (mixer)
    STUDOO_ID (sample)
    STUDOO_ID (root)
    STUDOO_ID (oneShot)
    STUDOO_ID (wave)
    STUDOO_ID (attack)
    STUDOO_ID (decay)
    STUDOO_ID (sustain)
    STUDOO_ID (release)
    STUDOO_ID (cutoff)
    STUDOO_ID (resonance)
    STUDOO_ID (detune)
    STUDOO_ID (glide)
    STUDOO_ID (drive)
    STUDOO_ID (pluginDesc)
    STUDOO_ID (pluginState)

    // Patrón
    STUDOO_ID (length)
    STUDOO_ID (channel)
    STUDOO_ID (data)
    STUDOO_ID (start)
    STUDOO_ID (pitch)
    STUDOO_ID (velocity)

    // Playlist
    STUDOO_ID (kind)
    STUDOO_ID (ref)
    STUDOO_ID (track)
    STUDOO_ID (offset)

    // Mezclador
    STUDOO_ID (index)
    STUDOO_ID (solo)
    STUDOO_ID (eqLow)
    STUDOO_ID (eqMid)
    STUDOO_ID (eqHigh)
    STUDOO_ID (compOn)
    STUDOO_ID (compThreshold)
    STUDOO_ID (compRatio)
    STUDOO_ID (delayMix)
    STUDOO_ID (delayTime)
    STUDOO_ID (delayFeedback)
    STUDOO_ID (reverbMix)
    STUDOO_ID (reverbSize)

    // Automatización
    STUDOO_ID (target)
    STUDOO_ID (time)
    STUDOO_ID (value)

#undef STUDOO_ID
}
