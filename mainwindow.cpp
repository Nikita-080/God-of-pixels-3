#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "previewpanel.h"
#include "settingspanel.h"
#include "planetglwidget.h"
#include "windowsettings.h"
#include "appsettings.h"
#include "global.h"
#include "spheremath.h"
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QPainter>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QThread>
#include <QDir>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QCloseEvent>
#include <QEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , preview(nullptr)
    , settingsPanel(nullptr)
    , liveTimer(nullptr)
    , loadingDelayTimer(nullptr)
    , genThread(nullptr)
    , genWork(nullptr)
    , isEmtyPlanet(true)
    , liveSuspended(false)
    , appearanceOnlyLive(false)
    , genRunning(false)
    , genQueued(false)
    , genQueuedCreateNew(false)
    , genQueuedSeed(0)
{
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
    ui->tabWidget->setIconSize(QSize(60, 60));
    for (int i = 0; i < 10; ++i)
    {
        ui->tabWidget->setTabIcon(i, QIcon(":/images/res/images/TabIcon" + QString::number(i + 1) + ".png"));
    }

    preview = new PreviewPanel;
    preview->liveCheck()->setChecked(ui->action_live->isChecked());
    settingsPanel = new SettingsPanel(ui->tabWidget);

    liveTimer = new QTimer(this);
    liveTimer->setSingleShot(true);
    liveTimer->setInterval(200);
    connect(liveTimer, &QTimer::timeout, this, &MainWindow::runLivePreview);

    loadingDelayTimer = new QTimer(this);
    loadingDelayTimer->setSingleShot(true);
    loadingDelayTimer->setInterval(250);
    connect(loadingDelayTimer, &QTimer::timeout, this, &MainWindow::showLoadingOverlay);

    connect(ui->btnCreate, &QPushButton::clicked, this, &MainWindow::CreateNewPlanet);
    connect(ui->btnRecreate, &QPushButton::clicked, this, &MainWindow::RecreatePlanet);
    connect(ui->btnAutogen, &QPushButton::clicked, this, &MainWindow::AutoGen);
    connect(ui->btnViewPlanet, &QPushButton::clicked, this, &MainWindow::ShowPlanet);
    connect(ui->btnViewDescription, &QPushButton::clicked, this, &MainWindow::ShowDescription);
    connect(ui->btnViewSystem, &QPushButton::clicked, this, &MainWindow::ShowSystem);
    connect(ui->btnViewMap, &QPushButton::clicked, this, &MainWindow::ShowMap);
    connect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::BiomGrad);
    connect(ui->pushButton_7, &QPushButton::clicked, this, &MainWindow::Img_Report);
    connect(ui->action_3, &QAction::triggered, this, &MainWindow::M_Save_Settings);
    connect(ui->action_4, &QAction::triggered, this, &MainWindow::M_Load_Settings);
    connect(ui->action_5, &QAction::triggered, this, &MainWindow::M_Load_Base_Settings);
    connect(ui->action, &QAction::triggered, this, &MainWindow::M_Save_Image);
    connect(ui->action_2, &QAction::triggered, this, &MainWindow::M_Save_Planet);
    connect(ui->action_6, &QAction::triggered, this, &MainWindow::M_About);
    connect(ui->action_8, &QAction::triggered, this, &MainWindow::M_Save_Full_Image);
    connect(ui->action_9, &QAction::triggered, this, &MainWindow::M_Load_Planet);
    connect(ui->action_10, &QAction::triggered, this, &MainWindow::M_Switch_Language);
    connect(ui->action_live, &QAction::toggled, this, [](bool on) {
        QSettings().setValue(AppKeys::livePreview, on);
    });
    connect(preview->liveCheck(), &QCheckBox::toggled, ui->action_live, &QAction::setChecked);
    connect(ui->action_live, &QAction::toggled, preview->liveCheck(), &QCheckBox::setChecked);
    connect(settingsPanel, &SettingsPanel::settingsChanged, this, [this](bool appearanceOnly) {
        appearanceOnlyLive = appearanceOnly;
        scheduleLivePreview();
    });

    setupMainLayout();
    SetStyle();

    Settings_Get();
    s.Load(":/txt_files/res/txt_files/settingsbase.json");
    Settings_Set();

    ui->pushButton_6->hide();
    ui->pushButton_7->hide();
    setWindowTitle("God of Pixels 3");

    const QByteArray geo = st.value(AppKeys::windowGeometry).toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);
}

MainWindow::~MainWindow()
{
    if (genThread)
    {
        disconnect(genThread, nullptr, this, nullptr);
        genThread->wait();
        delete genThread;
        genThread = nullptr;
    }
    delete genWork;
    genWork = nullptr;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings().setValue(AppKeys::windowGeometry, saveGeometry());
    QMainWindow::closeEvent(event);
}

QString MainWindow::ReadText(QString path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
        return QString::fromUtf8(file.readAll());
    return QString();
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
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if (settingsPanel)
            settingsPanel->retranslate();
        if (preview)
            preview->retranslate();
    }
}

void MainWindow::M_About()
{
    QMessageBox msb;
    QString message;
    message.append(tr("name - God of Pixels 3") + "\n");
    message.append(tr("version - "));
    message.append(PROGRAM_VERSION);
    message.append("\n");
    message.append(tr("author - Riabov Nikita") + "\n");
    message.append(tr("feedback - riabovnick080@yandex.ru"));
    msb.setText(message);
    msb.exec();
}

void MainWindow::SetStyle()
{
    QFile f(":/css_files/res/css_files/app.qss");
    if (f.open(QIODevice::ReadOnly))
    {
        const QString sheet = QString::fromUtf8(f.readAll());
        qApp->setStyleSheet(sheet);
        setStyleSheet(sheet);
    }
    ui->btnLogo->setIcon(QIcon(":/images/res/images/logo.png"));
}

void MainWindow::setupMainLayout()
{
    ui->line->hide();
    ui->line_3->hide();
    ui->line_4->hide();
    ui->pushButton_2->hide();
    ui->labelPlanetName->hide();

    ui->btnCreate->setStyleSheet(QString());
    ui->btnRecreate->setStyleSheet(QString());
    ui->btnCreate->setFixedWidth(480);
    ui->btnCreate->setMinimumHeight(52);
    ui->btnCreate->setMaximumHeight(64);
    ui->btnRecreate->setFixedWidth(480);
    ui->btnRecreate->setMinimumHeight(52);
    ui->btnRecreate->setMaximumHeight(64);
    ui->btnAutogen->setFixedSize(240, 80);
    ui->btnViewPlanet->setFixedSize(122, 122);
    ui->btnViewDescription->setFixedSize(122, 122);
    ui->btnViewSystem->setFixedSize(122, 122);
    ui->btnViewMap->setFixedSize(122, 122);
    ui->btnLogo->setFixedSize(150, 150);
    ui->progressAutogen->setFixedHeight(31);
    ui->progressAutogenSub->setFixedHeight(31);
    ui->progressBar->setFixedHeight(31);

    auto *progressCol = new QVBoxLayout;
    progressCol->setSpacing(8);
    progressCol->addWidget(ui->progressAutogen);
    progressCol->addWidget(ui->progressAutogenSub);
    progressCol->addStretch();

    auto *autogenRow = new QHBoxLayout;
    autogenRow->setSpacing(10);
    autogenRow->addWidget(ui->btnAutogen);
    autogenRow->addLayout(progressCol, 1);

    auto *viewRow = new QHBoxLayout;
    viewRow->setSpacing(0);
    viewRow->addWidget(ui->btnViewPlanet);
    viewRow->addWidget(ui->btnViewDescription);
    viewRow->addWidget(ui->btnViewSystem);
    viewRow->addWidget(ui->btnViewMap);
    viewRow->addStretch();

    auto *actionsCol = new QVBoxLayout;
    actionsCol->setSpacing(8);
    actionsCol->addWidget(ui->btnCreate);
    actionsCol->addWidget(ui->btnRecreate);
    actionsCol->addLayout(autogenRow);
    actionsCol->addLayout(viewRow);
    actionsCol->addWidget(ui->btnLogo, 0, Qt::AlignHCenter);
    actionsCol->addStretch();

    auto *previewCol = new QVBoxLayout;
    previewCol->setSpacing(8);
    previewCol->addWidget(preview, 1);
    previewCol->addWidget(ui->progressBar);

    auto *root = new QHBoxLayout(ui->centralwidget);
    root->setContentsMargins(10, 8, 10, 8);
    root->setSpacing(12);
    root->addWidget(settingsPanel, 0);
    root->addLayout(previewCol, 1);
    root->addLayout(actionsCol, 0);

    setMinimumSize(1280, 620);
    resize(1500, 680);
}

void MainWindow::AutoGen()
{
    windowsettings win(language, this);
    if (win.exec() != QDialog::Accepted)
        return;
    box = win.settings();
    beginLoadingWatch();
    ui->btnAutogen->setEnabled(false);
    ui->progressAutogen->setValue(0);
    Settings_Get();
    if (box.save_type == 1)
    {
        int delta = box.picturetype ? 658 : 257;
        int count = 0;
        const double percent = 1.0 * box.height * box.width / 100;
        QImage image(box.width * delta, box.height * delta, QImage::Format_RGB32);
        QPainter p;
        p.begin(&image);
        for (int i = 0; i < box.height; ++i)
        {
            for (int k = 0; k < box.width; ++k)
            {
                s.Random(box.isRndList);
                Gen(true, &autoplanet);
                if (box.picturetype)
                    p.drawImage(k * delta, i * delta, autoplanet.img_final);
                else
                    p.drawImage(k * delta, i * delta, autoplanet.img_nonscale);
                ++count;
                ui->progressAutogen->setValue(qRound(1.0 * count / percent));
                QApplication::processEvents();
            }
        }
        image.save(box.path);
        ui->progressAutogen->setValue(100);
    }
    else
    {
        const double percent = 1.0 * box.number / 100;
        for (int k = 0; k < box.number; ++k)
        {
            s.Random(box.isRndList);
            Gen(true, &autoplanet);
            ui->progressAutogen->setValue(qRound(1.0 * k / percent));
            QApplication::processEvents();
            QImage photo = box.picturetype ? autoplanet.img_final : autoplanet.img_nonscale;
            photo.save(QDir(box.path).filePath(autoplanet.name + ".png"));
        }
        ui->progressAutogen->setValue(100);
    }
    ui->btnAutogen->setEnabled(true);
    endLoadingWatch();
}

void MainWindow::CreateNewPlanet()
{
    Settings_Get();
    startGeneration(true);
}

void MainWindow::RecreatePlanet()
{
    Settings_Get();
    startGeneration(isEmtyPlanet);
}

void MainWindow::applyPlanetToView()
{
    if (!preview)
        return;
    preview->setHasPlanet(true);
    preview->glWidget()->setPlanet(&planet);
    preview->glWidget()->refreshTextures();
    preview->glWidget()->repaint();
    const QImage shot = preview->glWidget()->captureView();
    if (!shot.isNull())
        planet.img_view = shot;
    planet.FinalImage();
    ShowPlanet();
    preview->setPlanetName(planet.name);
}

void MainWindow::ShowPlanet()
{
    if (preview)
        preview->showGlobe();
}

void MainWindow::ShowDescription()
{
    if (isEmtyPlanet || !preview)
        return;
    preview->showCard(planet.img_dsc);
}

void MainWindow::ShowSystem()
{
    if (isEmtyPlanet || !preview)
        return;
    preview->showCard(planet.img_sys);
}

void MainWindow::ShowMap()
{
    if (isEmtyPlanet || !preview)
        return;
    preview->showCard(planet.img_gal);
}

void MainWindow::M_Save_Image()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save image"),
                                                    startPath(AppKeys::dirImage, planet.name),
                                                    tr("Image (*.png);;All files (*.*)"));
    if (filename.isEmpty())
        return;
    rememberPath(AppKeys::dirImage, filename);
    try
    {
        QImage out = planet.img_view.isNull() ? planet.img : planet.img_view;
        out.save(filename);
    }
    catch (...)
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0001 unable to save file"));
    }
}

void MainWindow::M_Save_Full_Image()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save full image"),
                                                    startPath(AppKeys::dirImage, planet.name),
                                                    tr("Image (*.png);;All files (*.*)"));
    if (filename.isEmpty())
        return;
    rememberPath(AppKeys::dirImage, filename);
    try
    {
        planet.img_final.save(filename);
    }
    catch (...)
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0001 unable to save file"));
    }
}

void MainWindow::M_Load_Planet()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Load planet"),
                                                    startPath(AppKeys::dirPlanet),
                                                    tr("Planet (*.planet);;All files (*.*)"));
    if (filename.isEmpty())
        return;
    rememberPath(AppKeys::dirPlanet, filename);
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    try
    {
        const QString a = file.readAll();
        QJsonObject jobject = QJsonDocument::fromJson(a.toUtf8()).object();
        if (!s.JSON_deserialize(jobject["settings"].toObject()))
        {
            file.close();
            return;
        }
        Settings_Set();
        startGeneration(true, jobject["seed"].toInt());
    }
    catch (...)
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0002 unable to load file"));
    }
    file.close();
}

void MainWindow::M_Save_Planet()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save planet"),
                                                    startPath(AppKeys::dirPlanet, planet.name),
                                                    tr("Planet (*.planet);;All files (*.*)"));
    if (filename.isEmpty())
        return;
    rememberPath(AppKeys::dirPlanet, filename);
    QFile file(filename);
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QJsonObject jobject;
        jobject["seed"] = planet.seed;
        jobject["settings"] = planet.s.JSON_serialize();
        QTextStream stream(&file);
        stream << QJsonDocument(jobject).toJson();
        file.close();
    }
    else
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0001 unable to save file"));
    }
}

void MainWindow::Gen(bool isCreateNew, Planet *p, int seed)
{
    p->s = s;
    if (isCreateNew)
        p->SetSeed(seed);
    p->Generate();
}

void MainWindow::beginLoadingWatch()
{
    if (loadingDelayTimer)
        loadingDelayTimer->start();
}

void MainWindow::showLoadingOverlay()
{
    if (preview)
        preview->setLoading(true);
}

void MainWindow::endLoadingWatch()
{
    if (loadingDelayTimer)
        loadingDelayTimer->stop();
    if (preview)
        preview->setLoading(false);
}

void MainWindow::startGeneration(bool createNew, int seed)
{
    if (genRunning)
    {
        genQueued = true;
        genQueuedCreateNew = createNew;
        genQueuedSeed = seed;
        return;
    }

    genRunning = true;
    beginLoadingWatch();

    auto *work = new Planet;
    genWork = work;
    work->s = s;
    if (createNew)
        work->SetSeed(seed);

    QThread *thread = QThread::create([work]() { work->Generate(); });
    genThread = thread;
    connect(thread, &QThread::finished, this, [this, work, thread]() {
        if (genThread == thread)
            genThread = nullptr;
        thread->deleteLater();
        if (genQueued)
        {
            delete work;
            if (genWork == work)
                genWork = nullptr;
            genRunning = false;
            const bool queuedCreate = genQueuedCreateNew;
            const int queuedSeed = genQueuedSeed;
            genQueued = false;
            startGeneration(queuedCreate, queuedSeed);
            return;
        }
        planet = *work;
        delete work;
        if (genWork == work)
            genWork = nullptr;
        isEmtyPlanet = false;
        applyPlanetToView();
        genRunning = false;
        endLoadingWatch();
        if (genQueued)
        {
            const bool queuedCreate = genQueuedCreateNew;
            const int queuedSeed = genQueuedSeed;
            genQueued = false;
            startGeneration(queuedCreate, queuedSeed);
        }
    });
    thread->start();
}

void MainWindow::M_Load_Base_Settings()
{
    Settings_Get();
    if (s.Load(":/txt_files/res/txt_files/settingsbase.json"))
        Settings_Set();
    else
        QMessageBox::critical(nullptr, tr("Error"), tr("0003 unable to load default settings"));
}

void MainWindow::Settings_Get()
{
    if (settingsPanel)
        settingsPanel->collect(s);
}

void MainWindow::Settings_Set()
{
    liveSuspended = true;
    if (settingsPanel)
        settingsPanel->push(s);
    liveSuspended = false;
}

void MainWindow::M_Save_Settings()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save file"),
                                                    startPath(AppKeys::dirSettings),
                                                    tr("Texts (*.json);;All files (*.*)"));
    if (filename.isEmpty())
        return;
    rememberPath(AppKeys::dirSettings, filename);
    Settings_Get();
    if (!s.Save(filename))
        QMessageBox::critical(nullptr, tr("Error"), tr("0001 unable to save file"));
}

void MainWindow::M_Load_Settings()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open file"),
                                                    startPath(AppKeys::dirSettings),
                                                    tr("Texts (*.json);;All files (*.*)"));
    if (filename.isEmpty())
        return;
    rememberPath(AppKeys::dirSettings, filename);
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0002 unable to load file"));
        return;
    }
    Settings_Get();
    if (s.JSON_deserialize(QJsonDocument::fromJson(file.readAll()).object()))
    {
        Settings_Set();
        update();
    }
}

void MainWindow::Img_Report()
{
    QString path = "C:/Users/Никита/Desktop/";
    planet.ImageReport(planet.t_map, QColor(0, 0, 255), QColor(255, 0, 0)).save(path + "t.png");
    planet.ImageReport(planet.matrix, QColor(0, 0, 0), QColor(255, 255, 255)).save(path + "m.png");
    planet.ImageReport(planet.r_map, QColor(252, 221, 118), QColor(0, 0, 255)).save(path + "r.png");
}

void MainWindow::BiomGrad()
{
    for (int i = -50; i <= 50; ++i)
    {
        s.temperature = i;
        Gen(false, &planet);
        QString name = QString::number(i + 50) + " (" + QString::number(i) + ").png";
        planet.img.save("C:/Users/Никита/Desktop/biom/" + name);
    }
}

void MainWindow::Report(QString s)
{
    QFile file("C:/Users/Никита/Desktop/DATA.txt");
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        stream << s << "\n";
        file.close();
    }
}

void MainWindow::scheduleLivePreview()
{
    if (isEmtyPlanet || liveSuspended || !ui->action_live->isChecked() || !liveTimer)
        return;
    liveTimer->start();
}

void MainWindow::runLivePreview()
{
    if (isEmtyPlanet || liveSuspended || !ui->action_live->isChecked())
        return;
    Settings_Get();
    if (appearanceOnlyLive && preview)
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
        preview->glWidget()->refreshAppearance();
        return;
    }
    startGeneration(false);
}
