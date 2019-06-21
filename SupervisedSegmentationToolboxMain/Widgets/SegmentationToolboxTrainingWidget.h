#ifndef SEGMENTATION_TOOLBOX_TRAINING_WIDGET_H
#define SEGMENTATION_TOOLBOX_TRAINING_WIDGET_H

// Qt includes
#include <QWidget>
#include <QStringList>
#include <QSharedPointer>

#include "qSlicerSupervisedSegmentationToolboxMainModuleWidgetsExport.h"

class SegmentationToolboxTrainingWidgetPrivate;
class VolumeManager;
class ClassifierList;
class SupervisedClassifier;
class VolumeSelectorDialog;

/// \ingroup Slicer_QtModules_SupervisedSegmentationToolboxMain
class Q_SLICER_MODULE_SUPERVISEDSEGMENTATIONTOOLBOXMAIN_WIDGETS_EXPORT SegmentationToolboxTrainingWidget
	: public QWidget
{
	Q_OBJECT
public:
	typedef QWidget Superclass;
	SegmentationToolboxTrainingWidget(QWidget *parent = 0);
	virtual ~SegmentationToolboxTrainingWidget();

protected:
	QScopedPointer<SegmentationToolboxTrainingWidgetPrivate> d_ptr;

private:
	VolumeManager* volumeManager;
	ClassifierList* classifierList;
	QSharedPointer<SupervisedClassifier> selectedClassifier;
	QStringList classifiedImages;
	VolumeSelectorDialog* volumeSelectorDialog;

	Q_DECLARE_PRIVATE(SegmentationToolboxTrainingWidget);
	Q_DISABLE_COPY(SegmentationToolboxTrainingWidget);

private slots:
	void disableEditing();
	void enableEditing();
	void classifierSelectionChanged(const QString& classifierString);
	void trainClicked();
	void classifiedImagesSelected(const QStringList& volumes);
	void saveRequested();
	void classifierFinishedTraining();
};

#endif // SEGMENTATION_TOOLBOX_TRAINING_WIDGET_H
