project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )
  
include_directories("../Classifying")
include_directories("../Preprocessing")

set(SRC_CLASSIFY
  ../Classifying/SupervisedClassifier.cpp
  ../Classifying/SupervisedClassifier.h
  ../Classifying/SupervisedClassifierWorker.cpp
  ../Classifying/SupervisedClassifierWorker.h
  ../Classifying/ClassifierList.cpp
  ../Classifying/ClassifierList.h
  ../Classifying/ClassifierDlibSvmNu.cpp
  ../Classifying/ClassifierDlibSvmNu.h
  ../Classifying/ClassifierDlibSvmC.cpp
  ../Classifying/ClassifierDlibSvmC.h
  ../Classifying/ClassifierSharkSvmC.cpp
  ../Classifying/ClassifierSharkSvmC.h
  ../Classifying/ClassifierSharkRandomForest.cpp
  ../Classifying/ClassifierSharkRandomForest.h
  ../Classifying/ClassificationResultCombinator.cpp
  ../Classifying/ClassificationResultCombinator.h
)

set(SRC_PREPROCESS
  ../Preprocessing/ImageRangePreprocessingSelector.cpp
  ../Preprocessing/ImageRangePreprocessingSelector.h
  ../Preprocessing/PreprocessingAlgorithm.cpp
  ../Preprocessing/PreprocessingAlgorithm.h
  ../Preprocessing/PreprocessingAlgorithmList.cpp
  ../Preprocessing/PreprocessingAlgorithmList.h
  ../Preprocessing/PreprocessingMedian3d.cpp
  ../Preprocessing/PreprocessingMedian3d.h
  ../Preprocessing/PreprocessingSobelOperator.cpp
  ../Preprocessing/PreprocessingSobelOperator.h
  ../Preprocessing/PreprocessingGaussFilter.cpp
  ../Preprocessing/PreprocessingGaussFilter.h
)

set(${KIT}_SRCS
  SegmentationToolboxTrainingWidget.cpp
  SegmentationToolboxTrainingWidget.h
  SegmentationToolboxClassifyingWidget.cpp
  SegmentationToolboxClassifyingWidget.h
  VolumeManager.cpp
  VolumeManager.h
  VolumeSelectorDialog.cpp
  VolumeSelectorDialog.h
  ClassifierWidget.cpp
  ClassifierWidget.h
  ${SRC_PREPROCESS}
  ${SRC_CLASSIFY}
  )

set(${KIT}_MOC_SRCS
  ../Preprocessing/ImageRangePreprocessingSelector.h
  ../Preprocessing/PreprocessingAlgorithm.h
  ../Preprocessing/PreprocessingMedian3d.h
  ../Preprocessing/PreprocessingSobelOperator.h
  ../Preprocessing/PreprocessingGaussFilter.h
  ../Classifying/SupervisedClassifier.h
  ../Classifying/SupervisedClassifierWorker.h
  ../Classifying/ClassifierDlibSvmNu.h
  ../Classifying/ClassifierDlibSvmC.h
  ../Classifying/ClassifierSharkSvmC.h
  ../Classifying/ClassifierSharkRandomForest.h
  ../Classifying/ClassificationResultCombinator.h
  SegmentationToolboxTrainingWidget.h
  SegmentationToolboxClassifyingWidget.h
  VolumeManager.h
  VolumeSelectorDialog.h
  ClassifierWidget.h
  )

set(${KIT}_UI_SRCS
  ../Resources/UI/SegmentationToolboxTrainingWidget.ui
  ../Resources/UI/SegmentationToolboxClassifyingWidget.ui
  ../Resources/UI/Classifiers/ClassifierSharkSvmCClassify.ui
  ../Resources/UI/Classifiers/ClassifierSharkRandomForestClassify.ui
  ../Resources/UI/Classifiers/ClassifierDlibSvmCClassify.ui
  ../Resources/UI/Classifiers/ClassifierDlibSvmNuClassify.ui
  ../Resources/UI/Classifiers/ClassifierSharkSvmCTrain.ui
  ../Resources/UI/Classifiers/ClassifierSharkRandomForestTrain.ui
  ../Resources/UI/Classifiers/ClassifierDlibSvmCTrain.ui
  ../Resources/UI/Classifiers/ClassifierDlibSvmNuTrain.ui
  )

set(${KIT}_RESOURCES
  )

# 	BOOST
find_package( 
        Boost REQUIRED COMPONENTS
        system date_time filesystem
        program_options serialization thread
        unit_test_framework
)
include_directories(SYSTEM ${Boost_INCLUDE_DIR} )
link_directories( ${Boost_LIBRARY_DIR} )
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(SHARK_DEBUG_SUFFIX _debug)
else ()
  set(SHARK_DEBUG_SUFFIX "")
endif ()

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  ${CMAKE_STATIC_LIBRARY_PREFIX}shark${SHARK_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX} 
  ${Boost_LIBRARIES}
  )
#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )
