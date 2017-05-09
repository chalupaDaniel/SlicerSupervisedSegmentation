#include "PreprocessingAlgorithmList.h"

#include "PreprocessingAlgorithm.h"
#include "PreprocessingMedian3d.h"
#include "PreprocessingSobelOperator.h"
#include "PreprocessingGaussFilter.h"

PreprocessingAlgorithmList::PreprocessingAlgorithmList()
{
	algorithms.append(QSharedPointer<PreprocessingMedian3d>(new PreprocessingMedian3d()));
	algorithms.append(QSharedPointer<PreprocessingGaussFilter>(new PreprocessingGaussFilter()));
	algorithms.append(QSharedPointer<PreprocessingSobel2d>(new PreprocessingSobel2d()));
}

PreprocessingAlgorithmList::~PreprocessingAlgorithmList()
{
}