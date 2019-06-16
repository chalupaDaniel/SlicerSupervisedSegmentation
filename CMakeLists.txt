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


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
include_directories(${DLIB_DIR})
#	SHARK
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

ExternalProject_Add(shark
   GIT_REPOSITORY https://github.com/Shark-ML/Shark.git
   GIT_TAG v3.1.4
   CMAKE_CACHE_ARGS -DBUILD_TESTING:BOOL=false -DBUILD_EXAMPLES:BOOL=false -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} -DENABLE_OPENMP:BOOL=false -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shark
   BUILD_IN_SOURCE 1
   UPDATE_COMMAND ""
   INSTALL_COMMAND "")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Shark/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/Shark/lib)

#	SLICER

find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(SegmentationToolboxModule)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG}) 
include(${Slicer_EXTENSION_CPACK})


