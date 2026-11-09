#pragma once
// ============================================================================
//  Playlist: la línea de tiempo donde colocas patrones para montar la canción.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    class Playlist;

    class PlaylistGrid : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        PlaylistGrid (AppContext&, Playlist&);

        static constexpr int rulerHeight = 24;
        static constexpr int trackHeight = 40;
        static constexpr int labelWidth = 80;

        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseMove (const juce::MouseEvent&) override;
        void mouseDoubleClick (const juce::MouseEvent&) override;

        double getPixelsPerStep() const;
        int getTotalBars();

    private:
        double xToSteps (float x) const;
        float stepsToX (double s) const;
        int yToTrack (float y) const;
        juce::ValueTree clipAt (juce::Point<float>, bool& onRightEdge);
        juce::Rectangle<float> clipBounds (const juce::ValueTree& clip) const;
        double snap (double steps) const;

        AppContext& ctx;
        Playlist& playlist;
        juce::ValueTree dragClip;
        enum class DragMode { none, move, resize, ruler } dragMode = DragMode::none;
        double dragStartSteps = 0.0, clipStart = 0.0, clipLength = 0.0;
        int dragStartTrack = 0, clipTrack = 0;
    };

    class Playlist : public juce::Component,
                     private juce::ChangeListener,
                     private juce::Timer
    {
    public:
        explicit Playlist (AppContext&);
        ~Playlist() override;

        int getPatternToPlace();
        double getSnap() const;
        double getZoom() const { return zoomSlider.getValue(); }

        void paint (juce::Graphics&) override;
        void resized() override;

        juce::Component& getGridComponent()    { return grid; }
        juce::Component& getPatternSelector()  { return patternBox; }

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void timerCallback() override;
        void refresh();
        void layoutGrid();

        AppContext& ctx;
        HelpButton helpButton;
        juce::ComboBox patternBox, snapBox;
        juce::Slider zoomSlider;
        juce::Label infoLabel;
        juce::Viewport viewport;
        PlaylistGrid grid;
        juce::Rectangle<int> headerArea;
    };
}
