#pragma once
// ============================================================================
//  LearnView: el apartado "Aprende". Cuatro pestañas:
//    - Ruta: lecciones por niveles, con ilustraciones, retos que se
//      comprueban solos, test al final y progreso guardado
//    - Juegos: juegos para entrenar el oído (EarGames)
//    - Logros: racha de días, estadísticas y medallas
//    - Diccionario: buscador de palabras y lecciones
//  En el móvil se ve a pantalla completa (lista -> lección) y con letra grande.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "LearnProgress.h"
#include "Widgets.h"

namespace studoo
{
    class LearnArticle;
    class LessonList;
    class AchievementsPage;
    class DictionaryPage;
    class EarGames;
    class AchievementToast;

    class LearnView : public juce::Component
    {
    public:
        explicit LearnView (AppContext&);
        ~LearnView() override;

        std::function<void()> onStartTutorial;
        std::function<void (View)> onGoToView;
        // Cargar la base de un género ("house", "trap"...) en un patrón nuevo
        std::function<void (const juce::String&)> onLoadGenre;

        // Dónde se guarda el progreso
        void setSettings (juce::PropertiesFile*);

        void paint (juce::Graphics&) override;
        void resized() override;

        // --- Usado por las páginas de Aprende ---------------------------------------
        enum class Tab { path = 0, games, achievements, dictionary };
        void showTab (Tab);
        void showLesson (int lesson, bool openOnPhone = true);
        void backToList();
        bool isDone (int lesson) const;
        void setDone (int lesson, bool done);
        int numDone() const;
        int currentLesson() const { return lessonIndex; }
        learn::Progress& getProgress() { return progress; }
        AppContext& getContext() { return ctx; }
        void playCelebration();

    private:
        void updateLayout();

        AppContext& ctx;
        learn::Progress progress;

        HelpButton helpButton;
        juce::TextButton tutorialButton, backButton;
        juce::OwnedArray<juce::TextButton> tabButtons;

        std::unique_ptr<LessonList> lessonList;
        juce::Viewport listViewport, articleViewport;
        std::unique_ptr<LearnArticle> article;
        std::unique_ptr<EarGames> games;
        std::unique_ptr<AchievementsPage> achievementsPage;
        juce::Viewport achievementsViewport;
        std::unique_ptr<DictionaryPage> dictionary;
        std::unique_ptr<AchievementToast> toast;

        Tab tab = Tab::path;
        bool phoneShowsArticle = false;   // en el móvil: lista o lección
        int lessonIndex = -1;
        juce::Rectangle<int> headerArea, statsArea, tabsArea, contentArea;
    };
}
