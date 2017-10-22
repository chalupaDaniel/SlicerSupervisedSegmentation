#include "PreprocessingMedian3d.h"

#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMutex>
#include <QMutexLocker>

#include "vtkImageMedian3D.h"

#include "vtkMRMLNode.h"
#include "vtkMRMLVolumeNode.h"

PreprocessingMedian3d::PreprocessingMedian3d()
	: PreprocessingAlgorithm()
{
	algorithmName = "3D Median Filter";
	maskSizeX = 2;
	maskSizeY = 2;
	maskSizeZ = 2;

	dialog = new QWidget();
	QVBoxLayout* mainLayout = new QVBoxLayout();
	QHBoxLayout* bottomLayout = new QHBoxLayout();

	mainLayout->addWidget(new QLabel("Mask size lateral:"));
	lineEditX = new QLineEdit(QString::number(maskSizeX));
	lineEditX->setInputMask("009");
	mainLayout->addWidget(lineEditX);

	mainLayout->addWidget(new QLabel("Mask size anterior:"));
	lineEditY = new QLineEdit(QString::number(maskSizeY));
	lineEditY->setInputMask("009");
	mainLayout->addWidget(lineEditY);

	mainLayout->addWidget(new QLabel("Mask size superior:"));
	lineEditZ = new QLineEdit(QString::number(maskSizeZ));
	lineEditZ->setInputMask("009");
	mainLayout->addWidget(lineEditZ);

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

PreprocessingMedian3d::~PreprocessingMedian3d()
{
}

void PreprocessingMedian3d::setMaskSize()
{
	//QMutexLocker lock(mutex);
	maskSizeX = lineEditX->text().toInt();
	maskSizeY = lineEditY->text().toInt();
	maskSizeZ = lineEditZ->text().toInt();
	dialog->hide();
}

void PreprocessingMedian3d::cancelled()
{
	lineEditX->setText(QString::number(maskSizeX));
	lineEditY->setText(QString::number(maskSizeY));
	lineEditZ->setText(QString::number(maskSizeZ));
	dialog->hide();
}


void PreprocessingMedian3d::preprocess(vtkSmartPointer<vtkMRMLNode> inputNode, vtkSmartPointer<vtkMRMLNode> outputNode, int referenceId)
{
	QMutexLocker lock(mutex);
	vtkSmartPointer<vtkImageMedian3D> medianFilter = vtkSmartPointer<vtkImageMedian3D>::New();
	vtkSmartPointer<vtkMRMLVolumeNode> inputVolume = vtkMRMLVolumeNode::SafeDownCast(inputNode);

	vtkSmartPointer<vtkMRMLVolumeNode> outputVolume = vtkMRMLVolumeNode::SafeDownCast(outputNode);

	medianFilter->SetKernelSize(maskSizeX, maskSizeY, maskSizeZ);
	medianFilter->SetInputConnection(inputVolume->GetImageDataConnection());

	medianFilter->Update();

	int state = outputNode->StartModify();
	outputVolume->SetImageDataConnection(medianFilter->GetOutputPort());
	outputNode->EndModify(state);

	emit preprocessingDone(referenceId);
}

QByteArray PreprocessingMedian3d::serialize() const
{
	QByteArray out;
	out.append("maskSizeX=");
	out.append(QByteArray::number(maskSizeX));
	out.append("\n");
	out.append("maskSizeY=");
	out.append(QByteArray::number(maskSizeY));
	out.append("\n");
	out.append("maskSizeZ=");
	out.append(QByteArray::number(maskSizeZ));
	return out;
}

bool PreprocessingMedian3d::deserialize(const QByteArray& byteArray)
{
	QList<QByteArray> settings = byteArray.split('\n');

	if (settings.count() < 3)
		return false;
	for (QByteArray& setting : settings) {
		QList<QByteArray> split = setting.split('=');
		if (split.count() < 2)
			return false;
		if (split.at(0) == "maskSizeX") {
			maskSizeX = split.at(1).toInt();
		}
		else if (split.at(0) == "maskSizeY") {
			maskSizeY = split.at(1).toInt();
		}
		else if (split.at(0) == "maskSizeZ") {
			maskSizeZ = split.at(1).toInt();
		}
	}
	cancelled();
	return true;
}
