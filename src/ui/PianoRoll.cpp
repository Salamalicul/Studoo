#include "PianoRoll.h"
#include "HelpTexts.h"

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
    for (int bar = 1; bar * 16 < length; bar += 2)
    {
        g.setColour (juce::Colours::white.withAlpha (0.018f));
        g.fillRect (juce::Rectangle<float> (stepsToX (bar * 16), 0.0f, (float) (16.0 * sw), (float) getHeight()));
    }

    // Líneas verticales: pasos, tiempos y compases
    for (int s = 0; s <= length; ++s)
    {
        const float x = stepsToX (s);
        g.setColour (s % 16 == 0 ? Colours::outline.brighter (0.5f) : s % 4 == 0 ? Colours::outline.brighter (0.1f)
                                                                                 : Colours::outline.withAlpha (0.3f));
        g.drawVerticalLine ((int) x, 0.0f, (float) getHeight());
    }

    // Notas de otros canales (fantasma) para orientarte
    for (auto n : pattern)
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] != uid)
        {
            g.setColour (juce::Colours::white.withAlpha (0.07f));
            g.fillRoundedRectangle (stepsToX (n[ids::start]), (float) yForNote (n[ids::pitch]) + 1.0f,
                                    (float) ((double) n[ids::length] * sw), (float) rowHeight - 2.0f, 3.0f);
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

    // Cursor de reproducción
    if (playhead >= 0.0)
    {
        auto line = juce::Rectangle<float> (stepsToX (playhead) - 1.0f, 0.0f, 2.0f, (float) getHeight());
        Draw::glow (g, line, Colours::green, 5.0f, 1.0f);
        g.setColour (Colours::green);
        g.fillRect (line);
    }
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

    // Teclado: escuchar la nota
    if (e.x < keyboardWidth)
    {
        ctx.engine.previewNote (uid, yToNote (e.position.y), 0.8f, 0.5);
        return;
    }

    bool edge = false;
    auto note = noteAt (e.position, edge);
    ctx.project.beginTransaction();

    if (e.mods.isPopupMenu())
    {
        if (note.isValid())
            roll.getPattern().removeChild (note, ctx.project.um());
        return;
    }

    const double steps = xToSteps (e.position.x);
    if (! note.isValid())
    {
        // Añadir nota (o acorde)
        const int pitch = yToNote (e.position.y);
        const double start = snap (steps);
        const int length = Project::getPatternLength (roll.getPattern());
        if (start >= length)
            return;
        const double len = juce::jmin (lastLength, length - start);
        for (int interval : roll.getChordIntervals())
        {
            auto added = ctx.project.addNote (roll.getPattern(), uid, start, len, pitch + interval);
            if (interval == 0)
                note = added;
            ctx.engine.previewNote (uid, pitch + interval, 0.8f, 0.3);
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
    if (dragMode == DragMode::none || ! dragNote.isValid())
        return;

    const int length = Project::getPatternLength (roll.getPattern());
    const double delta = xToSteps (e.position.x) - dragStartSteps;

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
        const int newPitch = juce::jlimit (lowestNote, highestNote, dragNotePitch + (yToNote (e.position.y) - dragStartNote));
        if ((int) dragNote[ids::pitch] != newPitch)
            ctx.engine.previewNote (roll.getChannelUid(), newPitch, 0.7f, 0.15);
        dragNote.setProperty (ids::start, newStart, ctx.project.um());
        dragNote.setProperty (ids::pitch, newPitch, ctx.project.um());
    }
}

void PianoRollGrid::mouseUp (const juce::MouseEvent&)
{
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
    setTooltip (txt ("Velocity: lo fuerte que suena cada nota. Arrastra las barras hacia arriba (más fuerte) "
                     "o hacia abajo (más suave). Si arrastras de lado, cambias varias notas seguidas."));
    makeNonFocusing (*this);
}

void VelocityLane::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff15181c));
    auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
    g.setColour (Colours::outline.withAlpha (0.4f));
    for (float f : { 0.25f, 0.5f, 0.75f })
        g.drawHorizontalLine ((int) (area.getBottom() - f * area.getHeight()), 0.0f, (float) getWidth());

    g.setColour (Colours::textDim);
    g.setFont (Fonts::bold (11.0f));
    g.drawFittedText ("VELOCITY", juce::Rectangle<int> (0, 0, PianoRollGrid::keyboardWidth, getHeight()),
                      juce::Justification::centred, 2);

    const int uid = roll.getChannelUid();
    const auto colour = Colours::forChannelType (roll.getChannelType());
    for (auto n : roll.getPattern())
    {
        if (! n.hasType (ids::NOTE) || (int) n[ids::channel] != uid)
            continue;
        const float x = roll.gridStepsToX (n[ids::start]);
        const float vel = juce::jlimit (0.0f, 1.0f, (float) n.getProperty (ids::velocity, 0.8f));
        const float top = area.getBottom() - vel * area.getHeight();
        g.setColour (colour.withAlpha (0.35f + 0.65f * vel));
        g.fillRect (juce::Rectangle<float> (x, top, 3.0f, area.getBottom() - top));
        g.fillEllipse (x - 3.0f, top - 4.5f, 9.0f, 9.0f);
    }
}

void VelocityLane::mouseDown (const juce::MouseEvent& e)
{
    ctx.project.beginTransaction();
    setVelocityAt (e.position);
}

void VelocityLane::mouseDrag (const juce::MouseEvent& e) { setVelocityAt (e.position); }

void VelocityLane::setVelocityAt (juce::Point<float> p)
{
    auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
    const float vel = juce::jlimit (0.05f, 1.0f, (area.getBottom() - p.y) / juce::jmax (1.0f, area.getHeight()));
    const int uid = roll.getChannelUid();

    // La nota (o acorde) cuyo inicio está más cerca del ratón, a menos de 10 píxeles
    float best = 10.0f;
    double bestStart = -1.0;
    for (auto n : roll.getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid)
        {
            const float d = std::abs (roll.gridStepsToX (n[ids::start]) - p.x);
            if (d < best) { best = d; bestStart = n[ids::start]; }
        }
    if (bestStart < 0.0)
        return;
    for (auto n : roll.getPattern())
        if (n.hasType (ids::NOTE) && (int) n[ids::channel] == uid
            && juce::approximatelyEqual ((double) n[ids::start], bestStart))
            n.setProperty (ids::velocity, vel, ctx.project.um());
    repaint();
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

    addAndMakeVisible (quantizeButton);
    quantizeButton.setButtonText ("Cuantizar");
    quantizeButton.setTooltip (txt ("Cuantizar: coloca todas las notas de este canal justo en la cuadrícula del \"Ajuste\". "
                                    "Arregla las notas que grabaste o pusiste un poco fuera de tiempo."));
    quantizeButton.onClick = [this] { quantize(); };
    makeNonFocusing (quantizeButton);

    addAndMakeVisible (velocityLane);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&grid, false);
    viewport.setScrollBarsShown (true, false);

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

void PianoRoll::changeListenerCallback (juce::ChangeBroadcaster*)
{
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
    viewport.setViewPosition (0, juce::jmax (0, y));
}

void PianoRoll::visibilityChanged()
{
    if (isVisible())
        scrollToNotes();
}

void PianoRoll::timerCallback()
{
    const bool show = ctx.engine.isPlaying() && ! ctx.engine.isSongMode();
    grid.setPlayhead (show ? ctx.engine.getPlayPosition() : -1.0);
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
    h.removeFromLeft (110);
    channelBox.setBounds (h.removeFromLeft (170)); h.removeFromLeft (8);
    snapBox.setBounds (h.removeFromLeft (170)); h.removeFromLeft (8);
    chordBox.setBounds (h.removeFromLeft (200)); h.removeFromLeft (8);
    clearButton.setBounds (h.removeFromRight (110)); h.removeFromRight (8);
    quantizeButton.setBounds (h.removeFromRight (96)); h.removeFromRight (8);
    patternLabel.setBounds (h);

    area.reduce (6, 6);
    auto lane = area.removeFromBottom (area.getHeight() > 300 ? 64 : 44);
    area.removeFromBottom (4);
    viewport.setBounds (area);
    velocityLane.setBounds (lane.withTrimmedRight (viewport.getScrollBarThickness()));
    const int rows = PianoRollGrid::highestNote - PianoRollGrid::lowestNote + 1;
    grid.setSize (viewport.getWidth() - viewport.getScrollBarThickness(), rows * PianoRollGrid::rowHeight);
}
}
