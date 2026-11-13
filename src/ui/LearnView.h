#pragma once
// ============================================================================
//  LearnView: el apartado "Aprende". Lecciones ordenadas en niveles, con
//  pasos, cuadros de trucos, ilustraciones animadas, botones para practicar
//  en Studoo, vídeos en YouTube y progreso guardado.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    class LearnArticle;

    class LearnView : public juce::Component, private juce::ListBoxModel
    {
    public:
        LearnView();
        ~LearnView() override;

        std::function<void()> onStartTutorial;
        std::function<void (View)> onGoToView;

        // Dónde se guarda qué lecciones están aprendidas
        void setSettings (juce::PropertiesFile*);

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        friend class LearnArticle;

        struct Row { bool header = false; int index = 0; };

        int getNumRows() override;
        void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
        void selectedRowsChanged (int lastRowSelected) override;

        void showLesson (int lesson);
        int rowForLesson (int lesson) const;
        bool isDone (int lesson) const;
        void setDone (int lesson, bool done);
        int numDone() const;

        HelpButton helpButton;
        juce::TextButton tutorialButton;
        juce::ListBox list { "lecciones", this };
        juce::Viewport viewport;
        std::unique_ptr<LearnArticle> article;
        juce::Array<Row> rows;
        juce::PropertiesFile* settings = nullptr;
        int currentLesson = -1;
        juce::Rectangle<int> headerArea, progressArea;
    };
}
