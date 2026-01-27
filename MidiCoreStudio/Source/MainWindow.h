/**
 * @file MainWindow.h
 * @brief MidiCore Studio - Main application window
 */

#pragma once

#include <JuceHeader.h>
#include "Components/MidiMonitor.h"
#include "Components/FileManagerComponent.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    std::unique_ptr<juce::TabbedComponent> tabbedComponent;
    std::unique_ptr<MidiMonitor> midiMonitor;
    std::unique_ptr<FileManagerComponent> fileManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
