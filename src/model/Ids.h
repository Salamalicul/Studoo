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
    STUDOO_ID (ZONE)              // multisample: un sample del instrumento (sample, root, lo, hi)
    STUDOO_ID (lo)                // multisample: nota más grave que usa este sample
    STUDOO_ID (hi)                // multisample: nota más aguda
    STUDOO_ID (MIDILINKS)         // controles MIDI conectados a ruedas de Studoo
    STUDOO_ID (LINK)
    STUDOO_ID (cc)                // número de control MIDI (0..127)
    STUDOO_ID (param)             // nombre de la propiedad que mueve
    STUDOO_ID (minValue)
    STUDOO_ID (maxValue)
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
    STUDOO_ID (timbre)            // sinte FM / tabla de ondas: brillo del sonido 0..1
    STUDOO_ID (fmRatio)           // sinte FM: relación del modulador (1..8)
    STUDOO_ID (clipMuted)         // clip: en silencio (para quedarte con otra toma)
    STUDOO_ID (slicePoints)       // sampler: dónde empieza cada trozo, "0,0.12,0.31..." (golpes detectados)
    STUDOO_ID (loopOn)            // proyecto: repetir una zona de la canción (para grabar tomas)
    STUDOO_ID (loopStart)         // proyecto: inicio de la zona (pasos)
    STUDOO_ID (loopEnd)           // proyecto: final de la zona (pasos)
    STUDOO_ID (timeSigNum)        // proyecto: compás, tiempos (2..7)
    STUDOO_ID (timeSigDen)        // proyecto: compás, figura (4 = negra, 8 = corchea)
    STUDOO_ID (gateThreshold)     // pista: puerta de ruido, dB (-80 = apagada)
    STUDOO_ID (tuneAmount)        // pista: autotune 0..1 (0 = apagado, 1 = robot)
    STUDOO_ID (tuneRoot)          // pista: nota base de la escala del autotune (0..11)
    STUDOO_ID (tuneScale)         // pista: escala del autotune (0 = todas las notas)
    STUDOO_ID (rhythmMode)        // pista: efecto de ritmo (0 apagado, 1 half-time, 2-3 tartamudeo, 4-5 puerta, 6 parada de cinta)
    STUDOO_ID (rhythmMix)         // pista: mezcla del efecto de ritmo 0..1
    STUDOO_ID (maximize)          // pista: maximizador, dB de ganancia antes del limitador (0..12)
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

    // Studoo 2.2
    STUDOO_ID (flangerMix)        // pista: flanger 0..1
    STUDOO_ID (phaserMix)         // pista: phaser 0..1
    STUDOO_ID (modRate)           // pista: velocidad del flanger/phaser (Hz)
    STUDOO_ID (stereoWidth)       // pista: 0 = mono, 1 = normal, 2 = muy ancho
    STUDOO_ID (transAttack)       // pista: golpes -1 (suaves) .. 1 (fuertes)
    STUDOO_ID (transSustain)      // pista: cola -1 (corta) .. 1 (larga)
    STUDOO_ID (multiband)         // pista: compresor multibanda 0..1
    STUDOO_ID (vocoderMix)        // pista: vocoder 0..1 (esta pista = sonido; la fuente = la voz)
    STUDOO_ID (vocoderSource)     // pista: de qué pista sale la voz del vocoder
    STUDOO_ID (peakTarget)        // pista: controlador de picos, a qué pista mueve (0 = ninguna)
    STUDOO_ID (peakParam)         // pista: qué control mueve (0 volumen, 1 filtro, 2 panorama, 3 reverb, 4 eco, 5 distorsión)
    STUDOO_ID (peakAmount)        // pista: cuánto y hacia dónde (-1..1)
    STUDOO_ID (MARKERS)           // marcadores de la canción
    STUDOO_ID (MARKER)
    STUDOO_ID (notePan)           // nota: panorama -1..1
    STUDOO_ID (noteFine)          // nota: afinación fina en cents (-100..100)
    STUDOO_ID (noteMod)           // nota: modulación 0..1 (abre el filtro del sinte)
    STUDOO_ID (slide)             // nota: se desliza desde la nota anterior
    STUDOO_ID (ghostNotes)        // proyecto: ver las notas de otros canales en el piano roll
    STUDOO_ID (group)             // pista de la playlist: nombre del grupo
    STUDOO_ID (TRACKS)            // ajustes de las pistas de la playlist
    STUDOO_ID (collapsed)         // grupo plegado
    STUDOO_ID (layerOf)           // canal: lista de uids que suenan juntos ("4,7,9")
    STUDOO_ID (crossfade)         // clip de audio: fundido cruzado automático con el clip anterior
    STUDOO_ID (launchQuant)       // modo directo: los clips empiezan en el siguiente compás

#undef STUDOO_ID
}
