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
            // Studoo está diseñado para una pantalla de unos 1320 x 820 puntos.
            //  - Tablet: si la pantalla es más pequeña, lo encogemos un poco para que quepa.
            //  - Móvil (pantalla baja): "modo móvil", con pestañas grandes abajo y la
            //    interfaz reorganizada, encogida solo lo justo.
            auto& desktop = juce::Desktop::getInstance();
            desktop.setOrientationsEnabled (juce::Desktop::rotatedClockwise | juce::Desktop::rotatedAntiClockwise);
            if (auto* display = desktop.getDisplays().getPrimaryDisplay())
            {
                const auto area = display->userArea;
                const float w = (float) juce::jmax (area.getWidth(), area.getHeight());
                const float h = (float) juce::jmin (area.getWidth(), area.getHeight());
                if (h < 600.0f)
                {
                    Layout::compactFlag() = true;
                    desktop.setGlobalScaleFactor (juce::jlimit (0.6f, 1.0f, juce::jmin (h / 500.0f, w / 1000.0f)));
                }
                else
                {
                    const float scale = juce::jmin (1.0f, w / 1320.0f, h / 800.0f);
                    if (scale < 0.99f)
                        desktop.setGlobalScaleFactor (scale);
                }
            }
           #else
            // Para probar el modo móvil en el ordenador: STUDOO_MOVIL=1 ./Studoo
            if (juce::SystemStats::getEnvironmentVariable ("STUDOO_MOVIL", {}).isNotEmpty())
                Layout::compactFlag() = true;
           #endif
            mainWindow = std::make_unique<MainWindow> ("Studoo");
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
