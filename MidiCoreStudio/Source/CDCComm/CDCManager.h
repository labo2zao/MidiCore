/**
 * @file CDCManager.h
 * @brief CDC (Virtual COM Port) Manager - Handles USB CDC communication
 */

#pragma once

#include <JuceHeader.h>

class CDCManager
{
public:
    CDCManager();
    ~CDCManager();

    bool isConnected() const;
    bool connect(const juce::String& portName);
    void disconnect();
    
    bool sendCommand(const juce::String& command);
    juce::String receiveResponse(int timeoutMs = 1000);
    
    // File protocol
    juce::StringArray listFiles();
    juce::String getFile(const juce::String& filename);
    bool putFile(const juce::String& filename, const juce::String& content);
    bool deleteFile(const juce::String& filename);

private:
    bool connected{false};
    juce::String portName;
    
    // TODO: Implement serial port communication
    // Options: JUCE SerialPort (if available) or platform-specific
};
