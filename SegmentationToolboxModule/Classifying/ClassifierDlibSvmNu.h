#ifndef CLASSIFIER_DLIB_SVM_NU_H
#define CLASSIFIER_DLIB_SVM_NU_H

#include "SupervisedClassifier.h"
#include "SupervisedClassifierWorker.h"
#include "ui_ClassifierDlibSvmNuTrain.h"
#include "ui_ClassifierDlibSvmNuClassify.h"

#include <QThread>

#include <dlib/svm.h>

#include <vector>
class QMutex;
namespace ClassifierDlibSvmNuNamespace
{

	typedef dlib::matrix<double, 0L, 1L> sample_type;
	typedef dlib::radial_basis_kernel<sample_type> kernel_type;
	typedef dlib::normalized_function<dlib::decision_function<kernel_type>> funct_type;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierDlibSvmNuWorker
	/// - run-method thread used by ClassifierDlibSvmNu
	/// - used for saving training vectors from volumes
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierDlibSvmNuWorker : public SupervisedClassifierWorker
	{
		Q_OBJECT
	public:
		ClassifierDlibSvmNuWorker();
		~ClassifierDlibSvmNuWorker();

		// The resulting decision function
		funct_type decisionFunction;

		// Training settings

		void setGamma(double gamma);
		void setNu(double nu);
		void setCropping(bool crop);
		void setBottomSlice(int bottom);
		void setTopSlice(int top);
		void setLabel(int label);

		// Optimization settings

		void setOptimization(bool state);
		void setNuLow(double nuLow);
		void setNuHigh(double nuHigh);
		void setNuSteps(int nuSteps);
		void setGammaLow(double gammaLow);
		void setGammaHigh(double gammaHigh);
		void setGammaSteps(int gammaSteps);

		// Classification settings

		void setClassifyCropping(bool crop);
		void setClassifyBottomSlice(int bottom);
		void setClassifyTopSlice(int top);
		void setThreshold(double threshold);

	signals:
		void volumeClassified(vtkSmartPointer<vtkMRMLNode> result);
		void valuesOptimized(double gamma, double nu, double specificity, double sensitivity);
		void infoMessageBoxRequest(const QString& title, const QString& text, const QString& button);

	private:
		void processTraining() override;
		void processTrainingEntry(const SingleVolumeTrainingEntry& entry) override;
		void processClassifyingEntry(const SingleVolumeClassifyingEntry& entry) override;

		// Entry function to the optimization algorithm
		// TODO: move optimization to another class
		void optimize();

		// Main function for threaded optimization
		void optimizeThreaded();

		// Main function for threaded classification
		void classifyThreaded(const SingleVolumeClassifyingEntry& entry);

		// Training

		std::vector<sample_type> samples;
		std::vector<double> labels;
		//std::vector<sample_type> allSamples;
		//std::vector<int> allLabels;
		int selectedLabel;
		double nu, gamma;
		bool crop;
		int bottomSlice, topSlice;

		// Optimizing

		bool autoOptimize;
		double nuLow, nuHigh, nuSteps;
		double gammaLow, gammaHigh, gammaSteps;

		double bestSpec;
		double bestSens;
		QVector<double> nus;
		QVector<double> gammas;

		// Classifying

		bool classifyCrop;
		int classifyBottomSlice, classifyTopSlice;
		double threshold;
		vtkSmartPointer<vtkMRMLNode> currentlyClassified;
		QVector<int> slicesToClassify;

		int subthreadCount;
		QMutex* classifyMutex;
		QMutex* optimizeMutex;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierDlibSvmNu
	/// - Supervised classifier using dlib's Support Vector Machine as training target and decision function
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierDlibSvmNu : public SupervisedClassifier
	{
		Q_OBJECT
	public:
		ClassifierDlibSvmNu();
		~ClassifierDlibSvmNu();

		virtual QByteArray serialize() override;
		virtual bool deserialize(const QByteArray& serializedClassifier) override;

	public slots:
		virtual void startTraining() override;
		virtual void addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth) override;
		virtual void addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result) override;
		void selectSlicesToggled(bool state);
		void setAutomaticallyToggled(bool state);
		void selectClassifySlicesToggled(bool state);
		void valuesOptimized(double gamma, double nu, double specificity, double sensitivity);
		void infoMessageBox(const QString& title, const QString& text, const QString& button);
		void thresholdValueChanged();
		void thresholdSliderChanged();

	private:
		ClassifierDlibSvmNuWorker* worker;
		Ui::ClassifierDlibSvmNuTrain ui;
		Ui::ClassifierDlibSvmNuClassify classifyUi;
	};

	//// Computing sensitivity and specificity since Shark does not support this by default
	//// When using multiple labels, sensitivity and specificity is computed per label and the result is an average of those
	//// First = Sensitivity, Second = Specificity
	//QPair<double, double> sensSpec(const std::vector<int>& classified, const std::vector<int>& truth);
}

#endif // CLASSIFIER_DLIB_SVM_H