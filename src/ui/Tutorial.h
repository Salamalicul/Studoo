#pragma once
// ============================================================================
//  Tutorial: una guía paso a paso que se muestra encima de la app.
//  Oscurece la pantalla excepto el control que hay que usar, explica qué
//  hacer y, cuando lo haces, te felicita y pasa al siguiente paso.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>

namespace studoo
{
    struct TutorialStep
    {
        juce::String title, text;
        std::function<void()> onEnter;                 // por ejemplo, cambiar de pestaña
        std::function<juce::Component*()> target;     // qué control resaltar (puede ser nulo)
        std::function<bool()> isDone;                 // si hay una tarea que completar
    };

    class TutorialOverlay : public juce::Component, private juce::Timer
    {
    public:
        TutorialOverlay();

        void start (std::vector<TutorialStep> steps);
        void stop();
        bool isRunning() const { return running; }
        std::function<void()> onFinished;

        void paint (juce::Graphics&) override;
        void resized() override;
        bool hitTest (int x, int y) override;

    private:
        void timerCallback() override;
        void goTo (int index);
        void updateLayout();

        std::vector<TutorialStep> steps;
        int current = 0;
        bool running = false, done = false;
        juce::Rectangle<int> hole, bubble;

        juce::TextButton nextButton, backButton, skipButton;
        juce::Label titleLabel, bodyLabel, progressLabel, doneLabel;
    };
}
