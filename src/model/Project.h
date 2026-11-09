#pragma once
// ============================================================================
//  Project: todos los datos de tu canción.
//
//  - Canales (instrumentos): bombo, caja, 808, sintetizador, samples, plugins...
//  - Patrones: los pasos del secuenciador y las notas del piano roll.
//  - Playlist: dónde se coloca cada patrón a lo largo de la canción.
//  - Mezclador: volumen, panorama y efectos de cada pista.
//  - Automatización: cambios de un control a lo largo del tiempo.
//
//  Todo se guarda en un árbol (juce::ValueTree). Gracias a eso:
//    * Guardar/abrir es convertir el árbol a un archivo y al revés.
//    * Deshacer/rehacer lo hace el "UndoManager", que recuerda cada cambio.
// ============================================================================
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include "Ids.h"

namespace studoo
{
    enum class ChannelType { sampler, synth, bass808, plugin, audio };

    juce::String channelTypeToString (ChannelType);
    ChannelType  channelTypeFromString (const juce::String&);

    constexpr int numMixerTracks    = 9;  // 0 = Master, 1..8 = pistas
    constexpr int numPlaylistTracks = 16;
    constexpr int stepsPerBar       = 16; // 16 semicorcheas = 1 compás
    constexpr int stepsPerBeat      = 4;

    // ------------------------------------------------------------------------
    //  Destino de una automatización, por ejemplo "mixer:3:volume"
    // ------------------------------------------------------------------------
    struct AutomationTarget
    {
        enum class Kind { mixer, channel, invalid };
        enum class Param { volume, pan, reverbMix, delayMix, cutoff, eqLow, eqHigh, invalid };

        Kind kind = Kind::invalid;
        int index = 0;  // número de pista del mezclador o uid del canal
        Param param = Param::invalid;

        static AutomationTarget parse (const juce::String&);
        juce::String toString() const;
        bool isValid() const { return kind != Kind::invalid && param != Param::invalid; }

        static juce::String paramName (Param);
        static juce::Identifier paramId (Param);

        // Convierte un valor 0..1 de la curva al valor real del parámetro y al revés
        static float fromNormalised (Param, float normalised);
        static float toNormalised (Param, float realValue);
    };

    class Project : public juce::ChangeBroadcaster,
                    private juce::ValueTree::Listener
    {
    public:
        Project();
        ~Project() override;

        juce::ValueTree& getState()        { return state; }
        juce::UndoManager& getUndo()       { return undoManager; }
        juce::UndoManager* um()            { return &undoManager; }
        void beginTransaction()            { undoManager.beginNewTransaction(); }

        // --- Proyecto completo ----------------------------------------------
        void resetToDefault();
        bool loadFromFile (const juce::File&, juce::String& error);
        bool saveToFile (const juce::File&);
        bool loadFromXmlString (const juce::String& xml, juce::String& error);
        juce::String toXmlString() const;

        juce::File getFile() const          { return file; }
        void setFile (const juce::File& f)  { file = f; }
        bool hasUnsavedChanges() const      { return dirty; }
        void markSaved()                    { dirty = false; }
        juce::uint32 getVersion() const     { return version; }
        // Cambia cada vez que se carga un proyecto nuevo (las vistas deben reconectarse)
        juce::uint32 getGeneration() const  { return generation; }
        juce::String getDisplayName() const;

        // --- Secciones del árbol ---------------------------------------------
        juce::ValueTree channels()   { return state.getChildWithName (ids::CHANNELS); }
        juce::ValueTree patterns()   { return state.getChildWithName (ids::PATTERNS); }
        juce::ValueTree playlist()   { return state.getChildWithName (ids::PLAYLIST); }
        juce::ValueTree mixer()      { return state.getChildWithName (ids::MIXER); }
        juce::ValueTree automation() { return state.getChildWithName (ids::AUTOMATION); }

        int newUid();

        // --- Canales -----------------------------------------------------------
        juce::ValueTree addChannel (ChannelType, const juce::String& name, int mixerTrack = 0);
        void removeChannel (int uid);
        juce::ValueTree findChannel (int uid);
        int getChannelIndex (int uid);
        juce::ValueTree findChannelByBuiltin (const juce::String& builtinName);
        juce::ValueTree findChannelByType (ChannelType);

        int  getSelectedChannel();
        void setSelectedChannel (int uid);

        // --- Patrones ----------------------------------------------------------
        juce::ValueTree addPattern (const juce::String& name = {});
        juce::ValueTree duplicatePattern (int uid);
        void removePattern (int uid);
        juce::ValueTree findPattern (int uid);
        juce::ValueTree getCurrentPattern();
        int  getCurrentPatternUid();
        void setCurrentPattern (int uid);

        static int getPatternLength (const juce::ValueTree& pattern);
        bool getStep (const juce::ValueTree& pattern, int channelUid, int step);
        void setStep (juce::ValueTree pattern, int channelUid, int step, bool on);
        void clearPattern (juce::ValueTree pattern);
        void loadExampleBeat (juce::ValueTree pattern);

        juce::ValueTree addNote (juce::ValueTree pattern, int channelUid, double start,
                                 double length, int pitch, float velocity = 0.8f);
        static bool patternHasNotes (const juce::ValueTree& pattern, int channelUid);

        // --- Playlist ------------------------------------------------------------
        juce::ValueTree addClip (const juce::String& kind, int ref, int track,
                                 double start, double length, double offset = 0.0);
        double getSongLengthSteps();

        // --- Mezclador -----------------------------------------------------------
        juce::ValueTree getMixerTrack (int index);

        // --- Automatización ------------------------------------------------------
        juce::ValueTree addAutomationLane (const AutomationTarget&);
        juce::String describeTarget (const AutomationTarget&);

    private:
        void setState (juce::ValueTree newState);
        void ensureStructure();
        static juce::ValueTree createMixerTrack (int index, const juce::String& name);
        juce::ValueTree getStepSet (juce::ValueTree pattern, int channelUid, bool create);

        void changed (bool affectsSavedData);
        void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;
        void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) override       { changed (true); }
        void valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree&, int) override { changed (true); }
        void valueTreeChildOrderChanged (juce::ValueTree&, int, int) override        { changed (true); }

        juce::ValueTree state;
        juce::UndoManager undoManager;
        juce::File file;
        bool dirty = false;
        juce::uint32 version = 0, generation = 0;
    };
}
