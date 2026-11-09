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

    if (isMouseOver)
        Draw::glow (g, circle, Colours::accent, 5.0f, size * 0.5f);
    g.setColour (Colours::accent.withAlpha (isMouseOver ? 0.3f : 0.14f));
    g.fillEllipse (circle);
    g.setColour (isButtonDown ? Colours::accentDeep : Colours::accent);
    g.drawEllipse (circle.reduced (1.0f), 1.6f);
    g.setColour (isMouseOver ? Colours::accentLight : Colours::accent);
    g.setFont (Fonts::bold (size * 0.62f));
    g.drawText ("?", circle.translated (0.0f, 0.5f), juce::Justification::centred);
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
