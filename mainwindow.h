#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <multislider.h>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <QVector>
#include <QRandomGenerator>
#include <planet.h>
#include <autogensettings.h>
#include <planetsettings.h>
#include <QTranslator>
class PlanetGLWidget;
class QStackedWidget;
class QTimer;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Planet planet;
    Planet autoplanet;
    bool isEmtyPlanet;
    QVector <bool> isRandom;
    MultiSlider* ms;
    QSlider* sliders[13];
    QLabel* labels[13];
    QSlider* sliderShineLat;
    QSlider* sliderShineLon;
    QSlider* sliderPolarLat;
    QSlider* sliderPolarLon;
    QLabel* labelShineLatValue;
    QLabel* labelShineLonValue;
    QLabel* labelPolarLatValue;
    QLabel* labelPolarLonValue;
    QLabel* labelShineLatTitle;
    QLabel* labelShineLonTitle;
    QLabel* labelPolarLatTitle;
    QLabel* labelPolarLonTitle;

    PlanetSettings s;

    AutoGenSettings box;

    QString language;

    int count;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void SliderShow();
    void AlgorithmChange();
    void ColorChoicer();
    void ColorToButton(QPushButton*,QColor);
    QColor ColorFromButton(QPushButton*);
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
    void Gen(bool isCreateNew,Planet *p,int seed=0);
    void SetStyle();
    QString ReadText(QString);
    void Report(QString s);
    void Img_Report();
    void BiomGrad();
    void scheduleLivePreview();
    void runLivePreview();
    void applyPlanetToView();
    void addLatLonControls(QWidget *parent,
                           QSlider *&latSlider, QSlider *&lonSlider,
                           QLabel *&latTitle, QLabel *&lonTitle,
                           QLabel *&latValue, QLabel *&lonValue);
    void retranslateCoordLabels();
public:
    Ui::MainWindow *ui;
private:
    QTranslator qtLanguageTranslator;
    PlanetGLWidget *glWidget;
    QStackedWidget *previewStack;
    QLabel *cardView;
    QPushButton *btnResetCamera;
    QTimer *liveTimer;
    bool liveSuspended;
    bool appearanceOnlyLive;
protected:
    void changeEvent(QEvent * event) override;
};
#endif
