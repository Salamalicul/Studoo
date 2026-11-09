#pragma once
// ============================================================================
//  StudooLookAndFeel: el "aspecto" de la app (colores, fuentes, botones,
//  ruedas, faders, menús...). Cambiando este archivo cambia el estilo de
//  toda la interfaz a la vez.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>

namespace studoo
{
    // Convierte texto con tildes y ñ (UTF-8) al formato de JUCE
    inline juce::String txt (const char* utf8) { return juce::String::fromUTF8 (utf8); }

    namespace Colours
    {
        const juce::Colour background   { 0xff111317 };
        const juce::Colour panelDark    { 0xff15181c };
        const juce::Colour panel        { 0xff1b1e23 };
        const juce::Colour panelLight   { 0xff252a31 };
        const juce::Colour header       { 0xff171a1f };
        const juce::Colour outline      { 0xff2d323a };
        const juce::Colour text         { 0xffe9ecf0 };
        const juce::Colour textDim      { 0xff8d95a0 };
        const juce::Colour accent       { 0xffff8a1f }; // naranja
        const juce::Colour accentLight  { 0xffffb04d };
        const juce::Colour accentDeep   { 0xffff6a00 };
        const juce::Colour accentDim    { 0xff7a4310 };
        const juce::Colour green        { 0xff52d273 };
        const juce::Colour blue         { 0xff4fc3f7 };
        const juce::Colour stepOffA     { 0xff272c33 }; // pasos grupo 1 y 3
        const juce::Colour stepOffB     { 0xff302a2c }; // pasos grupo 2 y 4 (tono cálido, como FL)
        const juce::Colour stepOn       { 0xffff8a1f };
    }

    // Fuentes incluidas en la app (Inter)
    namespace Fonts
    {
        juce::FontOptions regular (float height);
        juce::FontOptions bold (float height);
        juce::FontOptions display (float height); // para el logo y títulos grandes
    }

    // Utilidades de dibujo con el estilo de Studoo
    namespace Draw
    {
        // Brillo suave alrededor de un rectángulo
        void glow (juce::Graphics&, juce::Rectangle<float>, juce::Colour, float radius, float corner);
        // Sombra debajo de un panel
        void shadow (juce::Graphics&, juce::Rectangle<float>, float corner, int radius = 14);
        // Relleno naranja con degradado
        void accentFill (juce::Graphics&, juce::Rectangle<float>, float corner);
        // Pantalla LCD (BPM, posición)
        void lcd (juce::Graphics&, juce::Rectangle<float>);
    }

    class StudooLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        StudooLookAndFeel();

        juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;

        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                                   bool isMouseOver, bool isButtonDown) override;
        void drawButtonText (juce::Graphics&, juce::TextButton&, bool isMouseOver, bool isButtonDown) override;
        juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool isMouseOver, bool isButtonDown) override;

        void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;

        void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               juce::Slider::SliderStyle, juce::Slider&) override;
        juce::Label* createSliderTextBox (juce::Slider&) override;

        void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                           int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
        juce::Font getComboBoxFont (juce::ComboBox&) override;
        void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

        void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
        juce::Font getPopupMenuFont() override;

        void drawScrollbar (juce::Graphics&, juce::ScrollBar&, int x, int y, int width, int height,
                            bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                            bool isMouseOver, bool isMouseDown) override;
        int getDefaultScrollbarWidth() override { return 10; }

        void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
        juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                               juce::Rectangle<int> parentArea) override;

        void drawAlertBox (juce::Graphics&, juce::AlertWindow&, const juce::Rectangle<int>& textArea,
                           juce::TextLayout&) override;
        juce::Font getAlertWindowTitleFont() override   { return Fonts::bold (19.0f); }
        juce::Font getAlertWindowMessageFont() override { return Fonts::regular (15.0f); }
        juce::Font getAlertWindowFont() override        { return Fonts::regular (14.0f); }

        void fillTextEditorBackground (juce::Graphics&, int width, int height, juce::TextEditor&) override;
        void drawTextEditorOutline (juce::Graphics&, int width, int height, juce::TextEditor&) override;

    private:
        juce::Typeface::Ptr regularFace, boldFace;
    };
}
