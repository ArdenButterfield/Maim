# 3.24.1 is bundled in Visual Studio 2022 v17.4
# 3.24.1 is also bundled in CLion as of 2023
cmake_minimum_required(VERSION 3.24.1)

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (PamplejuceVersion)

# Configures universal binaries and decides which version of macOS to support
include(PamplejuceMacOS)

# Change me!
# This is the internal name of the project and the name of JUCE's shared code target
# Note: This cannot have spaces (it may be 2024, but you can't have it all!)
# Worry not, JUCE's PRODUCT_NAME can have spaces (and is what DAWs display)
set(PROJECT_NAME "Maim")

# Worry not, JUCE's PRODUCT_NAME can have spaces (and is what DAWs will display)
# You can also just have it be the same thing as PROJECT_NAME
# You may want to append the major version on the end of this (and PROJECT_NAME) ala:
#   set(PROJECT_NAME "MyPlugin_v${MAJOR_VERSION}")
# Doing so enables major versions to show up in IDEs and DAWs as separate plugins
# allowing you to change parameters and behavior without breaking existing user projects
set(PRODUCT_NAME "Maim")

# Change me! Used for the MacOS bundle name and Installers
set(COMPANY_NAME "Wildergarden")

# Change me! Used for the MacOS bundle identifier (and signing)
set(BUNDLE_ID "com.wildergarden.wildergarden")

# Change me! Set the plugin formats you want built
# Valid choices: AAX Unity VST VST3 AU AUv3 Standalone
set(FORMATS Standalone AU VST3 AUv3)

# For simplicity, the name of the CMake project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

set(LAMELIBRARYPATH "${CMAKE_CURRENT_SOURCE_DIR}/${LAME_LIB}")

add_subdirectory(opus)

# Couple tweaks that IMO should be JUCE defaults
include(JUCEDefaults)

# JUCE is setup as a submodule in the /JUCE folder
# Locally, you must run `git submodule update --init --recursive` once
# and later `git submodule update --remote --merge` to keep it up to date
# On Github Actions, this is done as a part of actions/checkout
add_subdirectory(JUCE)

# Add any other modules you want modules here, before the juce_add_plugin call
# juce_add_module(modules/my_module)

# See `docs/CMake API.md` in the JUCE repo for all config options
juce_add_plugin("${PROJECT_NAME}"
        # Icons for the standalone app
        ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"

        # Change me!
        COMPANY_NAME "${COMPANY_NAME}"
        BUNDLE_ID "${BUNDLE_ID}"

        IS_SYNTH FALSE                       # Is this a synth or an effect?
        NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT FALSE              # Does the plugin need midi output?
        IS_MIDI_EFFECT FALSE                 # Is this plugin a MIDI effect?

        # On MacOS, plugin is copied to ~/Users/yourname/Library/Audio/Plug-Ins/
        COPY_PLUGIN_AFTER_BUILD TRUE

        # Change me!
        # A four-character plugin id
        # First character MUST be uppercase for AU formats
        PLUGIN_MANUFACTURER_CODE Awbb

        # Change me!
        # A unique four-character plugin id
        # Note: this must have at least one upper-case character
        PLUGIN_CODE MA01
        FORMATS "${FORMATS}"

        # The name of your final executable
        # This is how it's listed in the DAW
        # This can be different from PROJECT_NAME and can have spaces!
        # You might want to use v${MAJOR_VERSION} here once you go to v2...
        PRODUCT_NAME "${PRODUCT_NAME}")


# This lets us use our code in both the JUCE targets and our Test target
# Without running into ODR violations
add_library(SharedCode INTERFACE)

# C++20, please
# Use cxx_std_23 for C++23 (as of CMake v 3.20)
target_compile_features(SharedCode INTERFACE cxx_std_20)

file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
file(GLOB_RECURSE BladeFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lib/blade/bladeenc/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/lib/blade/bladeenc/*.h")

add_library(blade_library STATIC ${BladeFiles})

set_target_properties(blade_library PROPERTIES LINKER_LANGUAGE CXX)

target_sources(SharedCode INTERFACE ${SourceFiles})

target_include_directories(SharedCode
        INTERFACE
        lib/lame/include
        ${CMAKE_SOURCE_DIR}/opus/include
        lib/blade/bladeenc
)


# Adds a BinaryData target for embedding assets into the binary
include(Assets)

# MacOS only: Cleans up folder and target organization on Xcode.
include(XcodePrettify)

# This is where you can set preprocessor definitions for JUCE and your plugin
target_compile_definitions(SharedCode
        INTERFACE

        # JUCE_WEB_BROWSER and JUCE_USE_CURL off by default
        JUCE_WEB_BROWSER=0  # If you set this to 1, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you set this to 1, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0

        # Uncomment if you are paying for a an Indie/Pro license or releasing under GPLv3
        JUCE_DISPLAY_SPLASH_SCREEN=0

        # lets the app known if we're Debug or Release
        CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        VERSION="${CURRENT_VERSION}"

        # JucePlugin_Name is for some reason doesn't use the nicer PRODUCT_NAME
        PRODUCT_NAME_WITHOUT_VERSION="Maim"
)

#add_library(my_warning_flags INTERFACE)
#target_compile_options(my_warning_flags INTERFACE
#    -Wno-float-conversion
#    -Wno-implicit-int-float-conversion
#    -Wno-implicit-float-conversion
#        )


# Link to any other modules you added (with juce_add_module) here!
# Usually JUCE modules must have PRIVATE visibility
# See https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md#juce_add_module
# However, with Pamplejuce, you'll link modules to SharedCode with INTERFACE visibility
# This allows the JUCE plugin targets and the Tests target to link against it
target_link_libraries(SharedCode
        INTERFACE
        Assets
        "${LAMELIBRARYPATH}"
        opus
        blade_library
        juce_audio_utils
        juce_audio_processors
        juce_dsp
        juce_gui_basics
        juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        #    my_warning_flags
)

set_property(TARGET opus PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET blade_library PROPERTY POSITION_INDEPENDENT_CODE ON)

if (WIN32)
    add_custom_target(
            lame_lib
            COMMAND nmake libmp3lame-static.lib -f Makefile.MSVC comp=msvc asm=no
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/lame"
    )

    add_dependencies(SharedCode lame_lib)
else()
    add_custom_target(
            lame_lib
            COMMAND make
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/lame"
    )

    add_dependencies(SharedCode lame_lib)
endif(WIN32)



# Link the JUCE plugin targets our SharedCode target
target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)


# IPP support, comment out to disable
include(PamplejuceIPP)

# Everything related to the tests target
include(Tests)

# A separate target keeps the Tests target fast!
include(Benchmarks)

# Pass some config to GA (like our PRODUCT_NAME)
include(GitHubENV)