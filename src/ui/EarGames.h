#pragma once
// ============================================================================
//  EarGames: juegos para entrenar el oído (pestaña "Juegos" de Aprende).
//    - ¿Qué instrumento es?   (bombo, caja, hi-hat...)
//    - ¿Sube o baja?          (dos notas)
//    - ¿Mayor o menor?        (acordes alegres o tristes)
//    - ¿Qué género es?        (las bases de géneros de Studoo)
//  Los sonidos se preparan aquí y suenan con AudioEngine::playBuffer.
// ============================================================================
#include "AppContext.h"
#include "LearnProgress.h"
#include <map>

namespace studoo
{
    class EarGames : public juce::Component
    {
    public:
        EarGames (AudioEngine&, learn::Progress&);
        ~EarGames() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        void visibilityChanged() override;

    private:
        struct Game { const char* id; const char* title; const char* description; int icon; };
        static const std::vector<Game>& games();

        void openGame (int index);
        void closeGame();
        void newRound();
        void playRound();
        void answer (int option);
        void layoutGame();

        // Sonidos
        AudioEngine::SharedBuffer renderNotes (const std::vector<std::vector<int>>& groups, double secondsEach);
        AudioEngine::SharedBuffer renderDrum (const juce::String& kitId, const juce::String& soundId);
        AudioEngine::SharedBuffer renderGenre (const juce::String& genreId);

        AudioEngine& engine;
        learn::Progress& progress;

        int current = -1;                 // juego abierto (-1 = lista de juegos)
        juce::OwnedArray<juce::Button> gameCards;
        juce::TextButton backButton, playButton, nextButton;
        juce::OwnedArray<juce::TextButton> optionButtons;
        juce::Label questionLabel, feedbackLabel;

        // Ronda actual
        juce::StringArray options;
        int correctOption = -1, chosen = -1;
        AudioEngine::SharedBuffer roundSound;
        int streak = 0;
        juce::Random random;
        std::map<juce::String, AudioEngine::SharedBuffer> genreCache;
        juce::Rectangle<int> headerArea;
    };
}
