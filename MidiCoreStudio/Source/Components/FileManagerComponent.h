/**
 * @file FileManagerComponent.h
 * @brief SD Card File Manager - Browse and edit files via CDC
 */

#pragma once

#include <JuceHeader.h>

class FileManagerComponent : public juce::Component,
                              private juce::Timer
{
public:
    FileManagerComponent();
    ~FileManagerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshFileList();
    void downloadFile(const juce::String& filename);
    void uploadFile(const juce::String& filename, const juce::String& content);

    juce::ListBox fileList;
    juce::TextEditor fileEditor;
    juce::TextButton refreshButton;
    juce::TextButton saveButton;
    juce::Label statusLabel;
    juce::Label connectionLabel;
    
    juce::StringArray currentFiles;
    juce::String currentFile;
    bool isConnected{false};

    class FileListModel : public juce::ListBoxModel
    {
    public:
        FileListModel(FileManagerComponent& owner) : owner(owner) {}
        
        int getNumRows() override { return owner.currentFiles.size(); }
        
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
        {
            if (selected)
                g.fillAll(juce::Colours::lightblue);
            
            g.setColour(juce::Colours::black);
            g.drawText(owner.currentFiles[row], 5, 0, width - 10, height, juce::Justification::centredLeft);
        }
        
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override
        {
            if (row >= 0 && row < owner.currentFiles.size())
            {
                owner.downloadFile(owner.currentFiles[row]);
            }
        }
        
    private:
        FileManagerComponent& owner;
    };
    
    std::unique_ptr<FileListModel> fileListModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileManagerComponent)
};
