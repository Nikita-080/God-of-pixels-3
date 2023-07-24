#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "multislider.h"
#include "pointchoicer.h"
#include <QColorDialog>
#include "QTime"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <windowsettings.h>
#include <QPainter>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QTime midnight(0,0,0);
    rnd.seed(midnight.secsTo(QTime::currentTime()));

    isEmtyPlanet=true;
    currentpath=CurrentPath();
    planet=Planet(rnd,currentpath);
    autoplanet=Planet(rnd,currentpath);
    s=PlanetSettings(rnd);

    planet=Planet(rnd,CurrentPath());

    ui->setupUi(this);
    // настройки
    ui->tabWidget->setIconSize(QSize(60,60));
    for (int i=0;i<10;i++){
        ui->tabWidget->setTabIcon(i,QIcon(currentpath+"res/images/TabIcon"+QString::number(i+1)+".png"));
    }

    //виджеты
    ms=new MultiSlider();
    pc=new PointChoicer(rnd);
    pc2=new PointChoicer(rnd);

    ms->setParent(ui->tab_2);
    pc->setParent(ui->tab_5);
    pc->setGeometry(0,101,400,500);
    pc2->setParent(ui->tab_10);
    pc2->setGeometry(0,101,400,500);
    //алгоритм генерации
    ui->comboBox->addItem("diamond square");
    ui->comboBox->addItem("fault formation");
    ui->horizontalSlider_4->setEnabled(false);
    connect(ui->comboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MainWindow::AlgorithmChange);
    //массивы
    sliders[0]= ui->horizontalSlider;    labels[0]= ui->label_2;
    sliders[1]= ui->horizontalSlider_2;  labels[1]= ui->label_4;
    sliders[2]= ui->horizontalSlider_3;  labels[2]= ui->label_6;
    sliders[3]= ui->horizontalSlider_6;  labels[3]= ui->label_22;
    sliders[4]= ui->horizontalSlider_8;  labels[4]= ui->label_30;
    sliders[5]= ui->horizontalSlider_9;  labels[5]= ui->label_27;
    sliders[6]= ui->horizontalSlider_10; labels[6]= ui->label_29;
    sliders[7]= ui->horizontalSlider_11; labels[7]= ui->label_35;
    sliders[8]=ui->horizontalSlider_13; labels[8]=ui->label_34;
    sliders[9]=ui->horizontalSlider_12; labels[9]=ui->label_38;
    sliders[10]=ui->horizontalSlider_14; labels[10]=ui->label_37;
    sliders[11]=ui->horizontalSlider_4; labels[11]=ui->label_46;
    sliders[12]=ui->horizontalSlider_5; labels[12]=ui->label_48;
    //коннекты
    for (int i=0;i<13;i++){
        connect(sliders[i],&QSlider::valueChanged,this,&MainWindow::SliderShow);
    }
    connect(ui->pushButton_26,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_27,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_28,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_29,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_30,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_31,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_32,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_33,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_34,&QPushButton::clicked,this,&MainWindow::ColorChoicer);
    connect(ui->pushButton_35,&QPushButton::clicked,this,&MainWindow::ColorChoicer);

    connect(ui->pushButton_18,&QPushButton::clicked,this,&MainWindow::CreatePlanet);
    connect(ui->pushButton_19,&QPushButton::clicked,this,&MainWindow::CreatePlanet);
    connect(ui->pushButton_20,&QPushButton::clicked,this,&MainWindow::AutoGod);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::ShowPlanet);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&MainWindow::ShowDescription);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&MainWindow::ShowSystem);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&MainWindow::ShowMap);

    //служебные функции
    connect(ui->pushButton_6,&QPushButton::clicked,this,&MainWindow::BiomGrad);
    connect(ui->pushButton_7,&QPushButton::clicked,this,&MainWindow::Img_Report);

    connect(ui->action_3,&QAction::triggered,this,&MainWindow::M_Save_Settings);
    connect(ui->action_4,&QAction::triggered,this,&MainWindow::M_Load_Settings);
    connect(ui->action_5,&QAction::triggered,this,&MainWindow::M_Load_Base_Settings);
    connect(ui->action,&QAction::triggered,this,&MainWindow::M_Save_Image);
    connect(ui->action_2,&QAction::triggered,this,&MainWindow::M_Save_Planet);
    connect(ui->action_6,&QAction::triggered,this,&MainWindow::M_About);
    connect(ui->action_8,&QAction::triggered,this,&MainWindow::M_Save_Full_Image);
    connect(ui->action_9,&QAction::triggered,this,&MainWindow::M_Load_Planet);

    ui->pushButton_2->setIconSize(QSize(465,465));

    SetStyle();

    //settings
    Settings_Get();//для начального заполнения "буфера" настроек
    s.Load(currentpath+"/res/text/settingsbase.txt"); //protected
    Settings_Set();

    //кнопки, вызывающие служебные функции
    //ui->pushButton_6->hide();
    //ui->pushButton_7->hide();
}
QString MainWindow::ReadText(QString path)
{
    QFile file(path);
    QByteArray data;
    if (file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        return QString(data);
    }
    else return "";
}
void MainWindow::AlgorithmChange()
{
    if (ui->comboBox->currentIndex()==0)
    {
        ui->horizontalSlider_2->setEnabled(true);
        ui->horizontalSlider_4->setEnabled(false);
    }
    else
    {
        ui->horizontalSlider_2->setEnabled(false);
        ui->horizontalSlider_4->setEnabled(true);
    }
}
void MainWindow::M_About()
{
    QMessageBox msb;
    msb.setText("название - God of Pixels 3\nверсия - 0.0.1\nавтор - Рябов Никита\nобратная связь - riabovnick080@yandex.ru");
    msb.exec();
}
QString MainWindow::CurrentPath()
{
    //TODO change for release
    return "C:/qtprojects/GodOfPixels3/";
    //return QDir::currentPath();
}

void MainWindow::SetStyle()
{
    QPushButton* a[]{ui->pushButton,ui->pushButton_18,
    ui->pushButton_19,ui->pushButton_20,
    ui->pushButton_3,ui->pushButton_4,
    ui->pushButton_5};

    QString sliderstyle=ReadText(currentpath+"res/text/sliderstyle.css");
    QString menubarstyle=ReadText(currentpath+"res/text/menubarstyle.css");
    QString tabwidgetstyle=ReadText(currentpath+"res/text/tabwidgetstyle.css");
    QString buttonstyle=ReadText(currentpath+"res/text/buttonstyle.css");
    QString comboboxstyle=ReadText(currentpath+"res/text/comboboxstyle.css");

    for (int i=0;i<7;i++)
    {
        QString name=a[i]->objectName();
        QString style=buttonstyle;
        style.replace("[currentpath]",currentpath);
        style.replace("[name]",name);
        a[i]->setStyleSheet(style);
    }

    foreach(QObject* i, this->findChildren<QObject*>())
    {
        if (i->inherits("QWidget"))
        {
            if (i->inherits("QLabel") || i->inherits("QGroupBox") ||
                i->inherits("QRadioButton") || i->inherits("QCheckBox"))
            {
                QWidget* k=static_cast<QWidget *>(i);
                QPalette palette;
                palette.setColor(QPalette::WindowText,QColor(110,170,200));
                k->setPalette(palette);
            }
            else if (i->inherits("QSlider"))
            {
                QSlider* k=static_cast<QSlider *>(i);
                k->setStyleSheet(sliderstyle);
            }
        }
    }

    ui->comboBox->setStyleSheet(comboboxstyle);
    ui->menubar->setStyleSheet(menubarstyle);
    ui->tabWidget->setStyleSheet(tabwidgetstyle);
    ui->pushButton_8->setIcon(QIcon(currentpath+"res/images/logo.png"));
}
void MainWindow::AutoGod()
{
    windowsettings win(CurrentPath(),this);
    isdatarecieved=false;
    win.exec();
    if (isdatarecieved)
    {
        ui->progressBar_2->setValue(0);
        Settings_Get();
        if (box.save_type==1) //коллаж
        {
            int delta;
            if (box.picturetype) delta=658;
            else delta=257;

            int count=0;
            double percent=1.0*box.height*box.width/100;
            QImage image=QImage(box.width*delta,box.height*delta,QImage::Format_RGB32);
            QPainter p;
            p.begin(&image);
            for (int i=0;i<box.height;i++) //высота
            {
                for (int k=0;k<box.width;k++) //ширина
                {
                    s.Random(box.isRndList,pc);
                    God(true,ui->progressBar_3,&autoplanet);
                    if (box.picturetype) p.drawImage(k*delta,i*delta,autoplanet.img_final);
                    else p.drawImage(k*delta,i*delta,autoplanet.img_nonscale);
                    count++;
                    ui->progressBar_2->setValue(qRound(1.0*count/percent));
                }
            }
            image.save(box.path);
            ui->progressBar_2->setValue(100);
        }
        else
        {
            double percent=1.0*box.number/100;
            for (int k=0;k<box.number;k++)
            {
                s.Random(box.isRndList,pc);
                God(true,ui->progressBar_3,&autoplanet);
                ui->progressBar_2->setValue(qRound(1.0*k/percent));
                QImage photo;
                if (box.picturetype) photo=autoplanet.img_final;
                else photo=autoplanet.img_nonscale;
                photo.save(box.path+"\\"+autoplanet.name+".png");
            }
            ui->progressBar_2->setValue(100);
        }
    }
}
void MainWindow::CreatePlanet()
{
    Settings_Get();
    bool flag=sender()->objectName()=="pushButton_18" or isEmtyPlanet;
    God(flag,ui->progressBar,&planet);
    QImage img=planet.img;
    ui->pushButton_2->setIcon(QIcon(QPixmap::fromImage(img)));
    ui->label_7->setText(planet.name);
    isEmtyPlanet=false;
}
void MainWindow::ShowPlanet()
{
    QImage img=planet.img;
    ui->pushButton_2->setIcon(QIcon(QPixmap::fromImage(img)));
}
void MainWindow::ShowDescription()
{
    QImage img=planet.img_dsc;
    ui->pushButton_2->setIcon(QIcon(QPixmap::fromImage(img)));
}
void MainWindow::ShowSystem()
{
    QImage img=planet.img_sys;
    ui->pushButton_2->setIcon(QIcon(QPixmap::fromImage(img)));
}
void MainWindow::ShowMap()
{
    QImage img=planet.img_gal;
    ui->pushButton_2->setIcon(QIcon(QPixmap::fromImage(img)));
}
void MainWindow::M_Save_Image()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Сохранить изображение"),
                                planet.name,
                                "Image (*.png);;All files (*.*)");
    try {
        planet.img.save(filename);
    }  catch (...) {
        QMessageBox::critical(nullptr,"Ошибка","не удалось сохранить файл");
    }
}
void MainWindow::M_Save_Full_Image()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Сохранить полное изображение"),
                                planet.name,
                                "Image (*.png);;All files (*.*)");
    try {
        planet.img_final.save(filename);
    }  catch (...) {
        QMessageBox::critical(nullptr,"Ошибка","не удалось сохранить файл");
    }
}
void MainWindow::M_Load_Planet()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Загрузить планету"),
                                                    "./",
                                "Planet (*.planet);;All files (*.*)");
    try {
        QFile readFile(filename);
        readFile.open(QFile::ReadOnly);
        QDataStream outFile(&readFile);
        //outFile.setVersion(QDataStream::Qt_4_8);
        outFile >> planet.matrix;
        outFile >> planet.name;
        outFile >> planet.u_map;
        outFile >> planet.facts.day;
        outFile >> planet.facts.year;
        outFile >> planet.facts.gravitation;
        outFile >> planet.facts.radiation;
        outFile >> planet.facts.resources;
        outFile >> planet.facts.seismicity;

        Settings_Get();
        God(false,ui->progressBar,&planet);
        QImage img=planet.img;
        ui->pushButton_2->setIcon(QIcon(QPixmap::fromImage(img)));
        ui->label_7->setText(planet.name);
        isEmtyPlanet=false;

        readFile.close();
    }  catch (...) {
        QMessageBox::critical(nullptr,"Ошибка","не удалось загрузить файл");
    }
}

void MainWindow::M_Save_Planet()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Сохранить планету"),
                                planet.name,
                                "Planet (*.planet);;All files (*.*)");
    try {
        QFile file(filename);
        file.open(QFile::Append);
        QDataStream infile(&file);

        infile << planet.matrix;
        infile << planet.name;
        infile << planet.u_map;
        infile << planet.facts.day;
        infile << planet.facts.year;
        infile << planet.facts.gravitation;
        infile << planet.facts.radiation;
        infile << planet.facts.resources;
        infile << planet.facts.seismicity;

        file.flush();
        file.close();
    }  catch (...) {
        QMessageBox::critical(nullptr,"Ошибка","не удалось сохранить файл");
    }
}
void MainWindow::God(bool isCreateNew, QProgressBar *pb,Planet *p)
{
    /* //systems pics generator
    for (int i=0;i<100;i++)
    {
        s.temperature=rnd.bounded(-9,15)*10;
        p->s=s;
        p->CreateMatrixNew();
        p->Calculator();
        p->SystemMap();
        p->img_sys.save("C:/Users/Никита/Desktop/systems/"+QString::number(i)+".png");
    }
    return;
    */

    pb->setValue(0);
    p->s=s;
    if (isCreateNew) p->CreateMatrixNew();
    pb->setValue(10);
    p->Calculator();
    if (isCreateNew) p->FixMatrix();
    p->LevelCreating();
    pb->setValue(25);
    p->ImageCreating();
    pb->setValue(30);
    if (isCreateNew) p->UMapCreating();
    p->TMapCreating();
    p->RMapCreating();
    pb->setValue(40);
    if (s.is_plant and p->water_level>0 and s.is_atmo) p->Plant();
    pb->setValue(45);
    if (p->water_level>0) p->Polar();
    if (s.noise>0) p->Noise();
    pb->setValue(50);
    if (s.is_cloud)
    {
        p->CloudMapCreating();
        p->Cloud();
    }
    pb->setValue(60);
    //planet.img.save("./img.png");
    p->UV();
    pb->setValue(70);
    if (s.is_atmo and s.atmo_transparent!=10) p->Atmosphere("in");
    p->Shadow();
    pb->setValue(75);
    if (s.is_atmo and s.atmo_transparent!=10) p->Atmosphere("out");
    pb->setValue(80);
    if (s.is_ring) p->Ring();
    pb->setValue(90);
    if (isCreateNew) p->Name();
    pb->setValue(95);
    if (isCreateNew) p->GenerateDescription();
    p->CalculateDescription();
    p->DrawDescription();
    p->SystemMap();
    p->GalaxyMap();
    p->FinalImage();
    p->ImagesScale();
    pb->setValue(100);
}


void MainWindow::M_Load_Base_Settings(){
    Settings_Get(); //для начального заполнения "буфера" настроек
    if (s.Load(currentpath+"res/text/settingsbase.txt"))
    {
        Settings_Set();
    }
    else QMessageBox::critical(nullptr,"Ошибка","не удалось загрузить файл");
}
void MainWindow::SliderShow(){
    QSlider* slider = qobject_cast<QSlider*>(sender());
    for (int i=0;i<13;i++){
        if (slider==sliders[i]){
            labels[i]->setText(QString::number(slider->value()));
            break;
        }
    }
}
void MainWindow::ColorToButton(QPushButton* button,QColor color){
    button->clearMask();
    QString s="QPushButton{background-color: rgb("+
            QString::number(color.red())+","+
            QString::number(color.green())+","+
            QString::number(color.blue())+");}";
    button->setStyleSheet(s);
    QPalette palette;
    QColor invert_color; //new color system
    if (((color.red()*299)+(color.green()*587)+(color.blue()*114))/1000>=128)
    {
        invert_color=QColor(0,0,0);
    }
    else invert_color=QColor(255,255,255);
    palette.setColor(QPalette::ButtonText,invert_color);
    button->setPalette(palette);
}
QColor MainWindow::ColorFromButton(QPushButton * button){
    QString s=button->styleSheet();
    QString r=""; QString g=""; QString b="";
    int ptr=0;
    while (s[ptr]!='('){ptr++;}
    ptr++;
    while (s[ptr]!=','){
        r+=s[ptr];
        ptr++;
    }
    ptr++;
    while (s[ptr]!=','){
        g+=s[ptr];
        ptr++;
    }
    ptr++;
    while (s[ptr]!=')'){
        b+=s[ptr];
        ptr++;
    }
    QColor color=QColor(r.toInt(),g.toInt(),b.toInt());
    return color;
}
void MainWindow::ColorChoicer(){
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QColor current=ColorFromButton(button);
    QColor color = QColorDialog::getColor(current);
    if (color.isValid()) ColorToButton(button,color);
}

void MainWindow::Settings_Get(){
    s.terramode=ui->comboBox->currentIndex();
    s.randomness=ui->horizontalSlider_2->value();
    s.iterations=ui->horizontalSlider_4->value();
    s.world_size=ui->horizontalSlider->value();
    s.temperature=ui->horizontalSlider_3->value();
    s.structure=ms->GetData();
    s.true_structure=ms->GetTrueData();
    s.ice_color=ColorFromButton(ui->pushButton_26);
    s.rock_color=ColorFromButton(ui->pushButton_27);
    s.mountain_color=ColorFromButton(ui->pushButton_28);
    s.plain_color=ColorFromButton(ui->pushButton_29);
    s.beach_color=ColorFromButton(ui->pushButton_30);
    s.shallow_color=ColorFromButton(ui->pushButton_31);
    s.ocean_color=ColorFromButton(ui->pushButton_32);
    s.noise=ui->horizontalSlider_5->value();
    s.is_gradient=ui->checkBox_6->isChecked();
    s.is_plant=ui->checkBox_5->isChecked();
    s.shine=ui->horizontalSlider_6->value();
    s.point_of_shine=pc->GetData();
    s.point_of_shine_true=pc->GetTrueData();
    if      (ui->radioButton->isChecked())  {s.name_algorithm=1;}
    else if (ui->radioButton_2->isChecked()){s.name_algorithm=2;}
    else if (ui->radioButton_3->isChecked()){s.name_algorithm=3;}
    s.is_cloud=ui->checkBox->isChecked();
    s.cloud_size=ui->horizontalSlider_8->value();
    s.cloud_quality=ui->horizontalSlider_9->value();
    s.cloud_transparent=ui->horizontalSlider_10->value();
    s.correction=ui->checkBox_2->isChecked();
    s.cloud_color=ColorFromButton(ui->pushButton_33);
    s.is_atmo=ui->checkBox_3->isChecked();
    s.atmo_transparent=ui->horizontalSlider_11->value();
    s.atmo_size=ui->horizontalSlider_13->value();
    s.atmo_color=ColorFromButton(ui->pushButton_34);
    s.is_ring=ui->checkBox_4->isChecked();
    s.R_internal_ring=ui->horizontalSlider_12->value();
    s.R_external_ring=ui->horizontalSlider_14->value();
    s.ring_color=ColorFromButton(ui->pushButton_35);
    s.point_of_polar=pc2->GetData();
    s.point_of_polar_true=pc2->GetTrueData();
}
void MainWindow::Settings_Set(){
    ui->comboBox->setCurrentIndex(s.terramode);
    ui->horizontalSlider_2->setValue(s.randomness);
    ui->horizontalSlider_4->setValue(s.iterations);
    ui->horizontalSlider->setValue(s.world_size);
    ui->horizontalSlider_3->setValue(s.temperature);
    ms->SetData(s.structure);
    ColorToButton(ui->pushButton_26,s.ice_color);
    ColorToButton(ui->pushButton_27,s.rock_color);
    ColorToButton(ui->pushButton_28,s.mountain_color);
    ColorToButton(ui->pushButton_29,s.plain_color);
    ColorToButton(ui->pushButton_30,s.beach_color);
    ColorToButton(ui->pushButton_31,s.shallow_color);
    ColorToButton(ui->pushButton_32,s.ocean_color);
    ui->horizontalSlider_5->setValue(s.noise);
    ui->checkBox_6->setChecked(s.is_gradient);
    ui->checkBox_5->setChecked(s.is_plant);
    ui->horizontalSlider_6->setValue(s.shine);
    pc->SetData(s.point_of_shine);
    if      (s.name_algorithm==1) {ui->radioButton->setChecked(true);}
    else if (s.name_algorithm==2) {ui->radioButton_2->setChecked(true);}
    else if (s.name_algorithm==3) {ui->radioButton_3->setChecked(true);}
    ui->checkBox->setChecked(s.is_cloud);
    ui->horizontalSlider_8->setValue(s.cloud_size);
    ui->horizontalSlider_9->setValue(s.cloud_quality);
    ui->horizontalSlider_10->setValue(s.cloud_transparent);
    ui->checkBox_2->setChecked(s.correction);
    ColorToButton(ui->pushButton_33,s.cloud_color);
    ui->checkBox_3->setChecked(s.is_atmo);
    ui->horizontalSlider_11->setValue(s.atmo_transparent);
    ui->horizontalSlider_13->setValue(s.atmo_size);
    ColorToButton(ui->pushButton_34,s.atmo_color);
    ui->checkBox_4->setChecked(s.is_ring);
    ui->horizontalSlider_12->setValue(s.R_internal_ring);
    ui->horizontalSlider_14->setValue(s.R_external_ring);
    ColorToButton(ui->pushButton_35,s.ring_color);
    pc2->SetData(s.point_of_polar);
}

void MainWindow::M_Save_Settings(){
    QString filename = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Сохранить файл"),
                                                    "./",
                                "Texts (*.txt);;All files (*.*)");
    Settings_Get();
    if (!s.Save(filename)) QMessageBox::critical(nullptr,"Ошибка","не удалось сохранить файл");
}
void MainWindow::M_Load_Settings(){
    QString filename = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Открыть файл"),
                                "./",
                                "Texts (*.txt);;All files (*.*)");
    Settings_Get();//для начального заполнения "буфера" настроек
    if (s.Load(filename))
    {
        Settings_Set();
        update();
    }
    else QMessageBox::critical(nullptr,"Ошибка","не удалось загрузить файл");
}
void MainWindow::Img_Report() //служебная функция
{
    planet.ImageReport("t",planet.t_map,QColor(0,0,255),QColor(255,0,0));
    planet.ImageReport("m",planet.matrix,QColor(0,0,0),QColor(255,255,255));
    planet.ImageReport("r",planet.r_map,QColor(252,221,118),QColor(0,0,255));
}
void MainWindow::BiomGrad() //служебная функция
{
    for (int i=-50;i<=50;i++)
    {
        s.temperature=i;
        God(false,ui->progressBar,&planet);
        QString name=QString::number(i+50)+" ("+QString::number(i)+").png";
        planet.img.save("C:/Users/Никита/Desktop/biom/"+name);
    }
}
void MainWindow::Report(QString s) //служебная функция
{
    QFile file("C:/Users/Никита/Desktop/DATA.txt");
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        stream<<s<<"\n";
        file.close();
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}

