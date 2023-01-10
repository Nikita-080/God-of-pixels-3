#ifndef WINDOWSETTINGS_H
#define WINDOWSETTINGS_H

#include <QDialog>
#include <QCheckBox>
#include <QVector>
#include <mainwindow.h>
namespace Ui {
class windowsettings;
}

class windowsettings : public QDialog
{
    Q_OBJECT
public:
    QVector <QCheckBox*> rndarr;
    QString filepath;
    QString dirpath;
    QString currentpath;
    MainWindow* myparent;
private:
    int numsettings;
public:
    explicit windowsettings(QString,QWidget *parent = nullptr);
    ~windowsettings();
    void ChangeType();
    void TakeAll();
    void TakeNothing();
    void AskFile();
    void AskDir();
    void EndWindow();
    void ButtonCancel();
private:
    Ui::windowsettings *ui;
};

#endif // WINDOWSETTINGS_H
