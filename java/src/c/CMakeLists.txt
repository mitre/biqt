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
project(biqt)
set(CMAKE_CXX_STANDARD 11)

set(BIQT_JAVA_SOURCE_FILES jnihelper.c
                           org_mitre_biqt_BIQT.cpp)

add_library(biqt_java SHARED ${BIQT_JAVA_SOURCE_FILES})

target_link_libraries(biqt_java biqtapi ${CMAKE_DL_LIBS})

if(WIN32)
    install(TARGETS biqt_java LIBRARY DESTINATION bin)
else()
    install(TARGETS biqt_java LIBRARY DESTINATION lib)
endif()
