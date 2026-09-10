#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <planet.h>
#include <autogensettings.h>
#include <planetsettings.h>

class PreviewPanel;
class SettingsPanel;
class QTimer;
class QThread;

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
    void M_Switch_Language();
    void CreateNewPlanet();
    void RecreatePlanet();
    void AutoGen();
    void ShowPlanet();
    void ShowDescription();
    void ShowSystem();
    void ShowMap();
    void Gen(bool isCreateNew, Planet *p, int seed = 0);
    void scheduleLivePreview();
    void runLivePreview();
    void applyPlanetToView();
    void startGeneration(bool createNew, int seed = 0);
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
};

#endif
