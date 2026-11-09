#include "Playlist.h"
#include "HelpTexts.h"

namespace studoo
{
// ============================================================================
//  PlaylistGrid
// ============================================================================
PlaylistGrid::PlaylistGrid (AppContext& c, Playlist& p) : ctx (c), playlist (p)
{
    setTooltip (txt ("Clic en un hueco: colocar el patrón elegido arriba. Arrastra un bloque: moverlo. Borde derecho: "
                     "alargarlo. Clic derecho: borrar. Doble clic: editar el patrón. Clic en la regla: mover el cursor."));
    makeNonFocusing (*this);
}

double PlaylistGrid::getPixelsPerStep() const { return playlist.getZoom() / stepsPerBar; }
double PlaylistGrid::xToSteps (float x) const  { return (x - labelWidth) / getPixelsPerStep(); }
float PlaylistGrid::stepsToX (double s) const  { return (float) (labelWidth + s * getPixelsPerStep()); }
int PlaylistGrid::yToTrack (float y) const     { return juce::jlimit (0, numPlaylistTracks - 1, (int) ((y - rulerHeight) / trackHeight)); }

double PlaylistGrid::snap (double steps) const
{
    const double s = playlist.getSnap();
    return std::floor (steps / s + 1.0e-6) * s;
}

int PlaylistGrid::getTotalBars()
{
    return juce::jmax (32, (int) (ctx.project.getSongLengthSteps() / stepsPerBar) + 8);
}

juce::Rectangle<float> PlaylistGrid::clipBounds (const juce::ValueTree& clip) const
{
    const float x = stepsToX (clip[ids::start]);
    const float w = (float) ((double) clip[ids::length] * getPixelsPerStep());
    const float y = (float) (rulerHeight + (int) clip[ids::track] * trackHeight);
    return { x, y + 2.0f, w, (float) trackHeight - 4.0f };
}

juce::ValueTree PlaylistGrid::clipAt (juce::Point<float> p, bool& onRightEdge)
{
    auto pl = ctx.project.playlist();
    for (int i = pl.getNumChildren(); --i >= 0;)
    {
        auto clip = pl.getChild (i);
        auto r = clipBounds (clip);
        if (r.contains (p))
        {
            onRightEdge = p.x > r.getRight() - juce::jmin (8.0f, r.getWidth() * 0.3f);
            return clip;
        }
    }
    return {};
}

void PlaylistGrid::paint (juce::Graphics& g)
{
    const double pps = getPixelsPerStep();
    const int bars = getTotalBars();

    // Pistas
    for (int t = 0; t < numPlaylistTracks; ++t)
    {
        const int y = rulerHeight + t * trackHeight;
        g.setColour (t % 2 == 0 ? juce::Colour (0xff2a3036) : juce::Colour (0xff262b31));
        g.fillRect (labelWidth, y, getWidth() - labelWidth, trackHeight);
        g.setColour (Colours::header);
        g.fillRect (0, y, labelWidth, trackHeight);
        g.setColour (Colours::textDim);
        g.setFont (juce::FontOptions (12.0f));
        g.drawText ("Pista " + juce::String (t + 1), juce::Rectangle<int> (8, y, labelWidth - 8, trackHeight), juce::Justification::centredLeft);
        g.setColour (Colours::outline.withAlpha (0.6f));
        g.drawHorizontalLine (y + trackHeight - 1, 0.0f, (float) getWidth());
    }

    // Compases y tiempos
    for (int b = 0; b <= bars; ++b)
    {
        const float x = stepsToX (b * stepsPerBar);
        g.setColour (Colours::outline.brighter (0.2f));
        g.drawVerticalLine ((int) x, (float) rulerHeight, (float) getHeight());
        if (pps * stepsPerBeat > 10.0)
            for (int beat = 1; beat < 4; ++beat)
            {
                g.setColour (Colours::outline.withAlpha (0.35f));
                g.drawVerticalLine ((int) (x + beat * stepsPerBeat * pps), (float) rulerHeight, (float) getHeight());
            }
    }

    // Regla
    g.setColour (Colours::header);
    g.fillRect (0, 0, getWidth(), rulerHeight);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    for (int b = 0; b < bars; ++b)
    {
        const float x = stepsToX (b * stepsPerBar);
        g.setColour (Colours::textDim);
        g.drawText (juce::String (b + 1), juce::Rectangle<float> (x + 4.0f, 0.0f, 40.0f, (float) rulerHeight), juce::Justification::centredLeft);
        g.drawVerticalLine ((int) x, 12.0f, (float) rulerHeight);
    }
    const double songEnd = ctx.project.getSongLengthSteps();
    g.setColour (Colours::accent.withAlpha (0.25f));
    g.fillRect (stepsToX (0.0), (float) rulerHeight - 4.0f, (float) (songEnd * pps), 4.0f);

    // Bloques (clips)
    for (auto clip : ctx.project.playlist())
    {
        auto r = clipBounds (clip);
        const bool audio = clip[ids::kind].toString() == "audio";
        juce::String name;
        juce::Colour colour = Colours::green;

        if (audio)
        {
            name = ctx.project.findChannel (clip[ids::ref])[ids::name].toString();
        }
        else
        {
            auto pattern = ctx.project.findPattern (clip[ids::ref]);
            name = pattern[ids::name].toString();
            colour = juce::Colour ((juce::uint32) (juce::int64) pattern.getProperty (ids::colour, (juce::int64) 0xffff8a1f));
        }

        g.setColour (colour.withAlpha (0.35f));
        g.fillRoundedRectangle (r, 3.0f);
        g.setColour (colour);
        g.fillRoundedRectangle (r.withHeight (14.0f), 3.0f);
        g.drawRoundedRectangle (r, 3.0f, 1.0f);
        g.setColour (juce::Colours::black);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (name, r.withHeight (14.0f).reduced (4.0f, 0.0f), juce::Justification::centredLeft);

        // Mini vista de las notas del patrón
        if (! audio)
        {
            auto pattern = ctx.project.findPattern (clip[ids::ref]);
            const double len = Project::getPatternLength (pattern);
            auto body = r.withTrimmedTop (16.0f).reduced (2.0f, 2.0f);
            g.setColour (colour.brighter (0.4f).withAlpha (0.8f));
            const int channels = juce::jmax (1, ctx.project.channels().getNumChildren());
            for (double rep = 0.0; rep < (double) clip[ids::length]; rep += len)
                for (auto item : pattern)
                {
                    const int row = ctx.project.getChannelIndex (item[ids::channel]);
                    const float y = body.getY() + body.getHeight() * (float) row / (float) channels;
                    if (item.hasType (ids::STEPSET))
                    {
                        const auto data = item[ids::data].toString();
                        for (int s = 0; s < juce::jmin (data.length(), (int) len); ++s)
                            if (data[s] == '1' && rep + s < (double) clip[ids::length])
                                g.fillRect (r.getX() + (float) ((rep + s) * pps), y, juce::jmax (1.0f, (float) pps - 1.0f), 2.0f);
                    }
                }
        }
        else
        {
            g.setColour (colour.withAlpha (0.8f));
            auto body = r.withTrimmedTop (16.0f).reduced (2.0f, 4.0f);
            g.drawHorizontalLine ((int) body.getCentreY(), body.getX(), body.getRight());
            g.setFont (juce::FontOptions (10.0f));
            g.drawText (txt ("audio grabado"), body, juce::Justification::centredLeft);
        }
    }

    // Cursor de reproducción
    if (ctx.engine.isSongMode())
    {
        const float x = stepsToX (ctx.engine.getPlayPosition());
        g.setColour (Colours::green);
        g.fillRect (x, 0.0f, 2.0f, (float) getHeight());
        juce::Path tri;
        tri.addTriangle (x - 6.0f, 0.0f, x + 8.0f, 0.0f, x + 1.0f, 10.0f);
        g.fillPath (tri);
    }
}

void PlaylistGrid::mouseMove (const juce::MouseEvent& e)
{
    bool edge = false;
    auto c = clipAt (e.position, edge);
    setMouseCursor (c.isValid() ? (edge ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::DraggingHandCursor)
                                : juce::MouseCursor::NormalCursor);
}

void PlaylistGrid::mouseDown (const juce::MouseEvent& e)
{
    dragMode = DragMode::none;
    if (e.x < labelWidth)
        return;

    const double steps = juce::jmax (0.0, xToSteps (e.position.x));

    if (e.y < rulerHeight)
    {
        ctx.engine.setPlayPosition (snap (steps));
        dragMode = DragMode::ruler;
        repaint();
        return;
    }

    bool edge = false;
    auto clip = clipAt (e.position, edge);
    ctx.project.beginTransaction();

    if (e.mods.isPopupMenu())
    {
        if (clip.isValid())
            ctx.project.playlist().removeChild (clip, ctx.project.um());
        return;
    }

    if (! clip.isValid())
    {
        const int patternUid = playlist.getPatternToPlace();
        auto pattern = ctx.project.findPattern (patternUid);
        if (! pattern.isValid())
            return;
        clip = ctx.project.addClip ("pattern", patternUid, yToTrack (e.position.y), snap (steps),
                                    Project::getPatternLength (pattern));
        edge = false;
    }

    dragClip = clip;
    dragMode = edge ? DragMode::resize : DragMode::move;
    dragStartSteps = steps;
    dragStartTrack = yToTrack (e.position.y);
    clipStart = clip[ids::start];
    clipLength = clip[ids::length];
    clipTrack = clip[ids::track];
}

void PlaylistGrid::mouseDrag (const juce::MouseEvent& e)
{
    const double steps = juce::jmax (0.0, xToSteps (e.position.x));

    if (dragMode == DragMode::ruler)
    {
        ctx.engine.setPlayPosition (snap (steps));
        repaint();
        return;
    }

    if (dragMode == DragMode::none || ! dragClip.isValid())
        return;

    const double s = playlist.getSnap();
    const double delta = steps - dragStartSteps;
    if (dragMode == DragMode::resize)
    {
        const double newLength = juce::jmax (s, std::round ((clipLength + delta) / s) * s);
        dragClip.setProperty (ids::length, newLength, ctx.project.um());
    }
    else
    {
        const double newStart = juce::jmax (0.0, std::round ((clipStart + delta) / s) * s);
        const int newTrack = juce::jlimit (0, numPlaylistTracks - 1, clipTrack + (yToTrack (e.position.y) - dragStartTrack));
        dragClip.setProperty (ids::start, newStart, ctx.project.um());
        dragClip.setProperty (ids::track, newTrack, ctx.project.um());
    }
}

void PlaylistGrid::mouseUp (const juce::MouseEvent&)
{
    dragMode = DragMode::none;
    dragClip = {};
}

void PlaylistGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    bool edge = false;
    auto clip = clipAt (e.position, edge);
    if (clip.isValid() && clip[ids::kind].toString() == "pattern")
    {
        ctx.project.setCurrentPattern (clip[ids::ref]);
        if (ctx.showView)
            ctx.showView (View::channelRack);
    }
}

// ============================================================================
//  Playlist
// ============================================================================
Playlist::Playlist (AppContext& c)
    : ctx (c), helpButton (txt (help::playlistTitle), txt (help::playlistText)), grid (c, *this)
{
    addAndMakeVisible (helpButton);

    addAndMakeVisible (patternBox);
    patternBox.setTooltip (txt ("El patrón que se coloca al hacer clic en un hueco de la playlist."));
    patternBox.onChange = [this]
    {
        auto p = ctx.project.patterns().getChild (patternBox.getSelectedId() - 1);
        if (p.isValid())
            ctx.project.setCurrentPattern (p[ids::uid]);
    };
    makeNonFocusing (patternBox);

    addAndMakeVisible (snapBox);
    snapBox.addItem (txt ("Ajuste: compás"), 1);
    snapBox.addItem (txt ("Ajuste: tiempo"), 2);
    snapBox.addItem (txt ("Ajuste: paso"), 3);
    snapBox.setSelectedId (1, juce::dontSendNotification);
    snapBox.setTooltip (txt ("A qué cuadrícula se pegan los bloques al colocarlos."));
    makeNonFocusing (snapBox);

    addAndMakeVisible (zoomSlider);
    zoomSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    zoomSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    zoomSlider.setRange (24.0, 240.0, 1.0);
    zoomSlider.setValue (72.0, juce::dontSendNotification);
    zoomSlider.setTooltip ("Zoom horizontal");
    zoomSlider.onValueChange = [this] { layoutGrid(); grid.repaint(); };
    makeNonFocusing (zoomSlider);

    addAndMakeVisible (infoLabel);
    infoLabel.setColour (juce::Label::textColourId, Colours::textDim);
    infoLabel.setMinimumHorizontalScale (0.6f);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&grid, false);

    ctx.project.addChangeListener (this);
    refresh();
    startTimerHz (30);
}

Playlist::~Playlist()
{
    ctx.project.removeChangeListener (this);
}

int Playlist::getPatternToPlace()
{
    auto p = ctx.project.patterns().getChild (patternBox.getSelectedId() - 1);
    return p.isValid() ? (int) p[ids::uid] : ctx.project.getCurrentPatternUid();
}

double Playlist::getSnap() const
{
    switch (snapBox.getSelectedId())
    {
        case 2:  return stepsPerBeat;
        case 3:  return 1.0;
        default: return stepsPerBar;
    }
}

void Playlist::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refresh();
}

void Playlist::refresh()
{
    patternBox.clear (juce::dontSendNotification);
    int i = 1, select = 1;
    for (auto p : ctx.project.patterns())
    {
        if ((int) p[ids::uid] == ctx.project.getCurrentPatternUid())
            select = i;
        patternBox.addItem (txt ("Colocar: ") + p[ids::name].toString(), i++);
    }
    patternBox.setSelectedId (select, juce::dontSendNotification);

    const bool song = ctx.project.getState()[ids::playMode].toString() == "song";
    infoLabel.setText (song ? txt ("Modo CANCIÓN activo: al reproducir suena la playlist.")
                            : txt ("Estás en modo PATRÓN. Pulsa CANCIÓN arriba (o L) para escuchar la playlist."),
                       juce::dontSendNotification);
    infoLabel.setColour (juce::Label::textColourId, song ? Colours::green : Colours::accent);

    layoutGrid();
    grid.repaint();
}

void Playlist::timerCallback()
{
    if (ctx.engine.isPlaying() || isMouseOverOrDragging (true))
        grid.repaint();
}

void Playlist::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, txt ("Playlist (canción)"));
}

void Playlist::layoutGrid()
{
    const int width = PlaylistGrid::labelWidth + (int) (grid.getTotalBars() * zoomSlider.getValue()) + 20;
    const int height = PlaylistGrid::rulerHeight + numPlaylistTracks * PlaylistGrid::trackHeight;
    grid.setSize (width, height);
}

void Playlist::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (42);
    auto h = headerArea.reduced (8, 7);
    helpButton.setBounds (h.removeFromLeft (26));
    h.removeFromLeft (170);
    patternBox.setBounds (h.removeFromLeft (220)); h.removeFromLeft (8);
    snapBox.setBounds (h.removeFromLeft (150)); h.removeFromLeft (8);
    zoomSlider.setBounds (h.removeFromRight (120));
    h.removeFromRight (8);
    infoLabel.setBounds (h);

    viewport.setBounds (area.reduced (6));
    layoutGrid();
}
}
