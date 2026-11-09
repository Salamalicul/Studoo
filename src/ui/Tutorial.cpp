#include "Tutorial.h"
#include "StudooLookAndFeel.h"
#include "Widgets.h"

namespace studoo
{
TutorialOverlay::TutorialOverlay()
{
    setInterceptsMouseClicks (true, true);
    setAlwaysOnTop (true);

    for (auto* b : { &nextButton, &backButton, &skipButton })
    {
        addAndMakeVisible (*b);
        makeNonFocusing (*b);
    }
    nextButton.setButtonText (txt ("Siguiente"));
    nextButton.setColour (juce::TextButton::buttonColourId, Colours::accent);
    nextButton.setColour (juce::TextButton::textColourOffId, juce::Colours::black);
    nextButton.onClick = [this] { goTo (current + 1); };
    backButton.setButtonText (txt ("Atrás"));
    backButton.onClick = [this] { goTo (current - 1); };
    skipButton.setButtonText (txt ("Salir del tutorial"));
    skipButton.onClick = [this] { stop(); };

    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::FontOptions (19.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, Colours::accent);

    addAndMakeVisible (bodyLabel);
    bodyLabel.setFont (juce::FontOptions (15.0f));
    bodyLabel.setColour (juce::Label::textColourId, Colours::text);
    bodyLabel.setJustificationType (juce::Justification::topLeft);

    addAndMakeVisible (progressLabel);
    progressLabel.setFont (juce::FontOptions (12.0f));
    progressLabel.setColour (juce::Label::textColourId, Colours::textDim);

    addAndMakeVisible (doneLabel);
    doneLabel.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    doneLabel.setColour (juce::Label::textColourId, Colours::green);
}

void TutorialOverlay::start (std::vector<TutorialStep> newSteps)
{
    steps = std::move (newSteps);
    running = true;
    setVisible (true);
    toFront (false);
    goTo (0);
    startTimerHz (5);
}

void TutorialOverlay::stop()
{
    running = false;
    stopTimer();
    setVisible (false);
    if (onFinished)
        onFinished();
}

void TutorialOverlay::goTo (int index)
{
    if (index >= (int) steps.size())
    {
        stop();
        return;
    }

    current = juce::jmax (0, index);
    done = false;
    auto& step = steps[(size_t) current];
    if (step.onEnter)
        step.onEnter();

    titleLabel.setText (step.title, juce::dontSendNotification);
    bodyLabel.setText (step.text, juce::dontSendNotification);
    progressLabel.setText (txt ("Paso ") + juce::String (current + 1) + " de " + juce::String ((int) steps.size()),
                           juce::dontSendNotification);
    doneLabel.setText ({}, juce::dontSendNotification);
    backButton.setEnabled (current > 0);
    nextButton.setButtonText (current == (int) steps.size() - 1 ? txt ("¡Terminar!") : txt ("Siguiente"));
    nextButton.setEnabled (step.isDone == nullptr);
    updateLayout();
}

void TutorialOverlay::timerCallback()
{
    if (! running)
        return;

    auto& step = steps[(size_t) current];
    if (step.isDone != nullptr && ! done && step.isDone())
    {
        done = true;
        doneLabel.setText (txt ("¡Muy bien! Pulsa Siguiente."), juce::dontSendNotification);
        nextButton.setEnabled (true);
    }
    updateLayout(); // por si el control se movió
}

void TutorialOverlay::updateLayout()
{
    auto old = hole;
    hole = {};
    if (running)
        if (auto& step = steps[(size_t) current]; step.target != nullptr)
            if (auto* c = step.target(); c != nullptr && c->isShowing())
                hole = getLocalArea (c, c->getLocalBounds()).expanded (6);

    // La burbuja se coloca debajo del control resaltado, o encima si no cabe
    const int w = juce::jmin (480, getWidth() - 40);
    const int h = 250;
    juce::Rectangle<int> b (0, 0, w, h);
    if (hole.isEmpty())
        b.setCentre (getLocalBounds().getCentre());
    else if (hole.getBottom() + h + 16 < getHeight())
        b.setPosition (hole.getCentreX() - w / 2, hole.getBottom() + 14);
    else if (hole.getY() - h - 14 > 0)
        b.setPosition (hole.getCentreX() - w / 2, hole.getY() - h - 14);
    else
        b.setPosition (getWidth() - w - 20, 20);
    b = b.constrainedWithin (getLocalBounds().reduced (12));

    if (b != bubble || old != hole)
    {
        bubble = b;
        resized();
        repaint();
    }
}

bool TutorialOverlay::hitTest (int x, int y)
{
    // Dentro del "agujero" los clics llegan al control de debajo
    return ! hole.contains (x, y);
}

void TutorialOverlay::paint (juce::Graphics& g)
{
    juce::Path dim;
    dim.addRectangle (getLocalBounds().toFloat());
    if (! hole.isEmpty())
    {
        dim.setUsingNonZeroWinding (false);
        dim.addRoundedRectangle (hole.toFloat(), 6.0f);
    }
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillPath (dim);

    if (! hole.isEmpty())
    {
        g.setColour (Colours::accent);
        g.drawRoundedRectangle (hole.toFloat(), 6.0f, 3.0f);
    }

    g.setColour (Colours::panel);
    g.fillRoundedRectangle (bubble.toFloat(), 10.0f);
    g.setColour (Colours::accent);
    g.drawRoundedRectangle (bubble.toFloat(), 10.0f, 2.0f);
}

void TutorialOverlay::resized()
{
    auto b = bubble.reduced (18, 14);
    titleLabel.setBounds (b.removeFromTop (28));
    auto bottom = b.removeFromBottom (32);
    progressLabel.setBounds (bottom.removeFromLeft (90));
    nextButton.setBounds (bottom.removeFromRight (110));
    bottom.removeFromRight (6);
    backButton.setBounds (bottom.removeFromRight (80));
    bottom.removeFromRight (6);
    skipButton.setBounds (bottom.removeFromRight (140));
    doneLabel.setBounds (b.removeFromBottom (24));
    bodyLabel.setBounds (b);
}
}
