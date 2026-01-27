/**
 * @file CCController.h
 * @brief CC (Control Change) Sender Component
 */

#pragma once

#include <JuceHeader.h>

class CCController : public juce::Component
{
public:
    CCController();
    ~CCController() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Set MIDI output callback
    std::function<void(const juce::MidiMessage&)> onMidiMessage;

private:
    struct CCSlider
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::Label> valueLabel;
        int ccNumber;
    };
    
    std::vector<CCSlider> ccSliders;
    
    juce::ComboBox channelSelector;
    juce::Label channelLabel;
    juce::TextButton sendAllButton;
    juce::TextButton resetAllButton;
    
    juce::Viewport viewport;
    juce::Component sliderContainer;
    
    int currentChannel{1};
    
    void createCCSlider(int ccNumber, const juce::String& name);
    void sendCC(int ccNumber, int value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCController)
};
