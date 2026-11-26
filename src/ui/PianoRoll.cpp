#include "PianoRoll.h"
#include "HelpTexts.h"
#include "model/MidiIO.h"
#include "model/Scales.h"
#include <map>

namespace studoo
{
// ============================================================================
//  PianoRollGrid
// ============================================================================
PianoRollGrid::PianoRollGrid (AppContext& c, PianoRoll& r) : ctx (c), roll (r)
{
    enableLongPressMenu (*this); // en tablets: mantener pulsado = clic derecho
    setTooltip (txt ("Clic en un hueco: añadir nota. Arrastra una nota: moverla. Arrastra su borde derecho: "
                     "cambiar la duración. Clic derecho: borrar. Teclas del piano: escuchar."));
    makeNonFocusing (*this);
}

double PianoRollGrid::stepWidth() const
{
    const int length = Project::getPatternLength (roll.getPattern());
    return (getWidth() - keyboardWidth) / (double) length;
}

double PianoRollGrid::xToSteps (float x) const { return (x - keyboardWidth) / stepWidth(); }
float PianoRollGrid::stepsToX (double steps) const { return (float) (keyboardWidth + steps * stepWidth()); }
int PianoRollGrid::yToNote (float y) const { return juce::jlimit (lowestNote, highestNote, highestNote - (int) (y / rowHeight)); }

double PianoRollGrid::snap (double steps) const
{
    const double s = roll.getSnap();
    return std::floor (steps / s + 1.0e-6) * s;
}

void PianoRollGrid::setPlayhead (double steps)
{
    if (! juce::approximatelyEqual (steps, playhead))
    {
        // Solo se redibuja la franja de la raya verde (donde estaba y donde está)
        auto strip = [this] (double s)
        {
            if (s < 0.0)
                return juce::Rectangle<int>();
            return juce::Rectangle<int> ((int) stepsToX (s) - 10, 0, 20, getHeight());
        };
        repaint (strip (playhead));
        playhead = steps;
        repaint (strip (playhead));
    }
}

juce::ValueTree PianoRollGrid::noteAt (juce::Point<float> p, bool& onRightEdge)
{
    const int uid = roll.getChannelUid();
    for (auto n : roll.getPattern())
    {
        if (! n.hasType (ids::NOTE) || (int) n[ids::channel] != uid)
            continue;
        const float x = stepsToX (n[ids::start]);
        const float w = (float) ((double) n[ids::length] * stepWidth());
        auto r = juce::Rectangle<float> (x, (float) yForNote (n[ids::pitch]), w, (float) rowHeight);
        if (r.contains (p))
        {
            onRightEdge = p.x > r.getRight() - juce::jmin (8.0f, w * 0.4f);
            return n;
        }
    }
    return {};
}

void PianoRollGrid::paint (juce::Graphics& g)
{
    const auto pattern = roll.getPattern();
    const int length = Project::getPatternLength (pattern);
    const double sw = stepWidth();
    const int uid = roll.getChannelUid();
    const auto clip = g.getClipBounds().toFloat();

    // Filas (teclas blancas y negras)
    for (int note = lowestNote; note <= highestNote; ++note)
    {
        const int y = yForNote (note);
        g.setColour (isBlackKey (note) ? juce::Colour (0xff181b20) : juce::Colour (0xff1f2329));
        if (roll.getScaleType() > 0)
        {
            // Con escala: las filas que no son de la escala, más oscuras; la nota base, con color
            if (! roll.isInScale (note))
                g.setColour (juce::Colour (0xff0d0f12));
            else if (((note - roll.getScaleRoot()) % 12 + 12) % 12 == 0)
                g.setColour (juce::Colour (0xff3b2e1a));
            else
                g.setColour (juce::Colour (0xff272c34));
        }
        g.fillRect (keyboardWidth, y, getWidth() - keyboardWidth, rowHeight);
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.drawHorizontalLine (y + rowHeight - 1, (float) keyboardWidth, (float) getWidth());
        if (note % 12 == 0)
        {
            g.setColour (Colours::outline.brighter (0.2f));
            g.drawHorizontalLine (y + rowHeight - 1, (float) keyboardWidth, (float) getWidth());
        }
    }

    // Compases alternos un poco más claros (ayuda a orientarse)
    const int barSteps = ctx.project.getStepsPerBar();
    for (int bar = 1; bar * barSteps < length; bar += 2)
    {
        g.setColour (juce::Colours::white.withAlpha (0.018f));
        g.fillRect (juce::Rectangle<float> (stepsToX (bar * barSteps), 0.0f, (float) (barSteps * sw), (float) getHeight()));
    }

    // Líneas verticales: pasos, tiempos y compases
    for (int s = 0; s <= length; ++s)
    {
        const float x = stepsToX (s);
        g.setColour (s % barSteps == 0 ? Colours::outline.brighter (0.5f) : s % 4 == 0 ? Colours::outline.brighter (0.1f)
                                                                                 : Colours::outline.withAlpha (0.3f));
        g.drawVerticalLine ((int) x, 0.0f, (float) getHeight());
    }

    // Notas fantasma: las de los otros canales, con su color, para orientarte (doble clic = ir a ese canal)
    if (roll.showGhosts())
    {
        std::map<int, juce::Colour> ghostColour;
        for (auto n : pattern)
            if (n.hasType (ids::NOTE) && (int) n[ids::channel] != uid)
            {
                const int other = n[ids::channel];
                if (ghostColour.find (other) == ghostColour.end())
                    ghostColour[other] = Colours::forChannelType (ctx.project.findChannel (other)[ids::type].toString());
                auto r = juce::Rectangle<float> (stepsToX (n[ids::start]), (float) yForNote (n[ids::pitch]) + 1.0f,
                                                 (float) ((double) n[ids::length] * sw), (float) rowHeight - 2.0f);
                g.setColour (ghostColour[other].withAlpha (0.16f));
                g.fillRoundedRectangle (r, 3.0f);
                g.setColour (ghostColour[other].withAlpha (0.35f));
                g.drawRoundedRectangle (r, 3.0f, 1.0f);
            }
    }

    // Notas del canal (con el color de su tipo: naranja, azul, morado...)
    const auto channelType = roll.getChannelType();
    const auto colour = Colours::forChannelType (channelType);
    const bool orange = colour == Colours::accent;
    int ownNotes = 0;
    for (auto n : pattern)
    {
        if (! n.hasType (ids::NOTE) || (int) n[ids::channel] != uid)
            continue;
        auto r = juce::Rectangle<float> (stepsToX (n[ids::start]), (float) yForNote (n[ids::pitch]),
                                         (float) ((double) n[ids::length] * sw), (float) rowHeight).reduced (0.5f, 0.5f);
        const float vel = n.getProperty (ids::velocity, 0.8f);
        ++ownNotes;
        if (! r.expanded (6.0f).intersects (clip))
            continue;   // fuera de la zona que se está redibujando
        Draw::glow (g, r, colour, 4.0f, 3.5f);
        if (orange) Draw::accentFill (g, r, 3.5f);
        else        Draw::colourFill (g, r, colour, 3.5f);
        if (vel < 0.7f)
        {
            g.setColour (juce::Colours::black.withAlpha ((0.7f - vel) * 0.8f));
            g.fillRoundedRectangle (r, 3.5f);
        }
        g.setColour ((orange ? Colours::accentDeep : colour).darker (0.7f));
        g.drawRoundedRectangle (r, 3.5f, 1.0f);
        if (roll.selection.contains (n))
        {
            g.setColour (juce::Colours::white);
            g.drawRoundedRectangle (r.expanded (0.5f), 3.5f, 2.0f);
        }
        // Nota deslizante: una flecha al principio
        if ((bool) n.getProperty (ids::slide, false))
        {
            juce::Path arrow;
            const float ax = r.getX() + 2.0f, ay = r.getBottom() - 2.0f;
            arrow.addTriangle (ax, ay, ax + 8.0f, ay, ax + 8.0f, r.getY() + 2.0f);
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.fillPath (arrow);
        }
        // Panorama distinto del centro: una rayita arriba (izquierda o derecha)
        if (std::abs ((float) n.getProperty (ids::notePan, 0.0f)) > 0.02f)
        {
            const float pan = (float) n.getProperty (ids::notePan, 0.0f);
            g.setColour (juce::Colours::white.withAlpha (0.7f));
            g.fillRect (juce::Rectangle<float> (r.getCentreX() + pan * r.getWidth() * 0.4f - 1.5f, r.getY() + 1.0f, 3.0f, 3.0f));
        }
        // Asa para alargar
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (r.withLeft (r.getRight() - 4.0f).reduced (1.0f, 2.5f), 1.0f);
        if (r.getWidth() > 30.0f)
        {
            g.setColour (colour.darker (0.9f).withMultipliedSaturation (0.6f).darker (0.6f));
            g.setFont (Fonts::bold (10.0f));
            g.drawText (noteName (n[ids::pitch]), r.reduced (4.0f, 0.0f), juce::Justification::centredLeft);
        }
    }

    // Teclado (fijo a la izquierda de lo que se ve)
    juce::Graphics::ScopedSaveState keyboardState (g);
    g.addTransform (juce::AffineTransform::translation (keyboardLeft(), 0.0f));
    // Teclado: teclas blancas
    auto keys = juce::Rectangle<float> (0.0f, 0.0f, (float) keyboardWidth, (float) getHeight());
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc9cdd3), 0.0f, 0.0f, juce::Colour (0xfff4f5f7), (float) keyboardWidth * 0.8f, 0.0f, false));
    g.fillRect (keys);
    for (int note = lowestNote; note <= highestNote; ++note)
    {
        const int y = yForNote (note);
        const int n = ((note % 12) + 12) % 12;
        g.setColour (juce::Colour (0xff8a9099));
        if (isBlackKey (note))
            g.drawHorizontalLine (y + rowHeight / 2, 0.0f, (float) keyboardWidth);       // la tecla blanca termina a mitad de la negra
        else if (n == 4 || n == 11)
            g.drawHorizontalLine (y, 0.0f, (float) keyboardWidth);                       // entre Mi-Fa y Si-Do
        if (n == 0)
        {
            g.setColour (juce::Colour (0xff5a616b));
            g.setFont (Fonts::bold (9.5f));
            g.drawText (noteName (note), juce::Rectangle<int> (0, y, keyboardWidth - 5, rowHeight), juce::Justification::centredRight);
        }
    }
    // Teclas negras
    for (int note = lowestNote; note <= highestNote; ++note)
    {
        if (! isBlackKey (note))
            continue;
        auto key = juce::Rectangle<float> (0.0f, (float) yForNote (note) + 1.0f, (float) keyboardWidth * 0.6f, (float) rowHeight - 2.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (key.translated (1.5f, 1.0f), 2.5f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3f46), 0.0f, 0.0f, juce::Colour (0xff0e1013), key.getRight(), 0.0f, false));
        g.fillRoundedRectangle (key, 2.5f);
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.fillRoundedRectangle (key.withTrimmedLeft (key.getWidth() - 6.0f).reduced (1.0f, 2.0f), 1.5f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.5f), (float) keyboardWidth, 0.0f,
                                             juce::Colours::transparentBlack, (float) keyboardWidth + 8.0f, 0.0f, false));
    g.fillRect ((float) keyboardWidth, 0.0f, 8.0f, (float) getHeight());
    g.addTransform (juce::AffineTransform::translation (-keyboardLeft(), 0.0f));

    // Sin notas: un pequeño cartel que explica qué hacer
    if (ownNotes == 0)
        if (auto* vp = findParentComponentOfClass<juce::Viewport>())
        {
            auto view = vp->getViewArea().toFloat().withTrimmedLeft ((float) keyboardWidth);
            auto card = view.withSizeKeepingCentre (380.0f, 92.0f);
            Draw::shadow (g, card, 12.0f, 16);
            g.setColour (Colours::panel.withAlpha (0.94f));
            g.fillRoundedRectangle (card, 12.0f);
            g.setColour (colour.withAlpha (0.6f));
            g.drawRoundedRectangle (card.reduced (0.5f), 12.0f, 1.2f);
            auto inner = card.reduced (18.0f, 14.0f);
            g.setColour (Colours::text);
            g.setFont (Fonts::bold (15.0f));
            g.drawText (txt ("Haz clic en la cuadrícula para poner notas"), inner.removeFromTop (24.0f), juce::Justification::centred);
            g.setColour (Colours::textDim);
            g.setFont (Fonts::regular (12.5f));
            g.drawFittedText (txt ("Arrastra una nota para moverla y su borde derecho para alargarla. "
                                   "Clic derecho la borra. Usa \"Acorde\" para poner varias a la vez."),
                              inner.toNearestInt(), juce::Justification::centred, 3, 1.0f);
        }

    // Rectángulo de selección
    if (dragMode == DragMode::select)
    {
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRect (selectionBox);
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawRect (selectionBox, 1.0f);
    }

    // Cursor de reproducción
    if (playhead >= 0.0)
    {
        auto line = juce::Rectangle<float> (stepsToX (playhead) - 1.0f, 0.0f, 2.0f, (float) getHeight());
        Draw::glow (g, line, Colours::green, 5.0f, 1.0f);
        g.setColour (Colours::green);
        g.fillRect (line);
    }
}

float PianoRollGrid::keyboardLeft() const
{
    if (auto* vp = findParentComponentOfClass<juce::Viewport>())
        return (float) vp->getViewPositionX();
    return 0.0f;
}

void PianoRollGrid::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    // Ctrl + rueda: zoom horizontal (como en los programas de música)
    if (e.mods.isCommandDown() || e.mods.isCtrlDown())
    {
        roll.setZoom (roll.getZoom() * (wheel.deltaY > 0 ? 1.25f : 0.8f), e.position.x);
        return;
    }
    Component::mouseWheelMove (e, wheel);
}

void PianoRollGrid::mouseMove (const juce::MouseEvent& e)
{
    bool edge = false;
    auto n = noteAt (e.position, edge);
    setMouseCursor (n.isValid() ? (edge ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::DraggingHandCursor)
                                : juce::MouseCursor::NormalCursor);
}

void PianoRollGrid::mouseDown (const juce::MouseEvent& e)
{
    dragMode = DragMode::none;
    const int uid = roll.getChannelUid();
    if (uid == 0)
        return;

    // Teclado: escuchar la nota (y grabarla si está grabando)
    if (e.position.x < keyboardLeft() + keyboardWidth)
    {
        keyboardNote = yToNote (e.position.y);
        ctx.engine.previewNote (uid, keyboardNote, 0.8f, 0.5);
        if (roll.isRecordingNotes())
            roll.liveNote (keyboardNote, true, 0.8f);
        return;
    }

    bool edge = false;
    auto note = noteAt (e.position, edge);
    ctx.project.beginTransaction();

    if (e.mods.isPopupMenu())
    {
        if (note.isValid() && roll.selection.contains (note))
            roll.deleteSelection();
        else if (note.isValid())
            roll.getPattern().removeChild (note, ctx.project.um());
        return;
    }

    const double steps = xToSteps (e.position.x);
    const bool selecting = roll.selectMode || e.mods.isCommandDown() || e.mods.isCtrlDown();

    // Doble clic en una nota fantasma: pasar a editar ese canal
    if (e.getNumberOfClicks() >= 2 && roll.showGhosts())
    {
        const int pitch = yToNote (e.position.y);
        for (auto n : roll.getPattern())
            if (n.hasType (ids::NOTE) && (int) n[ids::channel] != uid && (int) n[ids::pitch] == pitch
                && steps >= (double) n[ids::start] && steps < (double) n[ids::start] + (double) n[ids::length])
            {
                // quita la nota que puso el primer clic
                for (int i = roll.getPattern().getNumChildren(); --i >= 0;)
                {
                    auto c = roll.getPattern().getChild (i);
                    if (c.hasType (ids::NOTE) && (int) c[ids::channel] == uid && (int) c[ids::pitch] == pitch
                        && std::abs ((double) c[ids::start] - snap (steps)) < 1.0e-6)
                    {
                        roll.getPattern().removeChild (i, ctx.project.um());
                        break;
                    }
                }
                ctx.project.setSelectedChannel (n[ids::channel]);
                return;
            }
    }

    // Ctrl (o modo Seleccionar): clic en una nota la añade/quita de la selección; en un hueco, rectángulo
    if (selecting && ! edge)
    {
        if (note.isValid())
        {
            if (roll.selection.contains (note)) roll.selection.removeAllInstancesOf (note);
            else                                roll.selection.add (note);
            repaint();
            if (roll.selection.contains (note))
            {
                // y se puede arrastrar enseguida todo el grupo
                movingGroup.clear();
                for (auto& n : roll.selection)
                    movingGroup.push_back ({ n, (double) n[ids::start], (int) n[ids::pitch] });
                dragNote = note;
                dragMode = DragMode::move;
                dragStartSteps = steps;
                dragStartNote = yToNote (e.position.y);
            }
            return;
        }
        if (! e.mods.isShiftDown())
            roll.selection.clear();
        dragMode = DragMode::select;
        selectionBox = { e.position, e.position };
        repaint();
        return;
    }

    // Clic en una nota seleccionada: se mueven todas las seleccionadas juntas
    movingGroup.clear();
    if (note.isValid() && ! edge && roll.selection.contains (note))
    {
        for (auto& n : roll.selection)
            movingGroup.push_back ({ n, (double) n[ids::start], (int) n[ids::pitch] });
    }
    else if (! roll.selection.isEmpty())
    {
        roll.clearSelection();
    }

    if (! note.isValid())
    {
        // Añadir nota (o acorde)
        const int pitch = roll.snapToScale (yToNote (e.position.y));
        const double start = snap (steps);
        const int length = Project::getPatternLength (roll.getPattern());
        if (start >= length)
            return;
        const double len = juce::jmin (lastLength, length - start);
        for (int interval : roll.getChordIntervals())
        {
            const int p = interval == 0 ? pitch : roll.snapToScale (pitch + interval);
            auto added = ctx.project.addNote (roll.getPattern(), uid, start, len, p);
            if (roll.slideMode)
                added.setProperty (ids::slide, true, ctx.project.um());
            if (interval == 0)
                note = added;
            ctx.engine.previewNote (uid, p, 0.8f, 0.3);
        }
        edge = false;
    }
    else
    {
        ctx.engine.previewNote (uid, note[ids::pitch], 0.8f, 0.3);
    }

    dragNote = note;
    dragMode = edge ? DragMode::resize : DragMode::move;
    dragStartSteps = steps;
    dragStartNote = yToNote (e.position.y);
    dragNoteStart = note[ids::start];
    dragNoteLength = note[ids::length];
    dragNotePitch = note[ids::pitch];
}

void PianoRollGrid::mouseDrag (const juce::MouseEvent& e)
{
    if (dragMode == DragMode::select)
    {
        const auto old = selectionBox;
        selectionBox = juce::Rectangle<float> (e.mouseDownPosition, e.position);
        repaint (old.getUnion (selectionBox).expanded (4.0f).toNearestInt());
        return;
    }
    if (dragMode == DragMode::none || ! dragNote.isValid())
        return;

    const int length = Project::getPatternLength (roll.getPattern());
    const double delta = xToSteps (e.position.x) - dragStartSteps;

    if (dragMode == DragMode::move && movingGroup.size() > 1)
    {
        // Mover todo el grupo, sin que ninguna nota se salga del patrón
        const double s = roll.getSnap();
        double d = std::round (delta / s) * s;
        int dp = yToNote (e.position.y) - dragStartNote;
        for (auto& o : movingGroup)
        {
            d = juce::jlimit (-o.start, length - s - o.start, d);
            dp = juce::jlimit (lowestNote - o.pitch, highestNote - o.pitch, dp);
        }
        for (auto& o : movingGroup)
        {
            o.note.setProperty (ids::start, o.start + d, ctx.project.um());
            o.note.setProperty (ids::pitch, o.pitch + dp, ctx.project.um());
        }
        return;
    }

    if (dragMode == DragMode::resize)
    {
        const double s = roll.getSnap();
        const double newLength = juce::jlimit (s, length - dragNoteStart, std::round ((dragNoteLength + delta) / s) * s);
        dragNote.setProperty (ids::length, newLength, ctx.project.um());
        lastLength = newLength;
    }
    else
    {
        const double s = roll.getSnap();
        const double newStart = juce::jlimit (0.0, length - s, std::round ((dragNoteStart + delta) / s) * s);
        const int newPitch = roll.snapToScale (juce::jlimit (lowestNote, highestNote, dragNotePitch + (yToNote (e.position.y) - dragStartNote)));
        if ((int) dragNote[ids::pitch] != newPitch)
            ctx.engine.previewNote (roll.getChannelUid(), newPitch, 0.7f, 0.15);
        dragNote.setProperty (ids::start, newStart, ctx.project.um());
        dragNote.setProperty (ids::pitch, newPitch, ctx.project.um());
    }
}

void PianoRollGrid::mouseUp (const juce::MouseEvent&)
{
    if (keyboardNote >= 0)
    {
        if (roll.isRecordingNotes())
            roll.liveNote (keyboardNote, false, 0.0f);
        keyboardNote = -1;
        return;
    }
    if (dragMode == DragMode::select)
    {
        // Selecciona las notas del canal que tocan el rectángulo
        const int uid = roll.getChannelUid();
        for (auto n : roll.getPattern())
            if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid)
            {
                auto r = juce::Rectangle<float> (stepsToX (n[ids::start]), (float) yForNote (n[ids::pitch]),
                                                 (float) ((double) n[ids::length] * stepWidth()), (float) rowHeight);
                if (r.intersects (selectionBox) && ! roll.selection.contains (n))
                    roll.selection.add (n);
            }
        dragMode = DragMode::none;
        repaint();
        return;
    }
    movingGroup.clear();
    if (dragMode == DragMode::move && dragNote.isValid())
        lastLength = dragNote[ids::length];
    dragMode = DragMode::none;
    dragNote = {};
}

// ============================================================================
//  PianoRoll
// ============================================================================
//  VelocityLane
// ============================================================================
VelocityLane::VelocityLane (AppContext& c, PianoRoll& r) : ctx (c), roll (r)
{
    setTooltip (txt ("Carril de notas: lo fuerte que suena cada nota (o su panorama, afinación fina o modulación: "
                     "pulsa el nombre de la izquierda para cambiar). Arrastra las barras hacia arriba o abajo; "
                     "si arrastras de lado, cambias varias notas seguidas. Doble clic: valor normal."));
    makeNonFocusing (*this);
}

namespace
{
    struct LaneInfo { const juce::Identifier* id; float lo, hi, def; bool bipolar; const char* label; };
    LaneInfo laneInfo (int mode)
    {
        switch (mode)
        {
            case 1:  return { &ids::notePan, -1.0f, 1.0f, 0.0f, true, "PANORAMA" };
            case 2:  return { &ids::noteFine, -100.0f, 100.0f, 0.0f, true, "AFINAR" };
            case 3:  return { &ids::noteMod, 0.0f, 1.0f, 0.0f, false, "MODULAR" };
            default: return { &ids::velocity, 0.05f, 1.0f, 0.8f, false, "VELOCITY" };
        }
    }
}

void VelocityLane::showModeMenu()
{
    juce::PopupMenu m;
    m.addSectionHeader (txt ("Qué quieres cambiar de cada nota"));
    m.addItem (1, txt ("Fuerza (velocity)"), true, mode == 0);
    m.addItem (2, txt ("Panorama (izquierda / derecha)"), true, mode == 1);
    m.addItem (3, txt ("Afinación fina (cents)"), true, mode == 2);
    m.addItem (4, txt ("Modulación (abre el filtro del sinte)"), true, mode == 3);
    juce::Component::SafePointer<VelocityLane> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this), [safe] (int r)
    {
        if (safe != nullptr && r > 0) { safe->mode = r - 1; safe->repaint(); }
    });
}

void VelocityLane::paint (juce::Graphics& g)
{
    const auto info = laneInfo (mode);
    g.fillAll (juce::Colour (0xff15181c));
    auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
    g.setColour (Colours::outline.withAlpha (0.4f));
    for (float f : { 0.25f, 0.5f, 0.75f })
        g.drawHorizontalLine ((int) (area.getBottom() - f * area.getHeight()), 0.0f, (float) getWidth());

    auto label = juce::Rectangle<int> (2, 2, PianoRollGrid::keyboardWidth - 4, getHeight() - 26);
    g.setColour (Colours::outline.withAlpha (0.5f));
    g.fillRoundedRectangle (label.toFloat(), 4.0f);
    g.setColour (mode == 0 ? Colours::textDim : Colours::accent);
    g.setFont (Fonts::bold (10.0f));
    g.drawFittedText (juce::String (info.label) + juce::String::fromUTF8 (" \xe2\x96\xbe"), label, juce::Justification::centred, 2);

    const int uid = roll.getChannelUid();
    const auto colour = Colours::forChannelType (roll.getChannelType());
    const float zeroY = info.bipolar ? area.getCentreY() : area.getBottom();
    for (auto n : roll.getPattern())
    {
        if (! n.hasType (ids::NOTE) || (int) n[ids::channel] != uid)
            continue;
        const float x = roll.gridStepsToX (n[ids::start]) - (float) roll.viewX();
        if (x < (float) PianoRollGrid::keyboardWidth - 2.0f || x > (float) getWidth())
            continue;
        const float v = juce::jlimit (info.lo, info.hi, (float) n.getProperty (*info.id, info.def));
        const float norm = (v - info.lo) / (info.hi - info.lo);
        const float top = area.getBottom() - norm * area.getHeight();
        g.setColour (colour.withAlpha (0.35f + 0.65f * (info.bipolar ? std::abs (norm - 0.5f) * 2.0f : norm)));
        g.fillRect (juce::Rectangle<float> (x, juce::jmin (top, zeroY), 3.0f, std::abs (zeroY - top)));
        g.fillEllipse (x - 3.0f, top - 4.5f, 9.0f, 9.0f);
    }
}

void VelocityLane::mouseDown (const juce::MouseEvent& e)
{
    if (e.position.x < PianoRollGrid::keyboardWidth && e.position.y < getHeight() - 24)
    {
        showModeMenu();
        return;
    }
    ctx.project.beginTransaction();
    if (e.getNumberOfClicks() >= 2)
    {
        const auto info = laneInfo (mode);
        auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
        const float norm = (info.def - info.lo) / (info.hi - info.lo);
        setVelocityAt ({ e.position.x, area.getBottom() - norm * area.getHeight() });
        return;
    }
    setVelocityAt (e.position);
}

void VelocityLane::mouseDrag (const juce::MouseEvent& e)
{
    if (e.mouseDownPosition.x >= PianoRollGrid::keyboardWidth)
        setVelocityAt (e.position);
}

void VelocityLane::setVelocityAt (juce::Point<float> p)
{
    const auto info = laneInfo (mode);
    auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
    const float norm = juce::jlimit (0.0f, 1.0f, (area.getBottom() - p.y) / juce::jmax (1.0f, area.getHeight()));
    float value = info.lo + norm * (info.hi - info.lo);
    if (info.bipolar && std::abs (norm - 0.5f) < 0.04f)
        value = 0.0f;   // se "pega" al centro
    if (mode == 2)
        value = (float) juce::roundToInt (value);
    const int uid = roll.getChannelUid();

    // La nota (o acorde) cuyo inicio está más cerca del ratón, a menos de 10 píxeles
    float best = 10.0f;
    double bestStart = -1.0;
    for (auto n : roll.getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid)
        {
            const float d = std::abs (roll.gridStepsToX (n[ids::start]) - (float) roll.viewX() - p.x);
            if (d < best) { best = d; bestStart = n[ids::start]; }
        }
    if (bestStart < 0.0)
        return;
    for (auto n : roll.getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid
            && juce::approximatelyEqual ((double) n[ids::start], bestStart))
        {
            if (mode != 0 && juce::approximatelyEqual (value, info.def))
                n.removeProperty (*info.id, ctx.project.um());
            else
                n.setProperty (*info.id, value, ctx.project.um());
        }
    repaint();
    roll.getGridComponent().repaint();
}

// ============================================================================
PianoRoll::PianoRoll (AppContext& c)
    : ctx (c), helpButton (txt (help::pianoRollTitle), txt (help::pianoRollText)), grid (c, *this),
      velocityLane (c, *this)
{
    addAndMakeVisible (helpButton);

    addAndMakeVisible (channelBox);
    channelBox.setTooltip (txt ("El canal (instrumento) cuyas notas estás editando."));
    channelBox.onChange = [this]
    {
        const int i = channelBox.getSelectedId() - 1;
        if (juce::isPositiveAndBelow (i, channelUids.size()))
            ctx.project.setSelectedChannel (channelUids[i]);
    };
    makeNonFocusing (channelBox);

    addAndMakeVisible (snapBox);
    snapBox.addItem (txt ("Ajuste: 1/4 de paso"), 1);
    snapBox.addItem (txt ("Ajuste: 1/2 paso"), 2);
    snapBox.addItem (txt ("Ajuste: 1 paso"), 3);
    snapBox.addItem (txt ("Ajuste: 2 pasos"), 4);
    snapBox.addItem (txt ("Ajuste: 1 tiempo"), 5);
    snapBox.setSelectedId (3, juce::dontSendNotification);
    snapBox.setTooltip (txt ("A qué cuadrícula se \"pegan\" las notas al colocarlas y moverlas."));
    makeNonFocusing (snapBox);

    addAndMakeVisible (chordBox);
    chordBox.addItem (txt ("Nota sola"), 1);
    chordBox.addItem (txt ("Acorde mayor (alegre)"), 2);
    chordBox.addItem (txt ("Acorde menor (triste)"), 3);
    chordBox.addItem (txt ("Séptima"), 4);
    chordBox.addItem (txt ("Menor séptima"), 5);
    chordBox.addItem ("Sus4", 6);
    chordBox.addItem (txt ("Octava"), 7);
    chordBox.setSelectedId (1, juce::dontSendNotification);
    chordBox.setTooltip (txt ("Con un acorde elegido, cada clic añade todas sus notas a la vez."));
    makeNonFocusing (chordBox);

    addAndMakeVisible (patternLabel);
    patternLabel.setColour (juce::Label::textColourId, Colours::textDim);

    addAndMakeVisible (clearButton);
    clearButton.setButtonText (txt ("Borrar notas"));
    clearButton.setTooltip (txt ("Borra todas las notas de este canal en el patrón actual."));
    clearButton.onClick = [this]
    {
        ctx.project.beginTransaction();
        auto p = getPattern();
        for (int i = p.getNumChildren(); --i >= 0;)
            if (p.getChild (i).hasType (ids::NOTE) && (int) p.getChild (i)[ids::channel] == getChannelUid())
                p.removeChild (i, ctx.project.um());
    };
    makeNonFocusing (clearButton);

    addAndMakeVisible (recordButton);
    recordButton.setButtonText (txt ("● Grabar notas"));
    recordButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffd63c3c));
    recordButton.setTooltip (txt ("Graba lo que tocas en un teclado MIDI (o en las teclas del piano de la izquierda) "
                                  "dentro de este patrón, mientras suena. Pulsa Espacio o este botón para parar."));
    recordButton.onClick = [this] { setRecording (! recording); };
    makeNonFocusing (recordButton);

    addAndMakeVisible (toolsButton);
    toolsButton.setButtonText (txt ("Herramientas"));
    toolsButton.setTooltip (txt ("Copiar, pegar, duplicar, transponer, arpegiador, generador de ideas, humanizar, "
                                 "unir y partir notas, deslizar, notas fantasma y escalas."));
    toolsButton.onClick = [this] { showToolsMenu(); };
    makeNonFocusing (toolsButton);

    addAndMakeVisible (selectButton);
    selectButton.setButtonText (txt ("Seleccionar"));
    selectButton.setClickingTogglesState (true);
    selectButton.setTooltip (txt ("Activado: arrastra para seleccionar varias notas (o mantén Ctrl). Luego muévelas todas "
                                  "juntas, cópialas o bórralas."));
    selectButton.onClick = [this] { selectMode = selectButton.getToggleState(); };
    makeNonFocusing (selectButton);

    addAndMakeVisible (slideButton);
    slideButton.setButtonText ("Slide");
    slideButton.setClickingTogglesState (true);
    slideButton.setTooltip (txt ("Activado: las notas que pongas se deslizan desde la nota anterior (como el 808 del trap). "
                                 "Las notas deslizantes llevan una flecha. Para cambiar notas que ya están: Herramientas > Deslizar."));
    slideButton.onClick = [this] { slideMode = slideButton.getToggleState(); };
    makeNonFocusing (slideButton);

    addAndMakeVisible (quantizeButton);
    quantizeButton.setButtonText ("Cuantizar");
    quantizeButton.setTooltip (txt ("Cuantizar: coloca todas las notas de este canal justo en la cuadrícula del \"Ajuste\". "
                                    "Arregla las notas que grabaste o pusiste un poco fuera de tiempo."));
    quantizeButton.onClick = [this] { quantize(); };
    makeNonFocusing (quantizeButton);

    addAndMakeVisible (velocityLane);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&grid, false);
    viewport.setScrollBarsShown (true, true);
    viewport.onScroll = [this] { grid.repaint(); velocityLane.repaint(); };

    for (auto* b : { &zoomOutButton, &zoomInButton })
    {
        addAndMakeVisible (*b);
        makeNonFocusing (*b);
    }
    zoomOutButton.setButtonText (txt ("−"));
    zoomInButton.setButtonText ("+");
    zoomOutButton.setTooltip (txt ("Alejar: ver más compases (también Ctrl + rueda del ratón)."));
    zoomInButton.setTooltip (txt ("Acercar: ver las notas más grandes (también Ctrl + rueda del ratón)."));
    zoomOutButton.onClick = [this] { setZoom (zoom / 1.5f); };
    zoomInButton.onClick = [this] { setZoom (zoom * 1.5f); };

    ctx.project.addChangeListener (this);
    refreshChannels();
    startTimerHz (30);
}

PianoRoll::~PianoRoll()
{
    ctx.project.removeChangeListener (this);
}

int PianoRoll::getChannelUid() const
{
    const int uid = ctx.project.getSelectedChannel();
    return channelUids.contains (uid) ? uid : (channelUids.isEmpty() ? 0 : channelUids[0]);
}

juce::String PianoRoll::getChannelType() const
{
    return ctx.project.findChannel (getChannelUid())[ids::type].toString();
}

juce::ValueTree PianoRoll::getPattern()
{
    return ctx.project.getCurrentPattern();
}

double PianoRoll::getSnap() const
{
    switch (snapBox.getSelectedId())
    {
        case 1:  return 0.25;
        case 2:  return 0.5;
        case 4:  return 2.0;
        case 5:  return 4.0;
        default: return 1.0;
    }
}

juce::Array<int> PianoRoll::getChordIntervals() const
{
    switch (chordBox.getSelectedId())
    {
        case 2:  return { 0, 4, 7 };
        case 3:  return { 0, 3, 7 };
        case 4:  return { 0, 4, 7, 10 };
        case 5:  return { 0, 3, 7, 10 };
        case 6:  return { 0, 5, 7 };
        case 7:  return { 0, 12 };
        default: return { 0 };
    }
}

void PianoRoll::updateGridSize()
{
    const int rows = PianoRollGrid::highestNote - PianoRollGrid::lowestNote + 1;
    const int visibleW = viewport.getWidth() - viewport.getScrollBarThickness();
    const float bars4 = juce::jmax (1.0f, Project::getPatternLength (getPattern()) / 64.0f);
    const int notesW = juce::roundToInt ((float) (visibleW - PianoRollGrid::keyboardWidth) * zoom * bars4);
    const int w = PianoRollGrid::keyboardWidth + juce::jmax (visibleW - PianoRollGrid::keyboardWidth, notesW);
    if (grid.getWidth() != w || grid.getHeight() != rows * PianoRollGrid::rowHeight)
        grid.setSize (w, rows * PianoRollGrid::rowHeight);
}

void PianoRoll::setZoom (float newZoom, float anchorX)
{
    newZoom = juce::jlimit (0.25f, 16.0f, newZoom);
    if (juce::approximatelyEqual (newZoom, zoom))
        return;
    // Mantiene en su sitio el punto de la canción que hay debajo del ratón (o el centro)
    const float anchor = anchorX >= 0.0f ? anchorX : (float) viewport.getViewPositionX() + viewport.getViewWidth() * 0.5f;
    const double stepAtAnchor = grid.xToSteps (anchor);
    const float offsetInView = anchor - (float) viewport.getViewPositionX();
    zoom = newZoom;
    updateGridSize();
    viewport.setViewPosition (juce::jmax (0, juce::roundToInt (grid.stepsToX (stepAtAnchor) - offsetInView)), viewport.getViewPositionY());
    grid.repaint();
    velocityLane.repaint();
}

void PianoRoll::changeListenerCallback (juce::ChangeBroadcaster*)
{
    updateGridSize();
    refreshChannels();
    grid.repaint();
    velocityLane.repaint();
}

void PianoRoll::refreshChannels()
{
    channelUids.clear();
    channelBox.clear (juce::dontSendNotification);
    for (auto ch : ctx.project.channels())
    {
        if (ch[ids::type].toString() == "audio")
            continue;
        channelUids.add (ch[ids::uid]);
        channelBox.addItem (ch[ids::name].toString(), channelUids.size());
    }
    channelBox.setSelectedId (channelUids.indexOf (getChannelUid()) + 1, juce::dontSendNotification);
    patternLabel.setText (txt ("Patrón: ") + getPattern()[ids::name].toString(), juce::dontSendNotification);

    if (getChannelUid() != lastChannel)
    {
        lastChannel = getChannelUid();
        scrollToNotes();
    }
}

void PianoRoll::scrollToNotes()
{
    // Centramos la vista en las notas del canal (o en su nota base)
    int centre = ctx.project.findChannel (getChannelUid()).getProperty (ids::root, 60);
    for (auto n : getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == getChannelUid())
        {
            centre = n[ids::pitch];
            break;
        }
    const int y = grid.yForNote (centre) - viewport.getHeight() / 2;
    viewport.setViewPosition (viewport.getViewPositionX(), juce::jmax (0, y));
}

void PianoRoll::visibilityChanged()
{
    if (isVisible())
        scrollToNotes();
}

void PianoRoll::timerCallback()
{
    if (recording)
    {
        if (! ctx.engine.isPlaying())
            setRecording (false);   // al parar, se acaba la grabación
        else
            for (auto& r : ctx.engine.takeRecordedMidi())
                handleRecorded (r);
    }
    const bool show = ctx.engine.isPlaying() && ! ctx.engine.isSongMode();
    grid.setPlayhead (show ? ctx.engine.getPlayPosition() : -1.0);
}

bool PianoRollGrid::isSelected (const juce::ValueTree& n) const { return roll.selection.contains (n); }

// ----------------------------------------------------------------------------
//  Selección, copiar y pegar, escalas, arpegiador
// ----------------------------------------------------------------------------
namespace
{
    struct ClipNote { double start, length; int pitch; float velocity; juce::NamedValueSet extras; };

    // Copia los ajustes de una nota (panorama, afinación, modulación, deslizar) a otra
    void copyNoteExtras (const juce::ValueTree& from, juce::ValueTree to, juce::UndoManager* um)
    {
        for (auto* id : { &ids::notePan, &ids::noteFine, &ids::noteMod, &ids::slide })
            if (from.hasProperty (*id))
                to.setProperty (*id, from[*id], um);
    }
    std::vector<ClipNote>& clipboard() { static std::vector<ClipNote> c; return c; }

}

juce::StringArray PianoRoll::scaleNames()
{
    return { txt ("Sin escala"), txt ("Mayor (alegre)"), txt ("Menor (triste)"), txt ("Menor armónica (oriental)"),
             txt ("Pentatónica menor"), txt ("Pentatónica mayor"), "Blues", txt ("Dórica (lo-fi, jazz)"),
             txt ("Frigia (drill, flamenco)") };
}

int PianoRoll::getScaleType() const { return juce::jlimit (0, scaleNames().size() - 1, (int) ctx.project.getState().getProperty (ids::scaleType, 0)); }
int PianoRoll::getScaleRoot() const { return juce::jlimit (0, 11, (int) ctx.project.getState().getProperty (ids::scaleRoot, 0)); }

bool PianoRoll::isInScale (int pitch) const
{
    const int type = getScaleType();
    if (type == 0)
        return true;
    return scales::contains (type, getScaleRoot(), pitch);
}

int PianoRoll::snapToScale (int pitch) const
{
    if (getScaleType() == 0 || ! (bool) ctx.project.getState().getProperty (ids::scaleLock, true))
        return pitch;
    for (int d = 0; d <= 6; ++d)
    {
        if (isInScale (pitch - d)) return pitch - d;   // primero hacia abajo: así suena a "la tecla de al lado"
        if (isInScale (pitch + d)) return pitch + d;
    }
    return pitch;
}

void PianoRoll::pruneSelection()
{
    const int uid = getChannelUid();
    auto pattern = getPattern();
    for (int i = selection.size(); --i >= 0;)
        if (selection[i].getParent() != pattern || (int) selection[i][ids::channel] != uid)
            selection.remove (i);
}

void PianoRoll::selectAll()
{
    selection.clear();
    const int uid = getChannelUid();
    for (auto n : getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid)
            selection.add (n);
    grid.repaint();
}

void PianoRoll::clearSelection()
{
    selection.clear();
    grid.repaint();
}

juce::Array<juce::ValueTree> PianoRoll::selectedOrAll()
{
    pruneSelection();
    if (! selection.isEmpty())
        return selection;
    juce::Array<juce::ValueTree> all;
    const int uid = getChannelUid();
    for (auto n : getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid)
            all.add (n);
    return all;
}

void PianoRoll::copySelection()
{
    pruneSelection();
    if (selection.isEmpty())
        return;
    clipboard().clear();
    for (auto& n : selection)
    {
        juce::NamedValueSet extras;
        for (auto* id : { &ids::notePan, &ids::noteFine, &ids::noteMod, &ids::slide })
            if (n.hasProperty (*id))
                extras.set (*id, n[*id]);
        clipboard().push_back ({ (double) n[ids::start], (double) n[ids::length], (int) n[ids::pitch],
                                 (float) n.getProperty (ids::velocity, 0.8f), extras });
    }
}

void PianoRoll::cutSelection()
{
    copySelection();
    deleteSelection();
}

void PianoRoll::paste()
{
    if (clipboard().empty() || getChannelUid() == 0)
        return;
    ctx.project.beginTransaction();
    const int length = Project::getPatternLength (getPattern());
    selection.clear();
    for (auto& c : clipboard())
        if (c.start < length)
        {
            auto added = ctx.project.addNote (getPattern(), getChannelUid(), c.start, juce::jmin (c.length, length - c.start),
                                              c.pitch, c.velocity);
            for (int k = 0; k < c.extras.size(); ++k)
                added.setProperty (c.extras.getName (k), c.extras.getValueAt (k), ctx.project.um());
            selection.add (added);
        }
    grid.repaint();
}

void PianoRoll::duplicateSelection()
{
    auto notes = selectedOrAll();
    if (notes.isEmpty())
        return;
    double first = 1.0e9, last = 0.0;
    for (auto& n : notes)
    {
        first = juce::jmin (first, (double) n[ids::start]);
        last = juce::jmax (last, (double) n[ids::start] + (double) n[ids::length]);
    }
    const double span = std::ceil ((last - first) / 4.0 - 1.0e-6) * 4.0;   // en tiempos enteros
    const int length = Project::getPatternLength (getPattern());
    ctx.project.beginTransaction();
    juce::Array<juce::ValueTree> added;
    for (auto& n : notes)
    {
        const double start = (double) n[ids::start] + span;
        if (start < length)
        {
            auto copy = ctx.project.addNote (getPattern(), getChannelUid(), start,
                                             juce::jmin ((double) n[ids::length], length - start), n[ids::pitch],
                                             (float) n.getProperty (ids::velocity, 0.8f));
            copyNoteExtras (n, copy, ctx.project.um());
            added.add (copy);
        }
    }
    selection = added;
    grid.repaint();
}

void PianoRoll::deleteSelection()
{
    pruneSelection();
    if (selection.isEmpty())
        return;
    ctx.project.beginTransaction();
    for (auto& n : selection)
        getPattern().removeChild (n, ctx.project.um());
    selection.clear();
    grid.repaint();
}

void PianoRoll::transpose (int semitones)
{
    auto notes = selectedOrAll();
    if (notes.isEmpty())
        return;
    for (auto& n : notes)   // si alguna se sale del teclado, no se mueve ninguna
    {
        const int p = (int) n[ids::pitch] + semitones;
        if (p < PianoRollGrid::lowestNote || p > PianoRollGrid::highestNote)
            return;
    }
    ctx.project.beginTransaction();
    for (auto& n : notes)
        n.setProperty (ids::pitch, (int) n[ids::pitch] + semitones, ctx.project.um());
}

void PianoRoll::arpeggiate (int style)
{
    // Cada acorde (notas que empiezan a la vez) se convierte en notas sueltas una detrás de otra
    auto notes = selectedOrAll();
    if (notes.isEmpty())
        return;
    std::map<double, std::vector<juce::ValueTree>> chords;
    for (auto& n : notes)
        chords[(double) n[ids::start]].push_back (n);

    const double s = getSnap();
    const int length = Project::getPatternLength (getPattern());
    const int uid = getChannelUid();
    juce::Random rnd (1234);
    ctx.project.beginTransaction();
    selection.clear();
    for (auto& [start, chord] : chords)
    {
        if (chord.size() < 2)
            continue;
        std::vector<int> pitches;
        double chordLength = 0.0;
        float velocity = 0.8f;
        for (auto& n : chord)
        {
            pitches.push_back (n[ids::pitch]);
            chordLength = juce::jmax (chordLength, (double) n[ids::length]);
            velocity = n.getProperty (ids::velocity, 0.8f);
            getPattern().removeChild (n, ctx.project.um());
        }
        std::sort (pitches.begin(), pitches.end());
        std::vector<int> order;
        if (style == 1)      order.assign (pitches.rbegin(), pitches.rend());
        else if (style == 2) { order = pitches; for (int i = (int) pitches.size() - 2; i > 0; --i) order.push_back (pitches[(size_t) i]); }
        else                 order = pitches;

        int i = 0;
        for (double t = start; t < start + chordLength - 1.0e-6 && t < length; t += s, ++i)
        {
            const int pitch = style == 3 ? pitches[(size_t) rnd.nextInt ((int) pitches.size())]
                                         : order[(size_t) i % order.size()];
            selection.add (ctx.project.addNote (getPattern(), uid, t, juce::jmin (s, length - t), pitch, velocity));
        }
    }
    grid.repaint();
}

bool PianoRoll::showGhosts() const
{
    return (bool) ctx.project.getState().getProperty (ids::ghostNotes, true);
}

void PianoRoll::toggleSlide()
{
    auto notes = selectedOrAll();
    if (notes.isEmpty())
        return;
    bool allSlide = true;
    for (auto& n : notes)
        allSlide = allSlide && (bool) n.getProperty (ids::slide, false);
    ctx.project.beginTransaction();
    for (auto& n : notes)
    {
        if (allSlide) n.removeProperty (ids::slide, ctx.project.um());
        else          n.setProperty (ids::slide, true, ctx.project.um());
    }
    grid.repaint();
}

void PianoRoll::humanize (bool timing, bool velocity)
{
    // Mueve un poquito cada nota (tiempo y fuerza) para que no suene "a máquina"
    auto notes = selectedOrAll();
    if (notes.isEmpty())
        return;
    const int length = Project::getPatternLength (getPattern());
    juce::Random rnd;
    ctx.project.beginTransaction();
    for (auto& n : notes)
    {
        if (timing)
        {
            const double start = juce::jlimit (0.0, length - 0.05, (double) n[ids::start] + (rnd.nextDouble() - 0.5) * 0.16);
            n.setProperty (ids::start, start, ctx.project.um());
        }
        if (velocity)
        {
            const float v = juce::jlimit (0.1f, 1.0f, (float) n.getProperty (ids::velocity, 0.8f) + (rnd.nextFloat() - 0.5f) * 0.24f);
            n.setProperty (ids::velocity, v, ctx.project.um());
        }
    }
    grid.repaint();
}

void PianoRoll::glueNotes()
{
    // Une las notas de la misma altura que van seguidas (la una empieza donde acaba la otra)
    auto notes = selectedOrAll();
    std::map<int, std::vector<juce::ValueTree>> byPitch;
    for (auto& n : notes)
        byPitch[(int) n[ids::pitch]].push_back (n);
    ctx.project.beginTransaction();
    selection.clear();
    for (auto& [pitch, list] : byPitch)
    {
        std::sort (list.begin(), list.end(), [] (const juce::ValueTree& a, const juce::ValueTree& b)
                   { return (double) a[ids::start] < (double) b[ids::start]; });
        juce::ValueTree current;
        for (auto& n : list)
        {
            if (current.isValid())
            {
                const double end = (double) current[ids::start] + (double) current[ids::length];
                if ((double) n[ids::start] <= end + 0.01)
                {
                    const double newEnd = juce::jmax (end, (double) n[ids::start] + (double) n[ids::length]);
                    current.setProperty (ids::length, newEnd - (double) current[ids::start], ctx.project.um());
                    getPattern().removeChild (n, ctx.project.um());
                    continue;
                }
            }
            current = n;
            selection.add (n);
        }
    }
    grid.repaint();
}

void PianoRoll::splitNotes (int parts)
{
    // Parte cada nota en 'parts' trozos iguales (0 = en trozos del tamaño del ajuste)
    auto notes = selectedOrAll();
    if (notes.isEmpty())
        return;
    const int uid = getChannelUid();
    ctx.project.beginTransaction();
    selection.clear();
    for (auto& n : notes)
    {
        const double start = n[ids::start], len = n[ids::length];
        const double piece = parts > 0 ? len / parts : getSnap();
        if (piece < 0.05 || len <= piece + 1.0e-6)
        {
            selection.add (n);
            continue;
        }
        const int count = juce::jmin (64, (int) std::ceil (len / piece - 1.0e-6));
        n.setProperty (ids::length, piece, ctx.project.um());
        selection.add (n);
        for (int k = 1; k < count; ++k)
        {
            const double t = start + piece * k;
            auto copy = ctx.project.addNote (getPattern(), uid, t, juce::jmin (piece, start + len - t), n[ids::pitch],
                                             (float) n.getProperty (ids::velocity, 0.8f));
            copyNoteExtras (n, copy, ctx.project.um());
            copy.removeProperty (ids::slide, ctx.project.um());
            selection.add (copy);
        }
    }
    grid.repaint();
}

void PianoRoll::generateIdea (int kind)
{
    // Generador de ideas: crea una melodía, un bajo, acordes o un arpegio con las notas de la escala.
    // Cada vez sale distinto; Ctrl+Z para volver atrás y probar otra.
    const int uid = getChannelUid();
    if (uid == 0)
        return;
    const int scaleType = getScaleType() > 0 ? getScaleType() : 2;   // sin escala: menor
    const int root = getScaleRoot();
    const auto& iv = scales::intervals (scaleType);
    const int degrees = (int) iv.size();
    auto pitchOf = [&] (int degree, int baseOctave)
    {
        const int oct = (int) std::floor ((double) degree / degrees);
        const int d = ((degree % degrees) + degrees) % degrees;
        return juce::jlimit (PianoRollGrid::lowestNote, PianoRollGrid::highestNote, 12 * baseOctave + root + iv[(size_t) d] + 12 * oct);
    };
    const int length = juce::jmin (Project::getPatternLength (getPattern()), 64);
    const int barSteps = ctx.project.getStepsPerBar();
    juce::Random rnd;
    ctx.project.beginTransaction();
    auto p = getPattern();
    for (int i = p.getNumChildren(); --i >= 0;)
        if (p.getChild (i).hasType (ids::NOTE) && (int) p.getChild (i)[ids::channel] == uid)
            p.removeChild (i, ctx.project.um());
    selection.clear();
    auto add = [&] (double start, double len, int pitch, float vel)
    {
        if (start < length)
            selection.add (ctx.project.addNote (p, uid, start, juce::jmin (len, length - start), pitch, vel));
    };

    // Progresión de acordes (grados de la escala): una por compás
    static const int progressions[][4] = { { 0, 5, 3, 4 }, { 0, 3, 4, 3 }, { 0, 4, 5, 3 }, { 5, 3, 0, 4 }, { 0, 2, 5, 4 }, { 0, 5, 2, 6 } };
    const auto* prog = progressions[rnd.nextInt (6)];
    const bool isBass = kind == 1 || getChannelType() == "bass808";
    const int octave = isBass ? 3 : (kind == 0 ? 5 : 4);

    for (int bar = 0; bar * barSteps < length; ++bar)
    {
        const int chordRoot = prog[bar % 4];
        const double t0 = bar * barSteps;
        if (kind == 2)
        {
            // Acordes: tres notas de la escala (1-3-5) durante el compás
            for (int k : { 0, 2, 4 })
                add (t0, barSteps, pitchOf (chordRoot + k, octave), 0.7f);
        }
        else if (kind == 3)
        {
            // Arpegio: las notas del acorde una detrás de otra
            static const int shape[] = { 0, 2, 4, 7, 4, 2 };
            for (int s = 0; s < barSteps; s += 2)
                add (t0 + s, 2.0, pitchOf (chordRoot + shape[(s / 2) % 6], octave), s % 4 == 0 ? 0.85f : 0.65f);
        }
        else if (isBass)
        {
            // Bajo: la nota base del acorde con un ritmo que se repite
            static const int rhythms[][8] = { { 0, 3, 6, 8, 11, 14, -1, -1 }, { 0, 6, 10, -1, -1, -1, -1, -1 }, { 0, 4, 8, 12, -1, -1, -1, -1 } };
            const auto* r = rhythms[rnd.nextInt (3)];
            for (int k = 0; k < 8 && r[k] >= 0 && r[k] < barSteps; ++k)
            {
                const int next = (k + 1 < 8 && r[k + 1] >= 0) ? r[k + 1] : barSteps;
                const int deg = chordRoot + (rnd.nextFloat() < 0.2f ? 4 : 0);
                add (t0 + r[k], juce::jmax (1, next - r[k]), pitchOf (deg, octave), k == 0 ? 0.9f : 0.75f);
            }
        }
        else
        {
            // Melodía: pasos cortos por la escala, apoyándose en las notas del acorde
            int degree = chordRoot + 2 * rnd.nextInt (3);
            for (int s = 0; s < barSteps;)
            {
                const int len = rnd.nextFloat() < 0.5f ? 2 : (rnd.nextFloat() < 0.5f ? 1 : 4);
                if (rnd.nextFloat() < 0.18f && s > 0) { s += len; continue; }   // un silencio
                add (t0 + s, len, pitchOf (degree, octave), s % 4 == 0 ? 0.85f : 0.7f);
                const int move = rnd.nextInt (5) - 2;   // -2..2 grados
                degree = juce::jlimit (chordRoot - 3, chordRoot + 7, degree + move);
                s += len;
            }
        }
    }
    grid.repaint();
}

bool PianoRoll::handleKey (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const int code = key.getKeyCode();
    const auto upper = juce::CharacterFunctions::toUpperCase ((juce::juce_wchar) code);
    const bool cmd = mods.isCommandDown();
    if (cmd && upper == 'A') { selectAll(); return true; }
    if (cmd && upper == 'C') { copySelection(); return true; }
    if (cmd && upper == 'X') { cutSelection(); return true; }
    if (cmd && upper == 'V') { paste(); return true; }
    if (cmd && upper == 'B') { duplicateSelection(); return true; }
    if (code == juce::KeyPress::deleteKey || code == juce::KeyPress::backspaceKey)
    {
        pruneSelection();
        if (selection.isEmpty())
            return false;
        deleteSelection();
        return true;
    }
    if (code == juce::KeyPress::escapeKey && ! selection.isEmpty()) { clearSelection(); return true; }
    pruneSelection();
    if (selection.isEmpty())
        return false;   // las flechas solo mueven notas seleccionadas
    if (code == juce::KeyPress::upKey)   { transpose (mods.isShiftDown() ? 12 : 1); return true; }
    if (code == juce::KeyPress::downKey) { transpose (mods.isShiftDown() ? -12 : -1); return true; }
    return false;
}

void PianoRoll::showToolsMenu()
{
    pruneSelection();
    const bool hasSel = ! selection.isEmpty();
    juce::PopupMenu menu;
    menu.addSectionHeader (hasSel ? juce::String (selection.size()) + txt (" notas seleccionadas") : txt ("Sin selección: afecta a todas las notas"));
    menu.addItem (1, txt ("Seleccionar todo (Ctrl+A)"));
    menu.addItem (2, txt ("Copiar (Ctrl+C)"), hasSel);
    menu.addItem (3, txt ("Cortar (Ctrl+X)"), hasSel);
    menu.addItem (4, txt ("Pegar (Ctrl+V)"), ! clipboard().empty());
    menu.addItem (5, txt ("Duplicar detrás (Ctrl+B)"));
    menu.addItem (6, txt ("Borrar seleccionadas (Supr)"), hasSel);
    menu.addItem (7, txt ("Borrar todas las notas de este canal"));
    menu.addSeparator();
    menu.addItem (10, txt ("Subir 1 semitono (↑)"));
    menu.addItem (11, txt ("Bajar 1 semitono (↓)"));
    menu.addItem (12, txt ("Subir una octava (Mayús+↑)"));
    menu.addItem (13, txt ("Bajar una octava (Mayús+↓)"));
    menu.addSeparator();
    juce::PopupMenu arp;
    arp.addItem (20, txt ("Subiendo"));
    arp.addItem (21, txt ("Bajando"));
    arp.addItem (22, txt ("Sube y baja"));
    arp.addItem (23, txt ("Al azar"));
    menu.addSubMenu (txt ("Arpegiador (convierte los acordes en notas sueltas)"), arp);
    juce::PopupMenu idea;
    idea.addItem (40, txt ("Melodía"));
    idea.addItem (41, txt ("Línea de bajo"));
    idea.addItem (42, txt ("Acordes"));
    idea.addItem (43, txt ("Arpegio"));
    menu.addSubMenu (txt ("Generar una idea (con la escala; borra las notas de este canal)"), idea);
    menu.addSeparator();
    menu.addItem (50, txt ("Humanizar: mover un poco el tiempo y la fuerza"));
    menu.addItem (51, txt ("Fuerza al azar"));
    menu.addItem (52, txt ("Unir notas seguidas (pegar)"));
    menu.addItem (53, txt ("Partir notas en 2"));
    menu.addItem (54, txt ("Partir notas en trozos del ajuste"));
    menu.addItem (55, txt ("Deslizar (slide): activar o quitar"));
    menu.addItem (56, txt ("Ver las notas de otros canales (fantasma)"), true, showGhosts());

    juce::PopupMenu scale, root;
    const auto names = scaleNames();
    for (int i = 0; i < names.size(); ++i)
        scale.addItem (100 + i, names[i], true, i == getScaleType());
    static const char* roots[] = { "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si" };
    for (int i = 0; i < 12; ++i)
        root.addItem (200 + i, roots[i], true, i == getScaleRoot());
    menu.addSeparator();
    menu.addItem (30, txt ("Exportar este patrón a MIDI (.mid)"));
    menu.addItem (31, txt ("Importar MIDI en este canal..."));
    menu.addSeparator();
    menu.addSubMenu (txt ("Escala"), scale);
    menu.addSubMenu (txt ("Nota base de la escala"), root, getScaleType() > 0);
    menu.addItem (300, txt ("Pegar las notas a la escala"), getScaleType() > 0,
                  (bool) ctx.project.getState().getProperty (ids::scaleLock, true));

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&toolsButton), [this] (int r)
    {
        auto& st = ctx.project.getState();
        switch (r)
        {
            case 1: selectAll(); break;
            case 2: copySelection(); break;
            case 3: cutSelection(); break;
            case 4: paste(); break;
            case 5: duplicateSelection(); break;
            case 6: deleteSelection(); break;
            case 7: clearButton.onClick(); selection.clear(); break;
            case 10: transpose (1); break;
            case 11: transpose (-1); break;
            case 12: transpose (12); break;
            case 13: transpose (-12); break;
            case 20: case 21: case 22: case 23: arpeggiate (r - 20); break;
            case 40: case 41: case 42: case 43: generateIdea (r - 40); break;
            case 50: humanize (true, true); break;
            case 51: humanize (false, true); break;
            case 52: glueNotes(); break;
            case 53: splitNotes (2); break;
            case 54: splitNotes (0); break;
            case 55: toggleSlide(); break;
            case 56: st.setProperty (ids::ghostNotes, ! showGhosts(), nullptr); break;
            case 30: exportMidi(); break;
            case 31: importMidi(); break;
            case 300: st.setProperty (ids::scaleLock, ! (bool) st.getProperty (ids::scaleLock, true), ctx.project.um()); break;
            default:
                if (r >= 100 && r < 100 + scaleNames().size())
                {
                    ctx.project.beginTransaction();
                    st.setProperty (ids::scaleType, r - 100, ctx.project.um());
                }
                else if (r >= 200 && r < 212)
                {
                    ctx.project.beginTransaction();
                    st.setProperty (ids::scaleRoot, r - 200, ctx.project.um());
                }
                break;
        }
        grid.repaint();
    });
}

void PianoRoll::setRecording (bool on)
{
    recording = on;
    recordButton.setToggleState (on, juce::dontSendNotification);
    openNotes.clear();
    ctx.engine.takeRecordedMidi();   // descarta lo que hubiera de antes
    ctx.engine.setMidiRecording (on);
    if (on)
    {
        ctx.project.beginTransaction();   // todo lo grabado se deshace de una vez
        ctx.project.getState().setProperty (ids::playMode, "pattern", nullptr);
        if (! ctx.engine.isPlaying())
            ctx.engine.play();
    }
}

void PianoRoll::liveNote (int pitch, bool on, float velocity)
{
    if (! recording || ! ctx.engine.isPlaying())
        return;
    handleRecorded ({ on, pitch, velocity, ctx.engine.getPlayPosition() });
}

void PianoRoll::handleRecorded (const AudioEngine::RecordedMidi& r)
{
    const int uid = getChannelUid();
    if (uid == 0)
        return;
    const double length = Project::getPatternLength (getPattern());
    if (r.noteOn)
    {
        openNotes[r.note] = { r.position, juce::jlimit (0.05f, 1.0f, r.velocity) };
        return;
    }
    auto it = openNotes.find (r.note);
    if (it == openNotes.end())
        return;
    double start = std::round (it->second.first * 4.0) / 4.0;   // al cuarto de paso más cercano
    if (start >= length) start = 0.0;
    double len = r.position - it->second.first;
    if (len <= 0.0) len += length;                                // la nota cruzó el final del patrón
    len = juce::jlimit (0.25, length - start, std::round (len * 4.0) / 4.0);
    const int pitch = juce::jlimit (PianoRollGrid::lowestNote, PianoRollGrid::highestNote, r.note);
    ctx.project.addNote (getPattern(), uid, start, len, pitch, it->second.second);
    openNotes.erase (it);
}

void PianoRoll::exportMidi()
{
    auto midi = midiio::patternToMidi (ctx.project, getPattern());
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Studoo").getChildFile ("Exportados");
    dir.createDirectory();
    auto name = juce::File::createLegalFileName (ctx.project.getDisplayName() + " - " + getPattern()[ids::name].toString());
    auto file = dir.getNonexistentChildFile (name, ".mid", false);
    bool ok = false;
    {
        juce::FileOutputStream out (file);
        ok = out.openedOk() && midi.writeTo (out);
    }
    if (! ok)
    {
        juce::AlertWindow::showAsync (juce::MessageBoxOptions().withTitle (txt ("No se pudo exportar"))
                                          .withMessage (txt ("No se pudo escribir el archivo MIDI.")).withButton ("Vale"), nullptr);
        return;
    }
    juce::Component::SafePointer<PianoRoll> safe (this);
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::NoIcon)
                                      .withTitle (txt ("¡MIDI exportado!"))
                                      .withMessage (txt ("Se ha guardado en:\n") + file.getFullPathName()
                                                    + txt ("\n\nÁbrelo en cualquier programa de música (cada canal es una pista)."))
                                     #if JUCE_ANDROID || JUCE_IOS
                                      .withButton (txt ("Compartir"))
                                     #else
                                      .withButton (txt ("Mostrar en la carpeta"))
                                     #endif
                                      .withButton (txt ("Cerrar")),
                                  [safe, file] (int result)
    {
        if (safe != nullptr && result == 1 && safe->ctx.shareFile)
            safe->ctx.shareFile (file);
    });
}

void PianoRoll::importMidi()
{
    if (getChannelUid() == 0)
        return;
    chooser = std::make_unique<juce::FileChooser> (txt ("Elige un archivo MIDI"), juce::File(), "*.mid;*.midi");
    juce::Component::SafePointer<PianoRoll> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [safe] (const juce::FileChooser& fc)
    {
        if (safe == nullptr)
            return;
        auto files = chosenFiles (fc, juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                          .getChildFile ("Studoo").getChildFile ("Importados"));
        if (files.isEmpty())
            return;
        juce::MidiFile midi;
        {
            juce::FileInputStream in (files[0]);
            // Un .mid normal ocupa pocos KB: más de 4 MB no es un MIDI de verdad
            if (! in.openedOk() || in.getTotalLength() > 4 * 1024 * 1024 || ! midi.readFrom (in))
            {
                juce::AlertWindow::showAsync (juce::MessageBoxOptions().withTitle (txt ("No se pudo importar"))
                                                  .withMessage (txt ("Ese archivo no parece un MIDI válido.")).withButton ("Vale"), nullptr);
                return;
            }
        }
        safe->ctx.project.beginTransaction();
        const int count = midiio::importIntoPattern (safe->ctx.project, safe->getPattern(), safe->getChannelUid(), midi);
        juce::AlertWindow::showAsync (juce::MessageBoxOptions().withTitle (count > 0 ? txt ("MIDI importado") : txt ("Sin notas"))
                                          .withMessage (count > 0 ? juce::String (count) + txt (" notas añadidas a este canal (como mucho 16 compases).")
                                                                  : txt ("El archivo no tiene notas que se puedan usar."))
                                          .withButton ("Vale"), nullptr);
        safe->scrollToNotes();
    });
}

float PianoRoll::gridStepsToX (double steps) const { return grid.stepsToX (steps); }
double PianoRoll::gridXToSteps (float x) const { return grid.xToSteps (x); }

void PianoRoll::quantize()
{
    const double s = getSnap();
    const int uid = getChannelUid();
    const int length = Project::getPatternLength (getPattern());
    ctx.project.beginTransaction();
    for (auto n : getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid)
        {
            const double start = juce::jlimit (0.0, juce::jmax (0.0, length - s), std::round ((double) n[ids::start] / s) * s);
            const double len = juce::jmax (s, std::round ((double) n[ids::length] / s) * s);
            n.setProperty (ids::start, start, ctx.project.um());
            n.setProperty (ids::length, len, ctx.project.um());
        }
}

void PianoRoll::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, "Piano roll");
}

void PianoRoll::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (42);
    auto h = headerArea.reduced (8, 7);
    helpButton.setBounds (h.removeFromLeft (26));
    // Anchos ideales; si no cabe todo (pantallas pequeñas), se encoge todo a la vez
    constexpr int natural = 110 + 170 + 170 + 200 + 96 + 120 + 104 + 70 + 124 + 9 * 8;
    const bool roomForTitle = h.getWidth() >= natural;
    const float k = juce::jlimit (0.55f, 1.0f, (float) h.getWidth() / (float) (natural - (roomForTitle ? 0 : 110)));
    auto w = [k] (int px) { return juce::roundToInt ((float) px * k); };
    h.removeFromLeft (roomForTitle ? 110 : 8);
    channelBox.setBounds (h.removeFromLeft (w (170))); h.removeFromLeft (8);
    snapBox.setBounds (h.removeFromLeft (w (170))); h.removeFromLeft (8);
    chordBox.setBounds (h.removeFromLeft (w (200))); h.removeFromLeft (8);
    clearButton.setVisible (false);   // "Borrar notas" está en Herramientas
    quantizeButton.setBounds (h.removeFromRight (w (96))); h.removeFromRight (8);
    toolsButton.setBounds (h.removeFromRight (w (120))); h.removeFromRight (8);
    selectButton.setBounds (h.removeFromRight (w (104))); h.removeFromRight (8);
    slideButton.setBounds (h.removeFromRight (w (70))); h.removeFromRight (8);
    recordButton.setBounds (h.removeFromRight (w (124))); h.removeFromRight (8);
    patternLabel.setBounds (h);
    patternLabel.setVisible (h.getWidth() > 60);

    area.reduce (6, 6);
    auto lane = area.removeFromBottom (area.getHeight() > 300 ? 64 : 44);
    area.removeFromBottom (4);
    viewport.setBounds (area);
    velocityLane.setBounds (lane.withTrimmedRight (viewport.getScrollBarThickness()));
    // Zoom: dos botones pequeños en la esquina del carril de velocity
    auto zoomArea = lane.withWidth (PianoRollGrid::keyboardWidth).removeFromBottom (22).reduced (3, 1);
    zoomOutButton.setBounds (zoomArea.removeFromLeft (zoomArea.getWidth() / 2 - 1));
    zoomArea.removeFromLeft (2);
    zoomInButton.setBounds (zoomArea);
    updateGridSize();
}
}
