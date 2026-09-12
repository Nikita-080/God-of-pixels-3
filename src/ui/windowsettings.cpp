#include "windowsettings.h"
#include "ui_windowsettings.h"
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QRadioButton>
#include "appsettings.h"

windowsettings::windowsettings(const QString &language, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::windowsettings)
{
    ui->setupUi(this);
    setWindowTitle(tr("Autogen"));

    QFormLayout *formLayout = new QFormLayout();
    QGroupBox *groupBox = new QGroupBox();

    const QString path = (language == "ru")
                             ? QString(":/txt_files/res/txt_files/randomsettings_ru.txt")
                             : QString(":/txt_files/res/txt_files/randomsettings_en.txt");
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QTextStream dat(&file);
    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(110, 170, 200));

    while (!dat.atEnd())
    {
        QString line = dat.readLine();
        QCheckBox *box = new QCheckBox(line);
        box->setPalette(palette);
        formLayout->addRow(box);
        rndarr.append(box);
    }
    file.close();

    groupBox->setLayout(formLayout);
    ui->scrollArea->setWidget(groupBox);
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
}

windowsettings::~windowsettings()
{
    delete ui;
}
