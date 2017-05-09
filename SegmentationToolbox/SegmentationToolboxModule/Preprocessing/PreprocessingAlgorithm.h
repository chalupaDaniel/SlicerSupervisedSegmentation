#ifndef PREPROCESSING_ALGORITHM_H
#define PREPROCESSING_ALGORITHM_H

#include <QString>
#include <QWidget>
#include "vtkSmartPointer.h"

class vtkMRMLNode;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Template class for preprocessing algorithms
/// When subclassing
/// - set the algorithm name
/// - implement (de)serialization to be able to load settings from a file
/// - implement your algorithm in the preprocess function, which should be thread-safe
/// - emit preprocessingDone signal to enable correct synchronization
/// - when needed, create a dialog for algorithm setting
/// - add your algorithm to PreprocessingAlgorithmList.h
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class PreprocessingAlgorithm : public QObject
{
	Q_OBJECT
public:
	PreprocessingAlgorithm() : dialog(nullptr) {}
	~PreprocessingAlgorithm();

	// Has to be thread-safe to enable concurrent preprocessing and widget operation, otherwise a crash is expected
	virtual void preprocess(vtkSmartPointer<vtkMRMLNode> inputNode, vtkSmartPointer<vtkMRMLNode> outputNode, int referenceId) = 0;

	virtual QByteArray serialize() const = 0;
	virtual bool deserialize(const QByteArray&) = 0;

	QString name() const { return algorithmName; }
	void showDialog();
	bool hasDialog() const;

signals:
	void preprocessingDone(int referenceId);

protected:
	QString algorithmName;
	QWidget* dialog;
};

#endif //PREPROCESSING_ALGORITHM_H