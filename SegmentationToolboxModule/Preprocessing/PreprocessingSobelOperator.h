#ifndef PREPROCESSING_SOBEL_2D_H
#define PREPROCESSING_SOBEL_2D_H

#include "PreprocessingAlgorithm.h"

class QLineEdit;
class QMutex;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Preprocessing algorithm - 3D sobel operator
/// - enables three-axis sobel operator
/// - uses ITK's implementation
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class PreprocessingSobel2d : public PreprocessingAlgorithm
{
	Q_OBJECT
public:
	PreprocessingSobel2d();
	~PreprocessingSobel2d();

	void preprocess(vtkSmartPointer<vtkMRMLNode> inputNode, vtkSmartPointer<vtkMRMLNode> outputNode, int referenceId) override;

	QByteArray serialize() const override;
	bool deserialize(const QByteArray&) override;
};

#endif // PREPROCESSING_MEDIAN_3D_H