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