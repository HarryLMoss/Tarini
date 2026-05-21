# =============================================================================
#
#   CMakeLists.txt
#   Author: Harry Moss
#   Created: 21st May 2026
#
#   Modern JUCE CMake build configuration for Tarini.
#
# =============================================================================

cmake_minimum_required(VERSION 3.22)

project(Tarini VERSION 0.2.0)

set(JUCE_DIR "C:/Users/harry/Dev/JUCE"
    CACHE PATH "Path to JUCE source")

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(Tarini
    COMPANY_NAME "HarryMoss"
    PLUGIN_MANUFACTURER_CODE Hmos
    PLUGIN_CODE Tari

    FORMATS Standalone VST3

    PRODUCT_NAME "Tarini"

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
)

target_sources(Tarini PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DroneVoice.cpp
)

juce_generate_juce_header(Tarini)

target_compile_features(Tarini PRIVATE cxx_std_17)

target_link_libraries(Tarini PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_devices
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(Tarini PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

enable_testing()

add_subdirectory(tests)