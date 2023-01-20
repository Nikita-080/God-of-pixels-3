#ifndef PLANETSETTINGS_H
#define PLANETSETTINGS_H
#include <QColor>
#include <QVector>
#include <QRandomGenerator>
#include <pointchoicer.h>

class PlanetSettings
{
public:
    int terramode; //relocate
    int iterations;

    int world_size;
    int randomness;
    int temperature;
    QVector <int> structure;
    QColor ice_color;
    QColor rock_color;
    QColor mountain_color;
    QColor plain_color;
    QColor beach_color;
    QColor shallow_color;
    QColor ocean_color;
    int noise;
    bool is_gradient;
    bool is_plant;
    int shine;
    QVector<int> point_of_shine;
    QVector<double> point_of_shine_true;
    int name_algorithm;
    bool is_cloud;
    int cloud_size;
    int cloud_quality;
    int cloud_transparent;
    bool correction;
    QColor cloud_color;
    bool is_atmo;
    int atmo_transparent;
    int atmo_size;
    QColor atmo_color;
    bool is_ring;
    int R_internal_ring;
    int R_external_ring;
    QColor ring_color;
    QVector<int> point_of_polar;
    QVector<double> point_of_polar_true;
    QVector <double> true_structure;
public:
    PlanetSettings();
    PlanetSettings(QRandomGenerator);
    void Load(QString);
    void Save(QString);
    void Random(QVector<bool>,PointChoicer* pc);
private:
    QRandomGenerator rnd;
    int RAND(int x, int y);
};

#endif // PLANETSETTINGS_H
