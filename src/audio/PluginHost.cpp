#include "PluginHost.h"

namespace studoo
{
PluginHost::PluginHost (juce::PropertiesFile& s) : settings (s)
{
    formatManager.addDefaultFormats();

    if (auto xml = settings.getXmlValue ("pluginList"))
        knownPlugins.recreateFromXml (*xml);

    knownPlugins.addChangeListener (this);
}

PluginHost::~PluginHost()
{
    knownPlugins.removeChangeListener (this);
}

void PluginHost::changeListenerCallback (juce::ChangeBroadcaster*)
{
    if (auto xml = knownPlugins.createXml())
    {
        settings.setValue ("pluginList", xml.get());
        settings.saveIfNeeded();
    }
}

juce::Array<juce::PluginDescription> PluginHost::getInstruments() const
{
    juce::Array<juce::PluginDescription> result;
    for (const auto& d : knownPlugins.getTypes())
        if (d.isInstrument)
            result.add (d);
    return result;
}

juce::Array<juce::PluginDescription> PluginHost::getEffects() const
{
    juce::Array<juce::PluginDescription> result;
    for (const auto& d : knownPlugins.getTypes())
        if (! d.isInstrument)
            result.add (d);
    return result;
}

void PluginHost::createInstance (const juce::PluginDescription& desc, double sampleRate, int blockSize, Callback callback)
{
    formatManager.createPluginInstanceAsync (desc, sampleRate, blockSize,
        [callback] (std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& error)
        {
            if (instance != nullptr)
            {
                // Pedimos salida estéreo
                instance->enableAllBuses();
            }
            callback (std::move (instance), error);
        });
}

void PluginHost::createInstanceFromXml (const juce::String& descriptionXml, double sampleRate, int blockSize, Callback callback)
{
    juce::PluginDescription desc;
    auto xml = juce::parseXML (descriptionXml);
    if (xml == nullptr || ! desc.loadFromXml (*xml))
    {
        callback (nullptr, juce::String::fromUTF8 ("Descripción del plugin no válida"));
        return;
    }
    createInstance (desc, sampleRate, blockSize, std::move (callback));
}

void PluginHost::showScanner (juce::Component* centreAround)
{
    auto deadMansPedal = settings.getFile().getSiblingFile ("PluginsQueFallaron.txt");
    auto* list = new juce::PluginListComponent (formatManager, knownPlugins, deadMansPedal, &settings, true);
    list->setSize (700, 450);

    juce::DialogWindow::LaunchOptions o;
    o.content.setOwned (list);
    o.dialogTitle = juce::String::fromUTF8 ("Plugins VST3: pulsa \"Options...\" > \"Scan for new or updated VST3 plug-ins\"");
    o.dialogBackgroundColour = juce::Colour (0xff262b31);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = true;
    o.resizable = true;
    o.componentToCentreAround = centreAround;
    o.launchAsync();
}

// ============================================================================
//  Ventanas de plugins
// ============================================================================
namespace
{
    class PluginWindow : public juce::DocumentWindow
    {
    public:
        PluginWindow (juce::AudioPluginInstance& p, const juce::String& title)
            : DocumentWindow (title, juce::Colour (0xff262b31), DocumentWindow::closeButton), plugin (p)
        {
            setUsingNativeTitleBar (true);
            juce::AudioProcessorEditor* editor = plugin.hasEditor() ? plugin.createEditorIfNeeded() : nullptr;
            if (editor == nullptr)
                editor = new juce::GenericAudioProcessorEditor (plugin);
            setContentOwned (editor, true);
            setResizable (editor->isResizable(), false);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        ~PluginWindow() override { clearContentComponent(); }

        void closeButtonPressed() override { PluginWindows::closeFor (&plugin); }

        juce::AudioPluginInstance& plugin;
    };

    juce::OwnedArray<PluginWindow>& windows()
    {
        static juce::OwnedArray<PluginWindow> list;
        return list;
    }
}

void PluginWindows::show (juce::AudioPluginInstance* plugin, const juce::String& title)
{
    if (plugin == nullptr)
        return;

    for (auto* w : windows())
        if (&w->plugin == plugin)
        {
            w->toFront (true);
            return;
        }

    windows().add (new PluginWindow (*plugin, title));
}

void PluginWindows::closeFor (juce::AudioProcessor* plugin)
{
    for (int i = windows().size(); --i >= 0;)
        if (&windows()[i]->plugin == plugin)
            windows().remove (i);
}

void PluginWindows::closeAll()
{
    windows().clear();
}
}
