#include "StudooLookAndFeel.h"

namespace studoo
{
StudooLookAndFeel::StudooLookAndFeel()
{
    auto scheme = getDarkColourScheme();
    scheme.setUIColour (ColourScheme::windowBackground, Colours::background);
    scheme.setUIColour (ColourScheme::widgetBackground, Colours::panelLight);
    scheme.setUIColour (ColourScheme::menuBackground,   Colours::panel);
    scheme.setUIColour (ColourScheme::outline,          Colours::outline);
    scheme.setUIColour (ColourScheme::defaultText,      Colours::text);
    scheme.setUIColour (ColourScheme::defaultFill,      Colours::accent);
    scheme.setUIColour (ColourScheme::highlightedText,  juce::Colours::black);
    scheme.setUIColour (ColourScheme::highlightedFill,  Colours::accent);
    scheme.setUIColour (ColourScheme::menuText,         Colours::text);
    setColourScheme (scheme);

    setColour (juce::TextButton::buttonColourId,       Colours::panelLight);
    setColour (juce::TextButton::buttonOnColourId,     Colours::accent);
    setColour (juce::TextButton::textColourOffId,      Colours::text);
    setColour (juce::TextButton::textColourOnId,       juce::Colours::black);
    setColour (juce::Slider::thumbColourId,            Colours::accent);
    setColour (juce::Slider::rotarySliderFillColourId, Colours::accent);
    setColour (juce::Slider::trackColourId,            Colours::accent);
    setColour (juce::Slider::backgroundColourId,       Colours::panelLight);
    setColour (juce::Slider::textBoxTextColourId,      Colours::text);
    setColour (juce::Slider::textBoxOutlineColourId,   juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,              Colours::text);
    setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xff111417));
    setColour (juce::TooltipWindow::textColourId,      Colours::text);
    setColour (juce::TooltipWindow::outlineColourId,   Colours::accent);
    setColour (juce::AlertWindow::backgroundColourId,  Colours::panel);
    setColour (juce::AlertWindow::textColourId,        Colours::text);
    setColour (juce::AlertWindow::outlineColourId,     Colours::accent);
    setColour (juce::ResizableWindow::backgroundColourId, Colours::background);
}

void StudooLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool isMouseOver, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto colour = backgroundColour;
    if (isButtonDown)      colour = colour.darker (0.25f);
    else if (isMouseOver)  colour = colour.brighter (0.12f);

    g.setColour (colour);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (Colours::outline.brighter (isMouseOver ? 0.3f : 0.0f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void StudooLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    // Rueda (knob) redonda con un arco naranja que muestra el valor
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcWidth = juce::jmax (2.0f, radius * 0.18f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius - arcWidth * 0.5f, radius - arcWidth * 0.5f,
                         0.0f, startAngle, endAngle, true);
    g.setColour (Colours::background);
    g.strokePath (track, juce::PathStrokeType (arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius - arcWidth * 0.5f, radius - arcWidth * 0.5f,
                         0.0f, startAngle, angle, true);
    g.setColour (slider.isEnabled() ? Colours::accent : Colours::textDim);
    g.strokePath (value, juce::PathStrokeType (arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float knobRadius = radius - arcWidth - 2.0f;
    g.setGradientFill (juce::ColourGradient (Colours::panelLight.brighter (0.25f), centre.x, centre.y - knobRadius,
                                             Colours::panelLight.darker (0.4f), centre.x, centre.y + knobRadius, false));
    g.fillEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    juce::Path pointer;
    pointer.addRoundedRectangle (-1.25f, -knobRadius, 2.5f, knobRadius * 0.55f, 1.0f);
    g.setColour (Colours::text);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));
}

void StudooLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearBar)
    {
        // Caja de valor que se arrastra (como el BPM de FL Studio)
        auto area = juce::Rectangle<int> (x, y, width, height).toFloat();
        g.setColour (juce::Colour (0xff101316));
        g.fillRoundedRectangle (area, 4.0f);
        g.setColour (Colours::accentDim.withAlpha (0.6f));
        g.fillRoundedRectangle (area.withRight (sliderPos), 4.0f);
        g.setColour (Colours::outline);
        g.drawRoundedRectangle (area.reduced (0.5f), 4.0f, 1.0f);
        return;
    }

    LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}

juce::Rectangle<int> StudooLookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                                          juce::Rectangle<int> parentArea)
{
    juce::TextLayout tl;
    juce::AttributedString s;
    s.append (tipText, juce::Font (juce::FontOptions (14.0f)), Colours::text);
    tl.createLayoutWithBalancedLineLengths (s, 320.0f);

    const int w = (int) tl.getWidth() + 20;
    const int h = (int) tl.getHeight() + 14;

    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                                 w, h).constrainedWithin (parentArea);
}

void StudooLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto bounds = juce::Rectangle<int> (width, height).toFloat();
    g.setColour (findColour (juce::TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (findColour (juce::TooltipWindow::outlineColourId));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 5.0f, 1.0f);

    juce::AttributedString s;
    s.append (text, juce::Font (juce::FontOptions (14.0f)), Colours::text);
    juce::TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, 320.0f);
    tl.draw (g, bounds.reduced (10.0f, 7.0f));
}
}
