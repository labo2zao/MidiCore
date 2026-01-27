/**
 * @file Terminal.h
 * @brief Terminal Component - Command line interface
 */

#pragma once

#include <JuceHeader.h>

class Terminal : public juce::Component
{
public:
    Terminal();
    ~Terminal() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void addOutput(const juce::String& text, juce::Colour colour = juce::Colours::white);
    void clear();
    
    // Callback for command execution
    std::function<void(const juce::String&)> onCommand;

private:
    juce::TextEditor outputDisplay;
    juce::TextEditor inputField;
    juce::TextButton sendButton;
    juce::TextButton clearButton;
    
    std::vector<juce::String> commandHistory;
    int historyIndex{-1};
    
    void executeCommand(const juce::String& command);
    void navigateHistory(bool up);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Terminal)
};
