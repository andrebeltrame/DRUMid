cmake_minimum_required(VERSION 3.22)

project(DRUMid VERSION 1.0.0 LANGUAGES C CXX)

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
set(DRUMID_FORMATS VST3 Standalone)
if (APPLE AND DRUMID_BUILD_AU)
    list(APPEND DRUMID_FORMATS AU)
endif()

# Same house as the other plugins, so DRUMid lands in the same folder and shows
# up under the same manufacturer in the DAW.
option(DRUMID_COPY_AFTER_BUILD "Copy the VST3 into the system plugin folder" ON)

if (APPLE)
    set(DRUMID_VST3_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/Nowhr Dynamics")
elseif (WIN32)
    set(DRUMID_VST3_DIR "$ENV{CommonProgramFiles}/VST3/Nowhr Dynamics")
else()
    set(DRUMID_VST3_DIR "$ENV{HOME}/.vst3/Nowhr Dynamics")
endif()

juce_add_plugin(DRUMid
    COMPANY_NAME                "Nowhr Dynamics"
    BUNDLE_ID                   "com.nowhrdynamics.drumid"
    PLUGIN_MANUFACTURER_CODE    Nwhr
    PLUGIN_CODE                 Drid
    PRODUCT_NAME                "DRUMid"
    FORMATS                     ${DRUMID_FORMATS}
    VST3_CATEGORIES             "Instrument" "Tools"
    VST3_COPY_DIR               "${DRUMID_VST3_DIR}"

    # Instrument with MIDI out: this is what lets Ableton route DRUMid's MIDI
    # into another track (MIDI From: <DRUMid track> -> DRUMid).
    IS_SYNTH                    TRUE
    IS_MIDI_EFFECT              FALSE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     ${DRUMID_COPY_AFTER_BUILD}
)

target_sources(DRUMid PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Engine/PatternLibrary.cpp
    Source/Engine/Generator.cpp
    Source/Engine/NoteMap.cpp
    Source/Engine/Sequencer.cpp
    Source/Engine/MidiExport.cpp
    Source/UI/StepGrid.cpp
    Source/UI/Icons.cpp
    Source/UI/AboutPanel.cpp
    Source/UI/DrumidLookAndFeel.cpp
)

target_include_directories(DRUMid PRIVATE Source)

target_compile_definitions(DRUMid PUBLIC
    DRUMID_VERSION="${PROJECT_VERSION}"
    DRUMID_COMPANY="Nowhr Dynamics"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(DRUMid PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ---------------------------------------------------------------------------
#  Console checks for the engine and the exported MIDI.
#    cmake -B build -DDRUMID_BUILD_TESTS=ON
#    cmake --build build --target DrumidTests && ./build/DrumidTests
# ---------------------------------------------------------------------------
option(DRUMID_BUILD_TESTS "Build the engine console tests" OFF)

if (DRUMID_BUILD_TESTS)
    juce_add_console_app(DrumidTests PRODUCT_NAME "DrumidTests")

    target_sources(DrumidTests PRIVATE
        Tests/EngineTests.cpp
        Source/Engine/PatternLibrary.cpp
        Source/Engine/Generator.cpp
        Source/Engine/NoteMap.cpp
        Source/Engine/MidiExport.cpp
    )

    target_include_directories(DrumidTests PRIVATE Source)

    target_compile_definitions(DrumidTests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_STANDALONE_APPLICATION=1
    )

    target_link_libraries(DrumidTests PRIVATE
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_recommended_config_flags
    )
endif()

# ---------------------------------------------------------------------------
#  Renders the editor to PNG without opening a window - useful for reviewing
#  the UI over a terminal, and for README screenshots.
#    cmake -B build -DDRUMID_BUILD_PREVIEW=ON
#    cmake --build build --target DrumidPreview
# ---------------------------------------------------------------------------
option(DRUMID_BUILD_PREVIEW "Build the offscreen UI preview renderer" OFF)

if (DRUMID_BUILD_PREVIEW)
    juce_add_console_app(DrumidPreview PRODUCT_NAME "DrumidPreview")

    target_sources(DrumidPreview PRIVATE Tools/PreviewRender.cpp)
    target_include_directories(DrumidPreview PRIVATE Source)

    target_compile_definitions(DrumidPreview PRIVATE
        DRUMID_VERSION="${PROJECT_VERSION}"
        DRUMID_COMPANY="Nowhr Dynamics"
        JUCE_STANDALONE_APPLICATION=1
        JUCE_MODAL_LOOPS_PERMITTED=1
    )

    # The plugin target links its JUCE modules PRIVATE, so the preview has to
    # ask for the headers itself.
    target_link_libraries(DrumidPreview PRIVATE
        DRUMid
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
    )
endif()
