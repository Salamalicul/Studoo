#include "LearnView.h"
#include "Challenges.h"
#include "EarGames.h"
#include "Glossary.h"
#include "HelpTexts.h"
#include "Illustrations.h"
#include "LearnQuiz.h"
#include "LearnStyle.h"
#include "LearnTexts.h"
#include "Pro.h"
#include <iterator>

namespace studoo
{
using learn::fs;

namespace
{
    constexpr int numLessons = (int) std::size (learn::lessons);
    constexpr int numLevels = (int) std::size (learn::levels);
    constexpr int maxContentWidth = 820;

    int levelOf (int lesson)
    {
        const juce::String level = txt (learn::lessons[lesson].level);
        for (int i = 0; i < numLevels; ++i)
            if (level == txt (learn::levels[i]))
                return i;
        return 0;
    }

    juce::Colour levelColour (int level)
    {
        const juce::Colour colours[] = { Colours::accent, Colours::accentLight, Colours::blue, Colours::purple,
                                         Colours::green, Colours::teal, Colours::red, Colours::accent, Colours::blue };
        return colours[level % 9];
    }

    // Nombre corto del nivel: "Nivel 2 · Ritmo y batería" -> "Ritmo y batería"
    juce::String levelName (int level) { return txt (learn::levels[level]).fromFirstOccurrenceOf (juce::String::fromUTF8 ("· "), false, false); }

    // Para buscar sin importar tildes ni mayúsculas
    juce::String normalise (const juce::String& s)
    {
        return s.toLowerCase()
                .replaceCharacters (juce::String::fromUTF8 ("áéíóúüàèìòù"), "aeiouuaeiou");
    }

    // Añade texto con **negritas** a un AttributedString
    void appendRich (juce::AttributedString& s, const juce::String& line, float size, juce::Colour normal, juce::Colour boldColour)
    {
        auto rest = line;
        bool bold = false;
        while (rest.isNotEmpty())
        {
            const int idx = rest.indexOf ("**");
            const auto part = idx < 0 ? rest : rest.substring (0, idx);
            if (part.isNotEmpty())
                s.append (part, juce::Font (bold ? Fonts::bold (size) : Fonts::regular (size)), bold ? boldColour : normal);
            if (idx < 0)
                break;
            rest = rest.substring (idx + 2);
            bold = ! bold;
        }
    }

    juce::TextLayout makeLayout (const juce::String& text, float size, juce::Colour colour, float width, float spacing = 4.0f,
                                 juce::Colour boldColour = Colours::accentLight)
    {
        juce::AttributedString s;
        s.setLineSpacing (spacing);
        appendRich (s, text, size, colour, boldColour);
        juce::TextLayout layout;
        layout.createLayout (s, juce::jmax (40.0f, width));
        return layout;
    }

    // Una pequeña "pastilla" de texto (chip)
    juce::Rectangle<float> drawChip (juce::Graphics& g, const juce::String& text, juce::Point<float> topLeft, juce::Colour c, float size)
    {
        const auto font = Fonts::bold (size);
        auto r = juce::Rectangle<float> (topLeft.x, topLeft.y, (float) juce::GlyphArrangement::getStringWidthInt (font, text) + size * 1.6f, size * 1.9f);
        g.setColour (c.withAlpha (0.2f));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (c.brighter (0.35f));
        g.setFont (font);
        g.drawText (text, r, juce::Justification::centred);
        return r;
    }

    // Candado pequeño (cosas de Studoo Pro)
    void drawLock (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
    {
        const float s = juce::jmin (r.getWidth(), r.getHeight());
        auto body = juce::Rectangle<float> (s * 0.64f, s * 0.46f).withCentre ({ r.getCentreX(), r.getCentreY() + s * 0.14f });
        juce::Path shackle;
        shackle.addCentredArc (r.getCentreX(), body.getY(), s * 0.2f, s * 0.22f, 0.0f,
                               -juce::MathConstants<float>::halfPi, juce::MathConstants<float>::halfPi, true);
        g.setColour (c);
        g.strokePath (shackle, juce::PathStrokeType (s * 0.1f));
        g.fillRoundedRectangle (body, s * 0.08f);
    }

    // Dibujo de cada medalla
    void drawMedalIcon (juce::Graphics& g, int icon, juce::Rectangle<float> r, juce::Colour c)
    {
        g.setColour (c);
        const auto ctr = r.getCentre();
        const float s = r.getWidth();
        juce::Path p;
        switch (icon)
        {
            case 0: // estrella
                p.addStar (ctr, 5, s * 0.2f, s * 0.46f, 0.0f);
                g.fillPath (p);
                break;
            case 1: // libro
                p.addRoundedRectangle (r.reduced (s * 0.18f, s * 0.22f), 3.0f);
                g.fillPath (p);
                g.setColour (juce::Colours::black.withAlpha (0.35f));
                g.fillRect (ctr.x - 1.0f, r.getY() + s * 0.22f, 2.0f, s * 0.56f);
                break;
            case 2: // batería
                g.drawEllipse (r.reduced (s * 0.16f), 3.0f);
                g.fillEllipse (juce::Rectangle<float> (s * 0.3f, s * 0.3f).withCentre (ctr));
                break;
            case 3: // nota musical
                g.fillEllipse (juce::Rectangle<float> (s * 0.34f, s * 0.26f).withCentre ({ ctr.x - s * 0.08f, ctr.y + s * 0.2f }));
                g.fillRect (ctr.x + s * 0.06f, r.getY() + s * 0.15f, s * 0.07f, s * 0.5f);
                g.fillRect (ctr.x + s * 0.06f, r.getY() + s * 0.15f, s * 0.22f, s * 0.08f);
                break;
            case 4: // mezclador
                for (int i = 0; i < 3; ++i)
                {
                    const float x = r.getX() + s * (0.25f + 0.25f * (float) i);
                    g.fillRect (x - 1.5f, r.getY() + s * 0.18f, 3.0f, s * 0.64f);
                    g.fillRoundedRectangle (x - s * 0.08f, r.getY() + s * (0.3f + 0.18f * (float) ((i * 2) % 3)), s * 0.16f, s * 0.1f, 2.0f);
                }
                break;
            case 5: // globo
                g.drawEllipse (r.reduced (s * 0.16f), 2.5f);
                g.drawEllipse (r.reduced (s * 0.34f, s * 0.16f), 2.0f);
                g.fillRect (r.getX() + s * 0.16f, ctr.y - 1.0f, s * 0.68f, 2.0f);
                break;
            case 6: // check
                p.startNewSubPath (r.getX() + s * 0.22f, ctr.y);
                p.lineTo (r.getX() + s * 0.42f, r.getY() + s * 0.7f);
                p.lineTo (r.getRight() - s * 0.2f, r.getY() + s * 0.28f);
                g.strokePath (p, juce::PathStrokeType (s * 0.11f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                break;
            case 7: // trofeo
                p.addRoundedRectangle (r.getX() + s * 0.26f, r.getY() + s * 0.18f, s * 0.48f, s * 0.36f, s * 0.14f);
                p.addRectangle (ctr.x - s * 0.05f, r.getY() + s * 0.52f, s * 0.1f, s * 0.16f);
                p.addRoundedRectangle (r.getX() + s * 0.3f, r.getY() + s * 0.68f, s * 0.4f, s * 0.1f, 2.0f);
                g.fillPath (p);
                break;
            case 8: // diana
                g.drawEllipse (r.reduced (s * 0.14f), 2.5f);
                g.drawEllipse (r.reduced (s * 0.28f), 2.5f);
                g.fillEllipse (r.reduced (s * 0.42f));
                break;
            case 9: // oreja (auriculares)
            {
                juce::Path arc;
                arc.addCentredArc (ctr.x, ctr.y + s * 0.05f, s * 0.3f, s * 0.3f, 0.0f, -juce::MathConstants<float>::halfPi,
                                   juce::MathConstants<float>::halfPi, true);
                g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.fillRoundedRectangle (ctr.x - s * 0.36f, ctr.y, s * 0.16f, s * 0.26f, 3.0f);
                g.fillRoundedRectangle (ctr.x + s * 0.2f, ctr.y, s * 0.16f, s * 0.26f, 3.0f);
                break;
            }
            default: // llama
                p.startNewSubPath (ctr.x, r.getY() + s * 0.12f);
                p.cubicTo (ctr.x + s * 0.36f, ctr.y - s * 0.05f, ctr.x + s * 0.32f, r.getBottom() - s * 0.12f, ctr.x, r.getBottom() - s * 0.12f);
                p.cubicTo (ctr.x - s * 0.32f, r.getBottom() - s * 0.12f, ctr.x - s * 0.3f, ctr.y, ctr.x - s * 0.08f, ctr.y - s * 0.1f);
                p.cubicTo (ctr.x - s * 0.06f, ctr.y + s * 0.06f, ctr.x + s * 0.02f, ctr.y, ctr.x, r.getY() + s * 0.12f);
                p.closeSubPath();
                g.fillPath (p);
                break;
        }
    }

    // ------------------------------------------------------------------------
    //  Bloques de una lección
    // ------------------------------------------------------------------------
    struct Block : public juce::Component
    {
        // Prepara el bloque para un ancho y devuelve su alto
        virtual int layoutFor (int width) = 0;
        std::function<void()> onHeightChanged;
    };

    struct TextBlock : public Block
    {
        enum class Kind { paragraph, heading, bullet, step };
        Kind kind;
        juce::String content;
        int number = 0;
        juce::TextLayout layout;

        TextBlock (Kind k, juce::String c, int n = 0) : kind (k), content (std::move (c)), number (n)
        {
            setInterceptsMouseClicks (false, false);
        }

        float indent() const { return kind == Kind::bullet ? fs (24.0f) : kind == Kind::step ? fs (44.0f) : 0.0f; }

        int layoutFor (int width) override
        {
            const float width_ = (float) width - indent() - (kind == Kind::step ? 12.0f : 0.0f);
            if (kind == Kind::heading)
                layout = makeLayout (content, fs (21.0f), Colours::text, width_ - 16.0f, 0.0f);
            else
                layout = makeLayout (content, fs (16.0f), Colours::text.withAlpha (0.92f), width_, fs (5.0f));
            const int textH = (int) std::ceil (layout.getHeight());
            if (kind == Kind::heading) return textH + 34;
            if (kind == Kind::step)    return juce::jmax ((int) fs (38.0f), textH + 18);
            return textH + 6;
        }

        void paint (juce::Graphics& g) override
        {
            auto area = getLocalBounds().toFloat();
            if (kind == Kind::heading)
            {
                auto line = area.removeFromTop (18.0f);
                g.setGradientFill (juce::ColourGradient (Colours::outline.brighter (0.3f), 0.0f, 0.0f,
                                                         Colours::outline.withAlpha (0.0f), area.getWidth(), 0.0f, false));
                g.fillRect (line.withHeight (1.0f).withY (line.getCentreY()));
                auto mark = area.removeFromLeft (4.0f).withHeight (layout.getHeight() + 2.0f);
                Draw::accentFill (g, mark, 2.0f);
                area.removeFromLeft (12.0f);
                layout.draw (g, area.withHeight (layout.getHeight()));
                return;
            }
            if (kind == Kind::bullet)
            {
                const float d = fs (7.0f);
                auto dot = juce::Rectangle<float> (d, d).withCentre ({ fs (8.0f), fs (12.0f) });
                Draw::accentFill (g, dot, d * 0.5f);
            }
            if (kind == Kind::step)
            {
                auto card = area.reduced (0.0f, 2.0f);
                g.setColour (Colours::panelLight.withAlpha (0.55f));
                g.fillRoundedRectangle (card, 12.0f);
                const float d = fs (28.0f);
                auto circle = juce::Rectangle<float> (d, d).withCentre ({ fs (22.0f), card.getY() + 9.0f + d * 0.5f });
                Draw::glow (g, circle, Colours::accent, 4.0f, d * 0.5f);
                Draw::accentFill (g, circle, d * 0.5f);
                g.setColour (juce::Colour (0xff1a1208));
                g.setFont (Fonts::bold (fs (14.0f)));
                g.drawText (juce::String (number), circle, juce::Justification::centred);
                layout.draw (g, juce::Rectangle<float> (indent(), card.getY() + 9.0f, area.getWidth() - indent() - 12.0f, layout.getHeight()));
                return;
            }
            layout.draw (g, area.withTrimmedLeft (indent()).withHeight (layout.getHeight()));
        }
    };

    // Cuadros de color: Truco (azul), Ojo (naranja), Practica (verde)
    struct CalloutBlock : public Block
    {
        enum class Kind { tip, warning, practice };
        Kind kind;
        juce::String content;
        juce::TextLayout layout;

        CalloutBlock (Kind k, juce::String c) : kind (k), content (std::move (c)) { setInterceptsMouseClicks (false, false); }

        juce::Colour colour() const { return kind == Kind::tip ? Colours::blue : kind == Kind::warning ? Colours::accent : Colours::green; }
        juce::String label() const  { return kind == Kind::tip ? "TRUCO" : kind == Kind::warning ? "OJO" : "PRACTICA"; }

        int layoutFor (int width) override
        {
            layout = makeLayout (content, fs (15.5f), Colours::text, (float) width - 40.0f, fs (4.0f), colour().brighter (0.4f));
            return (int) std::ceil (layout.getHeight()) + (int) fs (54.0f);
        }

        void paint (juce::Graphics& g) override
        {
            auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
            const auto c = colour();
            g.setGradientFill (juce::ColourGradient (c.withAlpha (0.16f), 0.0f, area.getY(), c.withAlpha (0.06f), 0.0f, area.getBottom(), false));
            g.fillRoundedRectangle (area, 12.0f);
            g.setColour (c.withAlpha (0.55f));
            g.drawRoundedRectangle (area.reduced (0.5f), 12.0f, 1.0f);
            Draw::colourFill (g, area.withWidth (4.0f).reduced (0.0f, 10.0f).translated (1.0f, 0.0f), c, 2.0f);

            auto inner = area.reduced (20.0f, 12.0f);
            drawChip (g, label(), inner.getTopLeft(), c, fs (11.0f));
            inner.removeFromTop (fs (11.0f) * 1.9f + 8.0f);
            layout.draw (g, inner.withHeight (layout.getHeight()));
        }
    };

    // Ilustración animada con su pie de foto
    struct IllustrationBlock : public Block, private juce::Timer
    {
        juce::String id, caption;
        double startTime = juce::Time::getMillisecondCounterHiRes();

        IllustrationBlock (juce::String i, juce::String c) : id (std::move (i)), caption (std::move (c))
        {
            setInterceptsMouseClicks (false, false);
            startTimerHz (30);
        }

        int layoutFor (int width) override
        {
            return (int) illustrations::preferredHeight (id, (float) width - 40.0f) + 40 + (caption.isNotEmpty() ? (int) fs (30.0f) : 0);
        }

        void timerCallback() override
        {
            if (isShowing())
                repaint();
        }

        void paint (juce::Graphics& g) override
        {
            auto area = getLocalBounds().toFloat().reduced (0.0f, 6.0f);
            Draw::shadow (g, area, 14.0f, 12);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff16191e), 0.0f, area.getY(), juce::Colour (0xff111317), 0.0f, area.getBottom(), false));
            g.fillRoundedRectangle (area, 14.0f);
            g.setColour (Colours::outline);
            g.drawRoundedRectangle (area.reduced (0.5f), 14.0f, 1.0f);

            if (caption.isNotEmpty())
            {
                auto cap = area.removeFromBottom (fs (32.0f));
                g.setColour (juce::Colours::black.withAlpha (0.25f));
                g.fillRect (cap.reduced (1.0f, 0.0f));
                g.setColour (Colours::textDim);
                g.setFont (Fonts::regular (fs (13.0f)));
                g.drawFittedText (caption, cap.reduced (14.0f, 0.0f).toNearestInt(), juce::Justification::centred, 2, 0.9f);
            }
            const double t = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
            auto content = area.reduced (20.0f, 16.0f);
            juce::Graphics::ScopedSaveState save (g);
            g.reduceClipRegion (area.toNearestInt());
            illustrations::draw (g, id, content, t);
        }
    };

    struct ButtonBlock : public Block
    {
        juce::TextButton button;
        ButtonBlock (const juce::String& text, std::function<void()> action)
        {
            button.setButtonText (text);
            button.onClick = std::move (action);
            learn::styleButton (button, learn::ButtonStyle::primary, 15.0f);
            addAndMakeVisible (button);
        }
        int layoutFor (int) override { return learn::buttonHeight() + 16; }
        void resized() override
        {
            const int w = juce::GlyphArrangement::getStringWidthInt (Fonts::bold (fs (15.0f)), button.getButtonText()) + 60;
            button.setBounds (0, 8, juce::jmin (getWidth(), w), learn::buttonHeight());
        }
    };

    // ------------------------------------------------------------------------
    //  Reto: Studoo comprueba solo si ya lo has hecho
    // ------------------------------------------------------------------------
    struct ChallengeBlock : public Block, private juce::Timer
    {
        LearnView& view;
        const learn::Challenge& challenge;
        juce::TextButton goButton;
        juce::TextLayout titleLayout, textLayout, hintLayout;
        bool done = false;

        ChallengeBlock (LearnView& v, const learn::Challenge& c) : view (v), challenge (c)
        {
            done = view.getProgress().isChallengeDone (challenge.id);
            goButton.setButtonText (txt ("Ir a hacerlo  →"));
            learn::styleButton (goButton, learn::ButtonStyle::primary, 14.5f);
            goButton.onClick = [this] { if (view.onGoToView) view.onGoToView (challenge.view); };
            addAndMakeVisible (goButton);
            startTimer (800);
        }

        void timerCallback() override
        {
            if (done || ! isShowing())
                return;
            if (challenge.check (view.getContext().project))
            {
                done = true;
                view.getProgress().setChallengeDone (challenge.id);
                view.playCelebration();
                if (onHeightChanged) onHeightChanged();
                repaint();
            }
        }

        // También se comprueba al volver a Aprende después de hacerlo
        void visibilityChanged() override { if (isShowing()) timerCallback(); }

        int layoutFor (int width) override
        {
            const float w = (float) width - 44.0f;
            titleLayout = makeLayout ("**" + txt (challenge.title) + "**", fs (18.0f), Colours::text, w, 4.0f, Colours::text);
            textLayout = makeLayout (txt (challenge.instruction), fs (15.5f), Colours::text.withAlpha (0.9f), w);
            hintLayout = makeLayout (txt ("Pista: ") + txt (challenge.hint), fs (13.5f), Colours::textDim, w);
            goButton.setVisible (! done);
            const int h = (int) (fs (11.0f) * 1.9f) + 14 + (int) titleLayout.getHeight() + 6 + (int) textLayout.getHeight() + 8
                          + (done ? 0 : (int) hintLayout.getHeight() + 14 + learn::buttonHeight()) + 30;
            return h;
        }

        void resized() override
        {
            const int bh = learn::buttonHeight();
            goButton.setBounds (22, getHeight() - bh - 18, juce::jmin (getWidth() - 44, 240), bh);
        }

        void paint (juce::Graphics& g) override
        {
            auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
            const auto c = done ? Colours::green : Colours::purple;
            g.setGradientFill (juce::ColourGradient (c.withAlpha (0.2f), 0.0f, area.getY(), c.withAlpha (0.05f), 0.0f, area.getBottom(), false));
            g.fillRoundedRectangle (area, 14.0f);
            g.setColour (c.withAlpha (0.7f));
            g.drawRoundedRectangle (area.reduced (0.5f), 14.0f, 1.4f);

            auto inner = area.reduced (22.0f, 14.0f);
            auto chip = drawChip (g, done ? txt ("RETO SUPERADO  ✓") : txt ("RETO"), inner.getTopLeft(), c, fs (11.0f));
            if (! done)
            {
                g.setColour (Colours::textDim);
                g.setFont (Fonts::regular (fs (12.5f)));
                g.drawText (txt ("Studoo lo comprueba solo"), chip.withX (chip.getRight() + 10.0f).withWidth (260.0f), juce::Justification::centredLeft);
            }
            inner.removeFromTop (chip.getHeight() + 14.0f);
            titleLayout.draw (g, inner.removeFromTop (titleLayout.getHeight()));
            inner.removeFromTop (6.0f);
            textLayout.draw (g, inner.removeFromTop (textLayout.getHeight()));
            inner.removeFromTop (8.0f);
            if (! done)
                hintLayout.draw (g, inner.removeFromTop (hintLayout.getHeight()));
        }
    };

    // ------------------------------------------------------------------------
    //  Test de la lección
    // ------------------------------------------------------------------------
    struct QuizBlock : public Block
    {
        LearnView& view;
        juce::String lessonTitle;
        std::vector<const learn::QuizQuestion*> questions;
        std::vector<std::array<int, 3>> order;      // orden en que se enseñan las respuestas
        int current = 0, score = 0, chosen = -1;
        bool finished = false, started = false;
        juce::OwnedArray<juce::TextButton> options;
        juce::TextButton actionButton;
        juce::TextLayout questionLayout, whyLayout;

        QuizBlock (LearnView& v, const juce::String& title, std::vector<const learn::QuizQuestion*> qs)
            : view (v), lessonTitle (title), questions (std::move (qs))
        {
            for (int i = 0; i < 3; ++i)
            {
                auto* b = options.add (new juce::TextButton());
                b->onClick = [this, i] { choose (i); };
                addChildComponent (b);
            }
            actionButton.onClick = [this] { action(); };
            addAndMakeVisible (actionButton);
            reset (false);
        }

        void reset (bool start)
        {
            current = score = 0;
            chosen = -1;
            finished = false;
            started = start;
            juce::Random r;
            order.clear();
            for (size_t q = 0; q < questions.size(); ++q)
            {
                std::array<int, 3> o { 0, 1, 2 };
                for (int i = 2; i > 0; --i)
                    std::swap (o[(size_t) i], o[(size_t) r.nextInt (i + 1)]);
                order.push_back (o);
            }
            refresh();
        }

        void choose (int shown)
        {
            if (chosen >= 0 || finished)
                return;
            chosen = shown;
            const auto& q = *questions[(size_t) current];
            if (order[(size_t) current][(size_t) shown] == q.correct)
                ++score;
            refresh();
        }

        void action()
        {
            if (! started) { reset (true); return; }
            if (finished)  { reset (true); return; }
            if (chosen < 0) return;
            if (current + 1 < (int) questions.size())
            {
                ++current;
                chosen = -1;
            }
            else
            {
                finished = true;
                view.getProgress().recordQuiz (lessonTitle, score, (int) questions.size());
                if (score == (int) questions.size())
                    view.playCelebration();
            }
            refresh();
        }

        void refresh()
        {
            const bool asking = started && ! finished;
            for (int i = 0; i < 3; ++i)
            {
                auto* b = options[i];
                b->setVisible (asking);
                if (! asking)
                    continue;
                const auto& q = *questions[(size_t) current];
                const int real = order[(size_t) current][(size_t) i];
                b->setButtonText (txt (q.options[real]));
                auto style = learn::ButtonStyle::secondary;
                if (chosen >= 0 && real == q.correct) style = learn::ButtonStyle::good;
                else if (chosen == i)                 style = learn::ButtonStyle::bad;
                learn::styleButton (*b, style, 15.5f);
                b->setEnabled (chosen < 0 || real == q.correct || chosen == i);
            }
            if (! started)        actionButton.setButtonText (txt ("Empezar el test"));
            else if (finished)    actionButton.setButtonText (txt ("Repetir el test"));
            else if (current + 1 < (int) questions.size()) actionButton.setButtonText (txt ("Siguiente pregunta  →"));
            else                  actionButton.setButtonText (txt ("Ver mi resultado"));
            learn::styleButton (actionButton, learn::ButtonStyle::primary, 15.0f);
            actionButton.setVisible (! started || finished || chosen >= 0);
            if (onHeightChanged) onHeightChanged();
            repaint();
        }

        int layoutFor (int width) override
        {
            const float w = (float) width - 44.0f;
            const int bh = learn::buttonHeight();
            int h = (int) (fs (11.0f) * 1.9f) + 14;
            if (! started || finished)
            {
                questionLayout = makeLayout (! started ? txt ("¿Lo has entendido? Responde 3 preguntas rápidas.")
                                                       : resultText(), fs (17.0f), Colours::text, w);
                h += (int) questionLayout.getHeight() + 16 + bh + 22;
                return h;
            }
            const auto& q = *questions[(size_t) current];
            questionLayout = makeLayout (txt (q.question), fs (18.0f), Colours::text, w);
            h += (int) questionLayout.getHeight() + 14 + 3 * (bh + 8);
            if (chosen >= 0)
            {
                const bool right = order[(size_t) current][(size_t) chosen] == q.correct;
                whyLayout = makeLayout ((right ? txt ("**¡Correcto!** ") : txt ("**No es esa.** ")) + txt (q.why), fs (15.0f),
                                        Colours::text.withAlpha (0.9f), w, 4.0f, right ? Colours::green : Colours::red.brighter (0.3f));
                h += (int) whyLayout.getHeight() + 14 + bh + 10;
            }
            return h + 16;
        }

        juce::String resultText() const
        {
            const int total = (int) questions.size();
            juce::String s = txt ("**") + juce::String (score) + txt (" de ") + juce::String (total) + txt ("**  ");
            if (score == total)           s << txt ("¡Perfecto! Lo dominas.");
            else if (score * 2 >= total)  s << txt ("¡Bien! Repasa la lección y vuelve a intentarlo.");
            else                          s << txt ("Vuelve a leer la lección con calma y repite el test.");
            return s;
        }

        void resized() override
        {
            const int bh = learn::buttonHeight();
            auto inner = getLocalBounds().reduced (22, 14);
            inner.removeFromTop ((int) (fs (11.0f) * 1.9f) + 14 + (int) questionLayout.getHeight() + 14);
            if (started && ! finished)
            {
                for (auto* b : options)
                {
                    b->setBounds (inner.removeFromTop (bh));
                    inner.removeFromTop (8);
                }
                if (chosen >= 0)
                    inner.removeFromTop ((int) whyLayout.getHeight() + 14);
            }
            else
            {
                inner.removeFromTop (2);
            }
            actionButton.setBounds (inner.removeFromTop (bh).withWidth (juce::jmin (inner.getWidth(), 280)));
        }

        void paint (juce::Graphics& g) override
        {
            auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
            const auto c = Colours::blue;
            g.setGradientFill (juce::ColourGradient (c.withAlpha (0.16f), 0.0f, area.getY(), Colours::panel.withAlpha (0.6f), 0.0f, area.getBottom(), false));
            g.fillRoundedRectangle (area, 14.0f);
            g.setColour (c.withAlpha (0.6f));
            g.drawRoundedRectangle (area.reduced (0.5f), 14.0f, 1.2f);

            auto inner = area.reduced (22.0f, 14.0f);
            const juce::String label = ! started || finished ? txt ("TEST")
                                                             : txt ("TEST · PREGUNTA ") + juce::String (current + 1) + txt (" DE ") + juce::String ((int) questions.size());
            auto chip = drawChip (g, label, inner.getTopLeft(), c, fs (11.0f));
            const int best = view.getProgress().bestQuizScore (lessonTitle);
            if (best >= 0)
            {
                g.setColour (best == (int) questions.size() ? Colours::green : Colours::textDim);
                g.setFont (Fonts::bold (fs (12.5f)));
                g.drawText (txt ("Tu mejor nota: ") + juce::String (best) + "/" + juce::String ((int) questions.size()),
                            chip.withX (chip.getRight() + 10.0f).withWidth (240.0f), juce::Justification::centredLeft);
            }
            inner.removeFromTop (chip.getHeight() + 14.0f);
            questionLayout.draw (g, inner.removeFromTop (questionLayout.getHeight()));
            if (started && ! finished && chosen >= 0)
            {
                inner.removeFromTop (14.0f + 3.0f * (float) (learn::buttonHeight() + 8));
                whyLayout.draw (g, inner.removeFromTop (whyLayout.getHeight()));
            }
        }
    };
}

// ============================================================================
//  LearnArticle: una lección completa
// ============================================================================
class LearnArticle : public juce::Component
{
public:
    explicit LearnArticle (LearnView& v) : view (v)
    {
        for (auto* b : { &videoButton, &doneButton, &nextButton })
            addAndMakeVisible (*b);
        videoButton.setTooltip (txt ("Abre YouTube con vídeos en español sobre este tema."));
        doneButton.setTooltip (txt ("Marca esta lección como aprendida. Tu progreso se guarda."));
        doneButton.setClickingTogglesState (false);
    }

    void setLesson (int index)
    {
        lesson = index;
        blocks.clear();
        const auto& l = learn::lessons[index];

        auto lines = juce::StringArray::fromLines (txt (l.body));
        juce::String paragraph;
        auto flush = [&]
        {
            if (paragraph.isNotEmpty())
                add (new TextBlock (TextBlock::Kind::paragraph, paragraph.trim()));
            paragraph.clear();
        };

        for (auto line : lines)
        {
            const auto trimmed = line.trim();
            if (trimmed.isEmpty())                 { flush(); continue; }
            if (trimmed.startsWith ("## "))        { flush(); add (new TextBlock (TextBlock::Kind::heading, trimmed.substring (3))); }
            else if (trimmed.startsWith ("- "))    { flush(); add (new TextBlock (TextBlock::Kind::bullet, trimmed.substring (2))); }
            else if (trimmed.startsWith ("> "))    { flush(); add (new CalloutBlock (CalloutBlock::Kind::tip, trimmed.substring (2))); }
            else if (trimmed.startsWith ("! "))    { flush(); add (new CalloutBlock (CalloutBlock::Kind::warning, trimmed.substring (2))); }
            else if (trimmed.startsWith ("? "))    { flush(); add (new CalloutBlock (CalloutBlock::Kind::practice, trimmed.substring (2))); }
            else if (trimmed.startsWith ("[anim:"))
            {
                flush();
                auto inner = trimmed.fromFirstOccurrenceOf ("[anim:", false, false).upToLastOccurrenceOf ("]", false, false);
                add (new IllustrationBlock (inner.upToFirstOccurrenceOf ("|", false, false),
                                            inner.fromFirstOccurrenceOf ("|", false, false)));
            }
            else if (trimmed.startsWith ("[reto:"))
            {
                flush();
                const auto id = trimmed.fromFirstOccurrenceOf ("[reto:", false, false).upToLastOccurrenceOf ("]", false, false);
                if (auto* c = learn::findChallenge (id))
                    add (new ChallengeBlock (view, *c));
            }
            else if (trimmed.startsWith ("[base:"))
            {
                flush();
                auto inner = trimmed.fromFirstOccurrenceOf ("[base:", false, false).upToLastOccurrenceOf ("]", false, false);
                const auto genre = inner.upToFirstOccurrenceOf ("|", false, false);
                const auto label = inner.fromFirstOccurrenceOf ("|", false, false);
                add (new ButtonBlock (juce::String::fromUTF8 ("▶  ") + label, [this, genre] { if (view.onLoadGenre) view.onLoadGenre (genre); }));
            }
            else if (trimmed.startsWith ("[ir:"))
            {
                flush();
                auto inner = trimmed.fromFirstOccurrenceOf ("[ir:", false, false).upToLastOccurrenceOf ("]", false, false);
                const auto target = inner.upToFirstOccurrenceOf ("|", false, false);
                const auto label = inner.fromFirstOccurrenceOf ("|", false, false);
                View v = View::channelRack;
                if (target == "pianoroll")           v = View::pianoRoll;
                else if (target == "playlist")       v = View::playlist;
                else if (target == "mezclador")      v = View::mixer;
                else if (target == "automatizacion") v = View::automation;
                add (new ButtonBlock (label + juce::String::fromUTF8 ("  →"), [this, v] { if (view.onGoToView) view.onGoToView (v); }));
            }
            else
            {
                const auto digits = trimmed.initialSectionContainingOnly ("0123456789");
                if (digits.isNotEmpty() && trimmed.substring (digits.length()).startsWith (". "))
                {
                    flush();
                    add (new TextBlock (TextBlock::Kind::step, trimmed.substring (digits.length() + 2), digits.getIntValue()));
                }
                else
                {
                    paragraph << (paragraph.isEmpty() ? "" : " ") << trimmed;
                }
            }
        }
        flush();

        // Test al final de la lección
        std::vector<const learn::QuizQuestion*> questions;
        for (auto& q : learn::quiz)
            if (txt (q.lesson) == txt (l.title))
                questions.push_back (&q);
        if (! questions.empty())
        {
            add (new TextBlock (TextBlock::Kind::heading, txt ("Comprueba lo que has aprendido")));
            add (new QuizBlock (view, txt (l.title), std::move (questions)));
        }

        videoButton.setButtonText (txt ("Ver vídeos en YouTube"));
        nextButton.setButtonText (index + 1 < numLessons ? txt ("Siguiente lección  →") : txt ("¡Ruta completada!"));
        nextButton.setEnabled (index + 1 < numLessons);
        learn::styleButton (videoButton, learn::ButtonStyle::secondary, 14.0f);
        learn::styleButton (nextButton, learn::ButtonStyle::primary, 14.5f);
        videoButton.onClick = [index]
        {
            juce::URL ("https://www.youtube.com/results")
                .withParameter ("search_query", txt (learn::lessons[index].video))
                .launchInDefaultBrowser();
        };
        doneButton.onClick = [this, index]
        {
            view.setDone (index, ! view.isDone (index));
            refreshDone();
        };
        nextButton.onClick = [this, index]
        {
            if (! view.isDone (index))
                view.setDone (index, true);
            if (index + 1 < numLessons)
                view.showLesson (index + 1);
        };
        refreshDone();
    }

    void refreshDone()
    {
        const bool done = lesson >= 0 && view.isDone (lesson);
        learn::styleButton (doneButton, done ? learn::ButtonStyle::good : learn::ButtonStyle::secondary, 14.0f);
        doneButton.setButtonText (done ? txt ("Aprendida  ✓") : txt ("Marcar como aprendida"));
        repaint();
    }

    void layoutForWidth (int width)
    {
        lastWidth = width;
        const bool phone = Layout::compact() || width < 600;
        const int margin = phone ? 18 : 36;
        const int contentW = juce::jmin (maxContentWidth, width - margin * 2);
        const int x = (width - contentW) / 2;
        const int headerHeight = (int) (fs (64.0f) + fs (34.0f) * (titleLines (contentW) > 1 ? 2.2f : 1.15f)) + 24;
        int y = headerHeight;
        for (auto* b : blocks)
        {
            const int h = b->layoutFor (contentW);
            b->setBounds (x, y, contentW, h);
            y += h + 10;
        }
        y += 20;
        footerY = y;
        const int bh = learn::buttonHeight();
        if (phone)
        {
            // En el móvil, los botones uno debajo de otro y a todo lo ancho
            nextButton.setBounds (x, y + 18, contentW, bh);
            doneButton.setBounds (x, y + 18 + bh + 10, contentW, bh);
            videoButton.setBounds (x, y + 18 + 2 * (bh + 10), contentW, bh);
            y += 18 + 3 * (bh + 10);
        }
        else
        {
            videoButton.setBounds (x, y + 20, 220, bh);
            doneButton.setBounds (x + 230, y + 20, 220, bh);
            nextButton.setBounds (x + contentW - 230, y + 20, 230, bh);
            y += 20 + bh;
        }
        contentX = x;
        contentWidth = contentW;
        headerH = headerHeight;
        setSize (width, y + 50);
    }

    void relayout()
    {
        if (lastWidth > 0)
            layoutForWidth (lastWidth);
    }

    void paint (juce::Graphics& g) override
    {
        if (lesson < 0)
            return;
        const auto& l = learn::lessons[lesson];
        const float x = (float) contentX, w = (float) contentWidth;

        // Nivel, número y duración
        const auto levelIdx = levelOf (lesson);
        auto chip = drawChip (g, levelName (levelIdx).toUpperCase(), { x, fs (24.0f) }, levelColour (levelIdx), fs (11.5f));
        const auto meta = txt ("Lección ") + juce::String (lesson + 1) + txt (" de ") + juce::String (numLessons) + "  ·  " + txt (l.minutes);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (fs (13.0f)));
        g.drawText (meta, juce::Rectangle<float> (chip.getRight() + 12.0f, chip.getY(), w - chip.getWidth() - 12.0f, chip.getHeight()),
                    juce::Justification::centredLeft);

        g.setColour (Colours::text);
        g.setFont (Fonts::display (fs (31.0f)));
        const int lines = titleLines (contentWidth);
        auto titleArea = juce::Rectangle<float> (x, fs (58.0f), w, fs (34.0f) * (float) lines * 1.1f);
        g.drawFittedText (txt (l.title), titleArea.toNearestInt(), juce::Justification::topLeft, lines, 0.9f);
        g.setGradientFill (juce::ColourGradient (Colours::accent, x, 0.0f, Colours::accent.withAlpha (0.0f), x + w * 0.8f, 0.0f, false));
        g.fillRect (x, (float) headerH - 18.0f, w, 2.0f);

        // Pie: línea separadora
        g.setColour (Colours::outline);
        g.drawHorizontalLine (footerY, x, x + w);
    }

private:
    int titleLines (int width) const
    {
        if (lesson < 0)
            return 1;
        const auto w = juce::GlyphArrangement::getStringWidth (Fonts::display (fs (31.0f)), txt (learn::lessons[lesson].title));
        return w > (float) width ? 2 : 1;
    }

    void add (Block* b)
    {
        blocks.add (b);
        b->onHeightChanged = [this]
        {
            juce::Component::SafePointer<LearnArticle> safe (this);
            juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->relayout(); });
        };
        addAndMakeVisible (b);
    }

    LearnView& view;
    juce::OwnedArray<Block> blocks;
    juce::TextButton videoButton, doneButton, nextButton;
    int lesson = -1, footerY = 0, contentX = 0, contentWidth = 0, lastWidth = 0, headerH = 140;
};

// ============================================================================
//  LessonList: la lista de niveles y lecciones
// ============================================================================
class LessonList : public juce::Component
{
public:
    explicit LessonList (LearnView& v) : view (v) {}

    struct Row { bool header; int index; int y; int h; };

    int headerHeight() const { return (int) fs (Layout::compact() ? 50.0f : 42.0f); }
    int rowHeight() const    { return (int) fs (Layout::compact() ? 48.0f : 38.0f); }

    void layoutForWidth (int width)
    {
        rows.clear();
        int y = 6, lastLevel = -1;
        for (int i = 0; i < numLessons; ++i)
        {
            const int lv = levelOf (i);
            if (lv != lastLevel)
            {
                rows.push_back ({ true, lv, y, headerHeight() });
                y += headerHeight();
                lastLevel = lv;
            }
            rows.push_back ({ false, i, y, rowHeight() });
            y += rowHeight();
        }
        setSize (width, y + 12);
    }

    int yForLesson (int lesson) const
    {
        for (auto& r : rows)
            if (! r.header && r.index == lesson)
                return r.y;
        return 0;
    }

    void paint (juce::Graphics& g) override
    {
        const auto clip = g.getClipBounds();
        for (auto& row : rows)
        {
            if (row.y > clip.getBottom() || row.y + row.h < clip.getY())
                continue;
            auto r = juce::Rectangle<float> (8.0f, (float) row.y, (float) getWidth() - 16.0f, (float) row.h);
            if (row.header) paintHeader (g, row.index, r);
            else            paintLesson (g, row.index, r.reduced (0.0f, 2.0f));
        }
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown())
            return;
        for (auto& row : rows)
            if (e.y >= row.y && e.y < row.y + row.h)
            {
                int lesson = row.index;
                if (row.header)
                {
                    // Al tocar un nivel: su primera lección sin aprender
                    lesson = -1;
                    for (int i = 0; i < numLessons && lesson < 0; ++i)
                        if (levelOf (i) == row.index && ! view.isDone (i))
                            lesson = i;
                    for (int i = 0; i < numLessons && lesson < 0; ++i)
                        if (levelOf (i) == row.index)
                            lesson = i;
                }
                if (lesson >= 0)
                    view.showLesson (lesson);
                return;
            }
    }

private:
    void paintHeader (juce::Graphics& g, int level, juce::Rectangle<float> r)
    {
        int total = 0, done = 0;
        for (int i = 0; i < numLessons; ++i)
            if (levelOf (i) == level)
            {
                ++total;
                done += view.isDone (i) ? 1 : 0;
            }
        const auto c = levelColour (level);
        r.removeFromTop (r.getHeight() * 0.3f);
        auto numberArea = r.removeFromLeft (r.getHeight());
        g.setColour (c.withAlpha (done == total ? 0.9f : 0.22f));
        g.fillRoundedRectangle (numberArea.reduced (3.0f), 8.0f);
        g.setColour (done == total ? juce::Colour (0xff15110b) : c.brighter (0.4f));
        g.setFont (Fonts::bold (fs (13.0f)));
        g.drawText (done == total ? txt ("✓") : juce::String (level + 1), numberArea, juce::Justification::centred);
        r.removeFromLeft (10.0f);

        auto progressText = pro::isLearnLevelFree (level) ? juce::String (done) + "/" + juce::String (total) : txt ("PRO");
        g.setColour (Colours::textDim);
        g.setFont (Fonts::bold (fs (12.0f)));
        auto right = r.removeFromRight (fs (40.0f));
        g.drawText (progressText, right, juce::Justification::centredRight);

        g.setColour (Colours::text);
        g.setFont (Fonts::bold (fs (14.0f)));
        g.drawText (levelName (level), r.withTrimmedBottom (r.getHeight() * 0.25f), juce::Justification::centredLeft);
        auto bar = r.removeFromBottom (3.0f).withTrimmedRight (8.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (bar, 1.5f);
        if (done > 0)
            Draw::colourFill (g, bar.withWidth (bar.getWidth() * (float) done / (float) total), c, 1.5f);
    }

    void paintLesson (juce::Graphics& g, int lesson, juce::Rectangle<float> r)
    {
        const bool selected = lesson == view.currentLesson() && ! Layout::compact();
        const bool done = view.isDone (lesson);
        if (selected)
        {
            g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.28f), r.getX(), 0.0f, Colours::accent.withAlpha (0.05f), r.getRight(), 0.0f, false));
            g.fillRoundedRectangle (r, 9.0f);
        }
        else if (Layout::compact())
        {
            g.setColour (Colours::panelLight.withAlpha (0.35f));
            g.fillRoundedRectangle (r, 10.0f);
        }

        const float d = fs (22.0f);
        auto badge = juce::Rectangle<float> (d, d).withCentre ({ r.getX() + 10.0f + d * 0.5f, r.getCentreY() });
        const bool locked = ! pro::isLearnLevelFree (levelOf (lesson));
        if (locked)
        {
            g.setColour (Colours::accent.withAlpha (0.18f));
            g.fillEllipse (badge);
            drawLock (g, badge.reduced (d * 0.24f), Colours::accentLight);
        }
        else if (done)
        {
            Draw::colourFill (g, badge, Colours::green, d * 0.5f);
            juce::Path tick;
            tick.startNewSubPath (badge.getX() + d * 0.27f, badge.getCentreY() + d * 0.02f);
            tick.lineTo (badge.getX() + d * 0.45f, badge.getCentreY() + d * 0.2f);
            tick.lineTo (badge.getRight() - d * 0.25f, badge.getY() + d * 0.3f);
            g.setColour (juce::Colour (0xff0d2414));
            g.strokePath (tick, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        else
        {
            if (selected) Draw::accentFill (g, badge, d * 0.5f);
            else { g.setColour (Colours::panelLight); g.fillEllipse (badge); }
            g.setColour (selected ? juce::Colour (0xff1a1208) : Colours::textDim);
            g.setFont (Fonts::bold (fs (10.5f)));
            g.drawText (juce::String (lesson + 1), badge, juce::Justification::centred);
        }

        auto text = r.withTrimmedLeft (d + 22.0f).withTrimmedRight (8.0f);
        // Nota del test (si se hizo)
        const auto title = txt (learn::lessons[lesson].title);
        const int quizBest = view.getProgress().bestQuizScore (title);
        if (quizBest >= 0)
        {
            auto chipArea = text.removeFromRight (fs (34.0f)).withSizeKeepingCentre (fs (34.0f), fs (20.0f));
            g.setColour ((quizBest >= 3 ? Colours::green : Colours::blue).withAlpha (0.25f));
            g.fillRoundedRectangle (chipArea, chipArea.getHeight() * 0.5f);
            g.setColour (quizBest >= 3 ? Colours::green : Colours::blue.brighter (0.4f));
            g.setFont (Fonts::bold (fs (11.0f)));
            g.drawText (juce::String (quizBest) + "/3", chipArea, juce::Justification::centred);
        }
        if (Layout::compact())
        {
            auto chevron = text.removeFromRight (16.0f);
            juce::Path p;
            p.startNewSubPath (chevron.getX() + 4.0f, chevron.getCentreY() - 6.0f);
            p.lineTo (chevron.getX() + 10.0f, chevron.getCentreY());
            p.lineTo (chevron.getX() + 4.0f, chevron.getCentreY() + 6.0f);
            g.setColour (Colours::textDim);
            g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour (selected ? Colours::text : done ? Colours::text.withAlpha (0.8f) : Colours::text.withAlpha (0.88f));
        g.setFont (selected ? Fonts::bold (fs (14.0f)) : Fonts::regular (fs (14.0f)));
        g.drawFittedText (title, text.toNearestInt(), juce::Justification::centredLeft, 1, 0.85f);
    }

    LearnView& view;
    std::vector<Row> rows;
};

// ============================================================================
//  AchievementsPage: racha, estadísticas y medallas
// ============================================================================
class AchievementsPage : public juce::Component
{
public:
    explicit AchievementsPage (LearnView& v) : view (v) {}

    void layoutForWidth (int width)
    {
        const int margin = Layout::compact() ? 14 : 28;
        const int w = juce::jmin (900, width - margin * 2);
        const int cols = w < 520 ? 2 : w < 760 ? 3 : 4;
        const int medalH = (int) fs (150.0f);
        const int rowsN = ((int) learn::achievements().size() + cols - 1) / cols;
        setSize (width, juce::jmax (1, getHeight()));   // primero el ancho: statsHeight() depende de él
        setSize (width, statsHeight() + (int) fs (90.0f) + rowsN * (medalH + 12) + 30);
    }

    int statCols() const    { return getWidth() >= 700 ? 4 : 2; }
    int statsHeight() const { return statCols() == 4 ? (int) fs (Layout::compact() ? 112.0f : 140.0f) : (int) fs (230.0f); }

    void paint (juce::Graphics& g) override
    {
        auto& p = view.getProgress();
        const int margin = Layout::compact() ? 14 : 28;
        const float w = (float) juce::jmin (900, getWidth() - margin * 2);
        const float x = ((float) getWidth() - w) * 0.5f;
        float y = 18.0f;

        // --- Tarjetas de estadísticas -----------------------------------------------
        struct Stat { juce::String value, label; juce::Colour colour; int icon; };
        const Stat stats[] = {
            { juce::String (p.currentStreak()), p.currentStreak() == 1 ? txt ("día de racha") : txt ("días de racha"), Colours::accent, 10 },
            { juce::String (view.numDone()) + "/" + juce::String (numLessons), txt ("lecciones"), Colours::green, 1 },
            { juce::String (p.numChallengesDone()) + "/" + juce::String ((int) learn::challenges().size()), txt ("retos superados"), Colours::purple, 8 },
            { juce::String (p.numPerfectQuizzes()), txt ("tests perfectos"), Colours::blue, 6 },
        };
        const int cols_ = statCols();
        const float sw = (w - (float) (cols_ - 1) * 10.0f) / (float) cols_;
        const float sh = cols_ == 4 ? (float) statsHeight() : ((float) statsHeight() - 10.0f) * 0.5f;
        for (int i = 0; i < 4; ++i)
        {
            auto r = juce::Rectangle<float> (x + (float) (i % cols_) * (sw + 10.0f), y + (float) (i / cols_) * (sh + 10.0f), sw, sh);
            const auto& s = stats[i];
            g.setGradientFill (juce::ColourGradient (s.colour.withAlpha (0.22f), r.getX(), r.getY(), Colours::panel, r.getX(), r.getBottom(), false));
            g.fillRoundedRectangle (r, 14.0f);
            g.setColour (s.colour.withAlpha (0.5f));
            g.drawRoundedRectangle (r.reduced (0.5f), 14.0f, 1.2f);
            auto inner = r.reduced (16.0f, 10.0f);
            drawMedalIcon (g, s.icon, inner.removeFromTop (fs (24.0f)).withWidth (fs (24.0f)), s.colour.brighter (0.2f));
            g.setColour (Colours::text);
            g.setFont (Fonts::display (fs (28.0f)));
            g.drawText (s.value, inner.removeFromTop (inner.getHeight() * 0.58f), juce::Justification::bottomLeft);
            g.setColour (Colours::textDim);
            g.setFont (Fonts::regular (fs (13.0f)));
            g.drawText (s.label, inner, juce::Justification::centredLeft);
        }
        y += (float) statsHeight() + 20.0f;

        // Mensaje de la racha
        g.setColour (p.practicedToday() ? Colours::green : Colours::accentLight);
        g.setFont (Fonts::bold (fs (14.0f)));
        g.drawText (p.practicedToday() ? txt ("¡Hoy ya has practicado! Vuelve mañana para seguir la racha.")
                                       : txt ("Haz una lección, un test, un reto o un juego hoy para sumar a tu racha."),
                    juce::Rectangle<float> (x, y, w, fs (20.0f)), juce::Justification::centredLeft);
        y += fs (20.0f) + 6.0f;
        g.setColour (Colours::text);
        g.setFont (Fonts::bold (fs (17.0f)));
        g.drawText (txt ("Logros  ") + juce::String (p.numUnlocked()) + "/" + juce::String ((int) learn::achievements().size()),
                    juce::Rectangle<float> (x, y, w, fs (26.0f)), juce::Justification::centredLeft);
        y += fs (26.0f) + 10.0f;

        // --- Medallas --------------------------------------------------------------
        const int cols = w < 520 ? 2 : w < 760 ? 3 : 4;
        const float mw = (w - (float) (cols - 1) * 12.0f) / (float) cols;
        const float mh = fs (150.0f);
        int i = 0;
        for (auto& a : learn::achievements())
        {
            auto r = juce::Rectangle<float> (x + (float) (i % cols) * (mw + 12.0f), y + (float) (i / cols) * (mh + 12.0f), mw, mh);
            const bool unlocked = p.isUnlocked (a.id);
            const auto c = unlocked ? Colours::accent : Colours::textDim;
            g.setColour (unlocked ? Colours::panelLight : Colours::panel);
            g.fillRoundedRectangle (r, 14.0f);
            g.setColour (unlocked ? Colours::accent.withAlpha (0.6f) : Colours::outline);
            g.drawRoundedRectangle (r.reduced (0.5f), 14.0f, 1.2f);

            auto inner = r.reduced (12.0f, 12.0f);
            const float md = fs (54.0f);
            auto medal = inner.removeFromTop (md).withSizeKeepingCentre (md, md);
            if (unlocked)
            {
                Draw::glow (g, medal, Colours::accent, 8.0f, md * 0.5f);
                Draw::accentFill (g, medal, md * 0.5f);
            }
            else
            {
                g.setColour (Colours::panelLight);
                g.fillEllipse (medal);
            }
            drawMedalIcon (g, a.icon, medal.reduced (md * 0.2f), unlocked ? juce::Colour (0xff1a1208) : Colours::textDim.withAlpha (0.6f));
            inner.removeFromTop (8.0f);
            g.setColour (unlocked ? Colours::text : Colours::textDim);
            g.setFont (Fonts::bold (fs (14.0f)));
            g.drawFittedText (txt (a.title), inner.removeFromTop (fs (20.0f)).toNearestInt(), juce::Justification::centred, 1, 0.8f);
            g.setColour (Colours::textDim.withAlpha (unlocked ? 1.0f : 0.75f));
            g.setFont (Fonts::regular (fs (12.0f)));
            g.drawFittedText (txt (a.description), inner.toNearestInt(), juce::Justification::centredTop, 2, 0.85f);
            juce::ignoreUnused (c);
            ++i;
        }
    }

private:
    LearnView& view;
};

// ============================================================================
//  DictionaryPage: buscador de palabras y lecciones
// ============================================================================
class DictionaryPage : public juce::Component
{
public:
    explicit DictionaryPage (LearnView& v) : view (v)
    {
        addAndMakeVisible (search);
        search.setTextToShowWhenEmpty (txt ("Busca una palabra: 808, reverb, acorde, sidechain..."), Colours::textDim);
        search.setColour (juce::TextEditor::backgroundColourId, Colours::panelDark);
        search.setColour (juce::TextEditor::outlineColourId, Colours::outline);
        search.setColour (juce::TextEditor::focusedOutlineColourId, Colours::accent);
        search.setColour (juce::TextEditor::textColourId, Colours::text);
        search.setIndents (14, 0);
        search.onTextChange = [this] { rebuild(); };
        search.onEscapeKey = [this] { search.clear(); rebuild(); };

        addAndMakeVisible (viewport);
        viewport.setViewedComponent (&results, false);
        viewport.setScrollBarsShown (true, false);
        viewport.setScrollOnDragMode (juce::Viewport::ScrollOnDragMode::nonHover);
        rebuild();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (Layout::compact() ? 12 : 24, 12);
        search.setFont (Fonts::regular (fs (16.0f)));
        search.applyFontToAllText (Fonts::regular (fs (16.0f)));
        search.setJustification (juce::Justification::centredLeft);
        search.setBounds (area.removeFromTop (learn::buttonHeight() + 6).withWidth (juce::jmin (area.getWidth(), 820)));
        area.removeFromTop (10);
        viewport.setBounds (area);
        layoutResults();
    }

private:
    struct Result : public juce::Component
    {
        juce::String title, body, lessonTitle;
        int lessonIndex = -1;
        bool isLesson = false;
        juce::TextButton goButton;
        juce::TextLayout bodyLayout;

        int layoutFor (int width)
        {
            bodyLayout = makeLayout (body, fs (15.0f), Colours::text.withAlpha (0.88f), (float) width - 36.0f);
            goButton.setVisible (lessonIndex >= 0);
            return (int) fs (24.0f) + 20 + (int) bodyLayout.getHeight() + (lessonIndex >= 0 ? learn::buttonHeight() + 14 : 0) + 18;
        }
        void resized() override
        {
            const int bh = learn::buttonHeight();
            goButton.setBounds (18, getHeight() - bh - 14, juce::jmin (getWidth() - 36, 320), bh);
        }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
            g.setColour (Colours::panelLight.withAlpha (0.5f));
            g.fillRoundedRectangle (r, 12.0f);
            g.setColour (isLesson ? Colours::blue.withAlpha (0.5f) : Colours::outline);
            g.drawRoundedRectangle (r.reduced (0.5f), 12.0f, 1.0f);
            auto inner = r.reduced (18.0f, 12.0f);
            auto titleRow = inner.removeFromTop (fs (24.0f));
            if (isLesson)
            {
                auto chip = drawChip (g, txt ("LECCIÓN"), titleRow.getTopLeft().translated (0.0f, 2.0f), Colours::blue, fs (10.5f));
                titleRow.removeFromLeft (chip.getWidth() + 10.0f);
            }
            g.setColour (Colours::accentLight);
            g.setFont (Fonts::bold (fs (18.0f)));
            g.drawText (title, titleRow, juce::Justification::centredLeft);
            inner.removeFromTop (8.0f);
            bodyLayout.draw (g, inner.removeFromTop (bodyLayout.getHeight()));
        }
    };

    void rebuild()
    {
        resultList.clear();
        const auto q = normalise (search.getText().trim());

        for (auto& t : learn::glossary)
        {
            const auto word = txt (t.word), def = txt (t.definition);
            if (q.isNotEmpty() && ! normalise (word).contains (q) && ! normalise (def).contains (q))
                continue;
            auto* r = resultList.add (new Result());
            r->title = word;
            r->body = def;
            if (t.lesson != nullptr)
                for (int i = 0; i < numLessons; ++i)
                    if (txt (learn::lessons[i].title) == txt (t.lesson))
                        r->lessonIndex = i;
        }

        // Lecciones que hablan de lo que buscas
        if (q.length() >= 2)
            for (int i = 0; i < numLessons; ++i)
            {
                const auto title = txt (learn::lessons[i].title);
                const auto body = txt (learn::lessons[i].body);
                if (! normalise (title).contains (q) && ! normalise (body).contains (q))
                    continue;
                auto* r = resultList.add (new Result());
                r->isLesson = true;
                r->title = title;
                r->body = levelName (levelOf (i)) + txt (" · ") + txt (learn::lessons[i].minutes);
                r->lessonIndex = i;
            }

        for (auto* r : resultList)
        {
            r->goButton.setButtonText (r->isLesson ? txt ("Abrir la lección  →")
                                                   : txt ("Aprende más: ") + txt (learn::lessons[juce::jmax (0, r->lessonIndex)].title));
            learn::styleButton (r->goButton, r->isLesson ? learn::ButtonStyle::primary : learn::ButtonStyle::secondary, 13.5f);
            r->goButton.onClick = [this, index = r->lessonIndex]
            {
                if (index >= 0)
                {
                    view.showTab (LearnView::Tab::path);
                    view.showLesson (index);
                }
            };
            r->addAndMakeVisible (r->goButton);
            results.addAndMakeVisible (r);
        }
        layoutResults();
        repaint();
    }

    void layoutResults()
    {
        const int width = juce::jmax (200, viewport.getWidth() - viewport.getScrollBarThickness());
        const int w = juce::jmin (820, width);
        int y = 0;
        for (auto* r : resultList)
        {
            const int h = r->layoutFor (w);
            r->setBounds (0, y, w, h);
            y += h + 6;
        }
        results.setSize (width, juce::jmax (y + 20, 40));
    }

    void paint (juce::Graphics& g) override
    {
        if (resultList.isEmpty())
        {
            g.setColour (Colours::textDim);
            g.setFont (Fonts::regular (fs (15.0f)));
            g.drawText (txt ("No hay resultados. Prueba con otra palabra."), viewport.getBounds().withHeight (60), juce::Justification::centredLeft);
        }
    }

    LearnView& view;
    juce::TextEditor search;
    juce::Viewport viewport;
    juce::Component results;
    juce::OwnedArray<Result> resultList;
};

// ============================================================================
//  AchievementToast: el aviso de "¡Logro desbloqueado!"
// ============================================================================
class AchievementToast : public juce::Component, private juce::Timer
{
public:
    AchievementToast() { setInterceptsMouseClicks (false, false); }

    void show (const learn::Achievement& a)
    {
        title = txt (a.title);
        description = txt (a.description);
        icon = a.icon;
        shownAt = juce::Time::getMillisecondCounter();
        setVisible (true);
        toFront (false);
        startTimerHz (30);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto elapsed = (float) (juce::Time::getMillisecondCounter() - shownAt) / 1000.0f;
        const float alpha = juce::jlimit (0.0f, 1.0f, juce::jmin (elapsed * 5.0f, (4.0f - elapsed) * 2.0f));
        auto r = getLocalBounds().toFloat().reduced (6.0f);
        g.setOpacity (alpha);
        Draw::shadow (g, r, 16.0f, 16);
        g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.35f), r.getX(), r.getY(), Colours::panel, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 16.0f);
        g.setColour (Colours::panel.withAlpha (0.8f));
        g.fillRoundedRectangle (r.reduced (1.0f), 15.0f);
        g.setColour (Colours::accent.withAlpha (0.8f * alpha));
        g.drawRoundedRectangle (r.reduced (0.5f), 16.0f, 1.5f);
        auto inner = r.reduced (16.0f, 10.0f);
        auto medal = inner.removeFromLeft (inner.getHeight()).reduced (2.0f);
        Draw::accentFill (g, medal, medal.getWidth() * 0.5f);
        drawMedalIcon (g, icon, medal.reduced (medal.getWidth() * 0.2f), juce::Colour (0xff1a1208));
        inner.removeFromLeft (14.0f);
        g.setColour (Colours::accentLight);
        g.setFont (Fonts::bold (fs (12.0f)));
        g.drawText (txt ("¡LOGRO DESBLOQUEADO!"), inner.removeFromTop (inner.getHeight() * 0.33f), juce::Justification::bottomLeft);
        g.setColour (Colours::text);
        g.setFont (Fonts::bold (fs (17.0f)));
        g.drawText (title, inner.removeFromTop (inner.getHeight() * 0.5f), juce::Justification::centredLeft);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (fs (13.0f)));
        g.drawText (description, inner, juce::Justification::topLeft);
    }

private:
    void timerCallback() override
    {
        if (juce::Time::getMillisecondCounter() - shownAt > 4000)
        {
            stopTimer();
            setVisible (false);
        }
        repaint();
    }

    juce::String title, description;
    int icon = 0;
    juce::uint32 shownAt = 0;
};

// ============================================================================
//  LearnView
// ============================================================================
LearnView::LearnView (AppContext& c) : ctx (c), helpButton (txt (help::learnTitle), txt (help::learnText))
{
    progress.numLevels = numLevels;
    progress.isLevelComplete = [this] (int level)
    {
        bool any = false;
        for (int i = 0; i < numLessons; ++i)
            if (levelOf (i) == level)
            {
                any = true;
                if (! isDone (i))
                    return false;
            }
        return any;
    };

    article = std::make_unique<LearnArticle> (*this);
    lessonList = std::make_unique<LessonList> (*this);
    games = std::make_unique<EarGames> (ctx.engine, progress);
    achievementsPage = std::make_unique<AchievementsPage> (*this);
    dictionary = std::make_unique<DictionaryPage> (*this);
    toast = std::make_unique<AchievementToast>();

    addAndMakeVisible (helpButton);
    addAndMakeVisible (tutorialButton);
    tutorialButton.setButtonText (txt ("Tutorial guiado"));
    tutorialButton.setTooltip (txt ("Un recorrido guiado por Studoo, paso a paso, para hacer un beat desde cero."));
    tutorialButton.onClick = [this] { if (onStartTutorial) onStartTutorial(); };
    learn::styleButton (tutorialButton, learn::ButtonStyle::secondary, 13.0f);

    addChildComponent (backButton);
    backButton.setButtonText (txt ("←  Lecciones"));
    backButton.onClick = [this] { backToList(); };
    learn::styleButton (backButton, learn::ButtonStyle::secondary, 14.0f);

    const char* tabNames[] = { "Ruta", "Juegos", "Logros", "Diccionario" };
    for (int i = 0; i < 4; ++i)
    {
        auto* b = tabButtons.add (new juce::TextButton (txt (tabNames[i])));
        b->getProperties().set ("tab", true);
        b->onClick = [this, i] { showTab ((Tab) i); };
        b->getProperties().set ("fontSize", fs (14.5f));
        makeNonFocusing (*b);
        addAndMakeVisible (b);
    }

    for (auto* vp : { &listViewport, &articleViewport, &achievementsViewport })
    {
        vp->setScrollBarsShown (true, false);
        vp->setSingleStepSizes (0, 40);
        vp->setScrollOnDragMode (juce::Viewport::ScrollOnDragMode::nonHover);
        addChildComponent (vp);
    }
    listViewport.setViewedComponent (lessonList.get(), false);
    articleViewport.setViewedComponent (article.get(), false);
    achievementsViewport.setViewedComponent (achievementsPage.get(), false);
    addChildComponent (*games);
    addChildComponent (*dictionary);
    addChildComponent (*toast);

    progress.onChanged = [this]
    {
        lessonList->repaint();
        achievementsPage->repaint();
        repaint (statsArea);
    };
    juce::Component::SafePointer<LearnView> safe (this);
    progress.onUnlocked = [safe] (const learn::Achievement& a)
    {
        juce::MessageManager::callAsync ([safe, a]
        {
            if (safe == nullptr)
                return;
            safe->toast->show (a);
            safe->playCelebration();
        });
    };

    showLesson (0, false);
    showTab (Tab::path);
}

LearnView::~LearnView() = default;

void LearnView::setSettings (juce::PropertiesFile* s)
{
    progress.setSettings (s);
    // Volvemos a la primera lección sin terminar
    for (int i = 0; i < numLessons; ++i)
        if (! isDone (i) && pro::isLearnLevelFree (levelOf (i)))
        {
            showLesson (i, false);
            break;
        }
    lessonList->repaint();
    repaint();
}

bool LearnView::isDone (int lesson) const { return progress.isLessonDone (txt (learn::lessons[lesson].title)); }
void LearnView::setDone (int lesson, bool done) { progress.setLessonDone (txt (learn::lessons[lesson].title), done); }
int  LearnView::numDone() const { return progress.numLessonsDone(); }

void LearnView::playCelebration()
{
    // Un pequeño arpegio alegre (Do - Mi - Sol - Do)
    const double sr = 44100.0;
    const int noteLen = (int) (sr * 0.11);
    auto buffer = std::make_shared<juce::AudioBuffer<float>> (2, noteLen * 4 + (int) (sr * 0.5));
    buffer->clear();
    const int notes[] = { 72, 76, 79, 84 };
    for (int n = 0; n < 4; ++n)
    {
        const double f = 440.0 * std::pow (2.0, (notes[n] - 69) / 12.0);
        for (int i = 0; i < (int) (sr * 0.5); ++i)
        {
            const int pos = n * noteLen + i;
            if (pos >= buffer->getNumSamples()) break;
            const double t = i / sr;
            const float v = (float) (std::sin (juce::MathConstants<double>::twoPi * f * t) * std::exp (-t * 7.0) * std::min (1.0, t * 300.0) * 0.18);
            buffer->addSample (0, pos, v);
            buffer->addSample (1, pos, v);
        }
    }
    ctx.engine.playBuffer (buffer, sr, 0.8f);
}

void LearnView::showTab (Tab t)
{
    if ((t == Tab::games && ! pro::areGamesFree()) || (t == Tab::achievements && ! pro::areAchievementsFree()))
    {
        if (ctx.showPro)
            ctx.showPro (t == Tab::games ? txt ("Los juegos de oído son de Studoo Pro")
                                         : txt ("Los logros y la racha son de Studoo Pro"));
        t = tab;   // nos quedamos donde estábamos
    }
    tab = t;
    for (int i = 0; i < tabButtons.size(); ++i)
        tabButtons[i]->setToggleState (i == (int) t, juce::dontSendNotification);
    if (t != Tab::games)
        ctx.engine.stopBuffer();
    updateLayout();
}

void LearnView::showLesson (int lesson, bool openOnPhone)
{
    lesson = juce::jlimit (0, numLessons - 1, lesson);
    if (! pro::isLearnLevelFree (levelOf (lesson)))
    {
        if (openOnPhone && ctx.showPro)
            ctx.showPro (txt ("El nivel \"") + levelName (levelOf (lesson)) + txt ("\" es de Studoo Pro"));
        if (lessonIndex >= 0)
            return;
        lesson = 0;
    }
    lessonIndex = lesson;
    article->setLesson (lessonIndex);
    if (openOnPhone)
        phoneShowsArticle = true;
    updateLayout();
    articleViewport.setViewPosition (0, 0);
    lessonList->repaint();
    if (! Layout::compact())
    {
        const int y = lessonList->yForLesson (lessonIndex);
        const auto view_ = listViewport.getViewArea();
        if (y < view_.getY() || y > view_.getBottom() - 60)
            listViewport.setViewPosition (0, juce::jmax (0, y - 80));
    }
}

void LearnView::backToList()
{
    phoneShowsArticle = false;
    updateLayout();
    const int y = lessonList->yForLesson (lessonIndex);
    listViewport.setViewPosition (0, juce::jmax (0, y - 120));
}

void LearnView::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, Layout::compact() ? juce::String() : txt ("Aprende"));

    // Racha y progreso en la cabecera
    if (! statsArea.isEmpty())
    {
        auto s = statsArea.toFloat();
        const int streak = progress.currentStreak();
        auto flame = s.removeFromLeft (s.getHeight()).reduced (4.0f);
        drawMedalIcon (g, 10, flame, streak > 0 ? Colours::accent : Colours::textDim.withAlpha (0.6f));
        g.setColour (streak > 0 ? Colours::accentLight : Colours::textDim);
        g.setFont (Fonts::bold (fs (14.0f)));
        const auto streakText = juce::String (streak) + (Layout::compact() ? juce::String() : (streak == 1 ? txt (" día") : txt (" días")));
        auto streakArea = s.removeFromLeft (Layout::compact() ? fs (24.0f) : fs (64.0f));
        g.drawText (streakText, streakArea, juce::Justification::centredLeft);

        const int done = numDone();
        auto bar = s.withSizeKeepingCentre (s.getWidth(), 8.0f);
        const auto label = juce::String (done) + "/" + juce::String (numLessons);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::bold (fs (12.5f)));
        auto labelArea = bar.removeFromRight (fs (52.0f)).withHeight (s.getHeight()).withCentre ({ bar.getRight() + fs (26.0f), s.getCentreY() });
        g.drawText (label, labelArea, juce::Justification::centredRight);
        bar.removeFromRight (6.0f);
        if (bar.getWidth() < 50.0f)   // poco sitio (móvil): solo el número
            bar = {};
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillRoundedRectangle (bar, 4.0f);
        if (done > 0 && ! bar.isEmpty())
        {
            auto fill = bar.withWidth (bar.getWidth() * (float) done / (float) numLessons);
            Draw::glow (g, fill, Colours::green, 4.0f, 4.0f);
            Draw::colourFill (g, fill, Colours::green, 4.0f);
        }
    }

    if (tab == Tab::path && ! Layout::compact())
    {
        auto box = articleViewport.getBounds().toFloat();
        g.setColour (Colours::panelDark);
        g.fillRoundedRectangle (box, 10.0f);
        g.setColour (Colours::outline);
        g.drawRoundedRectangle (box.reduced (0.5f), 10.0f, 1.0f);
        auto listBox = listViewport.getBounds().toFloat();
        g.setColour (Colours::panel);
        g.fillRoundedRectangle (listBox, 10.0f);
        g.setColour (Colours::outline);
        g.drawRoundedRectangle (listBox.reduced (0.5f), 10.0f, 1.0f);
    }
}

void LearnView::resized()
{
    updateLayout();
}

void LearnView::updateLayout()
{
    const bool phone = Layout::compact();
    auto area = getLocalBounds();

    // Cabecera: ayuda, (volver), pestañas, racha y progreso, tutorial.
    // En el móvil va todo en una sola fila para dejar sitio a la lección.
    headerArea = area.removeFromTop (phone ? 56 : 46);
    auto h = headerArea.reduced (8, 6);
    helpButton.setBounds (h.removeFromLeft (26).withSizeKeepingCentre (26, 26));
    h.removeFromLeft (10);
    const bool showBack = phone && tab == Tab::path && phoneShowsArticle;
    backButton.setVisible (showBack);
    if (showBack)
    {
        backButton.setBounds (h.removeFromLeft (140));
        h.removeFromLeft (8);
    }
    else if (! phone)
    {
        h.removeFromLeft (90); // título "Aprende"
    }
    tutorialButton.setVisible (! phone);
    if (! phone)
        tutorialButton.setBounds (h.removeFromRight (170));
    h.removeFromRight (10);
    statsArea = h.removeFromRight (juce::jmin (phone ? 170 : 330, h.getWidth() / 3));
    h.removeFromRight (10);

    if (phone)
    {
        tabsArea = h;
    }
    else
    {
        tabsArea = area.removeFromTop (46).reduced (8, 4);
    }
    {
        auto t = tabsArea;
        const int w = phone ? t.getWidth() / 4 : juce::jmin (150, t.getWidth() / 4);
        tabButtons[1]->setButtonText (pro::areGamesFree() ? txt ("Juegos") : txt ("Juegos · Pro"));
        tabButtons[2]->setButtonText (pro::areAchievementsFree() ? txt ("Logros") : txt ("Logros · Pro"));
        for (auto* b : tabButtons)
        {
            b->setBounds (t.removeFromLeft (w).reduced (2, 0));
            b->getProperties().set ("fontSize", fs (phone ? 13.0f : 14.5f));
        }
    }

    contentArea = area.reduced (8, 6);
    listViewport.setVisible (false);
    articleViewport.setVisible (false);
    achievementsViewport.setVisible (false);
    games->setVisible (false);
    dictionary->setVisible (false);

    switch (tab)
    {
        case Tab::path:
        {
            if (phone)
            {
                auto& vp = phoneShowsArticle ? articleViewport : listViewport;
                vp.setVisible (true);
                vp.setBounds (contentArea);
            }
            else
            {
                auto c = contentArea;
                listViewport.setVisible (true);
                listViewport.setBounds (c.removeFromLeft (juce::jlimit (260, 340, c.getWidth() / 3)));
                c.removeFromLeft (8);
                articleViewport.setVisible (true);
                articleViewport.setBounds (c);
            }
            lessonList->layoutForWidth (juce::jmax (200, listViewport.getWidth() - listViewport.getScrollBarThickness()));
            article->layoutForWidth (juce::jmax (300, articleViewport.getWidth() - articleViewport.getScrollBarThickness()));
            break;
        }
        case Tab::games:
            games->setVisible (true);
            games->setBounds (contentArea);
            break;
        case Tab::achievements:
            achievementsViewport.setVisible (true);
            achievementsViewport.setBounds (contentArea);
            achievementsPage->layoutForWidth (juce::jmax (300, contentArea.getWidth() - achievementsViewport.getScrollBarThickness()));
            achievementsPage->repaint();
            break;
        case Tab::dictionary:
            dictionary->setVisible (true);
            dictionary->setBounds (contentArea);
            break;
    }

    toast->setBounds (getLocalBounds().withSizeKeepingCentre (juce::jmin (getWidth() - 24, 460), (int) fs (84.0f))
                                      .withY (headerArea.getBottom() + 8));
    repaint();
}
}
