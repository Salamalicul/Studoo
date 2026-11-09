#pragma once
// ============================================================================
//  HelpButton: el botón redondo "?" que hay en cada sección.
//  Al pulsarlo abre una ventana con la explicación de esa sección.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>

namespace studoo
{
    class HelpButton : public juce::Button
    {
    public:
        HelpButton (juce::String helpTitle, juce::String helpText);

        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    private:
        void showHelp();
        juce::String title, text;
    };
}
