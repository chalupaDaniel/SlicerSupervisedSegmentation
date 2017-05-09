#ifndef PREPROCESSING_GAUSS_FILTER_H
#define PREPROCESSING_GAUSS_FILTER_H

#include "PreprocessingAlgorithm.h"

class QLineEdit;
class QMutex;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Preprocessing algorithm - Gauss filter
/// - uses ITK's implementation
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class PreprocessingGaussFilter : public PreprocessingAlgorithm
{
	Q_OBJECT
public:
	PreprocessingGaussFilter();
	~PreprocessingGaussFilter();

	void preprocess(vtkSmartPointer<vtkMRMLNode> inputNode, vtkSmartPointer<vtkMRMLNode> outputNode, int referenceId) override;

	QByteArray serialize() const override;
	bool deserialize(const QByteArray&) override;

private:
	double radius, std;
	QLineEdit* radiusEdit;
	QLineEdit* stdEdit;
	QMutex* mutex;
private slots:
	void setMaskSize();
	void cancelled();
};

#endif // PREPROCESSING_GAUSS_FILTER_H