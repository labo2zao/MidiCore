cmake_minimum_required(VERSION 3.15)

project(MidiCoreStudio VERSION 1.0.0)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE setup
# User should set JUCE_PATH or have JUCE installed system-wide
if(NOT DEFINED JUCE_PATH)
    find_package(JUCE CONFIG REQUIRED)
else()
    add_subdirectory(${JUCE_PATH} JUCE)
endif()

# Project sources
set(SOURCE_FILES
    Source/Main.cpp
    Source/MainWindow.cpp
    Source/MainWindow.h
    Source/MidiIO/MidiManager.cpp
    Source/MidiIO/MidiManager.h
    Source/CDCComm/CDCManager.cpp
    Source/CDCComm/CDCManager.h
    Source/Components/MidiMonitor.cpp
    Source/Components/MidiMonitor.h
    Source/Components/FileManagerComponent.cpp
    Source/Components/FileManagerComponent.h
)

# Create executable
juce_add_gui_app(MidiCoreStudio
    PRODUCT_NAME "MidiCore Studio"
    COMPANY_NAME "MidiCore Project"
    BUNDLE_ID "com.midicore.studio"
    VERSION ${PROJECT_VERSION}
)

# Add sources
target_sources(MidiCoreStudio PRIVATE ${SOURCE_FILES})

# JUCE modules
target_link_libraries(MidiCoreStudio
    PRIVATE
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Preprocessor definitions
target_compile_definitions(MidiCoreStudio
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MidiCoreStudio,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MidiCoreStudio,JUCE_VERSION>"
)

# Binary directory
juce_generate_juce_header(MidiCoreStudio)

# Install target
install(TARGETS MidiCoreStudio
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
