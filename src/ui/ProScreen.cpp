#include "ProScreen.h"
#include "LearnStyle.h"
#include "Pro.h"

namespace studoo
{
using learn::fs;

namespace
{
    const char* benefits[] = {
        "Los 19 kits de batería y los efectos FX",
        "Todos los sonidos de sintetizador y de 808",
        "Las 11 bases de géneros (drill, techno, phonk...)",
        "Aprende completo: niveles 5 a 9, juegos de oído y logros",
        "Exportar canciones de cualquier duración",
        "Un solo pago, para siempre. Sin anuncios ni suscripciones",
    };

    struct Donation { const char* label; };
    const Donation donations[] = { { "Un café" }, { "Un bocadillo" }, { "Una pizza" } };
}

ProScreen::ProScreen()
{
    setAlwaysOnTop (false);
    buyButton.onClick = [] { pro::buy (pro::proProduct); };
    restoreButton.onClick = [this]
    {
        pro::refresh();
        restoreButton.setButtonText (txt ("Buscando tu compra..."));
    };
    closeButton.setButtonText (txt ("Ahora no"));
    closeButton.onClick = [this] { hide(); };
    for (auto* b : { &buyButton, &restoreButton, &closeButton })
        addAndMakeVisible (b);

    for (int i = 0; i < 3; ++i)
    {
        auto* b = donateButtons.add (new juce::TextButton());
        b->onClick = [i] { pro::buy (pro::donationProducts[i]); };
        addAndMakeVisible (b);
    }
    refreshButtons();
}

void ProScreen::show (const juce::String& why)
{
    reason = why;
    pro::refresh();
    refreshButtons();
    setVisible (true);
    toFront (true);
    startTimer (300);
    resized();
    repaint();
}

void ProScreen::hide()
{
    stopTimer();
    setVisible (false);
}

void ProScreen::timerCallback()
{
    pro::poll();
    refreshButtons();
    repaint();
}

void ProScreen::refreshButtons()
{
    const bool unlocked = pro::isUnlocked();
    if (unlocked != wasUnlocked)
    {
        wasUnlocked = unlocked;
        juce::Component::SafePointer<ProScreen> safe (this);
        juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->resized(); });
    }
    const auto proPrice = pro::price (pro::proProduct);
    buyButton.setButtonText (proPrice.isNotEmpty() ? txt ("Desbloquear Pro  ·  ") + proPrice : txt ("Desbloquear Studoo Pro"));
    buyButton.setVisible (! unlocked);
    if (! restoreButton.getButtonText().startsWith (txt ("Buscando")) || unlocked)
        restoreButton.setButtonText (txt ("Ya lo compré: restaurar"));
    restoreButton.setVisible (! unlocked);
    learn::styleButton (buyButton, learn::ButtonStyle::primary, 16.0f);
    learn::styleButton (restoreButton, learn::ButtonStyle::secondary, 13.5f);
    learn::styleButton (closeButton, learn::ButtonStyle::secondary, 13.5f);
    closeButton.setButtonText (unlocked ? txt ("Cerrar") : txt ("Ahora no"));

    for (int i = 0; i < donateButtons.size(); ++i)
    {
        const auto p = pro::price (pro::donationProducts[i]);
        donateButtons[i]->setButtonText (txt (donations[i].label) + (p.isNotEmpty() ? txt ("  ·  ") + p : juce::String()));
        learn::styleButton (*donateButtons[i], learn::ButtonStyle::secondary, 13.5f);
    }
}

juce::Rectangle<int> ProScreen::cardBounds() const
{
    const bool wide = getWidth() >= 720;
    const int w = juce::jmin (getWidth() - 24, wide ? 940 : 560);
    const int h = juce::jmin (getHeight() - 24, wide ? 470 : 760);
    return getLocalBounds().withSizeKeepingCentre (w, h);
}

void ProScreen::resized()
{
    card = cardBounds();
    const bool wide = getWidth() >= 720;
    auto inner = card.reduced (wide ? 30 : 20, wide ? 24 : 18);
    const int bh = learn::buttonHeight();

    if (wide)
    {
        leftArea = inner.removeFromLeft (inner.getWidth() * 55 / 100);
        inner.removeFromLeft (24);
        rightArea = inner;
    }
    else
    {
        leftArea = inner.removeFromTop (inner.getHeight() * 52 / 100);
        rightArea = inner;
    }

    auto r = rightArea;
    if (pro::isUnlocked())
    {
        r.removeFromTop ((int) fs (70.0f));
    }
    else
    {
        r.removeFromTop ((int) fs (34.0f));
        buyButton.setBounds (r.removeFromTop (bh + 10));
        r.removeFromTop (8);
        restoreButton.setBounds (r.removeFromTop (bh));
        r.removeFromTop ((int) fs (16.0f));
    }
    r.removeFromTop ((int) fs (26.0f));   // título de las donaciones
    if (wide)
    {
        // Uno debajo de otro, a todo lo ancho
        const int dh = juce::jmin (bh, (r.getHeight() - bh - 24) / 3 - 6);
        for (auto* b : donateButtons)
        {
            b->setBounds (r.removeFromTop (dh));
            r.removeFromTop (6);
        }
    }
    else
    {
        const int dw = (r.getWidth() - 16) / 3;
        auto donationRow = r.removeFromTop (bh);
        for (auto* b : donateButtons)
        {
            b->setBounds (donationRow.removeFromLeft (dw));
            donationRow.removeFromLeft (8);
        }
    }
    closeButton.setBounds (rightArea.removeFromBottom (bh).removeFromRight (juce::jmin (180, rightArea.getWidth())));
}

void ProScreen::mouseUp (const juce::MouseEvent& e)
{
    // Tocar fuera de la tarjeta la cierra
    if (! card.contains (e.getPosition()))
        hide();
}

void ProScreen::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.72f));

    auto c = card.toFloat();
    Draw::shadow (g, c, 18.0f, 24);
    g.setColour (Colours::panel);
    g.fillRoundedRectangle (c, 18.0f);
    g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.28f), c.getX(), c.getY(),
                                             Colours::panel, c.getX() + c.getWidth() * 0.6f, c.getBottom(), false));
    g.fillRoundedRectangle (c, 18.0f);
    g.setColour (Colours::accent.withAlpha (0.7f));
    g.drawRoundedRectangle (c.reduced (0.5f), 18.0f, 1.5f);

    // --- Izquierda: qué incluye Pro ------------------------------------------------
    auto l = leftArea.toFloat();
    const auto chipFont = Fonts::bold (fs (11.5f));
    const auto chipText = txt ("STUDOO PRO");
    auto chip = juce::Rectangle<float> (l.getX(), l.getY(), (float) juce::GlyphArrangement::getStringWidthInt (chipFont, chipText) + 24.0f, fs (22.0f));
    Draw::accentFill (g, chip, chip.getHeight() * 0.5f);
    g.setColour (juce::Colour (0xff1a1208));
    g.setFont (chipFont);
    g.drawText (chipText, chip, juce::Justification::centred);
    l.removeFromTop (chip.getHeight() + 10.0f);

    g.setColour (Colours::text);
    g.setFont (Fonts::display (fs (26.0f)));
    g.drawFittedText (pro::isUnlocked() ? txt ("¡Tienes Studoo Pro!") : txt ("Desbloquea todo Studoo"),
                      l.removeFromTop (fs (34.0f)).toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
    if (reason.isNotEmpty() && ! pro::isUnlocked())
    {
        g.setColour (Colours::accentLight);
        g.setFont (Fonts::bold (fs (14.0f)));
        g.drawFittedText (reason, l.removeFromTop (fs (22.0f)).toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
    }
    l.removeFromTop (8.0f);

    const float rowH = juce::jmin (fs (30.0f), l.getHeight() / 6.0f);
    for (auto* b : benefits)
    {
        auto row = l.removeFromTop (rowH);
        auto tick = row.removeFromLeft (rowH).withSizeKeepingCentre (fs (18.0f), fs (18.0f));
        Draw::colourFill (g, tick, Colours::green, tick.getWidth() * 0.5f);
        juce::Path p;
        p.startNewSubPath (tick.getX() + tick.getWidth() * 0.27f, tick.getCentreY());
        p.lineTo (tick.getX() + tick.getWidth() * 0.45f, tick.getY() + tick.getHeight() * 0.7f);
        p.lineTo (tick.getRight() - tick.getWidth() * 0.25f, tick.getY() + tick.getHeight() * 0.3f);
        g.setColour (juce::Colour (0xff0d2414));
        g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (Colours::text.withAlpha (0.92f));
        g.setFont (Fonts::regular (fs (14.5f)));
        g.drawFittedText (txt (b), row.withTrimmedLeft (8.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.75f);
    }

    // --- Derecha: comprar y donar -------------------------------------------------------
    auto r = rightArea.toFloat();
    g.setFont (Fonts::bold (fs (14.0f)));
    if (pro::isUnlocked())
    {
        g.setColour (Colours::green);
        g.drawFittedText (txt ("Gracias por apoyar Studoo. Tienes todo desbloqueado."),
                          r.removeFromTop (fs (70.0f)).toNearestInt(), juce::Justification::centredLeft, 3, 0.9f);
    }
    else
    {
        g.setColour (Colours::textDim);
        g.drawFittedText (pro::storeReady() || pro::price (pro::proProduct).isNotEmpty()
                              ? txt ("Pago seguro con Google Play")
                              : txt ("Conectando con Google Play..."),
                          r.removeFromTop (fs (34.0f)).toNearestInt(), juce::Justification::centredLeft, 1, 0.9f);
        r.removeFromTop ((float) (buyButton.getHeight() + 8 + restoreButton.getHeight()) + fs (16.0f));
    }
    g.setColour (Colours::text);
    g.setFont (Fonts::bold (fs (14.0f)));
    g.drawFittedText (txt ("¿Te gusta Studoo? Invita al creador a algo:"), r.removeFromTop (fs (26.0f)).toNearestInt(),
                      juce::Justification::centredLeft, 1, 0.8f);
}
}
