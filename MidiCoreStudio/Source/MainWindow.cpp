/**
 * @file MainWindow.cpp
 * @brief MidiCore Studio - Main application window implementation
 */

#include "MainWindow.h"

MainWindow::MainWindow(juce::String name)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                         .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    
    // Create tabbed component
    tabbedComponent = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    
    // Create all components
    deviceManager = std::make_unique<DeviceManager>();
    midiMonitor = std::make_unique<MidiMonitor>();
    virtualPiano = std::make_unique<VirtualPiano>();
    ccController = std::make_unique<CCController>();
    fileManager = std::make_unique<FileManagerComponent>();
    terminal = std::make_unique<Terminal>();
    firmwareUpdater = std::make_unique<FirmwareUpdater>();
    
    // Add tabs in logical order
    tabbedComponent->addTab("Device Manager", juce::Colours::lightblue, deviceManager.get(), false);
    tabbedComponent->addTab("MIDI Monitor", juce::Colours::lightgreen, midiMonitor.get(), false);
    tabbedComponent->addTab("Virtual Piano", juce::Colours::lightcyan, virtualPiano.get(), false);
    tabbedComponent->addTab("CC Controller", juce::Colours::lightyellow, ccController.get(), false);
    tabbedComponent->addTab("File Manager", juce::Colours::lightgrey, fileManager.get(), false);
    tabbedComponent->addTab("Terminal", juce::Colours::lightcoral, terminal.get(), false);
    tabbedComponent->addTab("Firmware Update", juce::Colours::lightsalmon, firmwareUpdater.get(), false);
    
    setContentOwned(tabbedComponent.get(), true);
    
    // Window properties
    setResizable(true, true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    
    // Set reasonable default size
    setSize(1200, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
