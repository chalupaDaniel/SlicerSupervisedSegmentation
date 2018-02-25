#ifndef CLASSIFIER_SHARK_SVM_C_H
#define CLASSIFIER_SHARK_SVM_C_H

#include "SupervisedClassifier.h"
#include "SupervisedClassifierWorker.h"
#include "ui_ClassifierSharkSvmCTrain.h"
#include "ui_ClassifierSharkSvmCClassify.h"

#include <shark/Models/Kernels/GaussianRbfKernel.h> //the used kernel for the SVM
#include <shark/Models/ConcatenatedModel.h>
#include <shark/Algorithms/Trainers/NormalizeComponentsUnitVariance.h>
#include <shark/Algorithms/Trainers/CSvmTrainer.h> // the C-SVM trainer

#include <QThread>
#include <vector>

class QMutex;
namespace ClassifierSharkSvmCNamespace
{

	typedef shark::RealVector sample_type;
	typedef shark::GaussianRbfKernel<> kernel_type;
	typedef shark::ConcatenatedModel<shark::RealVector, unsigned int> funct_type;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierSharkSvmCWorker
	/// - run-method thread used by ClassifierDlibSvm
	/// - used for saving training vectors from volumes, optimizing classifier's parameters and classification
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierSharkSvmCWorker : public SupervisedClassifierWorker
	{
		Q_OBJECT
	public:
		ClassifierSharkSvmCWorker();
		~ClassifierSharkSvmCWorker();

		funct_type getDecisionFunction();

		// Training settings

		void setCropping(bool crop);
		void setBottomSlice(int bottom);
		void setTopSlice(int top);
		void selectLabels(const QVector<int>& labels);
		void setGamma(double gamma);
		void setC(double c);

		// Optimization settings

		void setOptimization(bool state);
		void setCLow(double cLow);
		void setCHigh(double cHigh);
		void setCSteps(int cSteps);

		// Classification settings

		void setClassifyCropping(bool crop);
		void setClassifyBottomSlice(int bottom);
		void setClassifyTopSlice(int top);

		// The resulting decision function
		funct_type decisionFunction;		
		shark::KernelClassifier<sample_type> classifier;
		shark::Normalizer<sample_type> normalizer;
		kernel_type kernel;

	signals:
		void volumeClassified(vtkSmartPointer<vtkMRMLNode> result);
		void valuesOptimized(double gamma, double c);
		void infoMessageBoxRequest(const QString& title, const QString& text, const QString& button);

	private:
		void processTraining() override;
		void processTrainingEntry(const SingleVolumeTrainingEntry& entry) override;
		void processClassifyingEntry(const SingleVolumeClassifyingEntry& entry) override;

		// Training parameters optimization function
		void optimize();

		// Main function for threaded classification
		void classifyThreaded(const SingleVolumeClassifyingEntry& entry);

		// Training

		std::vector<shark::RealVector> samples;
		std::vector<unsigned int> labels;
		QVector<int> selectedLabels;
		shark::ClassificationDataset dataset;
		bool crop;
		double c, gamma;
		int bottomSlice, topSlice;

		// Optimizing

		bool autoOptimize;
		double cLow, cHigh, cSteps;

		// Classifying

		bool classifyCrop;
		int classifyBottomSlice, classifyTopSlice;
		vtkSmartPointer<vtkMRMLNode> currentlyClassified;
		QVector<int> slicesToClassify;

		int subthreadCount;
		QMutex* classifyMutex;
		QMutex* optimizeMutex;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierSharkSvmC
	/// - Supervised classifier using Shark's Support Vector Machine as training target and decision function
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierSharkSvmC : public SupervisedClassifier
	{
		Q_OBJECT
	public:
		ClassifierSharkSvmC();
		~ClassifierSharkSvmC();

		virtual QByteArray serialize() override;
		virtual bool deserialize(const QByteArray& serializedClassifier) override;

		public slots:
		virtual void startTraining() override;
		virtual void addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth) override;
		virtual void addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result) override;
		void selectSlicesToggled(bool state);
		void selectClassifySlicesToggled(bool state);
		void setAutomaticallyToggled(bool state);
		void valuesOptimized(double gamma, double nu);
		void infoMessageBox(const QString& title, const QString& text, const QString& button);

	private:
		ClassifierSharkSvmCWorker* worker;
		Ui::ClassifierSharkSvmCTrain ui;
		Ui::ClassifierSharkSvmCClassify classifyUi;
	};
}

#endif // CLASSIFIER_SHARK_SVM_C_H