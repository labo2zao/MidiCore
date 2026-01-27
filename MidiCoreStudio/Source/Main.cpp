/**
 * @file Main.cpp
 * @brief MidiCore Studio - Main application entry point
 */

#include <JuceHeader.h>
#include "MainWindow.h"

class MidiCoreStudioApplication : public juce::JUCEApplication
{
public:
    MidiCoreStudioApplication() {}

    const juce::String getApplicationName() override { return "MidiCore Studio"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// This macro generates the main() function
START_JUCE_APPLICATION(MidiCoreStudioApplication)
