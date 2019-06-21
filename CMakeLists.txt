cmake_minimum_required(VERSION 3.5)

project(SupervisedSegmentationToolbox)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://github.com/chalupaDaniel/slicerSupervisedSegmentation")
set(EXTENSION_CATEGORY "Segmentation")
set(EXTENSION_CONTRIBUTORS "Daniel Chalupa")
set(EXTENSION_DESCRIPTION "This module allows using machine learning to segment preprocessed images using one of the included algorithms.")
set(EXTENSION_ICONURL "https://github.com/chalupaDaniel/SlicerSupervisedSegmentation/blob/master/SupervisedSegmentationToolbox.png")
set(EXTENSION_SCREENSHOTURLS "")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any
set(EXTENSION_STATUS "Work in progress")


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#-----------------------------------------------------------------------------
# Extension dependencies
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

#	DLIB
ExternalProject_Add(dlib
   GIT_REPOSITORY https://github.com/davisking/dlib.git
   GIT_TAG v19.3
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dlib
   BUILD_COMMAND ""
   UPDATE_COMMAND ""
   INSTALL_COMMAND "")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dlib)
#	SHARK

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
add_subdirectory(SupervisedSegmentationToolboxMain)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG}) 
include(${Slicer_EXTENSION_CPACK})


