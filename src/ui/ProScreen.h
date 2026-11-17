#pragma once
// ============================================================================
//  ProScreen: la pantalla de "Studoo Pro" (solo en la versión de Google Play).
//  Explica qué incluye Pro, deja comprarlo o restaurar la compra, y también
//  donar para apoyar Studoo.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>

namespace studoo
{
    class ProScreen : public juce::Component, private juce::Timer
    {
    public:
        ProScreen();

        // Enseña la pantalla. 'reason' explica por qué ha salido ("El kit Drill es de Studoo Pro")
        void show (const juce::String& reason);
        void hide();

        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseUp (const juce::MouseEvent&) override;

    private:
        void timerCallback() override;
        void refreshButtons();
        juce::Rectangle<int> cardBounds() const;

        juce::String reason;
        juce::TextButton buyButton, restoreButton, closeButton;
        juce::OwnedArray<juce::TextButton> donateButtons;
        juce::Rectangle<int> card, leftArea, rightArea;
        bool wasUnlocked = false;
    };
}
