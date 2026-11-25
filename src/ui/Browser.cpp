#include "Browser.h"
#include "Pro.h"
#include "audio/Kits.h"
#include "audio/SampleCache.h"

namespace studoo
{
namespace
{
    juce::File studooFolder (const juce::String& sub)
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Studoo").getChildFile (sub);
        dir.createDirectory();
        return dir;
    }

    // Un elemento de la lista: carpeta, sonido, preset o proyecto
    class Item : public juce::TreeViewItem
    {
    public:
        enum class Kind { folder, kitFolder, userFolder, sample, synthPreset, preset808, project };

        Item (BrowserPanel& b, Kind k, juce::String text, juce::String data = {}, int number = 0, bool locked = false)
            : browser (b), kind (k), label (std::move (text)), path (std::move (data)), index (number), pro (locked) {}

        bool mightContainSubItems() override { return kind == Kind::folder || kind == Kind::kitFolder || kind == Kind::userFolder; }
        juce::String getUniqueName() const override { return label + "|" + path; }
        int getItemHeight() const override { return 24; }

        void paintItem (juce::Graphics& g, int width, int height) override
        {
            if (isSelected())
            {
                g.setColour (Colours::accent.withAlpha (0.25f));
                g.fillRoundedRectangle (0.0f, 1.0f, (float) width, (float) height - 2.0f, 4.0f);
            }
            const bool folder = mightContainSubItems();
            g.setColour (folder ? Colours::text : (pro ? Colours::textDim : Colours::text.withAlpha (0.85f)));
            g.setFont (folder ? Fonts::bold (12.5f) : Fonts::regular (12.5f));
            auto r = juce::Rectangle<int> (4, 0, width - 8, height);
            const juce::String icon = folder ? juce::String() : kind == Kind::project ? txt ("♪ ") : kind == Kind::sample ? txt ("▸ ") : txt ("◆ ");
            g.drawText (icon + label, r, juce::Justification::centredLeft, true);
            if (pro)
            {
                g.setColour (Colours::accent);
                g.setFont (Fonts::bold (10.0f));
                g.drawText ("Pro", r, juce::Justification::centredRight);
            }
        }

        void itemOpennessChanged (bool isNowOpen) override
        {
            if (! isNowOpen || getNumSubItems() > 0)
                return;
            if (kind == Kind::kitFolder)
            {
                if (auto* kit = kits::find (path))
                    for (auto& s : kit->sounds)
                        addSubItem (new Item (browser, Kind::sample, juce::String::fromUTF8 (s.name), kits::samplePath (*kit, s), 0,
                                              ! pro::isKitFree (kit->id)));
            }
            else if (kind == Kind::userFolder)
            {
                const juce::File dir (path);
                auto dirs = dir.findChildFiles (juce::File::findDirectories, false);
                dirs.sort();
                for (auto& d : dirs)
                    if (! d.isHidden())
                        addSubItem (new Item (browser, Kind::userFolder, d.getFileName(), d.getFullPathName()));
                auto files = dir.findChildFiles (juce::File::findFiles, false);
                files.sort();
                int count = 0;
                for (auto& f : files)
                {
                    if (count >= 2000) break;   // carpetas enormes: solo los 2000 primeros
                    if (SampleCache::isAudioFile (f))
                        addSubItem (new Item (browser, Kind::sample, f.getFileNameWithoutExtension(), f.getFullPathName()));
                    else if (f.hasFileExtension ("studoo"))
                        addSubItem (new Item (browser, Kind::project, f.getFileNameWithoutExtension(), f.getFullPathName()));
                    ++count;
                }
            }
        }

        void itemClicked (const juce::MouseEvent&) override
        {
            if (kind == Kind::sample)
                browser.preview (path);
        }

        void itemDoubleClicked (const juce::MouseEvent&) override
        {
            switch (kind)
            {
                case Kind::sample:
                    browser.addSample (path, label, path.startsWith ("builtin:") ? path.fromFirstOccurrenceOf ("builtin:", false, false)
                                                                                        .upToFirstOccurrenceOf ("/", false, false) : juce::String());
                    break;
                case Kind::synthPreset: browser.addSynthPreset (index); break;
                case Kind::preset808:   browser.add808Preset (index); break;
                case Kind::project:     if (browser.onOpenProject) browser.onOpenProject (juce::File (path)); break;
                default:                setOpen (! isOpen()); break;
            }
        }

    private:
        BrowserPanel& browser;
        Kind kind;
        juce::String label, path;
        int index = 0;
        bool pro = false;
    };
}

BrowserPanel::BrowserPanel (AppContext& c) : ctx (c)
{
    addAndMakeVisible (tree);
    tree.setRootItemVisible (false);
    tree.setDefaultOpenness (false);
    tree.setIndentSize (14);
    tree.setColour (juce::TreeView::backgroundColourId, juce::Colours::transparentBlack);
    tree.setTooltip (txt ("Un clic: escuchar. Doble clic: añadirlo al secuenciador (o abrir el proyecto)."));

    addAndMakeVisible (addFolderButton);
    addFolderButton.setButtonText (txt ("+ Carpeta"));
    addFolderButton.setTooltip (txt ("Añade al navegador una carpeta tuya con samples."));
    addFolderButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> (txt ("Elige una carpeta con samples"), juce::File());
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                              [this] (const juce::FileChooser& fc)
        {
            auto dir = fc.getResult();
            if (! dir.isDirectory())
                return;
            auto list = juce::StringArray::fromLines (ctx.settings.getValue ("browserFolders"));
            list.addIfNotAlreadyThere (dir.getFullPathName());
            list.removeEmptyStrings();
            ctx.settings.setValue ("browserFolders", list.joinIntoString ("\n"));
            refresh();
        });
    };
   #if JUCE_ANDROID || JUCE_IOS
    addFolderButton.setVisible (false);   // en el móvil se usan las carpetas de Studoo
   #endif
    makeNonFocusing (addFolderButton);
    refresh();
}

BrowserPanel::~BrowserPanel()
{
    tree.setRootItem (nullptr);
}

void BrowserPanel::refresh()
{
    tree.setRootItem (nullptr);
    root = std::make_unique<Item> (*this, Item::Kind::folder, "root");

    auto* kitsItem = new Item (*this, Item::Kind::folder, txt ("Kits de batería y FX"));
    for (auto& kit : kits::all())
        kitsItem->addSubItem (new Item (*this, Item::Kind::kitFolder, juce::String::fromUTF8 (kit.name), kit.id, 0, ! pro::isKitFree (kit.id)));
    root->addSubItem (kitsItem);

    auto* synths = new Item (*this, Item::Kind::folder, txt ("Sonidos de sintetizador"));
    for (size_t i = 0; i < kits::synthPresets().size(); ++i)
        synths->addSubItem (new Item (*this, Item::Kind::synthPreset, juce::String::fromUTF8 (kits::synthPresets()[i].name), {}, (int) i,
                                      ! pro::isSynthPresetFree ((int) i)));
    root->addSubItem (synths);

    auto* bass = new Item (*this, Item::Kind::folder, txt ("Tipos de 808"));
    for (size_t i = 0; i < kits::bass808Presets().size(); ++i)
        bass->addSubItem (new Item (*this, Item::Kind::preset808, juce::String::fromUTF8 (kits::bass808Presets()[i].name), {}, (int) i,
                                    ! pro::is808PresetFree ((int) i)));
    root->addSubItem (bass);

    root->addSubItem (new Item (*this, Item::Kind::userFolder, txt ("Tus samples (Documentos/Studoo/Samples)"),
                                studooFolder ("Samples").getFullPathName()));
    root->addSubItem (new Item (*this, Item::Kind::userFolder, txt ("Importados"), studooFolder ("Importados").getFullPathName()));
    root->addSubItem (new Item (*this, Item::Kind::userFolder, txt ("Grabaciones"), studooFolder ("Grabaciones").getFullPathName()));
    root->addSubItem (new Item (*this, Item::Kind::userFolder, txt ("Proyectos"), studooFolder ("Proyectos").getFullPathName()));
    for (auto& folder : juce::StringArray::fromLines (ctx.settings.getValue ("browserFolders")))
        if (juce::File::isAbsolutePath (folder) && juce::File (folder).isDirectory())
            root->addSubItem (new Item (*this, Item::Kind::userFolder, juce::File (folder).getFileName(), folder));

    tree.setRootItem (root.get());
}

void BrowserPanel::preview (const juce::String& samplePath)
{
    auto sample = ctx.samples.get (samplePath);
    if (sample == nullptr || sample->buffer.getNumSamples() == 0)
        return;
    // Solo los primeros 10 segundos (un loop largo no tiene que sonar entero)
    AudioEngine::SharedBuffer buffer (sample, &sample->buffer);
    ctx.engine.playBuffer (buffer, sample->sampleRate, 0.8f);
}

void BrowserPanel::addSample (const juce::String& samplePath, const juce::String& name, const juce::String& kitId)
{
    if (kitId.isNotEmpty() && ! pro::isKitFree (kitId))
    {
        if (ctx.showPro) ctx.showPro (txt ("Este kit es de Studoo Pro"));
        return;
    }
    ctx.project.beginTransaction();
    auto ch = ctx.project.addChannel (ChannelType::sampler, name.substring (0, 24), ctx.project.findFreeMixerTrack());
    ch.setProperty (ids::sample, samplePath, ctx.project.um());
    const int track = ch[ids::mixer];
    if (track > 0)
        ctx.project.getMixerTrack (track).setProperty (ids::name, name.substring (0, 12), ctx.project.um());
    ctx.project.setSelectedChannel (ch[ids::uid]);
    if (onStatus) onStatus (txt ("Añadido al secuenciador: ") + name);
}

void BrowserPanel::addSynthPreset (int index)
{
    if (! pro::isSynthPresetFree (index)) { if (ctx.showPro) ctx.showPro (txt ("Este sonido es de Studoo Pro")); return; }
    ctx.project.beginTransaction();
    auto ch = ctx.project.addSynthPreset (kits::synthPresets()[(size_t) index]);
    ctx.project.setSelectedChannel (ch[ids::uid]);
    if (onStatus) onStatus (txt ("Añadido al secuenciador: ") + ch[ids::name].toString());
}

void BrowserPanel::add808Preset (int index)
{
    if (! pro::is808PresetFree (index)) { if (ctx.showPro) ctx.showPro (txt ("Este 808 es de Studoo Pro")); return; }
    ctx.project.beginTransaction();
    auto ch = ctx.project.add808Preset (kits::bass808Presets()[(size_t) index]);
    ctx.project.setSelectedChannel (ch[ids::uid]);
    if (onStatus) onStatus (txt ("Añadido al secuenciador: ") + ch[ids::name].toString());
}

void BrowserPanel::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), getLocalBounds().withHeight (42), txt ("Navegador"));
}

void BrowserPanel::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (42).reduced (8, 8);
    addFolderButton.setBounds (header.removeFromRight (86));
    tree.setBounds (area.reduced (4, 4));
}
}
