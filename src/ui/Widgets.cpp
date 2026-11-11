#include "Widgets.h"

namespace studoo
{
// ============================================================================
//  Knob
// ============================================================================
Knob::Knob (const juce::String& labelText, const juce::String& tooltip) : label (labelText)
{
    addAndMakeVisible (slider);
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled (true, true, nullptr, 1500);
    slider.setTooltip (tooltip + txt ("\n(Arrastra arriba/abajo. Doble clic: valor por defecto)"));
    slider.textFromValueFunction = [this] (double v)
    {
        if (displayFunction) return displayFunction (v);
        return juce::String (v, numDecimals) + suffix;
    };
    slider.onDragStart = [this] { if (undo != nullptr) undo->beginNewTransaction(); };
    makeNonFocusing (slider);
}

void Knob::bind (juce::ValueTree tree, const juce::Identifier& property, juce::UndoManager* um,
                 double min, double max, double defaultValue, double skewMidPoint)
{
    undo = um;
    slider.getValueObject().referTo (juce::Value()); // desconecta el anterior
    slider.setRange (min, max, 0.0);
    if (skewMidPoint > min && skewMidPoint < max)
        slider.setSkewFactorFromMidPoint (skewMidPoint);
    slider.setDoubleClickReturnValue (true, defaultValue);
    if (! tree.hasProperty (property))
        tree.setProperty (property, defaultValue, nullptr);
    slider.getValueObject().referTo (tree.getPropertyAsValue (property, um));
}

void Knob::resized()
{
    auto area = getLocalBounds();
    area.removeFromBottom (14);
    slider.setBounds (area);
}

void Knob::paint (juce::Graphics& g)
{
    g.setColour (slider.isMouseOverOrDragging() ? Colours::text : Colours::textDim);
    g.setFont (Fonts::regular (10.5f));
    g.drawText (label, getLocalBounds().removeFromBottom (14), juce::Justification::centred);
}

// ============================================================================
//  IconButton
// ============================================================================
IconButton::IconButton (Icon i, juce::Colour activeColour) : juce::Button ({}), icon (i), colour (activeColour)
{
    makeNonFocusing (*this);
}

void IconButton::paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    auto area = getLocalBounds().toFloat().reduced (1.0f);
    const bool on = getToggleState();
    const float corner = 8.0f;

    // Fondo del botón
    if (on)
        Draw::glow (g, area, colour, 8.0f, corner);
    auto base = on ? colour.withAlpha (0.22f).overlaidWith (Colours::panelLight.withAlpha (0.4f)) : Colours::panelLight;
    if (isMouseOver) base = base.brighter (0.1f);
    if (isButtonDown) base = base.darker (0.2f);
    g.setGradientFill (juce::ColourGradient (base.brighter (0.08f), 0.0f, area.getY(), base.darker (0.25f), 0.0f, area.getBottom(), false));
    g.fillRoundedRectangle (area, corner);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawHorizontalLine ((int) area.getY() + 1, area.getX() + corner, area.getRight() - corner);
    g.setColour (on ? colour.withAlpha (0.9f) : Colours::outline.brighter (isMouseOver ? 0.35f : 0.05f));
    g.drawRoundedRectangle (area, corner, 1.0f);

    const float size = area.getHeight() * 0.42f;
    auto r = area.withSizeKeepingCentre (size, size);
    if (isButtonDown)
        r = r.reduced (1.0f);

    const auto c = isEnabled() ? (on || isMouseOver ? colour.brighter (0.15f) : colour.withMultipliedBrightness (0.85f))
                               : Colours::textDim.withAlpha (0.35f);

    juce::Path p;
    switch (icon)
    {
        case Icon::play:
            p.addTriangle (r.getX() + size * 0.08f, r.getY(), r.getX() + size * 0.08f, r.getBottom(), r.getRight(), r.getCentreY());
            break;
        case Icon::stop:
            p.addRoundedRectangle (r.reduced (size * 0.04f), 2.5f);
            break;
        case Icon::record:
            p.addEllipse (r);
            break;
        case Icon::undo:
        case Icon::redo:
        {
            // Flecha en forma de "U" (como en la mayoría de programas)
            const float top = r.getY() + size * 0.22f, bottom = r.getY() + size * 0.9f;
            const float rad = (bottom - top) * 0.5f;
            const float xa = r.getRight() - rad - size * 0.02f;
            juce::Path arrow;
            arrow.startNewSubPath (r.getX() + size * 0.3f, top);
            arrow.lineTo (xa, top);
            arrow.addCentredArc (xa, top + rad, rad, rad, 0.0f, 0.0f, juce::MathConstants<float>::pi, false);
            arrow.lineTo (r.getX() + size * 0.22f, bottom);
            const auto flip = icon == Icon::redo ? juce::AffineTransform::scale (-1.0f, 1.0f, r.getCentreX(), r.getCentreY())
                                                 : juce::AffineTransform();
            g.setColour (c);
            g.strokePath (arrow, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), flip);
            juce::Path head;
            head.addTriangle (r.getX() - size * 0.04f, top, r.getX() + size * 0.36f, top - size * 0.3f,
                              r.getX() + size * 0.36f, top + size * 0.3f);
            g.fillPath (head, flip);
            return;
        }
        case Icon::menu:
            for (int i = 0; i < 3; ++i)
                p.addRoundedRectangle (r.getX(), r.getY() + (float) i * size * 0.4f, size, size * 0.17f, 1.2f);
            break;
    }

    if (on && isEnabled())
    {
        g.setColour (colour.withAlpha (0.35f));
        g.fillPath (p, juce::AffineTransform::scale (1.25f, 1.25f, r.getCentreX(), r.getCentreY()));
    }
    g.setColour (c);
    g.fillPath (p);
}

// ============================================================================
//  Utilidades
// ============================================================================
void drawPanel (juce::Graphics& g, juce::Rectangle<int> area, juce::Rectangle<int> header, const juce::String& title)
{
    auto bounds = area.toFloat();
    Draw::shadow (g, bounds, 10.0f, 12);
    g.setGradientFill (juce::ColourGradient (Colours::panel.brighter (0.03f), 0.0f, bounds.getY(),
                                             Colours::panelDark, 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 10.0f);

    // Cabecera
    auto h = header.toFloat();
    juce::Path headerShape;
    headerShape.addRoundedRectangle (h.getX(), h.getY(), h.getWidth(), h.getHeight(), 10.0f, 10.0f, true, true, false, false);
    g.setGradientFill (juce::ColourGradient (Colours::panelLight, 0.0f, h.getY(), Colours::header, 0.0f, h.getBottom(), false));
    g.fillPath (headerShape);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawHorizontalLine ((int) h.getBottom(), h.getX(), h.getRight());

    g.setColour (Colours::outline.withAlpha (0.9f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 10.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawHorizontalLine ((int) h.getY() + 1, h.getX() + 10.0f, h.getRight() - 10.0f);

    g.setColour (Colours::text);
    g.setFont (Fonts::bold (16.0f));
    g.drawText (title, header.withTrimmedLeft (46), juce::Justification::centredLeft);
}

juce::String noteName (int midiNote, bool withOctave)
{
    static const char* names[] = { "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si" };
    juce::String s (names[((midiNote % 12) + 12) % 12]);
    if (withOctave)
        s << (midiNote / 12 - 1);
    return s;
}

bool isBlackKey (int midiNote)
{
    const int n = ((midiNote % 12) + 12) % 12;
    return n == 1 || n == 3 || n == 6 || n == 8 || n == 10;
}

void askForText (const juce::String& title, const juce::String& message, const juce::String& initial,
                 std::function<void (const juce::String&)> onOk)
{
    auto* w = new juce::AlertWindow (title, message, juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("text", initial);
    w->addButton ("Aceptar", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancelar", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    w->enterModalState (true, juce::ModalCallbackFunction::create ([w, onOk] (int result)
    {
        const auto text = w->getTextEditorContents ("text").trim();
        if (result == 1 && text.isNotEmpty() && onOk)
            onOk (text);
    }), true);
}
}
