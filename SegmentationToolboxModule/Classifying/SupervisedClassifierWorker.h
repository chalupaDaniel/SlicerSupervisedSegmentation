#ifndef SUPERVISED_CLASSIFIER_WORKER_H
#define SUPERVISED_CLASSIFIER_WORKER_H

#include "vtkSmartPointer.h"
#include "vtkMRMLNode.h"

#include <QVector>
#include <QThread>

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// SingleVolumeEntry
/// - structure representing all volumes that the classifier trains in one go
/// - used for saving training vectors from volumes in worker
///
///////////////////////////////////////////////////////////////////////////////////////////////////
struct SingleVolumeTrainingEntry
{
	QVector<vtkSmartPointer<vtkMRMLNode>> volumes;
	vtkSmartPointer<vtkMRMLNode> truth;
};
struct SingleVolumeClassifyingEntry
{
	QVector<vtkSmartPointer<vtkMRMLNode>> volumes;
	vtkSmartPointer<vtkMRMLNode> result;
};

class QWaitCondition;
class QMutex;
class SupervisedClassifierWorker : public QThread
{
	Q_OBJECT
public:
	SupervisedClassifierWorker();
	~SupervisedClassifierWorker();

	void appendToTrainingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth);
	void appendToClassifyingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result);

	void run() override final;

public slots:
	void startTraining();

signals:
	void classifierTrained();

protected:
	virtual void processTraining() = 0;
	virtual void processTrainingEntry(const SingleVolumeTrainingEntry& entry) = 0;
	virtual void processClassifyingEntry(const SingleVolumeClassifyingEntry& entry) = 0;

private:
	QMutex* mutex;
	QWaitCondition* waitCondition;
	bool readyToTrain;

	QVector<SingleVolumeTrainingEntry> trainingEntriesQueue;

	QVector<SingleVolumeClassifyingEntry> classifyingEntriesQueue;
};
#endif // SUPERVISED_CLASSIFIER_WORKER_H
