#include "ChannelRack.h"
#include "HelpTexts.h"

namespace studoo
{
namespace
{
    constexpr int headerHeight = 42;
    constexpr int numbersHeight = 18;
    constexpr int controlsWidth = 300;
    constexpr int panelHeight = 110;
    constexpr float cellGap = 3.0f;
    constexpr float groupGap = 6.0f;

    // Botón de "luz" verde para silenciar un canal
    class MuteLight : public juce::Button
    {
    public:
        MuteLight() : juce::Button ({})
        {
            setTooltip (txt ("Luz verde = el canal suena. Clic para silenciarlo (mute) o volver a activarlo."));
            makeNonFocusing (*this);
        }

        void paintButton (juce::Graphics& g, bool isMouseOver, bool) override
        {
            auto area = getLocalBounds().toFloat().withSizeKeepingCentre (12.0f, 12.0f);
            const bool on = getToggleState();
            if (on)
            {
                g.setColour (Colours::green.withAlpha (0.25f));
                g.fillEllipse (area.expanded (3.0f));
            }
            g.setColour (on ? Colours::green : Colours::panelLight.brighter (isMouseOver ? 0.3f : 0.1f));
            g.fillEllipse (area);
            g.setColour (juce::Colours::black.withAlpha (0.5f));
            g.drawEllipse (area, 1.0f);
        }
    };
}

// Botón del nombre del canal: clic izquierdo = seleccionar, clic derecho = menú.
// Tiene una franja con el color del tipo de canal que se ilumina cuando suena.
class ChannelRack::NameButton : public juce::TextButton
{
public:
    std::function<void()> onRightClick;
    juce::Colour colour { Colours::accent };
    juce::String typeName;
    bool selected = false;
    float activity = 0.0f;

    void setActivity (float a)
    {
        a = juce::jlimit (0.0f, 1.0f, a * 1.6f);
        if (std::abs (a - activity) > 0.02f)
        {
            activity = a;
            repaint();
        }
    }

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        const float corner = 6.0f;

        auto base = Colours::panelLight;
        if (selected)          base = base.interpolatedWith (colour, 0.18f);
        if (isButtonDown)      base = base.darker (0.15f);
        else if (isMouseOver)  base = base.brighter (0.1f);

        g.setGradientFill (juce::ColourGradient (base.brighter (0.08f), 0.0f, r.getY(),
                                                 base.darker (0.2f), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, corner);

        // Franja de color + luz de actividad
        auto strip = r.withWidth (5.0f).reduced (0.0f, 3.0f).translated (3.0f, 0.0f);
        if (activity > 0.03f)
            Draw::glow (g, strip, colour, 3.0f + activity * 7.0f, 2.5f);
        g.setColour (colour.withMultipliedBrightness (0.55f + 0.45f * activity));
        g.fillRoundedRectangle (strip, 2.5f);
        if (activity > 0.03f)
        {
            g.setColour (juce::Colours::white.withAlpha (activity * 0.6f));
            g.fillRoundedRectangle (strip.reduced (1.0f, 2.0f), 2.0f);
        }

        g.setColour (selected ? colour.withAlpha (0.85f) : juce::Colours::black.withAlpha (0.55f));
        g.drawRoundedRectangle (r.reduced (0.5f), corner, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawHorizontalLine ((int) r.getY() + 1, r.getX() + corner, r.getRight() - corner);

        auto textArea = r.withTrimmedLeft (14.0f).withTrimmedRight (6.0f);
        g.setColour (selected ? Colours::text : Colours::text.withAlpha (0.88f));
        g.setFont (Fonts::bold (13.0f));
        g.drawFittedText (getButtonText(), textArea.removeFromTop (r.getHeight() * 0.62f).toNearestInt(),
                          juce::Justification::bottomLeft, 1, 0.8f);
        g.setColour (colour.withAlpha (0.75f));
        g.setFont (Fonts::regular (9.5f));
        g.drawText (typeName.toUpperCase(), textArea, juce::Justification::topLeft);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick) onRightClick();
            return;
        }
        juce::TextButton::mouseDown (e);
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (! e.mods.isPopupMenu())
            juce::TextButton::mouseUp (e);
    }
};

// Fila "+ Añadir canal" al final de la lista
class ChannelRack::AddRow : public juce::Button
{
public:
    AddRow() : juce::Button ("addRow")
    {
        setTooltip (txt ("Añadir un canal nuevo: un kit de batería, un sonido incluido, tu propio sample, un 808, "
                         "un sintetizador o un plugin."));
        makeNonFocusing (*this);
    }

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto r = getLocalBounds().toFloat().reduced (6.0f, 6.0f);
        if (isMouseOver)
        {
            g.setColour (Colours::accent.withAlpha (isButtonDown ? 0.14f : 0.08f));
            g.fillRoundedRectangle (r, 8.0f);
        }
        juce::Path outline;
        outline.addRoundedRectangle (r, 8.0f);
        juce::Path dashed;
        const float dashes[] = { 5.0f, 4.0f };
        juce::PathStrokeType (1.2f).createDashedStroke (dashed, outline, dashes, 2);
        g.setColour (isMouseOver ? Colours::accent.withAlpha (0.8f) : Colours::outline.brighter (0.25f));
        g.fillPath (dashed);

        g.setColour (isMouseOver ? Colours::accentLight : Colours::textDim);
        g.setFont (Fonts::bold (13.0f));
        g.drawText (txt ("+  Añadir canal"), r, juce::Justification::centred);
    }
};

struct ChannelRack::Row
{
    int uid = 0;
    MuteLight mute;
    NameButton name;
    Knob volume { "VOL", txt ("Volumen de este canal.") };
    Knob pan { "PAN", txt ("Panorama: izquierda o derecha. En el centro, suena por los dos lados.") };
    juce::ComboBox mixerBox;
};

// ============================================================================
//  StepGrid
// ============================================================================
StepGrid::StepGrid (AppContext& c, ChannelRack& r) : ctx (c), rack (r)
{
    setTooltip (txt ("Pasos del patrón. Clic: encender/apagar. Arrastra: pintar varios. Clic derecho: borrar. "
                     "Cada grupo de 4 cuadrados es un tiempo."));
    makeNonFocusing (*this);
}

void StepGrid::setPlayingStep (int step)
{
    if (step != playingStep)
    {
        playingStep = step;
        repaint();
    }
}

juce::Rectangle<float> StepGrid::getCellBounds (int row, int step) const
{
    const int length = rack.getPatternLength();
    const int groups = (length + 3) / 4;
    const float cellWidth = ((float) getWidth() - groupGap * (float) (groups - 1)) / (float) length;
    const float x = (float) step * cellWidth + (float) (step / 4) * groupGap;
    const float y = (float) (row * ChannelRack::rowHeight);
    return { x + cellGap * 0.5f, y + 6.0f, cellWidth - cellGap, (float) ChannelRack::rowHeight - 12.0f };
}

bool StepGrid::cellAt (juce::Point<float> pos, int& row, int& step) const
{
    row = (int) (pos.y / ChannelRack::rowHeight);
    if (! juce::isPositiveAndBelow (row, rack.getNumRows()))
        return false;

    const int length = rack.getPatternLength();
    for (int s = 0; s < length; ++s)
        if (getCellBounds (row, s).expanded (cellGap * 0.5f + groupGap * 0.5f, 6.0f).contains (pos))
        {
            step = s;
            return true;
        }
    return false;
}

void StepGrid::paint (juce::Graphics& g)
{
    auto pattern = rack.getPattern();
    const int length = rack.getPatternLength();

    // Columna de luz del paso que está sonando
    if (playingStep >= 0 && playingStep < length && rack.getNumRows() > 0)
    {
        auto col = getCellBounds (0, playingStep).withBottom ((float) getHeight()).withY (0.0f).expanded (2.0f, 0.0f);
        g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.16f), col.getCentreX(), 0.0f,
                                                 Colours::accent.withAlpha (0.04f), col.getCentreX(), (float) getHeight(), false));
        g.fillRoundedRectangle (col, 5.0f);
    }

    for (int row = 0; row < rack.getNumRows(); ++row)
    {
        const int uid = rack.getRowUid (row);
        auto channel = ctx.project.findChannel (uid);
        const bool muted = channel[ids::muted];
        const bool isAudio = channel[ids::type].toString() == "audio";
        const auto colour = Colours::forChannelType (channel[ids::type].toString());
        const bool orange = colour == Colours::accent;

        juce::String data;
        for (auto item : pattern)
            if (item.hasType (ids::STEPSET) && (int) item[ids::channel] == uid)
                data = item[ids::data].toString();

        for (int s = 0; s < length; ++s)
        {
            auto cell = getCellBounds (row, s);
            const bool on = s < data.length() && data[s] == '1';
            const bool playing = (s == playingStep);
            const float corner = juce::jmin (6.0f, cell.getWidth() * 0.25f);

            if (on && ! muted && ! isAudio)
            {
                // Paso encendido: LED naranja con brillo (más intenso si está sonando)
                Draw::glow (g, cell, colour, playing ? 9.0f : 4.0f, corner);
                if (orange) Draw::accentFill (g, cell, corner);
                else        Draw::colourFill (g, cell, colour, corner);
                if (playing)
                {
                    g.setColour (juce::Colours::white.withAlpha (0.35f));
                    g.fillRoundedRectangle (cell, corner);
                }
                g.setColour ((orange ? Colours::accentDeep : colour).darker (0.6f));
                g.drawRoundedRectangle (cell.reduced (0.5f), corner, 1.0f);
                continue;
            }

            // Paso apagado
            juce::Colour base = ((s / 4) % 2 == 0) ? Colours::stepOffA : Colours::stepOffB;
            if (on) base = colour.withMultipliedSaturation (0.3f).withMultipliedBrightness (0.5f); // en mute
            if (playing) base = base.brighter (0.25f);
            if (isAudio) base = base.withAlpha (0.35f);

            g.setGradientFill (juce::ColourGradient (base.brighter (0.12f), 0.0f, cell.getY(),
                                                     base.darker (0.25f), 0.0f, cell.getBottom(), false));
            g.fillRoundedRectangle (cell, corner);
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.drawRoundedRectangle (cell.reduced (0.5f), corner, 1.0f);
            g.setColour (juce::Colours::white.withAlpha (0.05f));
            g.drawHorizontalLine ((int) cell.getY() + 1, cell.getX() + corner, cell.getRight() - corner);
        }

        // Vista previa de las notas del piano roll de este canal
        juce::Array<juce::ValueTree> notes;
        int lo = 127, hi = 0;
        for (auto item : pattern)
            if (item.hasType (ids::NOTE) && (int) item[ids::channel] == uid)
            {
                notes.add (item);
                lo = juce::jmin (lo, (int) item[ids::pitch]);
                hi = juce::jmax (hi, (int) item[ids::pitch]);
            }

        if (! notes.isEmpty())
        {
            const auto rowArea = juce::Rectangle<float> (0.0f, (float) (row * ChannelRack::rowHeight) + 8.0f,
                                                         (float) getWidth(), (float) ChannelRack::rowHeight - 16.0f);
            const float range = (float) juce::jmax (1, hi - lo);
            const float pxPerStep = (getCellBounds (0, length - 1).getRight() + cellGap) / (float) length;
            g.setColour (colour.brighter (0.3f).withAlpha (0.95f));
            for (auto& n : notes)
            {
                const float x = (float) (double) n[ids::start] * pxPerStep;
                const float w = juce::jmax (2.0f, (float) (double) n[ids::length] * pxPerStep - 1.0f);
                const float y = rowArea.getBottom() - ((float) ((int) n[ids::pitch] - lo) / range) * (rowArea.getHeight() - 3.0f) - 3.0f;
                g.fillRoundedRectangle (x, y, w, 3.0f, 1.0f);
            }
        }
    }
}

void StepGrid::mouseDown (const juce::MouseEvent& e)
{
    int row = 0, step = 0;
    if (! cellAt (e.position, row, step))
    {
        paintRow = -1;
        return;
    }

    ctx.project.beginTransaction();
    paintRow = row;
    paintValue = e.mods.isPopupMenu() ? false : ! ctx.project.getStep (rack.getPattern(), rack.getRowUid (row), step);
    ctx.project.setSelectedChannel (rack.getRowUid (row));
    applyPaint (e);
}

void StepGrid::mouseDrag (const juce::MouseEvent& e)
{
    if (paintRow >= 0)
        applyPaint (e);
}

void StepGrid::mouseUp (const juce::MouseEvent&)
{
    paintRow = -1;
}

void StepGrid::applyPaint (const juce::MouseEvent& e)
{
    int row = 0, step = 0;
    const juce::Point<float> pos (e.position.x, getCellBounds (paintRow, 0).getCentreY());
    if (cellAt (pos, row, step))
    {
        const int uid = rack.getRowUid (row);
        auto pattern = rack.getPattern();
        if (ctx.project.getStep (pattern, uid, step) != paintValue)
        {
            ctx.project.setStep (pattern, uid, step, paintValue);
            if (paintValue && ! ctx.engine.isPlaying())
            {
                auto ch = ctx.project.findChannel (uid);
                ctx.engine.previewNote (uid, ch.getProperty (ids::root, 60), 0.8f, 0.25);
            }
        }
    }
}

// ============================================================================
//  ChannelRack
// ============================================================================
ChannelRack::ChannelRack (AppContext& c)
    : ctx (c), helpButton (txt (help::rackTitle), txt (help::rackText)), grid (c, *this), instrumentPanel (c)
{
    addAndMakeVisible (helpButton);

    addAndMakeVisible (patternBox);
    patternBox.setTooltip (txt ("Patrón que estás editando. Cada patrón es un trozo de música (por ejemplo, el "
                                "estribillo) que luego colocas en la playlist."));
    patternBox.onChange = [this]
    {
        auto p = ctx.project.patterns().getChild (patternBox.getSelectedId() - 1);
        if (p.isValid())
            ctx.project.setCurrentPattern (p[ids::uid]);
    };
    makeNonFocusing (patternBox);

    addAndMakeVisible (patternMenuButton);
    patternMenuButton.setButtonText (txt ("Patrón..."));
    patternMenuButton.setTooltip (txt ("Crear un patrón nuevo, duplicar, renombrar o eliminar el patrón actual."));
    patternMenuButton.onClick = [this] { showPatternMenu(); };
    makeNonFocusing (patternMenuButton);

    addAndMakeVisible (lengthBox);
    lengthBox.addItem (txt ("16 pasos (1 compás)"), 16);
    lengthBox.addItem (txt ("32 pasos (2 compases)"), 32);
    lengthBox.addItem (txt ("64 pasos (4 compases)"), 64);
    lengthBox.setTooltip (txt ("Duración del patrón."));
    lengthBox.onChange = [this]
    {
        ctx.project.beginTransaction();
        getPattern().setProperty (ids::length, lengthBox.getSelectedId(), ctx.project.um());
    };
    makeNonFocusing (lengthBox);

    addAndMakeVisible (addButton);
    addButton.setButtonText (txt ("+ Añadir"));
    addButton.setTooltip (txt ("Añadir un canal: tu propio sample, un kit de batería, un sintetizador, un 808 o un plugin VST3."));
    addButton.onClick = [this] { showAddMenu(); };
    makeNonFocusing (addButton);

    addAndMakeVisible (exampleButton);
    exampleButton.setButtonText (txt ("Beat de ejemplo"));
    exampleButton.setTooltip (txt ("Rellena el patrón actual con un ritmo de ejemplo (batería + 808)."));
    exampleButton.onClick = [this]
    {
        ctx.project.beginTransaction();
        ctx.project.loadExampleBeat (getPattern());
    };
    makeNonFocusing (exampleButton);

    addAndMakeVisible (clearButton);
    clearButton.setButtonText ("Limpiar");
    clearButton.setTooltip (txt ("Borra todos los pasos y notas del patrón actual (puedes deshacerlo con Ctrl+Z)."));
    clearButton.onClick = [this]
    {
        ctx.project.beginTransaction();
        ctx.project.clearPattern (getPattern());
    };
    makeNonFocusing (clearButton);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);
    content.addAndMakeVisible (grid);
    addRow = std::make_unique<AddRow>();
    addRow->onClick = [this] { showAddMenu(); };
    content.addAndMakeVisible (*addRow);
    content.onPaint = [this] (juce::Graphics& g)
    {
        const int selected = ctx.project.getSelectedChannel();
        for (int r = 0; r < rows.size(); ++r)
        {
            auto rowArea = juce::Rectangle<int> (0, r * rowHeight, content.getWidth(), rowHeight);
            if (r % 2 == 1)
            {
                g.setColour (juce::Colours::white.withAlpha (0.015f));
                g.fillRect (rowArea);
            }
            if (rows[r]->uid == selected)
            {
                const auto rowColour = rows[r]->name.colour;
                g.setGradientFill (juce::ColourGradient (rowColour.withAlpha (0.16f), 0.0f, 0.0f,
                                                         rowColour.withAlpha (0.0f), (float) controlsWidth + 120.0f, 0.0f, false));
                g.fillRect (rowArea);
                auto bar = rowArea.removeFromLeft (3).toFloat().reduced (0.0f, 6.0f);
                Draw::glow (g, bar, rowColour, 5.0f, 1.5f);
                Draw::colourFill (g, bar, rowColour, 1.5f);
            }
            g.setColour (juce::Colours::black.withAlpha (0.3f));
            g.drawHorizontalLine ((r + 1) * rowHeight - 1, 0.0f, (float) content.getWidth());
        }
    };

    addAndMakeVisible (instrumentPanel);
    instrumentPanel.onLoadSample = [this] (juce::ValueTree ch) { loadSampleInto (ch); };

    ctx.project.addChangeListener (this);
    changeListenerCallback (nullptr);
    startTimerHz (30);
}

ChannelRack::~ChannelRack()
{
    ctx.project.removeChangeListener (this);
}

int ChannelRack::getRowUid (int row) const
{
    return juce::isPositiveAndBelow (row, rows.size()) ? rows[row]->uid : 0;
}

juce::Component* ChannelRack::getRowNameButton (int row)
{
    return juce::isPositiveAndBelow (row, rows.size()) ? &rows[row]->name : nullptr;
}

juce::ValueTree ChannelRack::getPattern()
{
    return ctx.project.getCurrentPattern();
}

int ChannelRack::getPatternLength()
{
    return Project::getPatternLength (getPattern());
}

void ChannelRack::changeListenerCallback (juce::ChangeBroadcaster*)
{
    juce::String key;
    for (auto ch : ctx.project.channels())
        key << (int) ch[ids::uid] << ch[ids::type].toString() << ",";

    if (key != structureKey || ctx.project.getGeneration() != lastGeneration)
        rebuildRows();

    // Actualizamos nombres, mute y pista del mezclador
    for (auto* row : rows)
    {
        auto ch = ctx.project.findChannel (row->uid);
        row->name.setButtonText (ch[ids::name].toString());
        row->name.selected = (row->uid == ctx.project.getSelectedChannel());
        row->mute.setToggleState (! (bool) ch[ids::muted], juce::dontSendNotification);
        row->mixerBox.setSelectedId ((int) ch[ids::mixer] + 1, juce::dontSendNotification);
    }

    const int selected = ctx.project.getSelectedChannel();
    if (selected != lastSelected)
    {
        lastSelected = selected;
        instrumentPanel.setChannel (ctx.project.findChannel (selected));
    }
    else
    {
        instrumentPanel.refresh();
    }

    refreshHeader();
    content.repaint();
    grid.repaint();
}

void ChannelRack::refreshHeader()
{
    patternBox.clear (juce::dontSendNotification);
    int i = 1;
    for (auto p : ctx.project.patterns())
        patternBox.addItem (p[ids::name].toString(), i++);
    patternBox.setSelectedId (ctx.project.patterns().indexOf (getPattern()) + 1, juce::dontSendNotification);
    lengthBox.setSelectedId (getPatternLength(), juce::dontSendNotification);
}

void ChannelRack::rebuildRows()
{
    structureKey.clear();
    lastGeneration = ctx.project.getGeneration();
    rows.clear();

    for (auto ch : ctx.project.channels())
    {
        structureKey << (int) ch[ids::uid] << ch[ids::type].toString() << ",";

        auto* row = rows.add (new Row());
        row->uid = ch[ids::uid];
        const int uid = row->uid;

        content.addAndMakeVisible (row->mute);
        row->mute.setToggleState (! (bool) ch[ids::muted], juce::dontSendNotification);
        row->mute.onClick = [this, uid]
        {
            auto c = ctx.project.findChannel (uid);
            ctx.project.beginTransaction();
            c.setProperty (ids::muted, ! (bool) c[ids::muted], ctx.project.um());
        };

        content.addAndMakeVisible (row->name);
        row->name.setButtonText (ch[ids::name].toString());
        row->name.colour = Colours::forChannelType (ch[ids::type].toString());
        row->name.typeName = Colours::channelTypeName (ch[ids::type].toString());
        if (ch[ids::type].toString() == "sampler" && ch[ids::sample].toString().startsWith ("builtin:"))
            row->name.typeName = txt ("Batería");
        row->name.setTooltip (txt ("Clic: seleccionar y escuchar este canal. Clic derecho: más opciones "
                                   "(piano roll, renombrar, cambiar sample, eliminar)."));
        row->name.onClick = [this, uid]
        {
            ctx.project.setSelectedChannel (uid);
            auto c = ctx.project.findChannel (uid);
            ctx.engine.previewNote (uid, c.getProperty (ids::root, 60), 0.8f, 0.4);
        };
        row->name.onRightClick = [this, uid] { showChannelMenu (uid); };
        makeNonFocusing (row->name);

        for (auto* k : { &row->volume, &row->pan })
            k->getSlider().setColour (juce::Slider::rotarySliderFillColourId, row->name.colour);
        content.addAndMakeVisible (row->volume);
        row->volume.bind (ch, ids::volume, ctx.project.um(), 0.0, 1.0, 0.8);
        content.addAndMakeVisible (row->pan);
        row->pan.bind (ch, ids::pan, ctx.project.um(), -1.0, 1.0, 0.0);
        row->pan.setDisplayFunction ([] (double v)
        {
            if (std::abs (v) < 0.02) return juce::String ("Centro");
            return juce::String ((int) std::abs (v * 100.0)) + (v < 0 ? "% izq." : "% der.");
        });

        content.addAndMakeVisible (row->mixerBox);
        row->mixerBox.addItem ("Master", 1);
        for (int t = 1; t < numMixerTracks; ++t)
            row->mixerBox.addItem ("Pista " + juce::String (t), t + 1);
        row->mixerBox.setSelectedId ((int) ch[ids::mixer] + 1, juce::dontSendNotification);
        row->mixerBox.setTooltip (txt ("A qué pista del mezclador va el sonido de este canal. Cada sonido en su propia "
                                       "pista te permite ponerle efectos por separado."));
        row->mixerBox.onChange = [this, uid, box = &row->mixerBox]
        {
            ctx.project.beginTransaction();
            ctx.project.findChannel (uid).setProperty (ids::mixer, box->getSelectedId() - 1, ctx.project.um());
        };
        makeNonFocusing (row->mixerBox);
    }

    lastSelected = -1;
    refreshHeader();
    resized();
}

void ChannelRack::timerCallback()
{
    const bool showPlayhead = ctx.engine.isPlaying() && ! ctx.engine.isSongMode();
    grid.setPlayingStep (showPlayhead ? (int) ctx.engine.getPlayPosition() : -1);

    if (isShowing())
        for (auto* row : rows)
            row->name.setActivity (ctx.engine.getChannelActivity (row->uid));
}

void ChannelRack::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().withTrimmedBottom (panelHeight + 8);
    drawPanel (g, area, headerArea, txt ("Secuenciador de patrones"));

    // Números de los pasos encima de la cuadrícula
    const int length = getPatternLength();
    g.setFont (juce::FontOptions (11.0f));
    const auto gridInRack = getLocalArea (&grid, grid.getLocalBounds());
    for (int s = 0; s < length; ++s)
    {
        if (length > 32 && s % 2 == 1)
            continue;
        auto cell = grid.getCellBounds (0, s);
        g.setColour (s % 4 == 0 ? Colours::text : Colours::textDim);
        g.drawText (juce::String (s + 1),
                    juce::Rectangle<float> ((float) gridInRack.getX() + cell.getX(), (float) (headerArea.getBottom() + 2),
                                            cell.getWidth(), (float) numbersHeight),
                    juce::Justification::centred);
    }

    g.setColour (Colours::textDim);
    g.drawText ("Canal", juce::Rectangle<int> (60, headerArea.getBottom() + 2, 100, numbersHeight), juce::Justification::centredLeft);
}

void ChannelRack::resized()
{
    auto area = getLocalBounds();
    auto panel = area.removeFromBottom (panelHeight);
    instrumentPanel.setBounds (panel);
    area.removeFromBottom (8);

    headerArea = area.removeFromTop (headerHeight);
    {
        auto h = headerArea.reduced (8, 7);
        helpButton.setBounds (h.removeFromLeft (26));
        h.removeFromLeft (230);
        patternBox.setBounds (h.removeFromLeft (150)); h.removeFromLeft (4);
        patternMenuButton.setBounds (h.removeFromLeft (84)); h.removeFromLeft (8);
        lengthBox.setBounds (h.removeFromLeft (170));
        clearButton.setBounds (h.removeFromRight (80)); h.removeFromRight (6);
        exampleButton.setBounds (h.removeFromRight (130)); h.removeFromRight (6);
        addButton.setBounds (h.removeFromRight (100));
    }

    area.removeFromTop (numbersHeight + 2);
    area.reduce (8, 0);
    area.removeFromBottom (6);
    viewport.setBounds (area);

    const int width = viewport.getWidth() - viewport.getScrollBarThickness();
    content.setSize (width, rows.size() * rowHeight + rowHeight + 4);
    addRow->setBounds (0, rows.size() * rowHeight + 2, controlsWidth - 4, rowHeight);
    grid.setBounds (controlsWidth, 0, width - controlsWidth - 4, rows.size() * rowHeight);

    for (int r = 0; r < rows.size(); ++r)
    {
        auto row = juce::Rectangle<int> (6, r * rowHeight, controlsWidth - 10, rowHeight).reduced (0, 4);
        auto* rw = rows[r];
        rw->mute.setBounds (row.removeFromLeft (22));
        row.removeFromLeft (4);
        rw->mixerBox.setBounds (row.removeFromRight (82).withSizeKeepingCentre (82, 26));
        row.removeFromRight (4);
        rw->pan.setBounds (row.removeFromRight (34).expanded (0, 4));
        rw->volume.setBounds (row.removeFromRight (34).expanded (0, 4));
        row.removeFromRight (4);
        rw->name.setBounds (row);
    }
    repaint();
}

// ----------------------------------------------------------------------------
//  Menús
// ----------------------------------------------------------------------------
int ChannelRack::findFreeMixerTrack()
{
    return ctx.project.findFreeMixerTrack();
}

void ChannelRack::addSampleChannels (const juce::Array<juce::File>& files)
{
    ctx.project.beginTransaction();
    int lastUid = 0;
    for (auto& f : files)
    {
        if (! SampleCache::isAudioFile (f))
            continue;
        const int track = findFreeMixerTrack();
        auto ch = ctx.project.addChannel (ChannelType::sampler, f.getFileNameWithoutExtension(), track);
        ch.setProperty (ids::sample, f.getFullPathName(), ctx.project.um());
        if (track > 0)
            ctx.project.getMixerTrack (track).setProperty (ids::name, f.getFileNameWithoutExtension().substring (0, 12), ctx.project.um());
        lastUid = ch[ids::uid];
    }
    if (lastUid != 0)
        ctx.project.setSelectedChannel (lastUid);
}

void ChannelRack::loadSampleInto (juce::ValueTree channel)
{
    chooser = std::make_unique<juce::FileChooser> (txt ("Elige un sample"), juce::File(), ctx.samples.getSupportedWildcard());
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this, channel] (const juce::FileChooser& fc) mutable
    {
        auto f = fc.getResult();
        if (! f.existsAsFile())
            return;
        ctx.project.beginTransaction();
        channel.setProperty (ids::sample, f.getFullPathName(), ctx.project.um());
        channel.setProperty (ids::name, f.getFileNameWithoutExtension(), ctx.project.um());
    });
}

void ChannelRack::showAddMenu()
{
    juce::PopupMenu menu;
    // Kits de batería y efectos incluidos
    const auto& kitList = kits::all();
    juce::PopupMenu drumKits, effects;
    for (size_t k = 0; k < kitList.size(); ++k)
    {
        const auto& kit = kitList[k];
        if (kit.isEffects)
        {
            for (size_t i = 0; i < kit.sounds.size(); ++i)
                effects.addItem (1000 + (int) k * 50 + (int) i, txt (kit.sounds[i].name));
            continue;
        }
        juce::PopupMenu sub;
        sub.addSectionHeader (txt (kit.description));
        sub.addItem (200 + (int) k * 2, txt ("Usar este kit (cambia los sonidos de tu batería y conserva el ritmo)"));
        sub.addItem (201 + (int) k * 2, txt ("Añadir todos sus sonidos como canales nuevos"));
        sub.addSeparator();
        for (size_t i = 0; i < kit.sounds.size(); ++i)
            sub.addItem (1000 + (int) k * 50 + (int) i, txt ("Añadir solo: ") + txt (kit.sounds[i].name));
        drumKits.addSubMenu (txt (kit.name) + "  (" + juce::String ((int) kit.sounds.size()) + ")", sub);
    }

    juce::PopupMenu synths;
    synths.addItem (3, txt ("Sintetizador básico"));
    synths.addSeparator();
    synths.addSectionHeader (txt ("Sonidos ya preparados"));
    for (size_t i = 0; i < kits::synthPresets().size(); ++i)
        synths.addItem (3000 + (int) i, txt (kits::synthPresets()[i].name) + "  -  " + txt (kits::synthPresets()[i].description));

    juce::PopupMenu basses;
    for (size_t i = 0; i < kits::bass808Presets().size(); ++i)
        basses.addItem (4000 + (int) i, txt (kits::bass808Presets()[i].name) + "  -  " + txt (kits::bass808Presets()[i].description));

    menu.addSectionHeader (txt ("Sonidos incluidos en Studoo"));
    menu.addSubMenu (txt ("Kits de batería"), drumKits);
    menu.addSubMenu ("808", basses);
    menu.addSubMenu (txt ("Sintetizador"), synths);
    menu.addSubMenu (txt ("Efectos (FX)"), effects);
    menu.addSeparator();
    menu.addSectionHeader (txt ("Tus propios sonidos"));
    menu.addItem (1, txt ("Sample (WAV, MP3...)"));
    menu.addItem (2, txt ("Kit de batería (todos los samples de una carpeta)"));
    menu.addSeparator();

    juce::PopupMenu plugins;
    const auto instruments = ctx.plugins.getInstruments();
    for (int i = 0; i < instruments.size(); ++i)
        plugins.addItem (100 + i, instruments[i].name + "  (" + instruments[i].manufacturerName + ")");
    if (instruments.isEmpty())
        plugins.addItem (-1, txt ("No hay plugins: pulsa \"Plugins\" arriba para buscarlos"), false);
    menu.addSubMenu ("Plugin VST3", plugins);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&addButton), [this, instruments] (int result)
    {
        if (result == 1)
        {
            chooser = std::make_unique<juce::FileChooser> (txt ("Elige uno o varios samples"), juce::File(),
                                                           ctx.samples.getSupportedWildcard());
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles
                                      | juce::FileBrowserComponent::canSelectMultipleItems,
                                  [this] (const juce::FileChooser& fc) { addSampleChannels (fc.getResults()); });
        }
        else if (result == 2)
        {
            chooser = std::make_unique<juce::FileChooser> (txt ("Elige la carpeta del kit de batería"));
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                                  [this] (const juce::FileChooser& fc)
            {
                auto dir = fc.getResult();
                if (! dir.isDirectory())
                    return;
                auto files = dir.findChildFiles (juce::File::findFiles, false, "*.wav;*.mp3;*.aif;*.aiff;*.flac;*.ogg");
                files.sort();
                if (files.size() > 32)
                    files.removeRange (32, files.size() - 32);
                addSampleChannels (files);
            });
        }
        else if (result == 3 || result == 4)
        {
            ctx.project.beginTransaction();
            const int track = findFreeMixerTrack();
            auto ch = ctx.project.addChannel (result == 3 ? ChannelType::synth : ChannelType::bass808,
                                              result == 3 ? "Sintetizador" : "808", track);
            ctx.project.setSelectedChannel (ch[ids::uid]);
        }
        else if (result >= 200 && result < 200 + (int) kits::all().size() * 2)
        {
            const auto& kit = kits::all()[(size_t) (result - 200) / 2];
            ctx.project.beginTransaction();
            if ((result - 200) % 2 == 0)
                ctx.project.useKit (kit);
            else
                ctx.project.addKit (kit);
        }
        else if (result >= 3000 && result < 3000 + (int) kits::synthPresets().size())
        {
            ctx.project.beginTransaction();
            auto ch = ctx.project.addSynthPreset (kits::synthPresets()[(size_t) (result - 3000)]);
            ctx.project.setSelectedChannel (ch[ids::uid]);
            ctx.engine.previewNote (ch[ids::uid], ch[ids::root], 0.8f, 0.6);
        }
        else if (result >= 4000 && result < 4000 + (int) kits::bass808Presets().size())
        {
            ctx.project.beginTransaction();
            auto ch = ctx.project.add808Preset (kits::bass808Presets()[(size_t) (result - 4000)]);
            ctx.project.setSelectedChannel (ch[ids::uid]);
            ctx.engine.previewNote (ch[ids::uid], 36, 0.8f, 0.8);
        }
        else if (result >= 1000 && result < 3000)
        {
            const auto k = (size_t) (result - 1000) / 50, i = (size_t) (result - 1000) % 50;
            if (k < kits::all().size() && i < kits::all()[k].sounds.size())
            {
                ctx.project.beginTransaction();
                auto ch = ctx.project.addKitSound (kits::all()[k], kits::all()[k].sounds[i]);
                ctx.project.setSelectedChannel (ch[ids::uid]);
                ctx.engine.previewNote (ch[ids::uid], 60, 0.85f, 0.4);
            }
        }
        else if (result >= 100 && result - 100 < instruments.size())
        {
            const auto desc = instruments[result - 100];
            ctx.plugins.createInstance (desc, ctx.engine.getSampleRate(), ctx.engine.getBlockSize(),
                [this, desc] (std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& error)
            {
                if (instance == nullptr)
                {
                    juce::AlertWindow::showAsync (juce::MessageBoxOptions().withTitle ("Plugin")
                                                      .withMessage (txt ("No se pudo abrir el plugin: ") + error)
                                                      .withButton ("Vale"), nullptr);
                    return;
                }
                ctx.project.beginTransaction();
                auto ch = ctx.project.addChannel (ChannelType::plugin, desc.name, findFreeMixerTrack());
                if (auto xml = desc.createXml())
                    ch.setProperty (ids::pluginDesc, xml->toString (juce::XmlElement::TextFormat().singleLine()), nullptr);
                const int uid = ch[ids::uid];
                auto* raw = instance.get();
                ctx.engine.setChannelPlugin (uid, std::move (instance));
                ctx.project.setSelectedChannel (uid);
                PluginWindows::show (raw, desc.name);
            });
        }
    });
}

void ChannelRack::showChannelMenu (int uid)
{
    auto ch = ctx.project.findChannel (uid);
    if (! ch.isValid())
        return;

    ctx.project.setSelectedChannel (uid);
    const auto type = channelTypeFromString (ch[ids::type].toString());

    juce::PopupMenu menu;
    menu.addSectionHeader (ch[ids::name].toString());
    if (type != ChannelType::audio)
        menu.addItem (1, txt ("Abrir en el piano roll"));
    menu.addItem (2, txt ("Renombrar..."));
    if (type == ChannelType::sampler)
        menu.addItem (3, txt ("Cambiar sample..."));
    if (type == ChannelType::plugin)
        menu.addItem (4, txt ("Abrir plugin"));
    menu.addItem (5, txt ("Mover arriba"));
    menu.addItem (6, txt ("Mover abajo"));
    menu.addSeparator();
    menu.addItem (7, txt ("Eliminar canal"));

    menu.showMenuAsync (juce::PopupMenu::Options(), [this, uid, ch] (int result) mutable
    {
        auto channels = ctx.project.channels();
        const int index = channels.indexOf (ch);
        switch (result)
        {
            case 1: if (ctx.showView) ctx.showView (View::pianoRoll); break;
            case 2:
                askForText (txt ("Renombrar canal"), txt ("Nuevo nombre:"), ch[ids::name].toString(), [this, ch] (const juce::String& name) mutable
                {
                    ctx.project.beginTransaction();
                    ch.setProperty (ids::name, name, ctx.project.um());
                });
                break;
            case 3: loadSampleInto (ch); break;
            case 4: PluginWindows::show (ctx.engine.getChannelPlugin (uid), ch[ids::name].toString()); break;
            case 5:
                if (index > 0) { ctx.project.beginTransaction(); channels.moveChild (index, index - 1, ctx.project.um()); }
                break;
            case 6:
                if (index < channels.getNumChildren() - 1) { ctx.project.beginTransaction(); channels.moveChild (index, index + 1, ctx.project.um()); }
                break;
            case 7:
                ctx.project.beginTransaction();
                ctx.project.removeChannel (uid);
                break;
            default: break;
        }
    });
}

void ChannelRack::showPatternMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, txt ("Nuevo patrón"));
    menu.addItem (2, txt ("Duplicar este patrón"));
    menu.addItem (3, txt ("Renombrar..."));
    menu.addSeparator();
    menu.addItem (4, txt ("Eliminar este patrón"), ctx.project.patterns().getNumChildren() > 1);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&patternMenuButton), [this] (int result)
    {
        auto current = getPattern();
        ctx.project.beginTransaction();
        if (result == 1)
            ctx.project.setCurrentPattern (ctx.project.addPattern()[ids::uid]);
        else if (result == 2)
            ctx.project.setCurrentPattern (ctx.project.duplicatePattern (current[ids::uid])[ids::uid]);
        else if (result == 3)
            askForText (txt ("Renombrar patrón"), txt ("Nuevo nombre (por ejemplo: Estribillo, Estrofa, Intro):"),
                        current[ids::name].toString(), [this, current] (const juce::String& name) mutable
            {
                ctx.project.beginTransaction();
                current.setProperty (ids::name, name, ctx.project.um());
            });
        else if (result == 4)
            ctx.project.removePattern (current[ids::uid]);
    });
}

// ----------------------------------------------------------------------------
//  Arrastrar archivos
// ----------------------------------------------------------------------------
bool ChannelRack::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (SampleCache::isAudioFile (juce::File (f)))
            return true;
    return false;
}

void ChannelRack::filesDropped (const juce::StringArray& files, int x, int y)
{
    juce::Array<juce::File> list;
    for (auto& f : files)
        list.add (juce::File (f));

    // Si sueltas UN archivo sobre un sampler, se cambia su sonido
    const auto local = content.getLocalPoint (this, juce::Point<int> (x, y));
    const int row = local.y / rowHeight;
    if (list.size() == 1 && juce::isPositiveAndBelow (row, rows.size()) && content.getLocalBounds().contains (local))
    {
        auto ch = ctx.project.findChannel (rows[row]->uid);
        if (channelTypeFromString (ch[ids::type].toString()) == ChannelType::sampler)
        {
            ctx.project.beginTransaction();
            ch.setProperty (ids::sample, list[0].getFullPathName(), ctx.project.um());
            ch.setProperty (ids::name, list[0].getFileNameWithoutExtension(), ctx.project.um());
            return;
        }
    }
    addSampleChannels (list);
}
}
