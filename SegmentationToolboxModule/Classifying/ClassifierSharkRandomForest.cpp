#include "ClassifierSharkRandomForest.h"

#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QMutex>
#include <QMessageBox>
#include <QLineEdit>
#include <QDateTime>

#include "vtkMRMLNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkImageData.h"

#include "qSlicerApplication.h"

#include <fstream>
#include <thread>

#include <QDebug>
#include <QFile>

using namespace ClassifierSharkRandomForestNamespace;

ClassifierSharkRandomForest::ClassifierSharkRandomForest()
	: SupervisedClassifier()
{
	classifierName = "Random Forest - Shark";

	ui.setupUi(trainWidget);
	classifyUi.setupUi(classifyWidget);

	ui.selectSlices->setChecked(true);

	ui.bottomSliceLimit->setVisible(true);
	ui.bottomSliceLimitLabel->setVisible(true);
	ui.topSliceLimit->setVisible(true);
	ui.topSliceLimitLabel->setVisible(true);

	ui.randAttr->setVisible(true);
	ui.randAttrLabel->setVisible(true);
	ui.numTrees->setVisible(true);
	ui.numTreesLabel->setVisible(true);
	ui.nodeSize->setVisible(true);
	ui.nodeSizeLabel->setVisible(true);
	ui.oob->setVisible(true);
	ui.oobLabel->setVisible(true);

	ui.randAttrLow->setVisible(false);
	ui.randAttrLowLabel->setVisible(false);
	ui.randAttrHigh->setVisible(false);
	ui.randAttrHighLabel->setVisible(false);
	ui.randAttrSteps->setVisible(false);
	ui.randAttrStepsLabel->setVisible(false);

	ui.numTreesLow->setVisible(false);
	ui.numTreesLowLabel->setVisible(false);
	ui.numTreesHigh->setVisible(false);
	ui.numTreesHighLabel->setVisible(false);
	ui.numTreesSteps->setVisible(false);
	ui.numTreesStepsLabel->setVisible(false);

	ui.nodeSizeLow->setVisible(false);
	ui.nodeSizeLowLabel->setVisible(false);
	ui.nodeSizeHigh->setVisible(false);
	ui.nodeSizeHighLabel->setVisible(false);
	ui.nodeSizeSteps->setVisible(false);
	ui.nodeSizeStepsLabel->setVisible(false);

	ui.oobLow->setVisible(false);
	ui.oobLowLabel->setVisible(false);
	ui.oobHigh->setVisible(false);
	ui.oobHighLabel->setVisible(false);
	ui.oobSteps->setVisible(false);
	ui.oobStepsLabel->setVisible(false);

	worker = new ClassifierSharkRandomForestWorker();
	worker->start();

	connect(worker, SIGNAL(volumeClassified(vtkSmartPointer<vtkMRMLNode>)), this, SIGNAL(finishedClassifying(vtkSmartPointer<vtkMRMLNode>)));
	connect(worker, SIGNAL(classifierTrained()), this, SIGNAL(finishedTraining()));
	connect(ui.selectSlices, SIGNAL(toggled(bool)), this, SLOT(selectSlicesToggled(bool)));
	connect(classifyUi.selectSlices, SIGNAL(toggled(bool)), this, SLOT(selectClassifySlicesToggled(bool)));
	connect(ui.setAutomatically, SIGNAL(toggled(bool)), this, SLOT(setAutomaticallyToggled(bool)));
	connect(worker, SIGNAL(valuesOptimized(int, int, int, double)), this, SLOT(valuesOptimized(int, int, int, double)));
	connect(worker, SIGNAL(infoMessageBoxRequest(QString, QString, QString)), this, SLOT(infoMessageBox(QString, QString, QString)), Qt::QueuedConnection);
}
ClassifierSharkRandomForest::~ClassifierSharkRandomForest()
{
	delete worker;
}

QByteArray ClassifierSharkRandomForest::serialize()
{
	ofstream ofs("tempSave.dat");
	boost::archive::polymorphic_text_oarchive oa(ofs);
	worker->decisionFunction.save(oa, 1);
	ofs.close();

	QFile file("tempSave.dat");
	file.open(QIODevice::ReadOnly);
	QByteArray out = file.readAll();
	file.close();
	file.remove();


	return out;
}

bool ClassifierSharkRandomForest::deserialize(const QByteArray& serializedClassifier)
{
	QFile file("tempLoad.dat");
	file.open(QIODevice::WriteOnly);
	file.write(serializedClassifier);
	file.close();

	ifstream ifs("tempLoad.dat");
	boost::archive::polymorphic_text_iarchive ia(ifs);
	worker->decisionFunction.load(ia, 1);
	ifs.close();
	file.remove();

	return true;
}

void ClassifierSharkRandomForest::addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth)
{
	worker->setCropping(ui.selectSlices->isChecked());
	worker->setBottomSlice(ui.bottomSliceLimit->text().toInt());
	worker->setTopSlice(ui.topSliceLimit->text().toInt());

	// Selected labels need to be known before any loading occurs (see run worker's run method)
	QVector<unsigned int> labels;
	QList<QString> labelsStrings = ui.selectedLabels->text().split(",");
	for (const QString& i : labelsStrings) {
		labels.append(i.toUInt());
	}
	worker->selectLabels(labels);

	worker->appendToTrainingQueue(volumes, truth);
}

void ClassifierSharkRandomForest::addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result)
{
	worker->setClassifyCropping(classifyUi.selectSlices->isChecked());
	worker->setClassifyBottomSlice(classifyUi.bottomSliceLimit->text().toInt());
	worker->setClassifyTopSlice(classifyUi.topSliceLimit->text().toInt());

	worker->appendToClassifyingQueue(volumes, result);
}

void ClassifierSharkRandomForest::setAutomaticallyToggled(bool state)
{
	ui.randAttr->setVisible(!state);
	ui.randAttrLabel->setVisible(!state);
	ui.numTrees->setVisible(!state);
	ui.numTreesLabel->setVisible(!state);
	ui.nodeSize->setVisible(!state);
	ui.nodeSizeLabel->setVisible(!state);
	ui.oob->setVisible(!state);
	ui.oobLabel->setVisible(!state);

	ui.randAttrLow->setVisible(state);
	ui.randAttrLowLabel->setVisible(state);
	ui.randAttrHigh->setVisible(state);
	ui.randAttrHighLabel->setVisible(state);
	ui.randAttrSteps->setVisible(state);
	ui.randAttrStepsLabel->setVisible(state);

	ui.numTreesLow->setVisible(state);
	ui.numTreesLowLabel->setVisible(state);
	ui.numTreesHigh->setVisible(state);
	ui.numTreesHighLabel->setVisible(state);
	ui.numTreesSteps->setVisible(state);
	ui.numTreesStepsLabel->setVisible(state);

	ui.nodeSizeLow->setVisible(state);
	ui.nodeSizeLowLabel->setVisible(state);
	ui.nodeSizeHigh->setVisible(state);
	ui.nodeSizeHighLabel->setVisible(state);
	ui.nodeSizeSteps->setVisible(state);
	ui.nodeSizeStepsLabel->setVisible(state);

	ui.oobLow->setVisible(state);
	ui.oobLowLabel->setVisible(state);
	ui.oobHigh->setVisible(state);
	ui.oobHighLabel->setVisible(state);
	ui.oobSteps->setVisible(state);
	ui.oobStepsLabel->setVisible(state);

}

void ClassifierSharkRandomForest::selectSlicesToggled(bool state)
{
	ui.bottomSliceLimit->setVisible(state);
	ui.bottomSliceLimitLabel->setVisible(state);
	ui.topSliceLimit->setVisible(state);
	ui.topSliceLimitLabel->setVisible(state);
}

void ClassifierSharkRandomForest::selectClassifySlicesToggled(bool state)
{
	classifyUi.bottomSliceLimit->setVisible(state);
	classifyUi.bottomSliceLimitLabel->setVisible(state);
	classifyUi.topSliceLimit->setVisible(state);
	classifyUi.topSliceLimitLabel->setVisible(state);
}

void ClassifierSharkRandomForest::valuesOptimized(int randAttr, int numTrees, int nodeSize, double oob)
{
	ui.randAttr->setText(QString::number(randAttr));
	ui.numTrees->setText(QString::number(numTrees));
	ui.nodeSize->setText(QString::number(nodeSize));
	ui.oob->setText(QString::number(oob));
}

void ClassifierSharkRandomForest::infoMessageBox(const QString& title, const QString& text, const QString& button)
{
	// GUI needs to be processed in main thread
	QMessageBox::information(qSlicerApplication::activeWindow(), title, text, button);
}

void ClassifierSharkRandomForest::startTraining()
{
	worker->setOptimization(ui.setAutomatically->isChecked());

	worker->setRandAttrLow(ui.randAttrLow->text().toInt());
	worker->setRandAttrHigh(ui.randAttrHigh->text().toInt());
	worker->setRandAttrSteps(ui.randAttrSteps->text().toInt());

	worker->setNumTreesLow(ui.numTreesLow->text().toInt());
	worker->setNumTreesHigh(ui.numTreesHigh->text().toInt());
	worker->setNumTreesSteps(ui.numTreesSteps->text().toInt());

	worker->setNodeSizeLow(ui.nodeSizeLow->text().toInt());
	worker->setNodeSizeHigh(ui.nodeSizeHigh->text().toInt());
	worker->setNodeSizeSteps(ui.nodeSizeSteps->text().toInt());

	worker->setOobLow(ui.oobLow->text().toDouble());
	worker->setOobHigh(ui.oobHigh->text().toDouble());
	worker->setOobSteps(ui.oobSteps->text().toDouble());

	worker->setRandAttr(ui.randAttr->text().toInt());
	worker->setNumTrees(ui.numTrees->text().toInt());
	worker->setNodeSize(ui.nodeSize->text().toInt());
	worker->setOob(ui.oob->text().toDouble());

	worker->startTraining();
}

ClassifierSharkRandomForestWorker::ClassifierSharkRandomForestWorker()
	: SupervisedClassifierWorker(),
	autoOptimize(false), decisionFunction(funct_type(&normalizer, &classifier))
{
	subthreadCount = qMax<int>(QThread::idealThreadCount() - 1, 4);
}

ClassifierSharkRandomForestWorker::~ClassifierSharkRandomForestWorker()
{
	delete classifyMutex;
	delete optimizeMutex;
}

funct_type ClassifierSharkRandomForestWorker::getDecisionFunction()
{
	return this->decisionFunction;
}

void ClassifierSharkRandomForestWorker::setOptimization(bool state)
{
	autoOptimize = state;
}
void ClassifierSharkRandomForestWorker::processTraining()
{
	using namespace shark;
	dataset = shark::createLabeledDataFromRange(samples, labels);


	qDebug() << "Samples: " << dataset.numberOfElements();


	qDebug() << "Normalizing...";
	NormalizeComponentsUnitVariance<RealVector> normalizingTrainer(true);
	normalizingTrainer.train(normalizer, dataset.inputs());

	// Transform data
	dataset.inputs() = transform(dataset.inputs(), normalizer);

	if (autoOptimize) {
		qDebug() << "Optimizing..";
		optimize();
		emit infoMessageBoxRequest("Optimization finished",
			QString("Maximum sensitivity: %1 \nMaximum specificity: %2 \nRandom attributes: %3 \nNumber of trees: %4 \nNode size: %5 \nOOB samples: %6 \nCSV file with all results is in <Slicer root>\\SharkRandomForest.csv")
			.arg(bestSens).arg(bestSpec).arg(randAttr).arg(numTrees).arg(nodeSize).arg(oob), "OK");
	}

	RFTrainer trainer;
	trainer.setMTry(randAttr);
	trainer.setNTrees(numTrees);
	trainer.setNodeSize(nodeSize);
	trainer.setOOBratio(oob);

	qDebug() << "Random attribute number: " << randAttr;
	qDebug() << "Number of trees: " << numTrees;
	qDebug() << "Node Size: " << nodeSize;
	qDebug() << "OOB ratio: " << oob;

	qDebug() << "Training";

	trainer.train(classifier, dataset);

	decisionFunction = (normalizer >> classifier);

	qDebug() << "Finished";

	//dataset = shark::createLabeledDataFromRange(allSamples, allLabels);
	//
	//
	//std::vector<unsigned int> classifiedResults;
	//
	//for (size_t j = 0; j < allSamples.size(); j++) {
	//	shark::RealVector a;
	//	decisionFunction.eval(allSamples.at(j), a);
	//
	//	int label = 0;
	//	double max = 0.0;
	//	for (size_t k = 0; k < a.size(); k++) {
	//		if (a(k) > max) {
	//			max = a(k);
	//			label = k;
	//		}
	//	}
	//	classifiedResults.push_back(label);
	//}
	//
	//QPair<double, double> out = sensSpec(classifiedResults, dataset.labels(), selectedLabels);
	//qDebug() << "Sensitivity" << out.first << "Specificity" << out.second;

	dataset = shark::ClassificationDataset();
	samples.clear();
	labels.clear();
}
void ClassifierSharkRandomForestWorker::processTrainingEntry(const SingleVolumeTrainingEntry& entry)
{
	QVector<vtkSmartPointer<vtkImageData>> nodesImageData;
	for (size_t i = 0; i < entry.volumes.count(); i++) {
		vtkSmartPointer<vtkMRMLVolumeNode> node = vtkMRMLVolumeNode::SafeDownCast(entry.volumes.at(i));
		nodesImageData.append(node->GetImageData());
	}
	vtkSmartPointer<vtkMRMLVolumeNode> truth = vtkMRMLVolumeNode::SafeDownCast(entry.truth);

	vtkImageData* truthImageData = truth->GetImageData();
	int dims[3];
	truthImageData->GetDimensions(dims);

	int topLimit, botLimit;
	if (crop) {
		topLimit = topSlice;
		botLimit = bottomSlice;
	}
	else {
		topLimit = dims[2];
		botLimit = 0;
	}

	for (size_t i = 0; i < dims[0]; i++) {
		for (size_t j = 0; j < dims[1]; j++) {
			for (size_t k = qMax(botLimit, 0); k < qMin(topLimit, dims[2]); k++) {
				unsigned int label = truthImageData->GetScalarComponentAsDouble(i, j, k, 0);

				sampleType oneVector(nodesImageData.count());

				for (size_t m = 0; m < nodesImageData.count(); m++) {
					oneVector(m) = nodesImageData.at(m)->GetScalarComponentAsDouble(i, j, k, 0);
				}

				// Save only labels that are selected, anything else is a negative sample
				bool found = false;
				for (size_t m = 0; m < selectedLabels.count(); m++) {
					if (label == selectedLabels.at(m))
						found = true;
				}
				if (!found)
					label = 0;

				// Get rid of duplicate values
				bool has = false;
				for (size_t m = 0; m < samples.size(); m++)
				{
					bool equal = true;
					if (label != labels.at(m))
						continue;

					for (size_t p = 0; p < samples.at(m).size(); p++) {
						if (samples.at(m)(p) != oneVector(p)) {
							equal = false;
							break;
						}
					}
					if (equal) {
						has = true;
						break;
					}
				}

				if (!has) {
					samples.push_back(oneVector);
					labels.push_back(label);
				}
			}
		}
	}
	//for (size_t i = 0; i < dims[0]; i++) {
	//	for (size_t j = 0; j < dims[1]; j++) {
	//		for (size_t k = 85; k < 106; k++) {
	//			double label = truthImageData->GetScalarComponentAsDouble(i, j, k, 0);
	//
	//			sampleType oneVector(nodesImageData.count());
	//
	//			for (size_t m = 0; m < nodesImageData.count(); m++) {
	//				oneVector(m) = nodesImageData.at(m)->GetScalarComponentAsDouble(i, j, k, 0);
	//			}
	//			
	//			bool found = false;
	//			for (size_t m = 0; m < selectedLabels.count(); m++) {
	//				if (label == selectedLabels.at(m))
	//					found = true;
	//			}
	//			if (!found)
	//				label = 0;
	//			// Get rid of duplicate values
	//			bool has = false;
	//			for (size_t m = 0; m < allSamples.size(); m++)
	//			{
	//				bool equal = true;
	//				if (label != allLabels.at(m))
	//					continue;
	//
	//				for (size_t p = 0; p < allSamples.at(m).size(); p++) {
	//					if (allSamples.at(m)(p) != oneVector(p)) {
	//						equal = false;
	//						break;
	//					}
	//				}
	//				if (equal) {
	//					has = true;
	//					break;
	//				}
	//			}
	//
	//			if (!has) {
	//				allSamples.push_back(oneVector);
	//				allLabels.push_back(label);
	//			}
	//		}
	//	}
	//}
}
void ClassifierSharkRandomForestWorker::processClassifyingEntry(const SingleVolumeClassifyingEntry& entry)
{
	int dims[3];
	vtkMRMLVolumeNode::SafeDownCast(entry.volumes.at(0))->GetImageData()->GetDimensions(dims);

	if (classifyCrop) {
		for (size_t i = qMax(0, classifyBottomSlice); i <= qMin(dims[2], classifyTopSlice); i++) {
			slicesToClassify.append(i);
		}
	}
	else {
		for (size_t i = 0; i < dims[2]; i++) {
			slicesToClassify.append(i);
		}
	}

	std::vector<std::thread> threads;
	for (size_t i = 0; i < subthreadCount; i++)
	{
		threads.emplace_back(&ClassifierSharkRandomForestWorker::classifyThreaded, this, std::cref(entry));
	}
	currentlyClassified = entry.result;
	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	qDebug() << "Threads joined";

	emit volumeClassified(currentlyClassified);
}

void ClassifierSharkRandomForestWorker::optimize()
{
	bestSpec = 0.0;
	bestSens = 0.0;
	int folds = 5;

	// Shark's training is somewhat optimized for multithreading
	const int idealThreadCount = qMax<int>(ceil(subthreadCount / 2.), 1);

	// Generate test data

	datasetSubsets.clear();

	for (size_t i = 0; i < folds; i++) {
		datasetSubsets.push_back(shark::ClassificationDataset());
	}

	for (size_t i = 0; i < samples.size(); i++) {
		// Ugly workaround
		std::vector<sampleType> a;
		a.push_back(samples.at(i));
		std::vector<unsigned int> b;
		b.push_back(labels.at(i));
		datasetSubsets[i % folds].append(shark::createLabeledDataFromRange(a, b));
	}

	// Generate queue

	int numFeatures = samples.at(0).size();
	for (size_t iRandAttr = 0; iRandAttr < randAttrSteps; iRandAttr++)
	{
		for (size_t iNumTrees = 0; iNumTrees < numTreesSteps; iNumTrees++)
		{			
			for (size_t iNodeSize = 0; iNodeSize < nodeSizeSteps; iNodeSize++)
			{
				for (size_t iOob = 0; iOob < oobSteps; iOob++)
				{
					double ra = randAttrLow + ((randAttrHigh - randAttrLow) / (double)randAttrSteps) * iRandAttr;
					ra = qRound(ra);
					if (ra > numFeatures)
						continue;
					OptimizationParameterSet set;
					set.nodeSize = nodeSizeLow + ((nodeSizeHigh - nodeSizeLow)/ (double)nodeSizeSteps) * iNodeSize;
					set.numTrees = numTreesLow + ((numTreesHigh - numTreesLow)/ (double)numTreesSteps) * iNumTrees;
					set.oob = oobLow + ((oobHigh - oobLow) / oobSteps) * iOob;
					set.randAttr = ra;
					optimizationQueue.append(set);
				}
			}
		}
	}

	// Create dumpfile

	QFile file("SharkRandomForest.csv");
	if (!file.exists()) {
		file.open(QIODevice::WriteOnly);
		file.write("Node size, Number of trees, Oob, Random Attributes");
		file.write("\r\n=====" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm").toUtf8());
	}
	else {
		file.open(QIODevice::ReadWrite | QIODevice::Append);
		file.write("\r\n=====" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm").toUtf8());
	}
	file.close();

	// Create threads

	optimizeMutex->lock();
	std::vector<std::thread> threads;
	for (size_t i = 0; i < idealThreadCount; i++)
	{
		threads.emplace_back(&ClassifierSharkRandomForestWorker::optimizeThreaded, this);
	}

	// Start threads
	optimizeMutex->unlock();
	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	qDebug() << "Threads joined";
}

void ClassifierSharkRandomForestWorker::optimizeThreaded()
{
	using namespace shark;
	optimizeMutex->lock();
	while (!optimizationQueue.isEmpty()) {
		OptimizationParameterSet set = optimizationQueue.first();
		optimizationQueue.remove(0);

		optimizeMutex->unlock();

		double actSens = 0.0;
		double actSpec = 0.0;

		for (size_t currentTrainingSet = 0; currentTrainingSet < datasetSubsets.size(); currentTrainingSet++) {

			RFTrainer optimizeTrainer;
			RFClassifier optimizeClassifier;

			optimizeTrainer.setMTry(set.randAttr);
			optimizeTrainer.setNTrees(set.numTrees);
			optimizeTrainer.setNodeSize(set.nodeSize);
			optimizeTrainer.setOOBratio(set.oob);

			const ClassificationDataset& currentSubset = datasetSubsets.at(currentTrainingSet);

			optimizeTrainer.train(optimizeClassifier, currentSubset);

			//funct_type optimizeDecisionFunction = (normalizer >> optimizeClassifier);

			for (size_t i = 0; i < datasetSubsets.size(); i++) {
				if (i == currentTrainingSet)
					continue;

				const ClassificationDataset& testedSubset = datasetSubsets.at(i);

				std::vector<unsigned int> classified;
				for (size_t j = 0; j < testedSubset.numberOfElements(); j++) {
					sampleType singleSample;
					optimizeClassifier.eval(testedSubset.element(j).input, singleSample);
					unsigned int label = 0;
					double max = 0.0;
					for (size_t k = 0; k < singleSample.size(); k++) {
						if (singleSample(k) > max) {
							max = singleSample(k);
							label = k;
						}
					}
					classified.push_back(label);

				}
				QPair<double, double> result = sensSpec(classified, testedSubset.labels(), selectedLabels);
				actSens += (double(1) / double(1 + i))*double(result.first - actSens);
				actSpec += (double(1) / double(1 + i))*double(result.second - actSpec);
			}
		}

		optimizeMutex->lock();
		qDebug() << optimizationQueue.count() 
			<< "Rand Attr: " << QString::number(set.randAttr, 'f', 0)
			<< "Num Trees: " << QString::number(set.numTrees, 'f', 0)
			<< "Node Size: " << QString::number(set.nodeSize, 'f', 0)
			<< "Oob: " << QString::number(set.oob, 'f', 6)
			<< "Sensitivity:" << QString::number(actSens, 'f', 6)
			<< "Specificity:" << QString::number(actSpec, 'f', 6);

		QFile file("SharkRandomForest.csv");
		file.open(QIODevice::ReadWrite | QIODevice::Append);
		file.write("\r\n"
			+ QByteArray::number(set.nodeSize) + ","
			+ QByteArray::number(set.numTrees) + ","
			+ QByteArray::number(set.oob) + ","
			+ QByteArray::number(set.randAttr) + ","
			+ QByteArray::number(actSens) + ","
			+ QByteArray::number(actSpec));
		file.close();
		if ((bestSpec <= actSpec && bestSens <= actSens)
			|| ((bestSpec <= actSpec) && ((actSpec - bestSpec) >= (bestSens - actSens)))
			|| ((bestSens <= actSens) && ((actSens - bestSens) >= (bestSpec - actSpec)))) {
			nodeSize = set.nodeSize;
			numTrees = set.numTrees;
			randAttr = set.randAttr;
			oob = set.oob;
			bestSpec = actSpec;
			bestSens = actSens;
			emit valuesOptimized(randAttr, numTrees, nodeSize, oob);
		}
	}
	optimizeMutex->unlock();
	qDebug() << "Thread finished";
}

QPair<double, double> ClassifierSharkRandomForestNamespace::sensSpec(const std::vector<unsigned int>& classified, const shark::Data<unsigned int>& truth, const QVector<unsigned int>& labels)
{
	QPair<double, double> out(0.0, 0.0);
	for (size_t iter = 0; iter < labels.count(); iter++) {
		int TP = 0;
		int TN = 0;
		int FP = 0;
		int FN = 0;

		int labelAgainst = labels.at(iter);

		for (size_t i = 0; i < classified.size(); i++) {
			if (truth.element(i) == classified.at(i)) {
				if (classified.at(i) == labelAgainst)
					TP++;
				else
					TN++;
			}
			else {
				if (classified.at(i) == labelAgainst)
					FP++;
				else
					FN++;
			}
		}

		double sensitivity = double(TP) / double(TP + FN);
		double specificity = double(TN) / double(TN + FP);

		out.first += (double(1) / double(1 + iter))*double(sensitivity - out.first);
		out.second += (double(1) / double(1 + iter))*double(specificity - out.second);
	}
	return out;
}

void ClassifierSharkRandomForestWorker::setCropping(bool crop)
{
	this->crop = crop;
}
void ClassifierSharkRandomForestWorker::setBottomSlice(int bottom)
{
	bottomSlice = bottom;
}
void ClassifierSharkRandomForestWorker::setTopSlice(int top)
{
	topSlice = top;
}
void ClassifierSharkRandomForestWorker::setRandAttr(int randAttr)
{ 
	if (randAttr < 0)
		randAttr = 0;
	this->randAttr = randAttr;
}
void ClassifierSharkRandomForestWorker::setNumTrees(int numTrees)
{
	if (numTrees < 1)
		numTrees = 1;
	this->numTrees = numTrees;
}
void ClassifierSharkRandomForestWorker::setNodeSize(int nodeSize)
{
	if (nodeSize < 1)
		nodeSize = 1;
	this->nodeSize = nodeSize;
}
void ClassifierSharkRandomForestWorker::setOob(double oob)
{
	if (oob < 0.)
		oob = 0.;
	this->oob = oob;
}

void ClassifierSharkRandomForestWorker::setRandAttrLow(int randAttrLow)
{
	if (randAttrLow < 0)
		randAttrLow = 0;
	this->randAttrLow = randAttrLow;
}
void ClassifierSharkRandomForestWorker::setNumTreesLow(int numTreesLow)
{
	if (numTreesLow < 1)
		numTreesLow = 1;
	this->numTreesLow = numTreesLow;
}
void ClassifierSharkRandomForestWorker::setNodeSizeLow(int nodeSizeLow)
{ 
	if (nodeSizeLow < 1)
		nodeSizeLow = 1;
	this->nodeSizeLow = nodeSizeLow;
}
void ClassifierSharkRandomForestWorker::setOobLow(double oobLow)
{ 
	if (oobLow < 0.)
		oobLow = 0.;
	this->oobLow = oobLow;
}
void ClassifierSharkRandomForestWorker::setRandAttrHigh(int randAttrHigh)
{
	if (randAttrHigh < 0)
		randAttrHigh = 0;
	this->randAttrHigh = randAttrHigh;
}
void ClassifierSharkRandomForestWorker::setNumTreesHigh(int numTreesHigh)
{
	if (numTreesHigh < 1)
		numTreesHigh = 1;
	this->numTreesHigh = numTreesHigh;
}
void ClassifierSharkRandomForestWorker::setNodeSizeHigh(int nodeSizeHigh)
{
	if (nodeSizeHigh < 1)
		nodeSizeHigh = 1;
	this->nodeSizeHigh = nodeSizeHigh;
}
void ClassifierSharkRandomForestWorker::setOobHigh(double oobHigh)
{
	if (oobHigh < 0.)
		oobHigh = 0.;
	this->oobHigh = oobHigh;
}
void ClassifierSharkRandomForestWorker::setRandAttrSteps(int randAttrSteps)
{
	if (randAttrSteps < 1)
		randAttrSteps = 1;
	this->randAttrSteps = randAttrSteps;
}
void ClassifierSharkRandomForestWorker::setNumTreesSteps(int numTreesSteps)
{ 
	if (numTreesSteps < 1)
		numTreesSteps = 1;
	this->numTreesSteps = numTreesSteps;
}
void ClassifierSharkRandomForestWorker::setNodeSizeSteps(int nodeSizeSteps)
{
	if (nodeSizeSteps < 1)
		nodeSizeSteps = 1;
	this->nodeSizeSteps = nodeSizeSteps;
}
void ClassifierSharkRandomForestWorker::setOobSteps(int oobSteps)
{
	if (oobSteps < 1)
		oobSteps = 1;
	this->oobSteps = oobSteps;
}
void ClassifierSharkRandomForestWorker::setClassifyCropping(bool crop)
{
	this->classifyCrop = crop;
}
void ClassifierSharkRandomForestWorker::setClassifyBottomSlice(int bottom)
{
	classifyBottomSlice = bottom;
}
void ClassifierSharkRandomForestWorker::setClassifyTopSlice(int top)
{
	classifyTopSlice = top;
}

void ClassifierSharkRandomForestWorker::selectLabels(const QVector<unsigned int>& labels)
{
	selectedLabels = labels;
}

void ClassifierSharkRandomForestWorker::classifyThreaded(const SingleVolumeClassifyingEntry& entry)
{
	QVector<vtkSmartPointer<vtkImageData>> nodesImageData;
	for (size_t i = 0; i < entry.volumes.count(); i++) {
		vtkSmartPointer<vtkMRMLVolumeNode> node = vtkMRMLVolumeNode::SafeDownCast(entry.volumes.at(i));
		nodesImageData.append(node->GetImageData());
	}

	QMutexLocker lock(classifyMutex);
	while (!slicesToClassify.isEmpty()) {

		size_t k = slicesToClassify.first();
		slicesToClassify.remove(0);
		lock.unlock();

		vtkSmartPointer<vtkMRMLVolumeNode> resultNode = vtkMRMLVolumeNode::SafeDownCast(entry.result);

		vtkSmartPointer<vtkImageData> resultImageData = resultNode->GetImageData();
		int dims[3];

		nodesImageData.front()->GetDimensions(dims);
		for (size_t i = 0; i < dims[0]; i++) {
			for (size_t j = 0; j < dims[1]; j++) {
				sampleType oneVector(nodesImageData.count());
				for (size_t m = 0; m < nodesImageData.count(); m++) {
					oneVector(m) = nodesImageData.at(m)->GetScalarComponentAsDouble(i, j, k, 0);
				}
				shark::RealVector a;
				decisionFunction.eval(oneVector, a);

				int label = 0;
				double max = 0.0;
				for (size_t k = 0; k < a.size(); k++) {
					if (a(k) > max) {
						max = a(k);
						label = k;
					}
				}

				resultImageData->SetScalarComponentFromDouble(i, j, k, 0, label);
			}
		}
		lock.relock();
	}
	lock.unlock();	
}