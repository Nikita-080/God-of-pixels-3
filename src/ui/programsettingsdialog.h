#ifndef PROGRAMSETTINGSDIALOG_H
#define PROGRAMSETTINGSDIALOG_H

#include <QDialog>
#include <QString>

class QComboBox;
class QCheckBox;

class ProgramSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    ProgramSettingsDialog(const QString &language, bool livePreview, QWidget *parent = nullptr);

    QString language() const;
    bool livePreview() const;

private:
    void loadDefaults();
    void loadValues(const QString &language, bool livePreview);

    QComboBox *comboLanguage;
    QCheckBox *checkLive;
};

#endif
