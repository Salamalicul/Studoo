#pragma once
// ============================================================================
//  StudooLookAndFeel: el "aspecto" de la app (colores, botones, ruedas...).
//  Cambiando este archivo cambia el estilo de toda la interfaz a la vez.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>

namespace studoo
{
    // Convierte texto con tildes y ñ (UTF-8) al formato de JUCE
    inline juce::String txt (const char* utf8) { return juce::String::fromUTF8 (utf8); }

    namespace Colours
    {
        const juce::Colour background   { 0xff1b1f23 };
        const juce::Colour panel        { 0xff262b31 };
        const juce::Colour panelLight   { 0xff313840 };
        const juce::Colour header       { 0xff1f2429 };
        const juce::Colour outline      { 0xff3c444d };
        const juce::Colour text         { 0xffdfe4ea };
        const juce::Colour textDim      { 0xff8a949e };
        const juce::Colour accent       { 0xffff8a1f }; // naranja
        const juce::Colour accentDim    { 0xff8a4c14 };
        const juce::Colour green        { 0xff5fd35a };
        const juce::Colour stepOffA     { 0xff3a4149 }; // pasos grupo 1 y 3
        const juce::Colour stepOffB     { 0xff4a3a3a }; // pasos grupo 2 y 4 (rojizo, como FL)
        const juce::Colour stepOn       { 0xffd8dde3 };
    }

    class StudooLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        StudooLookAndFeel();

        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                                   bool isMouseOver, bool isButtonDown) override;

        void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;

        void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               juce::Slider::SliderStyle, juce::Slider&) override;

        void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
        juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                               juce::Rectangle<int> parentArea) override;
    };
}
