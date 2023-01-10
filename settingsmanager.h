/*
#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QString>
#include <QVector>
#include <QColor>
#include <mainwindow.h>
class SettingsManager
{
public:
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
    int plants_number;
    int human_number;
    int shine;
    QVector<double> point_of_shine;
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
public:
    SettingsManager();
    void Get(MainWindow* window);
    void Set();
    void Save(QString path);
    void Load(QString path);
};

#endif // SETTINGSMANAGER_H
*/
