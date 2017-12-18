#include "SupervisedClassifier.h"

#include <QWidget>
#include <QMetaType>

SupervisedClassifier::SupervisedClassifier()
	: QObject(), classifyWidget(new QWidget()), trainWidget(new QWidget())
{
	qRegisterMetaType<vtkSmartPointer<vtkMRMLNode>>("vtkSmartPointer<vtkMRMLNode>");
}
SupervisedClassifier::~SupervisedClassifier()
{
	classifyWidget->deleteLater();
	trainWidget->deleteLater();
}

QString SupervisedClassifier::name() const
{
	return classifierName;
}
QWidget* SupervisedClassifier::classifyingWidget() const
{
	return classifyWidget;
}
QWidget* SupervisedClassifier::trainingWidget() const
{
	return trainWidget;
}