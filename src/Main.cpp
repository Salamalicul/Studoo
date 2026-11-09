// ============================================================================
//  Main.cpp: el punto de arranque de Studoo.
//  Aquí se crea la ventana principal cuando abres la app.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"

namespace studoo
{
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : DocumentWindow (name, Colours::background, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);
            setResizable (true, true);
            setResizeLimits (900, 340, 4000, 3000);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

    class StudooApplication : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() override    { return "Studoo"; }
        const juce::String getApplicationVersion() override { return STUDOO_VERSION; }
        bool moreThanOneInstanceAllowed() override          { return true; }

        void initialise (const juce::String&) override
        {
            mainWindow = std::make_unique<MainWindow> ("Studoo " + getApplicationVersion());
        }

        void shutdown() override { mainWindow = nullptr; }
        void systemRequestedQuit() override { quit(); }

    private:
        std::unique_ptr<MainWindow> mainWindow;
    };
}

START_JUCE_APPLICATION (studoo::StudooApplication)
