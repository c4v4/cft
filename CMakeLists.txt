# SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project(AC-CFT)
include(FetchContent)

set(CMAKE_CXX_STANDARD 11)

# Vscode integration for included libraries
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt)
FetchContent_MakeAvailable(fmt)

if (MSVC)
    set(WARNING_FLAGS "/W4 /WX")
    set(SANITIZERS_FLAGS "")
    set(OPT_FLAGS "/O2")
else()
    set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wnull-dereference -Winit-self -Wunused-macros -Wwrite-strings -Wextra-semi")
    set(SANITIZERS_FLAGS "-fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined")
    set(OPT_FLAGS "-O3 -flto=auto")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZERS_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAGS}")

set(SOURCE src/main.cpp)
include_directories(src)
add_executable(accft ${SOURCE})
set(LIBRARIES fmt::fmt pthread dl m)
target_link_libraries(accft PUBLIC ${LIBRARIES})

if (SKBUILD)
    set(PYTHON_BINDINGS ON)
endif()

if (PYTHON_BINDINGS)
    message(STATUS "PYTHON_BINDINGS: ${PYTHON_BINDINGS}")

    # PyBind11
    FetchContent_Declare(pybind11 GIT_REPOSITORY https://github.com/pybind/pybind11.git GIT_TAG v2.13.6)
    FetchContent_MakeAvailable(pybind11) # pybind11, essential
    set(CMAKE_POSITION_INDEPENDENT_CODE ON) # The code needs to be compiled as PIC
                                            # to build the shared lib for python.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # Ensure fmt is compiled with -fPIC
    set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON)

    pybind11_add_module(_bindings ./src/pycft/_bindings.cpp)
    target_link_libraries(_bindings PUBLIC fmt::fmt ${LIBRARIES})
    # enable compilation warnings
    target_compile_options(
        _bindings PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>")
    target_compile_definitions(_bindings PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
    install(TARGETS _bindings DESTINATION ./src/pycft/)
endif()

########################################
############## Unit tests ##############
########################################
option(UNIT_TESTS "Build unit tests." OFF)
message(STATUS "UNIT_TESTS: ${UNIT_TESTS}")
if (UNIT_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()