#ifndef PLANETSETTINGS_H
#define PLANETSETTINGS_H
#include <QColor>
#include <QVector>
#include <QRandomGenerator>
#include <QJsonArray>
#include <QJsonObject>
#include "starspectrum.h"

class PlanetSettings
{
public:
    QRandomGenerator rnd;
    int terramode; //relocate
    int iterations;

    int world_size;
    int randomness;
    int temperature;
    int seismicity;
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
    bool is_civ;
    QColor civ_color;
    bool has_star;
    int star_size;
    int shine_lat;
    int shine_lon;
    bool is_fill_light;
    bool is_starfield;
    QVector<int> star_spectrum;
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
    int ring_material;
    int ring_intensity;
    int polar_lat;
    int polar_lon;
    QVector <double> true_structure;
public:
    PlanetSettings();
    bool Load(QString);
    bool Save(QString);
    QJsonObject JSON_serialize();
    bool JSON_deserialize(QJsonObject);
    void rebuildDerived();
    void Random(const QVector<bool> &isRnd);
    int effectiveTemperature() const;
    double visibleLight() const;
    double parLight() const;
    double hazardLight() const;
private:
    int RAND(int x, int y);
    QJsonArray VecToJson(QVector<int>);
    QVector<int> JsonToVec(QJsonArray);
};

#endif // PLANETSETTINGS_H
