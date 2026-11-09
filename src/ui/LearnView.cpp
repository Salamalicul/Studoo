#include "LearnView.h"
#include "HelpTexts.h"
#include "LearnTexts.h"
#include <iterator>

namespace studoo
{
LearnView::LearnView() : helpButton (txt (help::learnTitle), txt (help::learnText))
{
    addAndMakeVisible (helpButton);

    addAndMakeVisible (tutorialButton);
    tutorialButton.setButtonText (txt ("Empezar tutorial: tu primer beat"));
    tutorialButton.setTooltip (txt ("Un recorrido guiado, paso a paso, para hacer un beat desde cero."));
    tutorialButton.onClick = [this] { if (onStartTutorial) onStartTutorial(); };
    makeNonFocusing (tutorialButton);

    addAndMakeVisible (list);
    list.setRowHeight (34);
    list.setColour (juce::ListBox::backgroundColourId, Colours::panel);
    list.setColour (juce::ListBox::outlineColourId, Colours::outline);
    list.setOutlineThickness (1);
    list.setWantsKeyboardFocus (false);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&article, false);
    viewport.setScrollBarsShown (true, false);

    list.selectRow (0);
}

int LearnView::getNumRows()
{
    return (int) std::size (learn::topics);
}

void LearnView::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    auto r = juce::Rectangle<float> (4.0f, 2.0f, (float) width - 8.0f, (float) height - 4.0f);
    if (selected)
    {
        g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.28f), r.getX(), 0.0f,
                                                 Colours::accent.withAlpha (0.05f), r.getRight(), 0.0f, false));
        g.fillRoundedRectangle (r, 7.0f);
    }
    auto badge = juce::Rectangle<float> (r.getX() + 8.0f, r.getCentreY() - 10.0f, 20.0f, 20.0f);
    if (selected) Draw::accentFill (g, badge, 10.0f);
    else { g.setColour (Colours::panelLight); g.fillEllipse (badge); }
    g.setColour (selected ? juce::Colour (0xff1a1208) : Colours::textDim);
    g.setFont (Fonts::bold (10.5f));
    g.drawText (juce::String (row + 1), badge, juce::Justification::centred);
    g.setColour (selected ? Colours::text : Colours::textDim);
    g.setFont (selected ? Fonts::bold (14.0f) : Fonts::regular (14.0f));
    g.drawText (txt (learn::topics[row].title), r.withTrimmedLeft (38.0f), juce::Justification::centredLeft);
}

void LearnView::selectedRowsChanged (int row)
{
    if (! juce::isPositiveAndBelow (row, getNumRows()))
        return;
    article.setArticle (txt (learn::topics[row].title), txt (learn::topics[row].body), row + 1);
    article.layoutForWidth (viewport.getWidth() - viewport.getScrollBarThickness());
    viewport.setViewPosition (0, 0);
}

void LearnView::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, txt ("Aprende producción musical"));
    auto box = viewport.getBounds().toFloat();
    g.setColour (Colours::panelDark);
    g.fillRoundedRectangle (box, 9.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (box.reduced (0.5f), 9.0f, 1.0f);
}

// ============================================================================
//  LearnArticle
// ============================================================================
void LearnArticle::setArticle (const juce::String& t, const juce::String& b, int n)
{
    title = t;
    body = b;
    number = n;
}

void LearnArticle::layoutForWidth (int width)
{
    const float textWidth = (float) juce::jmax (200, width - 72);
    juce::AttributedString s;
    s.setLineSpacing (5.0f);
    auto lines = juce::StringArray::fromLines (body);
    for (int i = 0; i < lines.size(); ++i)
    {
        auto line = lines[i];
        const bool bullet = line.startsWith ("- ");
        const bool heading = line.endsWithChar (':') && line.length() < 60;
        if (bullet)
        {
            s.append (juce::String::fromUTF8 ("\xe2\x97\x8f  "), juce::Font (Fonts::bold (13.0f)), Colours::accent);
            line = line.substring (2);
        }
        s.append (line + (i < lines.size() - 1 ? "\n" : ""), juce::Font (heading ? Fonts::bold (17.0f) : Fonts::regular (17.0f)),
                  heading ? Colours::accentLight : Colours::text);
    }
    layout.createLayout (s, textWidth);
    setSize (width, (int) layout.getHeight() + 140);
    repaint();
}

void LearnArticle::paint (juce::Graphics& g)
{
    // Título grande con su número
    auto badge = juce::Rectangle<float> (32.0f, 30.0f, 40.0f, 40.0f);
    Draw::glow (g, badge, Colours::accent, 8.0f, 20.0f);
    Draw::accentFill (g, badge, 20.0f);
    g.setColour (juce::Colour (0xff1a1208));
    g.setFont (Fonts::display (20.0f));
    g.drawText (juce::String (number), badge, juce::Justification::centred);

    g.setColour (Colours::text);
    g.setFont (Fonts::display (30.0f));
    g.drawText (title, juce::Rectangle<float> (88.0f, 26.0f, (float) getWidth() - 110.0f, 48.0f), juce::Justification::centredLeft);
    g.setGradientFill (juce::ColourGradient (Colours::accent, 32.0f, 0.0f, Colours::accent.withAlpha (0.0f), (float) getWidth() * 0.7f, 0.0f, false));
    g.fillRect (32.0f, 88.0f, (float) getWidth() - 64.0f, 2.0f);

    layout.draw (g, juce::Rectangle<float> (36.0f, 110.0f, (float) getWidth() - 72.0f, layout.getHeight()));
}

void LearnView::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (42);
    auto h = headerArea.reduced (8, 7);
    helpButton.setBounds (h.removeFromLeft (26));
    tutorialButton.setBounds (h.removeFromRight (260));

    area.reduce (8, 8);
    list.setBounds (area.removeFromLeft (260));
    area.removeFromLeft (8);
    viewport.setBounds (area);
    article.layoutForWidth (viewport.getWidth() - viewport.getScrollBarThickness());
}
}
