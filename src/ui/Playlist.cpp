#include "Playlist.h"
#include "HelpTexts.h"
#include "AudioEditor.h"

namespace studoo
{
// ============================================================================
//  PlaylistGrid
// ============================================================================
PlaylistGrid::PlaylistGrid (AppContext& c, Playlist& p) : ctx (c), playlist (p)
{
    enableLongPressMenu (*this); // en tablets: mantener pulsado = clic derecho
    setTooltip (txt ("Clic en un hueco: colocar el patrón elegido arriba. Arrastra un bloque: moverlo. Borde derecho: "
                     "alargarlo. Clic derecho: borrar y más opciones (hacer único, convertir a audio). Doble clic: editar el patrón. "
                     "Clic en la regla: mover el cursor; clic derecho en la regla: marcadores. Clic en el nombre de una pista: "
                     "renombrarla o agruparla.\n"
                     "Grabaciones: arrastra el borde izquierdo para quitar el principio. Clic derecho (o mantener pulsado): "
                     "cortar, duplicar, volumen y fundidos."));
    makeNonFocusing (*this);
}

double PlaylistGrid::getPixelsPerStep() const { return playlist.getZoom() / stepsPerBar; }
double PlaylistGrid::xToSteps (float x) const  { return (x - labelWidth) / getPixelsPerStep(); }
float PlaylistGrid::stepsToX (double s) const  { return (float) (labelWidth + s * getPixelsPerStep()); }
int PlaylistGrid::yToTrack (float y) const
{
    for (int t = 0; t < numPlaylistTracks; ++t)
        if (y < trackTop (t) + trackH (t))
            return t;
    return numPlaylistTracks - 1;
}

juce::String PlaylistGrid::trackGroup (int track) const
{
    return ctx.project.getState().getChildWithName (ids::PLAYLIST)["tgroup" + juce::String (track)].toString();
}

int PlaylistGrid::trackH (int track) const
{
    const auto pl = ctx.project.getState().getChildWithName (ids::PLAYLIST);
    return (bool) pl.getProperty ("tfold" + juce::String (track), false) ? foldedHeight : trackHeight;
}

float PlaylistGrid::trackTop (int track) const
{
    float y = (float) rulerHeight;
    for (int t = 0; t < track && t < numPlaylistTracks; ++t)
        y += (float) trackH (t);
    return y;
}

int PlaylistGrid::totalTracksHeight() const
{
    return (int) trackTop (numPlaylistTracks) - rulerHeight;
}

namespace
{
    void showMessage (const juce::String& title, const juce::String& message)
    {
        juce::AlertWindow::showAsync (juce::MessageBoxOptions().withIconType (juce::MessageBoxIconType::NoIcon)
                                          .withTitle (title).withMessage (message).withButton (txt ("Vale")), nullptr);
    }

    // Modo directo: un botón por patrón. Al pulsarlo empieza a sonar en el siguiente compás (y otra vez para pararlo).
    class LivePanel : public juce::Component, private juce::Timer
    {
    public:
        explicit LivePanel (AppContext& c) : ctx (c)
        {
            ctx.engine.setLiveMode (true);
            int i = 0;
            for (auto p : ctx.project.patterns())
            {
                if (i >= 24) break;
                auto* b = pads.add (new juce::TextButton (p[ids::name].toString()));
                const int uid = p[ids::uid];
                uids.add (uid);
                colours.add (juce::Colour ((juce::uint32) (juce::int64) p.getProperty (ids::colour, (juce::int64) 0xffff8a1f)));
                b->onClick = [this, uid]
                {
                    ctx.engine.toggleLivePattern (uid);
                    if (! ctx.engine.isPlaying())
                        ctx.engine.play();
                };
                b->setTooltip (txt ("Pulsa para que suene este patrón en bucle (empieza en el siguiente compás). Otra vez para pararlo."));
                makeNonFocusing (*b);
                addAndMakeVisible (b);
                ++i;
            }
            addAndMakeVisible (stopAll);
            stopAll.setButtonText (txt ("Parar todo"));
            stopAll.onClick = [this]
            {
                for (int k = 0; k < uids.size(); ++k)
                    if (const int st = ctx.engine.getLiveState (uids[k]); st == 1 || st == 2)
                        ctx.engine.toggleLivePattern (uids[k]);
            };
            makeNonFocusing (stopAll);
            const int cols = juce::jlimit (1, 4, uids.size());
            const int rows = (uids.size() + cols - 1) / juce::jmax (1, cols);
            setSize (juce::jmax (380, cols * 150 + 24), 140 + juce::jmax (1, rows) * 74);
            startTimerHz (15);
        }
        ~LivePanel() override { ctx.engine.setLiveMode (false); }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (Colours::panel);
            g.setColour (Colours::text);
            g.setFont (Fonts::bold (15.0f));
            g.drawText (txt ("Modo directo"), 12, 8, getWidth() - 24, 20, juce::Justification::centredLeft);
            g.setColour (Colours::textDim);
            g.setFont (Fonts::regular (11.5f));
            g.drawFittedText (txt ("Toca en directo: cada botón hace sonar un patrón en bucle. Empieza y para justo al empezar el "
                                   "siguiente compás, así siempre va a tiempo. Si cierras esta ventana, vuelve a sonar lo normal."),
                              12, 28, getWidth() - 24, 32, juce::Justification::topLeft, 2);
            for (int k = 0; k < pads.size(); ++k)
            {
                auto r = pads[k]->getBounds().toFloat().expanded (3.0f);
                const int st = ctx.engine.getLiveState (uids[k]);
                if (st == 0) continue;
                const bool blink = (juce::Time::getMillisecondCounter() / 250) % 2 == 0;
                g.setColour (st == 1 ? Colours::green : (blink ? Colours::accent : Colours::accent.withAlpha (0.3f)));
                g.drawRoundedRectangle (r, 7.0f, 3.0f);
            }
        }

        void resized() override
        {
            auto area = getLocalBounds().reduced (12).withTrimmedTop (56);
            stopAll.setBounds (area.removeFromBottom (28).removeFromRight (120));
            area.removeFromBottom (6);
            const int cols = juce::jlimit (1, 4, uids.size());
            const int w = area.getWidth() / juce::jmax (1, cols);
            for (int k = 0; k < pads.size(); ++k)
                pads[k]->setBounds (area.getX() + (k % cols) * w + 4, area.getY() + (k / cols) * 74 + 4, w - 8, 64);
        }

    private:
        void timerCallback() override
        {
            for (int k = 0; k < pads.size(); ++k)
            {
                const int st = ctx.engine.getLiveState (uids[k]);
                pads[k]->setColour (juce::TextButton::buttonColourId, st == 1 ? colours[k]
                                                                     : colours[k].withAlpha (st == 0 ? 0.35f : 0.6f));
                pads[k]->setColour (juce::TextButton::textColourOffId, st == 1 ? juce::Colours::black : Colours::text);
            }
            repaint();
        }

        AppContext& ctx;
        juce::OwnedArray<juce::TextButton> pads;
        juce::Array<int> uids;
        juce::Array<juce::Colour> colours;
        juce::TextButton stopAll;
    };

    juce::Colour groupColour (const juce::String& name)
    {
        return juce::Colour::fromHSV ((float) (name.hashCode() & 0xff) / 255.0f, 0.55f, 0.85f, 1.0f);
    }
}

double PlaylistGrid::snap (double steps) const
{
    const double s = playlist.getSnap();
    return std::floor (steps / s + 1.0e-6) * s;
}

int PlaylistGrid::getTotalBars()
{
    return juce::jmax (32, (int) (ctx.project.getSongLengthSteps() / ctx.project.getStepsPerBar()) + 8);
}

juce::Rectangle<float> PlaylistGrid::clipBounds (const juce::ValueTree& clip) const
{
    const float x = stepsToX (clip[ids::start]);
    const float w = (float) ((double) clip[ids::length] * getPixelsPerStep());
    const int track = juce::jlimit (0, numPlaylistTracks - 1, (int) clip[ids::track]);
    const float y = trackTop (track);
    const int h = trackH (track);
    return { x, y + (h < trackHeight ? 1.0f : 2.0f), w, (float) h - (h < trackHeight ? 2.0f : 4.0f) };
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
        const int y = (int) trackTop (t), h = trackH (t);
        g.setColour (t % 2 == 0 ? juce::Colour (0xff1d2126) : juce::Colour (0xff1a1d22));
        g.fillRect (labelWidth, y, getWidth() - labelWidth, h);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawHorizontalLine (y + h - 1, 0.0f, (float) getWidth());
    }

    // Compases y tiempos
    for (int b = 0; b <= bars; ++b)
    {
        const float x = stepsToX (b * ctx.project.getStepsPerBar());
        g.setColour (b % 4 == 0 ? Colours::outline.brighter (0.35f) : Colours::outline.brighter (0.05f));
        g.drawVerticalLine ((int) x, (float) rulerHeight, (float) getHeight());
        if (pps * stepsPerBeat > 10.0)
            for (int beat = 1; beat < 4; ++beat)
            {
                g.setColour (Colours::outline.withAlpha (0.3f));
                g.drawVerticalLine ((int) (x + beat * stepsPerBeat * pps), (float) rulerHeight, (float) getHeight());
            }
    }

    // Nombres de las pistas (y grupos)
    const auto plState = ctx.project.playlist();
    for (int t = 0; t < numPlaylistTracks; ++t)
    {
        const int y = (int) trackTop (t), h = trackH (t);
        auto label = juce::Rectangle<int> (0, y, labelWidth, h);
        g.setGradientFill (juce::ColourGradient (Colours::panelLight, 0.0f, (float) y, Colours::header, 0.0f, (float) (y + h), false));
        g.fillRect (label);
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.drawHorizontalLine (y + h - 1, 0.0f, (float) labelWidth);
        const auto group = trackGroup (t);
        const bool groupStart = group.isNotEmpty() && (t == 0 || trackGroup (t - 1) != group);
        if (group.isNotEmpty())
        {
            g.setColour (groupColour (group));
            g.fillRect (0, y, 4, h);
        }
        if (h < trackHeight)
        {
            // Grupo plegado: solo el nombre del grupo en la primera
            if (groupStart)
            {
                g.setColour (groupColour (group));
                g.setFont (Fonts::bold (9.5f));
                g.drawText (juce::String::fromUTF8 ("\xe2\x96\xb8 ") + group, label.withTrimmedLeft (7), juce::Justification::centredLeft);
            }
            continue;
        }
        auto dot = juce::Rectangle<float> (10.0f, (float) y + h * 0.5f - 9.0f, 18.0f, 18.0f);
        g.setColour (Colours::panelDark);
        g.fillEllipse (dot);
        g.setColour (Colours::accent.withAlpha (0.8f));
        g.setFont (Fonts::bold (10.5f));
        g.drawText (juce::String (t + 1), dot, juce::Justification::centred);
        const auto name = plState["tname" + juce::String (t)].toString();
        g.setColour (name.isNotEmpty() ? Colours::text : Colours::textDim);
        g.setFont (Fonts::regular (name.isNotEmpty() ? 11.0f : 12.0f));
        auto nameArea = label.withTrimmedLeft (32).withTrimmedRight (2);
        if (groupStart)
        {
            g.setColour (groupColour (group));
            g.setFont (Fonts::bold (9.0f));
            g.drawText (juce::String::fromUTF8 ("\xe2\x96\xbe ") + group, nameArea.removeFromTop (14), juce::Justification::bottomLeft);
            g.setColour (name.isNotEmpty() ? Colours::text : Colours::textDim);
            g.setFont (Fonts::regular (11.0f));
        }
        g.drawFittedText (name.isNotEmpty() ? name : juce::String ("Pista"), nameArea, juce::Justification::centredLeft, 1, 0.7f);
    }
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawVerticalLine (labelWidth - 1, (float) rulerHeight, (float) getHeight());

    // Regla
    g.setGradientFill (juce::ColourGradient (Colours::panelLight, 0.0f, 0.0f, Colours::header, 0.0f, (float) rulerHeight, false));
    g.fillRect (0, 0, getWidth(), rulerHeight);
    {
        // Zona en bucle
        const auto& st = ctx.project.getState();
        const double a = st.getProperty (ids::loopStart, 0.0), b = st.getProperty (ids::loopEnd, 0.0);
        if ((bool) st.getProperty (ids::loopOn, false) && b > a)
        {
            const float x0 = stepsToX (a), x1 = stepsToX (b);
            g.setColour (Colours::green.withAlpha (0.35f));
            g.fillRect (juce::Rectangle<float> (x0, 0.0f, x1 - x0, (float) rulerHeight));
            g.setColour (Colours::green.withAlpha (0.06f));
            g.fillRect (juce::Rectangle<float> (x0, (float) rulerHeight, x1 - x0, (float) getHeight()));
            g.setColour (Colours::green);
            g.drawVerticalLine ((int) x0, 0.0f, (float) getHeight());
            g.drawVerticalLine ((int) x1, 0.0f, (float) getHeight());
        }
    }
    for (int b = 0; b < bars; ++b)
    {
        const float x = stepsToX (b * ctx.project.getStepsPerBar());
        g.setColour (b % 4 == 0 ? Colours::text : Colours::textDim);
        g.setFont (b % 4 == 0 ? Fonts::bold (12.0f) : Fonts::regular (11.5f));
        g.drawText (juce::String (b + 1), juce::Rectangle<float> (x + 5.0f, 0.0f, 40.0f, (float) rulerHeight - 4.0f), juce::Justification::centredLeft);
        g.setColour (Colours::outline.brighter (0.4f));
        g.drawVerticalLine ((int) x, 8.0f, (float) rulerHeight);
        for (int beat = 1; beat < 4 && pps * stepsPerBeat > 8.0; ++beat)
            g.drawVerticalLine ((int) (x + beat * stepsPerBeat * pps), (float) rulerHeight - 5.0f, (float) rulerHeight);
    }
    const double songEnd = ctx.project.getSongLengthSteps();
    auto songBar = juce::Rectangle<float> (stepsToX (0.0), (float) rulerHeight - 3.0f, (float) (songEnd * pps), 3.0f);
    Draw::glow (g, songBar, Colours::accent, 4.0f, 1.5f);
    Draw::accentFill (g, songBar, 1.5f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawHorizontalLine (rulerHeight - 1, 0.0f, (float) getWidth());

    // Marcadores (Intro, Estribillo...): una banderita amarilla en la regla
    for (auto m : ctx.project.getState().getChildWithName (ids::MARKERS))
    {
        const float x = stepsToX (m[ids::time]);
        const auto name = m[ids::name].toString();
        const float w = juce::jmin (140.0f, (float) juce::GlyphArrangement::getStringWidthInt (Fonts::bold (10.0f), name) + 12.0f);
        auto flag = juce::Rectangle<float> (x, 1.0f, w, 13.0f);
        g.setColour (juce::Colour (0xffffd54a));
        g.fillRoundedRectangle (flag, 3.0f);
        g.drawVerticalLine ((int) x, 0.0f, (float) getHeight());
        g.setColour (juce::Colour (0xff2a2000));
        g.setFont (Fonts::bold (10.0f));
        g.drawText (name, flag.reduced (5.0f, 0.0f), juce::Justification::centredLeft, true);
    }

    // Bloques (clips)
    const auto clipArea = g.getClipBounds().toFloat();
    for (auto clip : ctx.project.playlist())
    {
        auto r = clipBounds (clip).reduced (1.0f, 1.0f);
        const bool clipMuted = clip.getProperty (ids::clipMuted, false);
        juce::Graphics::ScopedSaveState mutedState (g);
        if (clipMuted)
            g.setOpacity (0.35f);   // toma descartada: se ve apagada
        if (! r.expanded (10.0f).intersects (clipArea))
            continue;   // fuera de la zona que se está redibujando
        const bool audio = clip[ids::kind].toString() == "audio";
        const bool autoBlock = clip[ids::kind].toString() == "automation";
        juce::String name;
        juce::Colour colour = Colours::green;
        juce::ValueTree blockLane;
        if (r.getHeight() < 16.0f)
        {
            // Pista de un grupo plegado: solo una barrita del color del bloque
            juce::Colour c = audio ? Colours::green : autoBlock ? juce::Colour (0xffb388ff)
                : juce::Colour ((juce::uint32) (juce::int64) ctx.project.findPattern (clip[ids::ref]).getProperty (ids::colour, (juce::int64) 0xffff8a1f));
            g.setColour (c.withAlpha (0.75f));
            g.fillRoundedRectangle (r, 2.0f);
            continue;
        }

        if (audio)
        {
            name = ctx.project.findChannel (clip[ids::ref])[ids::name].toString();
        }
        else if (autoBlock)
        {
            blockLane = ctx.project.automation().getChildWithProperty (ids::uid, clip[ids::ref]);
            name = ctx.project.describeTarget (AutomationTarget::parse (blockLane[ids::target].toString()));
            colour = juce::Colour (0xffb388ff);   // morado: automatización
        }
        else
        {
            auto pattern = ctx.project.findPattern (clip[ids::ref]);
            name = pattern[ids::name].toString();
            colour = juce::Colour ((juce::uint32) (juce::int64) pattern.getProperty (ids::colour, (juce::int64) 0xffff8a1f));
        }

        const float corner = 6.0f;
        Draw::shadow (g, r, corner, 8);
        g.setGradientFill (juce::ColourGradient (colour.withAlpha (0.42f).overlaidWith (juce::Colour (0x22000000)), 0.0f, r.getY(),
                                                 colour.withAlpha (0.18f), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, corner);

        auto head = r.withHeight (15.0f);
        juce::Path headShape;
        headShape.addRoundedRectangle (head.getX(), head.getY(), head.getWidth(), head.getHeight(), corner, corner, true, true, false, false);
        g.setGradientFill (juce::ColourGradient (colour.brighter (0.25f), 0.0f, head.getY(), colour.darker (0.1f), 0.0f, head.getBottom(), false));
        g.fillPath (headShape);
        g.setColour (colour.brighter (0.3f));
        g.drawRoundedRectangle (r, corner, 1.0f);
        g.setColour (juce::Colour (0xff140c02));
        g.setFont (Fonts::bold (10.5f));
        g.drawText (name, head.reduced (6.0f, 0.0f), juce::Justification::centredLeft, true);

        auto body = r.withTrimmedTop (17.0f).reduced (3.0f, 2.0f);
        if (autoBlock)
        {
            // La curva del bloque (su tiempo empieza al principio del bloque)
            juce::Array<juce::ValueTree> points;
            for (auto p : blockLane) points.add (p);
            std::sort (points.begin(), points.end(), [] (const juce::ValueTree& a, const juce::ValueTree& b)
                       { return (double) a[ids::time] < (double) b[ids::time]; });
            const double start = clip[ids::start], len = clip[ids::length];
            juce::Path curve;
            auto yFor = [&body] (float v) { return body.getBottom() - juce::jlimit (0.0f, 1.0f, v) * body.getHeight(); };
            bool started = false;
            for (auto& p : points)
            {
                const double t = juce::jmin ((double) p[ids::time], len);
                const float x = stepsToX (start + t), y = yFor (p[ids::value]);
                if (! started) { curve.startNewSubPath (body.getX(), y); started = true; }
                curve.lineTo (x, y);
            }
            if (started)
            {
                curve.lineTo (body.getRight(), yFor (points.getLast()[ids::value]));
                g.setColour (colour.brighter (0.4f));
                g.strokePath (curve, juce::PathStrokeType (1.6f));
            }
        }
        else if (! audio)
        {
            // Mini vista de los pasos y notas del patrón
            auto pattern = ctx.project.findPattern (clip[ids::ref]);
            const double len = Project::getPatternLength (pattern);
            g.setColour (colour.brighter (0.6f).withAlpha (0.9f));
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
                                g.fillRoundedRectangle (r.getX() + 3.0f + (float) ((rep + s) * pps), y,
                                                        juce::jmax (1.5f, (float) pps - 1.0f), 2.2f, 1.0f);
                    }
                    else if (item.hasType (ids::NOTE) && rep + (double) item[ids::start] < (double) clip[ids::length])
                    {
                        g.fillRoundedRectangle (r.getX() + 3.0f + (float) ((rep + (double) item[ids::start]) * pps), y,
                                                juce::jmax (2.0f, (float) ((double) item[ids::length] * pps) - 1.0f), 2.2f, 1.0f);
                    }
                }
        }
        else
        {
            // Forma de onda real de la grabación (con su volumen y sus fundidos)
            double sr = 44100.0;
            const auto& pk = peaksFor (ctx.project.findChannel (clip[ids::ref])[ids::sample].toString(), sr);
            const double bpm = juce::jlimit (40.0, 250.0, (double) ctx.project.getState().getProperty (ids::bpm, 140.0));
            const double secondsPerStep = 60.0 / bpm / stepsPerBeat;
            const double start = clip[ids::start], length = clip[ids::length], offset = clip[ids::offset];
            const double fadeIn = clip.getProperty (ids::fadeIn, 0.0), fadeOut = clip.getProperty (ids::fadeOut, 0.0);
            const float gain = juce::jlimit (0.0f, 2.0f, (float) clip.getProperty (ids::clipGain, 1.0f));
            const bool stretched = (bool) clip.getProperty (ids::stretch, false);
            const double timeRatio = stretched ? juce::jlimit (0.25, 4.0, bpm / juce::jlimit (40.0, 250.0, (double) clip.getProperty (ids::recordBpm, bpm))) : 1.0;
            const int semis = juce::jlimit (-12, 12, (int) clip.getProperty (ids::clipPitch, 0));
            juce::Path wave;
            const float cy = body.getCentreY(), amp = body.getHeight() * 0.5f;
            const float x0 = juce::jmax (body.getX(), clipArea.getX() - 2.0f), x1 = juce::jmin (body.getRight(), clipArea.getRight() + 2.0f);
            for (float x = x0; x < x1 && ! pk.empty(); x += 2.0f)
            {
                const double inClip = xToSteps (x) - start;
                const double idx = (inClip + offset) * secondsPerStep * timeRatio * sr / samplesPerPeak;
                if (idx < 0.0 || idx >= (double) pk.size())
                    continue;
                double env = gain;
                if (fadeIn > 0.0)  env *= juce::jmin (1.0, inClip / fadeIn);
                if (fadeOut > 0.0) env *= juce::jmin (1.0, (length - inClip) / fadeOut);
                const float a = juce::jmax (0.5f, amp * (float) juce::jlimit (0.0, 1.0, pk[(size_t) idx] * env));
                wave.addRectangle (x, cy - a, 1.2f, a * 2.0f);
            }
            g.setColour (colour.brighter (0.4f).withAlpha (0.85f));
            g.fillPath (wave);
            juce::StringArray tags;
            if (stretched)  tags.add (txt ("al tempo"));
            if (semis != 0) tags.add ((semis > 0 ? "+" : "") + juce::String (semis) + " st");
            if (! juce::approximatelyEqual (gain, 1.0f)) tags.add (juce::String ((int) std::round (gain * 100.0f)) + "%");
            if (! tags.isEmpty())
            {
                g.setColour (juce::Colour (0xff140c02));
                g.setFont (Fonts::bold (9.5f));
                g.drawText (tags.joinIntoString (" · "), head.reduced (6.0f, 0.0f), juce::Justification::centredRight, false);
            }
        }
    }

    // Fila de automatización: la curva elegida en la pestaña Automatización
    {
        auto area = automationArea();
        g.setColour (juce::Colour (0xff16191d));
        g.fillRect (area.withX (0.0f).withWidth ((float) getWidth()));
        g.setColour (Colours::outline);
        g.drawHorizontalLine ((int) area.getY(), 0.0f, (float) getWidth());
        auto lane = selectedLane();
        g.setColour (Colours::textDim);
        g.setFont (Fonts::bold (10.0f));
        g.drawFittedText (lane.isValid() ? txt ("Automat.") : txt ("Automat.\n(ninguna)"),
                          juce::Rectangle<int> (4, (int) area.getY() + 4, labelWidth - 8, (int) area.getHeight() - 8),
                          juce::Justification::centredLeft, 3);
        if (lane.isValid())
        {
            const auto target = AutomationTarget::parse (lane[ids::target].toString());
            g.setColour (Colours::accent);
            g.setFont (Fonts::regular (10.0f));
            g.drawText (ctx.project.describeTarget (target), area.reduced (6.0f, 2.0f).withHeight (14.0f), juce::Justification::centredLeft);
            juce::Array<juce::ValueTree> points;
            for (auto p : lane) points.add (p);
            std::sort (points.begin(), points.end(), [] (const juce::ValueTree& a, const juce::ValueTree& b)
                       { return (double) a[ids::time] < (double) b[ids::time]; });
            juce::Path curve;
            for (int i = 0; i < points.size(); ++i)
            {
                const float x = stepsToX (points[i][ids::time]), y = autoValueToY (points[i][ids::value]);
                if (i == 0) { curve.startNewSubPath (area.getX(), y); curve.lineTo (x, y); }
                else        curve.lineTo (x, y);
                if (i == points.size() - 1) curve.lineTo ((float) getWidth(), y);
            }
            g.strokePath (curve, juce::PathStrokeType (2.0f));
            for (auto& p : points)
                g.fillEllipse (stepsToX (p[ids::time]) - 4.0f, autoValueToY (p[ids::value]) - 4.0f, 8.0f, 8.0f);
        }
    }

    // Cursor de reproducción
    if (ctx.engine.isSongMode())
    {
        const float x = stepsToX (ctx.engine.getPlayPosition());
        auto line = juce::Rectangle<float> (x - 1.0f, 0.0f, 2.0f, (float) getHeight());
        Draw::glow (g, line, Colours::green, 6.0f, 1.0f);
        g.setColour (Colours::green);
        g.fillRect (line);
        juce::Path tri;
        tri.addTriangle (x - 7.0f, 0.0f, x + 7.0f, 0.0f, x, 10.0f);
        g.fillPath (tri);
    }
}

void PlaylistGrid::updatePlayhead()
{
    const int x = ctx.engine.isSongMode() ? (int) stepsToX (ctx.engine.getPlayPosition()) : -1000;
    if (x == lastPlayheadX)
        return;
    repaint (lastPlayheadX - 12, 0, 24, getHeight());
    repaint (x - 12, 0, 24, getHeight());
    lastPlayheadX = x;
}

void PlaylistGrid::mouseMove (const juce::MouseEvent& e)
{
    bool edge = false;
    auto c = clipAt (e.position, edge);
    if (c.isValid() && ! edge && c[ids::kind].toString() == "audio")
    {
        const auto b = clipBounds (c);
        edge = e.position.x < b.getX() + juce::jmin (8.0f, b.getWidth() * 0.3f);
    }
    setMouseCursor (c.isValid() ? (edge ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::DraggingHandCursor)
                                : juce::MouseCursor::NormalCursor);
}

void PlaylistGrid::editAudioFile (juce::ValueTree clip, int operation)
{
    auto channel = ctx.project.findChannel (clip[ids::ref]);
    auto sample = ctx.samples.get (channel[ids::sample].toString());
    if (sample == nullptr || sample->buffer.getNumSamples() == 0)
        return;
    juce::AudioBuffer<float> b (sample->buffer);
    const int len = b.getNumSamples();
    switch (operation)
    {
        case 0: { const float m = b.getMagnitude (0, len); if (m > 1.0e-5f) b.applyGain (0.966f / m); break; }   // -0,3 dB
        case 1: b.reverse (0, len); break;
        case 2:
        {
            // Umbral: un poco por encima del 10% más silencioso de la grabación
            std::vector<float> levels;
            for (int i = 0; i + 1024 <= len; i += 1024)
                levels.push_back (b.getRMSLevel (0, i, 1024));
            if (levels.empty()) return;
            std::sort (levels.begin(), levels.end());
            const float threshold = juce::jmax (0.001f, levels[levels.size() / 10] * 2.5f);
            float gain = 1.0f, env = 0.0f;
            for (int i = 0; i < len; ++i)
            {
                float level = 0.0f;
                for (int ch = 0; ch < b.getNumChannels(); ++ch) level = juce::jmax (level, std::abs (b.getSample (ch, i)));
                env = level > env ? level : env * 0.9995f;
                gain += ((env > threshold ? 1.0f : 0.0f) - gain) * (env > threshold ? 0.05f : 0.0005f);
                for (int ch = 0; ch < b.getNumChannels(); ++ch) b.setSample (ch, i, b.getSample (ch, i) * gain);
            }
            break;
        }
        case 3:
        {
            // Quitar silencios: cambia dónde empieza y acaba el trozo (no hace falta otro archivo)
            const float th = juce::jmax (0.01f, b.getMagnitude (0, len) * 0.03f);
            int first = 0, last = len - 1;
            while (first < len && b.getMagnitude (first, juce::jmin (256, len - first)) < th) first += 256;
            while (last > first && b.getMagnitude (juce::jmax (0, last - 256), 256) < th) last -= 256;
            const double bpm = juce::jlimit (40.0, 250.0, (double) ctx.project.getState().getProperty (ids::bpm, 140.0));
            const double samplesPerStep = sample->sampleRate * 60.0 / bpm / stepsPerBeat;
            const double newOffset = juce::jmax ((double) clip[ids::offset], first / samplesPerStep);
            const double endSteps = juce::jmin ((double) clip[ids::offset] + (double) clip[ids::length], (last + 256) / samplesPerStep);
            if (endSteps - newOffset < 0.25)
                return;
            ctx.project.beginTransaction();
            const double shift = newOffset - (double) clip[ids::offset];
            clip.setProperty (ids::start, (double) clip[ids::start] + shift, ctx.project.um());
            clip.setProperty (ids::offset, newOffset, ctx.project.um());
            clip.setProperty (ids::length, endSteps - newOffset, ctx.project.um());
            return;
        }
        case 4: b.applyGain (1.4125f); break;
        case 5: b.applyGain (0.7079f); break;
        default: return;
    }
    // Nunca por encima del máximo (sin "romper" el sonido)
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        juce::FloatVectorOperations::clip (b.getWritePointer (ch), b.getReadPointer (ch), -1.0f, 1.0f, len);

    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Studoo").getChildFile ("Grabaciones");
    dir.createDirectory();
    auto file = dir.getNonexistentChildFile (juce::File::createLegalFileName (channel[ids::name].toString()) + " (editado)", ".wav", false);
    {
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> writer (wav.createWriterFor (new juce::FileOutputStream (file), sample->sampleRate,
                                                                               (unsigned int) b.getNumChannels(), 24, {}, 0));
        if (writer == nullptr || ! writer->writeFromAudioSampleBuffer (b, 0, len))
            return;
    }
    ctx.project.beginTransaction();
    channel.setProperty (ids::sample, file.getFullPathName(), ctx.project.um());
    peakCache.erase (file.getFullPathName());
    repaint();
}

const std::vector<float>& PlaylistGrid::peaksFor (const juce::String& path, double& sampleRate)
{
    auto it = peakCache.find (path);
    if (it == peakCache.end())
    {
        std::vector<float> peaks;
        double rate = 44100.0;
        if (auto sample = path.isNotEmpty() ? ctx.samples.get (path) : nullptr)
        {
            const auto& b = sample->buffer;
            rate = sample->sampleRate;
            const int len = b.getNumSamples();
            peaks.reserve ((size_t) (len / samplesPerPeak + 1));
            float maxPeak = 0.0001f;
            for (int from = 0; from < len; from += samplesPerPeak)
            {
                const int num = juce::jmin (samplesPerPeak, len - from);
                float m = 0.0f;
                for (int ch = 0; ch < b.getNumChannels(); ++ch)
                    m = juce::jmax (m, b.getMagnitude (ch, from, num));
                peaks.push_back (m);
                maxPeak = juce::jmax (maxPeak, m);
            }
            for (auto& p : peaks)
                p = juce::jmin (1.0f, p / maxPeak);
        }
        it = peakCache.emplace (path, std::make_pair (rate, std::move (peaks))).first;
    }
    sampleRate = it->second.first;
    return it->second.second;
}

void PlaylistGrid::showAudioClipMenu (juce::ValueTree clip, double atSteps)
{
    const double s = playlist.getSnap();
    const double start = clip[ids::start], length = clip[ids::length];
    const double cut = std::round (atSteps / s) * s;
    const bool canCut = cut > start + 0.01 && cut < start + length - 0.01;

    juce::PopupMenu menu;
    menu.addItem (1, txt ("Cortar aquí (en dos trozos)"), canCut);
    menu.addItem (2, txt ("Duplicar (pone una copia detrás)"));
    juce::PopupMenu volume, fadeIn, fadeOut;
    const float gain = clip.getProperty (ids::clipGain, 1.0f);
    for (int pc : { 25, 50, 75, 100, 125, 150, 200 })
        volume.addItem (100 + pc, juce::String (pc) + "%", true, juce::approximatelyEqual (gain * 100.0f, (float) pc));
    const double fi = clip.getProperty (ids::fadeIn, 0.0), fo = clip.getProperty (ids::fadeOut, 0.0);
    const std::pair<int, const char*> fades[] = { { 0, "Sin fundido" }, { 1, "Corto (1 paso)" }, { 4, "Medio (1 tiempo)" }, { 16, "Largo (1 compás)" } };
    for (auto& f : fades)
    {
        fadeIn.addItem (400 + f.first, txt (f.second), true, juce::approximatelyEqual (fi, (double) f.first));
        fadeOut.addItem (500 + f.first, txt (f.second), true, juce::approximatelyEqual (fo, (double) f.first));
    }
    menu.addSubMenu ("Volumen", volume);
    menu.addItem (600, txt ("Seguir el tempo de la canción (estirar sin cambiar el tono)"), true,
                  (bool) clip.getProperty (ids::stretch, false));
    juce::PopupMenu pitch;
    const int currentPitch = clip.getProperty (ids::clipPitch, 0);
    for (int st : { 12, 7, 5, 3, 2, 1, 0, -1, -2, -3, -5, -7, -12 })
        pitch.addItem (700 + st + 12, st == 0 ? txt ("Original") : (st > 0 ? "+" : "") + juce::String (st) + txt (" semitonos"),
                       true, st == currentPitch);
    menu.addSubMenu (txt ("Tono (sin cambiar la velocidad)"), pitch);
    menu.addSubMenu (txt ("Fundido de entrada (sube poco a poco)"), fadeIn);
    menu.addSubMenu (txt ("Fundido de salida (baja poco a poco)"), fadeOut);
    menu.addSeparator();
    juce::PopupMenu edit;
    edit.addItem (800, txt ("Normalizar (subir al máximo sin pasarse)"));
    edit.addItem (801, txt ("Al revés"));
    edit.addItem (802, txt ("Quitar ruido de fondo (puerta)"));
    edit.addItem (803, txt ("Quitar los silencios del principio y del final"));
    edit.addItem (804, txt ("Más fuerte (+3 dB)"));
    edit.addItem (805, txt ("Más flojo (-3 dB)"));
    edit.addSeparator();
    edit.addItem (806, txt ("Abrir el editor de audio (elegir un trozo y editarlo)..."));
    menu.addSubMenu (txt ("Editar el audio (crea una copia)"), edit);
    const bool muted = clip.getProperty (ids::clipMuted, false);
    menu.addItem (900, muted ? txt ("Volver a activar este trozo") : txt ("Silenciar este trozo"));
    menu.addItem (901, txt ("Quedarme con esta toma (silencia las que suenan a la vez)"));
    menu.addItem (902, txt ("Fundido cruzado con el trozo de antes (en la misma pista)"));
    menu.addSeparator();
    menu.addItem (3, "Borrar");

    juce::Component::SafePointer<PlaylistGrid> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withMousePosition(), [safe, clip, cut] (int result) mutable
    {
        if (safe == nullptr || result == 0 || ! clip.getParent().isValid())
            return;
        auto& project = safe->ctx.project;
        auto* um = project.um();
        project.beginTransaction();
        const double start = clip[ids::start], length = clip[ids::length], offset = clip[ids::offset];
        if (result == 1)
        {
            auto second = clip.createCopy();
            second.setProperty (ids::uid, project.newUid(), nullptr);
            second.setProperty (ids::start, cut, nullptr);
            second.setProperty (ids::length, start + length - cut, nullptr);
            second.setProperty (ids::offset, offset + (cut - start), nullptr);
            second.setProperty (ids::fadeIn, 0.0, nullptr);
            clip.setProperty (ids::length, cut - start, um);
            clip.setProperty (ids::fadeOut, 0.0, um);
            project.playlist().appendChild (second, um);
        }
        else if (result == 2)
        {
            auto copy = clip.createCopy();
            copy.setProperty (ids::uid, project.newUid(), nullptr);
            copy.setProperty (ids::start, start + length, nullptr);
            project.playlist().appendChild (copy, um);
        }
        else if (result == 3)
            project.playlist().removeChild (clip, um);
        else if (result >= 100 && result <= 300)
            clip.setProperty (ids::clipGain, (result - 100) / 100.0f, um);
        else if (result >= 400 && result < 500)
            clip.setProperty (ids::fadeIn, (double) (result - 400), um);
        else if (result >= 500 && result < 600)
            clip.setProperty (ids::fadeOut, (double) (result - 500), um);
        else if (result == 600)
        {
            const bool on = ! (bool) clip.getProperty (ids::stretch, false);
            if (on && ! clip.hasProperty (ids::recordBpm))
            {
                // El tempo "original" de la grabación: el que se guardó al grabar o, si no, el de ahora
                auto channel = project.findChannel (clip[ids::ref]);
                clip.setProperty (ids::recordBpm, channel.getProperty (ids::recordBpm, project.getState().getProperty (ids::bpm, 140.0)), um);
            }
            clip.setProperty (ids::stretch, on, um);
        }
        else if (result >= 700 && result <= 724)
            clip.setProperty (ids::clipPitch, result - 700 - 12, um);
        else if (result == 806)
        {
            auto channel = project.findChannel (clip[ids::ref]);
            juce::DialogWindow::LaunchOptions options;
            options.content.setOwned (new AudioEditorPanel (safe->ctx, channel));
            options.dialogTitle = txt ("Editor de audio");
            options.dialogBackgroundColour = Colours::panel;
            options.componentToCentreAround = safe->getTopLevelComponent();
            options.escapeKeyTriggersCloseButton = true;
            options.useNativeTitleBar = false;
            options.resizable = false;
            options.launchAsync();
        }
        else if (result >= 800 && result < 900)
            safe->editAudioFile (clip, result - 800);
        else if (result == 900)
            clip.setProperty (ids::clipMuted, ! (bool) clip.getProperty (ids::clipMuted, false), um);
        else if (result == 901)
        {
            // Comping: esta toma suena y las grabaciones que coinciden en el tiempo se silencian
            clip.setProperty (ids::clipMuted, false, um);
            for (auto other : project.playlist())
                if (other != clip && other[ids::kind].toString() == "audio"
                    && (double) other[ids::start] < start + length && (double) other[ids::start] + (double) other[ids::length] > start)
                    other.setProperty (ids::clipMuted, true, um);
        }
        else if (result == 902)
            safe->crossfadeWithPrevious (clip);
    });
}

// ----------------------------------------------------------------------------
//  Fundido cruzado, menús de bloques, pistas y marcadores
// ----------------------------------------------------------------------------
void PlaylistGrid::crossfadeWithPrevious (juce::ValueTree clip)
{
    // Busca el trozo de audio anterior en la misma pista y los solapa un poco:
    // el de antes baja mientras el nuevo sube (sin cortes bruscos)
    const int track = clip[ids::track];
    const double start = clip[ids::start];
    juce::ValueTree prev;
    for (auto other : ctx.project.playlist())
        if (other != clip && other[ids::kind].toString() == "audio" && (int) other[ids::track] == track
            && (double) other[ids::start] < start
            && (! prev.isValid() || (double) other[ids::start] > (double) prev[ids::start]))
            prev = other;
    if (! prev.isValid())
    {
        showMessage (txt ("Fundido cruzado"), txt ("No hay otro trozo de audio antes de este en la misma pista."));
        return;
    }
    auto* um = ctx.project.um();
    ctx.project.beginTransaction();
    const double prevEnd = (double) prev[ids::start] + (double) prev[ids::length];
    double overlap = prevEnd - start;
    if (overlap < 1.0)
    {
        // Se tocan (o hay un hueco): el trozo nuevo empieza antes si tiene audio de sobra; si no, se alarga el de antes
        const double want = 2.0 - juce::jmax (0.0, overlap);
        const double gap = juce::jmax (0.0, start - prevEnd);
        const double offset = clip[ids::offset];
        if (offset >= want + gap)
        {
            clip.setProperty (ids::start, start - want - gap, um);
            clip.setProperty (ids::offset, offset - want - gap, um);
            clip.setProperty (ids::length, (double) clip[ids::length] + want + gap, um);
        }
        else
        {
            prev.setProperty (ids::length, (double) prev[ids::length] + want + gap, um);
        }
        overlap = (double) prev[ids::start] + (double) prev[ids::length] - (double) clip[ids::start];
    }
    overlap = juce::jmin (overlap, (double) clip[ids::length] * 0.5, (double) prev[ids::length] * 0.5);
    prev.setProperty (ids::fadeOut, overlap, um);
    clip.setProperty (ids::fadeIn, overlap, um);
    repaint();
}

void PlaylistGrid::showPatternClipMenu (juce::ValueTree clip)
{
    auto pattern = ctx.project.findPattern (clip[ids::ref]);
    int uses = 0;
    for (auto c : ctx.project.playlist())
        if (c[ids::kind].toString() == "pattern" && (int) c[ids::ref] == (int) clip[ids::ref])
            ++uses;
    juce::PopupMenu menu;
    menu.addSectionHeader (pattern[ids::name].toString());
    menu.addItem (1, txt ("Borrar este bloque"));
    menu.addItem (2, txt ("Abrir el patrón (doble clic)"));
    menu.addSeparator();
    menu.addItem (3, txt ("Hacer único (una copia del patrón para cambiarlo sin tocar los otros bloques)"), uses > 1);
    menu.addItem (4, txt ("Convertir a audio (graba este bloque como un trozo de audio)"));
    juce::Component::SafePointer<PlaylistGrid> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options(), [safe, clip] (int r) mutable
    {
        if (safe == nullptr || r <= 0)
            return;
        auto& project = safe->ctx.project;
        if (r == 1)
        {
            project.beginTransaction();
            project.playlist().removeChild (clip, project.um());
        }
        else if (r == 2)
        {
            project.setCurrentPattern (clip[ids::ref]);
            if (safe->ctx.showView) safe->ctx.showView (View::channelRack);
        }
        else if (r == 3)
        {
            project.beginTransaction();
            auto copy = project.duplicatePattern (clip[ids::ref]);
            if (copy.isValid())
            {
                copy.setProperty (ids::colour, (juce::int64) juce::Colour::fromHSV (juce::Random::getSystemRandom().nextFloat(), 0.7f, 0.95f, 1.0f).getARGB(), project.um());
                clip.setProperty (ids::ref, copy[ids::uid], project.um());
                project.setCurrentPattern (copy[ids::uid]);
            }
        }
        else if (r == 4)
        {
            safe->renderPatternToAudio (clip);
        }
    });
}

void PlaylistGrid::renderPatternToAudio (juce::ValueTree clip)
{
    // Una copia del proyecto con solo este bloque, al principio de la canción, se graba en un WAV.
    // El Master de la copia va sin efectos (el audio nuevo ya pasará por el Master de verdad).
    auto copy = ctx.project.getState().createCopy();
    auto pl = copy.getChildWithName (ids::PLAYLIST);
    pl.removeAllChildren (nullptr);
    auto only = clip.createCopy();
    only.setProperty (ids::start, 0.0, nullptr);
    only.setProperty (ids::track, 0, nullptr);
    pl.appendChild (only, nullptr);
    copy.setProperty (ids::playMode, "song", nullptr);
    copy.setProperty (ids::loopOn, false, nullptr);
    copy.removeChild (copy.getChildWithName (ids::AUTOMATION), nullptr);
    auto mixer = copy.getChildWithName (ids::MIXER);
    for (auto t : mixer)
        if ((int) t[ids::index] == 0)
        {
            auto fresh = juce::ValueTree (ids::TRACK);
            fresh.setProperty (ids::index, 0, nullptr);
            fresh.setProperty (ids::name, "Master", nullptr);
            fresh.setProperty (ids::volume, 1.0f, nullptr);
            t.copyPropertiesFrom (fresh, nullptr);
        }
    bool hasPlugins = false;
    for (auto ch : copy.getChildWithName (ids::CHANNELS))
        hasPlugins = hasPlugins || ch[ids::type].toString() == "plugin";

    auto dir = ctx.project.getFile() != juce::File() ? ctx.project.getFile().getParentDirectory().getChildFile ("Grabaciones")
                                                     : juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Studoo").getChildFile ("Grabaciones");
    dir.createDirectory();
    const auto name = ctx.project.findPattern (clip[ids::ref])[ids::name].toString();
    auto file = dir.getNonexistentChildFile (juce::File::createLegalFileName (name) + " (audio)", ".wav", false);

    juce::String error;
    bool ok = false;
    {
        juce::MouseCursor::showWaitCursor();
        auto engine = std::make_unique<AudioEngine>();
        engine->prepare (44100.0, 512);
        engine->updateFromProject (copy, ctx.samples);
        ok = engine->renderSongToFile (file, 44100.0, nullptr, error,
                                       (double) clip[ids::length] * 60.0 / juce::jlimit (40.0, 250.0, (double) copy.getProperty (ids::bpm, 140.0)) / stepsPerBeat);
        juce::MouseCursor::hideWaitCursor();
    }
    if (! ok)
    {
        showMessage (txt ("Convertir a audio"), txt ("No se pudo crear el audio: ") + error);
        return;
    }

    auto& project = ctx.project;
    project.beginTransaction();
    auto ch = project.addChannel (ChannelType::audio, name + txt (" (audio)"), 0);
    ch.setProperty (ids::sample, file.getFullPathName(), project.um());
    ch.setProperty (ids::volume, 0.8f / 0.8f, project.um());
    ch.setProperty (ids::recordBpm, project.getState().getProperty (ids::bpm, 140.0), project.um());
    auto audioClip = project.addClip ("audio", ch[ids::uid], clip[ids::track], clip[ids::start], clip[ids::length]);
    juce::ignoreUnused (audioClip);
    project.playlist().removeChild (clip, project.um());
    if (hasPlugins)
        showMessage (txt ("Convertir a audio"), txt ("Hecho. Ojo: los instrumentos VST3 no se incluyen en el audio (solo los de Studoo)."));
}

juce::ValueTree PlaylistGrid::markerNear (float x)
{
    for (auto m : ctx.project.getState().getChildWithName (ids::MARKERS))
    {
        const float mx = stepsToX (m[ids::time]);
        if (x >= mx - 4.0f && x <= mx + 90.0f)
            return m;
    }
    return {};
}

void PlaylistGrid::showRulerMenu (double atSteps)
{
    const double bar = ctx.project.getStepsPerBar();
    const double at = std::floor (atSteps / bar + 1.0e-6) * bar;
    auto hit = markerNear (stepsToX (atSteps));
    auto markers = ctx.project.getState().getChildWithName (ids::MARKERS);
    juce::PopupMenu menu, add, go;
    static const char* names[] = { "Intro", "Verso", "Pre-estribillo", "Estribillo", "Puente", "Drop", "Final" };
    for (int i = 0; i < 7; ++i)
        add.addItem (10 + i, txt (names[i]));
    add.addItem (20, txt ("Otro nombre..."));
    menu.addSubMenu (txt ("Añadir marcador en el compás ") + juce::String ((int) (at / bar) + 1), add);
    if (hit.isValid())
    {
        menu.addItem (2, txt ("Renombrar el marcador \"") + hit[ids::name].toString() + "\"");
        menu.addItem (3, txt ("Borrar el marcador \"") + hit[ids::name].toString() + "\"");
    }
    int k = 0;
    for (auto m : markers)
        go.addItem (100 + k++, m[ids::name].toString() + txt ("  (compás ") + juce::String ((int) ((double) m[ids::time] / bar) + 1) + ")");
    if (k > 0)
        menu.addSubMenu (txt ("Ir a un marcador"), go);

    juce::Component::SafePointer<PlaylistGrid> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options(), [safe, at, hit] (int r) mutable
    {
        if (safe == nullptr || r <= 0)
            return;
        auto& project = safe->ctx.project;
        auto addMarker = [safe, at] (const juce::String& name)
        {
            if (safe == nullptr || name.trim().isEmpty()) return;
            auto& pr = safe->ctx.project;
            pr.beginTransaction();
            auto list = pr.getState().getOrCreateChildWithName (ids::MARKERS, pr.um());
            juce::ValueTree m (ids::MARKER);
            m.setProperty (ids::time, at, nullptr);
            m.setProperty (ids::name, name.trim().substring (0, 40), nullptr);
            int pos = 0;
            while (pos < list.getNumChildren() && (double) list.getChild (pos)[ids::time] <= at) ++pos;
            list.addChild (m, pos, pr.um());
            safe->repaint();
        };
        if (r >= 10 && r < 17)
            addMarker (txt (names[r - 10]));
        else if (r == 20)
            askForText (txt ("Marcador"), txt ("Nombre del marcador:"), txt ("Marcador"), addMarker);
        else if (r == 2 && hit.isValid())
            askForText (txt ("Marcador"), txt ("Nuevo nombre:"), hit[ids::name].toString(), [safe, hit] (const juce::String& name) mutable
            {
                if (safe == nullptr || name.trim().isEmpty()) return;
                safe->ctx.project.beginTransaction();
                hit.setProperty (ids::name, name.trim().substring (0, 40), safe->ctx.project.um());
                safe->repaint();
            });
        else if (r == 3 && hit.isValid())
        {
            project.beginTransaction();
            hit.getParent().removeChild (hit, project.um());
        }
        else if (r >= 100)
        {
            auto m = project.getState().getChildWithName (ids::MARKERS).getChild (r - 100);
            if (m.isValid())
                safe->ctx.engine.setPlayPosition (m[ids::time]);
        }
        safe->repaint();
    });
}

void PlaylistGrid::showTrackMenu (int track)
{
    auto pl = ctx.project.playlist();
    const auto key = juce::String (track);
    const auto group = trackGroup (track);
    juce::StringArray groups;
    for (int t = 0; t < numPlaylistTracks; ++t)
        if (trackGroup (t).isNotEmpty())
            groups.addIfNotAlreadyThere (trackGroup (t));

    juce::PopupMenu menu, join;
    menu.addSectionHeader (txt ("Pista ") + juce::String (track + 1));
    menu.addItem (1, txt ("Renombrar pista..."));
    menu.addSeparator();
    menu.addItem (2, txt ("Crear un grupo nuevo con esta pista..."));
    for (int i = 0; i < groups.size(); ++i)
        if (groups[i] != group)
            join.addItem (100 + i, groups[i]);
    menu.addSubMenu (txt ("Añadir al grupo"), join, join.getNumItems() > 0);
    if (group.isNotEmpty())
    {
        menu.addItem (3, txt ("Sacar del grupo \"") + group + "\"");
        const bool folded = (bool) pl.getProperty ("tfold" + key, false);
        menu.addItem (4, folded ? txt ("Desplegar el grupo") : txt ("Plegar el grupo (se ve más pequeño)"));
    }
    juce::Component::SafePointer<PlaylistGrid> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options(), [safe, track, key, group, groups] (int r)
    {
        if (safe == nullptr || r <= 0)
            return;
        auto p = safe->ctx.project.playlist();
        auto setGroup = [safe, track, key] (const juce::String& g)
        {
            if (safe == nullptr) return;
            auto pl2 = safe->ctx.project.playlist();
            if (g.trim().isEmpty()) pl2.removeProperty ("tgroup" + key, nullptr);
            else                    pl2.setProperty ("tgroup" + key, g.trim().substring (0, 24), nullptr);
            // si el grupo está plegado, la pista nueva también
            bool folded = false;
            for (int t = 0; t < numPlaylistTracks; ++t)
                if (t != track && safe->trackGroup (t) == g.trim() && (bool) pl2.getProperty ("tfold" + juce::String (t), false))
                    folded = true;
            if (folded) pl2.setProperty ("tfold" + key, true, nullptr);
            else        pl2.removeProperty ("tfold" + key, nullptr);
            safe->playlist.resized();
            safe->repaint();
        };
        if (r == 1)
            askForText (txt ("Pista"), txt ("Nombre de la pista:"), p["tname" + key].toString(), [safe, key] (const juce::String& name)
            {
                if (safe == nullptr) return;
                auto pl2 = safe->ctx.project.playlist();
                if (name.trim().isEmpty()) pl2.removeProperty ("tname" + key, nullptr);
                else                       pl2.setProperty ("tname" + key, name.trim().substring (0, 30), nullptr);
                safe->repaint();
            });
        else if (r == 2)
            askForText (txt ("Grupo"), txt ("Nombre del grupo (por ejemplo: Batería, Voces):"), txt ("Batería"), setGroup);
        else if (r == 3)
            setGroup ({});
        else if (r == 4)
        {
            const bool fold = ! (bool) p.getProperty ("tfold" + key, false);
            for (int t = 0; t < numPlaylistTracks; ++t)
                if (safe->trackGroup (t) == group)
                {
                    if (fold) p.setProperty ("tfold" + juce::String (t), true, nullptr);
                    else      p.removeProperty ("tfold" + juce::String (t), nullptr);
                }
            safe->playlist.resized();
            safe->repaint();
        }
        else if (r >= 100 && r - 100 < groups.size())
            setGroup (groups[r - 100]);
    });
}

juce::ValueTree PlaylistGrid::selectedLane()
{
    return ctx.project.automation().getChildWithProperty (ids::uid, ctx.project.getState()[ids::selectedLane]);
}

juce::Rectangle<float> PlaylistGrid::automationArea() const
{
    const float top = trackTop (numPlaylistTracks);
    return { (float) labelWidth, top, (float) (getWidth() - labelWidth), (float) automationHeight };
}

float PlaylistGrid::autoValueToY (float v) const
{
    auto a = automationArea().reduced (0.0f, 18.0f).withTrimmedBottom (-12.0f);
    return a.getBottom() - juce::jlimit (0.0f, 1.0f, v) * a.getHeight();
}

float PlaylistGrid::autoYToValue (float y) const
{
    auto a = automationArea().reduced (0.0f, 18.0f).withTrimmedBottom (-12.0f);
    return juce::jlimit (0.0f, 1.0f, (a.getBottom() - y) / a.getHeight());
}

void PlaylistGrid::mouseDown (const juce::MouseEvent& e)
{
    dragMode = DragMode::none;
    if (e.x < labelWidth)
    {
        // Nombre de la pista: plegar el grupo (triángulo) o el menú de la pista
        if (e.y >= rulerHeight && e.position.y < trackTop (numPlaylistTracks))
        {
            const int t = yToTrack (e.position.y);
            const auto group = trackGroup (t);
            const bool groupStart = group.isNotEmpty() && (t == 0 || trackGroup (t - 1) != group);
            if (groupStart && ! e.mods.isPopupMenu() && e.position.y < trackTop (t) + 16.0f)
            {
                auto pl = ctx.project.playlist();
                const bool fold = ! (bool) pl.getProperty ("tfold" + juce::String (t), false);
                for (int k = 0; k < numPlaylistTracks; ++k)
                    if (trackGroup (k) == group)
                    {
                        if (fold) pl.setProperty ("tfold" + juce::String (k), true, nullptr);
                        else      pl.removeProperty ("tfold" + juce::String (k), nullptr);
                    }
                playlist.resized();
                repaint();
                return;
            }
            showTrackMenu (t);
        }
        return;
    }

    const double steps = juce::jmax (0.0, xToSteps (e.position.x));

    // Fila de automatización: añadir, mover o borrar puntos de la curva
    if (automationArea().contains (e.position))
    {
        auto lane = selectedLane();
        if (! lane.isValid())
            return;
        ctx.project.beginTransaction();
        juce::ValueTree hit;
        for (auto p : lane)
            if (juce::Point<float> (stepsToX (p[ids::time]), autoValueToY (p[ids::value])).getDistanceFrom (e.position) < 8.0f)
                hit = p;
        if (e.mods.isPopupMenu())
        {
            if (hit.isValid() && lane.getNumChildren() > 1)
                lane.removeChild (hit, ctx.project.um());
            return;
        }
        if (! hit.isValid())
        {
            hit = juce::ValueTree (ids::POINT);
            hit.setProperty (ids::time, snap (steps), nullptr);
            hit.setProperty (ids::value, autoYToValue (e.position.y), nullptr);
            lane.appendChild (hit, ctx.project.um());
        }
        dragPoint = hit;
        dragMode = DragMode::automation;
        return;
    }

    if (e.y < rulerHeight)
    {
        if (e.mods.isPopupMenu())
        {
            showRulerMenu (steps);
            return;
        }
        if (auto marker = markerNear (e.position.x); marker.isValid() && e.y < 15)
        {
            ctx.engine.setPlayPosition (marker[ids::time]);   // clic en un marcador: ir allí
            repaint();
            return;
        }
        auto& st = ctx.project.getState();
        if ((bool) st.getProperty (ids::loopOn, false))
        {
            // Con el bucle encendido, arrastrar en la regla marca la zona que se repite
            const double bar = ctx.project.getStepsPerBar();
            loopAnchor = std::floor (steps / bar) * bar;
            st.setProperty (ids::loopStart, loopAnchor, nullptr);
            st.setProperty (ids::loopEnd, loopAnchor + bar, nullptr);
            ctx.engine.setPlayPosition (loopAnchor);
            dragMode = DragMode::loop;
            repaint();
            return;
        }
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
        if (clip.isValid() && clip[ids::kind].toString() == "audio")
            showAudioClipMenu (clip, steps);
        else if (clip.isValid() && clip[ids::kind].toString() == "pattern")
            showPatternClipMenu (clip);
        else if (clip.isValid())
            ctx.project.playlist().removeChild (clip, ctx.project.um());
        return;
    }

    if (! clip.isValid())
    {
        if (const int laneUid = playlist.getLaneToPlace(); laneUid > 0)
        {
            // Bloque de automatización: dura lo que su curva (como mínimo, un compás)
            auto lane = ctx.project.automation().getChildWithProperty (ids::uid, laneUid);
            double end = 0.0;
            for (auto p : lane)
                end = juce::jmax (end, (double) p[ids::time]);
            const double bar = ctx.project.getStepsPerBar();
            const double len = juce::jlimit (bar, bar * 64.0, std::ceil (end / bar) * bar);
            clip = ctx.project.addClip ("automation", laneUid, yToTrack (e.position.y), snap (steps), len);
        }
        else
        {
            const int patternUid = playlist.getPatternToPlace();
            auto pattern = ctx.project.findPattern (patternUid);
            if (! pattern.isValid())
                return;
            clip = ctx.project.addClip ("pattern", patternUid, yToTrack (e.position.y), snap (steps),
                                        Project::getPatternLength (pattern));
        }
        edge = false;
    }

    dragClip = clip;
    const bool audio = clip[ids::kind].toString() == "audio";
    const auto bounds = clipBounds (clip);
    const bool leftEdge = audio && ! edge && e.position.x < bounds.getX() + juce::jmin (8.0f, bounds.getWidth() * 0.3f);
    dragMode = leftEdge ? DragMode::trimStart : edge ? DragMode::resize : DragMode::move;
    clipOffset = clip[ids::offset];
    dragStartSteps = steps;
    dragStartTrack = yToTrack (e.position.y);
    clipStart = clip[ids::start];
    clipLength = clip[ids::length];
    clipTrack = clip[ids::track];
}

void PlaylistGrid::mouseDrag (const juce::MouseEvent& e)
{
    const double steps = juce::jmax (0.0, xToSteps (e.position.x));
    if (dragMode == DragMode::automation)
    {
        if (dragPoint.isValid())
        {
            dragPoint.setProperty (ids::time, juce::jmax (0.0, std::round (steps / playlist.getSnap() * 4.0) * playlist.getSnap() / 4.0), ctx.project.um());
            dragPoint.setProperty (ids::value, autoYToValue (e.position.y), ctx.project.um());
        }
        return;
    }

    if (dragMode == DragMode::ruler)
    {
        ctx.engine.setPlayPosition (snap (steps));
        repaint();
        return;
    }
    if (dragMode == DragMode::loop)
    {
        const double bar = ctx.project.getStepsPerBar();
        const double other = steps < loopAnchor ? std::floor (steps / bar) * bar : std::ceil (steps / bar) * bar;
        auto& st = ctx.project.getState();
        st.setProperty (ids::loopStart, juce::jmin (loopAnchor, other), nullptr);
        st.setProperty (ids::loopEnd, juce::jmax (loopAnchor + bar, juce::jmax (loopAnchor, other)), nullptr);
        ctx.engine.setPlayPosition (juce::jmin (loopAnchor, other));
        repaint();
        return;
    }

    if (dragMode == DragMode::none || ! dragClip.isValid())
        return;

    const double s = playlist.getSnap();
    const double delta = steps - dragStartSteps;
    if (dragMode == DragMode::trimStart)
    {
        // Recortar el principio de una grabación: el clip empieza más tarde y salta esa parte del audio
        double d = std::round (delta / s) * s;
        d = juce::jlimit (-juce::jmin (clipOffset, clipStart), clipLength - s, d);
        dragClip.setProperty (ids::start, clipStart + d, ctx.project.um());
        dragClip.setProperty (ids::length, clipLength - d, ctx.project.um());
        dragClip.setProperty (ids::offset, clipOffset + d, ctx.project.um());
        return;
    }
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
    dragPoint = {};
    dragMode = DragMode::none;
    dragClip = {};
}

void PlaylistGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    bool edge = false;
    auto clip = clipAt (e.position, edge);
    if (clip.isValid() && clip[ids::kind].toString() == "automation")
    {
        // Editar la curva de este bloque en la pestaña Automatización
        ctx.project.getState().setProperty (ids::selectedLane, clip[ids::ref], nullptr);
        if (ctx.showView)
            ctx.showView (View::automation);
        return;
    }
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
        lastPlaceId = patternBox.getSelectedId();
        if (lastPlaceId >= 10000)
        {
            auto lane = ctx.project.automation().getChild (lastPlaceId - 10000);
            if (lane.isValid())
                ctx.project.getState().setProperty (ids::selectedLane, lane[ids::uid], nullptr);
            return;
        }
        auto p = ctx.project.patterns().getChild (patternBox.getSelectedId() - 1);
        if (p.isValid())
            ctx.project.setCurrentPattern (p[ids::uid]);
    };
    makeNonFocusing (patternBox);

    addAndMakeVisible (loopButton);
    loopButton.setButtonText (txt ("Bucle"));
    loopButton.setClickingTogglesState (true);
    loopButton.setTooltip (txt ("Repite una zona de la canción. Con el bucle encendido, arrastra sobre la regla (los números) "
                                "para elegir la zona. Si grabas, cada vuelta se guarda como una toma distinta."));
    loopButton.onClick = [this]
    {
        auto& st = ctx.project.getState();
        const bool on = loopButton.getToggleState();
        st.setProperty (ids::loopOn, on, nullptr);
        if (on && (double) st.getProperty (ids::loopEnd, 0.0) <= (double) st.getProperty (ids::loopStart, 0.0))
        {
            // Zona inicial: 4 compases desde donde está el cursor
            const double bar = ctx.project.getStepsPerBar();
            const double from = std::floor (ctx.engine.getPlayPosition() / bar) * bar;
            st.setProperty (ids::loopStart, from, nullptr);
            st.setProperty (ids::loopEnd, from + 4.0 * bar, nullptr);
        }
        grid.repaint();
    };
    makeNonFocusing (loopButton);

    addAndMakeVisible (liveButton);
    liveButton.setButtonText (txt ("Directo"));
    liveButton.setTooltip (txt ("Modo directo: lanza tus patrones con botones mientras suena, como un DJ. "
                                "Cada uno entra justo al empezar el siguiente compás."));
    liveButton.onClick = [this]
    {
        // Suena en modo patrón mientras la ventana está abierta
        ctx.project.getState().setProperty (ids::playMode, "pattern", nullptr);
        auto panel = std::make_unique<LivePanel> (ctx);
        auto* top = getTopLevelComponent();
        juce::CallOutBox::launchAsynchronously (std::move (panel), top->getLocalArea (this, liveButton.getBounds()), top);
    };
    makeNonFocusing (liveButton);

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

int Playlist::getLaneToPlace()
{
    const int id = patternBox.getSelectedId();
    if (id < 10000)
        return 0;
    auto lane = ctx.project.automation().getChild (id - 10000);
    return lane.isValid() ? (int) lane[ids::uid] : 0;
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
        default: return ctx.project.getStepsPerBar();
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
    // Las automatizaciones también se colocan como bloques
    {
        const int previous = patternBox.getSelectedId();
        juce::ignoreUnused (previous);
        int laneIndex = 0;
        bool first = true;
        for (auto lane : ctx.project.automation())
        {
            if (first) { patternBox.addSeparator(); first = false; }
            patternBox.addItem (txt ("Colocar automatización: ")
                                    + ctx.project.describeTarget (AutomationTarget::parse (lane[ids::target].toString())),
                                10000 + laneIndex++);
        }
        if (lastPlaceId >= 10000 && lastPlaceId - 10000 < laneIndex)
            select = lastPlaceId;
    }
    patternBox.setSelectedId (select, juce::dontSendNotification);

    const bool song = ctx.project.getState()[ids::playMode].toString() == "song";
    infoLabel.setText (song ? txt ("Modo CANCIÓN activo: al reproducir suena la playlist.")
                            : txt ("Estás en modo PATRÓN. Pulsa CANCIÓN arriba (o L) para escuchar la playlist."),
                       juce::dontSendNotification);
    infoLabel.setColour (juce::Label::textColourId, song ? Colours::green : Colours::accent);

    loopButton.setToggleState ((bool) ctx.project.getState().getProperty (ids::loopOn, false), juce::dontSendNotification);
    layoutGrid();
    grid.repaint();
}

void Playlist::timerCallback()
{
    if (isShowing())
        grid.updatePlayhead();
}

void Playlist::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, txt ("Playlist (canción)"));
}

void Playlist::layoutGrid()
{
    const int width = PlaylistGrid::labelWidth + (int) (grid.getTotalBars() * zoomSlider.getValue()) + 20;
    const int height = PlaylistGrid::rulerHeight + grid.totalTracksHeight() + PlaylistGrid::automationHeight;
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
    loopButton.setBounds (h.removeFromLeft (80)); h.removeFromLeft (8);
    liveButton.setBounds (h.removeFromLeft (80)); h.removeFromLeft (8);
    zoomSlider.setBounds (h.removeFromRight (120));
    h.removeFromRight (8);
    infoLabel.setBounds (h);

    viewport.setBounds (area.reduced (6));
    layoutGrid();
}
}
