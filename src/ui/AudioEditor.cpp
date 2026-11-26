#include "AudioEditor.h"

namespace studoo
{
AudioEditorPanel::AudioEditorPanel (AppContext& c, juce::ValueTree ch) : ctx (c), channel (ch)
{
    if (auto sample = ctx.samples.get (channel[ids::sample].toString()))
    {
        audio = sample->buffer;
        sampleRate = sample->sampleRate;
    }

    struct B { const char* text; const char* tip; std::function<void()> action; };
    const std::vector<B> list = {
        { "Silenciar",      "Deja en silencio el trozo elegido.",                           [this] { run (Op::silence); } },
        { "Fundido entrada","El trozo empieza en silencio y sube poco a poco.",            [this] { run (Op::fadeIn); } },
        { "Fundido salida", "El trozo baja poco a poco hasta el silencio.",                [this] { run (Op::fadeOut); } },
        { "Al revés",       "El trozo suena del final al principio.",                      [this] { run (Op::reverse); } },
        { "Normalizar",     "Sube el trozo al máximo sin pasarse.",                        [this] { run (Op::normalise); } },
        { "+3 dB",          "Más fuerte.",                                                 [this] { run (Op::louder); } },
        { "-3 dB",          "Más flojo.",                                                  [this] { run (Op::quieter); } },
        { "Quitar ruido",   "Corta el ruido de fondo en las partes flojas.",               [this] { run (Op::denoise); } },
        { "Recortar",       "Se queda solo con el trozo elegido.",                         [this] { run (Op::crop); } },
        { "Borrar trozo",   "Quita el trozo elegido (lo de después se junta).",            [this] { run (Op::remove); } },
        { "Deshacer",       "Deshace el último cambio del editor.",                        [this]
            {
                if (history.empty()) return;
                audio = std::move (history.back());
                history.pop_back();
                selStart = selEnd = 0;
                status = txt ("Cambio deshecho.");
                repaint();
            } },
        { "Guardar",        "Guarda una copia editada y la usa en tu canción (el original no se toca).", [this] { save(); } },
    };
    for (auto& b : list)
    {
        auto* button = buttons.add (new juce::TextButton (txt (b.text)));
        button->setTooltip (txt (b.tip));
        button->onClick = b.action;
        makeNonFocusing (*button);
        addAndMakeVisible (button);
    }
    buttons.getLast()->setColour (juce::TextButton::buttonColourId, Colours::accent.withAlpha (0.6f));
    status = txt ("Arrastra sobre la onda para elegir un trozo. Sin elegir nada, se cambia todo.");
    setSize (760, 360);
}

juce::Rectangle<int> AudioEditorPanel::waveArea() const { return { 12, 32, getWidth() - 24, 200 }; }

int AudioEditorPanel::xToSample (float x) const
{
    auto a = waveArea();
    return juce::jlimit (0, audio.getNumSamples(), (int) ((x - a.getX()) / (float) a.getWidth() * (float) audio.getNumSamples()));
}

float AudioEditorPanel::sampleToX (int s) const
{
    auto a = waveArea();
    return (float) a.getX() + (float) a.getWidth() * (float) s / (float) juce::jmax (1, audio.getNumSamples());
}

void AudioEditorPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! waveArea().contains (e.getPosition()))
        return;
    anchor = selStart = selEnd = xToSample (e.position.x);
    repaint();
}

void AudioEditorPanel::mouseDrag (const juce::MouseEvent& e)
{
    const int s = xToSample (e.position.x);
    selStart = juce::jmin (anchor, s);
    selEnd = juce::jmax (anchor, s);
    repaint();
}

void AudioEditorPanel::apply (juce::AudioBuffer<float>& b, Op op, int from, int to)
{
    const int total = b.getNumSamples();
    from = juce::jlimit (0, total, from);
    to = juce::jlimit (from, total, to);
    const int len = to - from;
    if (len <= 0)
        return;
    switch (op)
    {
        case Op::silence:  b.clear (from, len); break;
        case Op::fadeIn:   b.applyGainRamp (from, len, 0.0f, 1.0f); break;
        case Op::fadeOut:  b.applyGainRamp (from, len, 1.0f, 0.0f); break;
        case Op::reverse:  b.reverse (from, len); break;
        case Op::normalise: { const float m = b.getMagnitude (from, len); if (m > 1.0e-5f) b.applyGain (from, len, 0.966f / m); break; }
        case Op::louder:   b.applyGain (from, len, 1.4125f); break;
        case Op::quieter:  b.applyGain (from, len, 0.7079f); break;
        case Op::denoise:
        {
            std::vector<float> levels;
            for (int i = from; i + 1024 <= to; i += 1024)
                levels.push_back (b.getRMSLevel (0, i, 1024));
            if (levels.empty()) return;
            std::sort (levels.begin(), levels.end());
            const float threshold = juce::jmax (0.001f, levels[levels.size() / 10] * 2.5f);
            float gain = 1.0f, env = 0.0f;
            for (int i = from; i < to; ++i)
            {
                float level = 0.0f;
                for (int ch = 0; ch < b.getNumChannels(); ++ch) level = juce::jmax (level, std::abs (b.getSample (ch, i)));
                env = level > env ? level : env * 0.9995f;
                gain += ((env > threshold ? 1.0f : 0.0f) - gain) * (env > threshold ? 0.05f : 0.0005f);
                for (int ch = 0; ch < b.getNumChannels(); ++ch) b.setSample (ch, i, b.getSample (ch, i) * gain);
            }
            break;
        }
        case Op::crop:
        {
            juce::AudioBuffer<float> out (b.getNumChannels(), len);
            for (int ch = 0; ch < b.getNumChannels(); ++ch) out.copyFrom (ch, 0, b, ch, from, len);
            b = std::move (out);
            break;
        }
        case Op::remove:
        {
            if (total - len < 16) return;   // no se puede quedar vacío
            juce::AudioBuffer<float> out (b.getNumChannels(), total - len);
            for (int ch = 0; ch < b.getNumChannels(); ++ch)
            {
                out.copyFrom (ch, 0, b, ch, 0, from);
                out.copyFrom (ch, from, b, ch, to, total - to);
            }
            b = std::move (out);
            break;
        }
    }
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        juce::FloatVectorOperations::clip (b.getWritePointer (ch), b.getReadPointer (ch), -1.0f, 1.0f, b.getNumSamples());
}

void AudioEditorPanel::run (Op op)
{
    if (audio.getNumSamples() == 0)
        return;
    const bool hasSel = selEnd - selStart > 16;
    if (! hasSel && (op == Op::crop || op == Op::remove))
    {
        status = txt ("Primero elige un trozo arrastrando sobre la onda.");
        repaint();
        return;
    }
    history.push_back (audio);
    if (history.size() > 10) history.erase (history.begin());
    apply (audio, op, hasSel ? selStart : 0, hasSel ? selEnd : audio.getNumSamples());
    if (op == Op::crop || op == Op::remove)
        selStart = selEnd = 0;
    status = txt ("Hecho. Pulsa Guardar para usarlo en tu canción.");
    repaint();
}

void AudioEditorPanel::save()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Studoo").getChildFile ("Grabaciones");
    dir.createDirectory();
    auto file = dir.getNonexistentChildFile (juce::File::createLegalFileName (channel[ids::name].toString()) + " (editado)", ".wav", false);
    {
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> writer (wav.createWriterFor (new juce::FileOutputStream (file), sampleRate,
                                                                               (unsigned int) audio.getNumChannels(), 24, {}, 0));
        if (writer == nullptr || ! writer->writeFromAudioSampleBuffer (audio, 0, audio.getNumSamples()))
        {
            status = txt ("No se pudo guardar el archivo.");
            repaint();
            return;
        }
    }
    ctx.project.beginTransaction();
    channel.setProperty (ids::sample, file.getFullPathName(), ctx.project.um());
    status = txt ("Guardado. Tu canción ya usa la versión editada.");
    history.clear();
    repaint();
}

void AudioEditorPanel::resized()
{
    auto area = getLocalBounds().reduced (12).withTrimmedTop (waveArea().getBottom() + 6 - 12);
    area.removeFromTop (20);   // texto de estado
    const int perRow = 6;
    const int w = area.getWidth() / perRow;
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setBounds (area.getX() + (i % perRow) * w, area.getY() + (i / perRow) * 40, w - 6, 32);
}

void AudioEditorPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::panel);
    g.setColour (Colours::text);
    g.setFont (Fonts::bold (14.0f));
    g.drawText (txt ("Editor de audio · ") + channel[ids::name].toString(), 12, 6, getWidth() - 24, 20, juce::Justification::centredLeft);

    auto a = waveArea();
    g.setColour (juce::Colour (0xff121417));
    g.fillRoundedRectangle (a.toFloat(), 6.0f);
    if (selEnd - selStart > 16)
    {
        g.setColour (Colours::accent.withAlpha (0.25f));
        g.fillRect (juce::Rectangle<float> (sampleToX (selStart), (float) a.getY(), sampleToX (selEnd) - sampleToX (selStart), (float) a.getHeight()));
    }
    const int total = audio.getNumSamples();
    if (total > 0)
    {
        g.setColour (Colours::forChannelType ("audio"));
        const float mid = (float) a.getCentreY(), half = a.getHeight() * 0.48f;
        for (int x = 0; x < a.getWidth(); ++x)
        {
            const int s0 = (int) ((juce::int64) total * x / a.getWidth());
            const int s1 = juce::jmax (s0 + 1, (int) ((juce::int64) total * (x + 1) / a.getWidth()));
            float m = 0.0f;
            for (int ch = 0; ch < audio.getNumChannels(); ++ch)
                m = juce::jmax (m, audio.getMagnitude (ch, s0, juce::jmin (s1, total) - s0));
            g.drawVerticalLine (a.getX() + x, mid - m * half, mid + m * half);
        }
    }
    g.setColour (Colours::textDim);
    g.setFont (Fonts::regular (11.5f));
    const double secs = total / juce::jmax (1.0, sampleRate);
    juce::String info = juce::String (secs, 2) + " s";
    if (selEnd - selStart > 16)
        info << txt ("  ·  trozo elegido: ") << juce::String ((selEnd - selStart) / sampleRate, 2) << " s";
    g.drawText (status + "   " + info, 12, a.getBottom() + 4, getWidth() - 24, 18, juce::Justification::centredLeft);
}
}
