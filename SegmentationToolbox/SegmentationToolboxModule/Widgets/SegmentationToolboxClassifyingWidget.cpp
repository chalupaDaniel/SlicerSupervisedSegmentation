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
#include "ClassifierWidget.h"
#include "ClassificationResultCombinator.h"

#include "vtkMRMLNode.h"

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
, d_ptr(new SegmentationToolboxClassifyingWidgetPrivate(*this))
{
	Q_D(SegmentationToolboxClassifyingWidget);
	d->setupUi(this);

	d->classify->setEnabled(false);

	connect(d->classify, SIGNAL(clicked()), this, SLOT(classifyClicked()));

	classifierList = QSharedPointer<ClassifierList>(new ClassifierList());

	volumeManager = new VolumeManager(qSlicerCoreApplication::application()->coreIOManager(), "Classifying");

	connect(d->imageRangePreprocessingSelector, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)),
		this, SLOT(disableEditing()));

	connect(d->imageRangePreprocessingSelector, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)),
		volumeManager, SLOT(switchPreview(QSharedPointer<PreprocessingAlgorithm>, const QString&)));
	connect(volumeManager, SIGNAL(previewComplete()), this, SLOT(enableEditing()));
	connect(volumeManager, SIGNAL(classifyingComplete(vtkSmartPointer<vtkMRMLNode>)), this, SLOT(singleClassifierFinished(vtkSmartPointer<vtkMRMLNode>)));
	connect(d->addImages, SIGNAL(clicked()), d->imageRangePreprocessingSelector, SLOT(addRow()));
	connect(d->addClassifier, SIGNAL(clicked()), this, SLOT(addClassifierClicked()));

	d->combination->addItem("No Combination", ClassificationResultCombinator::NoCombination);
	d->combination->addItem("Logical AND", ClassificationResultCombinator::LogicalAnd);
	d->combination->addItem("Logical OR", ClassificationResultCombinator::LogicalOr);
	d->combination->addItem("Statistical MODE", ClassificationResultCombinator::StatisticalMode);
	d->combination->hide();

	combinator = new ClassificationResultCombinator(this);
	connect(combinator, SIGNAL(finishedCombining(vtkSmartPointer<vtkMRMLNode>)), volumeManager, SLOT(showVolume(vtkSmartPointer<vtkMRMLNode>)));
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
	disableEditing();
	currentClassifierIndex = 0;

	classificationStep();
}

void SegmentationToolboxClassifyingWidget::classificationStep()
{
	Q_D(SegmentationToolboxClassifyingWidget);

	volumeManager->setSelectedClassifier(classifierWidgets.at(currentClassifierIndex)->classifier());
	volumeManager->startClassifyingSequence(d->imageRangePreprocessingSelector->allFilenames(), d->imageRangePreprocessingSelector->algorithms());

	currentClassifierIndex++;
}
void SegmentationToolboxClassifyingWidget::singleClassifierFinished(vtkSmartPointer<vtkMRMLNode> result)
{
	if (currentClassifierIndex == classifierWidgets.count()) {
		combinator->addToClassifiedBuffer(result);
		combineResults();
		enableEditing();
	} else {
		combinator->addToClassifiedBuffer(result);
		classificationStep();
	}
}
void SegmentationToolboxClassifyingWidget::combineResults()
{
	Q_D(SegmentationToolboxClassifyingWidget);

	combinator->combine(d->combination->itemData(d->combination->currentIndex()).toInt());
}

void SegmentationToolboxClassifyingWidget::addClassifierClicked()
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
				} else {
					algSettings.append(QByteArray());
				}
			}
		}

		currentChild = currentChild.nextSiblingElement();
	}

	Q_D(SegmentationToolboxClassifyingWidget);
	if (classifierWidgets.isEmpty()) {
		setImageRangePreprocessingSelector(algNames, algSettings);
	} else {
		if (d->imageRangePreprocessingSelector->algorithms().count() != algNames.count()) {
			QMessageBox::warning(qSlicerCoreApplication::activeWindow(),
				"Loading failed",
				"The selected classifier has different input dimensions to previous classifiers");
			return;
		}
	}

	for (QSharedPointer<SupervisedClassifier> classifier : classifierList->classifiers)
	{
		if (classifier->name() == classifierName)
		{
			QSharedPointer<SupervisedClassifier> selectedClassifier = QSharedPointer<SupervisedClassifier>(classifierList->returnCopyPointer(classifierName));
			if (selectedClassifier->deserialize(classifierSettings)) {

				d->classify->setEnabled(true);

				ClassifierWidget* cWidget = new ClassifierWidget(selectedClassifier, this);

				classifierWidgets.append(cWidget);

				connect(cWidget, SIGNAL(clearRequest()), this, SLOT(classifierWidgetClearRequested()));

				d->groupBox_2->layout()->addWidget(classifierWidgets.back());
				d->groupBox_2->update();

				if (classifierWidgets.count() > 1)
					d->combination->show();
			}
			break;
		}
	}
}

void SegmentationToolboxClassifyingWidget::classifierWidgetClearRequested()
{
	ClassifierWidget* cWidget = qobject_cast<ClassifierWidget*>(sender());

	if (cWidget == nullptr)
		return;

	Q_D(SegmentationToolboxClassifyingWidget);

	d->groupBox_2->layout()->removeWidget(cWidget);
	d->groupBox_2->update();

	classifierWidgets.remove(classifierWidgets.indexOf(cWidget));

	delete cWidget;

	if (classifierWidgets.isEmpty())
		d->classify->setEnabled(false);

	if (classifierWidgets.count() <= 1) {
		d->combination->setCurrentIndex(0);
		d->combination->hide();
	}
}