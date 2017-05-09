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
  ../Classifying/ClassifierDlibSvmNu.h
  ../Classifying/ClassifierDlibSvmC.h
  ../Classifying/ClassifierSharkSvmC.h
  ../Classifying/ClassifierSharkRandomForest.h
  SegmentationToolboxTrainingWidget.h
  SegmentationToolboxClassifyingWidget.h
  VolumeManager.h
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

set(${KIT}_TARGET_LIBRARIES
  ${SHARK_LIBRARIES}
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
