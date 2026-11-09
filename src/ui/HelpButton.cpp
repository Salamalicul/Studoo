#include "HelpButton.h"
#include "StudooLookAndFeel.h"

namespace studoo
{
HelpButton::HelpButton (juce::String helpTitle, juce::String helpText)
    : juce::Button ("?"), title (std::move (helpTitle)), text (std::move (helpText))
{
    setTooltip (txt ("Ayuda: pulsa aquí para ver qué es esta sección y cómo se usa."));
    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);
    onClick = [this] { showHelp(); };
}

void HelpButton::paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    auto area = getLocalBounds().toFloat().reduced (1.0f);
    const float size = juce::jmin (area.getWidth(), area.getHeight());
    auto circle = area.withSizeKeepingCentre (size, size);

    auto colour = Colours::accent;
    if (isButtonDown)     colour = colour.darker (0.3f);
    else if (isMouseOver) colour = colour.brighter (0.2f);

    g.setColour (colour);
    g.fillEllipse (circle);
    g.setColour (juce::Colours::black);
    g.setFont (juce::FontOptions (size * 0.7f, juce::Font::bold));
    g.drawText ("?", circle, juce::Justification::centred);
}

void HelpButton::showHelp()
{
    auto options = juce::MessageBoxOptions()
                       .withIconType (juce::MessageBoxIconType::NoIcon)
                       .withTitle (title)
                       .withMessage (text)
                       .withButton (txt ("¡Entendido!"))
                       .withAssociatedComponent (getTopLevelComponent());
    juce::AlertWindow::showAsync (options, nullptr);
}
}
