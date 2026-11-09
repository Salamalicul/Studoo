#pragma once
// ============================================================================
//  AutomationView: dibuja curvas que mueven un control a lo largo de la canción.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    class AutomationView;

    class AutomationEditor : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        AutomationEditor (AppContext&, AutomationView&);

        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;

    private:
        static constexpr int ruler = 22;
        double totalSteps();
        float timeToX (double t);
        double xToTime (float x);
        float valueToY (float v) const;
        float yToValue (float y) const;
        juce::ValueTree pointAt (juce::Point<float>);

        AppContext& ctx;
        AutomationView& view;
        juce::ValueTree dragPoint;
    };

    class AutomationView : public juce::Component,
                           private juce::ChangeListener,
                           private juce::Timer
    {
    public:
        explicit AutomationView (AppContext&);
        ~AutomationView() override;

        juce::ValueTree getLane();

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void timerCallback() override;
        void refresh();
        void showNewLaneMenu();

        AppContext& ctx;
        HelpButton helpButton;
        juce::ComboBox laneBox;
        juce::TextButton newButton, deleteButton;
        juce::Label infoLabel;
        AutomationEditor editor;
        juce::Rectangle<int> headerArea;
    };
}
