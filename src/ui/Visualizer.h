#pragma once
// ============================================================================
//  Visualizer: animaciones que se mueven con la música (barras, círculo y
//  ondas). Se ven en directo mientras suena y se pueden exportar a un vídeo
//  (.avi con la canción) para subirlo a YouTube, TikTok o Instagram.
// ============================================================================
#include "AppContext.h"
#include "Widgets.h"

namespace studoo
{
    namespace visual
    {
        constexpr int numBands = 48;
        using Bands = std::array<float, numBands>;

        // Bandas de frecuencia (0..1) de un trozo de sonido mono (graves primero)
        void analyse (const float* mono, int num, double sampleRate, Bands& out);

        // Dibuja un fotograma. 'level' = volumen (0..1), 'time' = segundos desde el principio
        void draw (juce::Graphics&, juce::Rectangle<float>, const Bands&, float level, double time,
                   const juce::String& title, int style, juce::Colour colour);

        // Escribe un vídeo AVI (MJPEG + sonido PCM) con la animación y el audio del WAV
        bool writeVideo (const juce::File& wav, const juce::File& avi, const juce::String& title, int style,
                         juce::Colour colour, int width, int height, int fps,
                         std::function<bool (float)> progress, juce::String& error);
    }

    class VisualizerPanel : public juce::Component, private juce::Timer
    {
    public:
        explicit VisualizerPanel (AppContext&);
        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void timerCallback() override;
        void exportVideo();

        AppContext& ctx;
        juce::ComboBox styleBox, colourBox, sizeBox;
        juce::TextButton exportButton;
        visual::Bands bands {};
        float level = 0.0f;
        double startMs = juce::Time::getMillisecondCounterHiRes();
        std::unique_ptr<juce::FileChooser> chooser;
        juce::String status;
    };
}
