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

# OPTIONS #####################################################################

OPTION(BUILD_SHARED_LIBS "Builds shared libraries for certain dependencies. Recommended: ON" ON)
OPTION(BUILD_STATIC_LIBS "Builds static libraries for certain dependencies. Recommended: OFF" OFF)
OPTION(WITH_JAVA         "Builds Java bindings. Requires a JDK installation. Default: ON" ON)
OPTION(SKIP_PROFILE      "Do not set up environment variables on Linux (turn on if you do not have root access)" OFF)

set(BIQT_VERSION "0.1" 
    CACHE STRING "Build version or tag associated with this BIQT release.")

message("BIQT v${BIQT_VERSION}")
add_definitions(-DBIQT_VERSION="${BIQT_VERSION}")

if(NOT WIN32)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
	set(CMAKE_CXX_FLAGS "-g -fPIC")
else()
  include_directories(cxx/windows)
endif()

include_directories(cxx/
                    cxx/jsoncpp/include
                    ${EXTRA_INCLUDES})

set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "biqt")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_VERSION 0.1)
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "MITRE Corporation")
set(CPACK_PACKAGE_VENDOR "MITRE Corporation")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_PACKAGE_PROVIDES "jsoncpp == 1.8.3")
include(CPack)

if(WITH_JAVA)
  # Java bindings have been requested.
  find_package(JNI REQUIRED)
	add_definitions(-DBIQT_JAVA_SUPPORT)
  
  # Add Java-related includes and C/C++ source files.
  include_directories(java/src/c ${JNI_INCLUDE_DIRS})
  set(JAVA_LIBRARY_FILES java/src/c/jnihelper.c
                         java/src/c/java_provider.cpp)

  # Add the Java maven project which builds the BIQT jar file.                         
  add_subdirectory(java/src/c java/src/main/resources)  
else()
  remove_definitions(-DBIQT_JAVA_SUPPORT)
  unset(JAVA_LIBRARY_FILES)
endif()

# BUILD JSONCPP DEPENDENCY ####################################################

add_subdirectory(cxx/jsoncpp)

# BUILD THE BIQT LIBRARY FILE #################################################
set(LIBRARY_FILES cxx/BIQT.cpp)
add_library(biqtapi SHARED ${LIBRARY_FILES} ${JAVA_LIBRARY_FILES})

# BUILD THE BIQT COMMAND LINE EXECUTABLE ######################################

set(SOURCE_FILES cxx/BIQT-cli.cpp)
add_executable(biqt ${SOURCE_FILES})

# BUILD JAVA BINDINGS (IF REQUESTED) ##########################################

if(WITH_JAVA)

	# Create the resources directory if it doesn't exist.
	FILE(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/java/src/main/resources/)
	
	# Locate the maven executable.
	find_program(MAVEN NAMES mvn mvnw PATHS ${CMAKE_SOURCE_DIR}/java/)
	
	if(NOT MAVEN)
		message(FATAL_ERROR "Failed to find the Maven executable named mvn or mvnw. Please add it to the PATH or specify its location in CMAKE_PREFIX_PATH.")
	endif()
	
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/biqt-${BIQT_VERSION}.jar"
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:biqtapi> ${CMAKE_SOURCE_DIR}/java/src/main/resources/ COMMENT "Copying library files to Java resources..."
		COMMAND ${MAVEN} -f ${CMAKE_SOURCE_DIR}/java/pom.xml -DskipTests -Dbiqt.version=${BIQT_VERSION} -Doutput.directory=${CMAKE_BINARY_DIR} package COMMENT "Running maven..."
	)
		
	add_custom_target(biqt_java_jar	ALL	
		DEPENDS "${CMAKE_BINARY_DIR}/biqt-${BIQT_VERSION}.jar"
	)
endif()

target_link_libraries(biqtapi ${CMAKE_DL_LIBS} jsoncpp_lib ${JAVA_JVM_LIBRARY})
target_link_libraries(biqt biqtapi ${CMAKE_DL_LIBS} jsoncpp_lib)

# INSTALLATION ################################################################

file(GLOB BIQT_INCLUDES "cxx/*.h")

if(WIN32)
  file(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX_WINDOWS)
  file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX_NATIVE)  
  string(REPLACE  "\\" "\\\\" CMAKE_INSTALL_PREFIX_NATIVE_ESCAPED ${CMAKE_INSTALL_PREFIX_NATIVE})
  
	# On Windows, most content is installed directly into the install prefix.
	install(TARGETS biqt biqtapi
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION bin
		RUNTIME DESTINATION bin
	)
	
	install(CODE "FILE(MAKE_DIRECTORY \"${CMAKE_INSTALL_PREFIX_WINDOWS}/providers\")")	
	if(WITH_JAVA)
		install(FILES     "${CMAKE_CURRENT_BINARY_DIR}/biqt-${BIQT_VERSION}.jar" DESTINATION ".")	
	endif()
	install(FILES     setup_provider.py                                      DESTINATION "./scripts")
	install(DIRECTORY templates                                              DESTINATION "./scripts")
	install(FILES     ${BIQT_INCLUDES}                                       DESTINATION "./include")
	
	# Try to set BIQT_HOME and PATH. If this fails, rely on the output messages.
	install(CODE "message(\"Remember that you must set or update the BIQT_HOME and PATH system environment variables.\")")
  install(CODE "message(\"You may execute `SET \\\"BIQT_HOME=${CMAKE_INSTALL_PREFIX_NATIVE_ESCAPED}\\\"` to define BIQT_HOME temporarily, but it will not be saved for future use.\")")
	install(CODE "message(\"You may execute `SET PATH=%JAVA_HOME%\\\\bin\\\\server;%BIQT_HOME%\\\\bin;%PATH%` to update PATH temporarily, but it will not be saved for future use.\")")
else()
	# On Linux, files are installed to different locations.
	#   * Providers install to ${CMAKE_INSTALL_PREFIX}/share/biqt/providers.
	#   * BIQT Java JAR installs to ${CMAKE_INSTALL_PREFIX}/share/biqt.
	#   * Scripts install to ${CMAKE_INSTALL_PREFIX}/share/biqt/scripts.
	install(TARGETS biqt biqtapi
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
	)
	file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/biqt.sh "export BIQT_HOME=\"${CMAKE_INSTALL_PREFIX}/share/biqt\";\n")
  file(APPEND  ${CMAKE_CURRENT_BINARY_DIR}/biqt.sh "export PATH=$PATH:\"${CMAKE_INSTALL_PREFIX}/bin\";\n")
  file(APPEND  ${CMAKE_CURRENT_BINARY_DIR}/biqt.sh "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:\"${CMAKE_INSTALL_PREFIX}/lib\":\"${CMAKE_INSTALL_PREFIX}/lib64\";\n")
	install(CODE "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/biqt/providers)")	
	
	install(FILES     setup_provider.py                                      DESTINATION "./share/biqt/scripts")
	install(DIRECTORY templates                                              DESTINATION "./share/biqt/scripts")
	install(FILES     ${BIQT_INCLUDES}                                       DESTINATION "./include")
	install(FILES     "${CMAKE_CURRENT_BINARY_DIR}/biqt.sh"                  DESTINATION "./share/biqt")
	if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT ${SKIP_PROFILE})
		install(FILES     "${CMAKE_CURRENT_BINARY_DIR}/biqt.sh"                  DESTINATION "/etc/profile.d" OPTIONAL)
		MESSAGE("You may need to log out or run `source /etc/profile.d/biqt.sh` before running BIQT.")	
	else()
		MESSAGE("You should add `source ${CMAKE_INSTALL_PREFIX}/share/biqt/biqt.sh` to your ~/.bashrc file.")	
	endif()
	if(WITH_JAVA)
		install(FILES     "${CMAKE_CURRENT_BINARY_DIR}/biqt-${BIQT_VERSION}.jar" DESTINATION "./share/biqt/")
	endif()
endif()
