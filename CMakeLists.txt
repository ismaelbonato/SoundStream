cmake_minimum_required(VERSION 3.20)
project(SoundStream VERSION 0.1.0 LANGUAGES CXX)

option(SOUNDSTREAM_ENABLE_COVERAGE "Enable coverage instrumentation for tests" OFF)
option(SOUNDSTREAM_ENABLE_COMPILER_WARNINGS "Enable project-wide compiler warnings" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Export compile commands for tools that need them
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SoundStreamStaticAnalysis.cmake)

if(SOUNDSTREAM_ENABLE_COMPILER_WARNINGS)
        if(MSVC)
                add_compile_options(/W4 /permissive-)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
                add_compile_options(
                        -Wall
                        -Wextra
                        -Wpedantic
                        -Wconversion
                        -Wsign-conversion
                        -Wshadow
                )
        endif()
endif()

if(SOUNDSTREAM_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-O0 -g --coverage)
        add_link_options(--coverage)
endif()

# ── KDE ECM ──────────────────────────────────────────────────────────────────
find_package(ECM 6.0 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(CTest)

# ── Sources ───────────────────────────────────────────────────────────────────
add_subdirectory(src)

if(BUILD_TESTING)
        add_subdirectory(tests)

        add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Running test suite with coverage instrumentation"
        )
endif()

# ── Install assets ────────────────────────────────────────────────────────────
install(FILES assets/org.kde.soundstream.desktop
        DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES assets/org.kde.soundstream.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
