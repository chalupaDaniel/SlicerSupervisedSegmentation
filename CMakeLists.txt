cmake_minimum_required(VERSION 3.5)

project(SegmentationToolbox)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://github.com/chalupaDaniel/slicerSupervisedSegmentation")
set(EXTENSION_CATEGORY "Segmentation")
set(EXTENSION_CONTRIBUTORS "Daniel Chalupa")
set(EXTENSION_DESCRIPTION "This is an example of a simple extension")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/SegmentationToolbox.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/SegmentationToolbox/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
set(DLIB_INCLUDE "" CACHE PATH "Path to dlib")
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
include_directories(${DLIB_INCLUDE})

set(BOOST_LIB "" CACHE PATH "Path to boost libs")
link_directories(${BOOST_LIB})
find_package(Shark REQUIRED)
include(${SHARK_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(SegmentationToolboxModule)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
