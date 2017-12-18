#ifndef CLASSIFIER_LIST_H
#define CLASSIFIER_LIST_H

#include <QSharedPointer>

class SupervisedClassifier;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// A complete list of included Classifier
/// Include your own classifier in the cpp file
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class ClassifierList
{
public:
	ClassifierList();
	~ClassifierList();

	SupervisedClassifier* returnCopyPointer(const QString& classifierName);

	QList<QSharedPointer<SupervisedClassifier>> classifiers;
};
#endif // CLASSIFIER_LIST_H