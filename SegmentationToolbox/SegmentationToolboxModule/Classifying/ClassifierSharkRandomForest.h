#ifndef CLASSIFIER_SHARK_RANDOM_FOREST_H
#define CLASSIFIER_SHARK_RANDOM_FOREST_H

#include "SupervisedClassifier.h"
#include "ui_ClassifierSharkRandomForestTrain.h"
#include "ui_ClassifierSharkRandomForestClassify.h"

#include <shark/Algorithms/Trainers/RFTrainer.h>
#include <shark/Models/ConcatenatedModel.h>
#include <shark/Algorithms/Trainers/NormalizeComponentsUnitVariance.h>

#include <QThread>
#include <vector>

class QWaitCondition;
class QMutex;
namespace ClassifierSharkRandomForestNamespace
{
	typedef shark::RealVector sampleType;
	typedef shark::ConcatenatedModel<shark::RealVector, shark::RealVector> funct_type;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// SingleVolumeEntries
	/// - structure representing all volumes that the classifier trains in one go
	/// - used for saving training vectors from volumes in VolumeReadingThread
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

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierSharkRandomForestWorker
	/// - run-method thread used by ClassifierSharkRandomForest
	/// - used for saving training vectors from volumes, optimizing classifier's parameters and classification
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierSharkRandomForestWorker : public QThread
	{
		Q_OBJECT
	public:
		ClassifierSharkRandomForestWorker();
		~ClassifierSharkRandomForestWorker();

		funct_type getDecisionFunction();

		void appendToTrainingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth);
		void appendToClassifyingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result);
		void run();

		// Training settings

		void setCropping(bool crop);
		void setBottomSlice(int bottom);
		void setTopSlice(int top);
		void selectLabels(const QVector<unsigned int>& labels);

		// Optimization settings

		void setOptimization(bool state);

		void setRandAttr(int randAttr);
		void setNumTrees(int numTrees);
		void setNodeSize(int nodeSize);
		void setOob(double oob);

		void setRandAttrLow(int randAttrLow);
		void setNumTreesLow(int numTreesLow);
		void setNodeSizeLow(int nodeSizeLow);
		void setOobLow(double oobLow);

		void setRandAttrHigh(int randAttrHigh);
		void setNumTreesHigh(int numTreesHigh);
		void setNodeSizeHigh(int nodeSizeHigh);
		void setOobHigh(double oobHigh);

		void setRandAttrSteps(int randAttrSteps);
		void setNumTreesSteps(int numTreesSteps);
		void setNodeSizeSteps(int nodeSizeSteps);
		void setOobSteps(int oobSteps);

		// Classification settings

		void setClassifyCropping(bool crop);
		void setClassifyBottomSlice(int bottom);
		void setClassifyTopSlice(int top);

		// The resulting decision function
		funct_type decisionFunction;
		shark::Normalizer<sampleType> normalizer;
		shark::RFClassifier classifier;

	public slots:
		// This gets called after all appendToTrainingQueue functions have been issued from the main thread
		void train();

	signals:
		void classifierTrained();
		void volumeClassified(vtkSmartPointer<vtkMRMLNode> result);
		void valuesOptimized(int randAttr, int numTrees, int nodeSize, double oob);
		void infoMessageBoxRequest(const QString& title, const QString& text, const QString& button);

	private:
		// Single classifier's parameter set
		struct OptimizationParameterSet {
			int randAttr;
			int numTrees;
			int nodeSize;
			double oob;
		};

		// Entry function to the optimization algorithm
		// TODO: move optimization to another class
		void optimize();

		// Main function for threaded optimization
		void optimizeThreaded();

		// Main function for threaded classification
		void classifyThreaded();

		// Flow control

		QMutex* mutex;
		QWaitCondition* waitCondition;
		bool readyToTrain;
		int subthreadCount;

		// Training

		QVector<SingleVolumeTrainingEntry> trainingEntriesQueue;
		std::vector<sampleType> samples;
		std::vector<unsigned int> labels;
		//std::vector<sampleType> allSamples;
		//std::vector<unsigned int> allLabels;

		QVector<unsigned int> selectedLabels;
		shark::ClassificationDataset dataset;
		bool crop;
		int bottomSlice, topSlice;
		int randAttr, numTrees, nodeSize;
		double oob;

		// Optimizing

		QVector<OptimizationParameterSet> optimizationQueue;
		QVector<shark::ClassificationDataset> datasetSubsets;
		bool autoOptimize;
		int randAttrLow, numTreesLow, nodeSizeLow;
		double oobLow;
		int randAttrHigh, numTreesHigh, nodeSizeHigh;
		double oobHigh;
		int randAttrSteps, numTreesSteps, nodeSizeSteps, oobSteps;
		double bestSpec, bestSens;

		// Classifying

		bool classifyCrop;
		int classifyBottomSlice, classifyTopSlice;
		QVector<SingleVolumeClassifyingEntry> classifyingEntriesQueue;
		vtkSmartPointer<vtkMRMLNode> currentlyClassified;

		QVector<int> slicesToClassify;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierSharkRandomForest
	/// - Supervised classifier using Shark's Random Forest as training target and decision function
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierSharkRandomForest : public SupervisedClassifier
	{
		Q_OBJECT
	public:
		ClassifierSharkRandomForest();
		~ClassifierSharkRandomForest();

		virtual QByteArray serialize() override;
		virtual bool deserialize(const QByteArray& serializedClassifier) override;

	public slots:
		virtual void startTraining() override;
		virtual void addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth) override;
		virtual void addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result) override;
		void selectSlicesToggled(bool state);
		void selectClassifySlicesToggled(bool state);
		void setAutomaticallyToggled(bool state);
		void valuesOptimized(int randAttr, int numTrees, int nodeSize, double oob);
		void infoMessageBox(const QString& title, const QString& text, const QString& button);

	private:
		ClassifierSharkRandomForestWorker* worker;
		Ui::ClassifierSharkRandomForestTrain ui;
		Ui::ClassifierSharkRandomForestClassify classifyUi;
	};

	// Computing sensitivity and specificity since Shark does not support this by default
	// When using multiple labels, sensitivity and specificity is computed per label and the result is an average of those
	// First = Sensitivity, Second = Specificity
	QPair<double, double> sensSpec(const std::vector<unsigned int>& classified, const shark::Data<unsigned int>& truth, const QVector<unsigned int>& labels);
}

#endif // CLASSIFIER_SHARK_RANDOM_FOREST_H