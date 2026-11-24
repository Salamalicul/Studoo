#pragma once
// ============================================================================
//  PianoRoll: editor de notas. Vertical = altura de la nota, horizontal = tiempo.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"
#include <map>

namespace studoo
{
    class PianoRoll;

    class PianoRollGrid : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        PianoRollGrid (AppContext&, PianoRoll&);

        static constexpr int lowestNote = 24;  // Do1
        static constexpr int highestNote = 107; // Si7
        static constexpr int rowHeight = 14;
        static constexpr int keyboardWidth = 70;

        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseMove (const juce::MouseEvent&) override;

        void setPlayhead (double steps);
        bool isSelected (const juce::ValueTree& n) const;
        int yForNote (int note) const { return (highestNote - note) * rowHeight; }

        double xToSteps (float x) const;
        float stepsToX (double steps) const;

    private:
        double stepWidth() const;
        int yToNote (float y) const;
        juce::ValueTree noteAt (juce::Point<float>, bool& onRightEdge);
        double snap (double steps) const;

        AppContext& ctx;
        PianoRoll& roll;
        juce::ValueTree dragNote;
        enum class DragMode { none, move, resize, select } dragMode = DragMode::none;
        juce::Rectangle<float> selectionBox;
        struct Original { juce::ValueTree note; double start; int pitch; };
        std::vector<Original> movingGroup;   // notas seleccionadas que se mueven juntas
        double dragStartSteps = 0.0, dragNoteStart = 0.0, dragNoteLength = 0.0;
        int dragStartNote = 0, dragNotePitch = 0;
        int keyboardNote = -1;   // tecla del piano pulsada (para grabar)
        double lastLength = 1.0;
        double playhead = -1.0;
    };

    // Debajo del piano roll: una barra por nota. Arrástrala para cambiar lo fuerte que suena (velocity).
    class VelocityLane : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        VelocityLane (AppContext&, PianoRoll&);
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;

    private:
        void setVelocityAt (juce::Point<float>);
        AppContext& ctx;
        PianoRoll& roll;
    };

    class PianoRoll : public juce::Component,
                      private juce::ChangeListener,
                      private juce::Timer
    {
    public:
        explicit PianoRoll (AppContext&);
        ~PianoRoll() override;

        int getChannelUid() const;
        juce::String getChannelType() const;
        juce::ValueTree getPattern();
        double getSnap() const;
        juce::Array<int> getChordIntervals() const;

        void paint (juce::Graphics&) override;
        void resized() override;
        void visibilityChanged() override;

        juce::Component& getGridComponent() { return grid; }
        juce::Component& getChannelSelector() { return channelBox; }
        float gridStepsToX (double steps) const;   // posición x (en el carril de velocity)
        double gridXToSteps (float x) const;
        void quantize();

        // --- Selección y herramientas ----------------------------------------
        juce::Array<juce::ValueTree> selection;
        bool selectMode = false;              // botón "Seleccionar" (para pantallas táctiles)
        void selectAll();
        void clearSelection();
        void copySelection();
        void cutSelection();
        void paste();
        void duplicateSelection();
        void deleteSelection();
        void transpose (int semitones);
        void arpeggiate (int style);          // 0 sube, 1 baja, 2 sube y baja, 3 al azar
        juce::Array<juce::ValueTree> selectedOrAll();
        bool handleKey (const juce::KeyPress&);

        // --- Grabar notas tocando (teclado MIDI o las teclas del piano de la izquierda) ---
        void setRecording (bool);
        bool isRecordingNotes() const { return recording; }
        void liveNote (int pitch, bool on, float velocity);   // desde las teclas en pantalla

        // Escala: 0 = sin escala
        int getScaleType() const;
        int getScaleRoot() const;
        bool isInScale (int pitch) const;
        int snapToScale (int pitch) const;    // la nota de la escala más cercana (si está activado)
        static juce::StringArray scaleNames();

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void timerCallback() override;
        void refreshChannels();
        void scrollToNotes();
        void showToolsMenu();
        void exportMidi();
        void importMidi();
        std::unique_ptr<juce::FileChooser> chooser;
        void pruneSelection();
        void handleRecorded (const AudioEngine::RecordedMidi&);
        bool recording = false;
        std::map<int, std::pair<double, float>> openNotes;   // nota -> (inicio, velocity)

        AppContext& ctx;
        HelpButton helpButton;
        juce::ComboBox channelBox, snapBox, chordBox;
        juce::Label patternLabel;
        juce::TextButton clearButton, quantizeButton, toolsButton, selectButton, recordButton;
        juce::Viewport viewport;
        PianoRollGrid grid;
        VelocityLane velocityLane;
        juce::Rectangle<int> headerArea;
        juce::Array<int> channelUids;
        int lastChannel = -1;
    };
}
