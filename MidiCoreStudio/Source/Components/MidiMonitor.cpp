/**
 * @file MidiMonitor.cpp
 * @brief MIDI Monitor Component implementation
 */

#include "MidiMonitor.h"

MidiMonitor::MidiMonitor()
{
    // Message display
    addAndMakeVisible(messageDisplay);
    messageDisplay.setMultiLine(true);
    messageDisplay.setReadOnly(true);
    messageDisplay.setScrollbarsShown(true);
    messageDisplay.setCaretVisible(false);
    messageDisplay.setPopupMenuEnabled(true);
    messageDisplay.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    
    // Clear button
    addAndMakeVisible(clearButton);
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] { 
        messageDisplay.clear();
        midiMessages.clear();
    };
    
    // Auto-scroll button
    addAndMakeVisible(autoScrollButton);
    autoScrollButton.setButtonText("Auto Scroll");
    autoScrollButton.setToggleState(true, juce::dontSendNotification);
    
    // Add welcome message
    addMidiMessage("MidiCore Studio - MIDI Monitor");
    addMidiMessage("Waiting for MIDI device...");
    
    // Start timer for updates
    startTimer(100);
}

MidiMonitor::~MidiMonitor()
{
    stopTimer();
}

void MidiMonitor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MidiMonitor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top controls
    auto controlsArea = area.removeFromTop(30);
    clearButton.setBounds(controlsArea.removeFromLeft(80));
    controlsArea.removeFromLeft(10);
    autoScrollButton.setBounds(controlsArea.removeFromLeft(100));
    
    area.removeFromTop(10);
    
    // Message display
    messageDisplay.setBounds(area);
}

void MidiMonitor::timerCallback()
{
    // Update display with new messages
    if (autoScrollButton.getToggleState())
    {
        messageDisplay.moveCaretToEnd();
    }
}

void MidiMonitor::addMidiMessage(const juce::String& message)
{
    juce::ScopedLock lock(messageLock);
    
    auto timestamp = juce::Time::getCurrentTime().toString(true, true, true, true);
    auto formattedMessage = "[" + timestamp + "] " + message + "\n";
    
    messageDisplay.insertTextAtCaret(formattedMessage);
    midiMessages.push_back(formattedMessage);
    
    // Limit message history
    if (midiMessages.size() > 1000)
    {
        midiMessages.erase(midiMessages.begin());
    }
}
