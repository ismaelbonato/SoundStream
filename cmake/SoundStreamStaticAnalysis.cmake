include_guard(GLOBAL)

option(SOUNDSTREAM_ENABLE_CLANG_TIDY "Enable clang-tidy for supported targets" ON)
option(SOUNDSTREAM_ENABLE_CPPCHECK "Enable cppcheck for supported targets" ON)
option(SOUNDSTREAM_ENABLE_CLAZY "Enable clazy for Qt-heavy targets" ON)

find_program(SOUNDSTREAM_CLANG_TIDY_EXE NAMES clang-tidy)
find_program(SOUNDSTREAM_CPPCHECK_EXE NAMES cppcheck)
find_program(SOUNDSTREAM_CLAZY_EXE NAMES clazy)

if(SOUNDSTREAM_ENABLE_CLANG_TIDY AND SOUNDSTREAM_CLANG_TIDY_EXE)
    message(STATUS "clang-tidy enabled: ${SOUNDSTREAM_CLANG_TIDY_EXE}")
endif()

if(SOUNDSTREAM_ENABLE_CLANG_TIDY AND NOT SOUNDSTREAM_CLANG_TIDY_EXE)
    message(STATUS "clang-tidy requested, but no clang-tidy executable was found")
endif()

if(SOUNDSTREAM_ENABLE_CPPCHECK AND SOUNDSTREAM_CPPCHECK_EXE)
    message(STATUS "cppcheck enabled: ${SOUNDSTREAM_CPPCHECK_EXE}")
endif()

if(SOUNDSTREAM_ENABLE_CPPCHECK AND NOT SOUNDSTREAM_CPPCHECK_EXE)
    message(STATUS "cppcheck requested, but no cppcheck executable was found")
endif()

if(SOUNDSTREAM_ENABLE_CLAZY AND NOT SOUNDSTREAM_CLAZY_EXE)
    message(STATUS "clazy requested, but no clazy executable was found")
endif()

function(soundstream_enable_cppcheck target)
    if(SOUNDSTREAM_ENABLE_CPPCHECK AND SOUNDSTREAM_CPPCHECK_EXE)
        set(_soundstream_cppcheck_command
            ${SOUNDSTREAM_CPPCHECK_EXE}
            --quiet
            --template=gcc
            --language=c++
            --enable=warning,style,performance,portability
            --inline-suppr
            --suppress=missingIncludeSystem
            --suppress=*:${CMAKE_BINARY_DIR}/*
            --suppress=*:*_autogen/*
            --suppress=*:.moc
        )

        set_target_properties(${target} PROPERTIES
            CXX_CPPCHECK "${_soundstream_cppcheck_command}"
        )
    endif()
endfunction()

function(soundstream_enable_static_analysis target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "soundstream_enable_static_analysis called with unknown target: ${target}")
    endif()

    if(SOUNDSTREAM_ENABLE_CLANG_TIDY AND SOUNDSTREAM_CLANG_TIDY_EXE)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${SOUNDSTREAM_CLANG_TIDY_EXE}"
        )
    endif()

    soundstream_enable_cppcheck(${target})
endfunction()

function(soundstream_enable_qt_static_analysis target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "soundstream_enable_qt_static_analysis called with unknown target: ${target}")
    endif()

    if(SOUNDSTREAM_ENABLE_CLAZY AND SOUNDSTREAM_CLAZY_EXE)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${SOUNDSTREAM_CLAZY_EXE};--standalone"
        )
    elseif(SOUNDSTREAM_ENABLE_CLANG_TIDY AND SOUNDSTREAM_CLANG_TIDY_EXE)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${SOUNDSTREAM_CLANG_TIDY_EXE}"
        )
    endif()

    soundstream_enable_cppcheck(${target})
endfunction()
