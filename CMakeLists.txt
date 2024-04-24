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


########################################
############## Unit tests ##############
########################################
option(UNIT_TESTS "Build unit tests." OFF)
message(STATUS "UNIT_TESTS: ${UNIT_TESTS}")
if (UNIT_TESTS) 
    enable_testing()
    add_subdirectory(test)
endif() 
