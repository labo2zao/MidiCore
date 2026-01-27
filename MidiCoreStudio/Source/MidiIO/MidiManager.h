/**
 * @file MidiManager.h
 * @brief MIDI Manager - Handles MIDI I/O for MidiCore device
 */

#pragma once

#include <JuceHeader.h>

class MidiManager : public juce::MidiInputCallback
{
public:
    MidiManager();
    ~MidiManager() override;

    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    
    bool openMidiDevice(const juce::String& deviceName);
    void closeMidiDevice();
    
    bool sendMidiMessage(const juce::MidiMessage& message);
    
    juce::StringArray getAvailableMidiDevices();
    
    // Callback for MIDI message display
    std::function<void(const juce::MidiMessage&)> onMidiMessage;

private:
    std::unique_ptr<juce::MidiInput> midiInput;
    std::unique_ptr<juce::MidiOutput> midiOutput;
};
