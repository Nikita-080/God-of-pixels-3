#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "multislider.h"
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
#include <appsettings.h>
#include <QApplication>
#include <QSettings>
#include "colorswatch.h"
#include "planetglwidget.h"
#include <QStackedWidget>
#include <QTimer>
#include <QDir>
#include <QtMath>
#include <QFont>
#include <QCheckBox>
#include <QRadioButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{ 
    isEmtyPlanet=true;
    planet=Planet();
    autoplanet=Planet();
    s=PlanetSettings();
    liveSuspended=false;
    appearanceOnlyLive=false;
    glWidget=nullptr;
    previewStack=nullptr;
    btnResetCamera=nullptr;
    liveTimer=nullptr;

    QSettings st;
    language = st.value(AppKeys::language, QStringLiteral("en")).toString();
    if (language != QStringLiteral("ru"))
        language = QStringLiteral("en");
    if (language == QStringLiteral("ru"))
    {
        qtLanguageTranslator.load(":/translations/QtLanguage_ru");
        qApp->installTranslator(&qtLanguageTranslator);
    }

    ui->setupUi(this);
    ui->action_live->setChecked(st.value(AppKeys::livePreview, false).toBool());
    ui->progressBar->hide();
    // настройки
    ui->tabWidget->setIconSize(QSize(60,60));
    for (int i=0;i<10;i++){
        ui->tabWidget->setTabIcon(i,QIcon(":/images/res/images/TabIcon"+
                                          QString::number(i+1)+".png"));
    }

    previewStack=new QStackedWidget(ui->centralwidget);
    glWidget=new PlanetGLWidget(previewStack);
    cardView=new QLabel(previewStack);
    cardView->setAlignment(Qt::AlignCenter);
    cardView->setScaledContents(true);
    cardView->setMinimumSize(257,257);
    previewStack->addWidget(glWidget);
    previewStack->addWidget(cardView);
    previewStack->setGeometry(ui->pushButton_2->geometry());
    ui->pushButton_2->hide();
    btnResetCamera=new QPushButton(ui->centralwidget);
    btnResetCamera->setObjectName(QStringLiteral("btnResetCamera"));
    const QRect previewRect=ui->pushButton_2->geometry();
    btnResetCamera->setGeometry(previewRect.x(), previewRect.bottom()+8, previewRect.width(), 28);
    connect(btnResetCamera,&QPushButton::clicked,glWidget,&PlanetGLWidget::resetCamera);

    liveTimer=new QTimer(this);
    liveTimer->setSingleShot(true);
    liveTimer->setInterval(200);
    connect(liveTimer,&QTimer::timeout,this,&MainWindow::runLivePreview);

    //виджеты
    ms=new MultiSlider();
    ms->setParent(ui->tab_2);
    addLatLonControls(ui->tab_5, sliderShineLat, sliderShineLon,
                      labelShineLatTitle, labelShineLonTitle,
                      labelShineLatValue, labelShineLonValue);
    addLatLonControls(ui->tab_10, sliderPolarLat, sliderPolarLon,
                      labelPolarLatTitle, labelPolarLonTitle,
                      labelPolarLatValue, labelPolarLonValue);
    retranslateCoordLabels();
    ui->horizontalSlider_4->setEnabled(false);
    connect(ui->comboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &MainWindow::AlgorithmChange);
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
    QList<QPushButton*> colorButtons={
        ui->pushButton_26,ui->pushButton_27,ui->pushButton_28,ui->pushButton_29,ui->pushButton_30,
        ui->pushButton_31,ui->pushButton_32,ui->pushButton_33,ui->pushButton_34,ui->pushButton_35
    };
    for (QPushButton* b : colorButtons)
        connect(b,&QPushButton::clicked,this,&MainWindow::ColorChoicer);

    connect(ui->pushButton_18,&QPushButton::clicked,this,&MainWindow::CreateNewPlanet);
    connect(ui->pushButton_19,&QPushButton::clicked,this,&MainWindow::RecreatePlanet);
    connect(ui->pushButton_20,&QPushButton::clicked,this,&MainWindow::AutoGen);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::ShowPlanet);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&MainWindow::ShowDescription);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&MainWindow::ShowSystem);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&MainWindow::ShowMap);

    connect(ui->pushButton_6,&QPushButton::clicked,this,&MainWindow::BiomGrad);
    connect(ui->pushButton_7,&QPushButton::clicked,this,&MainWindow::Img_Report);
count=0;
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
    setWindowTitle("God of Pixels 3");

    connect(ui->action_10, &QAction::triggered,this,&MainWindow::M_Switch_Language);
    connect(ui->action_live, &QAction::toggled, this, [](bool on) {
        QSettings().setValue(AppKeys::livePreview, on);
    });

    auto requestLive = [this]() { appearanceOnlyLive=false; scheduleLivePreview(); };
    auto requestAppearance = [this]() { appearanceOnlyLive=true; scheduleLivePreview(); };
    for (int i=0;i<13;i++)
        connect(sliders[i],&QSlider::valueChanged,this,[this]() { appearanceOnlyLive=false; scheduleLivePreview(); });
    connect(ui->comboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int){ appearanceOnlyLive=false; scheduleLivePreview(); });
    const QList<QCheckBox*> boxes={ui->checkBox,ui->checkBox_2,ui->checkBox_3,ui->checkBox_4,ui->checkBox_5,ui->checkBox_6};
    for (QCheckBox* b : boxes)
        connect(b,&QCheckBox::toggled,this,[this](bool){ appearanceOnlyLive=false; scheduleLivePreview(); });
    connect(ui->radioButton,&QRadioButton::toggled,this,[this](bool){ appearanceOnlyLive=false; scheduleLivePreview(); });
    connect(ui->radioButton_2,&QRadioButton::toggled,this,[this](bool){ appearanceOnlyLive=false; scheduleLivePreview(); });
    connect(ui->radioButton_3,&QRadioButton::toggled,this,[this](bool){ appearanceOnlyLive=false; scheduleLivePreview(); });
    connect(ms,&MultiSlider::valueChanged,this,requestLive);
    connect(sliderShineLat,&QSlider::valueChanged,this,requestAppearance);
    connect(sliderShineLon,&QSlider::valueChanged,this,requestAppearance);
    connect(sliderPolarLat,&QSlider::valueChanged,this,requestLive);
    connect(sliderPolarLon,&QSlider::valueChanged,this,requestLive);
    connect(ui->horizontalSlider_6,&QSlider::valueChanged,this,[this](){ appearanceOnlyLive=true; scheduleLivePreview(); });
    connect(ui->horizontalSlider_11,&QSlider::valueChanged,this,[this](){ appearanceOnlyLive=true; scheduleLivePreview(); });
    connect(ui->horizontalSlider_12,&QSlider::valueChanged,this,[this](){ appearanceOnlyLive=true; scheduleLivePreview(); });
    connect(ui->horizontalSlider_13,&QSlider::valueChanged,this,[this](){ appearanceOnlyLive=true; scheduleLivePreview(); });
    connect(ui->horizontalSlider_14,&QSlider::valueChanged,this,[this](){ appearanceOnlyLive=true; scheduleLivePreview(); });
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

void MainWindow::M_Switch_Language()
{
    if (language == "ru")
    {
        qApp->removeTranslator(&qtLanguageTranslator);
        language = "en";
    }
    else
    {
        qtLanguageTranslator.load(":/translations/QtLanguage_ru");
        language = "ru";
        qApp->installTranslator(&qtLanguageTranslator);
    }
    QSettings().setValue(AppKeys::language, language);

    ms->ReloadText();
}

void MainWindow::changeEvent(QEvent *event)
{
    // В случае получения события изменения языка приложения
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateCoordLabels();
    }
}

void MainWindow::M_About()
{
    QMessageBox msb;
    QString message;
    message.append(tr("name - God of Pixels 3")+"\n");
    message.append(tr("version - "));
    message.append(PROGRAM_VERSION);
    message.append("\n");
    message.append(tr("author - Riabov Nikita")+"\n");
    message.append(tr("feedback - riabovnick080@yandex.ru"));
    msb.setText(message);
    msb.exec();
}

void MainWindow::SetStyle()
{
    QFile f(":/css_files/res/css_files/app.qss");
    if (f.open(QIODevice::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    ui->pushButton_8->setIcon(QIcon(":/images/res/images/logo.png"));
}
void MainWindow::AutoGen()
{
    windowsettings win(language, this);
    if (win.exec() != QDialog::Accepted)
        return;
    box = win.settings();
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
            for (int i=0;i<box.height;i++)
            {
                for (int k=0;k<box.width;k++)
                {
                    s.Random(box.isRndList);
                    Gen(true,&autoplanet);
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
                s.Random(box.isRndList);
                Gen(true,&autoplanet);
                ui->progressBar_2->setValue(qRound(1.0*k/percent));
                QImage photo;
                if (box.picturetype) photo=autoplanet.img_final;
                else photo=autoplanet.img_nonscale;
                photo.save(QDir(box.path).filePath(autoplanet.name+".png"));
            }
            ui->progressBar_2->setValue(100);
        }
    }
}
void MainWindow::CreateNewPlanet()
{
    Settings_Get();
    Gen(true,&planet);
    applyPlanetToView();
    isEmtyPlanet=false;
}
void MainWindow::RecreatePlanet()
{
    Settings_Get();
    Gen(isEmtyPlanet,&planet);
    applyPlanetToView();
    isEmtyPlanet=false;
}
void MainWindow::applyPlanetToView()
{
    if (!glWidget)
        return;
    glWidget->setPlanet(&planet);
    glWidget->refreshTextures();
    glWidget->repaint();
    QImage shot=glWidget->captureView();
    if (!shot.isNull())
        planet.img_view=shot;
    planet.FinalImage();
    ShowPlanet();
    ui->label_7->setText(planet.name);
}
void MainWindow::ShowPlanet()
{
    if (previewStack)
        previewStack->setCurrentIndex(0);
}
void MainWindow::ShowDescription()
{
    if (!cardView || !previewStack) return;
    cardView->setPixmap(QPixmap::fromImage(planet.img_dsc));
    previewStack->setCurrentIndex(1);
}
void MainWindow::ShowSystem()
{
    if (!cardView || !previewStack) return;
    cardView->setPixmap(QPixmap::fromImage(planet.img_sys));
    previewStack->setCurrentIndex(1);
}
void MainWindow::ShowMap()
{
    if (!cardView || !previewStack) return;
    cardView->setPixmap(QPixmap::fromImage(planet.img_gal));
    previewStack->setCurrentIndex(1);
}
void MainWindow::M_Save_Image()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                tr("Save image"),
                                startPath(AppKeys::dirImage, planet.name),
                                tr("Image (*.png);;All files (*.*)"));
    if (filename.isEmpty()) return;
    rememberPath(AppKeys::dirImage, filename);
    try {
        QImage out = planet.img_view.isNull() ? planet.img : planet.img_view;
        out.save(filename);
    }  catch (...) {
        QMessageBox::critical(nullptr,tr("Error"),tr("0001 unable to save file"));
    }
}
void MainWindow::M_Save_Full_Image()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                tr("Save full image"),
                                startPath(AppKeys::dirImage, planet.name),
                                tr("Image (*.png);;All files (*.*)"));
    if (filename.isEmpty()) return;
    rememberPath(AppKeys::dirImage, filename);
    try {
        planet.img_final.save(filename);
    }  catch (...) {
        QMessageBox::critical(nullptr,tr("Error"),tr("0001 unable to save file"));
    }
}
void MainWindow::M_Load_Planet()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                tr("Load planet"),
                                startPath(AppKeys::dirPlanet),
                                tr("Planet (*.planet);;All files (*.*)"));
    if (filename.isEmpty()) return;
    rememberPath(AppKeys::dirPlanet, filename);
    QFile file(filename);
    file.open(QFile::ReadOnly|QFile::Text);
    try {
        QString a;
        a=file.readAll();
        QJsonObject jobject=QJsonDocument::fromJson(a.toUtf8()).object();

        if (!s.JSON_deserialize(jobject["settings"].toObject()))
        {
            file.close();
            return;
        }
        Settings_Set();

        Gen(true,&planet,jobject["seed"].toInt());
        applyPlanetToView();
        isEmtyPlanet=false;
    }  catch (...) {
        QMessageBox::critical(nullptr,tr("Error"),tr("0002 unable to load file"));
    }
    file.close();
}

void MainWindow::M_Save_Planet()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                tr("Save planet"),
                                startPath(AppKeys::dirPlanet, planet.name),
                                tr("Planet (*.planet);;All files (*.*)"));
    if (filename.isEmpty()) return;
    rememberPath(AppKeys::dirPlanet, filename);
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
        QMessageBox::critical(nullptr,tr("Error"),tr("0001 unable to save file"));
    }
}
void MainWindow::Gen(bool isCreateNew, Planet *p,int seed)
{
    p->s=s;
    if (isCreateNew) p->SetSeed(seed);
    p->Generate();
}


void MainWindow::M_Load_Base_Settings(){
    Settings_Get(); //для начального заполнения "буфера" настроек
    if (s.Load(":/txt_files/res/txt_files/settingsbase.json"))
    {
        Settings_Set();
    }
    else QMessageBox::critical(nullptr,tr("Error"),tr("0003 unable to load default settings"));
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
    ColorSwatch::setColor(button,color);
}
QColor MainWindow::ColorFromButton(QPushButton * button){
    return ColorSwatch::color(button);
}
void MainWindow::ColorChoicer(){
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QColor current=ColorFromButton(button);
    QColor color = QColorDialog::getColor(current);
    if (color.isValid()) {
        ColorToButton(button,color);
        appearanceOnlyLive=false;
        scheduleLivePreview();
    }
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
    s.shine_lat=sliderShineLat->value();
    s.shine_lon=sliderShineLon->value();
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
    s.polar_lat=sliderPolarLat->value();
    s.polar_lon=sliderPolarLon->value();
}
void MainWindow::Settings_Set(){
    liveSuspended=true;
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
    sliderShineLat->setValue(s.shine_lat);
    sliderShineLon->setValue(s.shine_lon);
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
    sliderPolarLat->setValue(s.polar_lat);
    sliderPolarLon->setValue(s.polar_lon);
    liveSuspended=false;
}

void MainWindow::M_Save_Settings(){
    QString filename = QFileDialog::getSaveFileName(this,
                                tr("Save file"),
                                startPath(AppKeys::dirSettings),
                                tr("Texts (*.json);;All files (*.*)"));
    if (filename.isEmpty()) return;
    rememberPath(AppKeys::dirSettings, filename);
    Settings_Get();
    if (!s.Save(filename)) QMessageBox::critical(nullptr,tr("Error"),tr("0001 unable to save file"));
}
void MainWindow::M_Load_Settings(){
    QString filename = QFileDialog::getOpenFileName(this,
                                tr("Open file"),
                                startPath(AppKeys::dirSettings),
                                tr("Texts (*.json);;All files (*.*)"));
    if (filename.isEmpty()) return;
    rememberPath(AppKeys::dirSettings, filename);
    QFile file(filename);
    if (!file.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::critical(nullptr,tr("Error"),tr("0002 unable to load file"));
        return;
    }
    Settings_Get();
    if (s.JSON_deserialize(QJsonDocument::fromJson(file.readAll()).object()))
    {
        Settings_Set();
        update();
    }
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
        Gen(false,&planet);
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

void MainWindow::scheduleLivePreview()
{
    if (liveSuspended || !ui->action_live->isChecked() || !liveTimer)
        return;
    liveTimer->start();
}

void MainWindow::runLivePreview()
{
    if (liveSuspended || !ui->action_live->isChecked())
        return;
    Settings_Get();
    if (isEmtyPlanet)
    {
        Gen(true, &planet);
        isEmtyPlanet = false;
        applyPlanetToView();
        return;
    }
    if (appearanceOnlyLive && glWidget)
    {
        planet.s = s;
        const SphereVec3 shine = latLonDegToSphere(s.shine_lat, s.shine_lon);
        planet.x_shine = shine.x;
        planet.y_shine = shine.y;
        planet.z_shine = shine.z;
        const SphereVec3 polar = latLonDegToSphere(s.polar_lat, s.polar_lon);
        planet.x_polar = polar.x;
        planet.y_polar = polar.y;
        planet.z_polar = polar.z;
        glWidget->refreshAppearance();
        return;
    }
    Gen(false, &planet);
    applyPlanetToView();
}

void MainWindow::addLatLonControls(QWidget *parent,
                                  QSlider *&latSlider, QSlider *&lonSlider,
                                  QLabel *&latTitle, QLabel *&lonTitle,
                                  QLabel *&latValue, QLabel *&lonValue)
{
    auto makeTitle = [parent](int y) {
        QLabel *label = new QLabel(parent);
        label->setGeometry(20, y, 200, 24);
        QFont font("Consolas", 10);
        label->setFont(font);
        return label;
    };
    auto makeValue = [parent](int y) {
        QLabel *label = new QLabel("0", parent);
        label->setGeometry(240, y, 50, 24);
        QFont font("Consolas", 10);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);
        return label;
    };
    auto makeSlider = [parent](int y, int min, int max, int value) {
        QSlider *slider = new QSlider(Qt::Horizontal, parent);
        slider->setGeometry(20, y, 221, 16);
        slider->setRange(min, max);
        slider->setValue(value);
        return slider;
    };

    latTitle = makeTitle(110);
    latSlider = makeSlider(140, -90, 90, 0);
    latValue = makeValue(135);
    lonTitle = makeTitle(180);
    lonSlider = makeSlider(210, -180, 180, 0);
    lonValue = makeValue(205);
    latValue->setGeometry(230, 135, 41, 21);
    lonValue->setGeometry(230, 205, 41, 21);
    latValue->setText(QString::number(latSlider->value()));
    lonValue->setText(QString::number(lonSlider->value()));
    connect(latSlider, &QSlider::valueChanged, latValue, QOverload<int>::of(&QLabel::setNum));
    connect(lonSlider, &QSlider::valueChanged, lonValue, QOverload<int>::of(&QLabel::setNum));
}

void MainWindow::retranslateCoordLabels()
{
    if (!labelShineLatTitle)
        return;
    labelShineLatTitle->setText(tr("Latitude"));
    labelShineLonTitle->setText(tr("Longitude"));
    labelPolarLatTitle->setText(tr("Latitude"));
    labelPolarLonTitle->setText(tr("Longitude"));
    if (btnResetCamera)
        btnResetCamera->setText(tr("Reset camera"));
}

