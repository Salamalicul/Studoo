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

    addAndMakeVisible (text);
    text.setMultiLine (true, true);
    text.setReadOnly (true);
    text.setScrollbarsShown (true);
    text.setCaretVisible (false);
    text.setFont (juce::FontOptions (17.0f));
    text.setIndents (16, 14);
    text.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff22272c));
    makeNonFocusing (text);

    list.selectRow (0);
}

int LearnView::getNumRows()
{
    return (int) std::size (learn::topics);
}

void LearnView::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (selected)
    {
        g.setColour (Colours::accent.withAlpha (0.2f));
        g.fillRect (0, 0, width, height);
        g.setColour (Colours::accent);
        g.fillRect (0, 0, 3, height);
    }
    g.setColour (selected ? Colours::text : Colours::textDim);
    g.setFont (juce::FontOptions (14.0f, selected ? juce::Font::bold : juce::Font::plain));
    g.drawText (txt (learn::topics[row].title), 12, 0, width - 16, height, juce::Justification::centredLeft);
}

void LearnView::selectedRowsChanged (int row)
{
    if (! juce::isPositiveAndBelow (row, getNumRows()))
        return;
    text.setText (txt (learn::topics[row].title) + "\n\n" + txt (learn::topics[row].body), false);
    text.moveCaretToTop (false);
}

void LearnView::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, txt ("Aprende producción musical"));
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
    text.setBounds (area);
}
}
