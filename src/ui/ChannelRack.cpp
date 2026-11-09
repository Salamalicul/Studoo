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

// Botón del nombre del canal: clic izquierdo = seleccionar, clic derecho = menú
class ChannelRack::NameButton : public juce::TextButton
{
public:
    std::function<void()> onRightClick;

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

    for (int row = 0; row < rack.getNumRows(); ++row)
    {
        const int uid = rack.getRowUid (row);
        auto channel = ctx.project.findChannel (uid);
        const bool muted = channel[ids::muted];
        const bool isAudio = channel[ids::type].toString() == "audio";

        juce::String data;
        for (auto item : pattern)
            if (item.hasType (ids::STEPSET) && (int) item[ids::channel] == uid)
                data = item[ids::data].toString();

        for (int s = 0; s < length; ++s)
        {
            auto cell = getCellBounds (row, s);
            const bool on = s < data.length() && data[s] == '1';

            juce::Colour colour = ((s / 4) % 2 == 0) ? Colours::stepOffA : Colours::stepOffB;
            if (on)    colour = Colours::stepOn;
            if (muted) colour = colour.withMultipliedSaturation (0.3f).withMultipliedBrightness (0.6f);
            if (isAudio) colour = colour.withAlpha (0.3f);

            g.setColour (colour);
            g.fillRoundedRectangle (cell, 3.0f);
            g.setColour (juce::Colours::white.withAlpha (on ? 0.25f : 0.05f));
            g.fillRoundedRectangle (cell.withHeight (cell.getHeight() * 0.35f), 3.0f);

            if (s == playingStep)
            {
                g.setColour (Colours::accent.withAlpha (on ? 0.9f : 0.5f));
                g.drawRoundedRectangle (cell.expanded (1.0f), 3.0f, 2.0f);
            }
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
            g.setColour (Colours::accent.withAlpha (0.85f));
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
    content.onPaint = [this] (juce::Graphics& g)
    {
        const int selected = ctx.project.getSelectedChannel();
        for (int r = 0; r < rows.size(); ++r)
        {
            auto rowArea = juce::Rectangle<int> (0, r * rowHeight, content.getWidth(), rowHeight);
            if (rows[r]->uid == selected)
            {
                g.setColour (Colours::accent.withAlpha (0.08f));
                g.fillRect (rowArea);
                g.setColour (Colours::accent);
                g.fillRect (rowArea.removeFromLeft (3));
            }
            g.setColour (Colours::outline.withAlpha (0.4f));
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
    content.setSize (width, juce::jmax (rows.size() * rowHeight, 10));
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
    for (int t = 1; t < numMixerTracks; ++t)
    {
        bool used = false;
        for (auto ch : ctx.project.channels())
            used = used || (int) ch[ids::mixer] == t;
        if (! used)
            return t;
    }
    return 0;
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
    menu.addSectionHeader (txt ("Añadir canal"));
    menu.addItem (1, txt ("Sample (WAV, MP3...)"));
    menu.addItem (2, txt ("Kit de batería (todos los samples de una carpeta)"));
    menu.addSeparator();
    menu.addItem (3, txt ("Sintetizador"));
    menu.addItem (4, "808");
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
