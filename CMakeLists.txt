cmake_minimum_required(VERSION 3.5)

project(SegmentationToolbox)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://github.com/chalupaDaniel/slicerSupervisedSegmentation")
set(EXTENSION_CATEGORY "Segmentation")
set(EXTENSION_CONTRIBUTORS "Daniel Chalupa")
set(EXTENSION_DESCRIPTION "This module allows using machine learning to segment preprocessed images using one of the included algorithms.")
set(EXTENSION_ICONURL "https://github.com/chalupaDaniel/slicerSupervisedSegmentation/blob/master/SegmentationToolbox.png")
set(EXTENSION_SCREENSHOTURLS "")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any
set(EXTENSION_STATUS "Work in progress")

#-----------------------------------------------------------------------------
# Download and build dependencies
#add_custom_command(
#    OUTPUT ${CMAKE_SOURCE_DIR}
#    WORKING_DIR ${CMAKE_SOURCE_DIR}
#    COMMAND ./downloadAndBuildDeps
#)

#-----------------------------------------------------------------------------
# Extension dependencies

#	DLIB
find_path(DLIB_DIR NAMES README.md  PATHS dlib)
if ((NOT DLIB_DIR) OR (NOT EXISTS ${DLIB_DIR}))
	message("DLIB not found")
	execute_process(COMMAND git clone --branch v19.3 https://github.com/davisking/dlib.git)
	set(DLIB_DIR dlib CACHE PATH "Path to dlib")
endif ()
if ((NOT DLIB_DIR) OR (NOT EXISTS ${DLIB_DIR}))
	message(FATAL_ERROR "dlib could not be cloned")
endif ()
message("DLIB found in ${DLIB_DIR}")

#	SHARK
find_path(SHARK_DIR NAMES CMakeLists.txt PATHS Shark)
if ((NOT SHARK_DIR) OR (NOT EXISTS ${SHARK_DIR}))
        message("Shark not found")
        execute_process(COMMAND git clone --branch v3.1.4 https://github.com/Shark-ML/Shark.git)
        set(SHARK_DIR Shark CACHE PATH "Path to shark")
endif ()
if ((NOT SHARK_DIR) OR (NOT EXISTS ${SHARK_DIR}))
        message(FATAL_ERROR "shark could not be cloned")
endif ()
message("Shark found in ${SHARK_DIR}")

#	SLICER

find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE}) 
include_directories(${DLIB_DIR})

set(ENABLE_OPENMP FALSE)
ExternalProject_Add(Shark
   SOURCE_DIR ${SHARK_DIR}
   BUILD_IN_SOURCE 1
   INSTALL "")
#ExternalProject_Get_Property(Shark SHARK_USE_FILE)
#include(${SHARK_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(SegmentationToolboxModule)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG}) 
include(${Slicer_EXTENSION_CPACK})
