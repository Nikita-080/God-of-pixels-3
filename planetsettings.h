#ifndef PLANETSETTINGS_H
#define PLANETSETTINGS_H
#include <QColor>
#include <QVector>
#include <QRandomGenerator>
#include <QJsonArray>
#include <QJsonObject>

class PlanetSettings
{
public:
    QRandomGenerator rnd;
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
    bool is_civ;
    QColor civ_color;
    int shine;
    int shine_lat;
    int shine_lon;
    bool is_fill_light;
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
    void Random(QVector<bool>);
private:
    int RAND(int x, int y);
    QJsonArray VecToJson(QVector<int>);
    QVector<int> JsonToVec(QJsonArray);
};

#endif // PLANETSETTINGS_H
