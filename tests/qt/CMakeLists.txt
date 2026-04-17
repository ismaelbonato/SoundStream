find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

add_executable(soundstream-qt-dispatcher-test
    QtMainThreadDispatcherTest.cpp
)

add_executable(soundstream-qt-scene-interaction-test
    PatchbaySceneInteractionTest.cpp
)

target_link_libraries(soundstream-qt-dispatcher-test
    PRIVATE
        soundstream-adapters-qt
        Qt6::Core
        Qt6::Test
)

target_link_libraries(soundstream-qt-scene-interaction-test
    PRIVATE
        soundstream-canvas
        Qt6::Core
        Qt6::Widgets
        Qt6::Test
)

soundstream_enable_static_analysis(soundstream-qt-dispatcher-test)
soundstream_enable_static_analysis(soundstream-qt-scene-interaction-test)

add_test(NAME qt-dispatcher-test COMMAND soundstream-qt-dispatcher-test)
set_tests_properties(qt-dispatcher-test PROPERTIES LABELS "qt")

add_test(NAME qt-scene-interaction-test COMMAND soundstream-qt-scene-interaction-test)
set_tests_properties(qt-scene-interaction-test PROPERTIES LABELS "qt")
