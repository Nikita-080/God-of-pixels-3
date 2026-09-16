#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QElapsedTimer>
#include <planet.h>
#include <autogensettings.h>
#include <planetsettings.h>

class PreviewPanel;
class SettingsPanel;
class PlanetGLWidget;
class AchievementToastHost;
class QTimer;
class QThread;
class QTextEdit;

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
    void SetStyle();
    void Settings_Get();
    void Settings_Set();
    void M_Save_Settings();
    void M_Load_Settings();
    void M_Load_Base_Settings();
    void M_Save_Image();
    void M_Save_Planet();
    void M_Load_Planet();
    void M_Save_Full_Image();
    void M_About();
    void M_Achievements();
    void M_Switch_Language();
    void CreateNewPlanet();
    void RecreatePlanet();
    void AutoGen();
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
    void logOp(const QString &action, qint64 ms, bool ok, const QString &detail = QString());
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
    QTimer *liveTimer;
    QTimer *loadingDelayTimer;
    QThread *genThread;
    Planet *genWork;
    Planet planet;
    Planet autoplanet;
    PlanetSettings s;
    AutoGenSettings box;
    QString language;
    bool isEmtyPlanet;
    bool liveSuspended;
    bool appearanceOnlyLive;
    bool genRunning;
    bool genQueued;
    bool genQueuedCreateNew;
    int genQueuedSeed;
    bool autogenRunning;
    PlanetGLWidget *autogenGl;
    QTextEdit *opConsole;
    AchievementToastHost *achievementToasts;
    GenOp genActiveOp;
    GenOp genQueuedOp;
    QElapsedTimer genOpTimer;
};

#endif
