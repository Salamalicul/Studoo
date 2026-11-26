#include "Visualizer.h"

namespace studoo
{
namespace visual
{
// ============================================================================
//  Análisis: bandas de frecuencia
// ============================================================================
void analyse (const float* mono, int num, double sampleRate, Bands& out)
{
    constexpr int order = 11, size = 1 << order;
    static thread_local juce::dsp::FFT fft (order);
    static thread_local juce::dsp::WindowingFunction<float> window ((size_t) size, juce::dsp::WindowingFunction<float>::hann);
    static thread_local std::array<float, size * 2> data;
    data.fill (0.0f);
    const int n = juce::jmin (num, size);
    for (int i = 0; i < n; ++i)
        data[(size_t) i] = mono[num - n + i];
    window.multiplyWithWindowingTable (data.data(), (size_t) size);
    fft.performFrequencyOnlyForwardTransform (data.data());

    for (int b = 0; b < numBands; ++b)
    {
        // Bandas repartidas como en un piano: de 40 Hz a 16 kHz
        const double f0 = 40.0 * std::pow (400.0, (double) b / numBands);
        const double f1 = 40.0 * std::pow (400.0, (double) (b + 1) / numBands);
        const int i0 = juce::jlimit (1, size / 2 - 1, (int) (f0 / sampleRate * size));
        const int i1 = juce::jlimit (i0 + 1, size / 2, (int) (f1 / sampleRate * size) + 1);
        float peak = 0.0f;
        for (int i = i0; i < i1; ++i)
            peak = juce::jmax (peak, data[(size_t) i]);
        const float db = juce::Decibels::gainToDecibels (peak / (size * 0.25f), -80.0f);
        out[(size_t) b] = juce::jlimit (0.0f, 1.0f, (db + 62.0f) / 58.0f);
    }
}

// ============================================================================
//  Dibujo de un fotograma
// ============================================================================
void draw (juce::Graphics& g, juce::Rectangle<float> r, const Bands& bands, float level, double time,
           const juce::String& title, int style, juce::Colour colour)
{
    float bass = 0.0f;
    for (int b = 0; b < 6; ++b) bass += bands[(size_t) b];
    bass /= 6.0f;

    // Fondo: degradado oscuro con un brillo que late con los graves
    const auto deep = colour.withMultipliedBrightness (0.12f).withMultipliedSaturation (0.8f);
    g.setGradientFill (juce::ColourGradient (deep.brighter (0.15f + bass * 0.4f), r.getCentreX(), r.getCentreY(),
                                             juce::Colour (0xff07080a), r.getX(), r.getY(), true));
    g.fillRect (r);

    // Estrellitas que se mueven despacio
    juce::Random rnd (1234);
    for (int i = 0; i < 70; ++i)
    {
        const float x = r.getX() + (float) std::fmod (rnd.nextFloat() * r.getWidth() + time * (8.0 + rnd.nextFloat() * 20.0), (double) r.getWidth());
        const float y = r.getY() + rnd.nextFloat() * r.getHeight();
        const float s = 1.0f + rnd.nextFloat() * 2.0f + bass * 2.0f;
        g.setColour (juce::Colours::white.withAlpha (0.08f + 0.25f * rnd.nextFloat() * (0.4f + level)));
        g.fillEllipse (x, y, s, s);
    }

    const auto bright = colour.brighter (0.3f);
    if (style == 1)
    {
        // Círculo: barras alrededor de un círculo que late
        const auto c = r.getCentre();
        const float base = juce::jmin (r.getWidth(), r.getHeight()) * (0.18f + bass * 0.05f);
        const int rays = numBands * 2;
        for (int k = 0; k < rays; ++k)
        {
            const int b = k < numBands ? k : rays - 1 - k;   // simétrico
            const float v = bands[(size_t) b];
            const float angle = (float) k / rays * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi + (float) time * 0.15f;
            const float len = base * 0.15f + v * base * 1.3f;
            const juce::Point<float> a (c.x + std::cos (angle) * base, c.y + std::sin (angle) * base);
            const juce::Point<float> e (c.x + std::cos (angle) * (base + len), c.y + std::sin (angle) * (base + len));
            g.setColour (bright.withRotatedHue ((float) b / numBands * 0.25f).withAlpha (0.5f + v * 0.5f));
            g.drawLine ({ a, e }, juce::jmax (2.0f, base * 0.035f));
        }
        g.setColour (bright.withAlpha (0.25f + bass * 0.5f));
        g.drawEllipse (juce::Rectangle<float> (base * 2.0f, base * 2.0f).withCentre (c), 3.0f);
        g.setColour (juce::Colour (0xff0c0d10).withAlpha (0.85f));
        g.fillEllipse (juce::Rectangle<float> (base * 1.9f, base * 1.9f).withCentre (c));
    }
    else if (style == 2)
    {
        // Ondas: varias líneas suaves que se mueven con cada zona del sonido
        for (int line = 0; line < 4; ++line)
        {
            juce::Path p;
            const float cy = r.getY() + r.getHeight() * (0.35f + 0.1f * line);
            for (float x = 0.0f; x <= r.getWidth(); x += 4.0f)
            {
                const float u = x / r.getWidth();
                const int b = juce::jlimit (0, numBands - 1, (int) (u * numBands));
                const float amp = r.getHeight() * 0.18f * bands[(size_t) b] * (1.0f - 0.18f * line);
                const float y = cy + amp * (float) std::sin (u * 18.0 + time * (2.0 + line) + line);
                if (x == 0.0f) p.startNewSubPath (r.getX() + x, y);
                else           p.lineTo (r.getX() + x, y);
            }
            g.setColour (bright.withRotatedHue (0.08f * line).withAlpha (0.75f - 0.13f * line));
            g.strokePath (p, juce::PathStrokeType (3.0f - 0.4f * line, juce::PathStrokeType::curved));
        }
    }
    else
    {
        // Barras: un espectro desde abajo, con reflejo
        const float floorY = r.getY() + r.getHeight() * 0.72f;
        const float bw = r.getWidth() * 0.8f / numBands;
        const float x0 = r.getX() + r.getWidth() * 0.1f;
        for (int b = 0; b < numBands; ++b)
        {
            const float v = bands[(size_t) b];
            const float h = juce::jmax (2.0f, v * r.getHeight() * 0.55f);
            auto bar = juce::Rectangle<float> (x0 + b * bw + bw * 0.15f, floorY - h, bw * 0.7f, h);
            const auto c = bright.withRotatedHue ((float) b / numBands * 0.3f);
            g.setGradientFill (juce::ColourGradient (c.brighter (0.4f), 0.0f, bar.getY(), c.darker (0.3f), 0.0f, floorY, false));
            g.fillRoundedRectangle (bar, bw * 0.2f);
            g.setColour (c.withAlpha (0.18f));
            g.fillRoundedRectangle (bar.withY (floorY + 2.0f).withHeight (h * 0.35f), bw * 0.2f);
        }
    }

    // Título de la canción
    if (title.isNotEmpty())
    {
        g.setColour (juce::Colours::white.withAlpha (0.92f));
        g.setFont (juce::FontOptions (r.getHeight() * 0.06f, juce::Font::bold));
        g.drawText (title, r.reduced (r.getWidth() * 0.05f, r.getHeight() * 0.05f), juce::Justification::bottomLeft, true);
    }
}

// ============================================================================
//  Vídeo AVI (fotogramas JPEG + sonido sin comprimir)
// ============================================================================
namespace
{
    struct AviWriter
    {
        juce::FileOutputStream& out;
        void fourcc (const char* c)   { out.write (c, 4); }
        void u32 (juce::uint32 v)     { out.writeInt ((int) v); }        // little endian
        void u16 (juce::uint16 v)     { out.writeShort ((short) v); }
        void patch (juce::int64 at, juce::uint32 v)
        {
            const auto here = out.getPosition();
            out.setPosition (at);
            u32 (v);
            out.setPosition (here);
        }
    };
}

bool writeVideo (const juce::File& wav, const juce::File& avi, const juce::String& title, int style,
                 juce::Colour colour, int width, int height, int fps,
                 std::function<bool (float)> progress, juce::String& error)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (wav));
    if (reader == nullptr || reader->lengthInSamples <= 0)
    {
        error = txt ("No se pudo leer el audio de la canción.");
        return false;
    }
    const double sr = reader->sampleRate;
    const int total = (int) juce::jmin ((juce::int64) (sr * 60.0 * 20.0), reader->lengthInSamples);   // como mucho 20 minutos
    juce::AudioBuffer<float> audio (2, total);
    reader->read (&audio, 0, total, 0, true, true);
    reader.reset();

    width &= ~1;
    height &= ~1;
    const int frames = (int) std::ceil (total / sr * fps);

    avi.deleteFile();
    juce::FileOutputStream out (avi);
    if (! out.openedOk())
    {
        error = txt ("No se puede crear el archivo del vídeo. ¿Tienes permiso para escribir en esa carpeta?");
        return false;
    }
    AviWriter w { out };
    const juce::uint32 bytesPerSec = (juce::uint32) sr * 4;
    const juce::uint32 maxFrameBytes = (juce::uint32) (width * height * 3);

    // --- Cabecera ---------------------------------------------------------------------
    w.fourcc ("RIFF"); const auto riffSizeAt = out.getPosition(); w.u32 (0); w.fourcc ("AVI ");
    w.fourcc ("LIST"); w.u32 (294); w.fourcc ("hdrl");
    w.fourcc ("avih"); w.u32 (56);
    w.u32 ((juce::uint32) (1000000 / fps)); w.u32 (maxFrameBytes * (juce::uint32) fps + bytesPerSec); w.u32 (0);
    w.u32 (0x10 | 0x100);   // tiene índice, intercalado
    w.u32 ((juce::uint32) frames); w.u32 (0); w.u32 (2); w.u32 (maxFrameBytes);
    w.u32 ((juce::uint32) width); w.u32 ((juce::uint32) height); w.u32 (0); w.u32 (0); w.u32 (0); w.u32 (0);
    // Vídeo
    w.fourcc ("LIST"); w.u32 (116); w.fourcc ("strl");
    w.fourcc ("strh"); w.u32 (56);
    w.fourcc ("vids"); w.fourcc ("MJPG"); w.u32 (0); w.u16 (0); w.u16 (0); w.u32 (0);
    w.u32 (1); w.u32 ((juce::uint32) fps); w.u32 (0); w.u32 ((juce::uint32) frames); w.u32 (maxFrameBytes);
    w.u32 (0xffffffffu); w.u32 (0); w.u16 (0); w.u16 (0); w.u16 ((juce::uint16) width); w.u16 ((juce::uint16) height);
    w.fourcc ("strf"); w.u32 (40);
    w.u32 (40); w.u32 ((juce::uint32) width); w.u32 ((juce::uint32) height); w.u16 (1); w.u16 (24); w.fourcc ("MJPG");
    w.u32 (maxFrameBytes); w.u32 (0); w.u32 (0); w.u32 (0); w.u32 (0);
    // Sonido
    w.fourcc ("LIST"); w.u32 (94); w.fourcc ("strl");
    w.fourcc ("strh"); w.u32 (56);
    w.fourcc ("auds"); w.u32 (0); w.u32 (0); w.u16 (0); w.u16 (0); w.u32 (0);
    w.u32 (4); w.u32 (bytesPerSec); w.u32 (0); w.u32 ((juce::uint32) total); w.u32 (bytesPerSec);
    w.u32 (0xffffffffu); w.u32 (4); w.u16 (0); w.u16 (0); w.u16 (0); w.u16 (0);
    w.fourcc ("strf"); w.u32 (18);
    w.u16 (1); w.u16 (2); w.u32 ((juce::uint32) sr); w.u32 (bytesPerSec); w.u16 (4); w.u16 (16); w.u16 (0);

    // --- Datos -------------------------------------------------------------------------
    w.fourcc ("LIST"); const auto moviSizeAt = out.getPosition(); w.u32 (0);
    const auto moviStart = out.getPosition(); w.fourcc ("movi");
    struct IndexEntry { char id[4]; juce::uint32 offset, size; };
    std::vector<IndexEntry> index;
    index.reserve ((size_t) frames * 2);

    juce::Image image (juce::Image::RGB, width, height, true);
    juce::JPEGImageFormat jpeg;
    jpeg.setQuality (0.8f);
    Bands bands {}, smooth {};
    std::vector<float> mono (2048, 0.0f);
    std::vector<juce::int16> pcm;

    for (int f = 0; f < frames; ++f)
    {
        const int s0 = (int) std::llround ((double) f * sr / fps);
        const int s1 = juce::jmin (total, (int) std::llround ((double) (f + 1) * sr / fps));
        // Análisis del sonido de este momento
        const int centre = juce::jlimit (0, total, s0 + (s1 - s0) / 2);
        float rms = 0.0f;
        for (int i = 0; i < 2048; ++i)
        {
            const int idx = centre - 2048 + i;
            const float v = idx >= 0 && idx < total ? 0.5f * (audio.getSample (0, idx) + audio.getSample (1, idx)) : 0.0f;
            mono[(size_t) i] = v;
            rms += v * v;
        }
        analyse (mono.data(), 2048, sr, bands);
        for (size_t b = 0; b < smooth.size(); ++b)
            smooth[b] = juce::jmax (bands[b], smooth[b] * 0.82f);   // sube rápido, baja despacio
        const float level = juce::jlimit (0.0f, 1.0f, std::sqrt (rms / 2048.0f) * 3.0f);

        {
            juce::Graphics g (image);
            draw (g, image.getBounds().toFloat(), smooth, level, (double) f / fps, title, style, colour);
        }
        juce::MemoryOutputStream jpg;
        jpeg.writeImageToStream (image, jpg);
        index.push_back ({ { '0', '0', 'd', 'c' }, (juce::uint32) (out.getPosition() - moviStart), (juce::uint32) jpg.getDataSize() });
        w.fourcc ("00dc"); w.u32 ((juce::uint32) jpg.getDataSize());
        out.write (jpg.getData(), jpg.getDataSize());
        if (jpg.getDataSize() % 2 != 0) out.writeByte (0);

        // El sonido de este fotograma (16 bits, estéreo)
        pcm.resize ((size_t) juce::jmax (0, s1 - s0) * 2);
        for (int i = s0; i < s1; ++i)
            for (int ch = 0; ch < 2; ++ch)
                pcm[(size_t) (i - s0) * 2 + (size_t) ch] = (juce::int16) juce::jlimit (-32767, 32767, (int) std::lround (audio.getSample (ch, i) * 32767.0f));
        const auto bytes = (juce::uint32) (pcm.size() * sizeof (juce::int16));
        index.push_back ({ { '0', '1', 'w', 'b' }, (juce::uint32) (out.getPosition() - moviStart), bytes });
        w.fourcc ("01wb"); w.u32 (bytes);
        out.write (pcm.data(), bytes);

        if (progress != nullptr && ! progress ((float) f / (float) frames))
        {
            out.flush();
            error = txt ("Exportación cancelada.");
            return false;
        }
    }
    const auto moviEnd = out.getPosition();
    w.patch (moviSizeAt, (juce::uint32) (moviEnd - moviStart));

    // --- Índice ------------------------------------------------------------------------------
    w.fourcc ("idx1"); w.u32 ((juce::uint32) (index.size() * 16));
    for (auto& e : index)
    {
        out.write (e.id, 4);
        w.u32 (0x10);   // fotograma clave
        w.u32 (e.offset);
        w.u32 (e.size);
    }
    w.patch (riffSizeAt, (juce::uint32) (out.getPosition() - 8));
    out.flush();
    if (out.getStatus().failed())
    {
        error = txt ("No se pudo terminar de escribir el vídeo (¿falta espacio?).");
        return false;
    }
    return true;
}
}

// ============================================================================
//  Ventana del visualizador
// ============================================================================
VisualizerPanel::VisualizerPanel (AppContext& c) : ctx (c)
{
    addAndMakeVisible (styleBox);
    styleBox.addItem (txt ("Barras"), 1);
    styleBox.addItem (txt ("Círculo"), 2);
    styleBox.addItem (txt ("Ondas"), 3);
    styleBox.setSelectedId (1, juce::dontSendNotification);
    styleBox.setTooltip (txt ("Cómo se ve la animación."));
    makeNonFocusing (styleBox);

    addAndMakeVisible (colourBox);
    const char* names[] = { "Naranja", "Azul", "Morado", "Verde", "Rosa" };
    for (int i = 0; i < 5; ++i)
        colourBox.addItem (txt (names[i]), i + 1);
    colourBox.setSelectedId (1, juce::dontSendNotification);
    makeNonFocusing (colourBox);

    addAndMakeVisible (sizeBox);
    sizeBox.addItem (txt ("Vídeo horizontal (YouTube) 854x480"), 1);
    sizeBox.addItem (txt ("Vídeo vertical (TikTok, Reels) 480x854"), 2);
    sizeBox.addItem (txt ("Cuadrado (Instagram) 640x640"), 3);
    sizeBox.setSelectedId (1, juce::dontSendNotification);
    makeNonFocusing (sizeBox);

    addAndMakeVisible (exportButton);
    exportButton.setButtonText (txt ("Exportar vídeo..."));
    exportButton.setTooltip (txt ("Crea un vídeo (.avi) con esta animación y tu canción de la playlist, para subirlo a redes."));
    exportButton.onClick = [this] { exportVideo(); };
    makeNonFocusing (exportButton);

    setSize (720, 480);
    startTimerHz (30);
}

namespace
{
    juce::Colour visualColour (int id)
    {
        switch (id)
        {
            case 2:  return juce::Colour (0xff4fc3f7);
            case 3:  return juce::Colour (0xffb388ff);
            case 4:  return juce::Colour (0xff69f0ae);
            case 5:  return juce::Colour (0xffff6fa8);
            default: return juce::Colour (0xffff8a1f);
        }
    }
}

void VisualizerPanel::timerCallback()
{
    std::array<float, 2048> mono {};
    ctx.engine.getScope (mono.data(), (int) mono.size());
    visual::Bands now {};
    visual::analyse (mono.data(), (int) mono.size(), ctx.engine.getSampleRate(), now);
    float rms = 0.0f;
    for (float v : mono) rms += v * v;
    level = juce::jlimit (0.0f, 1.0f, std::sqrt (rms / (float) mono.size()) * 3.0f);
    for (size_t b = 0; b < bands.size(); ++b)
        bands[b] = juce::jmax (now[b], bands[b] * 0.82f);
    repaint();
}

void VisualizerPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::panel);
    auto area = getLocalBounds().reduced (10).withTrimmedBottom (44).toFloat();
    juce::Graphics::ScopedSaveState state (g);
    g.reduceClipRegion (area.toNearestInt());
    const auto title = ctx.project.getDisplayName();
    visual::draw (g, area, bands, level, (juce::Time::getMillisecondCounterHiRes() - startMs) / 1000.0, title,
                  styleBox.getSelectedId() - 1, visualColour (colourBox.getSelectedId()));
    if (! ctx.engine.isPlaying())
    {
        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.setFont (Fonts::bold (14.0f));
        g.drawText (status.isNotEmpty() ? status : txt ("Pulsa Espacio para que suene y ver la animación"), area.reduced (10.0f),
                    juce::Justification::centredTop);
    }
}

void VisualizerPanel::resized()
{
    auto row = getLocalBounds().reduced (10).removeFromBottom (34);
    exportButton.setBounds (row.removeFromRight (150));
    row.removeFromRight (8);
    styleBox.setBounds (row.removeFromLeft (120)); row.removeFromLeft (8);
    colourBox.setBounds (row.removeFromLeft (110)); row.removeFromLeft (8);
    sizeBox.setBounds (row.removeFromLeft (juce::jmin (290, row.getWidth())));
}

void VisualizerPanel::exportVideo()
{
    if (ctx.exportVideo == nullptr)
        return;
    const int size = sizeBox.getSelectedId();
    const int w = size == 2 ? 480 : size == 3 ? 640 : 854;
    const int h = size == 2 ? 854 : size == 3 ? 640 : 480;
    ctx.exportVideo (styleBox.getSelectedId() - 1, visualColour (colourBox.getSelectedId()), w, h);
}
}
