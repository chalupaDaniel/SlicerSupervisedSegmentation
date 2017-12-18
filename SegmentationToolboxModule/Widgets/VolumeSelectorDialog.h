#ifndef VOLUME_SELECTOR_DIALOG_H
#define VOLUME_SELECTOR_DIALOG_H

#include <QDialog>

class QPushButton;
class QListWidget;
class QShowEvent;
class QStringList;
class VolumeSelectorDialog : public QDialog
{
	Q_OBJECT
public:
	VolumeSelectorDialog(QWidget* parent = nullptr);

	static bool isVolumeExternal(const QString& apparentFilename, bool& isValid);

signals:
	void volumesSelected(const QStringList&);

private slots:
	void showEvent(QShowEvent* event) override;
	void loadExternalClicked();
	void okClicked();
	void cancelClicked();

private:

	QStringList getCheckedVolumes() const;
	void createUi();
	void addMissingVolumes();
	// Clears external volumes that are not selected and internal volumes that dont exist anymore
	void clearRedundantVolumes();
	void reset();

	QPushButton* ok;
	QPushButton* cancel;
	QPushButton* loadExternal;
	QListWidget* currentVolumesView;
};


#endif // VOLUME_SELECTOR_DIALOG_H