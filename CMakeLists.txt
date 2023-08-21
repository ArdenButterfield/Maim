# 3.24.1 is bundled in Visual Studio 2022 v17.4
# 3.24.1 is also bundled in CLion as of 2023
cmake_minimum_required(VERSION 3.24.1)

# This is the name of your plugin
# This cannot have spaces (but PRODUCT_NAME can)
set(PROJECT_NAME "Maim")

# Set the plugin formats you'll be building here.
# Valid formats: AAX Unity VST AU AUv3 Standalone
set(FORMATS AU VST3 AUv3)

# This must be set before the project() call
# see: https://cmake.org/cmake/help/latest/variable/CMAKE_OSX_DEPLOYMENT_TARGET.html
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Support macOS down to High Sierra")

# Reads in our VERSION file and sticks in it CURRENT_VERSION variable
# Be sure the file has no newlines!
file(STRINGS VERSION CURRENT_VERSION)

# For simplicity, the name of the project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# By default we don't want Xcode schemes to be made for modules, etc
set(CMAKE_XCODE_GENERATE_SCHEME OFF)

# Building universal binaries on macOS increases build time
# This is set on CI but not during local dev
if ((DEFINED ENV{CI}) AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
  message("Building for Apple Silicon and x86_64")
  set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
endif ()

set(LAMELIBRARYPATH "${CMAKE_CURRENT_SOURCE_DIR}/${LAME_LIB}")

# Adds all the module sources so they appear correctly in the IDE
# Must be set before JUCE is added as a sub-dir (or any targets are made)
# https://github.com/juce-framework/JUCE/commit/6b1b4cf7f6b1008db44411f2c8887d71a3348889
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# Create a /Modules directory in the IDE with the JUCE Module code
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

# JUCE is setup as a submodule in the /JUCE folder
# Locally, you'll need to run `git submodule update --init --recursive` once
# and `git submodule update --remote --merge` to keep it up to date
# On Github Actions, it's managed by actions/checkout
add_subdirectory(JUCE)

# Check the readme at `docs/CMake API.md` in the JUCE repo for full config
juce_add_plugin("${PROJECT_NAME}"
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME Wildergarden
    BUNDLE_ID com.wildergarden.wildergarden
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE # On MacOS, plugin will be copied to /Users/you/Library/Audio/Plug-Ins/
    PLUGIN_MANUFACTURER_CODE Awbb               # This has to be one uppercase, rest lower for AU formats
    PLUGIN_CODE MA01                            # A unique four-character plugin id with at least one upper-case character
    FORMATS "${FORMATS}"
    PRODUCT_NAME "${PROJECT_NAME}")        # The name of the final executable, which can differ from the target name

# C++20, please
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_20)

# Manually list all .h and .cpp files for the plugin
set(SourceFiles
    Source/lib/blade/bladeenc/formatbitstream2.h
    Source/lib/blade/bladeenc/l3side.h
    Source/lib/blade/bladeenc/mdct.c
    Source/lib/blade/bladeenc/l3psy.c
    Source/lib/blade/bladeenc/l3bitstream.h
    Source/lib/blade/bladeenc/system.h
    Source/lib/blade/bladeenc/l3psy.h
    Source/lib/blade/bladeenc/codec.h
    Source/lib/blade/bladeenc/loop.h
    Source/lib/blade/bladeenc/blade.h
    Source/lib/blade/bladeenc/formatbitstream2.c
    Source/lib/blade/bladeenc/reservoir.c
    Source/lib/blade/bladeenc/run.sh
    Source/lib/blade/bladeenc/l3bitstream-pvt.h
    Source/lib/blade/bladeenc/encode.c
    Source/lib/blade/bladeenc/common.c
    Source/lib/blade/bladeenc/loop.c
    Source/lib/blade/bladeenc/common.h
    Source/lib/blade/bladeenc/tables.h
    Source/lib/blade/bladeenc/blade.c
    Source/lib/blade/bladeenc/reservoir.h
    Source/lib/blade/bladeenc/mdct.h
    Source/lib/blade/bladeenc/tables.c
    Source/lib/blade/bladeenc/huffman.h
    Source/lib/blade/bladeenc/loop-pvt.h
    Source/lib/blade/bladeenc/subs.c
    Source/lib/blade/bladeenc/codec.c
    Source/lib/blade/bladeenc/l3bitstream.c
    Source/lib/blade/bladeenc/encoder.h
        Source/GUIelements/ReassignmentSection.cpp
    Source/GUIelements/guiUtil.h
    Source/GUIelements/MainArea.cpp
    Source/GUIelements/PostSection.h
    Source/GUIelements/StageWindow.cpp
    Source/GUIelements/PsychoacousticSection.h
    Source/GUIelements/StageWindow.h
    Source/GUIelements/LineGraph.h
    Source/GUIelements/PsychoanalGraph.cpp
    Source/GUIelements/PostSection.cpp
        Source/GUIelements/MiscellaneaSection.cpp
        Source/GUIelements/MDCTGraphSection.cpp
    Source/GUIelements/DragBox.cpp
        Source/GUIelements/MiscellaneaSection.h
    Source/GUIelements/MainArea.h
    Source/GUIelements/IndicatorLight.cpp
        Source/GUIelements/ReassignmentSection.h
    Source/GUIelements/IndicatorLight.h
    Source/GUIelements/PsychoanalGraph.h
        Source/GUIelements/MDCTGraphSection.h
    Source/GUIelements/DragBox.h
    Source/GUIelements/PsychoacousticSection.cpp
    Source/MP3ControllerManager.cpp
    Source/BladeController.h
    Source/MP3ControllerManager.h
    Source/PluginProcessor.cpp
    Source/MP3Controller.cpp
    Source/QueueBuffer.h
    Source/RootMeanSquare.cpp
    Source/PluginEditor.h
    Source/RootMeanSquare.h
    Source/AutoGain.cpp
    Source/PluginEditor.cpp
    Source/LameController.h
    Source/AutoGain.h
    Source/LameController.cpp
    Source/PluginProcessor.h
    Source/BladeController.cpp
    Source/MP3Controller.h
    Source/GUIelements/TiltGraph.cpp
    Source/GUIelements/TiltGraph.h
    Source/GUIelements/TitlePanel.h
    Source/GUIelements/TitlePanel.cpp
    Source/GUIelements/MaimLookAndFeel.h
    Source/GUIelements/MaimLookAndFeel.cpp
    Source/GUIelements/NamedRotarySlider.h
    Source/GUIelements/NamedRotarySlider.cpp
    Source/GUIelements/MDCTGraph.cpp
    Source/GUIelements/MDCTGraph.h
    Source/GUIelements/BlockyLineGraph.cpp
    Source/GUIelements/BlockyLineGraph.h
    Source/GUIelements/DemureSlider.cpp
    Source/GUIelements/DemureSlider.h
    Source/GUIelements/ArrayAssignerButtons/ArrayAssignerButton.cpp
    Source/GUIelements/ArrayAssignerButtons/ArrayAssignerButton.h
)
target_include_directories("${PROJECT_NAME}"
        PRIVATE
        Source/lib/lame/include)

target_sources("${PROJECT_NAME}" PRIVATE ${SourceFiles})

# No, we don't want our source buried in extra nested folders
set_target_properties("${PROJECT_NAME}" PROPERTIES FOLDER "")

# The Xcode source tree should uhhh, still look like the source tree, yo
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "" FILES ${SourceFiles})

# Setup our binary data as a target
juce_add_binary_data(fonts SOURCES
        fonts/Gontserrat-Regular.ttf
        fonts/bedstead/bedstead-boldsemicondensed.otf
        fonts/bedstead/bedstead-semicondensed.otf
        fonts/bedstead/bedstead-boldextended.otf)

# Required for Linux happiness:
# See https://forum.juce.com/t/loading-pytorch-model-using-binarydata/39997/2
set_target_properties(fonts PROPERTIES POSITION_INDEPENDENT_CODE TRUE)


# Clean up folder organization on Xcode.
# It tucks the Plugin varieties into a "Targets" folder and generate an Xcode Scheme manually
# Xcode scheme generation is turned off globally to limit noise from other targets
# The non-hacky way of doing this is via the global PREDEFINED_TARGETS_FOLDER property
# However that doesn't seem to be working in Xcode
# Not all plugin types (au, vst) available on each build type (win, macos, linux)
foreach(target ${FORMATS} "All")
    if(TARGET ${PROJECT_NAME}_${target})
        set_target_properties(${PROJECT_NAME}_${target} PROPERTIES
            # Tuck the actual plugin targets into a folder where they won't bother us
            FOLDER "Targets"

            # MacOS only: Sets the default executable that Xcode will open on build
            # For this exact path to to work, manually build the AudioPluginHost.xcodeproj in the JUCE subdir
            XCODE_SCHEME_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app"

            # Let us build the target in Xcode
            XCODE_GENERATE_SCHEME ON)
    endif()
endforeach()
set_target_properties(fonts PROPERTIES FOLDER "Targets")


target_compile_definitions("${PROJECT_NAME}"
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries("${PROJECT_NAME}"
        PUBLIC
        fonts)

# link_directories(./Source/lib/lame/libmp3lame/.libs/)
target_link_libraries("${PROJECT_NAME}"
    PRIVATE
    "${LAMELIBRARYPATH}"
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
)



# When present, use Intel IPP for performance on Windows
if(WIN32) # Can't use MSVC here, as it won't catch Clang on Windows
    find_package(IPP)
    # target_link_libraries("${PROJECT_NAME}" PUBLIC c:/programdata/chocolatey/lib/mingw/tools/install/mingw64/x86_64-w64-mingw32/lib/libmingwex.a)
    if(IPP_FOUND)
        target_link_libraries("${PROJECT_NAME}" PUBLIC IPP::ipps IPP::ippcore IPP::ippi IPP::ippcv)
        message("IPP LIBRARIES FOUND")
        target_compile_definitions("${PROJECT_NAME}" PUBLIC PAMPLEJUCE_IPP=1)
    else()
        message("IPP LIBRARIES *NOT* FOUND")
    endif()
endif()

# Color our warnings and errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_compile_options (-fcolor-diagnostics)
endif ()
