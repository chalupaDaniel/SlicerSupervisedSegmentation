#include "ClassificationResultCombinator.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkNew.h"
#include "qSlicerCoreApplication.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkImageData.h"

#include <string>
#include <QMap>

ClassificationResultCombinator::ClassificationResultCombinator(QObject* parent)
	: volumesCombined(0), QObject(parent)
{
}

void ClassificationResultCombinator::addToClassifiedBuffer(vtkSmartPointer<vtkMRMLNode> volume)
{
	classifiedBuffer.append(volume);
}
void ClassificationResultCombinator::clearBuffer()
{
	classifiedBuffer.clear();
}
void ClassificationResultCombinator::combine(int mode)
{
	switch (mode)
	{
	case NoCombination:
		break;
	case LogicalAnd:
		combineLogicalAnd();
		break;
	case LogicalOr:
		combineLogicalOr();
		break;
	case StatisticalMode:
		combineStatMode();
		break;
	default:
		break;
	}
}

void ClassificationResultCombinator::combineLogicalAnd()
{
	// Prepare image data
	QVector<vtkSmartPointer<vtkImageData>> nodesImageData = prepareImageData(classifiedBuffer);

	int dims[3];
	nodesImageData.at(0)->GetDimensions(dims);

	// Prepare final volume
	vtkNew<vtkMRMLScalarVolumeDisplayNode> volDisplay;
	vtkNew<vtkImageData> volData;
	vtkNew<vtkMRMLScalarVolumeNode> vol;

	prepareFinalVolume(dims, volDisplay.GetPointer(), volData.GetPointer(), vol.GetPointer());
	volumesCombined++;

	for (size_t i = 0; i < dims[0]; i++) {
		for (size_t j = 0; j < dims[1]; j++) {
			for (size_t k = 0; k < dims[2]; k++) {
				bool label = true;
				for (size_t volumeIndex = 0; volumeIndex < nodesImageData.count(); volumeIndex++) {
					label &= !qFuzzyCompare(nodesImageData.at(volumeIndex)->GetScalarComponentAsDouble(i, j, k, 0), 0.0);
				}
				volData->SetScalarComponentFromDouble(i, j, k, 0, label);
			}
		}
	}

	vol->SetAndObserveImageData(volData.GetPointer());

	emit finishedCombining(vtkMRMLNode::SafeDownCast(vol.GetPointer()));
}
void ClassificationResultCombinator::combineLogicalOr()
{
	// Prepare image data
	QVector<vtkSmartPointer<vtkImageData>> nodesImageData = prepareImageData(classifiedBuffer);

	int dims[3];
	nodesImageData.at(0)->GetDimensions(dims);

	// Prepare final volume
	vtkNew<vtkMRMLScalarVolumeDisplayNode> volDisplay;
	vtkNew<vtkImageData> volData;
	vtkNew<vtkMRMLScalarVolumeNode> vol;

	prepareFinalVolume(dims, volDisplay.GetPointer(), volData.GetPointer(), vol.GetPointer());
	volumesCombined++;

	for (size_t i = 0; i < dims[0]; i++) {
		for (size_t j = 0; j < dims[1]; j++) {
			for (size_t k = 0; k < dims[2]; k++) {
				bool label = true;
				for (size_t volumeIndex = 0; volumeIndex < nodesImageData.count(); volumeIndex++) {
					label |= !qFuzzyCompare(nodesImageData.at(volumeIndex)->GetScalarComponentAsDouble(i, j, k, 0), 0.0);
				}
				volData->SetScalarComponentFromDouble(i, j, k, 0, label);
			}
		}
	}

	vol->SetAndObserveImageData(volData.GetPointer());

	emit finishedCombining(vtkMRMLNode::SafeDownCast(vol.GetPointer()));
}
void ClassificationResultCombinator::combineStatMode()
{
	// Prepare image data
	QVector<vtkSmartPointer<vtkImageData>> nodesImageData = prepareImageData(classifiedBuffer);

	int dims[3];
	nodesImageData.at(0)->GetDimensions(dims);

	// Prepare final volume
	vtkNew<vtkMRMLScalarVolumeDisplayNode> volDisplay;
	vtkNew<vtkImageData> volData;
	vtkNew<vtkMRMLScalarVolumeNode> vol;

	prepareFinalVolume(dims, volDisplay.GetPointer(), volData.GetPointer(), vol.GetPointer());
	volumesCombined++;

	for (size_t i = 0; i < dims[0]; i++) {
		for (size_t j = 0; j < dims[1]; j++) {
			for (size_t k = 0; k < dims[2]; k++) {
				QMap<int, int> labelCount;
				for (size_t volumeIndex = 0; volumeIndex < nodesImageData.count(); volumeIndex++) {
					int label = !qFuzzyCompare(nodesImageData.at(volumeIndex)->GetScalarComponentAsDouble(i, j, k, 0), 0.0);
					if (labelCount.contains(label))
						labelCount[label]++;
					else
						labelCount.insert(label, 1);
				}
				int labelMaxCount = 0;
				int modeLabel = 0;
				for (const int currentLabel : labelCount.keys()) {
					if (labelCount.value(currentLabel) > labelMaxCount)
						modeLabel = currentLabel;
				}
				volData->SetScalarComponentFromDouble(i, j, k, 0, modeLabel);
			}
		}
	}

	vol->SetAndObserveImageData(volData.GetPointer());

	emit finishedCombining(vtkMRMLNode::SafeDownCast(vol.GetPointer()));
}
QVector<vtkSmartPointer<vtkImageData>> ClassificationResultCombinator::prepareImageData(const QVector<vtkSmartPointer<vtkMRMLNode>>& classifiedBuffer) const
{
	QVector<vtkSmartPointer<vtkImageData>> nodesImageData;
	for (size_t i = 0; i < classifiedBuffer.count(); i++) {
		vtkSmartPointer<vtkMRMLVolumeNode> node = vtkMRMLVolumeNode::SafeDownCast(classifiedBuffer.at(i));
		nodesImageData.append(node->GetImageData());
	}
}
void ClassificationResultCombinator::prepareFinalVolume(const int* const dimensions, vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> display, vtkSmartPointer<vtkImageData> data,
	vtkSmartPointer<vtkMRMLScalarVolumeNode> volume) const
{
	data.GetPointer()->SetDimensions(dimensions);
	data.GetPointer()->AllocateScalars(VTK_UNSIGNED_INT, 1);
	display.GetPointer()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");

	qSlicerCoreApplication::application()->mrmlScene()->AddNode(display.GetPointer());

	volume.GetPointer()->SetName(std::string("ST " + std::string("Combined Results ") + std::to_string(volumesCombined)).c_str());

	volume.GetPointer()->AddAndObserveDisplayNodeID(display.GetPointer()->GetID());

	qSlicerCoreApplication::application()->mrmlScene()->AddNode(volume.GetPointer());
}