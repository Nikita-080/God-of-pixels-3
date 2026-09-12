#include "windowsettings.h"
#include "ui_windowsettings.h"
#include <QFile>
#include <QCheckBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QRegExp>
#include "appsettings.h"

windowsettings::windowsettings(const QString &language, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::windowsettings)
{
    ui->setupUi(this);
    setWindowTitle(tr("Autogen"));
    setStyleSheet(QStringLiteral(
        "#groupBox:disabled, #groupBox_2:disabled {"
        "  background-color: rgb(10, 10, 10);"
        "}"
        "#groupBox QLabel, #groupBox_2 QLabel, QLabel#label_4 {"
        "  color: rgb(110, 170, 200);"
        "}"
        "#groupBox:disabled QLabel, #groupBox_2:disabled QLabel,"
        "#groupBox QLabel:disabled, #groupBox_2 QLabel:disabled {"
        "  color: rgb(55, 75, 85);"
        "}"
        "QLineEdit {"
        "  color: rgb(0, 255, 127);"
        "  background-color: rgb(0, 0, 0);"
        "  border-width: 3px;"
        "  border-style: solid;"
        "  border-color: rgb(110, 170, 200);"
        "}"
        "QLineEdit:disabled {"
        "  color: rgb(80, 100, 90);"
        "  background-color: rgb(16, 16, 16);"
        "  border-color: rgb(45, 65, 75);"
        "}"
        "QPushButton {"
        "  color: rgb(110, 170, 200);"
        "  background-color: rgb(0, 0, 0);"
        "  border-width: 3px;"
        "  border-style: solid;"
        "  border-color: rgb(110, 170, 200);"
        "}"
        "QPushButton:disabled {"
        "  color: rgb(55, 75, 85);"
        "  background-color: rgb(12, 12, 12);"
        "  border-color: rgb(45, 65, 75);"
        "}"
        "QRadioButton { color: rgb(110, 170, 200); }"
        "QCheckBox { color: rgb(110, 170, 200); }"));

    for (QLineEdit *edit : findChildren<QLineEdit *>())
        edit->setStyleSheet(QString());
    for (QPushButton *btn : findChildren<QPushButton *>())
        btn->setStyleSheet(QString());

    QFormLayout *formLayout = new QFormLayout();
    QWidget *flagsHost = new QWidget;

    const QString path = (language == "ru")
                             ? QString(":/txt_files/res/txt_files/randomsettings_ru.txt")
                             : QString(":/txt_files/res/txt_files/randomsettings_en.txt");
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QByteArray bytes = file.readAll();
    file.close();
    if (bytes.startsWith("\xEF\xBB\xBF"))
        bytes = bytes.mid(3);
    const QStringList labels = QString::fromUtf8(bytes).split(QRegExp("\\r?\\n"), Qt::SkipEmptyParts);
    const QColor accent(110, 170, 200);
    QPalette palette;
    palette.setColor(QPalette::WindowText, accent);
    palette.setColor(QPalette::ButtonText, accent);
    palette.setColor(QPalette::Text, accent);
    const QFont checkFont(QStringLiteral("Consolas"), 12);
    const QString checkQss = QStringLiteral(
        "QCheckBox { color: rgb(110, 170, 200); background-color: transparent; }");
    flagsHost->setStyleSheet(checkQss);

    for (QString line : labels)
    {
        line = line.trimmed();
        if (line.isEmpty())
            continue;
        QCheckBox *box = new QCheckBox(line);
        box->setFont(checkFont);
        box->setPalette(palette);
        box->setStyleSheet(checkQss);
        formLayout->addRow(box);
        rndarr.append(box);
    }

    flagsHost->setLayout(formLayout);
    ui->scrollArea->setWidget(flagsHost);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->radioButton->setChecked(true);
    ui->groupBox_2->setEnabled(false);

    connect(ui->radioButton, &QRadioButton::toggled, this, &windowsettings::ChangeType);
    connect(ui->radioButton_2, &QRadioButton::toggled, this, &windowsettings::ChangeType);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &windowsettings::TakeAll);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &windowsettings::TakeNothing);
    connect(ui->pushButton, &QPushButton::clicked, this, &windowsettings::AskFile);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &windowsettings::AskDir);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &windowsettings::ButtonCancel);
    connect(ui->pushButton_6, &QPushButton::clicked, this, &windowsettings::EndWindow);
    ChangeType();
}

AutoGenSettings windowsettings::settings() const
{
    return resultSettings;
}

void windowsettings::ButtonCancel()
{
    reject();
}

void windowsettings::EndWindow()
{
    const bool collage = ui->radioButton->isChecked();
    const QString path = collage ? ui->lineEdit->text().trimmed() : ui->lineEdit_4->text().trimmed();
    const int height = ui->lineEdit_2->text().toInt();
    const int width = ui->lineEdit_3->text().toInt();
    const int number = ui->lineEdit_5->text().toInt();
    if (path.isEmpty())
    {
        QMessageBox::warning(this, tr("Autogen"),
                             collage ? tr("Choose a file for the collage.")
                                     : tr("Choose a folder for the images."));
        return;
    }
    if (collage && (width < 1 || height < 1))
    {
        QMessageBox::warning(this, tr("Autogen"), tr("Collage width and height must be at least 1."));
        return;
    }
    if (!collage && number < 1)
    {
        QMessageBox::warning(this, tr("Autogen"), tr("Number of images must be at least 1."));
        return;
    }

    resultSettings.mode = collage ? AutoGenMode::Collage : AutoGenMode::SeparateFiles;
    resultSettings.extendedFormat = ui->checkBox->isChecked();
    resultSettings.path = path;
    resultSettings.height = height;
    resultSettings.width = width;
    resultSettings.number = number;
    resultSettings.isRndList.clear();
    for (QCheckBox *box : rndarr)
        resultSettings.isRndList.append(box->isChecked());
    accept();
}

void windowsettings::AskFile()
{
    const QString chosen = QFileDialog::getSaveFileName(this, tr("Save collage"),
                                            startPath(AppKeys::dirAutogen, QStringLiteral("image.png")),
                                            tr("Image (*.png);;All files (*.*)"));
    if (chosen.isEmpty())
        return;
    filepath = chosen;
    rememberPath(AppKeys::dirAutogen, filepath);
    ui->lineEdit->setText(filepath);
}

void windowsettings::AskDir()
{
    const QString chosen = QFileDialog::getExistingDirectory(this, tr("Save images"),
                                                startPath(AppKeys::dirAutogen, QString()));
    if (chosen.isEmpty())
        return;
    dirpath = chosen;
    rememberPath(AppKeys::dirAutogen, dirpath);
    ui->lineEdit_4->setText(dirpath);
}

void windowsettings::TakeAll()
{
    for (QCheckBox *box : rndarr)
        box->setChecked(true);
}

void windowsettings::TakeNothing()
{
    for (QCheckBox *box : rndarr)
        box->setChecked(false);
}

void windowsettings::ChangeType()
{
    const bool collage = ui->radioButton->isChecked();
    ui->groupBox->setEnabled(collage);
    ui->groupBox_2->setEnabled(!collage);
    const QString on = QStringLiteral("color: rgb(110, 170, 200);");
    const QString off = QStringLiteral("color: rgb(55, 75, 85);");
    for (QLabel *lab : ui->groupBox->findChildren<QLabel *>())
        lab->setStyleSheet(collage ? on : off);
    for (QLabel *lab : ui->groupBox_2->findChildren<QLabel *>())
        lab->setStyleSheet(collage ? off : on);
}

windowsettings::~windowsettings()
{
    delete ui;
}
