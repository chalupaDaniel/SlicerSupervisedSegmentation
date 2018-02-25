#include "ClassifierDlibSvmC.h"

#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QMutex>
#include <QWaitCondition>
#include <QMessageBox>
#include <QLineEdit>
#include <QDateTime>

#include "vtkMRMLNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkImageData.h"

#include "qSlicerApplication.h"

#include "dlib/serialize.h"

#include <iostream>
#include <thread>

#include <QDebug>
#include <QFile>

using namespace ClassifierDlibSvmCNamespace;

ClassifierDlibSvmC::ClassifierDlibSvmC()
	: SupervisedClassifier()
{
	classifierName = "C Support Vector Machine - Dlib";

	ui.setupUi(trainWidget);
	classifyUi.setupUi(classifyWidget);

	ui.selectSlices->setChecked(true);

	ui.bottomSliceLimit->setVisible(true);
	ui.bottomSliceLimitLabel->setVisible(true);
	ui.topSliceLimit->setVisible(true);
	ui.topSliceLimitLabel->setVisible(true);

	ui.cLowLimit->setVisible(false);
	ui.cLowLimitLabel->setVisible(false);
	ui.cHighLimit->setVisible(false);
	ui.cHighLimitLabel->setVisible(false);
	ui.steps->setVisible(false);
	ui.stepsLabel->setVisible(false);
	ui.gammaLowLimit->setVisible(false);
	ui.gammaLowLimitLabel->setVisible(false);
	ui.gammaHighLimit->setVisible(false);
	ui.gammaHighLimitLabel->setVisible(false);
	ui.gammaSteps->setVisible(false);
	ui.gammaStepsLabel->setVisible(false);

	ui.gammaLabel->setVisible(true);
	ui.gamma->setVisible(true);
	ui.cLabel->setVisible(true);
	ui.c->setVisible(true);


	worker = new ClassifierDlibSvmCWorker();
	worker->start();

	connect(worker, SIGNAL(volumeClassified(vtkSmartPointer<vtkMRMLNode>)), this, SIGNAL(finishedClassifying(vtkSmartPointer<vtkMRMLNode>)));
	connect(worker, SIGNAL(classifierTrained()), this, SIGNAL(finishedTraining()));
	connect(worker, SIGNAL(valuesOptimized(double, double, double, double)), this, SLOT(valuesOptimized(double, double, double, double)));
	connect(ui.selectSlices, SIGNAL(toggled(bool)), this, SLOT(selectSlicesToggled(bool)));
	connect(classifyUi.selectSlices, SIGNAL(toggled(bool)), this, SLOT(selectClassifySlicesToggled(bool)));
	connect(ui.setAutomatically, SIGNAL(toggled(bool)), this, SLOT(setAutomaticallyToggled(bool)));
	connect(classifyUi.thresholdValue, SIGNAL(textChanged(QString)), this, SLOT(thresholdValueChanged()));
	connect(classifyUi.thresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(thresholdSliderChanged()));
	connect(worker, SIGNAL(infoMessageBoxRequest(QString, QString, QString)), this, SLOT(infoMessageBox(QString, QString, QString)), Qt::QueuedConnection);
}
ClassifierDlibSvmC::~ClassifierDlibSvmC()
{
	delete worker;
}
void ClassifierDlibSvmC::thresholdValueChanged()
{
	classifyUi.thresholdSlider->setValue(classifyUi.thresholdValue->text().toDouble() * 100);
}
void ClassifierDlibSvmC::thresholdSliderChanged()
{
	classifyUi.thresholdValue->setText(QString::number(classifyUi.thresholdSlider->value() / 100.0));
}

QByteArray ClassifierDlibSvmC::serialize()
{
	// dlib's serialization probably doesn't support saving to a variable
	// TODO:
	dlib::serialize("tempSave.dat") << worker->decisionFunction;
	QByteArray out;

	QFile file("tempSave.dat");
	file.open(QIODevice::ReadOnly);
	out = file.readAll();
	file.close();
	file.remove();

	return out;
}

bool ClassifierDlibSvmC::deserialize(const QByteArray & serializedClassifier)
{
	// TODO:
	QFile file("tempLoad.dat");
	file.open(QIODevice::WriteOnly);
	file.write(serializedClassifier);
	file.close();
	dlib::deserialize("tempLoad.dat") >> worker->decisionFunction;
	file.remove();

	if (worker->decisionFunction.function.basis_vectors.nr() == 0) {
		qDebug("ClassifierDlibSvmNu::The classifier could not be loaded");
		return false;
	}

	return true;
}

void ClassifierDlibSvmC::addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth)
{
	worker->setCropping(ui.selectSlices->isChecked());
	worker->setBottomSlice(ui.bottomSliceLimit->text().toInt());
	worker->setTopSlice(ui.topSliceLimit->text().toInt());
	worker->setLabel(ui.selectedLabel->value());

	worker->appendToTrainingQueue(volumes, truth);
}

void ClassifierDlibSvmC::addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result)
{
	worker->setClassifyCropping(classifyUi.selectSlices->isChecked());
	worker->setClassifyBottomSlice(classifyUi.bottomSliceLimit->text().toInt());
	worker->setClassifyTopSlice(classifyUi.topSliceLimit->text().toInt());
	worker->setThreshold(classifyUi.thresholdSlider->value() / 100.0);

	worker->appendToClassifyingQueue(volumes, result);
}

void ClassifierDlibSvmC::selectSlicesToggled(bool state)
{
	ui.bottomSliceLimit->setVisible(state);
	ui.bottomSliceLimitLabel->setVisible(state);
	ui.topSliceLimit->setVisible(state);
	ui.topSliceLimitLabel->setVisible(state);
}

void ClassifierDlibSvmC::selectClassifySlicesToggled(bool state)
{
	classifyUi.bottomSliceLimit->setVisible(state);
	classifyUi.bottomSliceLimitLabel->setVisible(state);
	classifyUi.topSliceLimit->setVisible(state);
	classifyUi.topSliceLimitLabel->setVisible(state);
}
void ClassifierDlibSvmC::setAutomaticallyToggled(bool state)
{
	ui.cLowLimit->setVisible(state);
	ui.cLowLimitLabel->setVisible(state);
	ui.cHighLimit->setVisible(state);
	ui.cHighLimitLabel->setVisible(state);
	ui.steps->setVisible(state);
	ui.stepsLabel->setVisible(state);
	ui.gammaLowLimit->setVisible(state);
	ui.gammaLowLimitLabel->setVisible(state);
	ui.gammaHighLimit->setVisible(state);
	ui.gammaHighLimitLabel->setVisible(state);
	ui.gammaSteps->setVisible(state);
	ui.gammaStepsLabel->setVisible(state);

	ui.gammaLabel->setVisible(!state);
	ui.gamma->setVisible(!state);
	ui.cLabel->setVisible(!state);
	ui.c->setVisible(!state);
}

void ClassifierDlibSvmC::valuesOptimized(double gamma, double c, double specificity, double sensitivity)
{
	ui.gamma->setText(QString::number(gamma));
	ui.c->setText(QString::number(c));
}

void ClassifierDlibSvmC::infoMessageBox(const QString& title, const QString& text, const QString& button)
{
	// GUI needs to be processed in main thread
	QMessageBox::information(qSlicerApplication::activeWindow(), title, text, button);
}

void ClassifierDlibSvmC::startTraining()
{
	worker->setOptimization(ui.setAutomatically->isChecked());

	worker->setCHigh(ui.cHighLimit->text().toDouble());
	worker->setCLow(ui.cLowLimit->text().toDouble());
	worker->setCSteps(ui.steps->text().toInt());
	worker->setGammaHigh(ui.gammaHighLimit->text().toDouble());
	worker->setGammaLow(ui.gammaLowLimit->text().toDouble());
	worker->setGammaSteps(ui.gammaSteps->text().toInt());

	worker->setGamma(ui.gamma->text().toDouble());
	worker->setC(ui.c->text().toDouble());
	worker->startTraining();
}

ClassifierDlibSvmCWorker::ClassifierDlibSvmCWorker()
	: SupervisedClassifierWorker(),
	autoOptimize(false)
{
	classifyMutex = new QMutex;
	optimizeMutex = new QMutex;
	subthreadCount = qMax<int>(QThread::idealThreadCount() - 1, 4);
}

ClassifierDlibSvmCWorker::~ClassifierDlibSvmCWorker()
{
	delete classifyMutex;
	delete optimizeMutex;
}

void ClassifierDlibSvmCWorker::setOptimization(bool state)
{
	autoOptimize = state;
}

void ClassifierDlibSvmCWorker::processTraining()
{
	dlib::vector_normalizer<sample_type> normalizer;
	// let the normalizer learn the mean and standard deviation of the samples
	normalizer.train(samples);
	// now normalize each sample
	for (unsigned long i = 0; i < samples.size(); ++i)
		samples[i] = normalizer(samples[i]);

	qDebug() << "Samples: " << samples.size();

	if (autoOptimize) {
		optimize();
		emit infoMessageBoxRequest("Optimization finished",
			QString("Maximum sensitivity: %1"
				"\nMaximum specificity: %2"
				"\nGamma: %3"
				"\nC: %4 "
				"\nCSV file with all results is in <Slicer root>\\DlibSvmC.csv")
			.arg(bestSens).arg(bestSpec).arg(gamma).arg(c), "OK");
	}


	dlib::svm_c_trainer<kernel_type> trainer;

	qDebug() << "Setting gamma";
	trainer.set_kernel(kernel_type(gamma));
	qDebug() << "Setting c";
	trainer.set_c(c);

	qDebug() << "Gamma: " << gamma;
	qDebug() << "C: " << c;

	qDebug() << "Setting normalizer";
	decisionFunction.normalizer = normalizer;  // save normalization information
	qDebug() << "Training";
	decisionFunction.function = trainer.train(samples, labels); // perform the actual SVM training and save the results

	qDebug() << "Finished";

	//std::vector<int> classifiedResults;
	//
	//for (size_t j = 0; j < allSamples.size(); j++) {
	//	double a = decisionFunction(allSamples.at(j));
	//	int classified;
	//	if (a > 0.3)
	//		classified = 1.0;
	//	else
	//		classified = -1.0;
	//	classifiedResults.push_back(classified);
	//}
	//
	//QPair<double, double> out = sensSpec(classifiedResults, allLabels);
	//qDebug() << "Sensitivity" << out.first << "Specificity" << out.second;

	samples.clear();
	labels.clear();
	//allSamples.clear();
	//allLabels.clear();
}
void ClassifierDlibSvmCWorker::processTrainingEntry(const SingleVolumeTrainingEntry& entry)
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
				double label = truthImageData->GetScalarComponentAsDouble(i, j, k, 0);

				sample_type oneVector(nodesImageData.count());
				oneVector.set_size(nodesImageData.count());

				for (size_t m = 0; m < nodesImageData.count(); m++) {
					oneVector(m) = nodesImageData.at(m)->GetScalarComponentAsDouble(i, j, k, 0);
				}

				// Save only label that is selected, anything else is a negative sample
				if (label == selectedLabel) {
					label = 1;
				}
				else {
					label = -1;
				}

				// Get rid of duplicate values
				bool has = false;
				for (size_t m = 0; m < samples.size(); m++)
				{
					if (label != labels.at(m))
						continue;
					if (samples.at(m) == oneVector) {
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
	//			sample_type oneVector(nodesImageData.count());
	//			oneVector.set_size(nodesImageData.count());
	//
	//			for (size_t m = 0; m < nodesImageData.count(); m++) {
	//				oneVector(m) = nodesImageData.at(m)->GetScalarComponentAsDouble(i, j, k, 0);
	//			}
	//			if (label == selectedLabel) {
	//				label = 1;
	//			}
	//			else {
	//				label = -1;
	//			}
	//
	//			bool has = false;
	//			for (size_t m = 0; m < allSamples.size(); m++)
	//			{
	//				if (label != allLabels.at(m))
	//					continue;
	//				if (allSamples.at(m) == oneVector) {
	//					has = true;
	//					break;
	//				}
	//			}
	//			if (!has) {
	//				allSamples.push_back(oneVector);
	//				allLabels.push_back(qRound(label) == 1 ? 1 : -1);
	//			}
	//		}
	//	}
	//}
}
void ClassifierDlibSvmCWorker::processClassifyingEntry(const SingleVolumeClassifyingEntry& entry)
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
	currentlyClassified = entry.result;
	for (size_t i = 0; i < subthreadCount; i++)
	{
		threads.emplace_back(&ClassifierDlibSvmCWorker::classifyThreaded, this, std::cref(entry));
	}
	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	qDebug() << "Threads joined";

	emit volumeClassified(currentlyClassified);
}
//QPair<double, double> ClassifierDlibSvmCNamespace::sensSpec(const std::vector<int>& classified, const std::vector<int>& truth)
//{
//	QPair<double, double> out(0.0, 0.0);
//	int TP = 0;
//	int TN = 0;
//	int FP = 0;
//	int FN = 0;
//
//	for (size_t i = 0; i < classified.size(); i++) {
//		if (truth.at(i) == classified.at(i)) {
//			if (classified.at(i) == 1)
//				TP++;
//			else
//				TN++;
//		}
//		else {
//			if (classified.at(i) == -1)
//				FP++;
//			else
//				FN++;
//		}
//	}
//
//	double sensitivity = double(TP) / double(TP + FN);
//	double specificity = double(TN) / double(TN + FP);
//
//	out.first += sensitivity;
//	out.second += specificity;
//	return out;
//}

void ClassifierDlibSvmCWorker::optimize()
{
	bestSpec = 0.0;
	bestSens = 0.0;
	gamma = 1.0;
	c = 1.0;

	const int idealThreadCount = qMax<int>(QThread::idealThreadCount() - 1, 4);

	for (size_t iGamma = 0; iGamma <= gammaSteps; iGamma++) {
		for (size_t iC = 0; iC <= cSteps; iC++) {
			gammas.append(pow(10, ((double)iGamma / gammaSteps) * (gammaHigh - gammaLow) + gammaLow));
			cs.append(pow(10, ((double)iC / cSteps) * (cHigh - cLow) + cLow));
		}
	}

	QFile file("DlibSvmC.csv");
	if (!file.exists()) {
		file.open(QIODevice::WriteOnly);
		file.write("Gamma,C,Sensitivity,Specificity");
		file.write("\r\n=====" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm").toUtf8());
	}
	else {
		file.open(QIODevice::ReadWrite | QIODevice::Append);
		file.write("\r\n=====" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm").toUtf8());
	}
	file.close();

	std::vector<std::thread> threads;
	for (size_t i = 0; i < idealThreadCount; i++)
	{
		threads.emplace_back(&ClassifierDlibSvmCWorker::optimizeThreaded, this);
	}
	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	qDebug() << "Threads joined";
}

void ClassifierDlibSvmCWorker::optimizeThreaded()
{
	optimizeMutex->lock();
	while (!gammas.isEmpty()) {
		double testGamma = gammas.at(0);
		gammas.remove(0);
		double testC = cs.at(0);
		cs.remove(0);
		optimizeMutex->unlock();
		dlib::svm_c_trainer<kernel_type> trainer;
		trainer.set_kernel(kernel_type(testGamma));
		trainer.set_c(testC);

		dlib::matrix<double, 1L, 2L> mat = dlib::cross_validate_trainer(trainer, samples, labels, 5);
		double actSens = mat(0);
		double actSpec = mat(1);
		optimizeMutex->lock();
		qDebug() << gammas.count() << "Gamma: " << QString::number(testGamma, 'f', 6) << "C: " << QString::number(testC, 'f', 6) << "Sensitivity:" << QString::number(mat(0), 'f', 6) << "Specificity:" << QString::number(mat(1), 'f', 6);

		QFile file("DlibSvmC.csv");
		file.open(QIODevice::ReadWrite | QIODevice::Append);
		file.write("\r\n"
			+ QByteArray::number(testGamma) + ","
			+ QByteArray::number(testC) + ","
			+ QByteArray::number(actSens) + ","
			+ QByteArray::number(actSpec));
		file.close();
		if ((bestSpec <= actSpec && bestSens <= actSens)
			|| ((bestSpec <= actSpec) && ((actSpec - bestSpec) >= (bestSens - actSens)))
			|| ((bestSens <= actSens) && ((actSens - bestSens) >= (bestSpec - actSpec)))) {
			gamma = testGamma;
			c = testC;
			bestSpec = actSpec;
			bestSens = actSens;
			emit valuesOptimized(gamma, c, actSpec, actSens);
		}
	}
	optimizeMutex->unlock();
	qDebug() << "Thread finished";
}

void ClassifierDlibSvmCWorker::setGamma(double gamma)
{
	this->gamma = gamma;
}
void ClassifierDlibSvmCWorker::setC(double c)
{
	this->c = c;
}
void ClassifierDlibSvmCWorker::setLabel(int label)
{
	selectedLabel = label;
}
void ClassifierDlibSvmCWorker::setCropping(bool crop)
{
	this->crop = crop;
}
void ClassifierDlibSvmCWorker::setBottomSlice(int bottom)
{
	bottomSlice = bottom;
}
void ClassifierDlibSvmCWorker::setTopSlice(int top)
{
	topSlice = top;
}
void ClassifierDlibSvmCWorker::setCLow(double cLow)
{
	this->cLow = cLow;
}
void ClassifierDlibSvmCWorker::setCHigh(double cHigh)
{
	this->cHigh = cHigh;
}
void ClassifierDlibSvmCWorker::setCSteps(int cSteps)
{
	this->cSteps = cSteps;
}
void ClassifierDlibSvmCWorker::setGammaLow(double gammaLow)
{
	this->gammaLow = gammaLow;
}
void ClassifierDlibSvmCWorker::setGammaHigh(double gammaHigh)
{
	this->gammaHigh = gammaHigh;
}
void ClassifierDlibSvmCWorker::setGammaSteps(int gammaSteps)
{
	this->gammaSteps = gammaSteps;
}
void ClassifierDlibSvmCWorker::setClassifyCropping(bool crop)
{
	this->classifyCrop = crop;
}
void ClassifierDlibSvmCWorker::setClassifyBottomSlice(int bottom)
{
	classifyBottomSlice = bottom;
}
void ClassifierDlibSvmCWorker::setClassifyTopSlice(int top)
{
	classifyTopSlice = top;
}
void ClassifierDlibSvmCWorker::setThreshold(double threshold)
{
	this->threshold = threshold;
}

void ClassifierDlibSvmCWorker::classifyThreaded(const SingleVolumeClassifyingEntry& entry)
{
	QVector<vtkSmartPointer<vtkImageData>> nodesImageData;
	for (size_t i = 0; i < entry.volumes.count(); i++) {
		vtkSmartPointer<vtkMRMLVolumeNode> node = vtkMRMLVolumeNode::SafeDownCast(entry.volumes.at(i));
		nodesImageData.append(node->GetImageData());
	}

	QMutexLocker lock(classifyMutex);
	// For some reason, dlib's decision functions' classification is not thread-safe 
	funct_type decFun = decisionFunction;

	while (!slicesToClassify.isEmpty()) {

		size_t k = slicesToClassify.first();
		slicesToClassify.remove(0);

		lock.unlock();

		vtkSmartPointer<vtkMRMLVolumeNode> resultNode = vtkMRMLVolumeNode::SafeDownCast(entry.result);

		vtkSmartPointer<vtkImageData> resultImageData = resultNode->GetImageData();
		int dims[3];

		nodesImageData.front()->GetDimensions(dims);
		qDebug() << k;
		for (size_t i = 0; i < dims[0]; i++) {
			for (size_t j = 0; j < dims[1]; j++) {
				sample_type oneVector(nodesImageData.count());
				for (size_t m = 0; m < nodesImageData.count(); m++) {
					oneVector(m) = nodesImageData.at(m)->GetScalarComponentAsDouble(i, j, k, 0);
				}
				double a = decFun(oneVector);

				if (a >= threshold)
					a = 1.0;
				else
					a = 0.0;

				resultImageData->SetScalarComponentFromDouble(i, j, k, 0, a);
			}
		}
		lock.relock();
	}
	lock.unlock();
}