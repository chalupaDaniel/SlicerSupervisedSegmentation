#include "SegmentationToolboxTrainingWidget.h"
#include "ui_SegmentationToolboxTrainingWidget.h"

#include "ImageRangePreprocessingSelector.h"
#include "preprocessingAlgorithm.h"
#include "VolumeManager.h"
#include "SupervisedClassifier.h"
#include "ClassifierList.h"

#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"

#include <QDebug>
#include <QSharedPointer>
#include <QVBoxLayout>
#include <QFileDialog>

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

	for (QSharedPointer<SupervisedClassifier> classifier : classifierList->classifiers)
		d->classifierSelection->addItem(classifier->name());
	classifierSelectionChanged(d->classifierSelection->currentText());

	connect(d->classifierSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(classifierSelectionChanged(const QString&)));

	connect(d->addImages, SIGNAL(clicked()), d->imageRangePreprocessingSelector, SLOT(addRow()));

	connect(d->train, SIGNAL(clicked()), this, SLOT(trainClicked()));
	connect(d->addClassified, SIGNAL(clicked()), this, SLOT(addClassifiedClicked()));

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
void SegmentationToolboxTrainingWidget::addClassifiedClicked()
{
	Q_D(SegmentationToolboxTrainingWidget);
	classifiedImages = QFileDialog::getOpenFileNames(this, "Select labels");
	
	if (classifiedImages.count())
		d->addClassified->setText(QString::number(classifiedImages.count()) + " file"
		+ (classifiedImages.count() > 1 ? "s" : QString())
		+ " selected");
	else
		d->addClassified->setText("Select labels");
}

void SegmentationToolboxTrainingWidget::saveRequested()
{
	QByteArray classifier = selectedClassifier->serialize();

	QVector<QStringList> filepathsList = volumeManager->lastTrainingFilepaths();
	QVector<QSharedPointer<PreprocessingAlgorithm>> algs = volumeManager->lastTrainingAlgorithms();
	QVector<QByteArray> fileLines;
	for (size_t i = 0; i < filepathsList.count(); i++)
	{
		QByteArray fileLine = "fileCount=";
		fileLine.append(QByteArray::number(filepathsList.count()));
		fileLine.append("\n");
		fileLine.append("fileList=");
		for (int j = 0; j < filepathsList.at(i).count(); j++) {
			const QString& singleFilename = filepathsList.at(i).at(j);
			fileLine.append(singleFilename);
			if ((j+1) != filepathsList.at(i).count())
				fileLine.append(",");
		}
		fileLine.append("\n");
		fileLine.append("preprocessingAlgorithm=");
		if (algs.at(i).isNull()) {
			fileLine.append("None");
			fileLine.append("\n");
			fileLine.append("===preprocessingSetting===\n");
			fileLine.append("===/preprocessingSetting===");
		}
		else {
			fileLine.append(algs.at(i)->name());
			fileLine.append("\n");
			fileLine.append("===preprocessingSetting===\n");
			fileLine.append(algs.at(i)->serialize());
			fileLine.append("\n");
			fileLine.append("===/preprocessingSetting===");
		}
		fileLine.append("\n");
		fileLines.append(fileLine);
	}

	QByteArray out;
	out.append("clasifierName=");
	out.append(selectedClassifier->name());
	out.append("\n");
	out.append("===classifierSetting===\n");
	out.append(selectedClassifier->serialize());
	out.append("\n===/classifierSetting===\n");
	for (const QByteArray& byteArray : fileLines)
		out.append(byteArray);

	QString saveFilepath = QFileDialog::getSaveFileName(this, "Save as..", "", "Supervised Toolbox Classifiers (*.stc)");
	if (saveFilepath.isEmpty())
		return;

	QFile file(saveFilepath);
	file.open(QIODevice::WriteOnly);
	file.write(out);
	file.close();
}