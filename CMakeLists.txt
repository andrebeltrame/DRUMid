cmake_minimum_required(VERSION 3.22)

project(Drummy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Universal binary (Apple Silicon + Intel). Harmless on other platforms.
if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.9
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# AU needs a full Xcode install; VST3 + Standalone only need Command Line Tools.
set(DRUMMY_FORMATS VST3 Standalone)
if (APPLE AND DRUMMY_BUILD_AU)
    list(APPEND DRUMMY_FORMATS AU)
endif()

juce_add_plugin(Drummy
    COMPANY_NAME                "Andre Beltrame"
    BUNDLE_ID                   "com.andrebeltrame.drummy"
    PLUGIN_MANUFACTURER_CODE    Abtr
    PLUGIN_CODE                 Drmy
    PRODUCT_NAME                "Drummy"
    FORMATS                     ${DRUMMY_FORMATS}

    # Instrument with MIDI out: this is what lets Ableton route Drummy's MIDI
    # into another track (MIDI From: <Drummy track> -> Drummy).
    IS_SYNTH                    TRUE
    IS_MIDI_EFFECT              FALSE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
)

target_sources(Drummy PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Engine/PatternLibrary.cpp
    Source/Engine/Generator.cpp
    Source/Engine/NoteMap.cpp
    Source/Engine/Sequencer.cpp
    Source/Engine/MidiExport.cpp
    Source/UI/StepGrid.cpp
    Source/UI/DrummyLookAndFeel.cpp
)

target_include_directories(Drummy PRIVATE Source)

target_compile_definitions(Drummy PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Drummy PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ---------------------------------------------------------------------------
#  Console checks for the engine and the exported MIDI.
#    cmake -B build -DDRUMMY_BUILD_TESTS=ON
#    cmake --build build --target DrummyTests && ./build/DrummyTests
# ---------------------------------------------------------------------------
option(DRUMMY_BUILD_TESTS "Build the engine console tests" OFF)

if (DRUMMY_BUILD_TESTS)
    juce_add_console_app(DrummyTests PRODUCT_NAME "DrummyTests")

    target_sources(DrummyTests PRIVATE
        Tests/EngineTests.cpp
        Source/Engine/PatternLibrary.cpp
        Source/Engine/Generator.cpp
        Source/Engine/NoteMap.cpp
        Source/Engine/MidiExport.cpp
    )

    target_include_directories(DrummyTests PRIVATE Source)

    target_compile_definitions(DrummyTests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_STANDALONE_APPLICATION=1
    )

    target_link_libraries(DrummyTests PRIVATE
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_recommended_config_flags
    )
endif()
