#include "SegmentationToolboxClassifyingWidget.h"
#include "ui_SegmentationToolboxClassifyingWidget.h"

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "ClassifierList.h"
#include "SupervisedClassifier.h"
#include "VolumeManager.h"

#include "qSlicerCoreApplication.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SegmentationToolboxModule
class SegmentationToolboxClassifyingWidgetPrivate
	: public Ui_SegmentationToolboxClassifyingWidget
{
	Q_DECLARE_PUBLIC(SegmentationToolboxClassifyingWidget);
protected:
	SegmentationToolboxClassifyingWidget* const q_ptr;

public:
	SegmentationToolboxClassifyingWidgetPrivate(
		SegmentationToolboxClassifyingWidget& object);
	virtual void setupUi(SegmentationToolboxClassifyingWidget*);
};

// --------------------------------------------------------------------------
SegmentationToolboxClassifyingWidgetPrivate
::SegmentationToolboxClassifyingWidgetPrivate(
SegmentationToolboxClassifyingWidget& object)
: q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void SegmentationToolboxClassifyingWidgetPrivate
::setupUi(SegmentationToolboxClassifyingWidget* widget)
{
	this->Ui_SegmentationToolboxClassifyingWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerSegmentationToolboxModuleFooBarWidget methods

//-----------------------------------------------------------------------------
SegmentationToolboxClassifyingWidget
::SegmentationToolboxClassifyingWidget(QWidget* parentWidget)
: Superclass(parentWidget)
, d_ptr(new SegmentationToolboxClassifyingWidgetPrivate(*this)), selectedClassifier(nullptr)
{
	Q_D(SegmentationToolboxClassifyingWidget);
	d->setupUi(this);

	d->classifierSettings->setLayout(new QVBoxLayout());

	d->classify->setEnabled(false);
	d->classifierName->setText("Load or train a classifier");

	connect(d->load, SIGNAL(clicked()), this, SLOT(loadClicked()));
	connect(d->classify, SIGNAL(clicked()), this, SLOT(classifyClicked()));

	classifierList = QSharedPointer<ClassifierList>(new ClassifierList());

	volumeManager = new VolumeManager(qSlicerCoreApplication::application()->coreIOManager(), "Classifying");

	connect(d->imageRangePreprocessingSelector, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)),
		this, SLOT(disableEditing()));

	connect(d->imageRangePreprocessingSelector, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)),
		volumeManager, SLOT(switchPreview(QSharedPointer<PreprocessingAlgorithm>, const QString&)));
	connect(volumeManager, SIGNAL(previewComplete()), this, SLOT(enableEditing()));
	connect(volumeManager, SIGNAL(classifyingComplete()), this, SLOT(enableEditing()));
	connect(d->addImages, SIGNAL(clicked()), d->imageRangePreprocessingSelector, SLOT(addRow()));
}

//-----------------------------------------------------------------------------
SegmentationToolboxClassifyingWidget
::~SegmentationToolboxClassifyingWidget()
{
}

void SegmentationToolboxClassifyingWidget::setImageRangePreprocessingSelector(const QVector<QString>& algNames, const QVector<QByteArray>& algSettings)
{
	if (algNames.count() != algSettings.count()) {
		QMessageBox::warning(qSlicerCoreApplication::activeWindow(),
			"Loading failed",
			"The number of algorithm names does not equal number of algorithm settings. File corrupted.");
		return;
	}

	Q_D(SegmentationToolboxClassifyingWidget);
	d->imageRangePreprocessingSelector->clear();
	for (size_t i = 0; i < algNames.count(); i++)
	{
		d->imageRangePreprocessingSelector->addRowAndSetAlgorithm(algNames.at(i), algSettings.at(i));
	}

}

void SegmentationToolboxClassifyingWidget::changeSelectedClassifier(const QString& name, const QByteArray& settings)
{
	Q_D(SegmentationToolboxClassifyingWidget);
	for (QSharedPointer<SupervisedClassifier> classifier : classifierList->classifiers)
	{
		if (classifier->name() == name)
		{
			if (selectedClassifier != nullptr) {
				d->classifierSettings->layout()->removeWidget(selectedClassifier->classifyingWidget());
				selectedClassifier->classifyingWidget()->hide();
			}
			if (classifier->deserialize(settings)) {
				selectedClassifier = classifier;
				volumeManager->setSelectedClassifier(selectedClassifier);
				d->classifierName->setText(name);
				d->classify->setEnabled(true);

				d->classifierSettings->layout()->addWidget(classifier->classifyingWidget());
				classifier->classifyingWidget()->show();
				d->classifierSettings->update();
			}
			break;
		}
	}
}

void SegmentationToolboxClassifyingWidget::disableEditing()
{
	setEnabled(false);
}
void SegmentationToolboxClassifyingWidget::enableEditing() 
{
	setEnabled(true);
}

void SegmentationToolboxClassifyingWidget::classifyClicked()
{
	Q_D(SegmentationToolboxClassifyingWidget);
	disableEditing();
	volumeManager->startClassifyingSequence(d->imageRangePreprocessingSelector->allFilenames(), d->imageRangePreprocessingSelector->algorithms());
}

void SegmentationToolboxClassifyingWidget::loadClicked()
{

	QString saveFilepath = QFileDialog::getOpenFileName(this, "Open..", "", "Supervised Toolbox Classifiers (*.stc)");
	if (saveFilepath.isEmpty())
		return;
	QByteArray serializedTraining;
	QFile file(saveFilepath);
	file.open(QIODevice::ReadOnly);
	serializedTraining = file.readAll();
	file.close();

	if (serializedTraining.isEmpty())
		return;

	QString classifierName;
	QByteArray classifierSettings;
	QVector<QString> algNames;
	QVector<QByteArray> algSettings;
	QList<QByteArray> split = serializedTraining.split('\n');

	for (size_t i = 0; i < split.count(); i++)
	{
		QByteArray oneSplit = split.at(i);
		if (oneSplit.contains("clasifierName=")) {
			classifierName = QString(oneSplit).remove("clasifierName=");
		} else if (oneSplit.contains("===classifierSetting===") && ((i + 1) < split.count())) {

			i += 1;
			QByteArray insideSplit;
			while (!insideSplit.contains("===/classifierSetting===") && (i < split.count()))
			{
				classifierSettings.append(insideSplit + '\n');
				insideSplit = split.at(i);
				i++;
			}
			classifierSettings.remove(0, 1);
			classifierSettings.remove(classifierSettings.length() - 1, 1);
		} else if (oneSplit.contains("preprocessingAlgorithm=")) {
			QString algName = QString(oneSplit).remove("preprocessingAlgorithm=");
			algNames.append(algName);
		} else if (oneSplit.contains("===preprocessingSetting===")) {
			QByteArray settings;
			i += 1;
			QByteArray insideSplit;
			while (!insideSplit.contains("===/preprocessingSetting===") && (i < split.count()))
			{
				settings.append(insideSplit + '\n');
				insideSplit = split.at(i);
				i++;
			}
			settings.remove(0, 1);
			settings.remove(settings.length() - 1, 1);
			algSettings.append(settings);
		}
	}
	changeSelectedClassifier(classifierName, classifierSettings);
	setImageRangePreprocessingSelector(algNames, algSettings);
}