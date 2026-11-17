#pragma once
// ============================================================================
//  LearnProgress: todo lo que se guarda de "Aprende" en los ajustes:
//    - lecciones aprendidas, mejores notas de los tests, retos superados,
//      récords de los juegos de oído
//    - la racha (días seguidos practicando)
//    - los logros (medallas) que se desbloquean con todo lo anterior
// ============================================================================
#include <juce_data_structures/juce_data_structures.h>
#include <functional>
#include <vector>

namespace studoo::learn
{
    struct Achievement
    {
        const char* id;
        const char* title;
        const char* description;
        int icon;   // dibujo de la medalla (ver LearnView.cpp)
    };

    // Todos los logros, en el orden en que se muestran
    const std::vector<Achievement>& achievements();

    class Progress
    {
    public:
        void setSettings (juce::PropertiesFile* s) { settings = s; }
        bool hasSettings() const { return settings != nullptr; }

        // --- Lecciones -----------------------------------------------------------------
        bool isLessonDone (const juce::String& title) const;
        void setLessonDone (const juce::String& title, bool done);
        int  numLessonsDone() const;

        // --- Tests ---------------------------------------------------------------------
        int  bestQuizScore (const juce::String& lessonTitle) const;          // -1 si no se ha hecho
        void recordQuiz (const juce::String& lessonTitle, int score, int total);
        int  numPerfectQuizzes() const;

        // --- Retos ---------------------------------------------------------------------
        bool isChallengeDone (const juce::String& id) const;
        void setChallengeDone (const juce::String& id);
        int  numChallengesDone() const;

        // --- Juegos de oído ------------------------------------------------------------
        int  bestGameStreak (const juce::String& gameId) const;
        void recordGameStreak (const juce::String& gameId, int streak);
        int  totalGameAnswers() const;
        void addGameAnswer (bool correct);

        // --- Racha de días -------------------------------------------------------------
        int  currentStreak() const;   // días seguidos (0 si ayer y hoy no practicaste)
        int  bestStreak() const;
        bool practicedToday() const;

        // --- Logros --------------------------------------------------------------------
        bool isUnlocked (const juce::String& achievementId) const;
        int  numUnlocked() const;

        // Se llama cuando se gana una medalla nueva (para enseñarla)
        std::function<void (const Achievement&)> onUnlocked;
        // Se llama cuando cambia cualquier cosa (para repintar)
        std::function<void()> onChanged;

        // Cuántas lecciones hay en cada nivel (lo rellena LearnView)
        std::function<bool (int level)> isLevelComplete;
        int numLevels = 0;

    private:
        juce::StringArray getList (const juce::String& key) const;
        void setList (const juce::String& key, const juce::StringArray&);
        void practiced();          // cuenta para la racha de hoy
        void checkAchievements();
        void changed();

        juce::PropertiesFile* settings = nullptr;
    };
}
