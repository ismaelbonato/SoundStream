find_package(Catch2 3 REQUIRED)
add_executable(soundstream-core-graph-service-test
    GraphServiceCatch2Test.cpp
)

add_executable(soundstream-pipewire-metadata-test
    PwMetadataCatch2Test.cpp
)

target_link_libraries(soundstream-core-graph-service-test
    PRIVATE
        Catch2::Catch2WithMain
        soundstream-core
)

target_link_libraries(soundstream-pipewire-metadata-test
    PRIVATE
        Catch2::Catch2WithMain
        soundstream-pipewire
)

set_target_properties(soundstream-core-graph-service-test
    soundstream-pipewire-metadata-test
    PROPERTIES
        AUTOMOC OFF
        AUTORCC OFF
        AUTOUIC OFF
)

soundstream_enable_static_analysis(soundstream-core-graph-service-test)
soundstream_enable_static_analysis(soundstream-pipewire-metadata-test)

include(Catch)
catch_discover_tests(soundstream-core-graph-service-test
    TEST_PREFIX "core::"
    PROPERTIES LABELS "core"
)

catch_discover_tests(soundstream-pipewire-metadata-test
    TEST_PREFIX "backend::"
    PROPERTIES LABELS "backend"
)
