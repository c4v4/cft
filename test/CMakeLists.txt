# SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
# SPDX-License-Identifier: MIT

FetchContent_Declare(doctest GIT_REPOSITORY https://github.com/doctest/doctest GIT_TAG master)
FetchContent_MakeAvailable(doctest)

set(LIBRARIES ${LIBRARIES} doctest)

set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
set(GCC_COVERAGE_LINK_FLAGS    "-lgcov --coverage")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -DCFT_ASSERT_FAIL_THROWS")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -DCFT_ASSERT_FAIL_THROWS" )

function(add_cft_test name)
    set(sources ${name}.cpp)
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
