# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

project(rpmcmp_tests CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.16.0
)
FetchContent_MakeAvailable(googletest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_executable(rpmcmp_tests
    rpmcmp_tests.cpp
    main.cpp
)

target_include_directories(
    rpmcmp_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_link_libraries(rpmcmp_tests PRIVATE gtest)
target_link_libraries(rpmcmp_tests PUBLIC rpmcmp)
target_compile_features(rpmcmp_tests PUBLIC cxx_std_17)

if (MSVC)
    target_compile_options(rpmcmp_tests PUBLIC /W4 /WX)
else()
    target_compile_options(rpmcmp_tests PUBLIC -Wall -Wextra -Werror -pedantic)
endif()