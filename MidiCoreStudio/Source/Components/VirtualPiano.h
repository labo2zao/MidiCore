/**
 * @file VirtualPiano.h
 * @brief Virtual Piano Component - MIDI keyboard interface
 */

#pragma once

#include <JuceHeader.h>

class VirtualPiano : public juce::Component,
                     private juce::MidiKeyboardStateListener
{
public:
    VirtualPiano();
    ~VirtualPiano() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Get the keyboard state for MIDI output
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }
    
    // Set MIDI output callback
    std::function<void(const juce::MidiMessage&)> onMidiMessage;

private:
    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    
    juce::MidiKeyboardState keyboardState;
    std::unique_ptr<juce::MidiKeyboardComponent> keyboardComponent;
    
    juce::ComboBox channelSelector;
    juce::Label channelLabel;
    juce::Slider velocitySlider;
    juce::Label velocityLabel;
    juce::ComboBox octaveSelector;
    juce::Label octaveLabel;
    
    int currentChannel{1};
    int currentOctave{4};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualPiano)
};
