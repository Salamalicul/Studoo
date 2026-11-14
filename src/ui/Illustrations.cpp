#include "Illustrations.h"
#include "StudooLookAndFeel.h"
#include <cmath>
#include <vector>

namespace studoo::illustrations
{
namespace
{
    using R = juce::Rectangle<float>;
    constexpr float pi = juce::MathConstants<float>::pi;

    // --------------------------------------------------------------------------
    //  Piezas comunes
    // --------------------------------------------------------------------------
    void text (juce::Graphics& g, const juce::String& s, R r, juce::Colour c, float size, bool bold = false,
               juce::Justification j = juce::Justification::centred)
    {
        g.setColour (c);
        g.setFont (bold ? Fonts::bold (size) : Fonts::regular (size));
        g.drawFittedText (s, r.toNearestInt(), j, 2, 1.0f);
    }

    // Textos escritos directamente en el código (con tildes): se leen como UTF-8
    void text (juce::Graphics& g, const char* s, R r, juce::Colour c, float size, bool bold = false,
               juce::Justification j = juce::Justification::centred)
    {
        text (g, juce::String::fromUTF8 (s), r, c, size, bold, j);
    }

    void box (juce::Graphics& g, R r, juce::Colour c, float corner = 8.0f, float alpha = 0.16f)
    {
        g.setColour (c.withAlpha (alpha));
        g.fillRoundedRectangle (r, corner);
        g.setColour (c.withAlpha (0.7f));
        g.drawRoundedRectangle (r.reduced (0.5f), corner, 1.2f);
    }

    void arrow (juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Colour c, float thickness = 2.0f)
    {
        g.setColour (c);
        juce::Path p;
        p.addArrow ({ a, b }, thickness, thickness * 5.0f, thickness * 4.0f);
        g.fillPath (p);
    }

    juce::Path wavePath (R r, int points, const std::function<float (float)>& f)
    {
        juce::Path p;
        for (int i = 0; i <= points; ++i)
        {
            const float x = (float) i / (float) points;
            const float y = r.getCentreY() - f (x) * r.getHeight() * 0.5f;
            const float px = r.getX() + x * r.getWidth();
            if (i == 0) p.startNewSubPath (px, y);
            else        p.lineTo (px, y);
        }
        return p;
    }

    // Paso actual (0-15) de un patrón de 16 pasos a un BPM dado
    float stepPosition (double time, double bpm, int steps = 16)
    {
        return (float) std::fmod (time * bpm / 60.0 * 4.0, (double) steps);
    }

    // Cuadrícula de batería animada: cada fila es "nombre", color y pasos ('x' = golpe)
    struct DrumRow { const char* name; juce::Colour colour; const char* steps; };

    void drumGrid (juce::Graphics& g, R area, std::initializer_list<DrumRow> rows, double bpm, double time,
                   const char* footer = nullptr)
    {
        const int length = 16;
        const float labelW = juce::jmin (90.0f, area.getWidth() * 0.16f);
        if (footer != nullptr)
        {
            text (g, juce::String::fromUTF8 (footer), area.removeFromBottom (22.0f), Colours::textDim, 12.5f);
            area.removeFromBottom (4.0f);
        }
        auto numbers = area.removeFromTop (18.0f);
        const float pos = stepPosition (time, bpm);
        const int current = (int) pos;
        const float rowH = area.getHeight() / (float) rows.size();
        auto gridX = area.getX() + labelW;
        const float gap = 3.0f, groupGap = 6.0f;
        const float cellW = (area.getRight() - gridX - groupGap * 3.0f - gap * 15.0f) / 16.0f;
        auto cellX = [&] (int s) { return gridX + (float) s * (cellW + gap) + (float) (s / 4) * groupGap; };

        for (int s = 0; s < length; ++s)
            text (g, juce::String (s + 1), R (cellX (s), numbers.getY(), cellW, numbers.getHeight()),
                  s % 4 == 0 ? Colours::text : Colours::textDim, 10.5f, s % 4 == 0);

        // Columna del paso que suena
        g.setColour (Colours::accent.withAlpha (0.12f));
        g.fillRoundedRectangle (R (cellX (current) - 2.0f, area.getY(), cellW + 4.0f, area.getHeight()), 4.0f);

        int r = 0;
        for (auto& row : rows)
        {
            const float y = area.getY() + (float) r * rowH;
            text (g, juce::String::fromUTF8 (row.name), R (area.getX(), y, labelW - 8.0f, rowH),
                  row.colour.brighter (0.2f), 12.5f, true, juce::Justification::centredLeft);
            for (int s = 0; s < length; ++s)
            {
                auto cell = R (cellX (s), y + 3.0f, cellW, rowH - 6.0f);
                const bool on = row.steps[s] == 'x' || row.steps[s] == 'o';
                if (on)
                {
                    const bool hit = s == current;
                    const float flash = hit ? 1.0f - (pos - (float) current) : 0.0f;
                    Draw::glow (g, cell, row.colour, 3.0f + flash * 8.0f, 4.0f);
                    Draw::colourFill (g, cell, row.colour.brighter (flash * 0.5f), 4.0f);
                    if (row.steps[s] == 'o') // golpe suave
                    {
                        g.setColour (juce::Colours::black.withAlpha (0.35f));
                        g.fillRoundedRectangle (cell, 4.0f);
                    }
                }
                else
                {
                    g.setColour (((s / 4) % 2 == 0) ? Colours::stepOffA : Colours::stepOffB);
                    g.fillRoundedRectangle (cell, 4.0f);
                }
            }
            ++r;
        }
    }

    // Teclado: 'first' es la primera tecla blanca (0 = Do). marks: notas (0-23 desde Do) resaltadas
    void keyboard (juce::Graphics& g, R area, int octaves, const std::vector<int>& marks, juce::Colour markColour,
                   bool names, int playing = -1)
    {
        static const char* whiteNames[] = { "Do", "Re", "Mi", "Fa", "Sol", "La", "Si" };
        static const int whiteSemis[] = { 0, 2, 4, 5, 7, 9, 11 };
        const int numWhite = octaves * 7;
        const float w = area.getWidth() / (float) numWhite;
        auto isMarked = [&] (int n) { return std::find (marks.begin(), marks.end(), n) != marks.end(); };

        for (int i = 0; i < numWhite; ++i)
        {
            const int note = (i / 7) * 12 + whiteSemis[i % 7];
            auto key = R (area.getX() + (float) i * w, area.getY(), w - 1.5f, area.getHeight());
            juce::Colour c (0xffe9ecf0);
            if (isMarked (note)) c = markColour.brighter (0.5f);
            if (note == playing) c = markColour;
            g.setGradientFill (juce::ColourGradient (c.darker (0.05f), 0.0f, key.getY(), c.darker (0.25f), 0.0f, key.getBottom(), false));
            g.fillRoundedRectangle (key, 3.0f);
            if (names)
                text (g, juce::String::fromUTF8 (whiteNames[i % 7]), key.removeFromBottom (22.0f),
                      juce::Colour (0xff20242a), juce::jmin (12.0f, w * 0.42f), isMarked (note));
        }
        static const int blackAfter[] = { 0, 1, 3, 4, 5 }; // después de Do, Re, Fa, Sol, La
        static const int blackSemis[] = { 1, 3, 6, 8, 10 };
        for (int o = 0; o < octaves; ++o)
            for (int b = 0; b < 5; ++b)
            {
                const int note = o * 12 + blackSemis[b];
                const float x = area.getX() + (float) (o * 7 + blackAfter[b] + 1) * w - w * 0.3f;
                auto key = R (x, area.getY(), w * 0.6f, area.getHeight() * 0.6f);
                juce::Colour c (0xff1c1f24);
                if (isMarked (note)) c = markColour.darker (0.2f);
                if (note == playing) c = markColour.brighter (0.3f);
                g.setGradientFill (juce::ColourGradient (c.brighter (0.15f), 0.0f, key.getY(), c.darker (0.3f), 0.0f, key.getBottom(), false));
                g.fillRoundedRectangle (key, 2.5f);
            }
    }

    // --------------------------------------------------------------------------
    //  Ilustraciones
    // --------------------------------------------------------------------------
    void ruta (juce::Graphics& g, R area, double t)
    {
        static const char* names[] = { "Primeros\npasos", "Ritmo y\nbatería", "Notas y\nmelodía", "Diseño\nde sonido",
                                       "Mezcla", "De beat a\ncanción", "Seguir\nmejorando" };
        const int n = 7;
        const float w = area.getWidth() / (float) n;
        const float active = (float) std::fmod (t * 0.8, (double) n + 1.0);
        juce::Path line;
        for (int i = 0; i < n; ++i)
        {
            const float x = area.getX() + w * ((float) i + 0.5f);
            const float y = area.getBottom() - 40.0f - (area.getHeight() - 80.0f) * (float) i / (float) (n - 1);
            if (i == 0) line.startNewSubPath (x, y); else line.lineTo (x, y);
        }
        g.setColour (Colours::outline.brighter (0.3f));
        g.strokePath (line, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        for (int i = 0; i < n; ++i)
        {
            const float x = area.getX() + w * ((float) i + 0.5f);
            const float y = area.getBottom() - 40.0f - (area.getHeight() - 80.0f) * (float) i / (float) (n - 1);
            auto dot = R (30.0f, 30.0f).withCentre ({ x, y });
            const bool lit = (float) i < active;
            if (lit) { Draw::glow (g, dot, Colours::accent, 8.0f, 15.0f); Draw::accentFill (g, dot, 15.0f); }
            else     { g.setColour (Colours::panelLight); g.fillEllipse (dot); }
            text (g, juce::String (i + 1), dot, lit ? juce::Colour (0xff1a1208) : Colours::textDim, 14.0f, true);
            text (g, juce::String::fromUTF8 (names[i]), R (x - w * 0.5f, y + 18.0f, w, 34.0f),
                  lit ? Colours::text : Colours::textDim, 11.5f, lit);
        }
    }

    void flujo (juce::Graphics& g, R area, double t)
    {
        static const char* names[] = { "Sonidos", "Patrones", "Playlist", "Mezclador", "Tu canción" };
        static const char* sub[] = { "batería, 808, sintes", "secuenciador y piano roll", "la canción en orden",
                                     "volumen y efectos", "WAV o MP3" };
        const juce::Colour cols[] = { Colours::accent, Colours::blue, Colours::purple, Colours::green, Colours::teal };
        const int n = 5;
        const float gap = 22.0f;
        const float w = (area.getWidth() - gap * (float) (n - 1)) / (float) n;
        auto row = area.withSizeKeepingCentre (area.getWidth(), juce::jmin (110.0f, area.getHeight()));
        for (int i = 0; i < n; ++i)
        {
            auto r = R (row.getX() + (float) i * (w + gap), row.getY(), w, row.getHeight());
            box (g, r, cols[i]);
            text (g, juce::String::fromUTF8 (names[i]), r.withTrimmedBottom (r.getHeight() * 0.45f), Colours::text, 15.0f, true);
            text (g, juce::String::fromUTF8 (sub[i]), r.withTrimmedTop (r.getHeight() * 0.5f).reduced (6.0f, 0.0f), Colours::textDim, 11.5f);
            if (i < n - 1)
                arrow (g, { r.getRight() + 3.0f, r.getCentreY() }, { r.getRight() + gap - 3.0f, r.getCentreY() }, Colours::textDim);
        }
        // Punto que viaja por el camino
        const float p = (float) std::fmod (t * 0.35, 1.0);
        const float x = row.getX() + p * row.getWidth();
        auto dot = R (12.0f, 12.0f).withCentre ({ x, row.getBottom() + 14.0f });
        Draw::glow (g, dot, Colours::accent, 8.0f, 6.0f);
        Draw::accentFill (g, dot, 6.0f);
    }

    void pulso (juce::Graphics& g, R area, double t)
    {
        const double bpm = 90.0;
        const double beats = t * bpm / 60.0;
        const int beat = (int) beats % 4;
        const float phase = (float) (beats - std::floor (beats));
        auto row = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), 480.0f), 110.0f).translated (0.0f, -12.0f);
        const float w = row.getWidth() / 4.0f;
        for (int i = 0; i < 4; ++i)
        {
            auto c = R (70.0f, 70.0f).withCentre ({ row.getX() + w * ((float) i + 0.5f), row.getCentreY() });
            if (i == beat)
            {
                const float size = 1.0f + (1.0f - phase) * 0.15f;
                c = c.withSizeKeepingCentre (70.0f * size, 70.0f * size);
                Draw::glow (g, c, Colours::accent, 6.0f + (1.0f - phase) * 12.0f, c.getWidth() * 0.5f);
                Draw::accentFill (g, c, c.getWidth() * 0.5f);
                text (g, juce::String (i + 1), c, juce::Colour (0xff1a1208), 28.0f, true);
            }
            else
            {
                g.setColour (Colours::panelLight);
                g.fillEllipse (c);
                text (g, juce::String (i + 1), c, Colours::textDim, 24.0f, true);
            }
        }
        text (g, juce::String::fromUTF8 ("90 BPM = 90 pulsos en un minuto (1,5 por segundo)"),
              R (area.getX(), area.getBottom() - 30.0f, area.getWidth(), 24.0f), Colours::textDim, 13.0f);
    }

    void pasos (juce::Graphics& g, R area, double t)
    {
        auto labels = area.removeFromTop (22.0f);
        const float w = area.getWidth() / 4.0f;
        for (int i = 0; i < 4; ++i)
        {
            auto r = R (labels.getX() + (float) i * w, labels.getY(), w, labels.getHeight());
            text (g, "Tiempo " + juce::String (i + 1), r, Colours::accentLight, 13.0f, true);
        }
        drumGrid (g, area, { { "Pasos", Colours::accent, "x...x...x...x..." } }, 80.0, t);
    }

    void swing (juce::Graphics& g, R area, double t)
    {
        const float pos = stepPosition (t, 90.0, 8);
        for (int half = 0; half < 2; ++half)
        {
            auto r = area.removeFromTop (area.getHeight() / (half == 0 ? 2.0f : 1.0f)).reduced (0.0f, 8.0f);
            auto label = r.removeFromLeft (110.0f);
            text (g, half == 0 ? "Recto" : "Con swing", label, half == 0 ? Colours::blue : Colours::accent, 14.0f, true,
                  juce::Justification::centredLeft);
            g.setColour (Colours::outline);
            g.drawHorizontalLine ((int) r.getCentreY(), r.getX(), r.getRight());
            const float stepW = r.getWidth() / 8.0f;
            for (int i = 0; i < 8; ++i)
            {
                g.setColour (Colours::outline.brighter (0.3f));
                g.drawVerticalLine ((int) (r.getX() + (float) i * stepW), r.getY(), r.getBottom());
                const float offset = (half == 1 && i % 2 == 1) ? stepW * 0.33f : 0.0f;
                auto dot = R (18.0f, 18.0f).withCentre ({ r.getX() + (float) i * stepW + stepW * 0.5f - stepW * 0.5f + offset + 9.0f, r.getCentreY() });
                const bool hit = (int) pos == i;
                const juce::Colour c = half == 0 ? Colours::blue : Colours::accent;
                if (hit) Draw::glow (g, dot, c, 10.0f, 9.0f);
                Draw::colourFill (g, dot, hit ? c.brighter (0.4f) : c, 9.0f);
            }
        }
    }

    void velocidad (juce::Graphics& g, R area, double t)
    {
        static const float vel[] = { 1.0f, 0.45f, 0.75f, 0.4f, 0.95f, 0.5f, 0.7f, 0.35f, 1.0f, 0.45f, 0.8f, 0.4f, 0.9f, 0.55f, 0.7f, 0.5f };
        auto footer = area.removeFromBottom (24.0f);
        text (g, juce::String::fromUTF8 ("Hi-hats con distinta fuerza: suenan más humanos que si todos golpean igual"),
              footer, Colours::textDim, 12.5f);
        const float w = area.getWidth() / 16.0f;
        const int current = (int) stepPosition (t, 100.0);
        for (int i = 0; i < 16; ++i)
        {
            const float h = (area.getHeight() - 10.0f) * vel[i];
            auto bar = R (area.getX() + (float) i * w + 3.0f, area.getBottom() - h, w - 6.0f, h);
            const bool hit = i == current;
            if (hit) Draw::glow (g, bar, Colours::accent, 10.0f, 4.0f);
            Draw::colourFill (g, bar, Colours::accent.withMultipliedBrightness (0.55f + vel[i] * 0.45f).brighter (hit ? 0.4f : 0.0f), 4.0f);
        }
    }

    void teclado (juce::Graphics& g, R area, double t)
    {
        static const int scale[] = { 0, 2, 4, 5, 7, 9, 11, 12 };
        const int i = (int) std::fmod (t * 2.0, 10.0);
        const int playing = i < 8 ? scale[i] : -1;
        keyboard (g, area.reduced (0.0f, 4.0f), 2, {}, Colours::accent, true, playing);
    }

    void escalas (juce::Graphics& g, R area, double t)
    {
        const bool minor = std::fmod (t, 8.0) >= 4.0;
        auto title = area.removeFromTop (28.0f);
        text (g, minor ? "La menor: La Si Do Re Mi Fa Sol (triste)" : "Do mayor: Do Re Mi Fa Sol La Si (alegre)",
              title, minor ? Colours::purple.brighter (0.3f) : Colours::accentLight, 15.0f, true);
        const std::vector<int> marks = minor ? std::vector<int> { 9, 11, 12, 14, 16, 17, 19, 21 }
                                             : std::vector<int> { 0, 2, 4, 5, 7, 9, 11, 12 };
        static const int majorSeq[] = { 0, 2, 4, 5, 7, 9, 11, 12 };
        static const int minorSeq[] = { 9, 11, 12, 14, 16, 17, 19, 21 };
        const int idx = (int) std::fmod (t * 2.0, 8.0);
        keyboard (g, area.reduced (0.0f, 4.0f), 2, marks, minor ? Colours::purple : Colours::accent, true,
                  minor ? minorSeq[idx] : majorSeq[idx]);
    }

    void acordes (juce::Graphics& g, R area, double t)
    {
        struct Chord { const char* name; int notes[3]; juce::Colour colour; };
        const Chord chords[] = { { "La menor", { 9, 12, 16 }, Colours::purple },
                                 { "Fa mayor", { 5, 9, 12 }, Colours::blue },
                                 { "Do mayor", { 0, 4, 7 }, Colours::accent },
                                 { "Sol mayor", { 7, 11, 14 }, Colours::green } };
        const int c = (int) std::fmod (t * 0.7, 4.0);
        auto top = area.removeFromTop (34.0f);
        const float w = top.getWidth() / 4.0f;
        for (int i = 0; i < 4; ++i)
        {
            auto r = R (top.getX() + (float) i * w, top.getY(), w, top.getHeight()).reduced (4.0f, 2.0f);
            if (i == c) box (g, r, chords[i].colour, 8.0f, 0.3f);
            text (g, juce::String::fromUTF8 (chords[i].name), r, i == c ? Colours::text : Colours::textDim, 14.0f, i == c);
            if (i < 3) text (g, juce::String::fromUTF8 ("→"), R (r.getRight() - 2.0f, r.getY(), 12.0f, r.getHeight()), Colours::textDim, 13.0f);
        }
        area.removeFromTop (6.0f);
        keyboard (g, area, 2, { chords[c].notes[0], chords[c].notes[1], chords[c].notes[2] }, chords[c].colour, true);
    }

    // Mini piano roll con notas (inicio, duración, fila desde abajo)
    void miniRoll (juce::Graphics& g, R area, double t, const std::vector<std::array<float, 3>>& notes, int rows,
                   juce::Colour colour, int steps = 32)
    {
        g.setColour (juce::Colour (0xff181b20));
        g.fillRoundedRectangle (area, 6.0f);
        const float rowH = area.getHeight() / (float) rows;
        const float stepW = area.getWidth() / (float) steps;
        for (int r = 0; r < rows; ++r)
        {
            g.setColour (juce::Colours::black.withAlpha (0.3f));
            g.drawHorizontalLine ((int) (area.getY() + (float) r * rowH), area.getX(), area.getRight());
        }
        for (int s = 0; s <= steps; s += 4)
        {
            g.setColour (s % 16 == 0 ? Colours::outline.brighter (0.5f) : Colours::outline.withAlpha (0.6f));
            g.drawVerticalLine ((int) (area.getX() + (float) s * stepW), area.getY(), area.getBottom());
        }
        const float pos = stepPosition (t, 100.0, steps);
        for (auto& n : notes)
        {
            auto r = R (area.getX() + n[0] * stepW + 1.0f, area.getBottom() - (n[2] + 1.0f) * rowH + 1.0f,
                        n[1] * stepW - 2.0f, rowH - 2.0f);
            const bool on = pos >= n[0] && pos < n[0] + n[1];
            if (on) Draw::glow (g, r, colour, 8.0f, 3.0f);
            Draw::colourFill (g, r, on ? colour.brighter (0.4f) : colour, 3.0f);
        }
        const float x = area.getX() + pos * stepW;
        g.setColour (Colours::green);
        g.fillRect (x - 1.0f, area.getY(), 2.0f, area.getHeight());
    }

    void pianoroll (juce::Graphics& g, R area, double t)
    {
        auto keys = area.removeFromLeft (46.0f);
        // Teclas verticales sencillas
        const int rows = 12;
        const float rowH = keys.getHeight() / (float) rows;
        for (int r = 0; r < rows; ++r)
        {
            const int semi = (11 - r);
            const bool black = semi == 1 || semi == 3 || semi == 6 || semi == 8 || semi == 10;
            g.setColour (black ? juce::Colour (0xff1c1f24) : juce::Colour (0xffe1e4e8));
            g.fillRect (keys.getX(), keys.getY() + (float) r * rowH + 0.5f, keys.getWidth() - 4.0f, rowH - 1.0f);
        }
        miniRoll (g, area, t, { { 0, 4, 9 }, { 4, 4, 0 }, { 8, 4, 4 }, { 12, 4, 2 },
                                { 16, 2, 9 }, { 18, 2, 7 }, { 20, 6, 4 }, { 28, 4, 0 } }, rows, Colours::blue);
    }

    void melodia (juce::Graphics& g, R area, double t)
    {
        auto labels = area.removeFromTop (22.0f);
        text (g, "Pregunta", labels.withWidth (labels.getWidth() * 0.5f), Colours::blue.brighter (0.3f), 13.0f, true);
        text (g, "Respuesta", labels.withTrimmedLeft (labels.getWidth() * 0.5f), Colours::accentLight, 13.0f, true);
        miniRoll (g, area, t, { { 0, 2, 4 }, { 2, 2, 6 }, { 4, 4, 7 }, { 10, 4, 6 },
                                { 16, 2, 4 }, { 18, 2, 6 }, { 20, 4, 7 }, { 26, 6, 2 } }, 10, Colours::accent);
    }

    void bass808 (juce::Graphics& g, R area, double t)
    {
        auto wave = area.removeFromTop (area.getHeight() * 0.62f).reduced (0.0f, 6.0f);
        const float sweep = (float) std::fmod (t * 0.3, 1.0);
        auto p = wavePath (wave, 600, [] (float x)
        {
            const float env = x < 0.02f ? x / 0.02f : std::exp (-(x - 0.02f) * 2.2f);
            const float freq = x < 0.62f ? 14.0f : 14.0f + (x - 0.62f) * 40.0f;
            return env * std::sin (2.0f * pi * freq * x * (1.0f + 0.4f * (x < 0.05f ? (0.05f - x) * 20.0f : 0.0f)));
        });
        g.setColour (Colours::purple.withAlpha (0.25f));
        g.strokePath (p, juce::PathStrokeType (5.0f));
        g.setColour (Colours::purple.brighter (0.3f));
        g.strokePath (p, juce::PathStrokeType (1.6f));
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.fillRect (wave.getX() + sweep * wave.getWidth(), wave.getY(), 2.0f, wave.getHeight());
        text (g, "Golpe", R (wave.getX(), wave.getY() - 4.0f, 80.0f, 18.0f), Colours::text, 12.0f, true, juce::Justification::centredLeft);
        text (g, "Cola grave que se apaga", R (wave.getX() + wave.getWidth() * 0.18f, wave.getY() - 4.0f, 220.0f, 18.0f), Colours::textDim, 12.0f);
        text (g, "Glide: la nota sube", R (wave.getX() + wave.getWidth() * 0.66f, wave.getY() - 4.0f, 180.0f, 18.0f), Colours::accentLight, 12.0f, true);

        // Notas del piano roll debajo
        area.removeFromTop (6.0f);
        auto roll = area;
        g.setColour (juce::Colour (0xff181b20));
        g.fillRoundedRectangle (roll, 6.0f);
        auto n1 = R (roll.getX() + 4.0f, roll.getBottom() - roll.getHeight() * 0.45f, roll.getWidth() * 0.66f, roll.getHeight() * 0.3f);
        auto n2 = R (roll.getX() + roll.getWidth() * 0.6f, roll.getY() + roll.getHeight() * 0.15f, roll.getWidth() * 0.38f, roll.getHeight() * 0.3f);
        Draw::colourFill (g, n1, Colours::purple, 4.0f);
        Draw::colourFill (g, n2, Colours::purple.brighter (0.2f), 4.0f);
        text (g, "La1", n1.reduced (8.0f, 0.0f), juce::Colour (0xff1a1022), 11.0f, true, juce::Justification::centredLeft);
        text (g, "Do2", n2.reduced (8.0f, 0.0f), juce::Colour (0xff1a1022), 11.0f, true, juce::Justification::centredLeft);
        text (g, "Las notas se solapan = glide", R (roll.getX() + 8.0f, n2.getY(), n2.getX() - roll.getX() - 20.0f, n2.getHeight()),
              Colours::textDim, 11.5f, false, juce::Justification::centredRight);
    }

    void sampler (juce::Graphics& g, R area, double t)
    {
        const int sel = (int) std::fmod (t * 0.8, 3.0);
        const char* names[] = { "Más grave (más lento)", "Original", "Más agudo (más rápido)" };
        const float speeds[] = { 5.0f, 8.0f, 13.0f };
        const juce::Colour cols[] = { Colours::purple, Colours::accent, Colours::blue };
        const float h = area.getHeight() / 3.0f;
        for (int i = 0; i < 3; ++i)
        {
            auto r = R (area.getX(), area.getY() + (float) i * h, area.getWidth(), h).reduced (0.0f, 5.0f);
            auto label = r.removeFromLeft (190.0f);
            text (g, juce::String::fromUTF8 (names[i]), label, i == sel ? Colours::text : Colours::textDim, 13.0f, i == sel,
                  juce::Justification::centredLeft);
            const float sp = speeds[i];
            auto p = wavePath (r, 300, [sp] (float x) { return std::exp (-x * 3.0f) * std::sin (2.0f * pi * sp * x) * 0.9f; });
            g.setColour (i == sel ? cols[i] : cols[i].withAlpha (0.4f));
            g.strokePath (p, juce::PathStrokeType (i == sel ? 2.2f : 1.4f));
        }
    }

    void ondas (juce::Graphics& g, R area, double t)
    {
        const char* names[] = { "Seno", "Triángulo", "Cuadrada", "Sierra" };
        const char* desc[] = { "suave", "dulce", "hueca", "brillante" };
        const juce::Colour cols[] = { Colours::teal, Colours::green, Colours::blue, Colours::accent };
        const float w = area.getWidth() / 4.0f;
        const float shift = (float) std::fmod (t * 0.5, 1.0);
        for (int i = 0; i < 4; ++i)
        {
            auto r = R (area.getX() + (float) i * w, area.getY(), w, area.getHeight()).reduced (6.0f, 0.0f);
            box (g, r, cols[i], 8.0f, 0.08f);
            auto label = r.removeFromBottom (40.0f);
            text (g, juce::String::fromUTF8 (names[i]), label.removeFromTop (20.0f), Colours::text, 14.0f, true);
            text (g, juce::String::fromUTF8 (desc[i]), label, Colours::textDim, 12.0f);
            auto wr = r.reduced (10.0f, 18.0f);
            auto p = wavePath (wr, 200, [i, shift] (float x)
            {
                const float ph = std::fmod (x * 2.0f + shift, 1.0f);
                switch (i)
                {
                    case 0: return std::sin (2.0f * pi * ph);
                    case 1: return ph < 0.5f ? ph * 4.0f - 1.0f : 3.0f - ph * 4.0f;
                    case 2: return ph < 0.5f ? 0.85f : -0.85f;
                    default: return ph * 2.0f - 1.0f;
                }
            });
            g.setColour (cols[i]);
            g.strokePath (p, juce::PathStrokeType (2.2f));
        }
    }

    void adsr (juce::Graphics& g, R area, double t)
    {
        auto r = area.reduced (10.0f, 20.0f).withTrimmedBottom (10.0f);
        const float a = 0.15f, d = 0.15f, s = 0.55f, sustainLen = 0.4f, rel = 0.3f;
        const float total = a + d + sustainLen + rel;
        auto px = [&] (float time) { return r.getX() + time / total * r.getWidth(); };
        auto py = [&] (float level) { return r.getBottom() - level * r.getHeight(); };
        juce::Path p;
        p.startNewSubPath (px (0), py (0));
        p.lineTo (px (a), py (1));
        p.quadraticTo (px (a + d * 0.3f), py (s), px (a + d), py (s));
        p.lineTo (px (a + d + sustainLen), py (s));
        p.quadraticTo (px (a + d + sustainLen + rel * 0.2f), py (0), px (total), py (0));
        juce::Path fill (p);
        fill.lineTo (px (total), py (0));
        fill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (Colours::blue.withAlpha (0.35f), 0.0f, r.getY(), Colours::blue.withAlpha (0.0f), 0.0f, r.getBottom(), false));
        g.fillPath (fill);
        g.setColour (Colours::blue);
        g.strokePath (p, juce::PathStrokeType (2.5f));

        const char* names[] = { "Ataque", "Caída", "Sostenido", "Relajación" };
        const float starts[] = { 0, a, a + d, a + d + sustainLen, total };
        for (int i = 0; i < 4; ++i)
        {
            g.setColour (Colours::outline.brighter (0.3f));
            g.drawVerticalLine ((int) px (starts[i + 1]), r.getY(), r.getBottom());
            text (g, names[i], R (px (starts[i]), area.getY(), px (starts[i + 1]) - px (starts[i]), 18.0f), Colours::accentLight, 12.5f, true);
        }
        // Barra de la nota pulsada
        text (g, "Nota pulsada", R (px (0), r.getBottom() + 4.0f, px (a + d + sustainLen) - px (0), 18.0f), Colours::textDim, 11.5f);
        g.setColour (Colours::accent.withAlpha (0.5f));
        g.fillRect (px (0), r.getBottom() + 2.0f, px (a + d + sustainLen) - px (0), 2.0f);

        // Punto animado
        const float time = (float) std::fmod (t * 0.45, 1.25) * total;
        if (time <= total)
        {
            float level = 0.0f;
            if (time < a) level = time / a;
            else if (time < a + d) level = 1.0f - (1.0f - s) * (time - a) / d;
            else if (time < a + d + sustainLen) level = s;
            else level = s * (1.0f - (time - a - d - sustainLen) / rel);
            auto dot = R (14.0f, 14.0f).withCentre ({ px (time), py (juce::jmax (0.0f, level)) });
            Draw::glow (g, dot, Colours::accent, 8.0f, 7.0f);
            Draw::accentFill (g, dot, 7.0f);
        }
    }

    void filtro (juce::Graphics& g, R area, double t)
    {
        auto r = area.reduced (6.0f, 6.0f).withTrimmedBottom (18.0f);
        const float cutoff = 0.35f + 0.3f * std::sin ((float) t * 1.3f);
        const int bars = 48;
        const float w = r.getWidth() / (float) bars;
        juce::Random rnd (7);
        for (int i = 0; i < bars; ++i)
        {
            const float x = (float) i / (float) bars;
            const float level = (0.5f + 0.5f * rnd.nextFloat()) * (1.0f - x * 0.4f);
            const float gain = x < cutoff ? 1.0f : std::exp (-(x - cutoff) * 9.0f);
            const float h = r.getHeight() * 0.72f * level * gain;
            Draw::colourFill (g, R (r.getX() + (float) i * w + 1.0f, r.getBottom() - h, w - 2.0f, h), Colours::blue, 2.0f);
        }
        juce::Path curve;
        for (int i = 0; i <= 200; ++i)
        {
            const float x = (float) i / 200.0f;
            float gain = x < cutoff ? 1.0f : std::exp (-(x - cutoff) * 9.0f);
            gain += 0.25f * std::exp (-std::pow ((x - cutoff) * 25.0f, 2.0f)); // resonancia
            const float y = r.getBottom() - gain * r.getHeight() * 0.74f;
            if (i == 0) curve.startNewSubPath (r.getX(), y); else curve.lineTo (r.getX() + x * r.getWidth(), y);
        }
        g.setColour (Colours::accent);
        g.strokePath (curve, juce::PathStrokeType (2.5f));
        text (g, "Graves", R (r.getX(), r.getBottom() + 2.0f, 80.0f, 16.0f), Colours::textDim, 11.5f, false, juce::Justification::centredLeft);
        text (g, "Agudos", R (r.getRight() - 80.0f, r.getBottom() + 2.0f, 80.0f, 16.0f), Colours::textDim, 11.5f, false, juce::Justification::centredRight);
        text (g, "Corte", R (r.getX() + cutoff * r.getWidth() - 40.0f, r.getY() + 2.0f, 80.0f, 16.0f), Colours::accentLight, 12.0f, true);
    }

    void estructura (juce::Graphics& g, R area, double t)
    {
        struct Sec { const char* name; int bars; juce::Colour colour; float energy; };
        const Sec secs[] = { { "Intro", 4, Colours::teal, 0.35f }, { "Estribillo", 8, Colours::accent, 1.0f },
                             { "Estrofa", 8, Colours::blue, 0.6f }, { "Estribillo", 8, Colours::accent, 1.0f },
                             { "Estrofa", 8, Colours::blue, 0.6f }, { "Estribillo", 8, Colours::accent, 1.0f },
                             { "Final", 4, Colours::purple, 0.3f } };
        int total = 0;
        for (auto& s : secs) total += s.bars;
        auto label = area.removeFromBottom (22.0f);
        text (g, juce::String::fromUTF8 ("La altura de cada bloque es la energía: el estribillo es la parte más llena"),
              label, Colours::textDim, 12.0f);
        const float pxPerBar = area.getWidth() / (float) total;
        float x = area.getX();
        for (auto& s : secs)
        {
            const float w = (float) s.bars * pxPerBar;
            const float h = area.getHeight() * (0.35f + 0.65f * s.energy);
            auto r = R (x + 2.0f, area.getBottom() - h, w - 4.0f, h);
            box (g, r, s.colour, 8.0f, 0.22f);
            text (g, juce::String::fromUTF8 (s.name), r.removeFromTop (24.0f), Colours::text, 12.5f, true);
            text (g, juce::String (s.bars) + juce::String::fromUTF8 (" compases"), r.removeFromTop (16.0f), Colours::textDim, 10.5f);
            x += w;
        }
        const float pos = (float) std::fmod (t * 4.0, (double) total);
        g.setColour (Colours::green);
        g.fillRect (area.getX() + pos * pxPerBar - 1.0f, area.getY(), 2.0f, area.getHeight());
    }

    void volumenes (juce::Graphics& g, R area, double t)
    {
        struct Item { const char* name; float level; juce::Colour colour; };
        const Item items[] = { { "Bombo", 0.85f, Colours::accent }, { "808", 0.82f, Colours::purple },
                               { "Clap", 0.7f, Colours::accent }, { "Melodía", 0.55f, Colours::blue },
                               { "Hi-hat", 0.45f, Colours::accent }, { "Percusión", 0.35f, Colours::accent },
                               { "MASTER", 0.78f, Colours::green } };
        const int n = 7;
        const float w = area.getWidth() / (float) n;
        auto labels = area.removeFromBottom (22.0f);
        for (int i = 0; i < n; ++i)
        {
            auto col = R (area.getX() + (float) i * w, area.getY(), w, area.getHeight()).reduced (w * 0.28f, 4.0f);
            g.setColour (juce::Colour (0xff07090b));
            g.fillRoundedRectangle (col, 3.0f);
            const float wobble = 0.06f * std::sin ((float) t * 7.0f + (float) i * 1.7f);
            const float level = juce::jlimit (0.0f, 1.0f, items[i].level + wobble);
            const int segs = 16;
            const float segH = col.getHeight() / (float) segs;
            for (int sIdx = 0; sIdx < segs; ++sIdx)
            {
                const float pos = (float) sIdx / (float) segs;
                const juce::Colour c = pos > 0.9f ? Colours::red : pos > 0.72f ? juce::Colour (0xffffc23d) : Colours::green;
                g.setColour (pos < level ? c : c.withAlpha (0.08f));
                g.fillRect (col.getX() + 2.0f, col.getBottom() - (float) (sIdx + 1) * segH + 1.0f, col.getWidth() - 4.0f, segH - 2.0f);
            }
            text (g, juce::String::fromUTF8 (items[i].name), R (col.getX() - w * 0.3f, labels.getY(), w, labels.getHeight()),
                  i == n - 1 ? Colours::green : items[i].colour.brighter (0.2f), 11.5f, true);
        }
        // Línea roja: no pasar
        const float redY = area.getY() + 4.0f + (area.getHeight() - 8.0f) * 0.1f;
        g.setColour (Colours::red.withAlpha (0.7f));
        const float dashes[] = { 6.0f, 4.0f };
        g.drawDashedLine ({ area.getX(), redY, area.getRight(), redY }, dashes, 2, 1.2f);
        text (g, "Zona roja: no llegar", R (area.getRight() - 160.0f, redY - 18.0f, 160.0f, 16.0f), Colours::red, 11.0f, true,
              juce::Justification::centredRight);
    }

    void panorama (juce::Graphics& g, R area, double t)
    {
        auto centre = juce::Point<float> (area.getCentreX(), area.getBottom() - 10.0f);
        const float radius = juce::jmin (area.getWidth() * 0.45f, area.getHeight() - 40.0f);
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, -pi * 0.5f, pi * 0.5f, true);
        g.setColour (Colours::outline.brighter (0.3f));
        g.strokePath (arc, juce::PathStrokeType (1.5f));
        text (g, "IZQUIERDA", R (centre.x - radius - 50.0f, centre.y - 20.0f, 100.0f, 16.0f), Colours::textDim, 11.0f, true);
        text (g, "DERECHA", R (centre.x + radius - 50.0f, centre.y - 20.0f, 100.0f, 16.0f), Colours::textDim, 11.0f, true);
        text (g, "CENTRO", R (centre.x - 50.0f, centre.y - radius - 22.0f, 100.0f, 16.0f), Colours::textDim, 11.0f, true);
        // Oyente
        auto head = R (34.0f, 34.0f).withCentre (centre.translated (0.0f, -6.0f));
        g.setColour (Colours::panelLight.brighter (0.2f));
        g.fillEllipse (head);
        text (g, "Tú", head, Colours::text, 11.0f, true);

        struct Src { const char* name; float pan; float dist; juce::Colour colour; };
        const Src srcs[] = { { "Bombo", 0.0f, 0.35f, Colours::accent }, { "808", 0.0f, 0.55f, Colours::purple },
                             { "Voz", 0.0f, 0.8f, Colours::teal }, { "Hi-hat", 0.35f, 0.7f, Colours::accent },
                             { "Percusión", -0.4f, 0.7f, Colours::accent }, { "Coros", -0.75f, 0.85f, Colours::teal },
                             { "Efectos", 0.8f, 0.9f, Colours::blue } };
        for (int i = 0; i < 7; ++i)
        {
            const auto& s = srcs[i];
            const float wob = 0.03f * std::sin ((float) t * 2.0f + (float) i);
            const float angle = (s.pan + wob) * pi * 0.5f;
            auto p = centre.translated (std::sin (angle) * radius * s.dist, -std::cos (angle) * radius * s.dist);
            auto dot = R (16.0f, 16.0f).withCentre (p);
            Draw::glow (g, dot, s.colour, 6.0f, 8.0f);
            Draw::colourFill (g, dot, s.colour, 8.0f);
            text (g, juce::String::fromUTF8 (s.name), R (p.x - 50.0f, p.y + 9.0f, 100.0f, 16.0f), Colours::text, 11.0f, true);
        }
    }

    void eq (juce::Graphics& g, R area, double t)
    {
        auto axis = area.removeFromBottom (20.0f);
        auto r = area.reduced (0.0f, 4.0f);
        // Zonas
        const juce::Colour zoneCols[] = { Colours::purple, Colours::blue, Colours::teal };
        const char* zones[] = { "GRAVES", "MEDIOS", "AGUDOS" };
        const float bounds[] = { 0.0f, 0.36f, 0.75f, 1.0f };
        for (int i = 0; i < 3; ++i)
        {
            auto z = R (r.getX() + bounds[i] * r.getWidth(), r.getY(), (bounds[i + 1] - bounds[i]) * r.getWidth(), r.getHeight());
            g.setColour (zoneCols[i].withAlpha (0.08f));
            g.fillRect (z);
            text (g, zones[i], z.removeFromTop (20.0f), zoneCols[i].brighter (0.3f), 12.0f, true);
        }
        // Instrumentos
        struct Inst { const char* name; float from, to; juce::Colour colour; };
        const Inst insts[] = { { "808", 0.02f, 0.3f, Colours::purple }, { "Bombo", 0.06f, 0.42f, Colours::accent },
                               { "Caja / clap", 0.3f, 0.7f, Colours::accent }, { "Voz", 0.33f, 0.82f, Colours::teal },
                               { "Hi-hat", 0.7f, 0.98f, Colours::blue } };
        const float rowH = (r.getHeight() - 26.0f) / 5.0f;
        for (int i = 0; i < 5; ++i)
        {
            auto bar = R (r.getX() + insts[i].from * r.getWidth(), r.getY() + 26.0f + (float) i * rowH + 3.0f,
                          (insts[i].to - insts[i].from) * r.getWidth(), rowH - 6.0f);
            const float pulse = 0.5f + 0.5f * std::sin ((float) t * 3.0f + (float) i);
            Draw::glow (g, bar, insts[i].colour, 2.0f + pulse * 4.0f, 5.0f);
            Draw::colourFill (g, bar, insts[i].colour, 5.0f);
            text (g, juce::String::fromUTF8 (insts[i].name), bar, juce::Colour (0xff15171b), 11.5f, true);
        }
        const char* freqs[] = { "20 Hz", "100", "250", "1000", "4000", "10 000", "20 000 Hz" };
        const float fpos[] = { 0.0f, 0.23f, 0.36f, 0.56f, 0.75f, 0.88f, 1.0f };
        for (int i = 0; i < 7; ++i)
            text (g, freqs[i], R (r.getX() + fpos[i] * r.getWidth() - 35.0f, axis.getY(), 70.0f, axis.getHeight()),
                  Colours::textDim, 10.5f);
    }

    void compresor (juce::Graphics& g, R area, double t)
    {
        const float shift = (float) t * 0.25f;
        auto signal = [shift] (float x)
        {
            const float env = 0.35f + 0.6f * std::pow (std::abs (std::sin ((x + shift) * pi * 3.0f)), 6.0f)
                              + 0.15f * std::sin ((x + shift) * 17.0f);
            return juce::jlimit (0.05f, 1.0f, env);
        };
        for (int half = 0; half < 2; ++half)
        {
            auto r = R (area.getX(), area.getY() + (float) half * area.getHeight() * 0.5f, area.getWidth(), area.getHeight() * 0.5f).reduced (0.0f, 6.0f);
            auto label = r.removeFromLeft (120.0f);
            text (g, half == 0 ? "Sin compresor" : "Con compresor", label, half == 0 ? Colours::textDim : Colours::accentLight, 13.0f, true,
                  juce::Justification::centredLeft);
            const float thr = 0.55f;
            juce::Path p;
            const int n = 160;
            for (int i = 0; i < n; ++i)
            {
                const float x = (float) i / (float) n;
                float env = signal (x);
                if (half == 1 && env > thr) env = thr + (env - thr) / 4.0f;
                const float h = env * r.getHeight() * 0.5f;
                p.addRectangle (r.getX() + x * r.getWidth(), r.getCentreY() - h, r.getWidth() / (float) n - 1.0f, h * 2.0f);
            }
            g.setColour (half == 0 ? Colours::blue.withAlpha (0.8f) : Colours::accent);
            g.fillPath (p);
            const float dashes[] = { 5.0f, 4.0f };
            g.setColour (Colours::red.withAlpha (0.8f));
            const float ty = r.getCentreY() - thr * r.getHeight() * 0.5f;
            g.drawDashedLine ({ r.getX(), ty, r.getRight(), ty }, dashes, 2, 1.2f);
            if (half == 0)
                text (g, "Umbral", R (r.getRight() - 70.0f, ty - 16.0f, 70.0f, 14.0f), Colours::red, 11.0f, true, juce::Justification::centredRight);
        }
    }

    void reverb (juce::Graphics& g, R area, double t)
    {
        const bool big = std::fmod (t, 6.0) >= 3.0;
        auto title = area.removeFromTop (24.0f);
        text (g, big ? "Sala grande: cola larga" : "Habitación pequeña: cola corta", title, Colours::accentLight, 14.0f, true);
        auto r = area.reduced (0.0f, 6.0f);
        const float decay = big ? 2.2f : 7.0f;
        const float sweep = (float) std::fmod (t / 3.0, 1.0);
        juce::Random rnd (3);
        const int n = 180;
        for (int i = 0; i < n; ++i)
        {
            const float x = (float) i / (float) n;
            float h;
            juce::Colour c = Colours::blue;
            if (i < 3) { h = 1.0f; c = Colours::accent; }
            else h = std::exp (-x * decay) * (0.3f + 0.7f * rnd.nextFloat()) * 0.75f;
            if (x > sweep) c = c.withAlpha (0.25f);
            const float bh = h * r.getHeight() * 0.5f;
            g.setColour (c);
            g.fillRect (r.getX() + x * r.getWidth(), r.getCentreY() - bh, r.getWidth() / (float) n - 1.0f, bh * 2.0f);
        }
        text (g, "Golpe original", R (r.getX() + 16.0f, r.getY(), 120.0f, 16.0f), Colours::accent, 11.5f, true, juce::Justification::centredLeft);
        text (g, "Rebotes en las paredes (la cola)", R (r.getX() + r.getWidth() * 0.2f, r.getY(), 260.0f, 16.0f), Colours::blue.brighter (0.3f), 11.5f);
    }

    void delay (juce::Graphics& g, R area, double t)
    {
        auto top = area.removeFromTop (area.getHeight() * 0.5f).reduced (0.0f, 4.0f);
        auto bottom = area.reduced (0.0f, 4.0f);
        text (g, "Izq.", top.removeFromLeft (44.0f), Colours::textDim, 12.0f, true);
        text (g, "Der.", bottom.removeFromLeft (44.0f), Colours::textDim, 12.0f, true);
        const float cycle = 2.4f;
        const float now = (float) std::fmod (t, (double) cycle) / cycle;
        const int repeats = 6;
        for (int i = 0; i <= repeats; ++i)
        {
            const float x = (float) i / (float) (repeats + 1);
            auto& lane = (i % 2 == 0) ? top : bottom;
            const float amp = std::pow (0.62f, (float) i);
            const float h = lane.getHeight() * amp;
            auto bar = R (lane.getX() + x * lane.getWidth(), lane.getCentreY() - h * 0.5f, 18.0f, h);
            const bool lit = now >= x;
            const juce::Colour c = i == 0 ? Colours::accent : Colours::teal;
            if (lit && now - x < 0.08f) Draw::glow (g, bar, c, 10.0f, 4.0f);
            Draw::colourFill (g, bar, lit ? c : c.withAlpha (0.2f), 4.0f);
        }
    }

    void automatizacion (juce::Graphics& g, R area, double t)
    {
        auto knobArea = area.removeFromRight (120.0f);
        auto r = area.reduced (4.0f, 8.0f);
        g.setColour (juce::Colour (0xff181b20));
        g.fillRoundedRectangle (r, 6.0f);
        const juce::Point<float> pts[] = { { 0.0f, 0.2f }, { 0.3f, 0.25f }, { 0.55f, 0.85f }, { 0.75f, 0.7f }, { 1.0f, 0.1f } };
        auto map = [&] (juce::Point<float> p) { return juce::Point<float> (r.getX() + p.x * r.getWidth(), r.getBottom() - p.y * r.getHeight()); };
        juce::Path curve, fill;
        curve.startNewSubPath (map (pts[0]));
        for (int i = 1; i < 5; ++i) curve.lineTo (map (pts[i]));
        fill = curve;
        fill.lineTo (r.getBottomRight());
        fill.lineTo (r.getBottomLeft());
        fill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.3f), 0.0f, r.getY(), Colours::accent.withAlpha (0.0f), 0.0f, r.getBottom(), false));
        g.fillPath (fill);
        g.setColour (Colours::accent);
        g.strokePath (curve, juce::PathStrokeType (2.2f));
        for (auto p : pts)
        {
            g.setColour (Colours::accentLight);
            g.fillEllipse (R (9.0f, 9.0f).withCentre (map (p)));
        }
        const float x = (float) std::fmod (t * 0.2, 1.0);
        float v = 0.0f;
        for (int i = 0; i < 4; ++i)
            if (x >= pts[i].x && x <= pts[i + 1].x)
                v = pts[i].y + (pts[i + 1].y - pts[i].y) * (x - pts[i].x) / (pts[i + 1].x - pts[i].x);
        g.setColour (Colours::green);
        g.fillRect (r.getX() + x * r.getWidth() - 1.0f, r.getY(), 2.0f, r.getHeight());

        // Rueda que se mueve sola
        auto knob = knobArea.withSizeKeepingCentre (80.0f, 80.0f);
        const float start = -pi * 0.75f, end = pi * 0.75f;
        juce::Path track, value;
        track.addCentredArc (knob.getCentreX(), knob.getCentreY(), 36.0f, 36.0f, 0.0f, start, end, true);
        value.addCentredArc (knob.getCentreX(), knob.getCentreY(), 36.0f, 36.0f, 0.0f, start, start + v * (end - start), true);
        g.setColour (juce::Colour (0xff0a0c0f));
        g.strokePath (track, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (Colours::accent);
        g.strokePath (value, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xff3e454f));
        g.fillEllipse (knob.reduced (12.0f));
        text (g, juce::String (juce::roundToInt (v * 100.0f)) + " %", knob.reduced (12.0f), Colours::text, 13.0f, true);
        text (g, "Filtro", R (knob.getX() - 10.0f, knob.getBottom() + 2.0f, knob.getWidth() + 20.0f, 16.0f), Colours::textDim, 12.0f);
    }

    void grabar (juce::Graphics& g, R area, double t)
    {
        const char* names[] = { "Micrófono", "Ordenador", "Studoo graba", "Auriculares" };
        const char* sub[] = { "tu voz", "tarjeta de sonido", "pista nueva en la playlist", "escuchas el beat" };
        const juce::Colour cols[] = { Colours::red, Colours::blue, Colours::accent, Colours::green };
        const float gap = 26.0f;
        const float w = (area.getWidth() - gap * 3.0f) / 4.0f;
        auto row = area.withSizeKeepingCentre (area.getWidth(), juce::jmin (100.0f, area.getHeight()));
        for (int i = 0; i < 4; ++i)
        {
            auto r = R (row.getX() + (float) i * (w + gap), row.getY(), w, row.getHeight());
            const bool rec = i == 2 && std::fmod (t, 1.0) < 0.5;
            box (g, r, cols[i], 10.0f, rec ? 0.35f : 0.15f);
            text (g, juce::String::fromUTF8 (names[i]), r.withTrimmedBottom (r.getHeight() * 0.45f), Colours::text, 14.0f, true);
            text (g, juce::String::fromUTF8 (sub[i]), r.withTrimmedTop (r.getHeight() * 0.5f).reduced (6.0f, 0.0f), Colours::textDim, 11.5f);
            if (i < 3)
                arrow (g, { r.getRight() + 3.0f, r.getCentreY() }, { r.getRight() + gap - 3.0f, r.getCentreY() }, Colours::textDim);
        }
        // Ondas saliendo del micrófono
        for (int i = 0; i < 3; ++i)
        {
            const float ph = (float) std::fmod (t * 0.8 + i / 3.0, 1.0);
            g.setColour (Colours::red.withAlpha ((1.0f - ph) * 0.6f));
            const float rad = 10.0f + ph * 30.0f;
            g.drawEllipse (R (rad * 2.0f, rad * 2.0f).withCentre ({ row.getX() + w * 0.5f, row.getY() - 4.0f }), 1.5f);
        }
    }

    void rutina (juce::Graphics& g, R area, double t)
    {
        const char* days[] = { "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb", "Dom" };
        const char* tasks[] = { "Ritmo\nnuevo", "Melodía y\nacordes", "Diseño\nde sonido", "Mezcla", "Estudiar\nuna canción", "Terminar\ncanción", "Exportar y\ncompartir" };
        const juce::Colour cols[] = { Colours::accent, Colours::blue, Colours::purple, Colours::green, Colours::teal, Colours::accentDeep, Colours::accentDeep };
        const float w = area.getWidth() / 7.0f;
        const int today = (int) std::fmod (t * 0.9, 7.0);
        for (int i = 0; i < 7; ++i)
        {
            auto col = R (area.getX() + (float) i * w, area.getY(), w, area.getHeight()).reduced (4.0f, 0.0f);
            const bool on = i == today;
            text (g, juce::String::fromUTF8 (days[i]), col.removeFromTop (22.0f), on ? Colours::text : Colours::textDim, 13.0f, true);
            auto card = col.reduced (0.0f, 4.0f);
            box (g, card, cols[i], 8.0f, on ? 0.35f : 0.12f);
            if (on) Draw::glow (g, card, cols[i], 6.0f, 8.0f);
            text (g, juce::String::fromUTF8 (tasks[i]), card.reduced (4.0f), Colours::text, 12.0f, on);
        }
    }
}

bool draw (juce::Graphics& g, const juce::String& id, juce::Rectangle<float> area, double time)
{
    const juce::Colour kick = Colours::accent, snare = juce::Colour (0xffffc23d), hat = Colours::blue,
                       perc = Colours::teal, bass = Colours::purple;

    if (id == "ruta")            ruta (g, area, time);
    else if (id == "flujo")      flujo (g, area, time);
    else if (id == "pulso")      pulso (g, area, time);
    else if (id == "pasos")      pasos (g, area, time);
    else if (id == "beat_basico")
        drumGrid (g, area, { { "Bombo", kick, "x.......x......." }, { "Caja", snare, "....x.......x..." },
                             { "Hi-hat", hat, "x.x.x.x.x.x.x.x." } }, 90.0, time);
    else if (id == "beat_trap")
        drumGrid (g, area, { { "Bombo", kick, "x......x..x....." }, { "Clap", snare, "........x......." },
                             { "Hi-hat", hat, "x.x.x.x.x.x.xxxx" }, { "808", bass, "x......x..x....." } }, 70.0, time,
                  "Los últimos 4 pasos del hi-hat seguidos hacen el roll \"trrrr\"");
    else if (id == "beat_reggaeton")
        drumGrid (g, area, { { "Bombo", kick, "x...x...x...x..." }, { "Caja", snare, "...x..x....x..x." },
                             { "Hi-hat", hat, "x.x.x.x.x.x.x.x." }, { "Shaker", perc, "xoxoxoxoxoxoxoxo" } }, 95.0, time);
    else if (id == "beat_house")
        drumGrid (g, area, { { "Bombo", kick, "x...x...x...x..." }, { "Clap", snare, "....x.......x..." },
                             { "Hat abierto", perc, "..x...x...x...x." }, { "Hat cerrado", hat, "oxoxoxoxoxoxoxox" } }, 124.0, time,
                  "Four on the floor: el bombo en cada tiempo");
    else if (id == "beat_boombap")
        drumGrid (g, area, { { "Bombo", kick, "x......x..x....." }, { "Caja", snare, "....x.......x..." },
                             { "Hi-hat", hat, "x.o.x.o.x.o.x.o." } }, 90.0, time);
    else if (id == "swing")      swing (g, area, time);
    else if (id == "velocidad")  velocidad (g, area, time);
    else if (id == "teclado")    teclado (g, area, time);
    else if (id == "escalas")    escalas (g, area, time);
    else if (id == "acordes")    acordes (g, area, time);
    else if (id == "pianoroll")  pianoroll (g, area, time);
    else if (id == "melodia")    melodia (g, area, time);
    else if (id == "808")        bass808 (g, area, time);
    else if (id == "sampler")    sampler (g, area, time);
    else if (id == "ondas")      ondas (g, area, time);
    else if (id == "adsr")       adsr (g, area, time);
    else if (id == "filtro")     filtro (g, area, time);
    else if (id == "estructura") estructura (g, area, time);
    else if (id == "volumenes")  volumenes (g, area, time);
    else if (id == "panorama")   panorama (g, area, time);
    else if (id == "eq")         eq (g, area, time);
    else if (id == "compresor")  compresor (g, area, time);
    else if (id == "reverb")     reverb (g, area, time);
    else if (id == "delay")      delay (g, area, time);
    else if (id == "automatizacion") automatizacion (g, area, time);
    else if (id == "grabar")     grabar (g, area, time);
    else if (id == "rutina")     rutina (g, area, time);
    else return false;
    return true;
}

float preferredHeight (const juce::String& id, float width)
{
    if (id == "flujo" || id == "grabar")                 return 150.0f;
    if (id == "pasos" || id == "beat_basico" || id == "beat_boombap") return 170.0f;
    if (id == "teclado" || id == "escalas")               return juce::jlimit (150.0f, 220.0f, width * 0.26f);
    if (id == "acordes")                                  return juce::jlimit (180.0f, 250.0f, width * 0.3f);
    if (id == "swing" || id == "velocidad")               return 170.0f;
    return 220.0f;
}
}
