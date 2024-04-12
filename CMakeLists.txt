cmake_minimum_required(VERSION 3.16)
project(AC-CFT)
include(FetchContent)

set(CMAKE_CXX_STANDARD 11)

# Vscode integration for included libraries
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt)
FetchContent_MakeAvailable(fmt)

set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wuninitialized -Wshadow")
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
if (UNIT_TESTS) 

    FetchContent_Declare(catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2  GIT_TAG v2.x)
    FetchContent_MakeAvailable(Catch2)
    set(LIBRARIES ${LIBRARIES} Catch2::Catch2)

    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS    "-lgcov --coverage")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

    set(SOURCES test/main.cpp
                test/Chrono_unittests.cpp
                test/StringView_unittests.cpp
                test/parse_utils_unittests.cpp
                test/sort_unittests.cpp
                test/Span_unittests.cpp
                test/coverage_unittests.cpp
                test/parsing_unittests.cpp
                test/random_unittests.cpp
                test/SortedArray_unittests.cpp
                test/utility_unittests.cpp)

    add_executable(tests ${SOURCES})
    target_link_libraries(tests PUBLIC ${LIBRARIES})
    add_test(NAME tests COMMAND tests)
    enable_testing()

endif() 