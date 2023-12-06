#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <multislider.h>
#include <pointchoicer.h>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <QVector>
#include <QRandomGenerator>
#include <planet.h>
#include <QProgressBar>
#include <autogodsettings.h>
#include <planetsettings.h>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    QRandomGenerator rnd;
    Planet planet;
    Planet autoplanet;
    bool isEmtyPlanet;
    QVector <bool> isRandom;
    MultiSlider* ms;
    PointChoicer* pc;
    PointChoicer* pc2;
    QSlider* sliders[13];
    QLabel* labels[13];

    PlanetSettings s;

    bool isdatarecieved;
    AutoGodSettings box;
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
    void CreatePlanet();
    void AutoGod();
    void ShowPlanet();
    void ShowDescription();
    void ShowSystem();
    void ShowMap();
    void God(bool isCreateNew,QProgressBar *pb,Planet *p);
    void SetStyle();
    QString ReadText(QString);
    void Report(QString s); //служебная функция
    void Img_Report(); //служебная функция
    void BiomGrad(); //служебная функция
public:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
