cmake_minimum_required(VERSION 3.16)
project(AC-CFT)
include(FetchContent)

set(CMAKE_CXX_STANDARD 11)

# Vscode integration for included libraries
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt)
FetchContent_MakeAvailable(fmt)

set(LIBRARIES fmt::fmt pthread dl m)

option(UNIT_TESTS "Build unit tests." OFF)

if (UNIT_TESTS) # Build unit tests only.

    FetchContent_Declare(catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2  GIT_TAG v2.x)
    FetchContent_MakeAvailable(Catch2)
    set(LIBRARIES ${LIBRARIES} Catch2::Catch2)

    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS    "-lgcov --coverage")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )


    # Simplified from fmtlib's CMakeLists.txt
    #function(add_cft_test name)
    #    set(sources test/${name}.cpp)
    #    target_link_libraries(${name} PUBLIC ${LIBRARIES})
    #    add_test(NAME ${name} COMMAND ${name})
    #endfunction()
#
    #add_cft_test(parsing_unittests)
    #add_cft_test(utility_unittests)
    #add_cft_test(StringView_unittests)
    #add_cft_test(coverage_unittests)
    #add_cft_test(SortedArray_unittests)

    set(SOURCES test/tests.cpp
                test/parsing_unittests.cpp
                test/utility_unittests.cpp
                test/StringView_unittests.cpp
                test/coverage_unittests.cpp
                test/SortedArray_unittests.cpp)

    add_executable(tests ${SOURCES})
    target_link_libraries(tests PUBLIC ${LIBRARIES})
    add_test(NAME tests COMMAND tests)
    enable_testing()

endif() # End -- Build the actual solver.

set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Werror")
set(SANITIZERS_FLAGS "-fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined")
#set(OPT_FLAGS "-O3 -flto=auto")
set(OPT_FLAGS "-O3 -flto=auto -march=native -mtune=native")
#set(OPT_FLAGS "-O3 -flto=auto -march=native -mtune=native -ffast-math")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZERS_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAGS}")

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE  src/main.cpp)

option(ENABLE_VERBOSE "Enable verbose output" OFF)

message("-- Build options")

if(ENABLE_VERBOSE)
    message("--- Verbose output ENABLED")
    add_definitions(-DVERBOSE)
else()
    message("--- Verbose output DISABLED")
endif()


include_directories(src)

add_executable(accft ${SOURCE})

target_link_libraries(accft PUBLIC ${LIBRARIES})
