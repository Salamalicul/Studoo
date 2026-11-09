#pragma once
// ============================================================================
//  LearnView: el apartado "Aprende" con explicaciones de producción musical.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    // Muestra un tema con título grande y texto bien maquetado
    class LearnArticle : public juce::Component
    {
    public:
        void setArticle (const juce::String& title, const juce::String& body, int number);
        void layoutForWidth (int width);
        void paint (juce::Graphics&) override;

    private:
        juce::String title, body;
        int number = 0;
        juce::TextLayout layout;
    };

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
        juce::Viewport viewport;
        LearnArticle article;
        juce::Rectangle<int> headerArea;
    };
}
