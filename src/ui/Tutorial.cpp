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
    titleLabel.setFont (Fonts::display (22.0f));
    titleLabel.setColour (juce::Label::textColourId, Colours::accent);

    addAndMakeVisible (bodyLabel);
    bodyLabel.setFont (Fonts::regular (15.5f));
    bodyLabel.setColour (juce::Label::textColourId, Colours::text);
    bodyLabel.setJustificationType (juce::Justification::topLeft);

    addAndMakeVisible (progressLabel);
    progressLabel.setFont (Fonts::regular (12.0f));
    progressLabel.setColour (juce::Label::textColourId, Colours::textDim);

    addAndMakeVisible (doneLabel);
    doneLabel.setFont (Fonts::bold (15.0f));
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
            {
                // Solo la parte visible del control (por ejemplo, dentro de una zona con scroll)
                auto visible = getLocalArea (c, c->getLocalBounds());
                for (auto* p = c->getParentComponent(); p != nullptr && p != getParentComponent(); p = p->getParentComponent())
                    visible = visible.getIntersection (getLocalArea (p, p->getLocalBounds()));
                hole = visible.expanded (6).getIntersection (getLocalBounds());
            }

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
        b.setPosition (hole.getRight() - w - 18, hole.getY() + 18); // zona grande: arriba a la derecha, dentro
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
    // La burbuja siempre recibe sus clics; dentro del "agujero" los clics llegan al control de debajo
    if (bubble.contains (x, y))
        return true;
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
        for (int i = 1; i <= 5; ++i)
        {
            g.setColour (Colours::accent.withAlpha (0.22f / (float) i));
            g.drawRoundedRectangle (hole.toFloat().expanded ((float) i * 2.0f), 8.0f + (float) i * 2.0f, 2.0f);
        }
        g.setColour (Colours::accent);
        g.drawRoundedRectangle (hole.toFloat(), 8.0f, 2.5f);
    }

    auto b = bubble.toFloat();
    Draw::shadow (g, b, 14.0f, 22);
    Draw::glow (g, b, Colours::accent, 10.0f, 14.0f);
    g.setGradientFill (juce::ColourGradient (Colours::panelLight, 0.0f, b.getY(), Colours::panelDark, 0.0f, b.getBottom(), false));
    g.fillRoundedRectangle (b, 14.0f);
    g.setColour (Colours::accent);
    g.drawRoundedRectangle (b.reduced (0.5f), 14.0f, 1.5f);
    Draw::accentFill (g, b.withHeight (5.0f).reduced (14.0f, 0.0f), 2.5f);
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
