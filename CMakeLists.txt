cmake_minimum_required(VERSION 3.22)

project(point_drone_vst VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")

add_subdirectory(JUCE)

juce_add_binary_data(PointDroneAssets
    SOURCES
        assets/Manrope-VariableFont_wght.ttf
)

juce_add_plugin(PointDrone
    COMPANY_NAME "PointDrone"
    BUNDLE_ID "com.pointdrone.vst"
    PLUGIN_MANUFACTURER_CODE Ptdr
    PLUGIN_CODE Ptd1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Point Drone"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
)

add_library(point_drone_vst_AU ALIAS PointDrone_AU)
add_library(point_drone_vst_VST3 ALIAS PointDrone_VST3)

set(PLUGINVAL_VST3_VALIDATOR OFF CACHE BOOL "" FORCE)
add_subdirectory(modules/pluginval)

juce_generate_juce_header(PointDrone)

target_sources(PointDrone
    PRIVATE
        source/audio/PointRenderer.cpp
        source/audio/PointRenderer.h
        source/audio/PointVoice.cpp
        source/audio/PointVoice.h
        source/controller/EditorController.cpp
        source/controller/EditorController.h
        source/controller/EditorViewModels.h
        source/core/ChartMapping.h
        source/core/RandomModulator.cpp
        source/core/RandomModulator.h
        source/core/Theme.h
        source/domain/PointModel.h
        source/domain/ProjectModel.h
        source/domain/SnapshotModel.h
        source/plugin/PointDroneAudioProcessor.cpp
        source/plugin/PointDroneAudioProcessor.h
        source/plugin/PointDroneAudioProcessorEditor.cpp
        source/plugin/PointDroneAudioProcessorEditor.h
        source/state/ProjectState.cpp
        source/state/ProjectState.h
        source/ui/AppLookAndFeel.cpp
        source/ui/AppLookAndFeel.h
        source/ui/ChartComponent.cpp
        source/ui/ChartComponent.h
        source/ui/InspectorPanel.cpp
        source/ui/InspectorPanel.h
        source/ui/MasterOutputStrip.cpp
        source/ui/MasterOutputStrip.h
        source/ui/ModulatableSlider.h
        source/ui/ModulationPopup.cpp
        source/ui/ModulationPopup.h
        source/ui/PointWavePreview.cpp
        source/ui/PointWavePreview.h
        source/ui/SnapshotSlots.cpp
        source/ui/SnapshotSlots.h
        source/ui/SnapshotTransitionStrip.cpp
        source/ui/SnapshotTransitionStrip.h
        source/ui/WaveMixSliders.cpp
        source/ui/WaveMixSliders.h
)

target_include_directories(PointDrone
    PRIVATE
        source
)

target_link_libraries(PointDrone
    PRIVATE
        PointDroneAssets
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(PointDrone
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

get_target_property(POINTDRONE_VST3_ARTEFACT PointDrone_VST3 JUCE_PLUGIN_ARTEFACT_FILE)

if(APPLE)
    add_custom_command(TARGET PointDrone_VST3 POST_BUILD
        COMMAND /usr/bin/codesign --force --deep --sign - "${POINTDRONE_VST3_ARTEFACT}"
        VERBATIM
    )
endif()

add_custom_target(validate_vst3
    COMMAND $<TARGET_FILE:pluginval>
        --validate "${POINTDRONE_VST3_ARTEFACT}"
        --strictness-level 5
    DEPENDS pluginval PointDrone_VST3
    COMMENT "Validate Point Drone VST3 with pluginval"
    USES_TERMINAL
)
