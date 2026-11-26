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
                     "alargarlo. Clic derecho: borrar. Doble clic: editar el patrón. Clic en la regla: mover el cursor.\n"
                     "Grabaciones: arrastra el borde izquierdo para quitar el principio. Clic derecho (o mantener pulsado): "
                     "cortar, duplicar, volumen y fundidos."));
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
    return juce::jmax (32, (int) (ctx.project.getSongLengthSteps() / ctx.project.getStepsPerBar()) + 8);
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
        g.setColour (t % 2 == 0 ? juce::Colour (0xff1d2126) : juce::Colour (0xff1a1d22));
        g.fillRect (labelWidth, y, getWidth() - labelWidth, trackHeight);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawHorizontalLine (y + trackHeight - 1, 0.0f, (float) getWidth());
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

    // Nombres de las pistas
    for (int t = 0; t < numPlaylistTracks; ++t)
    {
        const int y = rulerHeight + t * trackHeight;
        auto label = juce::Rectangle<int> (0, y, labelWidth, trackHeight);
        g.setGradientFill (juce::ColourGradient (Colours::panelLight, 0.0f, (float) y, Colours::header, 0.0f, (float) (y + trackHeight), false));
        g.fillRect (label);
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.drawHorizontalLine (y + trackHeight - 1, 0.0f, (float) labelWidth);
        auto dot = juce::Rectangle<float> (10.0f, (float) y + trackHeight * 0.5f - 9.0f, 18.0f, 18.0f);
        g.setColour (Colours::panelDark);
        g.fillEllipse (dot);
        g.setColour (Colours::accent.withAlpha (0.8f));
        g.setFont (Fonts::bold (10.5f));
        g.drawText (juce::String (t + 1), dot, juce::Justification::centred);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (12.0f));
        g.drawText ("Pista", label.withTrimmedLeft (34), juce::Justification::centredLeft);
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
    });
}

juce::ValueTree PlaylistGrid::selectedLane()
{
    return ctx.project.automation().getChildWithProperty (ids::uid, ctx.project.getState()[ids::selectedLane]);
}

juce::Rectangle<float> PlaylistGrid::automationArea() const
{
    const float top = (float) (rulerHeight + numPlaylistTracks * trackHeight);
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
        return;

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
    const int height = PlaylistGrid::rulerHeight + numPlaylistTracks * PlaylistGrid::trackHeight + PlaylistGrid::automationHeight;
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
    zoomSlider.setBounds (h.removeFromRight (120));
    h.removeFromRight (8);
    infoLabel.setBounds (h);

    viewport.setBounds (area.reduced (6));
    layoutGrid();
}
}
