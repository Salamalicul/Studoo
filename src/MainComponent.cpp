#include "MainComponent.h"

namespace studoo
{
MainComponent::MainComponent()
{
    setLookAndFeel (&lookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    // Archivo donde Studoo recuerda tus ajustes (por ejemplo, la tarjeta de sonido)
    juce::PropertiesFile::Options options;
    options.applicationName     = "Studoo";
    options.folderName          = "Studoo";
    options.filenameSuffix      = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    settings = std::make_unique<juce::PropertiesFile> (options);

    addAndMakeVisible (transportBar);
    addAndMakeVisible (sequencer);
    transportBar.onAudioSettings = [this] { showAudioSettings(); };

    initialiseAudio();

    setWantsKeyboardFocus (true);
    setSize (1100, 350);
}

MainComponent::~MainComponent()
{
    deviceManager.removeChangeListener (this);
    saveAudioSettings();
    deviceManager.removeAudioCallback (&audioPlayer);
    audioPlayer.setSource (nullptr);
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void MainComponent::initialiseAudio()
{
    // Conectamos: tarjeta de sonido -> reproductor -> nuestro motor
    audioPlayer.setSource (&engine);

    std::unique_ptr<juce::XmlElement> saved;
    if (settings != nullptr)
        saved = settings->getXmlValue ("audioDevice");

    deviceManager.initialise (0, 2, saved.get(), true);
    deviceManager.addAudioCallback (&audioPlayer);

    // Cuando cambie la tarjeta de sonido, actualizamos la barra de estado y lo guardamos
    deviceManager.addChangeListener (this);
}

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster*)
{
    saveAudioSettings();
    repaint();
}

void MainComponent::saveAudioSettings()
{
    if (settings == nullptr)
        return;
    if (auto xml = deviceManager.createStateXml())
        settings->setValue ("audioDevice", xml.get());
    settings->saveIfNeeded();
}

void MainComponent::showAudioSettings()
{
    auto* selector = new juce::AudioDeviceSelectorComponent (deviceManager, 0, 0, 2, 2, false, false, true, false);
    selector->setSize (520, 380);

    juce::DialogWindow::LaunchOptions o;
    o.content.setOwned (selector);
    o.dialogTitle = txt ("Ajustes de audio");
    o.dialogBackgroundColour = Colours::panel;
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = true;
    o.resizable = false;
    o.componentToCentreAround = this;
    o.launchAsync();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);

    // Barra de estado inferior con consejos
    g.setColour (Colours::header);
    g.fillRect (statusArea);
    g.setColour (Colours::textDim);
    g.setFont (juce::FontOptions (13.0f));
    g.drawText (txt ("Espacio: reproducir / parar   ·   Pasa el ratón por encima de cualquier control "
                     "para ver qué hace   ·   Pulsa \"?\" para ver la ayuda de cada sección"),
                statusArea.reduced (12, 0), juce::Justification::centredLeft);

    if (auto* device = deviceManager.getCurrentAudioDevice())
        g.drawText (deviceManager.getCurrentAudioDeviceType() + "  |  " + device->getName()
                        + "  |  " + juce::String ((int) device->getCurrentSampleRate()) + " Hz",
                    statusArea.reduced (12, 0), juce::Justification::centredRight);
    else
        g.drawText (txt ("Sin audio: pulsa \"Audio\""), statusArea.reduced (12, 0),
                    juce::Justification::centredRight);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    transportBar.setBounds (area.removeFromTop (56));
    statusArea = area.removeFromBottom (26);
    sequencer.setBounds (area.reduced (12));
}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        transportBar.togglePlayStop();
        return true;
    }
    return false;
}

void MainComponent::parentHierarchyChanged()
{
    // Pedimos el teclado para que funcione la barra espaciadora
    if (isShowing())
        grabKeyboardFocus();
}
}
