#include "vtkMRMLVolumeNode.h"

#include "preprocessingAlgorithm.h"
#include <QDebug>

PreprocessingAlgorithm::~PreprocessingAlgorithm()
{
	if (dialog != nullptr){
		dialog->deleteLater();
		dialog = nullptr;
	}
}

void PreprocessingAlgorithm::showDialog()
{
	if (dialog != nullptr) {
		dialog->show();
	}
}


bool PreprocessingAlgorithm::hasDialog() const
{
	return dialog != nullptr;
}