#############################################################################
# Name:        CMakeListst.txt
# Purpose:     Sample CMake file to show usage of cmake for wxWidgets users
# Author:      Tobias Taschner
# Created:     2016-10-23
# Copyright:   (c) 2016 wxWidgets development team
# Licence:     wxWindows licence
#############################################################################

#
# This file is just a sample to show using cmake from an application
# If you want to build the minimal and other samples please use the
# wxBUILD_SAMPLES option when using cmake on the library
#

# Declare the minimum required CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    # If no deployment target has been set default to the minimum supported
    # OS version (this has to be set before the first project() call)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "iOS Deployment Target")
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10 CACHE STRING "macOS Deployment Target")
    endif()
endif()

# Name the project
PROJECT(startpagedemo)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

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
# Link required libraries to the executable
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

MESSAGE(STATUS "Copying demo resources...")
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)
