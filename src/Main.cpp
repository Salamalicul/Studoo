// ============================================================================
//  Main.cpp: el punto de arranque de Studoo.
//  Aquí se crea la ventana principal cuando abres la app.
// ============================================================================
#include "Version.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"
#include <BinaryData.h>

namespace studoo
{
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : DocumentWindow (name, Colours::background, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setIcon (juce::ImageCache::getFromMemory (BinaryData::icon_png, BinaryData::icon_pngSize));
            content = new MainComponent();
            setContentOwned (content, true);
           #if JUCE_ANDROID || JUCE_IOS
            // Tablet: pantalla completa y en horizontal
            setFullScreen (true);
           #else
            setResizable (true, true);
            setResizeLimits (1000, 640, 6000, 4000);
            centreWithSize (getWidth(), getHeight());
           #endif
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

        // Si ningún control tiene el teclado, se lo pasamos a la pantalla principal
        bool keyPressed (const juce::KeyPress& key) override
        {
            return content != nullptr && content->keyPressed (key);
        }

        MainComponent* content = nullptr;
    };

    class StudooApplication : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() override    { return "Studoo"; }
        const juce::String getApplicationVersion() override { return STUDOO_VERSION; }
        bool moreThanOneInstanceAllowed() override          { return true; }

        void initialise (const juce::String&) override
        {
           #if JUCE_ANDROID || JUCE_IOS
            juce::Desktop::getInstance().setOrientationsEnabled (juce::Desktop::rotatedClockwise
                                                                 | juce::Desktop::rotatedAntiClockwise);
           #endif
            Layout::applyModeHook() = [this] (const juce::String& mode) { applyScreenMode (mode); };
            applyScreenMode ("auto");
            mainWindow = std::make_unique<MainWindow> ("Studoo");
            mainWindow->content->applyStoredScreenMode();
        }

        // Elige entre la pantalla normal (ordenador y tablet) y el modo móvil.
        //  - Tablet: si la pantalla es más pequeña que 1320 x 800 puntos, se encoge un poco.
        //  - Móvil: pestañas grandes abajo y la interfaz reorganizada, encogida lo justo.
        //  - "auto": modo móvil si la pantalla mide menos de 600 puntos de alto.
        void applyScreenMode (const juce::String& mode)
        {
           #if JUCE_ANDROID || JUCE_IOS
            auto& desktop = juce::Desktop::getInstance();
            desktop.setGlobalScaleFactor (1.0f);
            if (auto* display = desktop.getDisplays().getPrimaryDisplay())
            {
                const auto area = display->userArea;
                const float w = (float) juce::jmax (area.getWidth(), area.getHeight());
                const float h = (float) juce::jmin (area.getWidth(), area.getHeight());
                const bool compact = mode == "movil" || (mode != "tablet" && h < 600.0f);
                Layout::compactFlag() = compact;
                desktop.setGlobalScaleFactor (compact ? juce::jlimit (0.5f, 1.0f, juce::jmin (h / 500.0f, w / 1000.0f))
                                                      : juce::jlimit (0.4f, 1.0f, juce::jmin (w / 1320.0f, h / 800.0f)));
            }
            if (mainWindow != nullptr)
                if (auto* display = desktop.getDisplays().getPrimaryDisplay())
                    mainWindow->setBounds (display->userArea);
           #else
            // En el ordenador: modo móvil solo si se elige (o con STUDOO_MOVIL=1, para probar)
            Layout::compactFlag() = mode == "movil"
                                    || (mode == "auto" && juce::SystemStats::getEnvironmentVariable ("STUDOO_MOVIL", {}).isNotEmpty());
           #endif
            if (mainWindow != nullptr && mainWindow->content != nullptr)
                mainWindow->content->refreshLayout();
        }

        void shutdown() override { mainWindow = nullptr; }

        void systemRequestedQuit() override
        {
            if (mainWindow != nullptr && mainWindow->content != nullptr)
                mainWindow->content->requestQuit ([] { juce::JUCEApplication::getInstance()->quit(); });
            else
                quit();
        }

    private:
        std::unique_ptr<MainWindow> mainWindow;
    };
}

START_JUCE_APPLICATION (studoo::StudooApplication)
