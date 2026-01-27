/**
 * @file FileManagerComponent.cpp
 * @brief SD Card File Manager implementation
 */

#include "FileManagerComponent.h"

FileManagerComponent::FileManagerComponent()
{
    // File list
    fileListModel = std::make_unique<FileListModel>(*this);
    addAndMakeVisible(fileList);
    fileList.setModel(fileListModel.get());
    fileList.setColour(juce::ListBox::backgroundColourId, juce::Colours::white);
    
    // File editor
    addAndMakeVisible(fileEditor);
    fileEditor.setMultiLine(true);
    fileEditor.setReturnKeyStartsNewLine(true);
    fileEditor.setScrollbarsShown(true);
    fileEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    
    // Refresh button
    addAndMakeVisible(refreshButton);
    refreshButton.setButtonText("Refresh");
    refreshButton.onClick = [this] { refreshFileList(); };
    
    // Save button
    addAndMakeVisible(saveButton);
    saveButton.setButtonText("Save");
    saveButton.setEnabled(false);
    saveButton.onClick = [this] {
        if (currentFile.isNotEmpty())
        {
            uploadFile(currentFile, fileEditor.getText());
        }
    };
    
    // Status label
    addAndMakeVisible(statusLabel);
    statusLabel.setText("Status: Ready", juce::dontSendNotification);
    
    // Connection label
    addAndMakeVisible(connectionLabel);
    connectionLabel.setText("Device: Not Connected", juce::dontSendNotification);
    connectionLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    
    // Start timer
    startTimer(1000);
}

FileManagerComponent::~FileManagerComponent()
{
    stopTimer();
}

void FileManagerComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void FileManagerComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top controls
    auto controlsArea = area.removeFromTop(30);
    refreshButton.setBounds(controlsArea.removeFromLeft(80));
    controlsArea.removeFromLeft(10);
    saveButton.setBounds(controlsArea.removeFromLeft(80));
    controlsArea.removeFromLeft(20);
    connectionLabel.setBounds(controlsArea.removeFromLeft(200));
    
    area.removeFromTop(10);
    
    // Main content - split horizontally
    auto leftArea = area.removeFromLeft(area.getWidth() / 3);
    area.removeFromLeft(10);
    
    // File list on left
    fileList.setBounds(leftArea);
    
    // Editor on right
    auto editorArea = area;
    editorArea.removeFromBottom(30);
    fileEditor.setBounds(editorArea);
    
    // Status at bottom
    area.removeFromTop(editorArea.getHeight() + 10);
    statusLabel.setBounds(area);
}

void FileManagerComponent::timerCallback()
{
    // Check CDC connection status
    // TODO: Implement actual CDC connection check via CDCManager
    // For now returns false until serial port implementation is complete
    bool connected = false; // Placeholder - requires serial port enumeration
    
    if (connected != isConnected)
    {
        isConnected = connected;
        connectionLabel.setText(isConnected ? "Device: Connected" : "Device: Not Connected",
                              juce::dontSendNotification);
        connectionLabel.setColour(juce::Label::textColourId,
                                isConnected ? juce::Colours::green : juce::Colours::red);
        
        if (isConnected)
        {
            refreshFileList();
        }
    }
}

void FileManagerComponent::refreshFileList()
{
    statusLabel.setText("Status: Refreshing file list...", juce::dontSendNotification);
    
    // TODO: Implement CDC protocol to list files
    // For now, show placeholder
    currentFiles.clear();
    currentFiles.add("default.cfg");
    currentFiles.add("patch1.ngp");
    currentFiles.add("patch2.ngp");
    currentFiles.add("zones.ngc");
    
    fileList.updateContent();
    statusLabel.setText("Status: " + juce::String(currentFiles.size()) + " files found", 
                       juce::dontSendNotification);
}

void FileManagerComponent::downloadFile(const juce::String& filename)
{
    statusLabel.setText("Status: Downloading " + filename + "...", juce::dontSendNotification);
    
    // TODO: Implement CDC protocol to get file
    // For now, show placeholder
    currentFile = filename;
    fileEditor.setText("# " + filename + "\n# Content will be loaded from device via CDC\n");
    saveButton.setEnabled(true);
    
    statusLabel.setText("Status: Loaded " + filename, juce::dontSendNotification);
}

void FileManagerComponent::uploadFile(const juce::String& filename, const juce::String& content)
{
    statusLabel.setText("Status: Uploading " + filename + "...", juce::dontSendNotification);
    
    // TODO: Implement CDC protocol to put file
    
    statusLabel.setText("Status: Saved " + filename, juce::dontSendNotification);
}
