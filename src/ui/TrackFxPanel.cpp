#include "TrackFxPanel.h"
#include "model/Scales.h"

namespace studoo
{
namespace
{
    constexpr int knobW = 62, knobH = 58, rowGap = 26, pad = 14;
    const char* noteNames[] = { "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si" };
}

TrackFxPanel::TrackFxPanel (AppContext& c, int trackIndex) : ctx (c), index (trackIndex)
{
    track = ctx.project.getMixerTrack (index);
    auto* um = ctx.project.um();

    // --- Envíos 2 y 3 ---------------------------------------------------------------
    for (int s = 1; s < numSends; ++s)
    {
        auto* box = combo (txt ("A qué pista manda esta copia (solo a pistas de número mayor)."));
        box->addItem (txt ("Sin envío"), 1);
        for (int t = index + 1; t < numMixerTracks; ++t)
            box->addItem (txt ("→ ") + juce::String (t) + ". " + ctx.project.getMixerTrack (t)[ids::name].toString(), t + 1);
        const int current = track.getProperty (sendToId (s), 0);
        box->setSelectedId (current > index ? current + 1 : 1, juce::dontSendNotification);
        box->onChange = [this, box, s]
        {
            ctx.project.beginTransaction();
            const int target = juce::jmax (0, box->getSelectedId() - 1);
            track.setProperty (sendToId (s), target, ctx.project.um());
            if (target > 0 && (float) track.getProperty (sendLevelId (s), 0.0f) < 0.01f)
                track.setProperty (sendLevelId (s), 0.8f, ctx.project.um());
        };
        box->setEnabled (index > 0 && index < numMixerTracks - 1);
        sendBoxes[(size_t) s - 1] = box;
        sendKnobs[(size_t) s - 1] = knob (txt ("Envío ") + juce::String (s + 1), txt ("Cuánto sonido se manda a esa pista."),
                                          sendLevelId (s), 0.0, 1.0, 0.0);
    }

    // --- Autotune ------------------------------------------------------------------
    tuneAmount = knob ("Autotune", txt ("Corrige la afinación de la voz llevándola a la nota más cercana de la escala. "
                                        "Poco = natural; al 100% = el efecto \"robot\" del trap."), ids::tuneAmount, 0.0, 1.0, 0.0);
    tuneAmount->setDisplayFunction ([] (double v) { return v < 0.005 ? txt ("Apagado") : juce::String ((int) (v * 100.0)) + "%"; });
    tuneRoot = combo (txt ("La nota base de la escala de tu canción."));
    for (int i = 0; i < 12; ++i)
        tuneRoot->addItem (noteNames[i], i + 1);
    tuneRoot->setSelectedId ((int) track.getProperty (ids::tuneRoot, 0) + 1, juce::dontSendNotification);
    tuneRoot->onChange = [this] { ctx.project.beginTransaction(); track.setProperty (ids::tuneRoot, tuneRoot->getSelectedId() - 1, ctx.project.um()); };
    tuneScale = combo (txt ("La escala: el autotune solo deja las notas de esta escala."));
    const juce::StringArray scaleNames { txt ("Todas las notas"), txt ("Mayor"), txt ("Menor"), txt ("Menor armónica"),
                                         txt ("Pentatónica menor"), txt ("Pentatónica mayor"), "Blues", txt ("Dórica"), txt ("Frigia") };
    for (int i = 0; i < scaleNames.size(); ++i)
        tuneScale->addItem (scaleNames[i], i + 1);
    tuneScale->setSelectedId ((int) track.getProperty (ids::tuneScale, 0) + 1, juce::dontSendNotification);
    tuneScale->onChange = [this] { ctx.project.beginTransaction(); track.setProperty (ids::tuneScale, tuneScale->getSelectedId() - 1, ctx.project.um()); };

    // --- Puerta y maximizador ---------------------------------------------------------
    gate = knob ("Puerta", txt ("Puerta de ruido: lo que suena más flojo que este nivel se corta (quita ruido de fondo entre frases)."),
                 ids::gateThreshold, -80.0, 0.0, -80.0, -40.0);
    gate->setDisplayFunction ([] (double v) { return v <= -79.0 ? txt ("Apagada") : juce::String ((int) v) + " dB"; });
    maximize = knob ("Maximizar", txt ("Sube el volumen sin pasar del máximo (usa el limitador). Para el Master al terminar la canción."),
                     ids::maximize, 0.0, 12.0, 0.0);
    maximize->setDisplayFunction ([] (double v) { return "+" + juce::String (v, 1) + " dB"; });

    // --- Ritmo --------------------------------------------------------------------------
    rhythmMode = combo (txt ("Efectos de ritmo que siguen el tempo (suenan al reproducir)."));
    const juce::StringArray modes { txt ("Apagado"), txt ("Half-time (a mitad de velocidad)"), txt ("Tartamudeo 1/8"),
                                    txt ("Tartamudeo 1/16"), txt ("Puerta rítmica 1/16"), txt ("Puerta rítmica 1/32"),
                                    txt ("Parada de cinta (último tiempo)") };
    for (int i = 0; i < modes.size(); ++i)
        rhythmMode->addItem (modes[i], i + 1);
    rhythmMode->setSelectedId ((int) track.getProperty (ids::rhythmMode, 0) + 1, juce::dontSendNotification);
    rhythmMode->onChange = [this] { ctx.project.beginTransaction(); track.setProperty (ids::rhythmMode, rhythmMode->getSelectedId() - 1, ctx.project.um()); };
    rhythmMix = knob ("Mezcla", txt ("Cuánto se oye el efecto de ritmo."), ids::rhythmMix, 0.0, 1.0, 1.0);

    // --- EQ paramétrico --------------------------------------------------------------------
    static const double defaultFreq[] = { 100.0, 400.0, 2000.0, 8000.0 };
    for (int b = 0; b < numEqBands; ++b)
    {
        auto* f = knob ("Frec " + juce::String (b + 1), txt ("Qué frecuencia cambia esta banda (graves a la izquierda, agudos a la derecha)."),
                        peqId ("Freq", b), 20.0, 20000.0, defaultFreq[b], 1000.0);
        f->setDisplayFunction ([] (double v) { return v >= 1000.0 ? juce::String (v / 1000.0, 1) + " kHz" : juce::String ((int) v) + " Hz"; });
        auto* g = knob (txt ("Ganancia"), txt ("Cuánto sube o baja esa frecuencia."), peqId ("Gain", b), -18.0, 18.0, 0.0);
        g->setDisplayFunction ([] (double v) { return (v > 0 ? "+" : "") + juce::String (v, 1) + " dB"; });
        auto* q = knob ("Q", txt ("Anchura: bajo = cambia una zona ancha; alto = una zona muy estrecha (para quitar un pitido)."),
                        peqId ("Q", b), 0.2, 10.0, 1.0, 1.5);
        q->setDisplayFunction ([] (double v) { return juce::String (v, 1); });
        eqKnobs[(size_t) b] = { f, g, q };
    }
    juce::ignoreUnused (um);

    ctx.engine.setPitchAnalysis (index, true);
    startTimerHz (12);
    setSize (pad * 2 + 8 * knobW, 470);
}

TrackFxPanel::~TrackFxPanel()
{
    ctx.engine.setPitchAnalysis (index, false);
}

Knob* TrackFxPanel::knob (const juce::String& label, const juce::String& tip, const juce::Identifier& id,
                          double min, double max, double def, double skewMid)
{
    auto* k = new Knob (label, tip);
    k->bind (track, id, ctx.project.um(), min, max, def, skewMid);
    owned.add (k);
    addAndMakeVisible (k);
    return k;
}

juce::ComboBox* TrackFxPanel::combo (const juce::String& tip)
{
    auto* c = new juce::ComboBox();
    c->setTooltip (tip);
    makeNonFocusing (*c);
    owned.add (c);
    addAndMakeVisible (c);
    return c;
}

void TrackFxPanel::timerCallback()
{
    const float hz = ctx.engine.getDetectedPitch (index);
    if (hz > 20.0f)
    {
        const double midi = 69.0 + 12.0 * std::log2 (hz / 440.0);
        const int nearest = juce::roundToInt (midi);
        tunerCents = (float) ((midi - nearest) * 100.0);
        tunerText = noteName (nearest) + "  " + (tunerCents >= 0 ? "+" : "") + juce::String ((int) tunerCents) + " cents";
        tunerActive = true;
    }
    else
    {
        tunerActive = false;
        tunerText = txt ("(canta o toca una nota)");
    }
    repaint (getLocalBounds().withHeight (130));
    repaint (eqCurveArea);
}

void TrackFxPanel::resized()
{
    auto area = getLocalBounds().reduced (pad);
    area.removeFromTop (rowGap);   // título
    // Fila 1: envíos
    {
        auto row = area.removeFromTop (knobH);
        for (size_t s = 0; s < sendBoxes.size(); ++s)
        {
            auto col = row.removeFromLeft (row.getWidth() / (int) (sendBoxes.size() - s));
            sendBoxes[s]->setBounds (col.removeFromLeft (150).withSizeKeepingCentre (146, 26));
            sendKnobs[s]->setBounds (col.removeFromLeft (knobW));
        }
    }
    area.removeFromTop (rowGap);
    // Fila 2: autotune + afinador | puerta | maximizar
    {
        auto row = area.removeFromTop (knobH);
        tuneAmount->setBounds (row.removeFromLeft (knobW + 6));
        auto combos = row.removeFromLeft (150);
        tuneRoot->setBounds (combos.removeFromTop (knobH / 2).reduced (2, 2));
        tuneScale->setBounds (combos.reduced (2, 2));
        row.removeFromLeft (130);   // afinador (se dibuja)
        gate->setBounds (row.removeFromLeft (knobW + 6));
        maximize->setBounds (row.removeFromLeft (knobW + 6));
    }
    area.removeFromTop (rowGap);
    // Fila 3: ritmo
    {
        auto row = area.removeFromTop (34);
        rhythmMode->setBounds (row.removeFromLeft (260).reduced (0, 3));
        row.removeFromLeft (10);
        rhythmMix->setBounds (row.removeFromLeft (knobW).withHeight (knobH).translated (0, -12));
    }
    area.removeFromTop (rowGap + 4);
    // Fila 4: EQ paramétrico (curva + 4 bandas de 3 ruedas)
    eqCurveArea = area.removeFromTop (70);
    area.removeFromTop (6);
    auto row = area.removeFromTop (knobH);
    const int bandW = row.getWidth() / numEqBands;
    for (auto& band : eqKnobs)
    {
        auto col = row.removeFromLeft (bandW);
        for (auto* k : band)
            k->setBounds (col.removeFromLeft (bandW / 3));
    }
}

void TrackFxPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::panel);
    auto title = [&g] (int x, int y, const juce::String& text)
    {
        g.setColour (Colours::accent);
        g.setFont (Fonts::bold (11.0f));
        g.drawText (text, x, y, 300, 14, juce::Justification::centredLeft);
    };
    const auto name = index == 0 ? juce::String ("Master") : juce::String (index) + ". " + track[ids::name].toString();
    g.setColour (Colours::text);
    g.setFont (Fonts::bold (14.0f));
    g.drawText (txt ("Más efectos · ") + name, pad, 4, getWidth() - pad * 2, 18, juce::Justification::centredLeft);

    title (pad, sendBoxes[0]->getY() - 16, txt ("ENVÍOS 2 Y 3"));
    title (pad, tuneAmount->getY() - 16, txt ("AUTOTUNE · AFINADOR"));
    title (gate->getX(), gate->getY() - 16, txt ("PUERTA · MAXIMIZAR"));
    title (pad, rhythmMode->getY() - 16, txt ("RITMO (TIPO GROSS BEAT)"));
    title (pad, eqCurveArea.getY() - 16, txt ("EQ PARAMÉTRICO (4 BANDAS)"));

    // Afinador
    auto tuner = juce::Rectangle<int> (tuneScale->getRight() + 8, tuneAmount->getY(), 120, knobH);
    g.setColour (juce::Colour (0xff121417));
    g.fillRoundedRectangle (tuner.toFloat(), 6.0f);
    g.setColour (tunerActive ? (std::abs (tunerCents) < 10.0f ? Colours::green : Colours::accent) : Colours::textDim);
    g.setFont (Fonts::bold (tunerActive ? 15.0f : 10.0f));
    g.drawFittedText (tunerActive ? tunerText.upToFirstOccurrenceOf ("  ", false, false) : tunerText,
                      tuner.withTrimmedBottom (22), juce::Justification::centred, 2);
    if (tunerActive)
    {
        auto bar = tuner.removeFromBottom (18).reduced (8, 5).toFloat();
        g.setColour (Colours::outline);
        g.fillRect (bar);
        const float x = bar.getCentreX() + bar.getWidth() * 0.5f * juce::jlimit (-1.0f, 1.0f, tunerCents / 50.0f);
        g.setColour (std::abs (tunerCents) < 10.0f ? Colours::green : Colours::accent);
        g.fillRect (juce::Rectangle<float> (x - 2.0f, bar.getY() - 2.0f, 4.0f, bar.getHeight() + 4.0f));
        g.setColour (Colours::text);
        g.drawVerticalLine ((int) bar.getCentreX(), bar.getY() - 3.0f, bar.getBottom() + 3.0f);
    }
    drawEqCurve (g);
}

void TrackFxPanel::drawEqCurve (juce::Graphics& g)
{
    auto r = eqCurveArea.toFloat();
    g.setColour (juce::Colour (0xff121417));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (Colours::outline);
    g.drawHorizontalLine ((int) r.getCentreY(), r.getX(), r.getRight());
    // Curva: suma de los 4 filtros (en dB) a lo largo de las frecuencias
    juce::Path curve;
    const double sr = 44100.0;
    std::array<juce::dsp::IIR::Coefficients<float>::Ptr, numEqBands> coefs;
    for (int b = 0; b < numEqBands; ++b)
    {
        const float f = juce::jlimit (20.0f, 19000.0f, (float) track.getProperty (peqId ("Freq", b), 1000.0f));
        const float gain = juce::jlimit (-18.0f, 18.0f, (float) track.getProperty (peqId ("Gain", b), 0.0f));
        const float q = juce::jlimit (0.2f, 10.0f, (float) track.getProperty (peqId ("Q", b), 1.0f));
        coefs[(size_t) b] = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sr, f, q, juce::Decibels::decibelsToGain (gain));
    }
    for (float x = r.getX(); x <= r.getRight(); x += 2.0f)
    {
        const double freq = 20.0 * std::pow (1000.0, (x - r.getX()) / r.getWidth());
        double db = 0.0;
        for (auto& c : coefs)
            db += juce::Decibels::gainToDecibels (c->getMagnitudeForFrequency (freq, sr));
        const float y = r.getCentreY() - (float) juce::jlimit (-18.0, 18.0, db) / 18.0f * (r.getHeight() * 0.45f);
        if (x == r.getX()) curve.startNewSubPath (x, y);
        else               curve.lineTo (x, y);
    }
    g.setColour (Colours::accent);
    g.strokePath (curve, juce::PathStrokeType (2.0f));
    g.setColour (Colours::textDim);
    g.setFont (Fonts::regular (9.0f));
    g.drawText ("20 Hz", r.reduced (4.0f), juce::Justification::bottomLeft);
    g.drawText ("20 kHz", r.reduced (4.0f), juce::Justification::bottomRight);
}
}
