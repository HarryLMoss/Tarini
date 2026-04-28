// ==========================================================
// Author: Harry Moss
// Date: 29.05.2024
// ==========================================================

#include <JuceHeader.h>
#include "MainComponent.h"

// ==========================================================
// Main JUCE Application Class
// ==========================================================

class TariniApplication : public juce::JUCEApplication
{
public:
    TariniApplication() {}

    const juce::String getApplicationName() override
    {
        return "Tarini";
    }

    const juce::String getApplicationVersion() override
    {
        return "1.0";
    }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(
                name,
                juce::Desktop::getInstance()
                    .getDefaultLookAndFeel()
                    .findColour(juce::ResizableWindow::backgroundColourId),
                juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);

            setContentOwned(new MainComponent(), true);

            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(TariniApplication)
