/**
 * @file DeviceManager.cpp
 * @brief Device Manager implementation
 */

#include "DeviceManager.h"

DeviceManager::DeviceManager()
{
    // Device list
    listModel = std::make_unique<DeviceListModel>(*this);
    addAndMakeVisible(deviceList);
    deviceList.setModel(listModel.get());
    deviceList.setColour(juce::ListBox::backgroundColourId, juce::Colours::white);
    
    // Refresh button
    addAndMakeVisible(refreshButton);
    refreshButton.setButtonText("Refresh");
    refreshButton.onClick = [this] { refreshDeviceList(); };
    
    // Connect button
    addAndMakeVisible(connectButton);
    connectButton.setButtonText("Connect");
    connectButton.onClick = [this] {
        if (!selectedDevice.isEmpty() && onDeviceSelected)
        {
            onDeviceSelected(selectedDevice);
            isConnected = true;
            deviceList.repaint();
            statusLabel.setText("Connected to: " + selectedDevice, juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
        }
    };
    
    // Query button
    addAndMakeVisible(queryButton);
    queryButton.setButtonText("Query Info");
    queryButton.onClick = [this] { queryDeviceInfo(); };
    
    // Device info display
    addAndMakeVisible(deviceInfo);
    deviceInfo.setMultiLine(true);
    deviceInfo.setReadOnly(true);
    deviceInfo.setScrollbarsShown(true);
    deviceInfo.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain));
    
    // Status label
    addAndMakeVisible(statusLabel);
    statusLabel.setText("No device connected", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    
    // Initial refresh
    refreshDeviceList();
    
    // Start timer for periodic refresh
    startTimer(2000); // Every 2 seconds
}

DeviceManager::~DeviceManager()
{
    stopTimer();
}

void DeviceManager::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void DeviceManager::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top buttons
    auto buttonArea = area.removeFromTop(30);
    refreshButton.setBounds(buttonArea.removeFromLeft(80));
    buttonArea.removeFromLeft(10);
    connectButton.setBounds(buttonArea.removeFromLeft(80));
    buttonArea.removeFromLeft(10);
    queryButton.setBounds(buttonArea.removeFromLeft(100));
    
    area.removeFromTop(10);
    
    // Status
    auto statusArea = area.removeFromTop(25);
    statusLabel.setBounds(statusArea);
    
    area.removeFromTop(10);
    
    // Split area vertically
    auto leftArea = area.removeFromLeft(area.getWidth() / 2);
    area.removeFromLeft(10);
    
    // Device list on left
    deviceList.setBounds(leftArea);
    
    // Device info on right
    deviceInfo.setBounds(area);
}

void DeviceManager::timerCallback()
{
    // Periodic device list refresh (silently)
    // TODO: Implement actual device enumeration
}

void DeviceManager::refreshDeviceList()
{
    statusLabel.setText("Refreshing devices...", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    
    // TODO: Implement actual MIDI device enumeration
    // For now, show placeholder devices
    availableDevices.clear();
    availableDevices.add("MidiCore 4x4");
    availableDevices.add("MidiCore Accordion #1");
    availableDevices.add("MidiCore Accordion #2");
    
    deviceList.updateContent();
    
    statusLabel.setText("Found " + juce::String(availableDevices.size()) + " device(s)", 
                       juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    
    if (onRefreshDevices)
        onRefreshDevices();
}

void DeviceManager::queryDeviceInfo()
{
    if (selectedDevice.isEmpty())
    {
        deviceInfo.setText("No device selected");
        return;
    }
    
    // TODO: Query actual device info via MIDI SysEx
    juce::String info;
    info += "Device: " + selectedDevice + "\n";
    info += "Firmware: MidiCore v1.0.0\n";
    info += "Hardware: STM32F407VGT6\n";
    info += "MIDI Ports: 4 (USB MIDI 4x4)\n";
    info += "Features:\n";
    info += "  - USB MIDI (4 ports)\n";
    info += "  - USB CDC (Virtual COM Port)\n";
    info += "  - SD Card Storage\n";
    info += "  - Looper/Sequencer\n";
    info += "  - OLED Display\n";
    info += "\n";
    info += "Memory:\n";
    info += "  Flash: 1024 KB\n";
    info += "  RAM: 192 KB (128 + 64 CCMRAM)\n";
    info += "  SD Card: Available\n";
    
    deviceInfo.setText(info);
}
