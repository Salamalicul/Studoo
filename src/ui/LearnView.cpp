#include "LearnView.h"
#include "HelpTexts.h"
#include "Illustrations.h"
#include "LearnTexts.h"
#include <iterator>

namespace studoo
{
namespace
{
    constexpr int numLessons = (int) std::size (learn::lessons);
    constexpr int numLevels = (int) std::size (learn::levels);
    constexpr int maxContentWidth = 860;

    int levelOf (int lesson)
    {
        const juce::String level = txt (learn::lessons[lesson].level);
        for (int i = 0; i < numLevels; ++i)
            if (level == txt (learn::levels[i]))
                return i;
        return 0;
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

    // ------------------------------------------------------------------------
    //  Bloques de una lección
    // ------------------------------------------------------------------------
    struct Block : public juce::Component
    {
        // Prepara el bloque para un ancho y devuelve su alto
        virtual int layoutFor (int width) = 0;
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

        float indent() const { return kind == Kind::bullet ? 26.0f : kind == Kind::step ? 46.0f : 0.0f; }

        int layoutFor (int width) override
        {
            juce::AttributedString s;
            s.setLineSpacing (kind == Kind::heading ? 0.0f : 5.0f);
            if (kind == Kind::heading)
                appendRich (s, content, 21.0f, Colours::text, Colours::accentLight);
            else
                appendRich (s, content, 16.0f, Colours::text.withAlpha (0.92f), Colours::accentLight);
            layout.createLayout (s, (float) width - indent());
            const int textH = (int) std::ceil (layout.getHeight());
            if (kind == Kind::heading) return textH + 34;
            if (kind == Kind::step)    return juce::jmax (36, textH + 14);
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
                auto mark = area.removeFromLeft (4.0f).withHeight (24.0f).translated (0.0f, 2.0f);
                Draw::accentFill (g, mark, 2.0f);
                area.removeFromLeft (12.0f);
                layout.draw (g, area.withHeight (layout.getHeight()).translated (0.0f, 2.0f));
                return;
            }
            if (kind == Kind::bullet)
            {
                auto dot = juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ 9.0f, 12.0f });
                Draw::accentFill (g, dot, 3.5f);
            }
            if (kind == Kind::step)
            {
                auto card = area.reduced (0.0f, 2.0f);
                g.setColour (Colours::panelLight.withAlpha (0.55f));
                g.fillRoundedRectangle (card, 10.0f);
                auto circle = juce::Rectangle<float> (28.0f, 28.0f).withCentre ({ 23.0f, card.getY() + 18.0f });
                Draw::glow (g, circle, Colours::accent, 4.0f, 14.0f);
                Draw::accentFill (g, circle, 14.0f);
                g.setColour (juce::Colour (0xff1a1208));
                g.setFont (Fonts::bold (14.0f));
                g.drawText (juce::String (number), circle, juce::Justification::centred);
                layout.draw (g, juce::Rectangle<float> (indent(), card.getY() + 7.0f, area.getWidth() - indent() - 10.0f, layout.getHeight()));
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
            juce::AttributedString s;
            s.setLineSpacing (4.0f);
            appendRich (s, content, 15.5f, Colours::text, colour().brighter (0.4f));
            layout.createLayout (s, (float) width - 40.0f);
            return (int) std::ceil (layout.getHeight()) + 52;
        }

        void paint (juce::Graphics& g) override
        {
            auto area = getLocalBounds().toFloat().reduced (0.0f, 4.0f);
            const auto c = colour();
            g.setGradientFill (juce::ColourGradient (c.withAlpha (0.16f), 0.0f, area.getY(), c.withAlpha (0.06f), 0.0f, area.getBottom(), false));
            g.fillRoundedRectangle (area, 10.0f);
            g.setColour (c.withAlpha (0.55f));
            g.drawRoundedRectangle (area.reduced (0.5f), 10.0f, 1.0f);
            Draw::colourFill (g, area.withWidth (4.0f).reduced (0.0f, 10.0f).translated (1.0f, 0.0f), c, 2.0f);

            auto inner = area.reduced (20.0f, 12.0f);
            auto title = inner.removeFromTop (20.0f);
            auto chip = title.withWidth ((float) juce::GlyphArrangement::getStringWidthInt (Fonts::bold (11.0f), label()) + 18.0f);
            g.setColour (c.withAlpha (0.25f));
            g.fillRoundedRectangle (chip, 9.0f);
            g.setColour (c.brighter (0.4f));
            g.setFont (Fonts::bold (11.0f));
            g.drawText (label(), chip, juce::Justification::centred);
            inner.removeFromTop (6.0f);
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
            return (int) illustrations::preferredHeight (id, (float) width - 40.0f) + 40 + (caption.isNotEmpty() ? 26 : 0);
        }

        void timerCallback() override
        {
            if (isShowing())
                repaint();
        }

        void paint (juce::Graphics& g) override
        {
            auto area = getLocalBounds().toFloat().reduced (0.0f, 6.0f);
            Draw::shadow (g, area, 12.0f, 12);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff16191e), 0.0f, area.getY(), juce::Colour (0xff111317), 0.0f, area.getBottom(), false));
            g.fillRoundedRectangle (area, 12.0f);
            g.setColour (Colours::outline);
            g.drawRoundedRectangle (area.reduced (0.5f), 12.0f, 1.0f);

            if (caption.isNotEmpty())
            {
                auto cap = area.removeFromBottom (30.0f);
                g.setColour (juce::Colours::black.withAlpha (0.25f));
                g.fillRect (cap.withTrimmedBottom (0.0f).reduced (1.0f, 0.0f));
                g.setColour (Colours::textDim);
                g.setFont (Fonts::regular (13.0f));
                g.drawFittedText (caption, cap.reduced (14.0f, 0.0f).toNearestInt(), juce::Justification::centred, 2, 1.0f);
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
            button.setColour (juce::TextButton::buttonColourId, Colours::accent.withAlpha (0.85f));
            button.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff1a1208));
            button.setColour (juce::TextButton::textColourOnId, juce::Colour (0xff1a1208));
            makeNonFocusing (button);
            addAndMakeVisible (button);
        }
        int layoutFor (int) override { return 52; }
        void resized() override
        {
            const int w = juce::GlyphArrangement::getStringWidthInt (Fonts::bold (14.0f), button.getButtonText()) + 60;
            button.setBounds (0, 8, w, 36);
        }
    };

    struct SpacerBlock : public Block
    {
        int h;
        explicit SpacerBlock (int height) : h (height) {}
        int layoutFor (int) override { return h; }
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
        {
            makeNonFocusing (*b);
            addAndMakeVisible (*b);
        }
        videoButton.setTooltip (txt ("Abre YouTube con vídeos en español sobre este tema."));
        doneButton.setTooltip (txt ("Marca esta lección como aprendida. Tu progreso se guarda."));
        doneButton.setClickingTogglesState (false);
        doneButton.setColour (juce::TextButton::buttonOnColourId, Colours::green);
        nextButton.setColour (juce::TextButton::buttonColourId, Colours::accent.withAlpha (0.85f));
        nextButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff1a1208));
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

        videoButton.setButtonText (txt ("Ver vídeos en YouTube"));
        nextButton.setButtonText (index + 1 < numLessons ? txt ("Siguiente lección  →") : txt ("¡Ruta completada!"));
        nextButton.setEnabled (index + 1 < numLessons);
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
        doneButton.setToggleState (done, juce::dontSendNotification);
        doneButton.setButtonText (done ? txt ("Aprendida  ✓") : txt ("Marcar como aprendida"));
        repaint();
    }

    void layoutForWidth (int width)
    {
        const int contentW = juce::jmin (maxContentWidth, width - 72);
        const int x = (width - contentW) / 2;
        int y = headerHeight;
        for (auto* b : blocks)
        {
            const int h = b->layoutFor (contentW);
            b->setBounds (x, y, contentW, h);
            y += h + 8;
        }
        y += 20;
        footerY = y;
        videoButton.setBounds (x, y + 20, 210, 38);
        doneButton.setBounds (x + 220, y + 20, 210, 38);
        nextButton.setBounds (x + contentW - 210, y + 20, 210, 38);
        contentX = x;
        contentWidth = contentW;
        setSize (width, y + 90);
    }

    void paint (juce::Graphics& g) override
    {
        if (lesson < 0)
            return;
        const auto& l = learn::lessons[lesson];
        const float x = (float) contentX, w = (float) contentWidth;

        // Nivel, número y duración
        const auto level = txt (l.level);
        const auto levelIdx = levelOf (lesson);
        const juce::Colour levelColours[] = { Colours::accent, Colours::accentLight, Colours::blue, Colours::purple,
                                              Colours::green, Colours::teal, Colours::red };
        const auto lc = levelColours[levelIdx % 7];
        auto chip = juce::Rectangle<float> (x, 30.0f, (float) juce::GlyphArrangement::getStringWidthInt (Fonts::bold (12.0f), level) + 24.0f, 24.0f);
        g.setColour (lc.withAlpha (0.18f));
        g.fillRoundedRectangle (chip, 12.0f);
        g.setColour (lc.brighter (0.3f));
        g.setFont (Fonts::bold (12.0f));
        g.drawText (level, chip, juce::Justification::centred);

        const auto meta = txt ("Lección ") + juce::String (lesson + 1) + txt (" de ") + juce::String (numLessons)
                        + "   ·   " + txt (l.minutes);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (13.0f));
        g.drawText (meta, juce::Rectangle<float> (chip.getRight() + 14.0f, chip.getY(), 300.0f, chip.getHeight()), juce::Justification::centredLeft);

        if (view.isDone (lesson))
        {
            auto done = juce::Rectangle<float> (x + w - 110.0f, chip.getY(), 110.0f, 24.0f);
            g.setColour (Colours::green.withAlpha (0.18f));
            g.fillRoundedRectangle (done, 12.0f);
            g.setColour (Colours::green);
            g.setFont (Fonts::bold (12.0f));
            g.drawText (txt ("✓ Aprendida"), done, juce::Justification::centred);
        }

        g.setColour (Colours::text);
        g.setFont (Fonts::display (32.0f));
        g.drawFittedText (txt (l.title), juce::Rectangle<float> (x, 64.0f, w, 46.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
        g.setGradientFill (juce::ColourGradient (Colours::accent, x, 0.0f, Colours::accent.withAlpha (0.0f), x + w * 0.8f, 0.0f, false));
        g.fillRect (x, 118.0f, w, 2.0f);

        // Pie: línea separadora
        g.setColour (Colours::outline);
        g.drawHorizontalLine (footerY, x, x + w);
    }

private:
    void add (Block* b)
    {
        blocks.add (b);
        addAndMakeVisible (b);
    }

    static constexpr int headerHeight = 140;
    LearnView& view;
    juce::OwnedArray<Block> blocks;
    juce::TextButton videoButton, doneButton, nextButton;
    int lesson = -1, footerY = 0, contentX = 0, contentWidth = 0;
};

// ============================================================================
//  LearnView
// ============================================================================
LearnView::LearnView() : helpButton (txt (help::learnTitle), txt (help::learnText))
{
    article = std::make_unique<LearnArticle> (*this);

    addAndMakeVisible (helpButton);

    addAndMakeVisible (tutorialButton);
    tutorialButton.setButtonText (txt ("Tutorial guiado: tu primer beat"));
    tutorialButton.setTooltip (txt ("Un recorrido guiado por Studoo, paso a paso, para hacer un beat desde cero."));
    tutorialButton.onClick = [this] { if (onStartTutorial) onStartTutorial(); };
    makeNonFocusing (tutorialButton);

    // Filas de la lista: cabecera de cada nivel y sus lecciones
    int lastLevel = -1;
    for (int i = 0; i < numLessons; ++i)
    {
        const int lv = levelOf (i);
        if (lv != lastLevel)
        {
            rows.add ({ true, lv });
            lastLevel = lv;
        }
        rows.add ({ false, i });
    }

    addAndMakeVisible (list);
    list.setRowHeight (32);
    list.setColour (juce::ListBox::backgroundColourId, Colours::panel);
    list.setColour (juce::ListBox::outlineColourId, Colours::outline);
    list.setOutlineThickness (1);
    list.setWantsKeyboardFocus (false);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (article.get(), false);
    viewport.setScrollBarsShown (true, false);
    viewport.setSingleStepSizes (0, 40);

    showLesson (0);
}

LearnView::~LearnView() = default;

void LearnView::setSettings (juce::PropertiesFile* s)
{
    settings = s;
    // Volvemos a la primera lección sin terminar
    for (int i = 0; i < numLessons; ++i)
        if (! isDone (i))
        {
            showLesson (i);
            break;
        }
    list.repaint();
    repaint();
}

bool LearnView::isDone (int lesson) const
{
    if (settings == nullptr)
        return false;
    auto done = juce::StringArray::fromTokens (settings->getValue ("learnDone"), "|", {});
    return done.contains (txt (learn::lessons[lesson].title));
}

void LearnView::setDone (int lesson, bool done)
{
    if (settings == nullptr)
        return;
    auto list_ = juce::StringArray::fromTokens (settings->getValue ("learnDone"), "|", {});
    const auto title = txt (learn::lessons[lesson].title);
    list_.removeString (title);
    if (done)
        list_.add (title);
    list_.removeEmptyStrings();
    settings->setValue ("learnDone", list_.joinIntoString ("|"));
    settings->saveIfNeeded();
    list.repaint();
    repaint (progressArea);
}

int LearnView::numDone() const
{
    int n = 0;
    for (int i = 0; i < numLessons; ++i)
        if (isDone (i))
            ++n;
    return n;
}

int LearnView::rowForLesson (int lesson) const
{
    for (int r = 0; r < rows.size(); ++r)
        if (! rows[r].header && rows[r].index == lesson)
            return r;
    return 0;
}

void LearnView::showLesson (int lesson)
{
    currentLesson = lesson;
    article->setLesson (lesson);
    article->layoutForWidth (juce::jmax (300, viewport.getWidth() - viewport.getScrollBarThickness()));
    viewport.setViewPosition (0, 0);
    const int row = rowForLesson (lesson);
    if (list.getSelectedRow() != row)
        list.selectRow (row);
    list.scrollToEnsureRowIsOnscreen (row);
}

int LearnView::getNumRows()
{
    return rows.size();
}

void LearnView::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, rows.size()))
        return;
    const auto& entry = rows.getReference (row);
    auto r = juce::Rectangle<float> (4.0f, 1.0f, (float) width - 8.0f, (float) height - 2.0f);

    if (entry.header)
    {
        g.setColour (Colours::accentLight);
        g.setFont (Fonts::bold (12.0f));
        g.drawText (txt (learn::levels[entry.index]), r.withTrimmedLeft (8.0f).withTrimmedTop (8.0f),
                    juce::Justification::centredLeft);
        return;
    }

    if (selected)
    {
        g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.28f), r.getX(), 0.0f,
                                                 Colours::accent.withAlpha (0.05f), r.getRight(), 0.0f, false));
        g.fillRoundedRectangle (r, 7.0f);
    }

    const bool done = isDone (entry.index);
    auto badge = juce::Rectangle<float> (r.getX() + 10.0f, r.getCentreY() - 10.0f, 20.0f, 20.0f);
    if (done)
    {
        Draw::colourFill (g, badge, Colours::green, 10.0f);
        juce::Path tick;
        tick.startNewSubPath (badge.getX() + 5.5f, badge.getCentreY() + 0.5f);
        tick.lineTo (badge.getX() + 9.0f, badge.getCentreY() + 4.0f);
        tick.lineTo (badge.getRight() - 5.0f, badge.getY() + 6.0f);
        g.setColour (juce::Colour (0xff0d2414));
        g.strokePath (tick, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else
    {
        if (selected) Draw::accentFill (g, badge, 10.0f);
        else { g.setColour (Colours::panelLight); g.fillEllipse (badge); }
        g.setColour (selected ? juce::Colour (0xff1a1208) : Colours::textDim);
        g.setFont (Fonts::bold (10.0f));
        g.drawText (juce::String (entry.index + 1), badge, juce::Justification::centred);
    }

    g.setColour (selected ? Colours::text : done ? Colours::text.withAlpha (0.75f) : Colours::textDim.brighter (0.15f));
    g.setFont (selected ? Fonts::bold (13.5f) : Fonts::regular (13.5f));
    g.drawFittedText (txt (learn::lessons[entry.index].title), r.withTrimmedLeft (40.0f).withTrimmedRight (4.0f).toNearestInt(),
                      juce::Justification::centredLeft, 1, 0.85f);
}

void LearnView::selectedRowsChanged (int row)
{
    if (! juce::isPositiveAndBelow (row, rows.size()))
        return;
    const auto entry = rows[row];
    if (entry.header)
    {
        // Una cabecera de nivel: abrimos su primera lección
        if (row + 1 < rows.size())
            list.selectRow (row + 1);
        return;
    }
    if (entry.index != currentLesson)
        showLesson (entry.index);
}

void LearnView::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, txt ("Aprende producción musical"));

    // Barra de progreso
    if (! progressArea.isEmpty())
    {
        const int done = numDone();
        auto p = progressArea.toFloat();
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (12.5f));
        g.drawText (txt ("Tu progreso: ") + juce::String (done) + " / " + juce::String (numLessons),
                    p.removeFromLeft (130.0f), juce::Justification::centredLeft);
        auto bar = p.withSizeKeepingCentre (p.getWidth(), 8.0f);
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillRoundedRectangle (bar, 4.0f);
        if (done > 0)
        {
            auto fill = bar.withWidth (bar.getWidth() * (float) done / (float) numLessons);
            Draw::glow (g, fill, Colours::green, 4.0f, 4.0f);
            Draw::colourFill (g, fill, Colours::green, 4.0f);
        }
    }

    auto box = viewport.getBounds().toFloat();
    g.setColour (Colours::panelDark);
    g.fillRoundedRectangle (box, 9.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (box.reduced (0.5f), 9.0f, 1.0f);
}

void LearnView::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (42);
    auto h = headerArea.reduced (8, 7);
    helpButton.setBounds (h.removeFromLeft (26));
    tutorialButton.setBounds (h.removeFromRight (250));
    h.removeFromRight (20);
    h.removeFromLeft (240);
    progressArea = h.removeFromRight (juce::jmin (380, h.getWidth()));

    area.reduce (8, 8);
    list.setBounds (area.removeFromLeft (280));
    area.removeFromLeft (8);
    viewport.setBounds (area);
    article->layoutForWidth (juce::jmax (300, viewport.getWidth() - viewport.getScrollBarThickness()));
}
}
