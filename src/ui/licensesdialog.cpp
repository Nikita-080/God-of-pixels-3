#include "licensesdialog.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QPlainTextEdit>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {

QPlainTextEdit *makeTab(const QString &resourcePath)
{
    auto *edit = new QPlainTextEdit;
    edit->setReadOnly(true);
    edit->setLineWrapMode(QPlainTextEdit::NoWrap);
    QFile file(resourcePath);
    if (file.open(QFile::ReadOnly | QFile::Text))
        edit->setPlainText(QString::fromUtf8(file.readAll()));
    return edit;
}

} // namespace

LicensesDialog::LicensesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Licenses"));
    resize(720, 520);

    auto *tabs = new QTabWidget;
    tabs->addTab(makeTab(QStringLiteral(":/licenses/NOTICE.txt")), tr("Notice"));
    tabs->addTab(makeTab(QStringLiteral(":/licenses/MIT.txt")), tr("MIT"));
    tabs->addTab(makeTab(QStringLiteral(":/licenses/LGPL-3.0.txt")), tr("LGPLv3"));
    tabs->addTab(makeTab(QStringLiteral(":/licenses/GPL-3.0.txt")), tr("GPLv3"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->addWidget(buttons);
}
