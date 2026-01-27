/**
 * @file FirmwareUpdater.cpp
 * @brief Firmware Updater implementation - MIOS32 bootloader protocol
 */

#include "FirmwareUpdater.h"

FirmwareUpdater::FirmwareUpdater()
{
    // File selection
    addAndMakeVisible(selectFileButton);
    selectFileButton.setButtonText("Select Firmware (.hex)");
    selectFileButton.onClick = [this] { selectFirmwareFile(); };
    
    addAndMakeVisible(fileLabel);
    fileLabel.setText("No file selected", juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    
    // Bootloader controls
    addAndMakeVisible(enterBootloaderButton);
    enterBootloaderButton.setButtonText("Enter Bootloader");
    enterBootloaderButton.onClick = [this] { enterBootloader(); };
    
    addAndMakeVisible(queryButton);
    queryButton.setButtonText("Query Bootloader");
    queryButton.onClick = [this] { queryBootloaderInfo(); };
    
    addAndMakeVisible(startButton);
    startButton.setButtonText("Start Update");
    startButton.setEnabled(false);
    startButton.onClick = [this] { startUpdate(); };
    
    addAndMakeVisible(cancelButton);
    cancelButton.setButtonText("Cancel");
    cancelButton.setEnabled(false);
    cancelButton.onClick = [this] { cancelUpdate(); };
    
    // Progress bar
    addAndMakeVisible(progressBar);
    progressBar.setPercentageDisplay(true);
    
    // Log display
    addAndMakeVisible(logDisplay);
    logDisplay.setMultiLine(true);
    logDisplay.setReadOnly(true);
    logDisplay.setScrollbarsShown(true);
    logDisplay.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain));
    logDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    logDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
    
    // Status labels
    addAndMakeVisible(statusLabel);
    statusLabel.setText("Ready", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    
    addAndMakeVisible(bootloaderInfoLabel);
    bootloaderInfoLabel.setText("Bootloader: Not detected", juce::dontSendNotification);
    bootloaderInfoLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    
    // Welcome message
    addLog("MidiCore Firmware Updater", juce::Colours::cyan);
    addLog("MIOS32-compatible bootloader protocol", juce::Colours::grey);
    addLog("");
    addLog("Instructions:", juce::Colours::yellow);
    addLog("1. Select firmware file (.hex)", juce::Colours::white);
    addLog("2. Enter bootloader mode (or device will auto-detect)", juce::Colours::white);
    addLog("3. Query bootloader info to verify connection", juce::Colours::white);
    addLog("4. Click 'Start Update' to flash firmware", juce::Colours::white);
    addLog("");
    
    startTimer(500);
}

FirmwareUpdater::~FirmwareUpdater()
{
    stopTimer();
}

void FirmwareUpdater::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw warning box if updating
    if (currentState != UpdateState::Idle && currentState != UpdateState::Complete)
    {
        g.setColour(juce::Colours::orange);
        g.drawRect(getLocalBounds().reduced(5), 3);
    }
}

void FirmwareUpdater::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top controls
    auto topArea = area.removeFromTop(30);
    selectFileButton.setBounds(topArea.removeFromLeft(180));
    topArea.removeFromLeft(10);
    fileLabel.setBounds(topArea);
    
    area.removeFromTop(10);
    
    // Bootloader controls
    auto controlsArea = area.removeFromTop(30);
    enterBootloaderButton.setBounds(controlsArea.removeFromLeft(150));
    controlsArea.removeFromLeft(10);
    queryButton.setBounds(controlsArea.removeFromLeft(150));
    controlsArea.removeFromLeft(10);
    startButton.setBounds(controlsArea.removeFromLeft(120));
    controlsArea.removeFromLeft(10);
    cancelButton.setBounds(controlsArea.removeFromLeft(80));
    
    area.removeFromTop(10);
    
    // Status info
    auto statusArea = area.removeFromTop(50);
    statusLabel.setBounds(statusArea.removeFromTop(25));
    bootloaderInfoLabel.setBounds(statusArea);
    
    area.removeFromTop(10);
    
    // Progress bar
    progressBar.setBounds(area.removeFromTop(25));
    
    area.removeFromTop(10);
    
    // Log display
    logDisplay.setBounds(area);
}

void FirmwareUpdater::timerCallback()
{
    // Update progress bar
    progressBar.setProgress(progress);
    
    // TODO: Check for bootloader presence
    // For now, simulate detection
    static int tickCount = 0;
    tickCount++;
    
    if (tickCount == 5 && !isInBootloader)
    {
        // Simulate bootloader detection
        // TODO: Implement actual MIDI SysEx detection
    }
}

void FirmwareUpdater::selectFirmwareFile()
{
    juce::FileChooser chooser("Select Firmware File", juce::File(), "*.hex");
    
    if (chooser.browseForFileToOpen())
    {
        firmwareFile = chooser.getResult();
        fileLabel.setText(firmwareFile.getFileName(), juce::dontSendNotification);
        fileLabel.setColour(juce::Label::textColourId, juce::Colours::green);
        
        addLog("Selected file: " + firmwareFile.getFullPathName(), juce::Colours::green);
        
        // Enable start button if bootloader detected
        if (isInBootloader)
            startButton.setEnabled(true);
    }
}

void FirmwareUpdater::startUpdate()
{
    if (!firmwareFile.existsAsFile())
    {
        addLog("ERROR: No firmware file selected", juce::Colours::red);
        return;
    }
    
    if (!isInBootloader)
    {
        addLog("ERROR: Device not in bootloader mode", juce::Colours::red);
        return;
    }
    
    currentState = UpdateState::EraseFlash;
    updateStatus("Erasing flash...", juce::Colours::orange);
    progress = 0.0;
    
    startButton.setEnabled(false);
    cancelButton.setEnabled(true);
    
    addLog("", juce::Colours::white);
    addLog("===== Starting Firmware Update =====", juce::Colours::cyan);
    addLog("Firmware file: " + firmwareFile.getFileName(), juce::Colours::white);
    addLog("Size: " + juce::String(firmwareFile.getSize() / 1024) + " KB", juce::Colours::white);
    addLog("", juce::Colours::white);
    
    // TODO: Implement actual flash programming via MIOS32 bootloader protocol
    // Protocol:
    // 1. Send erase command via MIDI SysEx
    // 2. Wait for erase complete
    // 3. Send firmware data in blocks
    // 4. Verify each block
    // 5. Send reboot command
    
    addLog("Phase 1: Erasing flash memory...", juce::Colours::yellow);
    progress = 0.1;
}

void FirmwareUpdater::cancelUpdate()
{
    if (currentState == UpdateState::Idle || currentState == UpdateState::Complete)
        return;
    
    currentState = UpdateState::Idle;
    updateStatus("Update cancelled", juce::Colours::red);
    progress = 0.0;
    
    startButton.setEnabled(true);
    cancelButton.setEnabled(false);
    
    addLog("Update cancelled by user", juce::Colours::red);
}

void FirmwareUpdater::enterBootloader()
{
    addLog("Sending bootloader entry command...", juce::Colours::yellow);
    
    // TODO: Send MIOS32 bootloader entry command via MIDI SysEx
    // SysEx: F0 00 00 7E 48 [device_id] 00 01 F7
    
    addLog("Waiting for bootloader response...", juce::Colours::yellow);
    
    // Simulate bootloader entry
    isInBootloader = true;
    bootloaderVersion = "MIOS32 Bootloader v1.0";
    bootloaderInfoLabel.setText("Bootloader: " + bootloaderVersion, juce::dontSendNotification);
    bootloaderInfoLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    
    if (firmwareFile.existsAsFile())
        startButton.setEnabled(true);
    
    addLog("Bootloader detected: " + bootloaderVersion, juce::Colours::green);
}

void FirmwareUpdater::queryBootloaderInfo()
{
    if (!isInBootloader)
    {
        addLog("Querying bootloader...", juce::Colours::yellow);
        
        // TODO: Send query command via MIDI SysEx
        // SysEx: F0 00 00 7E 48 [device_id] 00 0F F7 (query)
        
        addLog("No bootloader response - device may not be in bootloader mode", juce::Colours::red);
        return;
    }
    
    addLog("Bootloader Info:", juce::Colours::cyan);
    addLog("  Version: " + bootloaderVersion, juce::Colours::white);
    addLog("  Flash size: 1024 KB", juce::Colours::white);
    addLog("  Bootloader size: 32 KB", juce::Colours::white);
    addLog("  Application start: 0x08008000", juce::Colours::white);
    addLog("  Device ID: STM32F407VGT6", juce::Colours::white);
}

void FirmwareUpdater::addLog(const juce::String& message, juce::Colour colour)
{
    auto timestamp = juce::Time::getCurrentTime().toString(true, true, true, true);
    auto formattedMessage = "[" + timestamp + "] " + message + "\n";
    
    logDisplay.setColour(juce::TextEditor::textColourId, colour);
    logDisplay.moveCaretToEnd();
    logDisplay.insertTextAtCaret(formattedMessage);
}

void FirmwareUpdater::updateStatus(const juce::String& status, juce::Colour colour)
{
    statusLabel.setText(status, juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, colour);
}
