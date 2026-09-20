#include "programsettingsdialog.h"
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ProgramSettingsDialog::ProgramSettingsDialog(const QString &language, bool livePreview, QWidget *parent)
    : QDialog(parent)
    , comboLanguage(nullptr)
    , checkLive(nullptr)
{
    setObjectName(QStringLiteral("programSettingsDialog"));
    setWindowTitle(tr("Program settings"));
    setModal(true);
    setMinimumWidth(360);

    auto *languageLabel = new QLabel(tr("Language"));
    comboLanguage = new QComboBox;
    comboLanguage->setObjectName(QStringLiteral("comboProgramLanguage"));
    comboLanguage->addItem(QStringLiteral("English"), QStringLiteral("en"));
    comboLanguage->addItem(QString::fromUtf8("Русский"), QStringLiteral("ru"));

    checkLive = new QCheckBox(tr("Quick update"));
    checkLive->setObjectName(QStringLiteral("checkProgramLive"));

    auto *btnReset = new QPushButton(tr("Reset settings"));
    btnReset->setObjectName(QStringLiteral("btnProgramReset"));
    auto *btnApply = new QPushButton(tr("Apply"));
    btnApply->setObjectName(QStringLiteral("btnProgramApply"));
    auto *btnCancel = new QPushButton(tr("Cancel"));
    btnCancel->setObjectName(QStringLiteral("btnProgramCancel"));

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    buttons->addWidget(btnReset);
    buttons->addStretch(1);
    buttons->addWidget(btnApply);
    buttons->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addWidget(languageLabel);
    layout->addWidget(comboLanguage);
    layout->addWidget(checkLive);
    layout->addStretch(1);
    layout->addLayout(buttons);

    loadValues(language, livePreview);

    connect(btnReset, &QPushButton::clicked, this, &ProgramSettingsDialog::loadDefaults);
    connect(btnApply, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void ProgramSettingsDialog::loadDefaults()
{
    loadValues(QStringLiteral("en"), false);
}

void ProgramSettingsDialog::loadValues(const QString &language, bool livePreview)
{
    const int index = comboLanguage->findData(language == QStringLiteral("ru")
                                                  ? QStringLiteral("ru")
                                                  : QStringLiteral("en"));
    comboLanguage->setCurrentIndex(qMax(0, index));
    checkLive->setChecked(livePreview);
}

QString ProgramSettingsDialog::language() const
{
    return comboLanguage->currentData().toString();
}

bool ProgramSettingsDialog::livePreview() const
{
    return checkLive->isChecked();
}
