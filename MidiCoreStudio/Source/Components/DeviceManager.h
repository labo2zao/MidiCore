/**
 * @file DeviceManager.h
 * @brief Device Manager Component - Device selection and info
 */

#pragma once

#include <JuceHeader.h>

class DeviceManager : public juce::Component,
                      private juce::Timer
{
public:
    DeviceManager();
    ~DeviceManager() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Callbacks
    std::function<void(const juce::String& deviceName)> onDeviceSelected;
    std::function<void()> onRefreshDevices;

private:
    void timerCallback() override;
    void refreshDeviceList();
    void queryDeviceInfo();
    
    juce::ListBox deviceList;
    juce::TextButton refreshButton;
    juce::TextButton connectButton;
    juce::TextButton queryButton;
    
    juce::TextEditor deviceInfo;
    juce::Label statusLabel;
    
    juce::StringArray availableDevices;
    juce::String selectedDevice;
    bool isConnected{false};
    
    class DeviceListModel : public juce::ListBoxModel
    {
    public:
        DeviceListModel(DeviceManager& owner) : owner(owner) {}
        
        int getNumRows() override { return owner.availableDevices.size(); }
        
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
        {
            if (selected)
                g.fillAll(juce::Colours::lightblue);
            
            g.setColour(juce::Colours::black);
            
            if (row >= 0 && row < owner.availableDevices.size())
            {
                auto text = owner.availableDevices[row];
                if (text == owner.selectedDevice && owner.isConnected)
                    text += " [CONNECTED]";
                
                g.drawText(text, 5, 0, width - 10, height, juce::Justification::centredLeft);
            }
        }
        
        void selectedRowsChanged(int lastRowSelected) override
        {
            if (lastRowSelected >= 0 && lastRowSelected < owner.availableDevices.size())
            {
                owner.selectedDevice = owner.availableDevices[lastRowSelected];
            }
        }
        
    private:
        DeviceManager& owner;
    };
    
    std::unique_ptr<DeviceListModel> listModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceManager)
};
