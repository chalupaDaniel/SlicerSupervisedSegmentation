#ifndef CLASSIFICATION_RESULT_COMBINATOR_H
#define CLASSIFICATION_RESULT_COMBINATOR_H

#include <QObject>
#include <QVector>
#include "vtkSmartPointer.h"

class vtkMRMLNode;
class ClassificationResultCombinator : public QObject
{
	Q_OBJECT
public:
	enum {
		NoCombination,
		LogicalAnd,
		LogicalOr,
		StatisticalMode
	};

	ClassificationResultCombinator(QObject* parent = nullptr);

	void addToClassifiedBuffer(vtkSmartPointer<vtkMRMLNode> volume);
	void clearBuffer();
	void combine(int mode);
signals:
	void finishedCombining(vtkSmartPointer<vtkMRMLNode> combinedVolume);

private:
	void combineLogicalAnd();
	void combineLogicalOr();
	void combineStatMode();

	QVector<vtkSmartPointer<vtkImageData>> prepareImageData(const QVector<vtkSmartPointer<vtkMRMLNode>>& classifiedBuffer) const;
	void prepareFinalVolume(const int* const dimensions, vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> display, vtkSmartPointer<vtkImageData> data,
		vtkSmartPointer<vtkMRMLScalarVolumeNode> volume) const;

	QVector<vtkSmartPointer<vtkMRMLNode>> classifiedBuffer;

	int volumesCombined;
};

#endif // CLASSIFICATION_RESULT_COMBINATOR_H