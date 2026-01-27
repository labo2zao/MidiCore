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
    
    // Create components
    midiMonitor = std::make_unique<MidiMonitor>();
    fileManager = std::make_unique<FileManagerComponent>();
    
    // Add tabs
    tabbedComponent->addTab("MIDI Monitor", juce::Colours::lightgrey, midiMonitor.get(), false);
    tabbedComponent->addTab("File Manager", juce::Colours::lightgrey, fileManager.get(), false);
    
    setContentOwned(tabbedComponent.get(), true);
    
    // Window properties
    setResizable(true, true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    
    // Set reasonable default size
    setSize(1024, 768);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
