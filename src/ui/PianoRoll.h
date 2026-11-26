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
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
        float keyboardLeft() const;   // el teclado se queda fijo a la izquierda aunque te desplaces

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
    public:
        // Qué se edita en el carril: 0 fuerza, 1 panorama, 2 afinación fina, 3 modulación
        int mode = 0;
        void showModeMenu();
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
        void generateIdea (int kind);         // 0 melodía, 1 bajo, 2 acordes, 3 arpegio (en la escala)
        void humanize (bool timing, bool velocity);
        void glueNotes();                     // une las notas seguidas de la misma altura
        void splitNotes (int parts);          // 0 = en trozos del ajuste
        void toggleSlide();                   // las seleccionadas se deslizan desde la nota anterior
        bool slideMode = false;               // botón "Slide": las notas nuevas se deslizan
        bool showGhosts() const;
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
        juce::TextButton clearButton, quantizeButton, toolsButton, selectButton, recordButton, slideButton;
        struct ScrollViewport : juce::Viewport
        {
            std::function<void()> onScroll;
            void visibleAreaChanged (const juce::Rectangle<int>&) override { if (onScroll) onScroll(); }
        };
        ScrollViewport viewport;
        juce::TextButton zoomOutButton, zoomInButton;
        float zoom = 1.0f;   // 1 = 4 compases en la pantalla
        void updateGridSize();
    public:
        void setZoom (float newZoom, float anchorX = -1.0f);
        float getZoom() const { return zoom; }
        int viewX() const { return viewport.getViewPositionX(); }
    private:
        PianoRollGrid grid;
        VelocityLane velocityLane;
        juce::Rectangle<int> headerArea;
        juce::Array<int> channelUids;
        int lastChannel = -1;
    };
}
