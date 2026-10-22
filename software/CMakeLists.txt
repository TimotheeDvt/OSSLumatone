cmake_minimum_required(VERSION 3.22)
project(OSSLumatoneEditor VERSION 0.1.0)
set(ENV{CMAKE_BUILD_PARALLEL_LEVEL} 4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(JUCE)

# Desktop configuration app (the Lumatone Editor equivalent). It only talks MIDI to the
# instrument and makes no sound, so it is a plain GUI app rather than a plugin.
juce_add_gui_app(OSSLumatoneEditor
    PRODUCT_NAME              "OSSLumatone Editor"
    COMPANY_NAME              "OSSLumatone"
    BUNDLE_ID                 com.OSSLumatone.Editor
)

juce_generate_juce_header(OSSLumatoneEditor)

file(GLOB_RECURSE SOURCE_FILES "Source/*.cpp" "Source/*.h")
target_sources(OSSLumatoneEditor PRIVATE ${SOURCE_FILES})

target_compile_definitions(OSSLumatoneEditor PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:OSSLumatoneEditor,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:OSSLumatoneEditor,JUCE_VERSION>"
)

target_link_options(OSSLumatoneEditor PRIVATE -fuse-ld=mold)

target_link_libraries(OSSLumatoneEditor
    PRIVATE juce::juce_gui_extra juce::juce_audio_devices
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags
)
