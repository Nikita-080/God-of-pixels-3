#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "previewpanel.h"
#include "settingspanel.h"
#include "planetglwidget.h"
#include "windowsettings.h"
#include "appsettings.h"
#include "global.h"
#include "spheremath.h"
#include "achievementengine.h"
#include "achievementcontext.h"
#include "achievementstore.h"
#include "achievementtoast.h"
#include "achievementsdialog.h"
#include "programsettingsdialog.h"
#include "planetcommands.h"
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
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
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QCheckBox>
#include <QCloseEvent>
#include <QEvent>
#include <QSize>
#include <QPlainTextEdit>
#include <QCoreApplication>
#include <QStringList>
#include <QPushButton>
#include <QButtonGroup>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QShortcut>
#include <QSplitter>
#include <QUndoStack>
#include <QLabel>
#include <QStatusBar>
#include <QComboBox>
#include <QAbstractSpinBox>
#include <QLineEdit>
#include <QKeySequence>
#include <algorithm>

namespace {

QString uniquePngPath(const QDir &dir, const QString &rawName)
{
    QString base = rawName;
    for (int i = 0; i < base.size(); ++i)
    {
        const QChar c = base.at(i);
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('-')))
            base[i] = QLatin1Char('_');
    }
    if (base.isEmpty())
        base = QStringLiteral("planet");
    QString name = base + QStringLiteral(".png");
    int n = 2;
    while (dir.exists(name))
        name = QStringLiteral("%1_%2.png").arg(base).arg(n++);
    return dir.filePath(name);
}

} // namespace


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , preview(nullptr)
    , settingsPanel(nullptr)
    , mainSplitter(nullptr)
    , cameraStatus(nullptr)
    , liveTimer(nullptr)
    , loadingDelayTimer(nullptr)
    , genThread(nullptr)
    , genWork(nullptr)
    , livePreview(false)
    , isEmtyPlanet(true)
    , liveSuspended(false)
    , appearanceOnlyLive(false)
    , genRunning(false)
    , genQueued(false)
    , genQueuedCreateNew(false)
    , genQueuedSeed(0)
    , restoreViewOnApply(false)
    , autogenRunning(false)
    , autogenGl(nullptr)
    , factsView(nullptr)
    , actionProgramSettings(nullptr)
    , actionSavePlanetAs(nullptr)
    , actionUndo(nullptr)
    , actionRedo(nullptr)
    , achievementToasts(nullptr)
    , undoStack(nullptr)
    , sessionActive(false)
    , undoApplying(false)
    , pendingRecreateUndo(false)
    , genActiveOp(GenOp::None)
    , genQueuedOp(GenOp::None)
{
    QSettings st(appSettingsFile(), QSettings::IniFormat);
    language = st.value(AppKeys::language, QStringLiteral("en")).toString();
    if (language != QStringLiteral("ru"))
        language = QStringLiteral("en");
    livePreview = st.value(AppKeys::livePreview, false).toBool();
    if (language == QStringLiteral("ru"))
    {
        qtLanguageTranslator.load(":/translations/QtLanguage_ru");
        qApp->installTranslator(&qtLanguageTranslator);
    }

    ui->setupUi(this);
    ui->tabWidget->hide();

    preview = new PreviewPanel;
    preview->spinCheck()->setChecked(st.value(AppKeys::globeSpin, false).toBool());
    preview->cropCheck()->setChecked(st.value(AppKeys::cropFrame, true).toBool());
    preview->glWidget()->setSpinning(preview->spinCheck()->isChecked());
    settingsPanel = new SettingsPanel(ui->tabWidget);

    undoStack = new QUndoStack(this);
    undoStack->setUndoLimit(20);

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
    connect(ui->action_2, &QAction::triggered, this, [this]() { M_Save_Planet(); });
    connect(ui->action_6, &QAction::triggered, this, &MainWindow::M_About);
    connect(ui->action_achievements, &QAction::triggered, this, &MainWindow::M_Achievements);
    connect(ui->action_8, &QAction::triggered, this, &MainWindow::M_Save_Full_Image);
    connect(ui->action_9, &QAction::triggered, this, &MainWindow::M_Load_Planet);
    actionSavePlanetAs = new QAction(this);
    if (ui->menu)
        ui->menu->insertAction(ui->action_9, actionSavePlanetAs);
    connect(actionSavePlanetAs, &QAction::triggered, this, [this]() { M_Save_PlanetAs(); });
    if (ui->menuProgramSettings)
        ui->menuProgramSettings->menuAction()->setVisible(false);
    if (ui->action_10)
        ui->action_10->setVisible(false);
    if (ui->action_live)
        ui->action_live->setVisible(false);
    actionProgramSettings = new QAction(this);
    if (ui->menu_2)
    {
        if (ui->menuProgramSettings)
            ui->menu_2->insertAction(ui->menuProgramSettings->menuAction(), actionProgramSettings);
        else
            ui->menu_2->addAction(actionProgramSettings);
    }
    connect(actionProgramSettings, &QAction::triggered, this, &MainWindow::M_ProgramSettings);
    connect(preview->spinCheck(), &QCheckBox::toggled, this, [](bool on) {
        QSettings(appSettingsFile(), QSettings::IniFormat).setValue(AppKeys::globeSpin, on);
    });
    connect(preview->spinCheck(), &QCheckBox::toggled, preview->glWidget(), &PlanetGLWidget::setSpinning);
    connect(preview->cropCheck(), &QCheckBox::toggled, this, [](bool on) {
        QSettings(appSettingsFile(), QSettings::IniFormat).setValue(AppKeys::cropFrame, on);
    });
    connect(preview->glWidget(), &PlanetGLWidget::cameraChanged, this, &MainWindow::updateCameraStatus);
    connect(settingsPanel, &SettingsPanel::settingsChanged, this, [this](bool appearanceOnly) {
        appearanceOnlyLive = appearanceOnly;
        scheduleLivePreview();
    });
    connect(settingsPanel, &SettingsPanel::settingsCommitted, this, &MainWindow::commitSettingsUndo);

    actionUndo = undoStack->createUndoAction(this);
    actionRedo = undoStack->createRedoAction(this);
    actionUndo->setShortcut(QKeySequence::Undo);
    actionRedo->setShortcuts({QKeySequence(QStringLiteral("Ctrl+Shift+Z")), QKeySequence::Redo});
    addAction(actionUndo);
    addAction(actionRedo);
    connect(undoStack, &QUndoStack::cleanChanged, this, [this](bool) {
        updateWindowTitle();
        updateUndoActions();
    });
    connect(undoStack, &QUndoStack::canUndoChanged, this, [this](bool) { updateUndoActions(); });
    connect(undoStack, &QUndoStack::canRedoChanged, this, [this](bool) { updateUndoActions(); });

    setupMainLayout();
    setupShortcuts();
    SetStyle();
    retranslateExtras();
    achievementToasts = new AchievementToastHost(this);

    Settings_Get();
    s.Load(":/txt_files/res/txt_files/settingsbase.json");
    Settings_Set();
    lastCommittedSettings = s;

    ui->pushButton_6->hide();
    ui->pushButton_7->hide();
    updateWindowTitle();
    updateCameraStatus();
    updateUndoActions();

    const QByteArray geo = st.value(AppKeys::windowGeometry).toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);
}

MainWindow::~MainWindow()
{
    autogenRunning = false;
    if (genThread)
    {
        disconnect(genThread, nullptr, this, nullptr);
        genThread->wait();
        delete genThread;
        genThread = nullptr;
    }
    delete genWork;
    genWork = nullptr;
    delete autogenGl;
    autogenGl = nullptr;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!confirmAbandonSession())
    {
        event->ignore();
        return;
    }
    QSettings st(appSettingsFile(), QSettings::IniFormat);
    st.setValue(AppKeys::windowGeometry, saveGeometry());
    if (mainSplitter)
        st.setValue(AppKeys::splitter, mainSplitter->saveState());
    QMainWindow::closeEvent(event);
}

QString MainWindow::ReadText(QString path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
        return QString::fromUtf8(file.readAll());
    return QString();
}

void MainWindow::M_ProgramSettings()
{
    ProgramSettingsDialog dlg(language, livePreview, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    livePreview = dlg.livePreview();
    QSettings st(appSettingsFile(), QSettings::IniFormat);
    st.setValue(AppKeys::livePreview, livePreview);
    applyLanguage(dlg.language());
}

void MainWindow::applyLanguage(const QString &lang)
{
    QString next = lang;
    if (next != QStringLiteral("ru"))
        next = QStringLiteral("en");
    if (next == language)
    {
        QSettings(appSettingsFile(), QSettings::IniFormat).setValue(AppKeys::language, language);
        return;
    }
    if (next == QStringLiteral("ru"))
    {
        qtLanguageTranslator.load(":/translations/QtLanguage_ru");
        qApp->installTranslator(&qtLanguageTranslator);
    }
    else
    {
        qApp->removeTranslator(&qtLanguageTranslator);
    }
    language = next;
    QSettings(appSettingsFile(), QSettings::IniFormat).setValue(AppKeys::language, language);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        retranslateExtras();
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

void MainWindow::M_Achievements()
{
    AchievementsDialog dlg(this);
    dlg.exec();
}

void MainWindow::evaluateAchievements(bool countCreate, bool planetSaved, bool autogenAllRandom)
{
    if (!achievementToasts)
        return;
    if (isEmtyPlanet && !planetSaved && !autogenAllRandom)
        return;
    AchievementContext ctx;
    if (!isEmtyPlanet)
        ctx = AchievementContext::fromPlanet(planet);
    ctx.createdCount = countCreate ? AchievementStore::addCreatedPlanet()
                                   : AchievementStore::createdCount();
    ctx.planetSaved = planetSaved;
    ctx.autogenAllRandom = autogenAllRandom;
    achievementToasts->enqueue(AchievementEngine::evaluate(ctx));
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
    ui->btnLogo->hide();
}

void MainWindow::setupMainLayout()
{
    ui->line->hide();
    ui->line_3->hide();
    ui->line_4->hide();
    ui->pushButton_2->hide();
    ui->labelPlanetName->hide();
    ui->btnLogo->hide();

    auto sizeActionButton = [](QPushButton *btn) {
        btn->setStyleSheet(QString());
        btn->setMinimumWidth(0);
        btn->setMaximumWidth(QWIDGETSIZE_MAX);
        btn->setMinimumHeight(40);
        btn->setMaximumHeight(48);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setIcon(QIcon());
        btn->setIconSize(QSize(0, 0));
    };
    sizeActionButton(ui->btnCreate);
    sizeActionButton(ui->btnRecreate);
    sizeActionButton(ui->btnAutogen);
    ui->btnCreate->setDefault(false);
    ui->btnCreate->setAutoDefault(false);
    ui->btnRecreate->setDefault(false);
    ui->btnRecreate->setAutoDefault(false);

    auto sizeViewButton = [](QPushButton *btn) {
        btn->setStyleSheet(QString());
        btn->setIcon(QIcon());
        btn->setIconSize(QSize(0, 0));
        btn->setCheckable(true);
        btn->setAutoExclusive(false);
        btn->setMinimumHeight(40);
        btn->setMaximumHeight(48);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    };
    sizeViewButton(ui->btnViewPlanet);
    sizeViewButton(ui->btnViewDescription);
    sizeViewButton(ui->btnViewSystem);
    sizeViewButton(ui->btnViewMap);
    auto *viewGroup = new QButtonGroup(this);
    viewGroup->setExclusive(true);
    viewGroup->addButton(ui->btnViewPlanet);
    viewGroup->addButton(ui->btnViewDescription);
    viewGroup->addButton(ui->btnViewSystem);
    viewGroup->addButton(ui->btnViewMap);
    ui->btnViewPlanet->setChecked(true);
    updateViewButtonsEnabled();

    factsView = new QPlainTextEdit;
    factsView->setObjectName(QStringLiteral("factsCard"));
    factsView->setReadOnly(true);
    factsView->setUndoRedoEnabled(false);
    factsView->setMinimumHeight(80);
    factsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    factsView->setFont(QFont(QStringLiteral("Consolas"), 10));

    auto *viewGrid = new QGridLayout;
    viewGrid->setSpacing(8);
    viewGrid->addWidget(ui->btnViewPlanet, 0, 0);
    viewGrid->addWidget(ui->btnViewDescription, 0, 1);
    viewGrid->addWidget(ui->btnViewSystem, 1, 0);
    viewGrid->addWidget(ui->btnViewMap, 1, 1);

    auto *actionsWrap = new QWidget;
    actionsWrap->setObjectName(QStringLiteral("actionsWrap"));
    actionsWrap->setAttribute(Qt::WA_StyledBackground, true);
    actionsWrap->setMinimumWidth(220);
    actionsWrap->setMaximumWidth(420);
    auto *actionsCol = new QVBoxLayout(actionsWrap);
    actionsCol->setContentsMargins(10, 10, 10, 10);
    actionsCol->setSpacing(8);
    actionsCol->addWidget(ui->btnCreate);
    actionsCol->addWidget(ui->btnRecreate);
    actionsCol->addWidget(ui->btnAutogen);
    actionsCol->addLayout(viewGrid);
    actionsCol->addWidget(factsView, 1);

    auto *previewCol = new QVBoxLayout;
    previewCol->setSpacing(8);
    previewCol->addWidget(preview, 1);
    auto *previewWrap = new QWidget;
    previewWrap->setLayout(previewCol);

    mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setObjectName(QStringLiteral("mainSplitter"));
    mainSplitter->setChildrenCollapsible(false);
    settingsPanel->setMinimumWidth(260);
    mainSplitter->addWidget(settingsPanel);
    mainSplitter->addWidget(previewWrap);
    mainSplitter->addWidget(actionsWrap);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setStretchFactor(2, 0);
    mainSplitter->setSizes({320, 800, 300});

    auto *root = new QHBoxLayout(ui->centralwidget);
    root->setContentsMargins(10, 8, 10, 8);
    root->setSpacing(0);
    root->addWidget(mainSplitter);

    cameraStatus = new QLabel;
    cameraStatus->setObjectName(QStringLiteral("cameraStatus"));
    statusBar()->addPermanentWidget(cameraStatus);

    setMinimumSize(1200, 620);
    resize(1500, 680);

    const QByteArray split = QSettings(appSettingsFile(), QSettings::IniFormat).value(AppKeys::splitter).toByteArray();
    if (!split.isEmpty())
        mainSplitter->restoreState(split);
}

void MainWindow::retranslateExtras()
{
    ui->btnAutogen->setText(tr("Autogen"));
    ui->btnViewPlanet->setText(tr("Planet"));
    ui->btnViewDescription->setText(tr("Description"));
    ui->btnViewSystem->setText(tr("Tags"));
    ui->btnViewMap->setText(tr("Map"));
    ui->btnCreate->setToolTip(tr("Create (%1)").arg(QKeySequence(Qt::Key_Return).toString(QKeySequence::NativeText)));
    ui->btnRecreate->setToolTip(tr("Recreate (%1)").arg(QKeySequence(Qt::CTRL | Qt::Key_Return).toString(QKeySequence::NativeText)));
    ui->btnAutogen->setToolTip(tr("Autogen (%1)").arg(QKeySequence(QStringLiteral("Ctrl+G")).toString(QKeySequence::NativeText)));
    ui->btnViewPlanet->setToolTip(tr("Planet (1)"));
    ui->btnViewDescription->setToolTip(tr("Description (2)"));
    ui->btnViewSystem->setToolTip(tr("Tags (3)"));
    ui->btnViewMap->setToolTip(tr("Map (4)"));
    if (actionProgramSettings)
        actionProgramSettings->setText(tr("Program settings"));
    if (actionSavePlanetAs)
        actionSavePlanetAs->setText(tr("Save planet as"));
    if (actionUndo)
        actionUndo->setText(tr("Undo"));
    if (actionRedo)
        actionRedo->setText(tr("Redo"));
    updateFactsCard();
    updateWindowTitle();
    updateCameraStatus();
}

void MainWindow::setupShortcuts()
{
    ui->action->setShortcut(QKeySequence(QStringLiteral("Ctrl+E")));
    ui->action_8->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+E")));
    ui->action_2->setShortcut(QKeySequence::Save);
    ui->action_9->setShortcut(QKeySequence::Open);
    if (actionSavePlanetAs)
        actionSavePlanetAs->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+S")));

    auto *enterCut = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    enterCut->setContext(Qt::WindowShortcut);
    connect(enterCut, &QShortcut::activated, this, [this]() {
        if (inputTakesDigits())
            return;
        CreateNewPlanet();
    });
    auto *createCut = new QShortcut(QKeySequence(Qt::Key_Return), this);
    createCut->setContext(Qt::WindowShortcut);
    connect(createCut, &QShortcut::activated, this, [this]() {
        if (inputTakesDigits())
            return;
        CreateNewPlanet();
    });
    auto *recreateCut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);
    recreateCut->setContext(Qt::WindowShortcut);
    connect(recreateCut, &QShortcut::activated, this, &MainWindow::RecreatePlanet);

    auto *autogenCut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+G")), this);
    connect(autogenCut, &QShortcut::activated, this, &MainWindow::AutoGen);

    auto bindView = [this](int key, void (MainWindow::*slot)()) {
        auto *cut = new QShortcut(QKeySequence(key), this);
        cut->setContext(Qt::WindowShortcut);
        connect(cut, &QShortcut::activated, this, [this, slot]() {
            if (inputTakesDigits())
                return;
            (this->*slot)();
        });
    };
    bindView(Qt::Key_1, &MainWindow::ShowPlanet);
    bindView(Qt::Key_2, &MainWindow::ShowDescription);
    bindView(Qt::Key_3, &MainWindow::ShowSystem);
    bindView(Qt::Key_4, &MainWindow::ShowMap);

    auto *resetCut = new QShortcut(QKeySequence(Qt::Key_R), this);
    resetCut->setContext(Qt::WindowShortcut);
    connect(resetCut, &QShortcut::activated, this, [this]() {
        if (inputTakesDigits())
            return;
        if (preview && preview->isGlobeVisible() && preview->glWidget())
            preview->glWidget()->resetCamera();
    });
}

bool MainWindow::inputTakesDigits() const
{
    QWidget *f = QApplication::focusWidget();
    return qobject_cast<QComboBox *>(f)
        || qobject_cast<QAbstractSpinBox *>(f)
        || qobject_cast<QLineEdit *>(f)
        || qobject_cast<QPlainTextEdit *>(f);
}

void MainWindow::updateCameraStatus()
{
    if (!cameraStatus || !preview || !preview->glWidget())
        return;
    PlanetGLWidget *gl = preview->glWidget();
    cameraStatus->setText(tr("Zoom %1%%    Az %2°    El %3°")
                              .arg(int(qRound(double(gl->zoomPercent()))))
                              .arg(double(gl->azimuthAngle()), 0, 'f', 1)
                              .arg(double(gl->elevationAngle()), 0, 'f', 1));
}

void MainWindow::updateWindowTitle()
{
    QString title = QStringLiteral("God of Pixels 3");
    if (sessionActive)
    {
        QString name;
        if (!sessionPath.isEmpty())
            name = QFileInfo(sessionPath).fileName();
        else if (!isEmtyPlanet)
            name = planet.name;
        if (!name.isEmpty())
            title += QStringLiteral(" — ") + name;
        if (sessionDirty())
            title += QStringLiteral(" *");
    }
    setWindowTitle(title);
}

bool MainWindow::sessionDirty() const
{
    if (!sessionActive)
        return false;
    if (sessionPath.isEmpty())
        return true;
    return undoStack && !undoStack->isClean();
}

void MainWindow::updateUndoActions()
{
    const bool on = sessionActive && !undoApplying;
    if (actionUndo)
        actionUndo->setEnabled(on && undoStack && undoStack->canUndo());
    if (actionRedo)
        actionRedo->setEnabled(on && undoStack && undoStack->canRedo());
}

void MainWindow::beginSession(const QString &path, bool)
{
    sessionActive = true;
    sessionPath = path;
    if (undoStack)
    {
        undoStack->clear();
        undoStack->setClean();
    }
    lastCommittedSettings = s;
    updateWindowTitle();
    updateUndoActions();
}

bool MainWindow::confirmAbandonSession()
{
    if (!sessionDirty())
        return true;
    const QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        tr("Unsaved changes"),
        tr("The current planet has unsaved changes."),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
    if (choice == QMessageBox::Cancel)
        return false;
    if (choice == QMessageBox::Save)
        return M_Save_Planet();
    return true;
}

void MainWindow::applySettingsFromUndo(const PlanetSettings &settings, bool appearanceOnly)
{
    undoApplying = true;
    s = settings;
    lastCommittedSettings = settings;
    Settings_Set();
    appearanceOnlyLive = appearanceOnly;
    undoApplying = false;
    scheduleLivePreview();
    updateWindowTitle();
}

void MainWindow::applyPlanetFromUndo(const Planet &p)
{
    undoApplying = true;
    planet = p;
    s = planet.s;
    lastCommittedSettings = s;
    Settings_Set();
    isEmtyPlanet = false;
    applyPlanetToView();
    undoApplying = false;
    updateWindowTitle();
}

void MainWindow::commitSettingsUndo(bool appearanceOnly)
{
    if (undoApplying || !sessionActive || !settingsPanel || !undoStack)
        return;
    Settings_Get();
    if (s.JSON_serialize() == lastCommittedSettings.JSON_serialize())
        return;
    auto apply = [this](const PlanetSettings &next, bool appearance) {
        applySettingsFromUndo(next, appearance);
    };
    undoStack->push(new SettingsUndoCommand(lastCommittedSettings, s, appearanceOnly, apply));
    lastCommittedSettings = s;
    updateWindowTitle();
}

void MainWindow::updateFactsCard()
{
    if (!factsView)
        return;
    if (isEmtyPlanet)
    {
        factsView->setPlainText(tr("Create a planet to see facts"));
        return;
    }
    auto planetTr = [](const char *source) {
        return QCoreApplication::translate("Planet", source);
    };
    QStringList lines;
    lines << tr("seed        - %1").arg(planet.seed);
    lines << planetTr("resources  - ") + planet.facts.resources;
    lines << planetTr("life         ") + QString::number(planet.facts.life) + QStringLiteral("/12");
    lines << planetTr("water        ") + QString::number(planet.facts.water) + QStringLiteral("/12");
    lines << planetTr("ice          ") + QString::number(planet.facts.ice) + QStringLiteral("/12");
    lines << planetTr("radiation    ") + QString::number(planet.facts.radiation) + QStringLiteral("/12");
    lines << planetTr("temperature  ") + QString::number(planet.facts.temperature) + QStringLiteral("/12");
    lines << planetTr("seismicity   ") + QString::number(planet.facts.seismicity) + QStringLiteral("/12");
    QStringList tags;
    for (const QString &key : planet.cardLabelKeys)
    {
        if (key.isEmpty())
            continue;
        tags << QCoreApplication::translate("PlanetTags", key.toUtf8().constData());
    }
    tags.sort();
    if (!tags.isEmpty())
    {
        lines << QString();
        lines << tags.join(QStringLiteral(" . "));
    }
    factsView->setPlainText(lines.join(QLatin1Char('\n')));
}

void MainWindow::AutoGen()
{
    if (autogenRunning || genRunning)
        return;

    windowsettings win(language, this);
    connect(&win, &windowsettings::stopRequested, this, [this]() {
        autogenRunning = false;
    });
    connect(&win, &windowsettings::runRequested, this, [this, &win]() {
        runAutogenWithDialog(&win);
    });
    win.exec();
}

void MainWindow::runAutogenWithDialog(windowsettings *dlg)
{
    if (!dlg || autogenRunning || genRunning)
        return;

    box = dlg->settings();
    const QVector<bool> flags = box.flagVector();

    autogenRunning = true;
    showLoadingOverlay();
    ui->btnAutogen->setEnabled(false);
    ui->btnCreate->setEnabled(false);
    ui->btnRecreate->setEnabled(false);
    dlg->setBusy(true);

    Settings_Get();
    const PlanetSettings base = s;
    bool ok = true;
    const int total = (box.mode == AutoGenMode::Collage)
                          ? qMax(1, box.width * box.height)
                          : qMax(1, box.number);
    dlg->setProgress(0, total);

    auto stepDone = [dlg](int done, int totalCount) {
        dlg->setProgress(done, totalCount);
        QApplication::processEvents();
    };

    if (box.mode == AutoGenMode::Collage)
    {
        QVector<QImage> tiles;
        tiles.reserve(total);
        int done = 0;
        for (int row = 0; row < box.height && autogenRunning; ++row)
        {
            for (int col = 0; col < box.width && autogenRunning; ++col)
            {
                PlanetSettings next = base;
                next.Random(flags);
                s = next;
                Gen(true, &autoplanet);
                tiles.append(autogenPreviewTile());
                ++done;
                stepDone(done, total);
            }
        }
        s = base;

        if (!autogenRunning)
            ok = false;
        else
        {
            int cellW = 0;
            int cellH = 0;
            for (const QImage &tile : tiles)
            {
                cellW = qMax(cellW, tile.width());
                cellH = qMax(cellH, tile.height());
            }
            if (cellW <= 0 || cellH <= 0)
            {
                QMessageBox::critical(dlg, tr("Error"), tr("0001 unable to save file"));
                ok = false;
            }
            else
            {
                QImage image(cellW * box.width, cellH * box.height, QImage::Format_RGB32);
                image.fill(Qt::black);
                QPainter p(&image);
                for (int i = 0; i < tiles.size(); ++i)
                {
                    const int row = i / box.width;
                    const int col = i % box.width;
                    const QImage &tile = tiles.at(i);
                    const int x = col * cellW + (cellW - tile.width()) / 2;
                    const int y = row * cellH + (cellH - tile.height()) / 2;
                    p.drawImage(x, y, tile);
                }
                p.end();
                if (!image.save(box.path))
                {
                    QMessageBox::critical(dlg, tr("Error"), tr("0001 unable to save file"));
                    ok = false;
                }
            }
        }
    }
    else
    {
        const QDir dir(box.path);
        for (int k = 0; k < box.number && autogenRunning; ++k)
        {
            PlanetSettings next = base;
            next.Random(flags);
            s = next;
            Gen(true, &autoplanet);
            const QImage photo = autogenPreviewTile();
            const QString filename = uniquePngPath(dir, autoplanet.name);
            if (!photo.save(filename))
            {
                QMessageBox::critical(dlg, tr("Error"), tr("0001 unable to save file"));
                ok = false;
                break;
            }
            stepDone(k + 1, total);
        }
        s = base;
    }

    if (ok && autogenRunning && box.allFlagsOn())
        evaluateAchievements(false, false, true);
    if (ok && autogenRunning)
        dlg->setLastOutputPath(box.path);
    dlg->markFinished(ok && autogenRunning);
    finishAutogen();
}

QImage MainWindow::autogenPreviewTile()
{
    QImage shot;
    if (PlanetGLWidget *gl = ensureAutogenGl())
    {
        gl->setPlanet(&autoplanet);
        gl->refreshTextures();
        gl->repaint();
        shot = gl->captureView().copy();
        gl->setPlanet(nullptr);
    }
    if (!shot.isNull())
        autoplanet.img_view = shot;
    autoplanet.FinalImage();
    if (box.extendedFormat)
        return autoplanet.img_final.copy();
    if (!shot.isNull())
        return shot;
    return autoplanet.img.copy();
}

PlanetGLWidget *MainWindow::ensureAutogenGl()
{
    QSize side(257, 257);
    if (preview && preview->glWidget())
        side = preview->glWidget()->size();
    side.setWidth(qMax(257, side.width()));
    side.setHeight(qMax(257, side.height()));
    if (!autogenGl)
    {
        autogenGl = new PlanetGLWidget;
        autogenGl->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint
                                 | Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput);
        autogenGl->setAttribute(Qt::WA_DontShowOnScreen, true);
        autogenGl->setAttribute(Qt::WA_ShowWithoutActivating, true);
        autogenGl->setAttribute(Qt::WA_QuitOnClose, false);
        autogenGl->setWindowOpacity(0.0);
        autogenGl->setFixedSize(side);
        autogenGl->move(-20000, -20000);
        autogenGl->show();
        QApplication::processEvents();
        autogenGl->move(-20000, -20000);
    }
    else if (autogenGl->size() != side)
    {
        autogenGl->setFixedSize(side);
        autogenGl->move(-20000, -20000);
        QApplication::processEvents();
    }
    return autogenGl;
}

void MainWindow::finishAutogen()
{
    autogenRunning = false;
    ui->btnAutogen->setEnabled(true);
    ui->btnCreate->setEnabled(true);
    ui->btnRecreate->setEnabled(true);
    if (autogenGl)
    {
        autogenGl->setPlanet(nullptr);
        autogenGl->hide();
        autogenGl->deleteLater();
        autogenGl = nullptr;
    }
    endLoadingWatch();
}

void MainWindow::CreateNewPlanet()
{
    if (autogenRunning)
        return;
    if (!confirmAbandonSession())
        return;
    Settings_Get();
    pendingSessionPath.clear();
    startGeneration(true, 0, GenOp::Create);
}

void MainWindow::RecreatePlanet()
{
    if (autogenRunning)
        return;
    Settings_Get();
    if (isEmtyPlanet)
    {
        if (!confirmAbandonSession())
            return;
        pendingSessionPath.clear();
        startGeneration(true, 0, GenOp::Create);
        return;
    }
    planetBeforeRecreate = planet;
    pendingRecreateUndo = true;
    startGeneration(false, 0, GenOp::Recreate);
}

void MainWindow::applyPlanetToView()
{
    if (!preview)
        return;
    preview->setHasPlanet(true);
    preview->glWidget()->setPlanet(&planet);
    preview->glWidget()->refreshTextures();
    if (restoreViewOnApply)
    {
        preview->glWidget()->applyViewJson(pendingView);
        restoreViewOnApply = false;
        pendingView = QJsonObject();
    }
    preview->glWidget()->repaint();
    const QImage shot = preview->glWidget()->captureView();
    if (!shot.isNull())
        planet.img_view = shot;
    planet.FinalImage();
    updateViewButtonsEnabled();
    ShowPlanet();
    preview->setPlanetName(planet.name);
    updateFactsCard();
}

void MainWindow::ShowPlanet()
{
    if (preview)
        preview->showGlobe();
    syncViewButton(ui->btnViewPlanet);
}

void MainWindow::ShowDescription()
{
    if (isEmtyPlanet || !preview)
        return;
    preview->showCard(planet.img_dsc);
    syncViewButton(ui->btnViewDescription);
}

void MainWindow::ShowSystem()
{
    if (isEmtyPlanet || !preview)
        return;
    preview->showCard(planet.img_sys);
    syncViewButton(ui->btnViewSystem);
}

void MainWindow::ShowMap()
{
    if (isEmtyPlanet || !preview)
        return;
    preview->showCard(planet.img_gal);
    syncViewButton(ui->btnViewMap);
}

void MainWindow::syncViewButton(QPushButton *active)
{
    if (active)
        active->setChecked(true);
}

void MainWindow::updateViewButtonsEnabled()
{
    const bool on = !isEmtyPlanet;
    ui->btnViewDescription->setEnabled(on);
    ui->btnViewSystem->setEnabled(on);
    ui->btnViewMap->setEnabled(on);
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
    if (!isEmtyPlanet && preview && preview->glWidget())
    {
        const QImage shot = preview->glWidget()->captureView();
        if (!shot.isNull())
            planet.img_view = shot;
    }
    const QImage out = planet.img_view.isNull() ? planet.img : planet.img_view;
    if (!out.save(filename))
        QMessageBox::critical(nullptr, tr("Error"), tr("0001 unable to save file"));
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
    if (!isEmtyPlanet && preview && preview->glWidget())
    {
        const QImage shot = preview->glWidget()->captureView();
        if (!shot.isNull())
        {
            planet.img_view = shot;
            planet.FinalImage();
        }
    }
    if (!planet.img_final.save(filename))
        QMessageBox::critical(nullptr, tr("Error"), tr("0001 unable to save file"));
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
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0002 unable to load file"));
        return;
    }
    const QString a = file.readAll();
    file.close();
    const QJsonObject jobject = QJsonDocument::fromJson(a.toUtf8()).object();
    PlanetSettings loaded;
    if (!loaded.JSON_deserialize(jobject["settings"].toObject()))
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0002 unable to load file"));
        return;
    }
    if (!confirmAbandonSession())
        return;
    s = loaded;
    Settings_Set();
    lastCommittedSettings = s;
    restoreViewOnApply = jobject.contains(QStringLiteral("view")) && jobject.value(QStringLiteral("view")).isObject();
    pendingView = restoreViewOnApply ? jobject.value(QStringLiteral("view")).toObject() : QJsonObject();
    pendingSessionPath = filename;
    startGeneration(true, jobject["seed"].toInt(), GenOp::Load);
}

bool MainWindow::M_Save_Planet()
{
    if (!sessionPath.isEmpty())
        return writePlanetFile(sessionPath);
    return M_Save_PlanetAs();
}

bool MainWindow::M_Save_PlanetAs()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save planet as"),
                                                    startPath(AppKeys::dirPlanet, isEmtyPlanet ? QString() : planet.name),
                                                    tr("Planet (*.planet);;All files (*.*)"));
    if (filename.isEmpty())
        return false;
    rememberPath(AppKeys::dirPlanet, filename);
    return writePlanetFile(filename);
}

bool MainWindow::writePlanetFile(const QString &filename)
{
    if (isEmtyPlanet)
        return false;
    QFile file(filename);
    const bool ok = file.open(QFile::WriteOnly | QFile::Text);
    if (!ok)
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("0001 unable to save file"));
        return false;
    }
    QJsonObject jobject;
    jobject["seed"] = planet.seed;
    jobject["settings"] = planet.s.JSON_serialize();
    if (preview && preview->glWidget())
        jobject["view"] = preview->glWidget()->viewToJson();
    QTextStream stream(&file);
    stream << QJsonDocument(jobject).toJson();
    file.close();
    sessionPath = filename;
    sessionActive = true;
    if (undoStack)
        undoStack->setClean();
    lastCommittedSettings = planet.s;
    evaluateAchievements(false, true);
    updateWindowTitle();
    return true;
}

void MainWindow::Gen(bool isCreateNew, Planet *p, int seed)
{
    p->s = s;
    if (isCreateNew)
        p->SetSeed(seed);
    else
        p->UseSeed(p->seed);
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

void MainWindow::startGeneration(bool createNew, int seed, GenOp op)
{
    if (autogenRunning)
        return;
    if (genRunning)
    {
        genQueued = true;
        genQueuedCreateNew = createNew;
        genQueuedSeed = seed;
        genQueuedOp = op;
        return;
    }

    genRunning = true;
    genActiveOp = op;
    beginLoadingWatch();

    auto *work = new Planet;
    genWork = work;
    work->s = s;
    if (createNew)
        work->SetSeed(seed);
    else
        work->UseSeed(planet.seed);

    QThread *thread = QThread::create([work]() { work->Generate(); });
    genThread = thread;
    connect(thread, &QThread::finished, this, [this, work, thread]() {
        if (genThread == thread)
            genThread = nullptr;
        thread->deleteLater();

        const bool queued = genQueued;
        const bool queuedCreate = genQueuedCreateNew;
        const int queuedSeed = genQueuedSeed;
        const GenOp queuedOp = genQueuedOp;
        genQueued = false;

        if (queued && queuedCreate)
        {
            delete work;
            if (genWork == work)
                genWork = nullptr;
            genRunning = false;
            startGeneration(true, queuedSeed, queuedOp);
            return;
        }

        planet = *work;
        delete work;
        if (genWork == work)
            genWork = nullptr;
        isEmtyPlanet = false;
        applyPlanetToView();
        const GenOp finishedOp = genActiveOp;
        genActiveOp = GenOp::None;
        genRunning = false;
        endLoadingWatch();
        if (finishedOp == GenOp::Create)
            beginSession(QString(), true);
        else if (finishedOp == GenOp::Load)
            beginSession(pendingSessionPath, false);
        else if (finishedOp == GenOp::Recreate && pendingRecreateUndo && undoStack && sessionActive && !undoApplying)
        {
            auto apply = [this](const Planet &next) { applyPlanetFromUndo(next); };
            undoStack->push(new PlanetUndoCommand(planetBeforeRecreate, planet, apply));
            pendingRecreateUndo = false;
            updateWindowTitle();
        }
        pendingRecreateUndo = (finishedOp == GenOp::Recreate) ? pendingRecreateUndo : false;
        if (finishedOp != GenOp::Load)
            evaluateAchievements(finishedOp == GenOp::Create);
        if (queued)
            startGeneration(false, 0, queuedOp);
    });
    thread->start();
}

void MainWindow::M_Load_Base_Settings()
{
    Settings_Get();
    const bool ok = s.Load(":/txt_files/res/txt_files/settingsbase.json");
    if (ok)
        Settings_Set();
    if (ok)
        commitSettingsUndo(false);
    if (!ok)
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
    const bool ok = s.Save(filename);
    if (!ok)
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
    const bool ok = s.JSON_deserialize(QJsonDocument::fromJson(file.readAll()).object());
    if (ok)
    {
        Settings_Set();
        commitSettingsUndo(false);
        update();
    }
    else
        QMessageBox::critical(nullptr, tr("Error"), tr("0002 unable to load file"));
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
    if (autogenRunning || isEmtyPlanet || liveSuspended || !livePreview || !liveTimer)
        return;
    liveTimer->start();
}

void MainWindow::runLivePreview()
{
    if (autogenRunning || isEmtyPlanet || liveSuspended || !livePreview)
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
        planet.PrepareRings();
        preview->glWidget()->refreshAppearance();
        return;
    }
    startGeneration(false);
}
