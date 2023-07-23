#ifndef PLANET_H
#define PLANET_H
#include <QVector>
#include <QImage>
#include <QRandomGenerator>
#include <planetsettings.h>
#include <facts.h>
class Planet
{
public:
    QString currentpath;
    QRandomGenerator rnd;
    Facts facts;
    QVector <int> starclass;
    QVector <QVector <double>> matrix;
    double world_heighth;
    int world_size;
    double world_deep;
    double R_planet;
    double R_atmo;
    double R_final;
    QImage img;
    QImage img_dsc;
    QImage img_sys;
    QImage img_gal;
    QImage img_final;
    QImage img_nonscale;
    QString name;
    double A;//коэффициенты уравнения плоскости колец
    double B;
    double C;
    double x_shine;//координаты точки максимальной освещенности
    double y_shine;
    double z_shine;
    double x_polar;//координаты точки северного полюса
    double y_polar;
    double z_polar;
    QVector <double> level_up;//разбиение биомов по уровням
    QVector <double> level_down;
    QVector <QColor> level_color;
    double water_level;
    QVector <QVector <double>> c_map; //cloud - облака
    QVector <QVector <double>> t_map; //temperature - температура
    QVector <QVector <double>> r_map; //rain - осадки
    QVector <QVector <QVector<int>>> u_map; //преобразование в сферу
    QColor color_black;

    int plant_pixel_count;
    int water_pixel_count;
    int ice_pixel_count;

    PlanetSettings s;
public:
    void WMapCreating();
    void TMapCreating();
    void RMapCreating();
    void UMapCreating();
    void Calculator();
    void CreateMatrix();
    void CreateMatrixNew();
    void FixMatrix();
    void LevelCreating();
    void ImageCreating();
    void Ring();
    void Noise();
    void Polar();
    void CloudMapCreating();
    void Cloud();
    void Atmosphere(QString mode);
    void UV();
    void Name();
    void Plant();
    void Shadow();
    void SystemMap();
    void GenerateDescription();
    void CalculateDescription();
    void DrawDescription();
    void GalaxyMap();
    void FinalImage();
    void ImagesScale();
    void ImageReport(QString name,QVector<QVector<double>> data,QColor lowcolor,QColor highcolor); //служебная функция
private:
    double Shadow_step(int x,int y, int R);
    QChar char2char(QChar,QVector<QVector<int>>);
    QString Name_gop2();
    QString Name_readable();
    QString Name_random();
    void Level(QString,int,int,QString,QPainter&);
    QColor DispersionColor(QColor color, int disp);
    QColor TransparentColor(QColor color1, QColor color2, double koef);
    QColor LowerColor(QColor color, double koef);
    int RAND(int a, int b);
    double RAND(double a,double b);
    bool isBlack(QColor);
    QString Resources();
    bool Collis(int,int,QVector<QVector<int>>);
    double ArcPolarDistance(int x, int y);
    void SystemMap_0star();
    void SystemMap_1star();
    void SystemMap_2star();
    void DrawPlanets(QPainter* p, int x, int y, int r_o_min, int r_o_max, int r_p_min, int r_p_max, int r_o, int r_p);
public:
    Planet(QRandomGenerator,QString);
    Planet();
};

#endif // PLANET_H
