/**
 * @file MainWindow.h
 * @brief MidiCore Studio - Main application window
 */

#pragma once

#include <JuceHeader.h>
#include "Components/MidiMonitor.h"
#include "Components/FileManagerComponent.h"
#include "Components/VirtualPiano.h"
#include "Components/CCController.h"
#include "Components/Terminal.h"
#include "Components/DeviceManager.h"
#include "Components/FirmwareUpdater.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    std::unique_ptr<juce::TabbedComponent> tabbedComponent;
    
    // Components
    std::unique_ptr<DeviceManager> deviceManager;
    std::unique_ptr<MidiMonitor> midiMonitor;
    std::unique_ptr<VirtualPiano> virtualPiano;
    std::unique_ptr<CCController> ccController;
    std::unique_ptr<FileManagerComponent> fileManager;
    std::unique_ptr<Terminal> terminal;
    std::unique_ptr<FirmwareUpdater> firmwareUpdater;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
