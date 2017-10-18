#include "SegmentationToolboxClassifyingWidget.h"
#include "ui_SegmentationToolboxClassifyingWidget.h"

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QtXml>

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

	QString saveFilepath = QFileDialog::getOpenFileName(this, "Open..", "", "Extensible Markup Language (*.xml)");
	if (saveFilepath.isEmpty())
		return;
	QByteArray serializedTraining;
	QFile file(saveFilepath);
	file.open(QIODevice::ReadOnly);
	serializedTraining = file.readAll();
	file.close();

	if (serializedTraining.isEmpty())
		return;

	QDomDocument doc;
	QString err;
	if (!doc.setContent(serializedTraining, &err)) {
		QMessageBox::warning(qSlicerCoreApplication::activeWindow(),
			"Loading failed",
			err);
		return;
	}

	QDomElement currentChild = doc.firstChildElement();

	if (currentChild.isNull())
		return;

	QString version = currentChild.attribute("xmlVersion");

	currentChild = currentChild.firstChildElement();

	QString classifierName;
	QByteArray classifierSettings;
	QVector<QString> algNames;
	QVector<QByteArray> algSettings;
	while (!currentChild.isNull()) {
		if (currentChild.tagName() == "classifier") {
			QDomElement settings = currentChild.firstChildElement();
			if (settings.tagName() == "settings") {
				if (settings.attribute("format") == "Base64") {
					classifierSettings = QByteArray::fromBase64(settings.text().toLocal8Bit());
					classifierName = currentChild.attribute("id");
				}
			}
		}
		else if (currentChild.tagName() == "preprocessing") {
			algNames.append(currentChild.attribute("id"));
			QDomElement settings = currentChild.firstChildElement();
			if (settings.tagName() == "settings") {
				if (settings.attribute("format") == "Base64") {
					algSettings.append(QByteArray::fromBase64(settings.text().toLocal8Bit()));
				}
				else {
					algSettings.append(QByteArray());
				}
			}
		}

		currentChild = currentChild.nextSiblingElement();
	}

	changeSelectedClassifier(classifierName, classifierSettings);
	setImageRangePreprocessingSelector(algNames, algSettings);
}