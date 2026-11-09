#pragma once
// ============================================================================
//  Widgets: piezas de interfaz reutilizables.
//   - Knob: rueda con su nombre debajo, conectada a un dato del proyecto.
//   - IconButton: botón con un dibujo (play, stop, grabar, deshacer...).
//   - Funciones para dibujar los paneles con el mismo estilo.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>
#include "StudooLookAndFeel.h"

namespace studoo
{
    // Evita que un control "robe" el teclado (así la barra espaciadora siempre funciona)
    inline void makeNonFocusing (juce::Component& c)
    {
        c.setWantsKeyboardFocus (false);
        c.setMouseClickGrabsKeyboardFocus (false);
    }

    // Componente que se dibuja con una función (para paneles sencillos)
    class CallbackComponent : public juce::Component
    {
    public:
        std::function<void (juce::Graphics&)> onPaint;
        void paint (juce::Graphics& g) override { if (onPaint) onPaint (g); }
    };

    // Rueda con etiqueta, enlazada a una propiedad del proyecto (con deshacer)
    class Knob : public juce::Component
    {
    public:
        Knob (const juce::String& labelText, const juce::String& tooltip);

        void bind (juce::ValueTree tree, const juce::Identifier& property, juce::UndoManager* um,
                   double min, double max, double defaultValue, double skewMidPoint = 0.0);
        void setValueSuffix (const juce::String& s, int decimals = 2) { suffix = s; numDecimals = decimals; }
        void setDisplayFunction (std::function<juce::String (double)> f) { displayFunction = std::move (f); }

        juce::Slider& getSlider() { return slider; }
        void resized() override;
        void paint (juce::Graphics&) override;

    private:
        juce::Slider slider;
        juce::String label, suffix;
        int numDecimals = 2;
        std::function<juce::String (double)> displayFunction;
        juce::UndoManager* undo = nullptr;
    };

    class IconButton : public juce::Button
    {
    public:
        enum class Icon { play, stop, record, undo, redo, menu };
        IconButton (Icon, juce::Colour activeColour);
        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    private:
        Icon icon;
        juce::Colour colour;
    };

    // Dibuja el fondo de una sección con su cabecera
    void drawPanel (juce::Graphics&, juce::Rectangle<int> area, juce::Rectangle<int> header, const juce::String& title);

    // Nombres de notas en español: 60 -> "Do4"
    juce::String noteName (int midiNote, bool withOctave = true);
    bool isBlackKey (int midiNote);

    // Ventana para escribir un texto (por ejemplo, renombrar). Llama a onOk si se acepta.
    void askForText (const juce::String& title, const juce::String& message, const juce::String& initial,
                     std::function<void (const juce::String&)> onOk);
}
