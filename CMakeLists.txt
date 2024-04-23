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
set(OPT_FLAGS "-O3 -flto=auto -fno-omit-frame-pointer -g")
#set(OPT_FLAGS "-O3 -flto=auto -march=native -mtune=native")
#set(OPT_FLAGS "-O3 -flto=auto -march=native -mtune=native -ffast-math")

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

    FetchContent_Declare(catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2  GIT_TAG v2.x)
    FetchContent_MakeAvailable(Catch2)
    set(LIBRARIES ${LIBRARIES} Catch2::Catch2)

    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS    "-lgcov --coverage")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -DCFT_ASSERT_FAIL_THROWS")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -DCFT_ASSERT_FAIL_THROWS" )

    function(add_cft_test name)
        set(sources test/${name}.cpp)
        add_executable(${name} ${sources})
        target_link_libraries(${name} PUBLIC ${LIBRARIES})
        add_test(NAME ${name} COMMAND ${name})
    endfunction()

    add_cft_test(cft_unittests)
    add_cft_test(Chrono_unittests)
    add_cft_test(CliArgs_unittests)
    add_cft_test(coverage_unittests)
    add_cft_test(custom_types_unittests)
    add_cft_test(Instance_unittests)
    add_cft_test(large_types_unittests)
    add_cft_test(parse_utils_unittests)
    add_cft_test(parsing_unittests)
    add_cft_test(random_unittests)
    add_cft_test(redundancy_unittests)
    add_cft_test(Refinement_unittests)
    add_cft_test(SortedArray_unittests)
    add_cft_test(small_types_unittests)
    add_cft_test(sort_unittests)
    add_cft_test(Span_unittests)
    add_cft_test(StringView_unittests)
    add_cft_test(utility_unittests)

    enable_testing()

endif() 