#include "VolumeManager.h"

#include "PreprocessingAlgorithm.h"
#include "SupervisedClassifier.h"
#include "VolumeSelectorDialog.h"

#include "qSlicerCoreIOManager.h"
#include "qSlicerCoreApplication.h"

#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLModelStorageNode.h"

#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkCollection.h"
#include "vtkImageData.h"
#include "vtkSlicerApplicationLogic.h"

#include <QDebug>
#include <QMessageBox>

#include <thread>
#include <string>

#include <QFile>

VolumeManager::VolumeManager(qSlicerCoreIOManager* manager, const std::string& id)
	: QObject(), slicerIoManager(manager), preprocessingIdCounter(0), volumeManagerId(id), classifiedImages(0), allClassifiedCounter(0)
{

}

void VolumeManager::switchPreview(QSharedPointer<PreprocessingAlgorithm> alg, const QString& pathName)
{
	clearPreview();

	qDebug() << "Previewing " << pathName << " by " << alg->name();

	vtkSmartPointer<vtkMRMLNode> volumeNode = nullptr;
	vtkSmartPointer<vtkMRMLNode> nodeToPreprocess = loadAndGetEmptyVolume(pathName, std::string("ST Preview " + volumeManagerId),
		std::string("ST Preview " + volumeManagerId + " Preprocessed"), volumeNode);
	
	createPreprocessingJob(alg, volumeNode, nodeToPreprocess, PreprocessingJob::Preview);
}
void VolumeManager::showVolume(vtkSmartPointer<vtkMRMLNode> volumeToShow)
{
	qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode()->SetActiveVolumeID(volumeToShow->GetID());
	qSlicerCoreApplication::application()->applicationLogic()->PropagateVolumeSelection();
}

void VolumeManager::volumeClassified(vtkSmartPointer<vtkMRMLNode> result)
{
	showVolume(result);
	classifiedImages--;
	if (!classifiedImages) {
		emit classifyingComplete(result);
		clearTemporaryVolumes();

		vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> display = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(qSlicerCoreApplication::application()->mrmlScene()->GetNodeByID(result->GetNodeReferenceID("display")));
		if (display.GetPointer() != nullptr) {
			display->AutoThresholdOff();
			display->AutoWindowLevelOff();
			display->SetUpperThreshold(10.0);
			display->SetLowerThreshold(1.0);
			display->SetWindowLevel(10.0, 0.0);
			display->ApplyThresholdOn();
			display->UpdateScene(qSlicerCoreApplication::application()->mrmlScene());
		}
	}
}

void VolumeManager::preprocessingJobComplete(int refId)
{
	if (preprocessingJobQueue.contains(refId))
	{
		PreprocessingJob job = preprocessingJobQueue.value(refId); 
		qSlicerCoreApplication::application()->mrmlScene()->AddNode(job.finishedNode);
		switch (job.role)
		{
		case PreprocessingJob::Preview:
			showVolume(job.finishedNode);
			emit previewComplete();
			break;
		case PreprocessingJob::Train :
			for (size_t i = 0; i < trainingJobQueue.count(); i++) {
				int index = trainingJobQueue.at(i).preprocessingJobs.indexOf(refId);
				if (index != -1) {
					trainingJobQueue[i].preprocessingJobs.remove(index);
					if (trainingJobQueue.at(i).preprocessingJobs.isEmpty()) {
						selectedClassifier->addVolumesToTraining(trainingJobQueue.at(i).preprocessedVolumes, trainingJobQueue.at(i).truth);
						trainingJobQueue.remove(i);

						if (trainingJobQueue.isEmpty())
							selectedClassifier->startTraining();
					}
					break;
				}
			}
			break;
		case PreprocessingJob::Classify :
			for (size_t i = 0; i < classifyingJobQueue.count(); i++) {
				int index = classifyingJobQueue.at(i).preprocessingJobs.indexOf(refId);
				if (index != -1) {
					classifyingJobQueue[i].preprocessingJobs.remove(index);
					if (classifyingJobQueue.at(i).preprocessingJobs.isEmpty()) {
						selectedClassifier->addVolumesToClassifying(classifyingJobQueue.at(i).preprocessedVolumes, classifyingJobQueue.at(i).result);
						classifyingJobQueue.remove(i);
					}
					break;
				}
			}
			break;		
		default:
			break;
		}
		preprocessingJobQueue.remove(refId);
	}
}
void VolumeManager::clearPreview()
{
	std::string str1, str2;
	const char* name = str1.append("ST Preview " + volumeManagerId).c_str();
	const char* namePreprocessed = str2.append("ST Preview " + volumeManagerId + " Preprocessed").c_str();

	vtkMRMLNode* oldNode = qSlicerCoreApplication::application()->mrmlScene()->GetFirstNodeByName(name);
	if (oldNode != nullptr)
		qSlicerCoreApplication::application()->mrmlScene()->RemoveNode(oldNode);

	oldNode = qSlicerCoreApplication::application()->mrmlScene()->GetFirstNodeByName(namePreprocessed);
	if (oldNode != nullptr)
		qSlicerCoreApplication::application()->mrmlScene()->RemoveNode(oldNode);
}

void VolumeManager::clearTemporaryVolumes() {
	qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode()->Reset(nullptr);
	qSlicerCoreApplication::application()->applicationLogic()->PropagateVolumeSelection();
	vtkSmartPointer<vtkCollection> allNodes;
	allNodes.TakeReference(qSlicerCoreApplication::application()->mrmlScene()->GetNodesByClass("vtkMRMLScalarVolumeNode"));

	for (size_t i = 0; i < allNodes->GetNumberOfItems(); i++) {
		vtkSmartPointer<vtkMRMLNode> singleNode = vtkMRMLNode::SafeDownCast(allNodes->GetItemAsObject(i));
		if (QString(singleNode->GetName()).contains("ST " + QString::fromStdString(volumeManagerId) + " Set ")
			|| QString(singleNode->GetName()).contains("ST Truth " + QString::fromStdString(volumeManagerId) + " Set "))
			qSlicerCoreApplication::application()->mrmlScene()->RemoveNode(singleNode);
	}
}

QVector<QStringList> VolumeManager::lastTrainingFilepaths() const
{
	return trainingFilepaths;
}

QVector<QSharedPointer<PreprocessingAlgorithm>> VolumeManager::lastTrainingAlgorithms() const
{
	return trainingAlgs;
}

void VolumeManager::setSelectedClassifier(QSharedPointer<SupervisedClassifier> newClassifier)
{
	selectedClassifier = newClassifier;
	connect(newClassifier.data(), SIGNAL(finishedTraining()), this, SIGNAL(trainingComplete()), Qt::UniqueConnection);
	connect(newClassifier.data(), SIGNAL(finishedTraining()), this, SLOT(clearTemporaryVolumes()), Qt::UniqueConnection);
	connect(newClassifier.data(), SIGNAL(finishedClassifying(vtkSmartPointer<vtkMRMLNode>)), this, SLOT(volumeClassified(vtkSmartPointer<vtkMRMLNode>)), Qt::UniqueConnection);
}

void VolumeManager::startTrainingSequence(QVector<QStringList> filepaths, const QVector<QSharedPointer<PreprocessingAlgorithm>>& algs, QStringList truthFilePaths)
{
	// Check file number consistency
	int truthCount = truthFilePaths.count();
	bool consistent = true;
	for (size_t i = 0; i < filepaths.count(); i++) {
		if (filepaths.at(i).count() != truthCount)
			consistent = false;
	}
	if (!consistent) {
		QMessageBox::warning(qSlicerCoreApplication::activeWindow(),
			"File number inconsistent",
			"The number of files in each image sequence is inconsistent. Operation aborted.");
		emit trainingComplete();
		return;
	}

	trainingAlgs = algs;
	trainingFilepaths = filepaths;

	for (size_t i = 0; i < filepaths.at(0).count(); i++)
	{
		VolumeTrainingJob job;

		// Load volumes
		QList<qSlicerIO::IOProperties> fileParameters;
		for (size_t j = 0; j < filepaths.count(); j++)
		{
			qSlicerIO::IOProperties fileProperty;
			fileProperty["fileName"] = filepaths.at(j).at(i);
			fileProperty["fileType"] = "VolumeFile";
			fileParameters.append(fileProperty);

			vtkSmartPointer<vtkMRMLNode> loadedNode;
			vtkSmartPointer<vtkMRMLNode> nodeToPreprocess = loadAndGetEmptyVolume(filepaths.at(j).at(i), std::string("ST " + volumeManagerId + " Set " + std::to_string(i) + " Image " + std::to_string(j)),
				std::string("ST " + volumeManagerId + " Set " + std::to_string(i)	+ " Image " + std::to_string(j) + " Preprocessed"),
				loadedNode);

			// Append volumes to job
			job.volumes.append(loadedNode);

			// Append volumes to preprocess to job
			job.preprocessedVolumes.append(nodeToPreprocess);
		}
		// Set preprocessing algorithms to job
		job.algorithms = algs;

		// Append truth to job
		job.truth = loadAndGetLoadedVolume(truthFilePaths.at(i), std::string("ST Truth " + volumeManagerId + " Set " + std::to_string(i)));

		trainingJobQueue.append(job);
	}
	for (size_t i = 0; i < trainingJobQueue.count(); i++)
	{
		VolumeTrainingJob& job = trainingJobQueue[i];
		for (int j = 0; j < job.algorithms.count(); j++)
		{
			int id = createPreprocessingJob(job.algorithms.at(j), job.volumes[j], job.preprocessedVolumes[j], PreprocessingJob::Train);
			if (id != -1) {
				job.preprocessingJobs.append(id);
			}
			else {
				job.preprocessedVolumes[j] = job.volumes[j];
			}
		}
		if (job.preprocessingJobs.isEmpty())
		{
			selectedClassifier->addVolumesToTraining(job.volumes, job.truth);
			trainingJobQueue.remove(i);
			i--;
			continue;
		}
	}
	if (trainingJobQueue.isEmpty())
		selectedClassifier->startTraining();
}

void VolumeManager::startClassifyingSequence(QVector<QStringList> filepaths, const QVector<QSharedPointer<PreprocessingAlgorithm>>& algs)
{
	// Check file number consistency
	bool consistent = filepaths.count() > 0;
	if (consistent) {
		for (size_t i = 0; i < (filepaths.count() - 1); i++) {
			if (filepaths.at(i).count() != filepaths.at(i + 1).count())
				consistent = false;
		}
	}
	if (!consistent) {
		QMessageBox::warning(qSlicerCoreApplication::activeWindow(),
			"File number inconsistent",
			"The number of files in each image sequence is inconsistent. Operation aborted.");
		emit trainingComplete();
		return;
	}

	for (size_t i = 0; i < filepaths.at(0).count(); i++)
	{
		VolumeClassifyingJob job;

		// Load volumes
		for (size_t j = 0; j < filepaths.count(); j++)
		{
			vtkSmartPointer<vtkMRMLNode> loadedNode;
			vtkSmartPointer<vtkMRMLNode> nodeToPreprocess = loadAndGetEmptyVolume(filepaths.at(j).at(i), std::string("ST " + volumeManagerId + " Set " + std::to_string(i) + " Image " + std::to_string(j)),
				std::string("ST " + volumeManagerId + " Set " + std::to_string(i) + " Image " + std::to_string(j) + " Preprocessed"),
				loadedNode);

			// Append volumes to job
			job.volumes.append(loadedNode);

			// Append volumes to preprocess to job
			job.preprocessedVolumes.append(nodeToPreprocess);
		}

		//vtkSmartPointer<vtkMRMLNode> vol = loadAndGetLoadedVolume(filepaths.at(0).at(i), std::string("ST " + volumeManagerId + " Result " + std::to_string(i)).c_str());

		vtkNew<vtkMRMLScalarVolumeDisplayNode> volDisplay;
		
		vtkNew<vtkImageData> volData;
		volData.GetPointer()->SetDimensions(vtkMRMLVolumeNode::SafeDownCast(job.volumes.front())->GetImageData()->GetDimensions());
		volData.GetPointer()->AllocateScalars(VTK_UNSIGNED_INT, 1);
		volDisplay.GetPointer()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");

		qSlicerCoreApplication::application()->mrmlScene()->AddNode(volDisplay.GetPointer());

		vtkNew<vtkMRMLScalarVolumeNode> vol;
		vol.GetPointer()->SetName(std::string("ST " + volumeManagerId + " Result " + std::to_string(allClassifiedCounter) + std::to_string(i)).c_str());
		allClassifiedCounter++;
		vol.GetPointer()->AddAndObserveDisplayNodeID(volDisplay.GetPointer()->GetID());

		qSlicerCoreApplication::application()->mrmlScene()->AddNode(vol.GetPointer());

		vol->SetAndObserveImageData(volData.GetPointer());


		// set origin and rotation
		vol.GetPointer()->SetOrigin(vtkMRMLVolumeNode::SafeDownCast(job.volumes.front())->GetOrigin());
		vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
		vtkMRMLVolumeNode::SafeDownCast(job.volumes.front())->GetIJKToRASDirectionMatrix(mat);
		vol.GetPointer()->SetIJKToRASDirectionMatrix(mat);

		job.result = vtkMRMLNode::SafeDownCast(vol.GetPointer());

		classifyingJobQueue.append(job);
		classifiedImages++;
	}
	for (size_t i = 0; i < classifyingJobQueue.count(); i++)
	{
		VolumeClassifyingJob& job = classifyingJobQueue[i];
		for (int j = 0; j < job.algorithms.count(); j++)
		{
			int id = createPreprocessingJob(job.algorithms.at(j), job.volumes[j], job.preprocessedVolumes[j], PreprocessingJob::Classify);
			if (id != -1) {
				job.preprocessingJobs.append(id);
			}
			else {
				job.preprocessedVolumes[j] = job.volumes[j];
			}
		}
		if (job.preprocessingJobs.isEmpty())
		{
			selectedClassifier->addVolumesToClassifying(job.volumes, job.result);
			classifyingJobQueue.remove(i);
			i--;
			continue;
		}
	}
}

vtkSmartPointer<vtkMRMLNode> VolumeManager::loadAndGetEmptyVolume(const QString& filename, const std::string& loadedNodeName, const std::string& emptyNodeName, vtkSmartPointer<vtkMRMLNode>& original)
{
	original = loadAndGetLoadedVolume(filename, loadedNodeName); 

	if (original == nullptr)
		return original;

	vtkSmartPointer<vtkMRMLScalarVolumeNode> vol = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
	// set origin and rotation
	vol->SetOrigin(vtkMRMLVolumeNode::SafeDownCast(original)->GetOrigin());
	vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkMRMLVolumeNode::SafeDownCast(original)->GetIJKToRASDirectionMatrix(mat);
	vol->SetIJKToRASDirectionMatrix(mat);

	vol->SetName(emptyNodeName.c_str());

	return vol;
}

vtkSmartPointer<vtkMRMLNode> VolumeManager::loadAndGetLoadedVolume(const QString& filename, const std::string& loadedNodeName)
{
	vtkSmartPointer<vtkMRMLNode> loadedVolume;
	qSlicerIO::IOProperties fileParameters;
	fileParameters["fileName"] = filename;

	// This loads the node to scene, unexpectedly

	bool dummy = false;
	if (VolumeSelectorDialog::isVolumeExternal(filename, dummy)) {
		loadedVolume = slicerIoManager->loadNodesAndGetFirst("VolumeFile", fileParameters);
		loadedVolume->SetName(loadedNodeName.c_str());
	}
	else {
		const char* c = filename.toLatin1().data();
		loadedVolume = vtkMRMLScalarVolumeNode::
				SafeDownCast(qSlicerCoreApplication::
					application()->mrmlScene()->GetFirstNodeByName(c));
	}

	return loadedVolume;
}

int VolumeManager::createPreprocessingJob(QSharedPointer<PreprocessingAlgorithm> alg, vtkSmartPointer<vtkMRMLNode>& original, vtkSmartPointer<vtkMRMLNode>& preprocessingResult, PreprocessingJob::VolumeRole role)
{
	if (original == nullptr || preprocessingResult == nullptr)
	{
		qDebug() << "VolumeManager::Incorrect volumes loaded. Aborting preprocessing job.";
		return -1;
	}

	preprocessingIdCounter++;


	PreprocessingJob job;
	job.role = role;
	job.finishedNode = preprocessingResult;

	preprocessingJobQueue.insert(preprocessingIdCounter, job);

	if (alg.isNull())
	{
		preprocessingJobQueue[preprocessingJobQueue.count() - 1].finishedNode = original;
		preprocessingJobComplete(preprocessingIdCounter);
		return -1;
	}

	connect(alg.data(), SIGNAL(preprocessingDone(int)), this, SLOT(preprocessingJobComplete(int)), Qt::UniqueConnection);

	std::thread t(&PreprocessingAlgorithm::preprocess, alg, original, job.finishedNode, preprocessingIdCounter);
	t.detach();

	return preprocessingIdCounter;
}