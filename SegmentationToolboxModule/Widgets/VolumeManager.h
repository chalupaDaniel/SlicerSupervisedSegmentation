#ifndef VOLUME_MANAGER_H
#define VOLUME_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSharedPointer>
#include <QHash>
#include "vtkSmartPointer.h"

class PreprocessingAlgorithm;
class vtkMRMLNode;
struct VolumeTrainingJob
{
	QVector<vtkSmartPointer<vtkMRMLNode>> volumes;
	QVector<vtkSmartPointer<vtkMRMLNode>> preprocessedVolumes;
	vtkSmartPointer<vtkMRMLNode> truth;
	QVector<QSharedPointer<PreprocessingAlgorithm>> algorithms;
	QVector<int> preprocessingJobs;
};
struct VolumeClassifyingJob
{
	QVector<vtkSmartPointer<vtkMRMLNode>> volumes;
	QVector<vtkSmartPointer<vtkMRMLNode>> preprocessedVolumes;
	vtkSmartPointer<vtkMRMLNode> result;
	QVector<QSharedPointer<PreprocessingAlgorithm>> algorithms;
	QVector<int> preprocessingJobs;
};

struct PreprocessingJob
{
	enum VolumeRole {
		Preview,
		Train,
		Classify
	};
	vtkSmartPointer<vtkMRMLNode> finishedNode;
	VolumeRole role;
};

class qSlicerCoreIOManager;
class SupervisedClassifier;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
///
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class VolumeManager : public QObject
{
	Q_OBJECT
public:
	VolumeManager(qSlicerCoreIOManager* manager, const std::string& id);
	~VolumeManager() {}

	QVector<QStringList> lastTrainingFilepaths() const;
	QVector<QSharedPointer<PreprocessingAlgorithm>> lastTrainingAlgorithms() const;

	void setSelectedClassifier(QSharedPointer<SupervisedClassifier> newClassifier);

	void startTrainingSequence(QVector<QStringList> filepaths, const QVector<QSharedPointer<PreprocessingAlgorithm>>& algs, QStringList truthFilePaths);
	void startClassifyingSequence(QVector<QStringList> filepaths, const QVector<QSharedPointer<PreprocessingAlgorithm>>& algs);

signals:
	void previewComplete();
	void trainingComplete();
	void classifyingComplete(vtkSmartPointer<vtkMRMLNode> result);

public slots:
	void switchPreview(QSharedPointer<PreprocessingAlgorithm> alg, const QString& pathName);
	void showVolume(vtkSmartPointer<vtkMRMLNode> volumeToShow);

private:
	void clearPreview();
	int createPreprocessingJob(QSharedPointer<PreprocessingAlgorithm> alg, vtkSmartPointer<vtkMRMLNode>& original,
		vtkSmartPointer<vtkMRMLNode>& preprocessingResult, PreprocessingJob::VolumeRole role);
	vtkSmartPointer<vtkMRMLNode> loadAndGetLoadedVolume(const QString& filename, const std::string& loadedNodeName);
	vtkSmartPointer<vtkMRMLNode> loadAndGetEmptyVolume(const QString& filename, const std::string& loadedNodeName, const std::string& emptyNodeName, vtkSmartPointer<vtkMRMLNode>& original);

	qSlicerCoreIOManager* slicerIoManager;
	QString lastPreview;
	std::string volumeManagerId;
	QSharedPointer<SupervisedClassifier> selectedClassifier;
	QVector<VolumeTrainingJob> trainingJobQueue;
	QVector <VolumeClassifyingJob> classifyingJobQueue;
	QHash<int, PreprocessingJob> preprocessingJobQueue;
	QVector<QStringList> trainingFilepaths;
	QVector<QSharedPointer<PreprocessingAlgorithm>> trainingAlgs;

	int preprocessingIdCounter;
	int classifiedImages;
	int allClassifiedCounter;

private slots:
	void preprocessingJobComplete(int refId);
	void volumeClassified(vtkSmartPointer<vtkMRMLNode> result);
	void clearTemporaryVolumes();
};

#endif // VOLUME_MANAGER_H