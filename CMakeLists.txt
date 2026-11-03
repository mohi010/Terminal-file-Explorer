cmake_minimum_required(VERSION 3.16)
project(OSFileExplorer VERSION 1.0.0 LANGUAGES CXX)

# ==============================================================================
# C++17 Standard Enforcement
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Compiler Warnings and Quality Options
# ==============================================================================
option(FX_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
    if(FX_SANITIZE)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# ==============================================================================
# Layer 1: Core Static Library (fxcore)
# ==============================================================================
set(CORE_SOURCES
    src/core/types.hpp
    src/core/format.hpp
    src/core/format.cpp
    src/core/safety.hpp
    src/core/safety.cpp
    src/core/fs_ops.hpp
    src/core/fs_ops.cpp
    src/core/navigator.hpp
    src/core/navigator.cpp
    src/core/search.hpp
    src/core/search.cpp
    src/core/preview.hpp
    src/core/preview.cpp
    src/core/os_layer_demo.cpp
)

add_library(fxcore STATIC ${CORE_SOURCES})
target_include_directories(fxcore PUBLIC src/core)

# ==============================================================================
# Layer 2: Terminal Front End (fx_cli)
# ==============================================================================
set(CLI_SOURCES
    src/cli/terminal.hpp
    src/cli/terminal.cpp
    src/cli/commands.hpp
    src/cli/commands.cpp
    src/cli/main.cpp
)

add_executable(fx_cli ${CLI_SOURCES})
target_link_libraries(fx_cli PRIVATE fxcore)
target_include_directories(fx_cli PRIVATE src)

# ==============================================================================
# Layer 3: Qt GUI Front End (fx_gui) - Phase 8
# ==============================================================================
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Widgets QUIET)
endif()

if(TARGET Qt6::Widgets OR TARGET Qt5::Widgets)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    set(GUI_SOURCES
        src/gui/models.hpp
        src/gui/models.cpp
        src/gui/mainwindow.hpp
        src/gui/mainwindow.cpp
        src/gui/main.cpp
    )

    add_executable(fx_gui ${GUI_SOURCES})
    target_include_directories(fx_gui PRIVATE src)
    target_link_libraries(fx_gui PRIVATE fxcore)

    if(TARGET Qt6::Widgets)
        target_link_libraries(fx_gui PRIVATE Qt6::Widgets)
    else()
        target_link_libraries(fx_gui PRIVATE Qt5::Widgets)
    endif()
    message(STATUS "Qt Widgets GUI target 'fx_gui' enabled.")
else()
    message(STATUS "Qt Widgets not found on system. Building core and terminal CLI 'fx_cli'.")
endif()

# ==============================================================================
# Automated Testing Suite (CTest)
# ==============================================================================
enable_testing()

function(fx_add_unit_test TEST_NAME SOURCE_FILE)
    add_executable(${TEST_NAME} ${SOURCE_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE fxcore)
    target_include_directories(${TEST_NAME} PRIVATE src tests)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

fx_add_unit_test(test_format tests/test_format.cpp)
fx_add_unit_test(test_core_list tests/test_core_list.cpp)
fx_add_unit_test(test_navigator tests/test_navigator.cpp)
fx_add_unit_test(test_core_ops tests/test_core_ops.cpp)
fx_add_unit_test(test_search tests/test_search.cpp)
fx_add_unit_test(test_api_contract tests/test_api_contract.cpp)
fx_add_unit_test(test_stress tests/test_stress.cpp)
