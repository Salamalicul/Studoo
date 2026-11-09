#pragma once
// ============================================================================
//  LearnView: el apartado "Aprende" con explicaciones de producción musical.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    class LearnView : public juce::Component, private juce::ListBoxModel
    {
    public:
        LearnView();

        std::function<void()> onStartTutorial;
        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        int getNumRows() override;
        void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
        void selectedRowsChanged (int lastRowSelected) override;

        HelpButton helpButton;
        juce::TextButton tutorialButton;
        juce::ListBox list { "temas", this };
        juce::TextEditor text;
        juce::Rectangle<int> headerArea;
    };
}
