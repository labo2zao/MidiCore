/**
 * @file VirtualPiano.cpp
 * @brief Virtual Piano Component implementation
 */

#include "VirtualPiano.h"

VirtualPiano::VirtualPiano()
{
    // Keyboard component
    keyboardComponent = std::make_unique<juce::MidiKeyboardComponent>(
        keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
    addAndMakeVisible(*keyboardComponent);
    
    // Channel selector
    addAndMakeVisible(channelLabel);
    channelLabel.setText("MIDI Channel:", juce::dontSendNotification);
    channelLabel.attachToComponent(&channelSelector, true);
    
    addAndMakeVisible(channelSelector);
    for (int i = 1; i <= 16; i++)
        channelSelector.addItem("Channel " + juce::String(i), i);
    channelSelector.setSelectedId(1);
    channelSelector.onChange = [this] {
        currentChannel = channelSelector.getSelectedId();
    };
    
    // Velocity slider
    addAndMakeVisible(velocityLabel);
    velocityLabel.setText("Velocity:", juce::dontSendNotification);
    velocityLabel.attachToComponent(&velocitySlider, true);
    
    addAndMakeVisible(velocitySlider);
    velocitySlider.setRange(1, 127, 1);
    velocitySlider.setValue(100);
    velocitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    velocitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    
    // Octave selector
    addAndMakeVisible(octaveLabel);
    octaveLabel.setText("Octave:", juce::dontSendNotification);
    octaveLabel.attachToComponent(&octaveSelector, true);
    
    addAndMakeVisible(octaveSelector);
    for (int i = -2; i <= 8; i++)
        octaveSelector.addItem("Octave " + juce::String(i), i + 3);
    octaveSelector.setSelectedId(6); // Octave 4
    octaveSelector.onChange = [this] {
        currentOctave = octaveSelector.getSelectedId() - 3;
        // Shift keyboard display
        keyboardComponent->setLowestVisibleKey(currentOctave * 12);
    };
    
    // Listen to keyboard state
    keyboardState.addListener(this);
}

VirtualPiano::~VirtualPiano()
{
    keyboardState.removeListener(this);
}

void VirtualPiano::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void VirtualPiano::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Controls at top
    auto controlsArea = area.removeFromTop(30);
    
    controlsArea.removeFromLeft(100); // Space for label
    channelSelector.setBounds(controlsArea.removeFromLeft(120));
    
    controlsArea.removeFromLeft(80); // Space for label
    velocitySlider.setBounds(controlsArea.removeFromLeft(200));
    
    controlsArea.removeFromLeft(80); // Space for label
    octaveSelector.setBounds(controlsArea.removeFromLeft(120));
    
    area.removeFromTop(10);
    
    // Keyboard takes remaining space
    keyboardComponent->setBounds(area);
}

void VirtualPiano::handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::ignoreUnused(source);
    
    auto message = juce::MidiMessage::noteOn(currentChannel, midiNoteNumber, 
                                            (uint8_t)(velocitySlider.getValue()));
    
    if (onMidiMessage)
        onMidiMessage(message);
}

void VirtualPiano::handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::ignoreUnused(source);
    
    auto message = juce::MidiMessage::noteOff(currentChannel, midiNoteNumber);
    
    if (onMidiMessage)
        onMidiMessage(message);
}
