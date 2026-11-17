#include "EarGames.h"
#include "LearnStyle.h"
#include "audio/Kits.h"
#include "model/GenreBeats.h"

namespace studoo
{
namespace
{
    constexpr double gameRate = 44100.0;

    // Dibujo sencillo de cada juego
    void drawGameIcon (juce::Graphics& g, int icon, juce::Rectangle<float> r, juce::Colour c)
    {
        g.setColour (c);
        const auto ctr = r.getCentre();
        const float s = r.getWidth();
        if (icon == 0) // batería
        {
            g.drawEllipse (r.reduced (s * 0.12f), 2.5f);
            g.fillEllipse (juce::Rectangle<float> (s * 0.3f, s * 0.3f).withCentre (ctr));
        }
        else if (icon == 1) // sube / baja
        {
            juce::Path p;
            p.addArrow ({ ctr.x - s * 0.18f, r.getBottom() - s * 0.15f, ctr.x - s * 0.18f, r.getY() + s * 0.15f }, 3.0f, 12.0f, 10.0f);
            p.addArrow ({ ctr.x + s * 0.18f, r.getY() + s * 0.15f, ctr.x + s * 0.18f, r.getBottom() - s * 0.15f }, 3.0f, 12.0f, 10.0f);
            g.fillPath (p);
        }
        else if (icon == 2) // acorde: tres notas apiladas
        {
            for (int i = 0; i < 3; ++i)
                g.fillEllipse (juce::Rectangle<float> (s * 0.34f, s * 0.24f).withCentre ({ ctr.x, r.getBottom() - s * (0.22f + 0.24f * (float) i) }));
        }
        else // género: ondas de sonido
        {
            for (int i = 0; i < 3; ++i)
            {
                juce::Path arc;
                const float rad = s * (0.16f + 0.14f * (float) i);
                arc.addCentredArc (ctr.x - s * 0.2f, ctr.y, rad, rad, 0.0f, 0.6f, 2.55f, true);
                g.strokePath (arc, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            g.fillEllipse (juce::Rectangle<float> (s * 0.18f, s * 0.18f).withCentre ({ ctr.x - s * 0.2f, ctr.y }));
        }
    }

    // Tarjeta de un juego en la lista
    struct GameCard : public juce::Button
    {
        juce::String title, description;
        int icon = 0;
        std::function<int()> record;

        GameCard (const juce::String& t, const juce::String& d, int i) : juce::Button (t), title (t), description (d), icon (i)
        {
            makeNonFocusing (*this);
        }

        void paintButton (juce::Graphics& g, bool over, bool down) override
        {
            auto r = getLocalBounds().toFloat().reduced (4.0f);
            const juce::Colour colours[] = { Colours::accent, Colours::blue, Colours::purple, Colours::green };
            const auto c = colours[icon % 4];
            Draw::shadow (g, r, 14.0f, 10);
            g.setGradientFill (juce::ColourGradient (c.withAlpha (down ? 0.32f : over ? 0.26f : 0.18f), r.getX(), r.getY(),
                                                     Colours::panel, r.getX(), r.getBottom(), false));
            g.fillRoundedRectangle (r, 14.0f);
            g.setColour (c.withAlpha (over ? 0.9f : 0.55f));
            g.drawRoundedRectangle (r.reduced (0.5f), 14.0f, 1.4f);

            auto inner = r.reduced (18.0f, 14.0f);
            auto iconArea = inner.removeFromLeft (juce::jmin (64.0f, inner.getHeight())).withSizeKeepingCentre (52.0f, 52.0f);
            g.setColour (c.withAlpha (0.18f));
            g.fillEllipse (iconArea.expanded (8.0f));
            drawGameIcon (g, icon, iconArea, c.brighter (0.3f));
            inner.removeFromLeft (16.0f);

            g.setColour (Colours::text);
            g.setFont (Fonts::bold (learn::fs (19.0f)));
            g.drawText (title, inner.removeFromTop (inner.getHeight() * 0.42f), juce::Justification::bottomLeft);
            g.setColour (Colours::textDim);
            g.setFont (Fonts::regular (learn::fs (14.0f)));
            auto desc = inner.removeFromTop (inner.getHeight() * 0.62f);
            g.drawFittedText (description, desc.toNearestInt(), juce::Justification::topLeft, 2, 0.9f);
            const int best = record != nullptr ? record() : 0;
            g.setColour (best > 0 ? Colours::accentLight : Colours::textDim.withAlpha (0.7f));
            g.setFont (Fonts::bold (learn::fs (13.0f)));
            g.drawText (best > 0 ? txt ("Récord: ") + juce::String (best) + txt (" seguidas") : txt ("¡Todavía sin récord!"),
                        inner, juce::Justification::centredLeft);
        }
    };

    struct DrumRole { const char* id; const char* name; };
    const DrumRole drumRoles[] = { { "kick", "Bombo" }, { "snare", "Caja" }, { "clap", "Palmada" }, { "hihat", "Hi-hat cerrado" },
                                   { "openhat", "Hi-hat abierto" }, { "crash", "Platillo" }, { "rim", "Rim (aro)" }, { "cowbell", "Cencerro" },
                                   { "shaker", "Shaker" }, { "tomlo", "Tom grave" } };
}

const std::vector<EarGames::Game>& EarGames::games()
{
    static const std::vector<Game> list = {
        { "instrumento", "¿Qué instrumento es?", "Escucha un golpe de batería y adivina qué es: bombo, caja, hi-hat...", 0 },
        { "altura",      "¿Sube o baja?",        "Suenan dos notas. ¿La segunda es más aguda, más grave o igual?", 1 },
        { "acordes",     "¿Mayor o menor?",      "Escucha un acorde: ¿suena alegre (mayor) o triste (menor)?", 2 },
        { "genero",      "¿Qué género es?",      "Suena un ritmo: ¿es trap, reggaetón, house...?", 3 },
    };
    return list;
}

EarGames::EarGames (AudioEngine& e, learn::Progress& p) : engine (e), progress (p)
{
    for (int i = 0; i < (int) games().size(); ++i)
    {
        const auto& gm = games()[(size_t) i];
        auto* card = new GameCard (txt (gm.title), txt (gm.description), gm.icon);
        card->record = [this, id = juce::String (gm.id)] { return progress.bestGameStreak (id); };
        card->onClick = [this, i] { openGame (i); };
        gameCards.add (card);
        addAndMakeVisible (card);
    }

    backButton.setButtonText (txt ("←  Juegos"));
    backButton.onClick = [this] { closeGame(); };
    learn::styleButton (backButton, learn::ButtonStyle::secondary, 14.0f);
    playButton.setButtonText (txt ("▶   Escuchar otra vez"));
    playButton.onClick = [this] { playRound(); };
    learn::styleButton (playButton, learn::ButtonStyle::primary, 17.0f);
    nextButton.setButtonText (txt ("Siguiente  →"));
    nextButton.onClick = [this] { newRound(); };
    learn::styleButton (nextButton, learn::ButtonStyle::primary, 16.0f);

    for (auto* l : { &questionLabel, &feedbackLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setInterceptsMouseClicks (false, false);
        addChildComponent (l);
    }
    questionLabel.setColour (juce::Label::textColourId, Colours::text);
    for (auto* c : { (juce::Component*) &backButton, (juce::Component*) &playButton, (juce::Component*) &nextButton })
        addChildComponent (c);
}

EarGames::~EarGames()
{
    engine.stopBuffer();
}

void EarGames::visibilityChanged()
{
    if (! isVisible())
        engine.stopBuffer();
}

// ----------------------------------------------------------------------------
//  Sonidos
// ----------------------------------------------------------------------------
AudioEngine::SharedBuffer EarGames::renderNotes (const std::vector<std::vector<int>>& groups, double secondsEach)
{
    const int each = (int) (secondsEach * gameRate);
    auto buffer = std::make_shared<juce::AudioBuffer<float>> (2, each * (int) groups.size() + (int) (gameRate * 0.3));
    buffer->clear();
    for (size_t gi = 0; gi < groups.size(); ++gi)
    {
        const auto& notes = groups[gi];
        const float level = 0.32f / std::sqrt ((float) juce::jmax ((size_t) 1, notes.size()));
        for (int note : notes)
        {
            const double f = 440.0 * std::pow (2.0, (note - 69) / 12.0);
            for (int i = 0; i < each + (int) (gameRate * 0.25); ++i)
            {
                const int pos = (int) gi * each + i;
                if (pos >= buffer->getNumSamples())
                    break;
                const double t = i / gameRate;
                // Sonido de "piano eléctrico": armónicos que se apagan poco a poco
                const double env = std::min (1.0, t * 200.0) * std::exp (-t * 2.2) * (i < each ? 1.0 : std::exp (-(i - each) / gameRate * 30.0));
                const double twoPiT = juce::MathConstants<double>::twoPi * f * t;
                const double v = std::sin (twoPiT) + 0.45 * std::sin (2.0 * twoPiT) * std::exp (-t * 3.0)
                               + 0.2 * std::sin (3.0 * twoPiT) * std::exp (-t * 5.0);
                const float s = (float) (v * env) * level;
                buffer->addSample (0, pos, s);
                buffer->addSample (1, pos, s);
            }
        }
    }
    return buffer;
}

AudioEngine::SharedBuffer EarGames::renderDrum (const juce::String& kitId, const juce::String& soundId)
{
    auto rendered = kits::render (kitId, soundId, gameRate);
    if (rendered.getNumSamples() == 0)
        return nullptr;
    auto buffer = std::make_shared<juce::AudioBuffer<float>> (2, rendered.getNumSamples());
    for (int ch = 0; ch < 2; ++ch)
        buffer->copyFrom (ch, 0, rendered, juce::jmin (ch, rendered.getNumChannels() - 1), 0, rendered.getNumSamples());
    return buffer;
}

AudioEngine::SharedBuffer EarGames::renderGenre (const juce::String& genreId)
{
    if (auto it = genreCache.find (genreId); it != genreCache.end())
        return it->second;

    // Un proyecto y un motor de audio solo para preparar el sonido (sin tarjeta de sonido)
    Project project;
    if (! genres::load (project, project.getCurrentPattern(), genreId))
        return nullptr;
    SampleCache cache;
    AudioEngine offline;
    offline.prepare (gameRate, 512);
    offline.updateFromProject (project.getState(), cache);
    offline.play();

    const double bpm = project.getState().getProperty (ids::bpm, 120.0);
    const double seconds = juce::jmin (7.0, 2.0 * 4.0 * 60.0 / bpm);   // 2 compases
    const int total = (int) (seconds * gameRate);
    auto buffer = std::make_shared<juce::AudioBuffer<float>> (2, total);
    juce::AudioBuffer<float> block (2, 512);
    for (int pos = 0; pos < total; pos += 512)
    {
        const int n = juce::jmin (512, total - pos);
        offline.render (nullptr, 0, block.getArrayOfWritePointers(), 2, n);
        buffer->copyFrom (0, pos, block, 0, 0, n);
        buffer->copyFrom (1, pos, block, 1, 0, n);
    }
    // Final suave
    const int fade = (int) (gameRate * 0.15);
    buffer->applyGainRamp (total - fade, fade, 1.0f, 0.0f);
    genreCache[genreId] = buffer;
    return buffer;
}

// ----------------------------------------------------------------------------
//  Rondas
// ----------------------------------------------------------------------------
void EarGames::openGame (int index)
{
    current = index;
    streak = 0;
    for (auto* c : gameCards)
        c->setVisible (false);
    for (auto* c : { (juce::Component*) &backButton, (juce::Component*) &playButton, (juce::Component*) &questionLabel,
                     (juce::Component*) &feedbackLabel })
        c->setVisible (true);
    newRound();
}

void EarGames::closeGame()
{
    engine.stopBuffer();
    current = -1;
    optionButtons.clear();
    for (auto* c : { (juce::Component*) &backButton, (juce::Component*) &playButton, (juce::Component*) &nextButton,
                     (juce::Component*) &questionLabel, (juce::Component*) &feedbackLabel })
        c->setVisible (false);
    for (auto* c : gameCards)
        c->setVisible (true);
    resized();
    repaint();
}

void EarGames::newRound()
{
    if (current < 0)
        return;
    chosen = -1;
    options.clear();
    roundSound = nullptr;
    const juce::String id = games()[(size_t) current].id;

    if (id == "instrumento")
    {
        // Un kit de batería al azar y 4 sonidos suyos
        std::vector<const kits::KitInfo*> drumKits;
        for (auto& k : kits::all())
            if (! k.isEffects)
                drumKits.push_back (&k);
        const auto* kit = drumKits[(size_t) random.nextInt ((int) drumKits.size())];
        juce::StringArray ids_, names;
        for (auto& role : drumRoles)
            for (auto& s : kit->sounds)
                if (juce::String (s.id) == role.id)
                {
                    ids_.add (role.id);
                    names.add (txt (role.name));
                }
        // Elegimos 4 (o los que haya) al azar
        while (ids_.size() > 4)
        {
            const int r = random.nextInt (ids_.size());
            ids_.remove (r);
            names.remove (r);
        }
        correctOption = random.nextInt (ids_.size());
        options = names;
        roundSound = renderDrum (kit->id, ids_[correctOption]);
        questionLabel.setText (txt ("¿Qué instrumento suena?"), juce::dontSendNotification);
    }
    else if (id == "altura")
    {
        const int first = 55 + random.nextInt (14);
        const int kind = random.nextInt (5);                  // 0-1 sube, 2-3 baja, 4 igual
        const int interval = 1 + random.nextInt (juce::jmax (1, 12 - streak / 2));   // más difícil con la racha
        const int second = kind < 2 ? first + interval : kind < 4 ? first - interval : first;
        options = { txt ("Sube ↑"), txt ("Baja ↓"), txt ("Igual =") };
        correctOption = kind < 2 ? 0 : kind < 4 ? 1 : 2;
        roundSound = renderNotes ({ { first }, { second } }, 0.75);
        questionLabel.setText (txt ("La segunda nota... ¿sube, baja o es igual?"), juce::dontSendNotification);
    }
    else if (id == "acordes")
    {
        const int root = 52 + random.nextInt (14);
        const bool major = random.nextBool();
        const int third = root + (major ? 4 : 3), fifth = root + 7;
        options = { txt ("Mayor (alegre)"), txt ("Menor (triste)") };
        correctOption = major ? 0 : 1;
        // Primero las notas una a una y luego el acorde entero
        roundSound = renderNotes ({ { root }, { third }, { fifth }, { root, third, fifth } }, 0.42);
        questionLabel.setText (txt ("¿Este acorde es mayor o menor?"), juce::dontSendNotification);
    }
    else // género
    {
        juce::Array<int> pick;
        const int numGenres = (int) genres::all().size();
        while (pick.size() < 4)
            pick.addIfNotAlreadyThere (random.nextInt (numGenres));
        for (int gi : pick)
            options.add (genres::nameOf (genres::all()[(size_t) gi].id));
        correctOption = random.nextInt (4);
        questionLabel.setText (txt ("Preparando el ritmo..."), juce::dontSendNotification);
        const juce::String genreId = genres::all()[(size_t) pick[correctOption]].id;
        juce::Component::SafePointer<EarGames> safe (this);
        // Se prepara un momento después, para que la pantalla se dibuje antes
        juce::MessageManager::callAsync ([safe, genreId]
        {
            if (safe == nullptr || safe->current < 0)
                return;
            safe->roundSound = safe->renderGenre (genreId);
            safe->questionLabel.setText (txt ("¿De qué género es este ritmo?"), juce::dontSendNotification);
            safe->playRound();
        });
    }

    optionButtons.clear();
    for (int i = 0; i < options.size(); ++i)
    {
        auto* b = optionButtons.add (new juce::TextButton (options[i]));
        learn::styleButton (*b, learn::ButtonStyle::secondary, 17.0f);
        b->onClick = [this, i] { answer (i); };
        addAndMakeVisible (b);
    }
    feedbackLabel.setText (txt ("Escucha bien y elige una respuesta."), juce::dontSendNotification);
    feedbackLabel.setColour (juce::Label::textColourId, Colours::textDim);
    nextButton.setVisible (false);
    layoutGame();
    repaint();
    if (roundSound != nullptr)
        playRound();
}

void EarGames::playRound()
{
    if (roundSound != nullptr)
        engine.playBuffer (roundSound, gameRate, 0.9f);
}

void EarGames::answer (int option)
{
    if (chosen >= 0 || current < 0 || roundSound == nullptr)
        return;
    chosen = option;
    const bool right = option == correctOption;
    const juce::String id = games()[(size_t) current].id;
    progress.addGameAnswer (right);

    if (right)
    {
        ++streak;
        progress.recordGameStreak (id, streak);
        feedbackLabel.setText (txt ("¡Correcto!  Racha: ") + juce::String (streak), juce::dontSendNotification);
        feedbackLabel.setColour (juce::Label::textColourId, Colours::green);
    }
    else
    {
        streak = 0;
        feedbackLabel.setText (txt ("Casi... era \"") + options[correctOption] + txt ("\". Pulsa \"Escuchar otra vez\" para compararlo."),
                               juce::dontSendNotification);
        feedbackLabel.setColour (juce::Label::textColourId, Colours::red.brighter (0.3f));
    }

    for (int i = 0; i < optionButtons.size(); ++i)
    {
        auto* b = optionButtons[i];
        if (i == correctOption)      learn::styleButton (*b, learn::ButtonStyle::good, 17.0f);
        else if (i == option)        learn::styleButton (*b, learn::ButtonStyle::bad, 17.0f);
        b->setEnabled (i == correctOption || i == option);
    }
    nextButton.setVisible (true);
    layoutGame();
    repaint();
}

// ----------------------------------------------------------------------------
//  Dibujo
// ----------------------------------------------------------------------------
void EarGames::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    if (current < 0)
    {
        auto top = area.removeFromTop (Layout::compact() ? 84.0f : 92.0f).reduced (24.0f, 12.0f);
        g.setColour (Colours::text);
        g.setFont (Fonts::display (learn::fs (26.0f)));
        g.drawText (txt ("Juegos de oído"), top.removeFromTop (top.getHeight() * 0.55f), juce::Justification::bottomLeft);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (learn::fs (14.0f)));
        g.drawText (txt ("Entrena tus oídos como un productor. Juega un poco cada día."), top, juce::Justification::centredLeft);
        return;
    }

    // Cabecera del juego: título y racha
    const auto& gm = games()[(size_t) current];
    auto head = headerArea.toFloat();
    g.setColour (Colours::text);
    g.setFont (Fonts::bold (learn::fs (19.0f)));
    g.drawText (txt (gm.title), head.withTrimmedLeft ((float) backButton.getRight() - head.getX() + 14.0f), juce::Justification::centredLeft);

    const int best = progress.bestGameStreak (gm.id);
    const auto streakText = txt ("Racha ") + juce::String (streak) + txt ("   ·   Récord ") + juce::String (best);
    g.setColour (streak > 0 ? Colours::accentLight : Colours::textDim);
    g.setFont (Fonts::bold (learn::fs (14.0f)));
    g.drawText (streakText, head.reduced (8.0f, 0.0f), juce::Justification::centredRight);

    // Indicador de racha: puntos que se encienden (si hay sitio)
    if (Layout::compact() || getHeight() < 420)
        return;
    auto dots = juce::Rectangle<float> (0.0f, 0.0f, 10.0f * 18.0f, 14.0f)
                    .withCentre ({ area.getCentreX(), (float) questionLabel.getY() - 18.0f });
    for (int i = 0; i < 10; ++i)
    {
        auto d = juce::Rectangle<float> (12.0f, 12.0f).withCentre ({ dots.getX() + 9.0f + 18.0f * (float) i, dots.getCentreY() });
        if (i < streak % 11) Draw::accentFill (g, d, 6.0f);
        else { g.setColour (Colours::panelLight); g.fillEllipse (d); }
    }
}

void EarGames::resized()
{
    auto area = getLocalBounds();
    if (current < 0)
    {
        area.removeFromTop (Layout::compact() ? 84 : 92);
        area.reduce (16, 8);
        const bool oneColumn = area.getWidth() < 640;
        const int cols = oneColumn ? 1 : 2;
        const int cardH = Layout::compact() ? 128 : 136;
        for (int i = 0; i < gameCards.size(); ++i)
        {
            const int col = i % cols, row = i / cols;
            const int w = area.getWidth() / cols;
            gameCards[i]->setBounds (area.getX() + col * w, area.getY() + row * (cardH + 8), w, cardH);
        }
        return;
    }
    layoutGame();
}

void EarGames::layoutGame()
{
    const int bh = learn::buttonHeight() + (Layout::compact() ? 0 : 8);
    if (Layout::compact() || getHeight() < 420)
    {
        // Móvil en horizontal: todo más junto para que quepa sin bajar
        auto area = getLocalBounds().reduced (12, 6);
        headerArea = area.removeFromTop (bh);
        backButton.setBounds (headerArea.withWidth (120));
        area.removeFromTop (8);
        auto column = area.withSizeKeepingCentre (juce::jmin (900, area.getWidth()), area.getHeight());
        auto row = column.removeFromTop (bh);
        playButton.setBounds (row.removeFromRight (juce::jmin (300, row.getWidth() / 2)));
        questionLabel.setFont (Fonts::bold (learn::fs (17.0f)));
        questionLabel.setJustificationType (juce::Justification::centredLeft);
        questionLabel.setBounds (row.withTrimmedRight (10));
        column.removeFromTop (10);
        const int n = optionButtons.size();
        const int cols = n <= 3 ? juce::jmax (1, n) : 2;
        for (int i = 0; i < n; ++i)
        {
            const int col = i % cols, r = i / cols;
            const int w = (column.getWidth() - (cols - 1) * 10) / cols;
            optionButtons[i]->setBounds (column.getX() + col * (w + 10), column.getY() + r * (bh + 8), w, bh);
        }
        column.removeFromTop (((n + cols - 1) / cols) * (bh + 8) + 2);
        auto last = column.removeFromTop (bh);
        nextButton.setBounds (last.removeFromRight (juce::jmin (220, last.getWidth() / 3)));
        feedbackLabel.setFont (Fonts::bold (learn::fs (14.0f)));
        feedbackLabel.setJustificationType (juce::Justification::centredLeft);
        feedbackLabel.setBounds (last.withTrimmedRight (10));
        return;
    }
    questionLabel.setJustificationType (juce::Justification::centred);
    feedbackLabel.setJustificationType (juce::Justification::centred);

    auto area = getLocalBounds().reduced (16, 10);
    headerArea = area.removeFromTop (learn::buttonHeight());
    backButton.setBounds (headerArea.withWidth (Layout::compact() ? 130 : 120));

    auto column = area.withSizeKeepingCentre (juce::jmin (620, area.getWidth()), area.getHeight());
    column.removeFromTop (Layout::compact() ? 30 : 46);
    questionLabel.setFont (Fonts::bold (learn::fs (22.0f)));
    questionLabel.setBounds (column.removeFromTop (Layout::compact() ? 64 : 56));
    column.removeFromTop (10);
    playButton.setBounds (column.removeFromTop (bh).withSizeKeepingCentre (juce::jmin (340, column.getWidth()), bh));
    column.removeFromTop (22);

    const int n = optionButtons.size();
    const int cols = n <= 3 ? n : 2;
    const int rowsN = cols > 0 ? (n + cols - 1) / cols : 0;
    for (int i = 0; i < n; ++i)
    {
        const int col = i % cols, row = i / cols;
        const int w = (column.getWidth() - (cols - 1) * 10) / juce::jmax (1, cols);
        optionButtons[i]->setBounds (column.getX() + col * (w + 10), column.getY() + row * (bh + 10), w, bh);
    }
    column.removeFromTop (rowsN * (bh + 10) + 8);
    feedbackLabel.setFont (Fonts::bold (learn::fs (16.0f)));
    feedbackLabel.setBounds (column.removeFromTop (Layout::compact() ? 64 : 52));
    nextButton.setBounds (column.removeFromTop (bh).withSizeKeepingCentre (juce::jmin (260, column.getWidth()), bh));
}
}
