# #######################################################################
# NOTICE
#
# This software (or technical data) was produced for the U.S. Government
# under contract, and is subject to the Rights in Data-General Clause
# 52.227-14, Alt. IV (DEC 2007).
#
# Copyright 2019 The MITRE Corporation. All Rights Reserved.
# ####################################################################### 

cmake_minimum_required(VERSION 3.1)
project(NewProvider)
set(CMAKE_CXX_STANDARD 11)

OPTION(BUILD_SHARED_LIBS "Builds shared libraries for certain dependencies. Recommended: ON" ON)
OPTION(BUILD_STATIC_LIBS "Builds static libraries for certain dependencies. Recommended: OFF" OFF)

if(NOT WIN32)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(CMAKE_CXX_FLAGS "-g -fPIC")
endif()

if(DEFINED ENV{BIQT_HOME})
	if(BIQT_HOME)
		message(WARNING "Both -DBIQT_HOME and a BIQT_HOME environment variable are defined. Using -DBIQT_HOME (${BIQT_HOME}).")
	else()
	  set(BIQT_HOME "$ENV{BIQT_HOME}")
  endif()  
endif()

if(NOT BIQT_HOME)
	message(FATAL_ERROR "BIQT_HOME must be defined as an environment variable or passed to CMake as a parameter using -DBIQT_HOME.")
else()
	set(TO_CMAKE_PATH ${BIQT_HOME} BIQT_HOME)
endif()

if(CMAKE_INSTALL_PREFIX)
	# Ignore CMAKE_INSTALL_PREFIX, but let the user know about it.
	message(WARNING "Overriding provided CMAKE_INSTALL_PREFIX with ${BIQT_HOME}/providers.")
endif()

set(CMAKE_INSTALL_PREFIX "${BIQT_HOME}/providers")

find_library(JSONCPP NAMES libjsoncpp jsoncpp HINTS ${BIQT_HOME}/lib ${BIQT_HOME}/lib64 ${BIQT_HOME}/bin ${BIQT_HOME}/../../lib ${BIQT_HOME}/../../lib64 ${BIQT_HOME}/../../bin)
find_library(BIQTAPI NAMES libbiqtapi biqtapi HINTS ${BIQT_HOME}/lib ${BIQT_HOME}/lib64 ${BIQT_HOME}/bin ${BIQT_HOME}/../../lib ${BIQT_HOME}/../../lib64 ${BIQT_HOME}/../../bin)

if(NOT JSONCPP)
	message(FATAL_ERROR "Failed to find library: jsoncpp.")
endif()

if(NOT BIQTAPI)
	message(FATAL_ERROR "Failed to find library: biqtapi.")
endif()

# TODO: Include any additional packages.
# find_package(...)

# TODO: Append any additional include files.
# set(EXTRA_INCLUDES "3rdparty")
include_directories(. "include" ${BIQT_HOME}/include ${BIQT_HOME}/../../include ${EXTRA_INCLUDES})

# Create the provider library 
file(GLOB SOURCE_FILES "*.cpp")

# TODO: Add any other source files.
# set(EXTRA_FILES MyFile.cpp)
add_library(NewProvider SHARED ${SOURCE_FILES} ${EXTRA_FILES})

target_link_libraries(NewProvider ${JSONCPP} ${BIQTAPI})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/descriptor.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Set installation rules
install(TARGETS NewProvider DESTINATION "${CMAKE_PROJECT_NAME}")
install(FILES descriptor.json DESTINATION "${CMAKE_PROJECT_NAME}")
