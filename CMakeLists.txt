cmake_minimum_required(VERSION 3.16)
project(biqt)
set(CMAKE_CXX_STANDARD 11)

# Sourced from: https://stackoverflow.com/questions/7249225/can-cmake-recognize-cmakelists-txt-with-another-name-cmakelists-nightly-txt
OPTION(BUILD_TARGET "The target build type for the docker image. Supported options: CENTOS7, UBUNTU." "CENTOS7")

if (BUILD_TARGET STREQUAL  "CENTOS7" )
    include(CMakeLists.centos.cmake)
elseif (BUILD_TARGET STREQUAL  "UBUNTU")
    include(CMakeLists.ubuntu.cmake)
else ()
    message(FATAL_ERROR "Unknown build kind: ${BUILD_TARGET}")
endif () 