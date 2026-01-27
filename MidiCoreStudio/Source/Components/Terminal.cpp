/**
 * @file Terminal.cpp
 * @brief Terminal implementation
 */

#include "Terminal.h"

Terminal::Terminal()
{
    // Output display
    addAndMakeVisible(outputDisplay);
    outputDisplay.setMultiLine(true);
    outputDisplay.setReadOnly(true);
    outputDisplay.setScrollbarsShown(true);
    outputDisplay.setCaretVisible(false);
    outputDisplay.setPopupMenuEnabled(true);
    outputDisplay.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    outputDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    outputDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
    
    // Input field
    addAndMakeVisible(inputField);
    inputField.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    inputField.onReturnKey = [this] {
        executeCommand(inputField.getText());
    };
    inputField.onEscapeKey = [this] {
        inputField.clear();
        historyIndex = -1;
    };
    
    // Handle up/down arrows for history
    inputField.onKeyPress = [this](const juce::KeyPress& key) {
        if (key == juce::KeyPress::upKey)
        {
            navigateHistory(true);
            return true;
        }
        else if (key == juce::KeyPress::downKey)
        {
            navigateHistory(false);
            return true;
        }
        return false;
    };
    
    // Send button
    addAndMakeVisible(sendButton);
    sendButton.setButtonText("Send");
    sendButton.onClick = [this] {
        executeCommand(inputField.getText());
    };
    
    // Clear button
    addAndMakeVisible(clearButton);
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] { clear(); };
    
    // Welcome message
    addOutput("MidiCore Studio Terminal", juce::Colours::cyan);
    addOutput("Type 'help' for available commands", juce::Colours::grey);
    addOutput("", juce::Colours::white);
}

Terminal::~Terminal()
{
}

void Terminal::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Terminal::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Bottom input area
    auto inputArea = area.removeFromBottom(30);
    sendButton.setBounds(inputArea.removeFromRight(80));
    inputArea.removeFromRight(10);
    clearButton.setBounds(inputArea.removeFromRight(80));
    inputArea.removeFromRight(10);
    inputField.setBounds(inputArea);
    
    area.removeFromBottom(10);
    
    // Output display
    outputDisplay.setBounds(area);
}

void Terminal::addOutput(const juce::String& text, juce::Colour colour)
{
    auto timestamp = juce::Time::getCurrentTime().toString(true, true, true, true);
    auto formattedText = "[" + timestamp + "] " + text + "\n";
    
    outputDisplay.setColour(juce::TextEditor::textColourId, colour);
    outputDisplay.moveCaretToEnd();
    outputDisplay.insertTextAtCaret(formattedText);
}

void Terminal::clear()
{
    outputDisplay.clear();
    addOutput("Terminal cleared", juce::Colours::grey);
}

void Terminal::executeCommand(const juce::String& command)
{
    if (command.isEmpty())
        return;
    
    // Add to history
    commandHistory.insert(commandHistory.begin(), command);
    if (commandHistory.size() > 50)
        commandHistory.resize(50);
    historyIndex = -1;
    
    // Echo command
    addOutput("> " + command, juce::Colours::yellow);
    
    // Execute via callback
    if (onCommand)
        onCommand(command);
    
    // Clear input
    inputField.clear();
}

void Terminal::navigateHistory(bool up)
{
    if (commandHistory.empty())
        return;
    
    if (up)
    {
        if (historyIndex < (int)commandHistory.size() - 1)
        {
            historyIndex++;
            inputField.setText(commandHistory[historyIndex]);
        }
    }
    else
    {
        if (historyIndex > 0)
        {
            historyIndex--;
            inputField.setText(commandHistory[historyIndex]);
        }
        else if (historyIndex == 0)
        {
            historyIndex = -1;
            inputField.clear();
        }
    }
}
