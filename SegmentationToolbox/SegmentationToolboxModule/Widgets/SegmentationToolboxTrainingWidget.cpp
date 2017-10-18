#include "SegmentationToolboxTrainingWidget.h"
#include "ui_SegmentationToolboxTrainingWidget.h"

#include "ImageRangePreprocessingSelector.h"
#include "preprocessingAlgorithm.h"
#include "VolumeManager.h"
#include "SupervisedClassifier.h"
#include "ClassifierList.h"
#include "VolumeSelectorDialog.h"

#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"

#include <QDebug>
#include <QSharedPointer>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QtXml>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SegmentationToolboxModule
class SegmentationToolboxTrainingWidgetPrivate
	: public Ui_SegmentationToolboxTrainingWidget
{
	Q_DECLARE_PUBLIC(SegmentationToolboxTrainingWidget);
protected:
	SegmentationToolboxTrainingWidget* const q_ptr;

public:
	SegmentationToolboxTrainingWidgetPrivate(
		SegmentationToolboxTrainingWidget& object);
	virtual void setupUi(SegmentationToolboxTrainingWidget*);
};

// --------------------------------------------------------------------------
SegmentationToolboxTrainingWidgetPrivate
::SegmentationToolboxTrainingWidgetPrivate(
SegmentationToolboxTrainingWidget& object)
: q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void SegmentationToolboxTrainingWidgetPrivate
::setupUi(SegmentationToolboxTrainingWidget* widget)
{
	this->Ui_SegmentationToolboxTrainingWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerSegmentationToolboxModuleFooBarWidget methods

//-----------------------------------------------------------------------------
SegmentationToolboxTrainingWidget
::SegmentationToolboxTrainingWidget(QWidget* parentWidget)
: Superclass(parentWidget)
, d_ptr(new SegmentationToolboxTrainingWidgetPrivate(*this)), selectedClassifier(nullptr)
{
	Q_D(SegmentationToolboxTrainingWidget);
	d->setupUi(this);

	d->classifierSettings->setLayout(new QVBoxLayout());

	volumeManager = new VolumeManager(qSlicerCoreApplication::application()->coreIOManager(), "Training");
	classifierList = new ClassifierList();
	volumeSelectorDialog = new VolumeSelectorDialog(this);

	for (QSharedPointer<SupervisedClassifier> classifier : classifierList->classifiers)
		d->classifierSelection->addItem(classifier->name());
	classifierSelectionChanged(d->classifierSelection->currentText());

	connect(d->classifierSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(classifierSelectionChanged(const QString&)));

	connect(d->addImages, SIGNAL(clicked()), d->imageRangePreprocessingSelector, SLOT(addRow()));

	connect(d->train, SIGNAL(clicked()), this, SLOT(trainClicked()));

	connect(d->addClassified, SIGNAL(clicked()), volumeSelectorDialog, SLOT(exec()));
	connect(volumeSelectorDialog, SIGNAL(volumesSelected(const QStringList&)), this, SLOT(classifiedImagesSelected(const QStringList&)));

	connect(d->imageRangePreprocessingSelector, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)),
		this, SLOT(disableEditing()));
	connect(volumeManager, SIGNAL(previewComplete()), this, SLOT(enableEditing()));
	connect(volumeManager, SIGNAL(trainingComplete()), this, SLOT(classifierFinishedTraining()));

	connect(d->imageRangePreprocessingSelector, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)),
		volumeManager, SLOT(switchPreview(QSharedPointer<PreprocessingAlgorithm>, const QString&)));
	connect(d->saveAs, SIGNAL(clicked()), this, SLOT(saveRequested()));
}

//-----------------------------------------------------------------------------
SegmentationToolboxTrainingWidget
::~SegmentationToolboxTrainingWidget()
{
	delete volumeManager;
	delete classifierList;
}
void SegmentationToolboxTrainingWidget::classifierFinishedTraining()
{
	Q_D(SegmentationToolboxTrainingWidget);
	enableEditing();
	d->saveAs->setEnabled(true);
}
void SegmentationToolboxTrainingWidget::disableEditing()
{
	setEnabled(false);
}
void SegmentationToolboxTrainingWidget::enableEditing()
{
	setEnabled(true);
}

void SegmentationToolboxTrainingWidget::classifierSelectionChanged(const QString& classifierString)
{
	Q_D(SegmentationToolboxTrainingWidget);
	for (QSharedPointer<SupervisedClassifier> classifier : classifierList->classifiers){
		if (classifierString == classifier->name())
		{
			if (selectedClassifier != nullptr) {
				d->classifierSettings->layout()->removeWidget(selectedClassifier->trainingWidget());
				selectedClassifier->trainingWidget()->hide();
			}
			d->classifierSettings->layout()->addWidget(classifier->trainingWidget());
			classifier->trainingWidget()->show();
			d->classifierSettings->update();
			volumeManager->setSelectedClassifier(classifier);
			selectedClassifier = classifier;
			d->saveAs->setEnabled(false);
		}
	}
}
void SegmentationToolboxTrainingWidget::trainClicked()
{
	Q_D(SegmentationToolboxTrainingWidget);
	disableEditing();
	volumeManager->startTrainingSequence(d->imageRangePreprocessingSelector->allFilenames(), d->imageRangePreprocessingSelector->algorithms(), classifiedImages);
	
}
void SegmentationToolboxTrainingWidget::classifiedImagesSelected(const QStringList& volumes)
{
	Q_D(SegmentationToolboxTrainingWidget);
	classifiedImages = volumes;

	if (classifiedImages.count())
		d->addClassified->setText(QString::number(classifiedImages.count()) + " file"
			+ (classifiedImages.count() > 1 ? "s" : QString())
			+ " selected");
	else
		d->addClassified->setText("Select labels");
}

void SegmentationToolboxTrainingWidget::saveRequested()
{
	QVector<QStringList> filepathsList = volumeManager->lastTrainingFilepaths();
	QVector<QSharedPointer<PreprocessingAlgorithm>> algs = volumeManager->lastTrainingAlgorithms();

	QDomDocument doc;
	QDomElement supervisedClassifierToolbox = doc.createElement("supervisedClassifierToolbox");
	supervisedClassifierToolbox.setAttribute("xmlVersion", "1.0");

	QDomElement classifier = doc.createElement("classifier");
	classifier.setAttribute("id", selectedClassifier->name());
	QDomElement classifierSettings = doc.createElement("settings");
	classifierSettings.setAttribute("format", "Base64");
	QDomText b64 = doc.createTextNode(selectedClassifier->serialize().toBase64());
	classifierSettings.appendChild(b64);
	classifier.appendChild(classifierSettings);
	supervisedClassifierToolbox.appendChild(classifier);

	for (size_t i = 0; i < filepathsList.count(); i++)
	{
		QDomElement preprocessing = doc.createElement("preprocessing");
		QDomElement preprocessingSettings = doc.createElement("settings");
		preprocessingSettings.setAttribute("format", "Base64");

		if (algs.at(i).isNull()) {
			preprocessing.setAttribute("id", "none");
		}
		else {
			preprocessing.setAttribute("id", algs.at(i)->name());
			b64 = doc.createTextNode(algs.at(i)->serialize().toBase64());
		}
		preprocessingSettings.appendChild(b64);
		preprocessing.appendChild(preprocessingSettings);
		supervisedClassifierToolbox.appendChild(preprocessing);
	}

	doc.appendChild(supervisedClassifierToolbox);

	QString saveFilepath = QFileDialog::getSaveFileName(this, "Save as..", "", "Extensible Markup Language (*.xml)");
	if (saveFilepath.isEmpty())
		return;

	QFile file(saveFilepath);
	file.open(QIODevice::WriteOnly);
	file.write(doc.toByteArray());
	file.close();
}