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

    // Studoo 2.0
    STUDOO_ID (swing)             // proyecto: 0..1
    STUDOO_ID (sampleTune)        // sampler: semitonos -24..24
    STUDOO_ID (sampleStart)       // sampler: 0..1
    STUDOO_ID (sampleEnd)         // sampler: 0..1
    STUDOO_ID (reverse)           // sampler: al revés
    STUDOO_ID (slices)            // sampler: 1..16 trozos (chops)
    STUDOO_ID (lfoRate)           // sinte: Hz
    STUDOO_ID (lfoAmount)         // sinte: 0..1 (mueve el filtro)
    STUDOO_ID (trackFilter)       // pista: -1 (paso bajo) .. 0 .. 1 (paso alto)
    STUDOO_ID (distortion)        // pista: 0..1
    STUDOO_ID (chorusMix)         // pista: 0..1
    STUDOO_ID (sidechain)         // pista: 0..1 (cuánto se agacha)
    STUDOO_ID (sidechainSource)   // pista: de qué pista escucha (1..)
    STUDOO_ID (sendTo)            // pista: a qué pista manda una copia (0 = a ninguna; solo a pistas de número mayor)
    STUDOO_ID (sendLevel)         // pista: 0..1, cuánto manda
    STUDOO_ID (scaleRoot)         // proyecto: nota base de la escala (0 = Do ... 11 = Si)
    STUDOO_ID (scaleType)         // proyecto: 0 = sin escala, 1 mayor, 2 menor...
    STUDOO_ID (scaleLock)         // proyecto: las notas nuevas se pegan a la escala
    STUDOO_ID (stepVel)           // pasos: fuerza de cada paso, "85,40,100..." (0..100)
    STUDOO_ID (stepPitch)         // pasos: tono de cada paso en semitonos, "0,0,12..." (-24..24)
    STUDOO_ID (mono)              // sinte: una sola nota a la vez (con glide si se solapan)
    STUDOO_ID (recordBpm)         // grabación/clip: el BPM con el que se grabó (para seguir el tempo)
    STUDOO_ID (stretch)           // clip de audio: sigue el tempo de la canción sin cambiar el tono
    STUDOO_ID (clipPitch)         // clip de audio: tono en semitonos (-12..12) sin cambiar la velocidad
    STUDOO_ID (clipGain)          // clip de audio: volumen 0..2
    STUDOO_ID (fadeIn)            // clip de audio: fundido de entrada (en pasos)
    STUDOO_ID (fadeOut)           // clip de audio: fundido de salida (en pasos)
    STUDOO_ID (sendOnly)          // pista: true = solo suena a través del envío (no va directa al Master)
    STUDOO_ID (limiter)           // pista: limitador encendido

#undef STUDOO_ID
}
