#ifndef CLASSIFIER_WIDGET_H
#define CLASSIFIER_WIDGET_H

#include <QWidget>
#include <QSharedPointer>

class QComboBox;
class SupervisedClassifier;
class ClassifierWidget : public QWidget
{
	Q_OBJECT
public:
	enum CombinationType {
		LogicalAnd,
		LogicalOr,
		StatisticalMode,
	};

	ClassifierWidget(QSharedPointer<SupervisedClassifier> classifier, QWidget* parent = nullptr);
	QSharedPointer<SupervisedClassifier> classifier() const;

signals:
	void clearRequest();

private slots:
	void clearClicked();

private:
	QSharedPointer<SupervisedClassifier> loadedClassifier;
};

#endif // CLASSIFIER_WIDGET_H