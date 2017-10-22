#include "ClassifierWidget.h"
#include "SupervisedClassifier.h"

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

ClassifierWidget::ClassifierWidget(QSharedPointer<SupervisedClassifier> classifier, QWidget* parent)
	: QWidget(parent), loadedClassifier(classifier)
{
	loadedClassifier = classifier;

	QVBoxLayout* mainLayout = new QVBoxLayout;

	QLabel* name = new QLabel(classifier->name());
	mainLayout->addWidget(name);

	mainLayout->addWidget(classifier->classifyingWidget());

	QPushButton* clear = new QPushButton("Close classifier");
	mainLayout->addWidget(clear);
	connect(clear, SIGNAL(clicked()), this, SLOT(clearClicked()));
	
	setLayout(mainLayout);
}

QSharedPointer<SupervisedClassifier> ClassifierWidget::classifier() const
{
	return loadedClassifier;
}

void ClassifierWidget::clearClicked()
{
	emit clearRequest();
}