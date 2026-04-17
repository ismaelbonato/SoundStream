find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED IMPORTED_TARGET libpipewire-0.3)

add_library(soundstream-pipewire OBJECT
    PwClient.cpp
    PwMetadata.cpp
)

target_include_directories(soundstream-pipewire
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(soundstream-pipewire
    PUBLIC
        soundstream-core
        PkgConfig::PIPEWIRE
)

set_target_properties(soundstream-pipewire PROPERTIES
    AUTOMOC OFF
    AUTORCC OFF
    AUTOUIC OFF
)

soundstream_enable_static_analysis(soundstream-pipewire)
