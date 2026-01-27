/**
 * @file MidiMonitor.h
 * @brief MIDI Monitor Component - Displays real-time MIDI messages
 */

#pragma once

#include <JuceHeader.h>

class MidiMonitor : public juce::Component,
                    private juce::Timer
{
public:
    MidiMonitor();
    ~MidiMonitor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    juce::TextEditor messageDisplay;
    juce::TextButton clearButton;
    juce::ToggleButton autoScrollButton;
    
    std::vector<juce::String> midiMessages;
    juce::CriticalSection messageLock;

    void addMidiMessage(const juce::String& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMonitor)
};
