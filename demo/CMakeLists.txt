#############################################################################
# Name:        CMakeListst.txt
# Purpose:     Demonstration project for wxStartPage
# Author:      Blake Madden
# Created:     2023-03-25
# Copyright:   (c) 2023 Blake Madden
# Licence:     BSD-3-Clause
#############################################################################

# Declare the minimum required CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

ADD_COMPILE_OPTIONS("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
ADD_COMPILE_OPTIONS("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    # If no deployment target has been set default to the minimum supported
    # OS version (this has to be set before the first project() call)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        SET(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "iOS Deployment Target")
    else()
        SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.10 CACHE STRING "macOS Deployment Target")
    endif()
endif()

# Name the project
PROJECT(startpagedemo)

# Request the required wxWidgets libs
# Find wxWidgets and select its components
FIND_PACKAGE(wxWidgets REQUIRED core base adv html ribbon xml xrc qa propgrid OPTIONAL_COMPONENTS net)
# Include the wxWidgets use file to initialize various settings
INCLUDE(${wxWidgets_USE_FILE})
MESSAGE(STATUS "Adding wxWidgets libraries: ${wxWidgets_LIBRARIES}")
MESSAGE(STATUS "Adding wxWidgets configuration file: ${wxWidgets_CONFIGURATION}/wx/setup.h")

# Define a variable containing a list of source files for the project
SET(SRC_FILES
    startdemo.cpp
    ../startpage.cpp
    )

# Define the build target for the executable
ADD_EXECUTABLE(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SRC_FILES})

# set up compiler flags
IF(MSVC)
  TARGET_COMPILE_OPTIONS(${CMAKE_PROJECT_NAME} PRIVATE /Zc:__cplusplus /MP /W3 /WX
                         $<$<CONFIG:Debug>:/Od> $<$<CONFIG:Release>:/O2>)
  TARGET_COMPILE_DEFINITIONS(${CMAKE_PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
ELSE()
  TARGET_COMPILE_OPTIONS(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Werror
                         $<$<CONFIG:Debug>:-Og> $<$<CONFIG:Release>:-O2>)
ENDIF()

# Link required libraries to the executable
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

MESSAGE(STATUS "Copying demo resources...")
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)
