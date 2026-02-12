#include "windowsettings.h"
#include "ui_windowsettings.h"
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <mainwindow.h>
#include <autogensettings.h>
windowsettings::windowsettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::windowsettings)
{
    myparent=(MainWindow*)parent;

    ui->setupUi(this);

    QFormLayout *formLayout = new QFormLayout();
    QGroupBox *groupBox = new QGroupBox();

    QFile* file;
    if (myparent->language == "ru")
        file = new QFile(":/txt_files/res/txt_files/randomsettings_ru.txt");
    else
        file = new QFile(":/txt_files/res/txt_files/randomsettings_en.txt");
    file->open(QIODevice::ReadOnly);
    QTextStream dat(file);
    int count=0;
    QPalette palette;
    palette.setColor(QPalette::WindowText,QColor(110,170,200));

    numsettings=0;
    while (!dat.atEnd())
    {
        numsettings+=1;
        QString line = dat.readLine();
        QCheckBox *box=new QCheckBox();
        box->setGeometry(10,10+count*25,200,20);
        box->setText(line);
        box->setPalette(palette);
        formLayout->addRow(box);
        count++;
        rndarr.append(box);
    }
    file->close();

    groupBox->setLayout(formLayout);
    ui->scrollArea->setWidget(groupBox);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->radioButton->setChecked(true);
    ui->groupBox_2->setEnabled(false);

    connect(ui->radioButton,&QPushButton::clicked,this,&windowsettings::ChangeType);
    connect(ui->radioButton_2,&QPushButton::clicked,this,&windowsettings::ChangeType);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&windowsettings::TakeAll);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&windowsettings::TakeNothing);
    connect(ui->pushButton,&QPushButton::clicked,this,&windowsettings::AskFile);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&windowsettings::AskDir);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&windowsettings::ButtonCancel);
    connect(ui->pushButton_6,&QPushButton::clicked,this,&windowsettings::EndWindow);
}
void windowsettings::ButtonCancel()
{
    close();
}
void windowsettings::EndWindow()
{
    AutoGenSettings settings;

    settings.save_type=ui->radioButton->isChecked();
    settings.picturetype=ui->checkBox->isChecked();
    if (ui->radioButton->isChecked()) settings.path=filepath;
    else settings.path=dirpath;
    settings.height=ui->lineEdit_2->text().toInt();
    settings.width=ui->lineEdit_3->text().toInt();
    settings.number=ui->lineEdit_5->text().toInt();
    for (int i=0;i<numsettings;i++) settings.isRndList.append(rndarr[i]->isChecked());
    myparent->isdatarecieved=true;
    myparent->box=settings;

    close();
}
void windowsettings::AskFile()
{
    filepath = QFileDialog::getSaveFileName(this,
               tr("Save collage"),
               "image.png",
               tr("Image (*.png);;All files (*.*)"));
    ui->lineEdit->setText(filepath);
}
void windowsettings::AskDir()
{
    dirpath = QFileDialog::getExistingDirectory(this,
              tr("Save images"));
    ui->lineEdit_4->setText(dirpath);
}
void windowsettings::TakeAll()
{
    for (int i=0;i<numsettings;i++)
    {
        rndarr[i]->setChecked(true);
    }
}
void windowsettings::TakeNothing()
{
    for (int i=0;i<numsettings;i++)
    {
        rndarr[i]->setChecked(false);
    }
}
void windowsettings::ChangeType()
{
    if (ui->radioButton->isChecked())
    {
        ui->groupBox->setEnabled(true);
        ui->groupBox_2->setEnabled(false);
    }
    else
    {
        ui->groupBox->setEnabled(false);
        ui->groupBox_2->setEnabled(true);
    }
}
windowsettings::~windowsettings()
{
    delete ui;
}
