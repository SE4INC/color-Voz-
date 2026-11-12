cmake_minimum_required(VERSION 3.24)
project(CrisVoxColor VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  add_compile_options(/utf-8)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7c9d3783b127263d72bb65fe0a7e2dc8a02a7ac2 GIT_SUBMODULES "")
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(CrisVoxColor COMPANY_NAME "CRIS Audio" BUNDLE_ID "audio.cris.voxcolor"
  PLUGIN_MANUFACTURER_CODE CrAu PLUGIN_CODE CvCl PRODUCT_NAME "CRIS VOX COLOR"
  DESCRIPTION "Selective vocal saturation with a local analysis assistant"
  FORMATS VST3 Standalone IS_SYNTH FALSE IS_MIDI_EFFECT FALSE
  NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Fx Distortion)
target_sources(CrisVoxColor PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp
  Source/ColorEngine.cpp Source/AnalysisWorker.cpp)
target_compile_definitions(CrisVoxColor PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(CrisVoxColor PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags)
option(CRIS_BUILD_TESTS "Build audio and integration tests" ON)
if(CRIS_BUILD_TESTS)
  enable_testing()
  add_executable(cris_color_tests tests/Tests.cpp)
  target_include_directories(cris_color_tests PRIVATE Source "${juce_SOURCE_DIR}/modules")
  target_compile_definitions(cris_color_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    CRIS_TEST_VOCAL="${juce_SOURCE_DIR}/examples/Assets/singing.ogg")
  target_link_libraries(cris_color_tests PRIVATE CrisVoxColor)
  add_test(NAME color_audio_and_state COMMAND cris_color_tests)
endif()
