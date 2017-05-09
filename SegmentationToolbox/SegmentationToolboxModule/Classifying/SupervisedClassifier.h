#ifndef SUPERVISED_CLASSIFIER_H
#define SUPERVISED_CLASSIFIER_H

#include <QObject>
#include <QVector>
#include <QSharedPointer>
#include "vtkSmartPointer.h"

class vtkMRMLNode;
class QWidget;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Template class for supervised classifiers
/// When subclassing:
/// - set the classifierName
/// - overload serialization/deserialization functions
/// - implement your classifier to accept training volumes
/// - properly emit finishedTraining signal, when finished
/// - add your classifier to ClassifierList.h
/// - when needed, set the widgets to include your classifier settings
/// - when in doubt, look at finished classifiers
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class SupervisedClassifier : public QObject
{
	Q_OBJECT
public:
	SupervisedClassifier();
	~SupervisedClassifier();

	QString name() const;
	QWidget* classifyingWidget() const;
	QWidget* trainingWidget() const;

	virtual QByteArray serialize() = 0;
	virtual bool deserialize(const QByteArray& serializedClassifier) = 0;
signals:
	// This signal is necessary for UI unlocking after training
	void finishedTraining();
	void finishedClassifying(vtkSmartPointer<vtkMRMLNode> result);

public slots:
	// It is not necessary to start training on this slot, but it is recommended
	// This function is called from VolumeManager (main thread)
	virtual void startTraining() = 0;
	// This function is called from VolumeManager (main thread)
	virtual void addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth) = 0;
	// This function is called from VolumeManager (main thread)
	virtual void addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result) = 0;

protected:
	QString classifierName;
	QWidget* classifyWidget;
	QWidget* trainWidget;
};
#endif // SUPERVISED_CLASSIFIER_H