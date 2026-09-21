#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QJsonObject>
#include <planet.h>
#include <autogensettings.h>
#include <planetsettings.h>

class PreviewPanel;
class SettingsPanel;
class PlanetGLWidget;
class AchievementToastHost;
class windowsettings;
class QTimer;
class QThread;
class QAction;
class QPlainTextEdit;
class QLabel;
class QSplitter;
class QUndoStack;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    enum class GenOp
    {
        None,
        Create,
        Recreate,
        Load
    };

    void setupMainLayout();
    void setupShortcuts();
    void retranslateExtras();
    void SetStyle();
    void Settings_Get();
    void Settings_Set();
    void M_Save_Settings();
    void M_Load_Settings();
    void M_Load_Base_Settings();
    void M_Save_Image();
    bool M_Save_Planet();
    bool M_Save_PlanetAs();
    bool writePlanetFile(const QString &filename);
    void M_Load_Planet();
    void M_Save_Full_Image();
    void M_About();
    void M_Achievements();
    void M_ProgramSettings();
    void applyLanguage(const QString &lang);
    void updateFactsCard();
    void updateWindowTitle();
    void updateCameraStatus();
    void updateUndoActions();
    bool sessionDirty() const;
    bool confirmAbandonSession();
    void beginSession(const QString &path, bool markDirty);
    void applySettingsFromUndo(const PlanetSettings &settings, bool appearanceOnly);
    void applyPlanetFromUndo(const Planet &p);
    void commitSettingsUndo(bool appearanceOnly);
    bool inputTakesDigits() const;
    void CreateNewPlanet();
    void RecreatePlanet();
    void AutoGen();
    void runAutogenWithDialog(windowsettings *dlg);
    QImage autogenPreviewTile();
    PlanetGLWidget *ensureAutogenGl();
    void finishAutogen();
    void ShowPlanet();
    void ShowDescription();
    void ShowSystem();
    void ShowMap();
    void Gen(bool isCreateNew, Planet *p, int seed = 0);
    void scheduleLivePreview();
    void runLivePreview();
    void applyPlanetToView();
    void evaluateAchievements(bool countCreate, bool planetSaved = false, bool autogenAllRandom = false);
    void startGeneration(bool createNew, int seed = 0, GenOp op = GenOp::None);
    void beginLoadingWatch();
    void endLoadingWatch();
    void showLoadingOverlay();
    void Img_Report();
    void BiomGrad();
    void Report(QString s);
    QString ReadText(QString path);

    Ui::MainWindow *ui;
    QTranslator qtLanguageTranslator;
    PreviewPanel *preview;
    SettingsPanel *settingsPanel;
    QSplitter *mainSplitter;
    QLabel *cameraStatus;
    QTimer *liveTimer;
    QTimer *loadingDelayTimer;
    QThread *genThread;
    Planet *genWork;
    Planet planet;
    Planet autoplanet;
    Planet planetBeforeRecreate;
    PlanetSettings s;
    PlanetSettings lastCommittedSettings;
    AutoGenSettings box;
    QString language;
    QString sessionPath;
    bool livePreview;
    bool isEmtyPlanet;
    bool liveSuspended;
    bool appearanceOnlyLive;
    bool genRunning;
    bool genQueued;
    bool genQueuedCreateNew;
    int genQueuedSeed;
    bool restoreViewOnApply;
    QJsonObject pendingView;
    bool autogenRunning;
    PlanetGLWidget *autogenGl;
    QPlainTextEdit *factsView;
    QAction *actionProgramSettings;
    QAction *actionSavePlanetAs;
    QAction *actionUndo;
    QAction *actionRedo;
    AchievementToastHost *achievementToasts;
    QUndoStack *undoStack;
    bool sessionActive;
    bool undoApplying;
    bool pendingRecreateUndo;
    QString pendingSessionPath;
    GenOp genActiveOp;
    GenOp genQueuedOp;
};

#endif
