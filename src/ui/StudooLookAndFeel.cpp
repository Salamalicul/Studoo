#include "StudooLookAndFeel.h"
#include <BinaryData.h>

namespace studoo
{
// ============================================================================
//  Fuentes (Inter, incluida dentro de la app)
// ============================================================================
namespace
{
    juce::Typeface::Ptr loadFace (const void* data, int size)
    {
        return juce::Typeface::createSystemTypefaceFor (data, (size_t) size);
    }

    juce::Typeface::Ptr regularFace()
    {
        static auto face = loadFace (BinaryData::InterRegular_otf, BinaryData::InterRegular_otfSize);
        return face;
    }

    juce::Typeface::Ptr boldFace()
    {
        static auto face = loadFace (BinaryData::InterSemiBold_otf, BinaryData::InterSemiBold_otfSize);
        return face;
    }

    juce::Typeface::Ptr displayFace()
    {
        static auto face = loadFace (BinaryData::InterDisplayBlack_otf, BinaryData::InterDisplayBlack_otfSize);
        return face;
    }

    juce::Path roundedBox (juce::Rectangle<float> r, float corner, const juce::Button& b)
    {
        juce::Path p;
        const bool left = b.isConnectedOnLeft(), right = b.isConnectedOnRight();
        p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), corner, corner,
                               ! left, ! right, ! left, ! right);
        return p;
    }
}

namespace Fonts
{
    juce::FontOptions regular (float h) { return juce::FontOptions (regularFace()).withHeight (h); }
    juce::FontOptions bold (float h)    { return juce::FontOptions (boldFace()).withHeight (h); }
    juce::FontOptions display (float h) { return juce::FontOptions (displayFace()).withHeight (h); }
}

// ============================================================================
//  Utilidades de dibujo
// ============================================================================
namespace Draw
{
    void glow (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c, float radius, float corner)
    {
        for (int i = 4; i >= 1; --i)
        {
            const float e = radius * (float) i / 4.0f;
            g.setColour (c.withAlpha (0.055f));
            g.fillRoundedRectangle (r.expanded (e), corner + e);
        }
    }

    void shadow (juce::Graphics& g, juce::Rectangle<float> r, float corner, int radius)
    {
        for (int i = 0; i < 4; ++i)
        {
            const float e = (float) radius * (float) (i + 1) / 4.0f;
            g.setColour (juce::Colours::black.withAlpha (0.09f));
            g.fillRoundedRectangle (r.translated (0.0f, e * 0.35f).expanded (e * 0.5f), corner + e * 0.5f);
        }
    }

    void accentFill (juce::Graphics& g, juce::Rectangle<float> r, float corner)
    {
        g.setGradientFill (juce::ColourGradient (Colours::accentLight, 0.0f, r.getY(),
                                                 Colours::accentDeep, 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, corner);
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.fillRoundedRectangle (r.withHeight (juce::jmin (r.getHeight() * 0.45f, 10.0f)).reduced (1.0f, 0.0f).translated (0.0f, 1.0f), corner);
    }

    void lcd (juce::Graphics& g, juce::Rectangle<float> r)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff07090b), 0.0f, r.getY(),
                                                 juce::Colour (0xff0d1013), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 7.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRoundedRectangle (r.reduced (1.0f), 6.0f, 1.0f);
        g.setColour (Colours::outline.withAlpha (0.9f));
        g.drawRoundedRectangle (r.reduced (0.5f), 7.0f, 1.0f);
        g.setColour (Colours::accent.withAlpha (0.05f));
        g.fillRoundedRectangle (r.reduced (3.0f), 5.0f);
    }
}

// ============================================================================
//  LookAndFeel
// ============================================================================
StudooLookAndFeel::StudooLookAndFeel()
{
    regularFace = studoo::regularFace();
    boldFace = studoo::boldFace();

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
    setColour (juce::TextButton::textColourOnId,       juce::Colour (0xff1a1208));
    setColour (juce::Slider::thumbColourId,            Colours::accent);
    setColour (juce::Slider::rotarySliderFillColourId, Colours::accent);
    setColour (juce::Slider::trackColourId,            Colours::accent);
    setColour (juce::Slider::backgroundColourId,       Colours::panelLight);
    setColour (juce::Slider::textBoxTextColourId,      Colours::text);
    setColour (juce::Slider::textBoxOutlineColourId,   juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,              Colours::text);
    setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xf20b0d10));
    setColour (juce::TooltipWindow::textColourId,      Colours::text);
    setColour (juce::TooltipWindow::outlineColourId,   Colours::accent);
    setColour (juce::AlertWindow::backgroundColourId,  Colours::panel);
    setColour (juce::AlertWindow::textColourId,        Colours::text);
    setColour (juce::AlertWindow::outlineColourId,     Colours::accent);
    setColour (juce::ResizableWindow::backgroundColourId, Colours::background);
    setColour (juce::ComboBox::backgroundColourId,     Colours::panelLight);
    setColour (juce::ComboBox::outlineColourId,        Colours::outline);
    setColour (juce::ComboBox::textColourId,           Colours::text);
    setColour (juce::ComboBox::arrowColourId,          Colours::accent);
    setColour (juce::PopupMenu::backgroundColourId,    Colours::panel);
    setColour (juce::PopupMenu::textColourId,          Colours::text);
    setColour (juce::PopupMenu::headerTextColourId,    Colours::accent);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::accent);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xff1a1208));
    setColour (juce::ScrollBar::thumbColourId,         Colours::outline.brighter (0.25f));
    setColour (juce::ToggleButton::textColourId,       Colours::text);
    setColour (juce::ToggleButton::tickColourId,       juce::Colour (0xff1a1208));
    setColour (juce::ListBox::backgroundColourId,      Colours::panelDark);
    setColour (juce::TextEditor::backgroundColourId,   Colours::panelDark);
    setColour (juce::TextEditor::textColourId,         Colours::text);
    setColour (juce::TextEditor::outlineColourId,      Colours::outline);
    setColour (juce::TextEditor::focusedOutlineColourId, Colours::accent);
    setColour (juce::TextEditor::highlightColourId,    Colours::accent.withAlpha (0.35f));
    setColour (juce::CaretComponent::caretColourId,    Colours::accent);
}

juce::Typeface::Ptr StudooLookAndFeel::getTypefaceForFont (const juce::Font& font)
{
    const auto name = font.getTypefaceName();
    if (name == juce::Font::getDefaultSansSerifFontName() || name.isEmpty())
        return font.isBold() ? boldFace : regularFace;
    return LookAndFeel_V4::getTypefaceForFont (font);
}

// ----------------------------------------------------------------------------
//  Botones
// ----------------------------------------------------------------------------
void StudooLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool isMouseOver, bool isButtonDown)
{
    auto r = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = button.getToggleState();
    const float corner = 7.0f;

    // Pestañas (Secuenciador, Piano roll...)
    if ((bool) button.getProperties()["tab"])
    {
        if (on || isMouseOver)
        {
            g.setColour (on ? Colours::panelLight : Colours::panelLight.withAlpha (0.5f));
            g.fillRoundedRectangle (r, corner);
        }
        if (on)
        {
            auto bar = r.removeFromBottom (3.0f).reduced (r.getWidth() * 0.18f, 0.0f);
            Draw::glow (g, bar, Colours::accent, 6.0f, 1.5f);
            Draw::accentFill (g, bar, 1.5f);
        }
        return;
    }

    const auto onColour = button.findColour (juce::TextButton::buttonOnColourId);
    const bool coloured = on || backgroundColour != findColour (juce::TextButton::buttonColourId);
    auto base = on ? onColour : backgroundColour;
    if (isButtonDown)      base = base.darker (0.2f);
    else if (isMouseOver)  base = base.brighter (coloured ? 0.08f : 0.12f);
    if (! button.isEnabled())
        base = base.withMultipliedAlpha (0.5f);

    auto shape = roundedBox (r, corner, button);

    if (on)
    {
        Draw::glow (g, r, onColour, 7.0f, corner);
        g.setGradientFill (juce::ColourGradient (base.brighter (0.25f), 0.0f, r.getY(),
                                                 base.darker (0.15f), 0.0f, r.getBottom(), false));
    }
    else if (coloured)
    {
        g.setGradientFill (juce::ColourGradient (base.brighter (0.2f), 0.0f, r.getY(),
                                                 base.darker (0.2f), 0.0f, r.getBottom(), false));
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (base.brighter (0.06f), 0.0f, r.getY(),
                                                 base.darker (0.18f), 0.0f, r.getBottom(), false));
    }
    g.fillPath (shape);

    // Brillo superior y borde
    g.setColour (juce::Colours::white.withAlpha (on || coloured ? 0.2f : 0.05f));
    g.drawHorizontalLine ((int) r.getY() + 1, r.getX() + corner, r.getRight() - corner);
    g.setColour (on ? onColour.brighter (0.3f).withAlpha (0.9f)
                    : Colours::outline.brighter (isMouseOver ? 0.35f : 0.05f));
    g.strokePath (shape, juce::PathStrokeType (1.0f));
}

juce::Font StudooLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return Fonts::bold (juce::jlimit (11.0f, 15.0f, (float) buttonHeight * 0.48f));
}

void StudooLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool)
{
    const bool on = button.getToggleState();
    const bool coloured = on || button.findColour (juce::TextButton::buttonColourId) != findColour (juce::TextButton::buttonColourId);
    const bool tab = (bool) button.getProperties()["tab"];

    juce::Colour c = (coloured && ! tab) ? button.findColour (juce::TextButton::textColourOnId)
                                         : (tab && ! on ? Colours::textDim : Colours::text);
    if (tab && on)
        c = Colours::text;
    if (! button.isEnabled())
        c = c.withMultipliedAlpha (0.4f);

    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.setColour (c);
    g.drawFittedText (button.getButtonText(), button.getLocalBounds().reduced (6, 2), juce::Justification::centred, 1, 0.8f);
}

void StudooLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool isMouseOver, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto box = bounds.removeFromLeft (bounds.getHeight()).withSizeKeepingCentre (18.0f, 18.0f);
    const bool on = button.getToggleState();

    if (on)
    {
        Draw::glow (g, box, Colours::accent, 5.0f, 5.0f);
        Draw::accentFill (g, box, 5.0f);
        juce::Path tick;
        tick.startNewSubPath (box.getX() + 4.5f, box.getCentreY() + 0.5f);
        tick.lineTo (box.getX() + 7.8f, box.getBottom() - 5.0f);
        tick.lineTo (box.getRight() - 4.0f, box.getY() + 5.0f);
        g.setColour (juce::Colour (0xff1a1208));
        g.strokePath (tick, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else
    {
        g.setColour (Colours::panelDark);
        g.fillRoundedRectangle (box, 5.0f);
        g.setColour (Colours::outline.brighter (isMouseOver ? 0.4f : 0.15f));
        g.drawRoundedRectangle (box.reduced (0.5f), 5.0f, 1.2f);
    }

    g.setColour (Colours::text.withAlpha (button.isEnabled() ? 1.0f : 0.5f));
    g.setFont (Fonts::regular (14.0f));
    g.drawFittedText (button.getButtonText(), bounds.withTrimmedLeft (6.0f).toNearestInt(), juce::Justification::centredLeft, 1);
}

// ----------------------------------------------------------------------------
//  Ruedas (knobs)
// ----------------------------------------------------------------------------
void StudooLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcWidth = juce::jmax (2.5f, radius * 0.15f);
    const float arcRadius = radius - arcWidth * 0.5f;
    const bool enabled = slider.isEnabled();
    const bool hover = slider.isMouseOverOrDragging();

    // Pista de fondo
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff0a0c0f));
    g.strokePath (track, juce::PathStrokeType (arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Arco de valor (desde el centro si el rango es positivo y negativo, como el panorama)
    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float zeroAngle = bipolar ? startAngle + (float) slider.valueToProportionOfLength (0.0) * (endAngle - startAngle)
                                    : startAngle;
    if (std::abs (angle - zeroAngle) > 0.01f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             juce::jmin (zeroAngle, angle), juce::jmax (zeroAngle, angle), true);
        if (enabled)
        {
            g.setColour (Colours::accent.withAlpha (hover ? 0.35f : 0.22f));
            g.strokePath (value, juce::PathStrokeType (arcWidth * 2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setGradientFill (juce::ColourGradient (Colours::accentLight, centre.x, centre.y - radius,
                                                 Colours::accentDeep, centre.x, centre.y + radius, false));
        if (! enabled)
            g.setColour (Colours::textDim);
        g.strokePath (value, juce::PathStrokeType (arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Cuerpo de la rueda
    const float knobRadius = radius - arcWidth - juce::jmax (2.0f, radius * 0.08f);
    auto knob = juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f).withCentre (centre);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (knob.translated (0.0f, 2.0f).expanded (1.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (hover ? 0xff4a525d : 0xff3e454f), centre.x, knob.getY(),
                                             juce::Colour (0xff1b1f25), centre.x, knob.getBottom(), false));
    g.fillEllipse (knob);
    g.setColour (juce::Colour (0xff0b0d10));
    g.drawEllipse (knob, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawEllipse (knob.reduced (1.5f), 1.0f);

    // Indicador
    juce::Path pointer;
    const float len = knobRadius * 0.55f;
    pointer.startNewSubPath (0.0f, -knobRadius * 0.28f);
    pointer.lineTo (0.0f, -knobRadius * 0.28f - len);
    g.setColour (enabled ? Colours::text : Colours::textDim);
    g.strokePath (pointer, juce::PathStrokeType (juce::jmax (1.8f, knobRadius * 0.14f), juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded),
                  juce::AffineTransform::rotation (angle).translated (centre));
}

// ----------------------------------------------------------------------------
//  Faders y barras
// ----------------------------------------------------------------------------
void StudooLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto area = juce::Rectangle<int> (x, y, width, height).toFloat();

    if (style == juce::Slider::LinearBar)
    {
        // Pantalla LCD (el BPM)
        Draw::lcd (g, area);
        auto fill = area.reduced (4.0f).withTrimmedTop (area.getHeight() - 7.0f);
        g.setColour (Colours::accent.withAlpha (0.18f));
        g.fillRoundedRectangle (fill, 1.5f);
        g.setColour (Colours::accent.withAlpha (0.8f));
        g.fillRoundedRectangle (fill.withRight (juce::jmax (fill.getX() + 2.0f, sliderPos - 4.0f)), 1.5f);
        return;
    }

    if (style == juce::Slider::LinearVertical)
    {
        // Fader del mezclador
        const float cx = area.getCentreX();
        auto groove = juce::Rectangle<float> (cx - 3.0f, area.getY() + 8.0f, 6.0f, area.getHeight() - 16.0f);

        g.setColour (Colours::textDim.withAlpha (0.25f));
        for (int i = 0; i <= 10; ++i)
        {
            const float ty = groove.getY() + groove.getHeight() * (float) i / 10.0f;
            const float w = (i % 5 == 0) ? 9.0f : 5.0f;
            g.drawHorizontalLine ((int) ty, cx - 16.0f - w, cx - 16.0f);
            g.drawHorizontalLine ((int) ty, cx + 16.0f, cx + 16.0f + w);
        }

        g.setColour (juce::Colour (0xff07090b));
        g.fillRoundedRectangle (groove, 3.0f);
        auto filled = groove.withTop (sliderPos);
        g.setGradientFill (juce::ColourGradient (Colours::accentLight, 0.0f, filled.getY(),
                                                 Colours::accentDeep.withAlpha (0.7f), 0.0f, filled.getBottom(), false));
        g.fillRoundedRectangle (filled, 3.0f);

        // Tapa del fader
        auto cap = juce::Rectangle<float> (juce::jmin (area.getWidth() - 6.0f, 34.0f), 22.0f).withCentre ({ cx, sliderPos });
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (cap.translated (0.0f, 2.5f), 4.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (slider.isMouseOverOrDragging() ? 0xff5a636f : 0xff4b535e), 0.0f, cap.getY(),
                                                 juce::Colour (0xff23282f), 0.0f, cap.getBottom(), false));
        g.fillRoundedRectangle (cap, 4.0f);
        g.setColour (juce::Colour (0xff0b0d10));
        g.drawRoundedRectangle (cap, 4.0f, 1.0f);
        g.setColour (Colours::accent);
        g.fillRect (cap.withSizeKeepingCentre (cap.getWidth() - 10.0f, 2.0f));
        return;
    }

    if (style == juce::Slider::LinearHorizontal)
    {
        const float cy = area.getCentreY();
        auto groove = juce::Rectangle<float> (area.getX() + 6.0f, cy - 2.0f, area.getWidth() - 12.0f, 4.0f);
        g.setColour (juce::Colour (0xff07090b));
        g.fillRoundedRectangle (groove, 2.0f);
        Draw::accentFill (g, groove.withRight (sliderPos), 2.0f);
        auto thumb = juce::Rectangle<float> (14.0f, 14.0f).withCentre ({ sliderPos, cy });
        Draw::glow (g, thumb, Colours::accent, 5.0f, 7.0f);
        g.setColour (Colours::text);
        g.fillEllipse (thumb);
        g.setColour (Colours::accent);
        g.drawEllipse (thumb.reduced (1.0f), 2.0f);
        return;
    }

    LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}

juce::Label* StudooLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox (slider);
    if (slider.getSliderStyle() == juce::Slider::LinearBar)
    {
        label->setFont (Fonts::bold (20.0f));
        label->setColour (juce::Label::textColourId, Colours::accentLight);
        label->setJustificationType (juce::Justification::centred);
    }
    else
    {
        label->setFont (Fonts::regular (13.0f));
    }
    label->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    return label;
}

// ----------------------------------------------------------------------------
//  Listas desplegables y menús
// ----------------------------------------------------------------------------
void StudooLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                      int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<int> (width, height).toFloat().reduced (1.0f);
    const bool hover = box.isMouseOver (true);
    auto base = Colours::panelLight.brighter (hover ? 0.08f : 0.0f);
    if (isButtonDown) base = base.darker (0.15f);

    g.setGradientFill (juce::ColourGradient (base.brighter (0.05f), 0.0f, r.getY(), base.darker (0.2f), 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 7.0f);
    g.setColour (Colours::outline.brighter (hover ? 0.35f : 0.05f));
    g.drawRoundedRectangle (r, 7.0f, 1.0f);

    // Flecha
    const float ax = r.getRight() - 16.0f, ay = r.getCentreY();
    juce::Path arrow;
    arrow.startNewSubPath (ax - 4.5f, ay - 2.0f);
    arrow.lineTo (ax, ay + 2.5f);
    arrow.lineTo (ax + 4.5f, ay - 2.0f);
    g.setColour (Colours::accent);
    g.strokePath (arrow, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font StudooLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return Fonts::regular (juce::jmin (14.5f, (float) box.getHeight() * 0.55f));
}

void StudooLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 1, box.getWidth() - 34, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void StudooLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (Colours::panel);
    g.setColour (Colours::outline.brighter (0.15f));
    g.drawRect (0, 0, width, height, 1);
    g.setColour (Colours::accent);
    g.fillRect (0, 0, width, 2);
}

juce::Font StudooLookAndFeel::getPopupMenuFont()
{
    return Fonts::regular (15.0f);
}

void StudooLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar&, int x, int y, int width, int height,
                                       bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                                       bool isMouseOver, bool isMouseDown)
{
    juce::Rectangle<float> thumb;
    if (isScrollbarVertical)
        thumb = { (float) x + 2.0f, (float) thumbStartPosition + 2.0f, (float) width - 4.0f, (float) thumbSize - 4.0f };
    else
        thumb = { (float) thumbStartPosition + 2.0f, (float) y + 2.0f, (float) thumbSize - 4.0f, (float) height - 4.0f };

    g.setColour (isMouseDown ? Colours::accent : Colours::outline.brighter (isMouseOver ? 0.6f : 0.3f));
    g.fillRoundedRectangle (thumb, juce::jmin (thumb.getWidth(), thumb.getHeight()) * 0.5f);
}

// ----------------------------------------------------------------------------
//  Mensajes de ayuda (tooltips) y ventanas de aviso
// ----------------------------------------------------------------------------
juce::Rectangle<int> StudooLookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                                          juce::Rectangle<int> parentArea)
{
    juce::TextLayout tl;
    juce::AttributedString s;
    s.append (tipText, juce::Font (Fonts::regular (14.0f)), Colours::text);
    tl.createLayoutWithBalancedLineLengths (s, 320.0f);

    const int w = (int) tl.getWidth() + 26;
    const int h = (int) tl.getHeight() + 18;

    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                                 w, h).constrainedWithin (parentArea);
}

void StudooLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto bounds = juce::Rectangle<int> (width, height).toFloat();
    g.setColour (findColour (juce::TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle (bounds, 7.0f);
    g.setColour (Colours::accent.withAlpha (0.8f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 7.0f, 1.0f);
    g.fillRoundedRectangle (bounds.withWidth (3.0f).reduced (0.0f, 6.0f), 1.5f);

    juce::AttributedString s;
    s.append (text, juce::Font (Fonts::regular (14.0f)), Colours::text);
    juce::TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, 320.0f);
    tl.draw (g, bounds.reduced (14.0f, 9.0f));
}

void StudooLookAndFeel::drawAlertBox (juce::Graphics& g, juce::AlertWindow& alert,
                                      const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout)
{
    auto bounds = alert.getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (Colours::panelLight, 0.0f, 0.0f, Colours::panel, 0.0f, bounds.getHeight(), false));
    g.fillAll();
    g.setColour (Colours::outline.brighter (0.2f));
    g.drawRect (bounds, 1.0f);
    Draw::accentFill (g, bounds.withHeight (4.0f), 0.0f);
    textLayout.draw (g, textArea.toFloat());
}

void StudooLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& editor)
{
    g.setColour (editor.findColour (juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle (juce::Rectangle<int> (width, height).toFloat(), 7.0f);
}

void StudooLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& editor)
{
    if (! editor.isEnabled() || editor.isReadOnly())
    {
        g.setColour (Colours::outline);
        g.drawRoundedRectangle (juce::Rectangle<int> (width, height).toFloat().reduced (0.5f), 7.0f, 1.0f);
        return;
    }
    g.setColour (editor.hasKeyboardFocus (true) ? Colours::accent : Colours::outline.brighter (0.2f));
    g.drawRoundedRectangle (juce::Rectangle<int> (width, height).toFloat().reduced (0.5f), 7.0f, editor.hasKeyboardFocus (true) ? 1.6f : 1.0f);
}
}
