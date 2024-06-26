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
#include <QJsonObject>
#include <QJsonDocument>
#include <global.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{ 
    isEmtyPlanet=true;
    planet=Planet();
    autoplanet=Planet();
    s=PlanetSettings();

    ui->setupUi(this);
    // настройки
    ui->tabWidget->setIconSize(QSize(60,60));
    for (int i=0;i<10;i++){
        ui->tabWidget->setTabIcon(i,QIcon(":/images/res/images/TabIcon"+QString::number(i+1)+".png"));
    }

    //виджеты
    ms=new MultiSlider();
    pc=new PointChoicer();
    pc2=new PointChoicer();

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
    connect(ui->pushButton_20,&QPushButton::clicked,this,&MainWindow::AutoGen);
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
    s.Load(":/txt_files/res/txt_files/settingsbase.json");
    Settings_Set();

    //кнопки, вызывающие служебные функции
    ui->pushButton_6->hide();
    ui->pushButton_7->hide();
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
    QString message;
    message.append("название - God of Pixels 3\n");
    message.append("версия - ");
    message.append(QString::number(VERSION));
    message.append("\n");
    message.append("автор - Рябов Никита\n");
    message.append("обратная связь - riabovnick080@yandex.ru");
    msb.setText(message);
    msb.exec();
}

void MainWindow::SetStyle()
{
    QPushButton* a[]{ui->pushButton,ui->pushButton_18,
    ui->pushButton_19,ui->pushButton_20,
    ui->pushButton_3,ui->pushButton_4,
    ui->pushButton_5};

    QString sliderstyle=ReadText(":/css_files/res/css_files/slider.css");
    QString menubarstyle=ReadText(":/css_files/res/css_files/menubar.css");
    QString tabwidgetstyle=ReadText(":/css_files/res/css_files/tabwidget.css");
    QString buttonstyle=ReadText(":/css_files/res/css_files/button.css");
    QString comboboxstyle=ReadText(":/css_files/res/css_files/combobox.css");

    for (int i=0;i<7;i++)
    {
        QString name=a[i]->objectName();
        QString style=buttonstyle;
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
    ui->pushButton_8->setIcon(QIcon(":/images/res/images/logo.png"));
}
void MainWindow::AutoGen()
{
    windowsettings win(this);
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
                    Gen(true,ui->progressBar_3,&autoplanet);
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
                Gen(true,ui->progressBar_3,&autoplanet);
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
    Gen(flag,ui->progressBar,&planet);
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
    if (filename.isEmpty()) return;
    try {
        planet.img.save(filename);
    }  catch (...) {
        QMessageBox::critical(nullptr,"Ошибка",CANT_SAVE_FILE);
    }
}
void MainWindow::M_Save_Full_Image()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Сохранить полное изображение"),
                                planet.name,
                                "Image (*.png);;All files (*.*)");
    if (filename.isEmpty()) return;
    try {
        planet.img_final.save(filename);
    }  catch (...) {
        QMessageBox::critical(nullptr,"Ошибка",CANT_SAVE_FILE);
    }
}
void MainWindow::M_Load_Planet()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Загрузить планету"),
                                                    "./",
                                "Planet (*.planet);;All files (*.*)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    file.open(QFile::ReadOnly|QFile::Text);
    try {
        QString a;
        a=file.readAll();
        QJsonObject jobject=QJsonDocument::fromJson(a.toUtf8()).object();

        if (!s.JSON_deserialize(jobject["settings"].toObject()))
        {
            file.close();
            QMessageBox::critical(nullptr,"Ошибка",CANT_LOAD_FILE);
            return;
        }
        Settings_Set();

        Gen(true,ui->progressBar,&planet,jobject["seed"].toInt());
        QImage img=planet.img;
        ui->pushButton_2->setIcon(QIcon(QPixmap::fromImage(img)));
        ui->label_7->setText(planet.name);
        isEmtyPlanet=false;
    }  catch (...) {
        QMessageBox::critical(nullptr,"Ошибка",CANT_LOAD_FILE);
    }
    file.close();
}

void MainWindow::M_Save_Planet()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Сохранить планету"),
                                planet.name,
                                "Planet (*.planet);;All files (*.*)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (file.open(QFile::WriteOnly|QFile::Text)){
        QTextStream stream(&file);

        QJsonObject jobject;
        jobject["seed"]=planet.seed;
        jobject["settings"]=planet.s.JSON_serialize();
        stream<<QJsonDocument(jobject).toJson();

        file.close();
    }
    else
    {
        QMessageBox::critical(nullptr,"Ошибка",CANT_SAVE_FILE);
    }
}
void MainWindow::Gen(bool isCreateNew, QProgressBar *pb,Planet *p,int seed)
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
    int percent[26] {20, //height map
                      1, //main numbers
                      3, //normalize height map
                      2, //levels
                      1, //image
                      3, //UV map
                      3, //temperature map
                      3, //water map
                      3, //plants
                      3, //polar ice
                      2, //noise
                      25, //clouds map
                      3, //clouds
                      2, //UV
                      3, //internal atmosphere
                      3, //shadows
                      3, //external atmosphere
                      3, //rings
                      1, //name
                      1, //description (generated)
                      1, //description (calculated)
                      2, //description (draw)
                      2, //system map
                      2, //galaxy map
                      2, //final image
                      3};//scale

    pb->setValue(0);
    p->s=s;
    if (isCreateNew) p->SetSeed(seed);

    /*
    for (int i=0;i<26;i++)
    {
        p->SetSeed(p->seed);
        p->Iteration(i);
        pb->setValue(pb->value()+percent[i]);
        if (i==12) p->img=p->ImageReport(p->matrix,QColor(200,200,200),QColor(50,50,50));
    }
    p->img.save("C:/Users/Никита/Desktop/imgs/land.png");
    for (int i=0;i<26;i++)
    {
        p->SetSeed(p->seed);
        p->Iteration(i);
        pb->setValue(pb->value()+percent[i]);
        if (i==12) p->img=p->ImageReport(p->t_map,QColor(0,0,255),QColor(255,0,0));
    }
    p->img.save("C:/Users/Никита/Desktop/imgs/temperature.png");
    for (int i=0;i<26;i++)
    {
        p->SetSeed(p->seed);
        p->Iteration(i);
        pb->setValue(pb->value()+percent[i]);
        if (i==12) p->img=p->ImageReport(p->r_map,QColor(252,221,118),QColor(0,255,255));
    }
    p->img.save("C:/Users/Никита/Desktop/imgs/water.png");
    */
    for (int i=0;i<26;i++)
    {
        p->SetSeed(p->seed);
        p->Iteration(i);
        pb->setValue(pb->value()+percent[i]);
    }
    //p->img.save("C:/Users/Никита/Desktop/imgs/biom.png");

}


void MainWindow::M_Load_Base_Settings(){
    Settings_Get(); //для начального заполнения "буфера" настроек
    if (s.Load(":/txt_files/res/txt_files/settingsbase.json"))
    {
        Settings_Set();
    }
    else QMessageBox::critical(nullptr,"Ошибка",CANT_LOAD_DEFAULT);
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
                                "Texts (*.json);;All files (*.*)");
    if (filename.isEmpty()) return;
    Settings_Get();
    if (!s.Save(filename)) QMessageBox::critical(nullptr,"Ошибка",CANT_SAVE_FILE);
}
void MainWindow::M_Load_Settings(){
    QString filename = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Открыть файл"),
                                "./",
                                "Texts (*.json);;All files (*.*)");
    if (filename.isEmpty()) return;
    Settings_Get();//для начального заполнения "буфера" настроек
    if (s.Load(filename))
    {
        Settings_Set();
        update();
    }
    else QMessageBox::critical(nullptr,"Ошибка",CANT_LOAD_FILE);
}
void MainWindow::Img_Report() //служебная функция
{
    QString path="C:/Users/Никита/Desktop/";
    planet.ImageReport(planet.t_map,QColor(0,0,255),QColor(255,0,0)).save(path+"t.png");
    planet.ImageReport(planet.matrix,QColor(0,0,0),QColor(255,255,255)).save(path+"m.png");
    planet.ImageReport(planet.r_map,QColor(252,221,118),QColor(0,0,255)).save(path+"r.png");
}
void MainWindow::BiomGrad() //служебная функция
{
    for (int i=-50;i<=50;i++)
    {
        s.temperature=i;
        Gen(false,ui->progressBar,&planet);
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

