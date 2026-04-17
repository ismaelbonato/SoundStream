# ── canvas/ — backend-agnostic graph widget ───────────────────────────────
# Canvas code may use shared core graph data types, but not PipeWire.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick)

add_library(soundstream-canvas OBJECT
    Item.cpp
    GraphLayouter.cpp
    LinkInitializer.cpp
    LinkItem.cpp
    LinkPathUpdater.cpp
    NodeItem.cpp
    NodeRemovalHelper.cpp
    PatchbayModel.cpp
    PatchbayScene.cpp
    PortItem.cpp
)

target_include_directories(soundstream-canvas
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(soundstream-canvas
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Quick
)

# Suppress Qt keyword macros inside canvas/ (QT_NO_KEYWORDS is global from KDE settings)
target_compile_definitions(soundstream-canvas PRIVATE QT_NO_KEYWORDS)

soundstream_enable_qt_static_analysis(soundstream-canvas)
