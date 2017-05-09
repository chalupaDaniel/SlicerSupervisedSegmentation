#include "ImageRangePreprocessingSelector.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QFileDialog>

#include "PreprocessingAlgorithm.h"

#include <QDebug>

SelectorSingleLine::SelectorSingleLine(int id, QWidget* parent)
	: QWidget(parent), id(id)
{
	QHBoxLayout* mainLayout = new QHBoxLayout();

	imageRange = new QPushButton("Select Images");
	preprocessing = new QComboBox();
	remove = new QPushButton("X");
	edit = new QPushButton("Edit");
	preview = new QPushButton("Prev");

	preprocessing->addItem("None", -1);

	remove->setMaximumWidth(32);

	edit->setMaximumWidth(64);
	edit->setEnabled(false);

	preview->setMaximumWidth(64);

	// [Select Images][Preprocessor combobox][Edit preprocessor][Preview volume][Remove line]

	mainLayout->addWidget(imageRange);
	mainLayout->addWidget(preprocessing);
	mainLayout->addWidget(edit);
	mainLayout->addWidget(preview);
	mainLayout->addWidget(remove);

	setLayout(mainLayout);

	connect(imageRange, SIGNAL(clicked()), this, SLOT(imageRangeClicked()));

	connect(remove, SIGNAL(clicked()), this, SLOT(removeClicked()));
	connect(remove, SIGNAL(clicked()), this, SLOT(deleteLater()));

	connect(edit, SIGNAL(clicked()), this, SLOT(editClicked()));

	connect(preview, SIGNAL(clicked()), this, SLOT(previewClicked()));

	for (size_t i = 0; i < algorithms.algorithms.count(); i++) {
		preprocessing->addItem(algorithms.algorithms.at(i)->name(), i);
	}

	connect(preprocessing, SIGNAL(currentIndexChanged(int)), this, SLOT(comboboxChanged()));
}

SelectorSingleLine::~SelectorSingleLine()
{
}

void SelectorSingleLine::setAlgorithm(const QString& name, const QByteArray& settings)
{
	if (name == "None") {
		preprocessing->setCurrentIndex(0);
		return;
	}
	for (size_t i = 0; i < algorithms.algorithms.count(); i++) {
		if (algorithms.algorithms.at(i)->name() == name) {
			algorithms.algorithms[i]->deserialize(settings);
			// + 1 for "None"
			preprocessing->setCurrentIndex(i + 1);
			return;
		}
	}
}

void SelectorSingleLine::removeLine()
{
	removeClicked();
}

void SelectorSingleLine::removeClicked()
{
	emit aboutToBeDestroyed(id);
}

void SelectorSingleLine::previewClicked()
{
	int index = preprocessing->itemData(preprocessing->currentIndex()).toInt();

	if (index == -1 || index >= algorithms.algorithms.count() || imageRangePaths.isEmpty())
		return;

	emit previewRequested(algorithms.algorithms.at(index), imageRangePaths.at(0));
}

QSharedPointer<PreprocessingAlgorithm> SelectorSingleLine::getSelectedAlgorithm() const
{
	int index = preprocessing->itemData(preprocessing->currentIndex()).toInt();

	if (index == -1 || index >= algorithms.algorithms.count() || imageRangePaths.isEmpty())
		return QSharedPointer<PreprocessingAlgorithm>();

	return algorithms.algorithms.at(index);
}

void SelectorSingleLine::imageRangeClicked()
{
	//imageRangePaths = QFileDialog::getOpenFileNames(this, "Select images", "", "DICOM files (*.dicom)");
	imageRangePaths = QFileDialog::getOpenFileNames(this, "Select images");

	if (imageRangePaths.count())
		imageRange->setText(QString::number(imageRangePaths.count()) + " file"
		+ (imageRangePaths.count() > 1 ? "s" : QString())
		+ " selected");
	else
		imageRange->setText("Select Images");		
}

void SelectorSingleLine::editClicked()
{
	int index = preprocessing->itemData(preprocessing->currentIndex()).toInt();
	algorithms.algorithms[index]->showDialog();
}

void SelectorSingleLine::comboboxChanged()
{
	int index = preprocessing->itemData(preprocessing->currentIndex()).toInt();

	if (index != -1 && algorithms.algorithms.at(index)->hasDialog())
		edit->setEnabled(true);
	else
		edit->setEnabled(false);
}

ImageRangePreprocessingSelector::ImageRangePreprocessingSelector(QWidget* parent)
	: QWidget(parent), idCounter(0)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	addRow();
}

void ImageRangePreprocessingSelector::addRow()
{
	idCounter++;
	SelectorSingleLine* newLine = new SelectorSingleLine(idCounter, this);

	connect(newLine, SIGNAL(aboutToBeDestroyed(int)), this, SLOT(rowRemoved(int)));
	connect(newLine, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)),
		this, SIGNAL(previewRequested(QSharedPointer<PreprocessingAlgorithm>, const QString&)));

	lines.append(newLine);

	layout()->addWidget(newLine);
}

void ImageRangePreprocessingSelector::rowRemoved(int id)
{
	for (size_t i = 0; i < lines.count(); i++)
	{
		if (lines.at(i)->id == id) {
			delete lines.takeAt(i);
			return;
		}
	}
}

QVector<QStringList> ImageRangePreprocessingSelector::allFilenames() const
{
	QVector<QStringList> out;
	for (size_t i = 0; i < lines.count(); i++)
	{
		out.append(lines.at(i)->getImagePaths());
	}
	return out;
}

QVector<QSharedPointer<PreprocessingAlgorithm>> ImageRangePreprocessingSelector::algorithms() const
{
	QVector<QSharedPointer<PreprocessingAlgorithm>> out;
	for (size_t i = 0; i < lines.count(); i++)
	{
		out.append(lines.at(i)->getSelectedAlgorithm());
	}
	return out;
}

void ImageRangePreprocessingSelector::clear()
{
	for (size_t i = 0; i < lines.count(); i++)
	{
		lines[i]->removeLine();
		i--;
	}
}

void ImageRangePreprocessingSelector::addRowAndSetAlgorithm(const QString& name, const QByteArray& settings)
{
	addRow();
	lines[lines.count() - 1]->setAlgorithm(name, settings);
}
