/**
 * @file MidiManager.cpp
 * @brief MIDI Manager implementation
 */

#include "MidiManager.h"

MidiManager::MidiManager()
{
}

MidiManager::~MidiManager()
{
    closeMidiDevice();
}

void MidiManager::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    juce::ignoreUnused(source);
    
    // Forward to callback if registered
    if (onMidiMessage)
    {
        onMidiMessage(message);
    }
}

bool MidiManager::openMidiDevice(const juce::String& deviceName)
{
    closeMidiDevice();
    
    // Find and open MIDI input
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (const auto& device : midiInputs)
    {
        if (device.name == deviceName)
        {
            midiInput = juce::MidiInput::openDevice(device.identifier, this);
            if (midiInput)
            {
                midiInput->start();
            }
            break;
        }
    }
    
    // Find and open MIDI output
    auto midiOutputs = juce::MidiOutput::getAvailableDevices();
    for (const auto& device : midiOutputs)
    {
        if (device.name == deviceName)
        {
            midiOutput = juce::MidiOutput::openDevice(device.identifier);
            break;
        }
    }
    
    return (midiInput != nullptr || midiOutput != nullptr);
}

void MidiManager::closeMidiDevice()
{
    if (midiInput)
    {
        midiInput->stop();
        midiInput.reset();
    }
    
    midiOutput.reset();
}

bool MidiManager::sendMidiMessage(const juce::MidiMessage& message)
{
    if (midiOutput)
    {
        midiOutput->sendMessageNow(message);
        return true;
    }
    return false;
}

juce::StringArray MidiManager::getAvailableMidiDevices()
{
    juce::StringArray devices;
    
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (const auto& device : midiInputs)
    {
        devices.add(device.name);
    }
    
    return devices;
}
