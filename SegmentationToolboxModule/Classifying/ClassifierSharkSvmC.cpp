#include "ClassifierSharkSvmC.h"

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

#include <fstream>
#include <thread>

#include <QDebug>
#include <QFile>

#include <shark/ObjectiveFunctions/CrossValidationError.h>
#include <shark/Algorithms/DirectSearch/GridSearch.h>
#include <shark/Algorithms/JaakkolaHeuristic.h>   
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h>

using namespace ClassifierSharkSvmCNamespace;

ClassifierSharkSvmC::ClassifierSharkSvmC()
	: SupervisedClassifier()
{
	classifierName = "C Support Vector Machine - Shark";

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

	ui.gammaLabel->setVisible(true);
	ui.gamma->setVisible(true);
	ui.cLabel->setVisible(true);
	ui.c->setVisible(true);

	worker = new ClassifierSharkSvmCWorker();
	worker->start();

	connect(worker, SIGNAL(volumeClassified(vtkSmartPointer<vtkMRMLNode>)), this, SIGNAL(finishedClassifying(vtkSmartPointer<vtkMRMLNode>)));
	connect(worker, SIGNAL(classifierTrained()), this, SIGNAL(finishedTraining()));
	connect(worker, SIGNAL(valuesOptimized(double, double)), this, SLOT(valuesOptimized(double, double)));
	connect(ui.selectSlices, SIGNAL(toggled(bool)), this, SLOT(selectSlicesToggled(bool)));
	connect(classifyUi.selectSlices, SIGNAL(toggled(bool)), this, SLOT(selectClassifySlicesToggled(bool)));
	connect(ui.setAutomatically, SIGNAL(toggled(bool)), this, SLOT(setAutomaticallyToggled(bool)));
	connect(worker, SIGNAL(infoMessageBoxRequest(QString, QString, QString)), this, SLOT(infoMessageBox(QString, QString, QString)), Qt::QueuedConnection);
}
ClassifierSharkSvmC::~ClassifierSharkSvmC()
{
	worker->deleteLater();
}

QByteArray ClassifierSharkSvmC::serialize()
{
	ofstream ofs("tempSave.dat");
	boost::archive::polymorphic_text_oarchive oa(ofs);
	worker->decisionFunction.save(oa,1);
	ofs.close();

	QByteArray out = "gamma=" + QByteArray::number(worker->kernel.gamma()) + "\n";

	QFile file("tempSave.dat");
	file.open(QIODevice::ReadOnly);
	out.append(file.readAll());
	file.close();
	file.remove();


	return out;
}

bool ClassifierSharkSvmC::deserialize(const QByteArray & serializedClassifier)
{
	int i = 0;
	while (serializedClassifier.at(i) != '\n')
		i++;

	QByteArray gammaByteArray = serializedClassifier.left(i);
	QList<QByteArray> gammaSplit = gammaByteArray.split('=');
	if (gammaSplit.count() != 2)
		return false;

	double gamma = gammaSplit.at(1).toDouble();
	qDebug() << gamma;

	QByteArray classifierByteArray = serializedClassifier;
	classifierByteArray.remove(0, i + 1);

	using namespace shark;
	// TODO:
	QFile file("tempLoad.dat");
	file.open(QIODevice::WriteOnly);
	file.write(classifierByteArray);
	file.close();
	worker->kernel = GaussianRbfKernel<>(gamma, true);

	worker->classifier = shark::KernelClassifier<sample_type>(&worker->kernel);
	ifstream ifs("tempLoad.dat");
	boost::archive::polymorphic_text_iarchive ia(ifs);
	worker->decisionFunction.load(ia,1);
	ifs.close();
	file.remove();

	return true;
}

void ClassifierSharkSvmC::addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth)
{
	worker->setCropping(ui.selectSlices->isChecked());
	worker->setBottomSlice(ui.bottomSliceLimit->text().toInt());
	worker->setTopSlice(ui.topSliceLimit->text().toInt());

	// Selected labels need to be known before any loading occurs (see run worker's run method)
	QVector<int> labels;
	QList<QString> labelsStrings = ui.selectedLabels->text().split(",");
	for (const QString& i : labelsStrings) {
		labels.append(i.toInt());
	}
	worker->selectLabels(labels);
	worker->appendToTrainingQueue(volumes, truth);
}

void ClassifierSharkSvmC::addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result)
{
	worker->setClassifyCropping(classifyUi.selectSlices->isChecked());
	worker->setClassifyBottomSlice(classifyUi.bottomSliceLimit->text().toInt());
	worker->setClassifyTopSlice(classifyUi.topSliceLimit->text().toInt());

	worker->appendToClassifyingQueue(volumes, result);
}

void ClassifierSharkSvmC::selectSlicesToggled(bool state)
{
	ui.bottomSliceLimit->setVisible(state);
	ui.bottomSliceLimitLabel->setVisible(state);
	ui.topSliceLimit->setVisible(state);
	ui.topSliceLimitLabel->setVisible(state);
}

void ClassifierSharkSvmC::selectClassifySlicesToggled(bool state)
{
	classifyUi.bottomSliceLimit->setVisible(state);
	classifyUi.bottomSliceLimitLabel->setVisible(state);
	classifyUi.topSliceLimit->setVisible(state);
	classifyUi.topSliceLimitLabel->setVisible(state);
}

void ClassifierSharkSvmC::setAutomaticallyToggled(bool state)
{
	ui.cLowLimit->setVisible(state);
	ui.cLowLimitLabel->setVisible(state);
	ui.cHighLimit->setVisible(state);
	ui.cHighLimitLabel->setVisible(state);
	ui.steps->setVisible(state);
	ui.stepsLabel->setVisible(state);

	ui.gammaLabel->setVisible(!state);
	ui.gamma->setVisible(!state);
	ui.cLabel->setVisible(!state);
	ui.c->setVisible(!state);
}


void ClassifierSharkSvmC::valuesOptimized(double gamma, double c)
{
	ui.gamma->setText(QString::number(gamma));
	ui.c->setText(QString::number(c));
}

void ClassifierSharkSvmC::infoMessageBox(const QString& title, const QString& text, const QString& button)
{
	// GUI needs to be processed in main thread
	QMessageBox::information(qSlicerApplication::activeWindow(), title, text, button);
}

void ClassifierSharkSvmC::startTraining()
{
	worker->setOptimization(ui.setAutomatically->isChecked());

	worker->setCHigh(ui.cHighLimit->text().toDouble());
	worker->setCLow(ui.cLowLimit->text().toDouble());
	worker->setCSteps(ui.steps->text().toInt());

	worker->setGamma(ui.gamma->text().toDouble());
	worker->setC(ui.c->text().toDouble());
	worker->train();
}

ClassifierSharkSvmCWorker::ClassifierSharkSvmCWorker()
	: QThread(), mutex(new QMutex()),
	waitCondition(new QWaitCondition()), readyToTrain(false),
	autoOptimize(false), decisionFunction(funct_type(&normalizer, &classifier))
{
	subthreadCount = qMax<int>(QThread::idealThreadCount() - 1, 4);
}

ClassifierSharkSvmCWorker::~ClassifierSharkSvmCWorker()
{
	delete mutex;
	delete waitCondition;
	exit();
}

funct_type ClassifierSharkSvmCWorker::getDecisionFunction()
{
	return this->decisionFunction;
}

void ClassifierSharkSvmCWorker::appendToTrainingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth)
{
	QMutexLocker lock(mutex);

	SingleVolumeTrainingEntry entry;
	entry.truth = truth;
	entry.volumes = volumes;
	trainingEntriesQueue.append(entry);

	waitCondition->wakeAll();
}

void ClassifierSharkSvmCWorker::appendToClassifyingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result)
{
	QMutexLocker lock(mutex);

	if (decisionFunction.name() == "") {
		QMessageBox::warning(qSlicerApplication::activeWindow(), "No decision function", "This classifier is currently untrained. Aborting.");
		return;
	}

	SingleVolumeClassifyingEntry entry;
	entry.result = result;
	entry.volumes = volumes;
	classifyingEntriesQueue.append(entry);

	waitCondition->wakeAll();
}

void ClassifierSharkSvmCWorker::train()
{
	QMutexLocker lock(mutex);
	readyToTrain = true;

	waitCondition->wakeAll();
}

void ClassifierSharkSvmCWorker::setOptimization(bool state)
{
	QMutexLocker lock(mutex);
	autoOptimize = state;
}

void ClassifierSharkSvmCWorker::run()
{
	forever{
		QMutexLocker lock(mutex);
	// Train when all training volumes have been loaded and train function has been called
	if (trainingEntriesQueue.isEmpty() && classifyingEntriesQueue.isEmpty()) {
		if (readyToTrain) {

			using namespace shark;
			dataset = shark::createLabeledDataFromRange(samples, labels);


			qDebug() << "Samples: " << dataset.numberOfElements();

			// Create and train data normalizer
			qDebug() << "Normalizing..";
			bool removeMean = true;
			NormalizeComponentsUnitVariance<RealVector> normalizingTrainer(removeMean);
			normalizingTrainer.train(normalizer, dataset.inputs());

			// Transform data
			dataset.inputs() = transform(dataset.inputs(), normalizer);

			if (autoOptimize) {
				qDebug() << "Optimizing..";
				optimize();
				emit infoMessageBoxRequest("Optimization finished",
					QString("\nGamma: %1 \nC: %2")
					.arg(gamma).arg(c), "OK");
			}
			qDebug() << "Gamma: " << gamma;
			qDebug() << "C: " << c;

			qDebug() << "Setting gamma and C";

			kernel = shark::GaussianRbfKernel<>(gamma, true);
			shark::CSvmTrainer<sample_type> trainer(&kernel, c, true);

			qDebug() << "Training";
			
			trainer.sparsify() = true;
			//to relax or tighten the stopping criterion from 1e-3 (here, tightened to 1e-6)
			trainer.stoppingCondition().minAccuracy = 1e-6;
			//to set the cache size to 128MB for double (16**6 times sizeof(double), when double was selected as cache type above)
			//or to 64MB for float (16**6 times sizeof(float), when the CSvmTrainer is declared without second template argument)
			trainer.setCacheSize(0x1000000);
			
			trainer.train(classifier, dataset);

			decisionFunction = (normalizer >> classifier);

			qDebug() << "Finished";
			emit classifierTrained();
			dataset = shark::ClassificationDataset();
			samples.clear();
			labels.clear();
		}
		waitCondition->wait(mutex);
		lock.unlock();
	}
	// Load queued volumes
	else if (!trainingEntriesQueue.isEmpty()) {
		SingleVolumeTrainingEntry entry = trainingEntriesQueue.at(0);
		trainingEntriesQueue.remove(0);

		lock.unlock();

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

					sample_type oneVector(nodesImageData.count());

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

					// Get rid of duplicite values
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
	}
	// Classify queued volumes
	else if (!classifyingEntriesQueue.isEmpty()) {
		int dims[3];
		vtkMRMLVolumeNode::SafeDownCast(classifyingEntriesQueue.at(0).volumes.at(0))->GetImageData()->GetDimensions(dims);

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
			threads.emplace_back(&ClassifierSharkSvmCWorker::classifyThreaded, this);
		}
		currentlyClassified = classifyingEntriesQueue.at(0).result;
		// Start threads
		mutex->unlock();
		for (size_t i = 0; i < threads.size(); i++)
		{
			threads[i].join();
		}
		qDebug() << "Threads joined";

		lock.relock();
		classifyingEntriesQueue.remove(0);
		lock.unlock();

		emit volumeClassified(currentlyClassified);
	}
	}
}

void ClassifierSharkSvmCWorker::optimize()
{
	using namespace shark;
	using namespace std;
	gamma = 1.0;
	c = 1.0;
	GaussianRbfKernel<> sampleKernel(0.5, true);
	KernelClassifier<RealVector> svm;
	CSvmTrainer<RealVector> trainer(&sampleKernel, 1.0, true, true);
	const unsigned int K = 5;  // Number of folds
	ZeroOneLoss<unsigned int> loss;
	CVFolds<ClassificationDataset> folds = createCVSameSizeBalanced(dataset, K);
	CrossValidationError<KernelClassifier<RealVector>, unsigned int> cvError(
		folds, &trainer, &svm, &trainer, &loss
	);
	JaakkolaHeuristic ja(dataset);
	double ljg = log(ja.gamma());
	qDebug() << "ljg: " << ljg;
	GridSearch grid;
	vector<double> min(2);
	vector<double> max(2);
	vector<size_t> sections(2);
	min[0] = ljg - 4.; max[0] = ljg + 4; sections[0] = 9;  // Kernel parameter gamma
	min[1] = cLow; max[1] = cHigh; sections[1] = cSteps;     // Regularization parameter C
	grid.configure(min, max, sections);
	grid.step(cvError);

	for (size_t i = 0; i < grid.solution().point.size(); i++)
		qDebug() << grid.solution().point(i);
	qDebug() << grid.solution().value;
	gamma = grid.solution().point(0);
	c = grid.solution().point(1);

	emit valuesOptimized(gamma, c);
}

void ClassifierSharkSvmCWorker::setGamma(double gamma)
{
	QMutexLocker lock(mutex);
	this->gamma = gamma;
}

void ClassifierSharkSvmCWorker::setC(double c)
{
	QMutexLocker lock(mutex);
	this->c = c;
}
void ClassifierSharkSvmCWorker::setCropping(bool crop)
{
	QMutexLocker lock(mutex);
	this->crop = crop;
}
void ClassifierSharkSvmCWorker::setBottomSlice(int bottom)
{
	QMutexLocker lock(mutex);
	bottomSlice = bottom;
}
void ClassifierSharkSvmCWorker::setTopSlice(int top)
{
	QMutexLocker lock(mutex);
	topSlice = top;
}
void ClassifierSharkSvmCWorker::selectLabels(const QVector<int>& labels)
{
	QMutexLocker lock(mutex);
	selectedLabels = labels;
}

void ClassifierSharkSvmCWorker::setCLow(double cLow)
{
	QMutexLocker lock(mutex);
	this->cLow = cLow;
}
void ClassifierSharkSvmCWorker::setCHigh(double cHigh)
{
	QMutexLocker lock(mutex);
	this->cHigh = cHigh;
}
void ClassifierSharkSvmCWorker::setCSteps(int cSteps)
{
	QMutexLocker lock(mutex);
	this->cSteps = cSteps;
}

void ClassifierSharkSvmCWorker::setClassifyCropping(bool crop)
{
	QMutexLocker lock(mutex);
	this->classifyCrop = crop;
}
void ClassifierSharkSvmCWorker::setClassifyBottomSlice(int bottom)
{
	QMutexLocker lock(mutex);
	classifyBottomSlice = bottom;
}
void ClassifierSharkSvmCWorker::setClassifyTopSlice(int top)
{
	QMutexLocker lock(mutex);
	classifyTopSlice = top;
}

void ClassifierSharkSvmCWorker::classifyThreaded()
{
	QMutexLocker lock(mutex);
	lock.unlock();

	QVector<vtkSmartPointer<vtkImageData>> nodesImageData;
	for (size_t i = 0; i < classifyingEntriesQueue.at(0).volumes.count(); i++) {
		vtkSmartPointer<vtkMRMLVolumeNode> node = vtkMRMLVolumeNode::SafeDownCast(classifyingEntriesQueue.at(0).volumes.at(i));
		nodesImageData.append(node->GetImageData());
	}

	lock.relock();

	while (!slicesToClassify.isEmpty()) {

		size_t k = slicesToClassify.first();
		slicesToClassify.remove(0);
		lock.unlock();

		vtkSmartPointer<vtkMRMLVolumeNode> resultNode = vtkMRMLVolumeNode::SafeDownCast(classifyingEntriesQueue.at(0).result);

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
				unsigned int a;
				decisionFunction.eval(oneVector, a);

				resultImageData->SetScalarComponentFromDouble(i, j, k, 0, a);
			}
		}
		lock.relock();
	}
	lock.unlock();
}