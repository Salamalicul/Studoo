#pragma once
// ============================================================================
//  PianoRoll: editor de notas. Vertical = altura de la nota, horizontal = tiempo.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

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
        int yForNote (int note) const { return (highestNote - note) * rowHeight; }

    private:
        double stepWidth() const;
        double xToSteps (float x) const;
        float stepsToX (double steps) const;
        int yToNote (float y) const;
        juce::ValueTree noteAt (juce::Point<float>, bool& onRightEdge);
        double snap (double steps) const;

        AppContext& ctx;
        PianoRoll& roll;
        juce::ValueTree dragNote;
        enum class DragMode { none, move, resize } dragMode = DragMode::none;
        double dragStartSteps = 0.0, dragNoteStart = 0.0, dragNoteLength = 0.0;
        int dragStartNote = 0, dragNotePitch = 0;
        double lastLength = 1.0;
        double playhead = -1.0;
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

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void timerCallback() override;
        void refreshChannels();
        void scrollToNotes();

        AppContext& ctx;
        HelpButton helpButton;
        juce::ComboBox channelBox, snapBox, chordBox;
        juce::Label patternLabel;
        juce::TextButton clearButton;
        juce::Viewport viewport;
        PianoRollGrid grid;
        juce::Rectangle<int> headerArea;
        juce::Array<int> channelUids;
        int lastChannel = -1;
    };
}
