#ifndef CLASSIFIER_DLIB_SVM_C_H
#define CLASSIFIER_DLIB_SVM_C_H

#include "SupervisedClassifier.h"
#include "ui_ClassifierDlibSvmCTrain.h"
#include "ui_ClassifierDlibSvmCClassify.h"

#include <QThread>

#include <dlib/svm.h>

#include <vector>
class QWaitCondition;
class QMutex;
namespace ClassifierDlibSvmCNamespace
{

	typedef dlib::matrix<double, 0L, 1L> sample_type;
	typedef dlib::radial_basis_kernel<sample_type> kernel_type;
	typedef dlib::normalized_function<dlib::decision_function<kernel_type>> funct_type;

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

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierDlibSvmCWorker
	/// - run-method thread used by ClassifierDlibSvmC
	/// - used for saving training vectors from volumes, optimizing classifier's parameters and classification
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierDlibSvmCWorker : public QThread
	{
		Q_OBJECT
	public:
		ClassifierDlibSvmCWorker();
		~ClassifierDlibSvmCWorker();

		funct_type getDecisionFunction();

		void appendToTrainingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth);
		void appendToClassifyingQueue(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result);
		void run();

		// Training settings

		void setCropping(bool crop);
		void setBottomSlice(int bottom);
		void setTopSlice(int top);
		void setGamma(double gamma);
		void setC(double c);
		void setLabel(int label);

		// Optimization settings

		void setOptimization(bool state);
		void setCLow(double cLow);
		void setCHigh(double cHigh);
		void setCSteps(int cSteps);
		void setGammaLow(double gammaLow);
		void setGammaHigh(double gammaHigh);
		void setGammaSteps(int gammaSteps);

		// Clssification settings

		void setClassifyCropping(bool crop);
		void setClassifyBottomSlice(int bottom);
		void setClassifyTopSlice(int top);
		void setThreshold(double threshold);

		// The resulting decision function
		funct_type decisionFunction;

	public slots:
		void train();

	signals:
		void classifierTrained();
		void volumeClassified(vtkSmartPointer<vtkMRMLNode> result);
		void valuesOptimized(double gamma, double c, double specificity, double sensitivity);
		void infoMessageBoxRequest(const QString& title, const QString& text, const QString& button);

	private:
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
		std::vector<sample_type> samples;
		std::vector<double> labels;
		//std::vector<sample_type> allSamples;
		//std::vector<int> allLabels;
		int selectedLabel;
		bool crop;
		double c, gamma;
		int bottomSlice, topSlice;

		// Optimizing

		bool autoOptimize;
		double cLow, cHigh, cSteps;
		double gammaLow, gammaHigh, gammaSteps;

		double bestSpec;
		double bestSens;
		QVector<double> cs;
		QVector<double> gammas;

		// Classifying
		bool classifyCrop;
		int classifyBottomSlice, classifyTopSlice;
		double threshold;
		QVector<SingleVolumeClassifyingEntry> classifyingEntriesQueue;
		vtkSmartPointer<vtkMRMLNode> currentlyClassified;
		QVector<int> slicesToClassify;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///
	/// ClassifierDlibSvmC
	/// - Supervised classifier using dlib's Support Vector Machine as training target and decision function
	///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ClassifierDlibSvmC : public SupervisedClassifier
	{
		Q_OBJECT
	public:
		ClassifierDlibSvmC();
		~ClassifierDlibSvmC();

		virtual QByteArray serialize() override;
		virtual bool deserialize(const QByteArray& serializedClassifier) override;

		public slots:
		virtual void startTraining() override;
		virtual void addVolumesToTraining(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, const vtkSmartPointer<vtkMRMLNode> truth) override;
		virtual void addVolumesToClassifying(const QVector<vtkSmartPointer<vtkMRMLNode>>& volumes, vtkSmartPointer<vtkMRMLNode> result) override;
		void selectSlicesToggled(bool state);
		void setAutomaticallyToggled(bool state);
		void selectClassifySlicesToggled(bool state);
		void valuesOptimized(double gamma, double c, double specificity, double sensitivity);
		void infoMessageBox(const QString& title, const QString& text, const QString& button);
		void thresholdValueChanged();
		void thresholdSliderChanged();

	private:
		ClassifierDlibSvmCWorker* worker;
		Ui::ClassifierDlibSvmCTrain ui;
		Ui::ClassifierDlibSvmCClassify classifyUi;
	};

	// Computing sensitivity and specificity since Shark does not support this by default
	// When using multiple labels, sensitivity and specificity is computed per label and the result is an average of those
	// First = Sensitivity, Second = Specificity
	QPair<double, double> sensSpec(const std::vector<int>& classified, const std::vector<int>& truth);
}

#endif // CLASSIFIER_DLIB_SVM_C_H