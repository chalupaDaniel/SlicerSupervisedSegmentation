#include "PreprocessingGaussFilter.h"

#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMutex>
#include <QMutexLocker>

#include "vtkImageGaussianSmooth.h"

#include "vtkMRMLNode.h"
#include "vtkMRMLVolumeNode.h"

PreprocessingGaussFilter::PreprocessingGaussFilter()
	: PreprocessingAlgorithm()
{
	algorithmName = "Gaussian Filter";
	radius = 3.;
	std = 3.;

	dialog = new QWidget();
	QVBoxLayout* mainLayout = new QVBoxLayout();
	QHBoxLayout* bottomLayout = new QHBoxLayout();

	mainLayout->addWidget(new QLabel("Radius:"));
	radiusEdit = new QLineEdit(QString::number(radius));
	mainLayout->addWidget(radiusEdit);

	mainLayout->addWidget(new QLabel("Standard deviation:"));
	stdEdit = new QLineEdit(QString::number(std));
	mainLayout->addWidget(stdEdit);

	QPushButton* okButton = new QPushButton("OK");
	QPushButton* cancelButton = new QPushButton("Cancel");
	bottomLayout->addWidget(okButton);
	bottomLayout->addWidget(cancelButton);

	mainLayout->addLayout(bottomLayout);

	dialog->setLayout(mainLayout);
	dialog->hide();
	mutex = new QMutex();

	connect(okButton, SIGNAL(clicked()), this, SLOT(setMaskSize()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelled()));
}

PreprocessingGaussFilter::~PreprocessingGaussFilter()
{
}

void PreprocessingGaussFilter::setMaskSize()
{
	//QMutexLocker lock(mutex);
	radius = radiusEdit->text().toDouble();
	std = stdEdit->text().toDouble();
	dialog->hide();
}

void PreprocessingGaussFilter::cancelled()
{
	radiusEdit->setText(QString::number(radius));
	stdEdit->setText(QString::number(std));
	dialog->hide();
}


void PreprocessingGaussFilter::preprocess(vtkSmartPointer<vtkMRMLNode> inputNode, vtkSmartPointer<vtkMRMLNode> outputNode, int referenceId)
{
	QMutexLocker lock(mutex);
	vtkSmartPointer<vtkImageGaussianSmooth> gaussianFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
	vtkSmartPointer<vtkMRMLVolumeNode> inputVolume = vtkMRMLVolumeNode::SafeDownCast(inputNode);

	vtkSmartPointer<vtkMRMLVolumeNode> outputVolume = vtkMRMLVolumeNode::SafeDownCast(outputNode);

	gaussianFilter->SetStandardDeviation(std);
	gaussianFilter->SetRadiusFactor(radius);
	gaussianFilter->SetInputConnection(inputVolume->GetImageDataConnection());

	gaussianFilter->Update();

	int state = outputNode->StartModify();
	outputVolume->SetImageDataConnection(gaussianFilter->GetOutputPort());
	outputNode->EndModify(state);

	emit preprocessingDone(referenceId);
}

#include <QDebug>
QByteArray PreprocessingGaussFilter::serialize() const
{
	QByteArray out;
	out.append("Radius=");
	out.append(QByteArray::number(radius));
	out.append("\n");
	out.append("Std=");
	out.append(QByteArray::number(std));
	qDebug() << out;
	return out;
}

bool PreprocessingGaussFilter::deserialize(const QByteArray& byteArray)
{
	qDebug() << byteArray;
	QList<QByteArray> settings = byteArray.split('\n');

	if (settings.count() < 3)
		return false;
	for (QByteArray& setting : settings) {
		QList<QByteArray> split = setting.split('=');
		if (split.count() < 2)
			return false;
		if (split.at(0) == "Radius") {
			radius = split.at(1).toDouble();
		}
		else if (split.at(0) == "Std") {
			std = split.at(1).toDouble();
		}
	}

	cancelled();
	return true;
}
