/**
 * @file CCController.cpp
 * @brief CC Controller implementation
 */

#include "CCController.h"

CCController::CCController()
{
    // Channel selector
    addAndMakeVisible(channelLabel);
    channelLabel.setText("MIDI Channel:", juce::dontSendNotification);
    
    addAndMakeVisible(channelSelector);
    for (int i = 1; i <= 16; i++)
        channelSelector.addItem("Channel " + juce::String(i), i);
    channelSelector.setSelectedId(1);
    channelSelector.onChange = [this] {
        currentChannel = channelSelector.getSelectedId();
    };
    
    // Buttons
    addAndMakeVisible(sendAllButton);
    sendAllButton.setButtonText("Send All");
    sendAllButton.onClick = [this] {
        for (auto& cc : ccSliders)
            sendCC(cc.ccNumber, (int)cc.slider->getValue());
    };
    
    addAndMakeVisible(resetAllButton);
    resetAllButton.setButtonText("Reset All");
    resetAllButton.onClick = [this] {
        for (auto& cc : ccSliders)
        {
            cc.slider->setValue(0, juce::dontSendNotification);
            sendCC(cc.ccNumber, 0);
        }
    };
    
    // Create common CC controllers
    createCCSlider(1, "Modulation");
    createCCSlider(7, "Volume");
    createCCSlider(10, "Pan");
    createCCSlider(11, "Expression");
    createCCSlider(64, "Sustain");
    createCCSlider(71, "Resonance");
    createCCSlider(74, "Brightness");
    createCCSlider(91, "Reverb");
    createCCSlider(93, "Chorus");
    
    // Add custom CCs (for accordion)
    createCCSlider(2, "Breath");
    createCCSlider(20, "Bellows");
    createCCSlider(21, "Register 1");
    createCCSlider(22, "Register 2");
    createCCSlider(23, "Register 3");
    createCCSlider(24, "Register 4");
    
    // Setup viewport
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&sliderContainer, false);
    sliderContainer.setSize(800, ccSliders.size() * 40);
}

CCController::~CCController()
{
}

void CCController::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void CCController::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top controls
    auto controlsArea = area.removeFromTop(30);
    channelLabel.setBounds(controlsArea.removeFromLeft(100));
    channelSelector.setBounds(controlsArea.removeFromLeft(120));
    controlsArea.removeFromLeft(20);
    sendAllButton.setBounds(controlsArea.removeFromLeft(80));
    controlsArea.removeFromLeft(10);
    resetAllButton.setBounds(controlsArea.removeFromLeft(80));
    
    area.removeFromTop(10);
    
    // Viewport with sliders
    viewport.setBounds(area);
    
    // Layout sliders in container
    int yPos = 0;
    for (auto& cc : ccSliders)
    {
        cc.label->setBounds(10, yPos, 120, 30);
        cc.slider->setBounds(140, yPos, 400, 30);
        cc.valueLabel->setBounds(550, yPos, 50, 30);
        yPos += 40;
    }
}

void CCController::createCCSlider(int ccNumber, const juce::String& name)
{
    CCSlider cc;
    cc.ccNumber = ccNumber;
    
    cc.label = std::make_unique<juce::Label>();
    cc.label->setText(name + " (CC" + juce::String(ccNumber) + ")", juce::dontSendNotification);
    sliderContainer.addAndMakeVisible(*cc.label);
    
    cc.slider = std::make_unique<juce::Slider>();
    cc.slider->setRange(0, 127, 1);
    cc.slider->setValue(0);
    cc.slider->setSliderStyle(juce::Slider::LinearHorizontal);
    cc.slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cc.slider->onValueChange = [this, ccNumber, &cc] {
        int value = (int)cc.slider->getValue();
        cc.valueLabel->setText(juce::String(value), juce::dontSendNotification);
        sendCC(ccNumber, value);
    };
    sliderContainer.addAndMakeVisible(*cc.slider);
    
    cc.valueLabel = std::make_unique<juce::Label>();
    cc.valueLabel->setText("0", juce::dontSendNotification);
    cc.valueLabel->setJustificationType(juce::Justification::centred);
    sliderContainer.addAndMakeVisible(*cc.valueLabel);
    
    ccSliders.push_back(std::move(cc));
}

void CCController::sendCC(int ccNumber, int value)
{
    auto message = juce::MidiMessage::controllerEvent(currentChannel, ccNumber, value);
    
    if (onMidiMessage)
        onMidiMessage(message);
}
