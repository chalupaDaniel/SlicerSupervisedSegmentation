#include "SupervisedClassifierWorker.h"

#include <QMutex>
#include <QWaitCondition>

SupervisedClassifierWorker::SupervisedClassifierWorker()
	: QThread(), mutex(new QMutex()),
	waitCondition(new QWaitCondition()), readyToTrain(false)
{

}
SupervisedClassifierWorker::~SupervisedClassifierWorker()
{
	requestInterruption();
	waitCondition->wakeAll();
	wait();

	delete mutex;
	delete waitCondition;
}

void SupervisedClassifierWorker::appendToTrainingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth)
{
	QMutexLocker lock(mutex);

	SingleVolumeTrainingEntry entry;
	entry.truth = truth;
	entry.volumes = volumes;
	trainingEntriesQueue.append(entry);

	waitCondition->wakeAll();
}
void SupervisedClassifierWorker::appendToClassifyingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result)
{
	QMutexLocker lock(mutex);

	SingleVolumeClassifyingEntry entry;
	entry.result = result;
	entry.volumes = volumes;
	classifyingEntriesQueue.append(entry);

	waitCondition->wakeAll();
}

void SupervisedClassifierWorker::startTraining()
{
	QMutexLocker lock(mutex);
	readyToTrain = true;

	waitCondition->wakeAll();
}

void SupervisedClassifierWorker::run()
{
	while (1 > 0) {
		if (isInterruptionRequested())
			return;

		QMutexLocker lock(mutex);
		bool trainingEntriesQueueEmpty = trainingEntriesQueue.isEmpty();
		bool classifyingEntriesQueueEmpty = classifyingEntriesQueue.isEmpty();
		bool isReadyToTrain = readyToTrain;
		lock.unlock();

		if (trainingEntriesQueueEmpty && classifyingEntriesQueueEmpty && isReadyToTrain) {
			processTraining();

			emit classifierTrained();

			lock.relock();
			readyToTrain = false;
			waitCondition->wait(mutex);
			lock.unlock();
		}
		else if (!trainingEntriesQueueEmpty) {
			lock.relock();
			SingleVolumeTrainingEntry entry = trainingEntriesQueue.takeFirst();
			lock.unlock();

			processTrainingEntry(entry);
		}
		else if (!classifyingEntriesQueueEmpty) {
			lock.relock();
			SingleVolumeClassifyingEntry entry = classifyingEntriesQueue.takeFirst();
			lock.unlock();

			processClassifyingEntry(entry);
		} else {
			lock.relock();
			waitCondition->wait(mutex);
			lock.unlock();
		}
	}
}