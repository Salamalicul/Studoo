#pragma once
// ============================================================================
//  LearnStyle: tamaños de letra y botones del apartado "Aprende".
//  En el móvil todo es un poco más grande para leerlo cómodamente.
// ============================================================================
#include "StudooLookAndFeel.h"
#include "Widgets.h"

namespace studoo::learn
{
    // Tamaño de letra de Aprende (más grande en el móvil)
    inline float fs (float base) { return base * (Layout::compact() ? 1.32f : 1.1f); }

    // Alto de los botones de Aprende
    inline int buttonHeight() { return Layout::compact() ? 52 : 40; }

    enum class ButtonStyle { primary, secondary, good, bad };

    inline void styleButton (juce::TextButton& b, ButtonStyle style = ButtonStyle::secondary, float fontSize = 15.0f)
    {
        b.getProperties().set ("fontSize", fs (fontSize));
        juce::Colour bg;
        switch (style)
        {
            case ButtonStyle::primary:   bg = Colours::accent; break;
            case ButtonStyle::good:      bg = Colours::green; break;
            case ButtonStyle::bad:       bg = Colours::red; break;
            case ButtonStyle::secondary: bg = Colours::panelLight; break;
        }
        if (style == ButtonStyle::secondary)
        {
            b.removeColour (juce::TextButton::buttonColourId);
            b.removeColour (juce::TextButton::textColourOffId);
            b.removeColour (juce::TextButton::textColourOnId);
        }
        else
        {
            b.setColour (juce::TextButton::buttonColourId, bg);
            b.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff15110b));
            b.setColour (juce::TextButton::textColourOnId, juce::Colour (0xff15110b));
        }
        makeNonFocusing (b);
    }
}
