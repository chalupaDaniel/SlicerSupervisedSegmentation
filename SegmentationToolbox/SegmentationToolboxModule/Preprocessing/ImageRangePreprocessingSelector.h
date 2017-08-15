#ifndef IMAGE_RANGE_PREPROCESSING_SELECTOR_H
#define IMAGE_RANGE_PREPROCESSING_SELECTOR_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QStringList>

#include "PreprocessingAlgorithmList.h"

class QPushButton;
class QComboBox;
class VolumeSelectorDialog;

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// A class representing single volume selection and preprocessing line
/// Enables the user to select multiple volumes that should be preprocessed together and
/// used as a single dataset (i.e. all T1-weighted MRI images)
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class SelectorSingleLine : public QWidget
{
	Q_OBJECT
public:
	SelectorSingleLine(int id, QWidget* parent = nullptr);
	~SelectorSingleLine();

	void setAlgorithm(const QString& name, const QByteArray& settings);
	void removeLine();

	int id;
	QStringList getImagePaths() const { return imageRangePaths; }
	QSharedPointer<PreprocessingAlgorithm> getSelectedAlgorithm() const;

signals:
	void aboutToBeDestroyed(int id);
	void previewRequested(QSharedPointer<PreprocessingAlgorithm> preprocessingAlgorithm, const QString& pathName);

private slots:
	void removeClicked();
	void volumesSelected(const QStringList& volumes);
	void editClicked();
	void comboboxChanged();
	void previewClicked();

private:
	QPushButton* imageRange;
	QComboBox* preprocessing;
	QPushButton* remove;
	QPushButton* edit; 
	QStringList imageRangePaths;
	QPushButton* preview;
	PreprocessingAlgorithmList algorithms;
	VolumeSelectorDialog* volumeSelectorDialog;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widget managing SelectorSingleLines and makes them accessible in a single interface
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class ImageRangePreprocessingSelector : public QWidget
{
	Q_OBJECT
public:
	ImageRangePreprocessingSelector(QWidget* parent = nullptr);
	// Every vector entry contains filenames for single line
	QVector<QStringList> allFilenames() const;
	// Returns all of the selected algorithms, None entries are listed as shared pointers to null
	QVector<QSharedPointer<PreprocessingAlgorithm>> algorithms() const;

public slots:
	void addRow();
	void clear();
	void addRowAndSetAlgorithm(const QString& name, const QByteArray& settings);

signals:
	void previewRequested(QSharedPointer<PreprocessingAlgorithm> preprocessingAlgorithm, const QString& pathName);

private slots:
	void rowRemoved(int id);

private:
	QList<SelectorSingleLine*> lines;

	int idCounter;
};

#endif // IMAGE_RANGE_PREPROCESSING_SELECTOR_H