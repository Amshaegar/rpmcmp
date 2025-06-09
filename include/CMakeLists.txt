# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

add_library(rpmcmp INTERFACE)

target_sources(rpmcmp
  INTERFACE rpmcmp.hpp
)