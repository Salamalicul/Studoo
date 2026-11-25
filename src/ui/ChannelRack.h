#pragma once
// ============================================================================
//  ChannelRack: el secuenciador de patrones (en FL Studio, "Channel Rack").
//  Cada fila es un canal (instrumento) y cada cuadrado un paso.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "InstrumentPanel.h"
#include "Widgets.h"

namespace studoo
{
    class ChannelRack;

    // La cuadrícula de pasos de todos los canales
    class StepGrid : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        StepGrid (AppContext&, ChannelRack&);

        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;

        void setPlayingStep (int step);
        juce::Rectangle<float> getCellBounds (int row, int step) const;

    private:
        bool cellAt (juce::Point<float>, int& row, int& step) const;
        void applyPaint (const juce::MouseEvent&);

        AppContext& ctx;
        ChannelRack& rack;
        int playingStep = -1;
        bool paintValue = true;
        int paintRow = -1;
    };

    // Editor de pasos: una barra por paso del canal elegido (fuerza o tono de cada golpe)
    class StepGraph : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        StepGraph (AppContext&, ChannelRack&, StepGrid&);
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void resized() override;
        static constexpr int labelWidth = 300;   // igual que la columna de controles del secuenciador

    private:
        void setAt (juce::Point<float>);
        bool pitchMode() const { return modeBox.getSelectedId() == 2; }
        AppContext& ctx;
        ChannelRack& rack;
        StepGrid& grid;
        juce::ComboBox modeBox;
    };

    class ChannelRack : public juce::Component,
                        public juce::FileDragAndDropTarget,
                        private juce::ChangeListener,
                        private juce::Timer
    {
    public:
        explicit ChannelRack (AppContext&);
        ~ChannelRack() override;

        static constexpr int rowHeight = 44;

        // Datos que usa la cuadrícula
        int getNumRows() const { return rows.size(); }
        int getRowUid (int row) const;
        juce::ValueTree getPattern();
        int getPatternLength();
        // Los pasos se ven en páginas de 4 compases (64 pasos) cuando el patrón es más largo
        static constexpr int stepsPerPage = 64;
        int getPageStart();
        int getVisibleSteps();
        int page = 0;
        

        void paint (juce::Graphics&) override;
        void resized() override;

        bool isInterestedInFileDrag (const juce::StringArray&) override;
        void filesDropped (const juce::StringArray&, int x, int y) override;

        void loadSampleInto (juce::ValueTree channel);
        void showAddMenu();

        juce::Component& getGridComponent()       { return grid; }
        juce::Component& getAddButton()           { return addButton; }
        juce::Component& getExampleButton()       { return exampleButton; }
        juce::Component& getPatternSelector()     { return patternBox; }
        juce::Component* getRowNameButton (int row);

    private:
        struct Row;
        class NameButton;
        class AddRow;

        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void timerCallback() override;
        void rebuildRows();
        void refreshHeader();
        void showChannelMenu (int uid);
        void showPatternMenu();
        void addSampleChannels (const juce::Array<juce::File>&);
        int findFreeMixerTrack();

        AppContext& ctx;
        HelpButton helpButton;
        juce::ComboBox patternBox, lengthBox, swingBox, pageBox;
        juce::TextButton patternMenuButton, addButton, exampleButton, clearButton, soundButton, graphButton, padsButton;
        std::unique_ptr<StepGraph> stepGraph;
        bool panelOpen = false; // modo móvil: panel del sonido abierto

        juce::Viewport viewport;
        CallbackComponent content;
        StepGrid grid;
        juce::OwnedArray<Row> rows;
        std::unique_ptr<AddRow> addRow;
        InstrumentPanel instrumentPanel;

        juce::String structureKey;
        juce::uint32 lastGeneration = 0;
        int lastSelected = -1;
        juce::Rectangle<int> headerArea;
        std::unique_ptr<juce::FileChooser> chooser;
    };
}
