/**
 * @file FirmwareUpdater.h
 * @brief Firmware Update Tool - MidiCore-compatible bootloader interface
 */

#pragma once

#include <JuceHeader.h>

class FirmwareUpdater : public juce::Component,
                        private juce::Timer
{
public:
    FirmwareUpdater();
    ~FirmwareUpdater() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void selectFirmwareFile();
    void startUpdate();
    void cancelUpdate();
    void enterBootloader();
    void queryBootloaderInfo();
    
    enum class UpdateState
    {
        Idle,
        QueryingBootloader,
        EraseFlash,
        Writing,
        Verifying,
        Complete,
        Error
    };
    
    UpdateState currentState{UpdateState::Idle};
    
    juce::File firmwareFile;
    juce::TextButton selectFileButton;
    juce::Label fileLabel;
    
    juce::TextButton enterBootloaderButton;
    juce::TextButton queryButton;
    juce::TextButton startButton;
    juce::TextButton cancelButton;
    
    juce::ProgressBar progressBar;
    double progress{0.0};
    
    juce::TextEditor logDisplay;
    
    juce::Label statusLabel;
    juce::Label bootloaderInfoLabel;
    
    bool isInBootloader{false};
    juce::String bootloaderVersion;
    
    void addLog(const juce::String& message, juce::Colour colour = juce::Colours::white);
    void updateStatus(const juce::String& status, juce::Colour colour);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FirmwareUpdater)
};
