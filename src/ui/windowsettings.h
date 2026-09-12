#ifndef WINDOWSETTINGS_H
#define WINDOWSETTINGS_H

#include <QDialog>
#include <QCheckBox>
#include <QVector>
#include <autogensettings.h>

namespace Ui {
class windowsettings;
}

class windowsettings : public QDialog
{
    Q_OBJECT
public:
    explicit windowsettings(const QString &language, QWidget *parent = nullptr);
    ~windowsettings();
    AutoGenSettings settings() const;

    void ChangeType();
    void TakeAll();
    void TakeNothing();
    void AskFile();
    void AskDir();
    void EndWindow();
    void ButtonCancel();

private:
    Ui::windowsettings *ui;
    QVector<QCheckBox *> rndarr;
    QString filepath;
    QString dirpath;
    AutoGenSettings resultSettings;
};

#endif
