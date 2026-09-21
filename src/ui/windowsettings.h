#ifndef WINDOWSETTINGS_H
#define WINDOWSETTINGS_H

#include <QDialog>
#include <QEvent>
#include <QMap>
#include <QVector>
#include <autogensettings.h>

class QTreeWidgetItem;
class QCloseEvent;
class QTimer;

namespace Ui {
class windowsettings;
}

class windowsettings : public QDialog
{
    Q_OBJECT
public:
    explicit windowsettings(const QString &language, QWidget *parent = nullptr);
    ~windowsettings() override;

    AutoGenSettings settings() const;
    void setProgress(int done, int total);
    void setBusy(bool busy);
    void markFinished(bool ok);
    void setLastOutputPath(const QString &path);

signals:
    void runRequested();
    void stopRequested();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void applyDialogStyle();
    void buildFlagTree();
    void applySettings(const AutoGenSettings &settings);
    AutoGenSettings collectSettings() const;
    void fillDefaultPaths();
    void updateWindowTitle();
    void filterFlags(const QString &query);
    void invertFlags();
    void askCollageFile();
    void askImagesDir();
    void loadPreset();
    void savePreset();
    void openOutputFolder();
    void onRun();
    void onStop();
    void onClose();
    void saveSession() const;
    void restoreSession();
    void loadProgressMessages();
    void showRandomProgressCaption();
    QMap<QString, bool> leafFlags() const;
    void setLeafFlags(const QMap<QString, bool> &flags);
    QTreeWidgetItem *makeGroup(QTreeWidgetItem *parent, const QString &title,
                               const QStringList &keys, const QStringList &labels);

    Ui::windowsettings *ui;
    QString language;
    QString presetName;
    QString lastOutputPath;
    QStringList flagKeys;
    QStringList flagLabels;
    QVector<QTreeWidgetItem *> leafItems;
    QTreeWidgetItem *rootItem = nullptr;
    QStringList progressMessages;
    QString lastProgressCaption;
    QTimer *progressCaptionTimer = nullptr;
    bool busy = false;
    AutoGenSettings resultSettings;
};

#endif
