## src/adapters/qt/CMakeLists.txt
# Qt adapter code: dispatcher and Qt bridges.
find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(soundstream-adapters-qt STATIC
    QtMainThreadDispatcher.cpp
    QtGraphModelBridge.cpp
)

target_include_directories(soundstream-adapters-qt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(soundstream-adapters-qt
    PUBLIC
        Qt6::Core
)

soundstream_enable_qt_static_analysis(soundstream-adapters-qt)
