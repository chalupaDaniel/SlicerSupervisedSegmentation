#include "ClassifierList.h"

#include "SupervisedClassifier.h"
#include "ClassifierDlibSvmNu.h"
#include "ClassifierDlibSvmC.h"
#include "ClassifierSharkSvmC.h"
#include "ClassifierSharkRandomForest.h"
ClassifierList::ClassifierList()
{
	classifiers.append(QSharedPointer<ClassifierDlibSvmNuNamespace::ClassifierDlibSvmNu>(new ClassifierDlibSvmNuNamespace::ClassifierDlibSvmNu()));
	classifiers.append(QSharedPointer<ClassifierDlibSvmCNamespace::ClassifierDlibSvmC>(new ClassifierDlibSvmCNamespace::ClassifierDlibSvmC()));
	classifiers.append(QSharedPointer<ClassifierSharkSvmCNamespace::ClassifierSharkSvmC>(new ClassifierSharkSvmCNamespace::ClassifierSharkSvmC()));
	classifiers.append(QSharedPointer<ClassifierSharkRandomForestNamespace::ClassifierSharkRandomForest>(new ClassifierSharkRandomForestNamespace::ClassifierSharkRandomForest()));
}
ClassifierList::~ClassifierList()
{
}

SupervisedClassifier* ClassifierList::returnCopyPointer(const QString& classifierName)
{
	if (classifierName == "Nu Support Vector Machine - Dlib") {
		return new ClassifierDlibSvmNuNamespace::ClassifierDlibSvmNu();
	} else if (classifierName == "C Support Vector Machine - Dlib") {
		return new ClassifierDlibSvmCNamespace::ClassifierDlibSvmC();
	} else if (classifierName == "C Support Vector Machine - Shark") {
		return new ClassifierSharkSvmCNamespace::ClassifierSharkSvmC();
	} else if (classifierName == "Random Forest - Shark") {
		return new ClassifierSharkRandomForestNamespace::ClassifierSharkRandomForest();
	} else
		return nullptr;
}