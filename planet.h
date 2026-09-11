#ifndef PLANET_H
#define PLANET_H
#include <QVector>
#include <QImage>
#include <QColor>
#include <QRandomGenerator>
#include <planetsettings.h>
#include <facts.h>
#include "spheremath.h"
class QPainter;

struct RingBand
{
    float inner;
    float outer;
    QColor color;
    bool empty;
};

struct RingRock
{
    float x;
    float y;
    float z;
    float radius;
    QColor color;
};

struct CityLight
{
    float x;
    float y;
    float z;
};

class Planet
{
public:
    QRandomGenerator rnd;
    int seed;
    Facts facts;
    QVector<int> starclass;
    QVector<QVector<double>> matrix;
    double world_heighth;
    int map_w;
    int map_h;
    int world_size;
    double world_deep;
    double R_planet;
    double R_atmo;
    double R_final;
    QImage img;
    QImage img_clouds;
    QImage img_dsc;
    QImage img_sys;
    QImage img_gal;
    QImage img_final;
    QImage img_view;
    QString name;
    double x_shine;
    double y_shine;
    double z_shine;
    double x_polar;
    double y_polar;
    double z_polar;
    QVector<double> level_up;
    QVector<double> level_down;
    QVector<QColor> level_color;
    double water_level;
    QVector<QVector<double>> c_map;
    QVector<QVector<double>> t_map;
    QVector<QVector<double>> r_map;
    QColor color_black;
    int plant_pixel_count;
    int water_pixel_count;
    int ice_pixel_count;
    QVector<QColor> ring_colors;
    QVector<QColor> ring_colors_dark;
    QVector<RingBand> ring_bands;
    QVector<RingRock> ring_rocks;
    double ring_inner;
    double ring_outer;
    QVector<CityLight> cities;
    PlanetSettings s;

    void TMapCreating();
    void RMapCreating();
    void Calculator();
    void CreateMatrixNew();
    void FixMatrix();
    void LevelCreating();
    void ImageCreating();
    void PrepareRings();
    void Noise();
    void Polar();
    void CloudMapCreating();
    void CloudImageCreating();
    void Name();
    void Plant();
    void Civilization();
    void SystemMap();
    void GenerateDescription();
    void CalculateDescription();
    void DrawDescription();
    void GalaxyMap();
    void FinalImage();
    void ImagesScale();
    void Generate();
    QImage ImageReport(QVector<QVector<double>> data, QColor lowcolor, QColor highcolor);
    void SetSeed(int value = 0);
    SphereVec3 TexelXYZ(int x, int y) const;
    double ArcPolarDistance(int x, int y) const;
    int viewResolution() const;
    QColor TransparentColor(QColor color1, QColor color2, double koef);
    Planet();

private:
    QChar char2char(QChar, QVector<QVector<int>>);
    QString Name_gop2();
    QString Name_readable();
    QString Name_random();
    void Level(QString, int, int, QString, QPainter &);
    QColor DispersionColor(QColor color, int disp);
    QColor LowerColor(QColor color, double koef);
    int RAND(int a, int b);
    QString Resources();
    bool Collis(int, int, QVector<QVector<int>>);
    void SystemMap_0star();
    void SystemMap_1star();
    void SystemMap_2star();
    void DrawPlanets(QPainter *p, int x, int y, int r_o_min, int r_o_max, int r_p_min, int r_p_max, int r_o, int r_p);
};

#endif
