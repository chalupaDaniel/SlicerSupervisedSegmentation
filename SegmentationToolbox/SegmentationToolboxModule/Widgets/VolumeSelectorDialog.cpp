#include "VolumeSelectorDialog.h"

#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QShowEvent>
#include <QFile>

#include "qSlicerCoreApplication.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkCollection.h"


VolumeSelectorDialog::VolumeSelectorDialog(QWidget* parent) 
	: QDialog(parent)
{
	createUi();


	connect(ok, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(loadExternal, SIGNAL(clicked()), this, SLOT(loadExternalClicked()));
}

void VolumeSelectorDialog::createUi() 
{
	ok = new QPushButton("Select", this);
	cancel = new QPushButton("Cancel", this);
	currentVolumesView = new QListWidget(this);
	loadExternal = new QPushButton("Load File(s)");

	QVBoxLayout* mainLayout = new QVBoxLayout();

	mainLayout->addWidget(currentVolumesView);
	mainLayout->addWidget(loadExternal);

	QHBoxLayout* bottomButtonsLayout = new QHBoxLayout();
	bottomButtonsLayout->addWidget(ok);
	bottomButtonsLayout->addWidget(cancel);

	mainLayout->addLayout(bottomButtonsLayout);

	setLayout(mainLayout);
	setWindowTitle("Select Volumes");
}

void VolumeSelectorDialog::addMissingVolumes()
{

	// Go through all nodes in scene and if their model counterpart is not found, add them
	vtkCollection* col = qSlicerCoreApplication::application()->mrmlScene()->GetNodesByClass("vtkMRMLScalarVolumeNode");
	for (size_t i = 0; i < col->GetNumberOfItems(); i++) {
		vtkSmartPointer<vtkMRMLNode> singleNode = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(i));

		bool found = false;
		for (size_t i = 0; i < currentVolumesView->count(); i++) {
			QListWidgetItem* item = currentVolumesView->item(i);

			if (QString(singleNode->GetName()) == item->data(Qt::DisplayRole).toString()) {
				found = true;
				break;
			}
		}

		if (!found) {
			QListWidgetItem* newItem = new QListWidgetItem;
			newItem->setData(Qt::DisplayRole, QString(singleNode->GetName()));
			newItem->setData(Qt::CheckStateRole, Qt::Unchecked);
			currentVolumesView->addItem(newItem);
		}
	}
	col->Delete();
}

void VolumeSelectorDialog::clearRedundantVolumes() 
{
	vtkCollection* col = qSlicerCoreApplication::application()->mrmlScene()->GetNodesByClass("vtkMRMLScalarVolumeNode");

	for (size_t i = 0; i < currentVolumesView->count(); i++) {
		QListWidgetItem* item = currentVolumesView->item(i);

		bool valid;
		bool external = isVolumeExternal(item->data(Qt::DisplayRole).toString(), valid);

		if (!valid) {
			currentVolumesView->removeItemWidget(item);
			i--;
			continue;
		}

		if (item->checkState() != Qt::Checked && external) {
			currentVolumesView->removeItemWidget(item);
			i--;
		}
	}
	col->Delete();
}

void VolumeSelectorDialog::reset() 
{
	clearRedundantVolumes();
	addMissingVolumes();
}


void VolumeSelectorDialog::loadExternalClicked()
{
	QStringList imageRangePaths = QFileDialog::getOpenFileNames(this, "Select images");
	
	for (const QString& image : imageRangePaths) {
		QListWidgetItem *item = new QListWidgetItem;
		item->setData(Qt::DisplayRole, image);
		item->setData(Qt::CheckStateRole, Qt::Unchecked);
		currentVolumesView->addItem(item);
	}
}
void VolumeSelectorDialog::okClicked()
{
	QStringList volumes = getCheckedVolumes();

	emit volumesSelected(volumes);
	done(volumes.count());
}
void VolumeSelectorDialog::cancelClicked() 
{
	rejected();
	done(0);
}
void VolumeSelectorDialog::showEvent(QShowEvent* event)
{
	reset();
	event->accept();
}
QStringList VolumeSelectorDialog::getCheckedVolumes() const
{
	QStringList volumes;
	for (size_t i = 0; i < currentVolumesView->count(); i++) {
		QListWidgetItem* item = currentVolumesView->item(i);
		if (item->checkState() == Qt::Checked) {
			QString file = item->data(Qt::DisplayRole).toString();

			bool valid;
			isVolumeExternal(file, valid);

			if (valid) {
				volumes.append(file);
			} 
		}
	}

	return volumes;
}

bool VolumeSelectorDialog::isVolumeExternal(const QString& apparentFilename, bool& isValid) 
{
	isValid = false;

	// Look in scene
	vtkCollection* col = qSlicerCoreApplication::application()->mrmlScene()->GetNodesByClass("vtkMRMLScalarVolumeNode");
	for (size_t i = 0; i < col->GetNumberOfItems(); i++) {
		vtkSmartPointer<vtkMRMLNode> singleNode = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(i));

		if (QString(singleNode->GetName()) == apparentFilename) {
			col->Delete();
			isValid = true;
			return false;
		}
	}

	if (QFile::exists(apparentFilename)) {
		col->Delete();
		isValid = true;
		return true;
	}

	col->Delete();
	return false;
}