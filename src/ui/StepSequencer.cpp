#include "StepSequencer.h"
#include "HelpTexts.h"
#include "StudooLookAndFeel.h"

namespace studoo
{
namespace
{
    constexpr int headerHeight  = 40;
    constexpr int numbersHeight = 20;
    constexpr int rowHeight     = 44;
    constexpr int controlsWidth = 230; // luz + nombre + volumen
    constexpr float cellGap     = 4.0f;

    void makeNonFocusing (juce::Component& c)
    {
        c.setWantsKeyboardFocus (false);
        c.setMouseClickGrabsKeyboardFocus (false);
    }
}

// ---------------------------------------------------------------------------
//  StepGrid
// ---------------------------------------------------------------------------
StepGrid::StepGrid (AudioEngine& e) : engine (e)
{
    setTooltip (txt ("Pasos del patrón. Clic: encender/apagar. Arrastra: pintar varios. "
                     "Clic derecho: borrar. Cada grupo de 4 cuadrados es un tiempo."));
    makeNonFocusing (*this);
}

int StepGrid::getRowHeight() const { return rowHeight; }

void StepGrid::setPlayingStep (int step)
{
    if (step != playingStep)
    {
        playingStep = step;
        repaint();
    }
}

juce::Rectangle<float> StepGrid::getCellBounds (int channel, int step) const
{
    const float groupGap = 8.0f; // separación extra entre grupos de 4
    const float available = (float) getWidth() - groupGap * 3.0f;
    const float cellWidth = available / (float) numSteps;
    const float x = (float) step * cellWidth + (float) (step / 4) * groupGap;
    const float y = (float) (channel * rowHeight);
    return { x + cellGap * 0.5f, y + 6.0f, cellWidth - cellGap, (float) rowHeight - 12.0f };
}

bool StepGrid::cellAt (juce::Point<float> pos, int& channel, int& step) const
{
    for (int c = 0; c < numChannels; ++c)
        for (int s = 0; s < numSteps; ++s)
            if (getCellBounds (c, s).expanded (cellGap * 0.5f, 6.0f).contains (pos))
            {
                channel = c;
                step = s;
                return true;
            }
    return false;
}

void StepGrid::paint (juce::Graphics& g)
{
    for (int c = 0; c < numChannels; ++c)
    {
        const bool muted = engine.isChannelMuted (c);

        for (int s = 0; s < numSteps; ++s)
        {
            auto cell = getCellBounds (c, s);
            const bool on = engine.getStep (c, s);
            const bool isPlayhead = (s == playingStep);

            juce::Colour colour = ((s / 4) % 2 == 0) ? Colours::stepOffA : Colours::stepOffB;
            if (on)
                colour = Colours::stepOn;
            if (muted)
                colour = colour.withMultipliedSaturation (0.3f).withMultipliedBrightness (0.6f);

            g.setColour (colour);
            g.fillRoundedRectangle (cell, 3.0f);

            // Brillo en la parte de arriba para que parezca un botón
            g.setColour (juce::Colours::white.withAlpha (on ? 0.25f : 0.06f));
            g.fillRoundedRectangle (cell.withHeight (cell.getHeight() * 0.35f), 3.0f);

            // Luz del paso que está sonando ahora
            if (isPlayhead)
            {
                g.setColour (Colours::accent.withAlpha (on ? 0.9f : 0.5f));
                g.drawRoundedRectangle (cell.expanded (1.0f), 3.0f, 2.0f);
                g.setColour (Colours::accent);
                g.fillRect (cell.withY (cell.getBottom() + 2.0f).withHeight (3.0f));
            }
        }
    }
}

void StepGrid::mouseDown (const juce::MouseEvent& e)
{
    int c = 0, s = 0;
    if (! cellAt (e.position, c, s))
    {
        paintChannel = -1;
        return;
    }

    paintChannel = c;
    paintValue = e.mods.isRightButtonDown() ? false : ! engine.getStep (c, s);
    applyPaint (e);
}

void StepGrid::mouseDrag (const juce::MouseEvent& e)
{
    if (paintChannel >= 0)
        applyPaint (e);
}

void StepGrid::mouseUp (const juce::MouseEvent&)
{
    paintChannel = -1;
}

void StepGrid::applyPaint (const juce::MouseEvent& e)
{
    int c = 0, s = 0;
    // Solo pintamos en la fila donde se empezó a hacer clic
    const juce::Point<float> pos (e.position.x, getCellBounds (paintChannel, 0).getCentreY());
    if (cellAt (pos, c, s) && engine.getStep (c, s) != paintValue)
    {
        engine.setStep (c, s, paintValue);
        repaint();
    }
    juce::ignoreUnused (c);
}

// ---------------------------------------------------------------------------
//  MuteLight
// ---------------------------------------------------------------------------
MuteLight::MuteLight() : juce::Button ({})
{
    setClickingTogglesState (true);
    setToggleState (true, juce::dontSendNotification); // encendida = suena
    setTooltip (txt ("Mute: luz verde = el sonido está activo. Haz clic para silenciarlo "
                     "(la luz se apaga) o para volver a activarlo."));
    makeNonFocusing (*this);
}

void MuteLight::paintButton (juce::Graphics& g, bool isMouseOver, bool)
{
    auto area = getLocalBounds().toFloat().withSizeKeepingCentre (12.0f, 12.0f);
    const bool on = getToggleState();

    if (on)
    {
        g.setColour (Colours::green.withAlpha (0.25f));
        g.fillEllipse (area.expanded (3.0f));
    }
    g.setColour (on ? Colours::green : Colours::panelLight.brighter (isMouseOver ? 0.3f : 0.1f));
    g.fillEllipse (area);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawEllipse (area, 1.0f);
}

// ---------------------------------------------------------------------------
//  StepSequencer
// ---------------------------------------------------------------------------
StepSequencer::StepSequencer (AudioEngine& e)
    : engine (e), grid (e), helpButton (txt (help::sequencerTitle), txt (help::sequencerText))
{
    for (int c = 0; c < numChannels; ++c)
    {
        auto* ch = channels.add (new ChannelControls());

        addAndMakeVisible (ch->muteLight);
        ch->muteLight.setToggleState (! engine.isChannelMuted (c), juce::dontSendNotification);
        ch->muteLight.onClick = [this, c, ch]
        {
            engine.setChannelMuted (c, ! ch->muteLight.getToggleState());
            grid.repaint();
        };

        addAndMakeVisible (ch->nameButton);
        ch->nameButton.setButtonText (AudioEngine::getChannelName (c));
        ch->nameButton.setTooltip (txt ("Haz clic para escuchar este sonido una vez."));
        ch->nameButton.onClick = [this, c] { engine.previewChannel (c); };
        makeNonFocusing (ch->nameButton);

        addAndMakeVisible (ch->volumeKnob);
        ch->volumeKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        ch->volumeKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        ch->volumeKnob.setRange (0.0, 1.0, 0.01);
        ch->volumeKnob.setValue (engine.getChannelVolume (c), juce::dontSendNotification);
        ch->volumeKnob.setDoubleClickReturnValue (true, 0.8);
        ch->volumeKnob.setTooltip (txt ("Volumen de este sonido. Arrastra hacia arriba o abajo. "
                                        "Doble clic: valor por defecto."));
        ch->volumeKnob.onValueChange = [this, c, ch]
        {
            engine.setChannelVolume (c, (float) ch->volumeKnob.getValue());
        };
        makeNonFocusing (ch->volumeKnob);
    }

    addAndMakeVisible (grid);
    addAndMakeVisible (helpButton);

    addAndMakeVisible (exampleButton);
    exampleButton.setButtonText (txt ("Beat de ejemplo"));
    exampleButton.setTooltip (txt ("Carga un ritmo de ejemplo para que veas cómo se construye un beat."));
    exampleButton.onClick = [this] { engine.loadExamplePattern(); grid.repaint(); };
    makeNonFocusing (exampleButton);

    addAndMakeVisible (clearButton);
    clearButton.setButtonText ("Limpiar");
    clearButton.setTooltip (txt ("Borra todos los pasos del patrón para empezar desde cero."));
    clearButton.onClick = [this] { engine.clearPattern(); grid.repaint(); };
    makeNonFocusing (clearButton);

    startTimerHz (60);
}

void StepSequencer::timerCallback()
{
    grid.setPlayingStep (engine.getCurrentStep());
}

void StepSequencer::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (Colours::panel);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

    // Cabecera
    g.setColour (Colours::header);
    g.fillRoundedRectangle (headerArea.toFloat(), 6.0f);
    g.fillRect (headerArea.withTrimmedTop (headerArea.getHeight() / 2));
    g.setColour (Colours::text);
    g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    g.drawText (txt ("Secuenciador de patrones"), headerArea.reduced (44, 0), juce::Justification::centredLeft);

    // Números de los pasos (1..16) encima de la cuadrícula
    g.setFont (juce::FontOptions (11.0f));
    for (int s = 0; s < numSteps; ++s)
    {
        const float groupGap = 8.0f;
        const float cellWidth = ((float) grid.getWidth() - groupGap * 3.0f) / (float) numSteps;
        const float x = (float) grid.getX() + (float) s * cellWidth + (float) (s / 4) * groupGap;
        g.setColour (s % 4 == 0 ? Colours::text : Colours::textDim);
        g.drawText (juce::String (s + 1), juce::Rectangle<float> (x, (float) numbersArea.getY(),
                                                                  cellWidth, (float) numbersArea.getHeight()),
                    juce::Justification::centred);
    }

    // Separadores de filas
    g.setColour (Colours::outline.withAlpha (0.5f));
    for (int c = 1; c < numChannels; ++c)
    {
        const int y = grid.getY() + c * rowHeight;
        g.drawHorizontalLine (y, 10.0f, (float) getWidth() - 10.0f);
    }
}

void StepSequencer::resized()
{
    auto area = getLocalBounds();

    headerArea = area.removeFromTop (headerHeight);
    {
        auto h = headerArea.reduced (8, 7);
        helpButton.setBounds (h.removeFromLeft (26));
        clearButton.setBounds (h.removeFromRight (80));
        h.removeFromRight (8);
        exampleButton.setBounds (h.removeFromRight (130));
    }

    area.reduce (10, 4);
    numbersArea = area.removeFromTop (numbersHeight);

    auto rows = area.removeFromTop (rowHeight * numChannels);
    auto controls = rows.removeFromLeft (controlsWidth);
    rows.removeFromLeft (10);
    grid.setBounds (rows);

    for (int c = 0; c < numChannels; ++c)
    {
        auto row = controls.removeFromTop (rowHeight).reduced (0, 6);
        auto* ch = channels[c];
        ch->muteLight.setBounds (row.removeFromLeft (24));
        row.removeFromLeft (6);
        ch->volumeKnob.setBounds (row.removeFromRight (36));
        row.removeFromRight (6);
        ch->nameButton.setBounds (row);
    }
}
}
