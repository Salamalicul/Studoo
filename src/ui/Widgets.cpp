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
    g.setColour (Colours::textDim);
    g.setFont (juce::FontOptions (11.0f));
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
    auto area = getLocalBounds().toFloat().reduced (0.5f);
    const bool on = getToggleState();

    g.setColour (on ? colour.withAlpha (0.25f) : Colours::panelLight.brighter (isMouseOver ? 0.12f : 0.0f));
    g.fillRoundedRectangle (area, 4.0f);
    g.setColour (on ? colour : Colours::outline);
    g.drawRoundedRectangle (area, 4.0f, 1.0f);

    const float size = area.getHeight() * 0.45f;
    auto r = area.withSizeKeepingCentre (size, size);
    if (isButtonDown)
        r = r.reduced (1.0f);

    const auto c = isEnabled() ? (on || isMouseOver ? colour : colour.withMultipliedBrightness (0.8f))
                               : Colours::textDim.withAlpha (0.4f);
    g.setColour (c);

    juce::Path p;
    switch (icon)
    {
        case Icon::play:
            p.addTriangle (r.getX(), r.getY(), r.getX(), r.getBottom(), r.getRight(), r.getCentreY());
            g.fillPath (p);
            break;
        case Icon::stop:
            g.fillRoundedRectangle (r, 1.5f);
            break;
        case Icon::record:
            g.fillEllipse (r);
            break;
        case Icon::undo:
        case Icon::redo:
        {
            p.addCentredArc (r.getCentreX(), r.getCentreY() + size * 0.1f, size * 0.42f, size * 0.42f, 0.0f,
                             -juce::MathConstants<float>::halfPi * 1.6f, juce::MathConstants<float>::halfPi * 1.2f, true);
            g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                          icon == Icon::redo ? juce::AffineTransform::scale (-1.0f, 1.0f, r.getCentreX(), r.getCentreY())
                                             : juce::AffineTransform());
            juce::Path arrow;
            const float ax = r.getX() + size * 0.08f, ay = r.getY() + size * 0.22f;
            arrow.addTriangle (ax - 4.0f, ay, ax + 4.0f, ay, ax, ay + 6.0f);
            g.fillPath (arrow, icon == Icon::redo ? juce::AffineTransform::scale (-1.0f, 1.0f, r.getCentreX(), r.getCentreY())
                                                   : juce::AffineTransform());
            break;
        }
        case Icon::menu:
            for (int i = 0; i < 3; ++i)
                g.fillRoundedRectangle (r.getX(), r.getY() + (float) i * size * 0.4f, size, size * 0.18f, 1.0f);
            break;
    }
}

// ============================================================================
//  Utilidades
// ============================================================================
void drawPanel (juce::Graphics& g, juce::Rectangle<int> area, juce::Rectangle<int> header, const juce::String& title)
{
    auto bounds = area.toFloat();
    g.setColour (Colours::panel);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

    g.setColour (Colours::header);
    g.fillRoundedRectangle (header.toFloat(), 6.0f);
    g.fillRect (header.withTrimmedTop (header.getHeight() / 2));
    g.setColour (Colours::text);
    g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    g.drawText (title, header.withTrimmedLeft (44), juce::Justification::centredLeft);
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
