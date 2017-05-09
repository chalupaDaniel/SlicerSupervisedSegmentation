#ifndef SEGMENTATION_TOOLBOX_CLASSIFYING_WIDGET_H
#define SEGMENTATION_TOOLBOX_CLASSIFYING_WIDGET_H

// Qt includes
#include <QWidget>
#include <QStringList>
#include <QSharedPointer>

#include "qSlicerSegmentationToolboxModuleModuleWidgetsExport.h"

class SegmentationToolboxClassifyingWidgetPrivate;
class SupervisedClassifier;
class ClassifierList;
class VolumeManager;

/// \ingroup Slicer_QtModules_SegmentationToolboxModule
class Q_SLICER_MODULE_SEGMENTATIONTOOLBOXMODULE_WIDGETS_EXPORT SegmentationToolboxClassifyingWidget
	: public QWidget
{
	Q_OBJECT
public:
	typedef QWidget Superclass;
	SegmentationToolboxClassifyingWidget(QWidget *parent = 0);
	virtual ~SegmentationToolboxClassifyingWidget();

	public slots:
	void changeSelectedClassifier(const QString& name, const QByteArray& settings);
	void setImageRangePreprocessingSelector(const QVector<QString>& algNames, const QVector<QByteArray>& algSettings);


protected:
	QScopedPointer<SegmentationToolboxClassifyingWidgetPrivate> d_ptr;

private:

	Q_DECLARE_PRIVATE(SegmentationToolboxClassifyingWidget);
	Q_DISABLE_COPY(SegmentationToolboxClassifyingWidget);

	QSharedPointer<SupervisedClassifier> selectedClassifier;
	QSharedPointer<ClassifierList> classifierList;
	VolumeManager* volumeManager;

private slots:
	void loadClicked();
	void disableEditing();
	void enableEditing();
	void classifyClicked();
};

#endif // SEGMENTATION_TOOLBOX_CLASSIFYING_WIDGET_H
