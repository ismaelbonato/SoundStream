# App and backend composition live here so backend-specific compile details stay
# with the backend folder instead of the src root.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2)

# KF6KirigamiConfig.cmake on some distros was built with OpenMP and
# unconditionally calls find_dependency(OpenMP). Our code does not use OpenMP,
# so pre-satisfy the search to avoid fatal configuration failures with Clang.
set(OpenMP_CXX_FLAGS "-fopenmp=libgomp" CACHE STRING "" FORCE)
set(OpenMP_CXX_LIB_NAMES "gomp" CACHE STRING "" FORCE)
set(OpenMP_gomp_LIBRARY "/usr/lib/gcc/x86_64-linux-gnu/14/libgomp.so" CACHE FILEPATH "" FORCE)
set(OpenMP_CXX_FOUND TRUE CACHE BOOL "" FORCE)

find_package(KF6Kirigami REQUIRED)
find_package(KF6KirigamiPlatform QUIET)
find_package(KF6 REQUIRED COMPONENTS
    I18n
    CoreAddons
)

function(_soundstream_ensure_placeholder_target _target)
        if(NOT TARGET ${_target})
                add_library(${_target} INTERFACE IMPORTED)
        endif()
endfunction()

foreach(_qml_plugin_target IN ITEMS
        Qt6::qtquick2plugin
        Qt6::qmlplugin
        Qt6::modelsplugin
        Qt6::workerscriptplugin
        Qt6::qtquickcontrols2plugin
        Qt6::qquicklayoutsplugin
        Qt6::qtquicktemplates2plugin
        Qt6::qmlshapesplugin
        Qt6::quickwindow
        Qt6::qtquickcontrols2implplugin
        Qt6::qtquickcontrols2fusionstyleplugin
        Qt6::qtquickcontrols2materialstyleplugin
        Qt6::qtquickcontrols2imaginestyleplugin
        Qt6::qtquickcontrols2universalstyleplugin
        Qt6::qtquickcontrols2basicstyleplugin
        Qt6::qtquickcontrols2fusionstyleimplplugin
        Qt6::qtquickcontrols2materialstyleimplplugin
        Qt6::qtquickcontrols2imaginestyleimplplugin
        Qt6::qtquickcontrols2universalstyleimplplugin
        Qt6::qtquickcontrols2basicstyleimplplugin
        Qt6::labsmodelsplugin
        KF6Kirigamiplugin
        KF6KirigamiPrivateplugin
        KF6KirigamiPolyfillplugin
        KF6::KirigamiPlatformplugin
        KF6KirigamiDialogsplugin
        KF6KirigamiLayoutsplugin
        KF6KirigamiLayoutsPrivateplugin
)
        _soundstream_ensure_placeholder_target(${_qml_plugin_target})
endforeach()

add_subdirectory(pipewire)

set(SOUNDSTREAM_QML_SOURCES
    ../../qml/Main.qml
)

qt_policy(SET QTP0004 NEW)

qt_add_executable(soundstream
    ../main.cpp
    ../SoundStream.cpp
)

qt_add_qml_module(soundstream
    URI org.kde.soundstream
    VERSION 1.0
    QML_FILES ${SOUNDSTREAM_QML_SOURCES}
    SOURCES
        ../SoundStream.h
        ../canvas/PatchbayView.h
        ../canvas/PatchbayView.cpp
    RESOURCE_PREFIX /
)

target_link_libraries(soundstream
    PRIVATE
        soundstream-core
        soundstream-adapters-qt
        soundstream-canvas
        soundstream-pipewire
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        KF6::Kirigami
        KF6::I18n
        KF6::CoreAddons
)


soundstream_enable_static_analysis(soundstream)

qt_import_qml_plugins(soundstream)

install(TARGETS soundstream DESTINATION ${KDE_INSTALL_BINDIR})
