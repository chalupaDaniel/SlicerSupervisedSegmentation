#ifndef SEGMENTATION_TOOLBOX_CLASSIFYING_WIDGET_H
#define SEGMENTATION_TOOLBOX_CLASSIFYING_WIDGET_H

// Qt includes
#include <QWidget>
#include <QStringList>
#include <QSharedPointer>

#include "vtkSmartPointer.h"

#include "qSlicerSupervisedSegmentationToolboxMainModuleWidgetsExport.h"

class SegmentationToolboxClassifyingWidgetPrivate;
class SupervisedClassifier;
class ClassifierList;
class VolumeManager;
class ClassifierWidget;
class ClassificationResultCombinator;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_SupervisedSegmentationToolboxMain
class Q_SLICER_MODULE_SUPERVISEDSEGMENTATIONTOOLBOXMAIN_WIDGETS_EXPORT SegmentationToolboxClassifyingWidget
	: public QWidget
{
	Q_OBJECT
public:
	typedef QWidget Superclass;
	SegmentationToolboxClassifyingWidget(QWidget *parent = 0);
	virtual ~SegmentationToolboxClassifyingWidget();

public slots:
	void setImageRangePreprocessingSelector(const QVector<QString>& algNames, const QVector<QByteArray>& algSettings);


protected:
	QScopedPointer<SegmentationToolboxClassifyingWidgetPrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(SegmentationToolboxClassifyingWidget);
	Q_DISABLE_COPY(SegmentationToolboxClassifyingWidget);

	// Goes through all loaded classifiers
	void classificationStep();

	// Combines classification results
	void combineResults();

	QSharedPointer<ClassifierList> classifierList;
	VolumeManager* volumeManager;

	QVector<ClassifierWidget*> classifierWidgets;
	int currentClassifierIndex;
	ClassificationResultCombinator* combinator;

private slots:
	void addClassifierClicked();
	void disableEditing();
	void enableEditing();
	void classifyClicked();
	void classifierWidgetClearRequested();
	void singleClassifierFinished(vtkSmartPointer<vtkMRMLNode> result);
};

#endif // SEGMENTATION_TOOLBOX_CLASSIFYING_WIDGET_H
