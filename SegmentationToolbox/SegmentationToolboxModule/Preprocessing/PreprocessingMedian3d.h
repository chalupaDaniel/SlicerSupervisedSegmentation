#ifndef PREPROCESSING_MEDIAN_3D_H
#define PREPROCESSING_MEDIAN_3D_H

#include "PreprocessingAlgorithm.h"

class QLineEdit;
class QMutex;
///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Preprocessing algorithm - 3D median
/// - enables three-axis median filter with variable window size
/// - uses ITK's implementation
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class PreprocessingMedian3d : public PreprocessingAlgorithm
{
	Q_OBJECT
public:
	PreprocessingMedian3d();
	~PreprocessingMedian3d();

	void preprocess(vtkSmartPointer<vtkMRMLNode> inputNode, vtkSmartPointer<vtkMRMLNode> outputNode, int referenceId) override;

	QByteArray serialize() const override;
	bool deserialize(const QByteArray&) override;

private:
	int maskSizeX, maskSizeY, maskSizeZ;
	QLineEdit* lineEditX;
	QLineEdit* lineEditY;
	QLineEdit* lineEditZ;
	QMutex* mutex;
private slots:
	void setMaskSize();
	void cancelled();
};

#endif // PREPROCESSING_MEDIAN_3D_H