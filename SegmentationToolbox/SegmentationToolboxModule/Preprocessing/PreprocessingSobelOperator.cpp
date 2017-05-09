#include "PreprocessingSobelOperator.h"

#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMutex>
#include <QMutexLocker>

#include "vtkImageSobel2D.h"

#include "vtkMRMLNode.h"
#include "vtkImageData.h"
#include "vtkMRMLVolumeNode.h"

#include <QDebug>

PreprocessingSobel2d::PreprocessingSobel2d()
	: PreprocessingAlgorithm()
{
	algorithmName = "Sobel operator";
}

PreprocessingSobel2d::~PreprocessingSobel2d()
{
}

void PreprocessingSobel2d::preprocess(vtkSmartPointer<vtkMRMLNode> inputNode, vtkSmartPointer<vtkMRMLNode> outputNode, int referenceId)
{
	vtkSmartPointer<vtkImageSobel2D> sobelOperator = vtkSmartPointer<vtkImageSobel2D>::New();
	vtkSmartPointer<vtkMRMLVolumeNode> inputVolume = vtkMRMLVolumeNode::SafeDownCast(inputNode);

	vtkSmartPointer<vtkMRMLVolumeNode> outputVolume = vtkMRMLVolumeNode::SafeDownCast(outputNode);

	sobelOperator->SetInputConnection(inputVolume->GetImageDataConnection());

	sobelOperator->Update();

	vtkSmartPointer<vtkImageData> outputData = sobelOperator->GetOutput();
	vtkSmartPointer<vtkImageData> newOutputData = vtkSmartPointer<vtkImageData>::New();
	int* outputDimensions = outputData->GetDimensions();
	newOutputData->SetExtent(outputData->GetExtent());
	newOutputData->AllocateScalars(outputData->GetDataObjectType(), 1);
	for (size_t i = 0; i < outputDimensions[0]; i++) {

		for (size_t j = 0; j < outputDimensions[1]; j++) {

			for (size_t k = 0; k < outputDimensions[2]; k++) {
				double xData = outputData->GetScalarComponentAsDouble(i,j,k,0);
				double yData = outputData->GetScalarComponentAsDouble(i,j,k,1);
				newOutputData->SetScalarComponentFromDouble(i, j, k, 0, sqrt(pow(xData, 2) + pow(yData, 2)));
			}
		}
	}

	int state = outputNode->StartModify();
	outputVolume->SetAndObserveImageData(newOutputData);
	outputNode->EndModify(state);

	emit preprocessingDone(referenceId);
}

QByteArray PreprocessingSobel2d::serialize() const
{
	return QByteArray();
}

bool PreprocessingSobel2d::deserialize(const QByteArray& byteArray)
{
	return true;
}
