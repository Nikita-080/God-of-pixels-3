#include "planetsettings.h"
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QTime>
#include <QMessageBox>
#include <QCoreApplication>
#include <global.h>
PlanetSettings::PlanetSettings()
{
    QTime midnight(0,0,0);
    rnd.seed(midnight.secsTo(QTime::currentTime()));
    shine_lat = 25;
    shine_lon = 90;
    is_fill_light = true;
    polar_lat = 90;
    polar_lon = 0;
    ring_material = 0;
    ring_intensity = 2;
    is_civ = false;
    civ_color = QColor(QStringLiteral("#ffcc66"));
}

int PlanetSettings::RAND(int a, int b)
{
    return rnd.generate()%((b+1)-a)+a;
}
QJsonArray PlanetSettings::VecToJson(QVector<int> vector)
{
    QJsonArray array;
    foreach (int i,vector) array.append(i);
    return array;
}
QVector<int> PlanetSettings::JsonToVec(QJsonArray jarray)
{
    QVector<int> vector;
    foreach (QJsonValue i,jarray) vector.append(i.toInt());
    return vector;
}
QJsonObject PlanetSettings::JSON_serialize()
{
    QJsonObject jobject;
    jobject["version"]=PARSER_VERSION;
    jobject["terramode"] = terramode;
    jobject["randomness"] = randomness;
    jobject["iterations"] = iterations;
    jobject["world_size"] = world_size;
    jobject["temperature"] = temperature;
    jobject["structure"]=VecToJson(structure);
    jobject["ice_color"] = ice_color.name();
    jobject["rock_color"] = rock_color.name();
    jobject["mountain_color"] = mountain_color.name();
    jobject["plain_color"] = plain_color.name();
    jobject["beach_color"] = beach_color.name();
    jobject["shallow_color"] = shallow_color.name();
    jobject["ocean_color"] = ocean_color.name();
    jobject["noise"] = noise;
    jobject["is_gradient"] = is_gradient;
    jobject["is_plant"] = is_plant;
    jobject["is_civ"] = is_civ;
    jobject["civ_color"] = civ_color.name();
    jobject["shine"] = shine;
    jobject["shine_lat"] = shine_lat;
    jobject["shine_lon"] = shine_lon;
    jobject["is_fill_light"] = is_fill_light;
    jobject["name_algorithm"] = name_algorithm;
    jobject["is_cloud"] = is_cloud;
    jobject["cloud_size"] = cloud_size;
    jobject["cloud_quality"] = cloud_quality;
    jobject["cloud_transparent"] = cloud_transparent;
    jobject["correction"] = correction;
    jobject["cloud_color"] = cloud_color.name();
    jobject["is_atmo"] = is_atmo;
    jobject["atmo_transparent"] = atmo_transparent;
    jobject["atmo_size"] = atmo_size;
    jobject["atmo_color"] = atmo_color.name();
    jobject["is_ring"] = is_ring;
    jobject["R_internal_ring"] = R_internal_ring;
    jobject["R_external_ring"] = R_external_ring;
    jobject["ring_color"] = ring_color.name();
    jobject["ring_material"] = ring_material;
    jobject["ring_intensity"] = ring_intensity;
    jobject["polar_lat"] = polar_lat;
    jobject["polar_lon"] = polar_lon;

    return jobject;
}
bool PlanetSettings::JSON_deserialize(QJsonObject jobject)
{
    int version;
    try {
         version=jobject["version"].toInt();
    }  catch (...) {
        QMessageBox::critical(nullptr,QCoreApplication::translate("PlanetSettings", "Error"),QCoreApplication::translate("PlanetSettings", "0004 unable to get version from file"));
        return false;
    }
    switch (version)
    {
        case 1:
            QMessageBox::critical(nullptr,
                QCoreApplication::translate("PlanetSettings", "Error"),
                QCoreApplication::translate("PlanetSettings",
                    "0006 this file was created by an older version and cannot be loaded"));
            return false;
        case 2:
            terramode = jobject["terramode"].toInt();
            randomness = jobject["randomness"].toInt();
            iterations = jobject["iterations"].toInt();
            world_size = jobject["world_size"].toInt();
            temperature = jobject["temperature"].toInt();
            structure =JsonToVec(jobject["structure"].toArray());
            ice_color = jobject["ice_color"].toString();
            rock_color = jobject["rock_color"].toString();
            mountain_color = jobject["mountain_color"].toString();
            plain_color = jobject["plain_color"].toString();
            beach_color = jobject["beach_color"].toString();
            shallow_color = jobject["shallow_color"].toString();
            ocean_color = jobject["ocean_color"].toString();
            noise = jobject["noise"].toInt();
            is_gradient = jobject["is_gradient"].toBool();
            is_plant = jobject["is_plant"].toBool();
            is_civ = jobject.contains("is_civ") ? jobject["is_civ"].toBool() : false;
            civ_color = jobject.contains("civ_color") ? QColor(jobject["civ_color"].toString()) : QColor(QStringLiteral("#ffcc66"));
            shine = jobject["shine"].toInt();
            shine_lat = jobject["shine_lat"].toInt();
            shine_lon = jobject["shine_lon"].toInt();
            is_fill_light = jobject.contains("is_fill_light") ? jobject["is_fill_light"].toBool() : true;
            name_algorithm = jobject["name_algorithm"].toInt();
            is_cloud = jobject["is_cloud"].toBool();
            cloud_size = jobject["cloud_size"].toInt();
            cloud_quality = jobject["cloud_quality"].toInt();
            cloud_transparent = jobject["cloud_transparent"].toInt();
            correction = jobject["correction"].toBool();
            cloud_color = jobject["cloud_color"].toString();
            is_atmo = jobject["is_atmo"].toBool();
            atmo_transparent = jobject["atmo_transparent"].toInt();
            atmo_size= jobject["atmo_size"].toInt();
            atmo_color = jobject["atmo_color"].toString();
            is_ring = jobject["is_ring"].toBool();
            R_internal_ring = jobject["R_internal_ring"].toInt();
            R_external_ring = jobject["R_external_ring"].toInt();
            ring_color = jobject["ring_color"].toString();
            ring_material = jobject.contains("ring_material") ? jobject["ring_material"].toInt() : 0;
            ring_intensity = jobject.contains("ring_intensity") ? jobject["ring_intensity"].toInt() : 2;
            polar_lat = jobject["polar_lat"].toInt();
            polar_lon = jobject["polar_lon"].toInt();
            rebuildDerived();
            break;
        default:
            QMessageBox::critical(nullptr,QCoreApplication::translate("PlanetSettings", "Error"),QCoreApplication::translate("PlanetSettings", "0005 unknown file version"));
            return false;
    }
    return true;
}
bool PlanetSettings::Save(QString path)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly|QFile::Text)){
        QTextStream stream(&file);
        stream<<QJsonDocument(JSON_serialize()).toJson();
        file.close();
        return true;
    }
    return false;
}
void PlanetSettings::rebuildDerived()
{
    true_structure.clear();
    for (int i = 0; i < structure.size(); ++i)
        true_structure.append(280.0 - ((structure[i] - 40) * 280.0 / 385.0));
}
bool PlanetSettings::Load(QString path){
    QFile file(path);
    if(file.open(QFile::ReadOnly|QFile::Text)){
        QString a;
        a=file.readAll();
        return JSON_deserialize(QJsonDocument::fromJson(a.toUtf8()).object());
    }
    return false;
}
void PlanetSettings::Random(const QVector<bool> &isRnd)
{
    rnd.seed(QRandomGenerator::global()->generate());
    auto on = [&](int i) { return i >= 0 && i < isRnd.size() && isRnd[i]; };

    if (on(0)) terramode=RAND(0,1);
    if (on(1)) randomness=RAND(1,10);
    if (on(2)) iterations=RAND(50,1000);
    if (on(3)) world_size=RAND(1,20);
    if (on(4)) temperature=RAND(-90,140);
    if (on(5))
    {
        structure[0]=40;
        structure[7]=425;
        for (int i=1;i<7;i++)
        {
            int num=RAND(structure[i-1],425);
            structure[i]=num;
        }
        rebuildDerived();
    }
    if (on(6)) ice_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(7)) rock_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(8)) mountain_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(9)) plain_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(10)) beach_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(11)) shallow_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(12)) ocean_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(13)) noise=RAND(1,100);
    if (on(14)) is_gradient=RAND(0,1);
    if (on(15)) is_plant=RAND(0,1);
    if (on(16)) shine=RAND(0,5);
    if (on(17)) { shine_lat=RAND(-90,90); shine_lon=RAND(-180,180); }
    if (on(18)) name_algorithm=RAND(1,3);
    if (on(19)) is_cloud=RAND(0,1);
    if (on(20)) cloud_size=RAND(1,10);
    if (on(21)) cloud_quality=RAND(1,6);
    if (on(22)) cloud_transparent=RAND(0,10);
    if (on(23)) correction=RAND(0,1);
    if (on(24)) cloud_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(25)) is_atmo=RAND(0,1);
    if (on(26)) atmo_transparent=RAND(0,10);
    if (on(27)) atmo_size=RAND(1,10);
    if (on(28)) atmo_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(29)) is_ring=RAND(0,1);
    if (on(30)) R_internal_ring=RAND(1,5);
    if (on(31)) R_external_ring=RAND(1,5);
    if (on(32)) ring_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (on(33)) { polar_lat=RAND(-90,90); polar_lon=RAND(-180,180); }
    if (on(34)) ring_material=RAND(0,1);
    if (on(35)) ring_intensity=RAND(0,10);
    if (on(36)) is_civ=RAND(0,1);
    if (on(37)) civ_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
}
