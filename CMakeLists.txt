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

set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wnull-dereference -Winit-self -Wunused-macros -Wwrite-strings -Wextra-semi")

set(SANITIZERS_FLAGS "-fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined")
set(OPT_FLAGS "-O3 -flto=auto")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZERS_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAGS}")

set(SOURCE  src/main.cpp)
include_directories(src)
add_executable(accft ${SOURCE})
set(LIBRARIES fmt::fmt pthread dl m)
target_link_libraries(accft PUBLIC ${LIBRARIES})

if (SKBUILD)
  # CPM Dependencies ~~~~~~~~~~~~~~~~ 
  # CPM allows us to include some simple
  # dependencies without much fuss. Great default, for everything more
  # complicated, uses conan. Why not always use conan? CPM works without any
  # interaction (conan needs to be called externally). CPM also has no problems
  # with install PyBind11 (which conan has at the time). If the dependency has
  # complicated dependencies on its own or is slow to compile, better use conan.
  # Check out https://github.com/cpm-cmake/CPM.cmake
  include(./cmake/CPM.cmake) # Package manager for simple requirements.

  # PyBind11
  cpmaddpackage("gh:pybind/pybind11@2.10.0") # pybind11, essential
  set(CMAKE_POSITION_INDEPENDENT_CODE ON) # The code needs to be compiled as PIC
                      # to build the shared lib for python.
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  pybind11_add_module(_bindings ./src/pyaccft/_bindings.cpp)
  target_link_libraries(_bindings PUBLIC fmt::fmt ${LIBRARIES})
  # enable compilation warnings
  target_compile_options(
    _bindings PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>")
  target_compile_definitions(_bindings PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
  install(TARGETS _bindings DESTINATION ./src/pyaccft/)
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
