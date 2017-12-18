#ifndef PREPROCESSING_ALGORITHM_LIST_H
#define PREPROCESSING_ALGORITHM_LIST_H

#include <QList>
#include <QSharedPointer>

class PreprocessingAlgorithm;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// A complete list of included preprocessing algorithms
/// Include your own in the cpp file
///
///////////////////////////////////////////////////////////////////////////////////////////////////
struct PreprocessingAlgorithmList
{
	PreprocessingAlgorithmList();
	~PreprocessingAlgorithmList();
	QList<QSharedPointer<PreprocessingAlgorithm>> algorithms;
};
#endif //PREPROCESSING_ALGORITHM_LIST_H