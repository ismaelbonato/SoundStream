## src/core/CMakeLists.txt
# Builds the pure core library for SoundStream.

add_library(soundstream-core STATIC
    GraphService.cpp
    GraphTypes.h
    GraphBackend.h
    IMainThreadDispatcher.h
)

target_include_directories(soundstream-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(soundstream-core PROPERTIES
    AUTOMOC OFF
    AUTORCC OFF
    AUTOUIC OFF
)

soundstream_enable_static_analysis(soundstream-core)
