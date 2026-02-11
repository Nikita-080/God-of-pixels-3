#include "planetsettings.h"
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <global.h>
#include <QMessageBox>
PlanetSettings::PlanetSettings()
{
    QTime midnight(0,0,0);
    rnd.seed(midnight.secsTo(QTime::currentTime()));
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
    jobject["version"]=VERSION;
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
    jobject["shine"] = shine;
    jobject["point_of_shine"]=VecToJson(point_of_shine);
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
    jobject["point_of_polar"]=VecToJson(point_of_polar);

    return jobject;
}
bool PlanetSettings::JSON_deserialize(QJsonObject jobject)
{
    int version;
    try {
         version=jobject["version"].toInt();
    }  catch (...) {
        QMessageBox::critical(nullptr,QT_TR_NOOP("Error"),VERSION_NOT_FOUND);
        return false;
    }
    switch (version)
    {
        case 1:
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
            shine = jobject["shine"].toInt();
            point_of_shine = JsonToVec(jobject["point_of_shine"].toArray());
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
            point_of_polar = JsonToVec(jobject["point_of_polar"].toArray());
            break;
        default:
            QMessageBox::critical(nullptr,QT_TR_NOOP("Error"),UNKNOW_VERSION);
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
bool PlanetSettings::Load(QString path){
    QFile file(path);
    if(file.open(QFile::ReadOnly|QFile::Text)){
        QString a;
        a=file.readAll();
        return JSON_deserialize(QJsonDocument::fromJson(a.toUtf8()).object());
    }
    return false;
}
void PlanetSettings::Random(QVector<bool> isRnd,PointChoicer* pc){
    if (isRnd[0]) terramode=RAND(0,1);
    if (isRnd[1]) randomness=RAND(1,10);
    if (isRnd[2]) iterations=RAND(50,1000);
    if (isRnd[3]) world_size=RAND(4,8);
    if (isRnd[4]) temperature=RAND(-90,140);
    if (isRnd[5])
    {
        structure[0]=40;
        structure[7]=425;
        for (int i=1;i<7;i++)
        {
            int num=RAND(structure[i-1],425);
            structure[i]=num;
            true_structure[i]=280-((num-40)*280*1.0/385);
        }
    }
    if (isRnd[6]) ice_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[7]) rock_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[8]) mountain_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[9]) plain_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[10]) beach_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[11]) shallow_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[12]) ocean_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[13]) noise=RAND(1,100);
    if (isRnd[14]) is_gradient=RAND(0,1);
    if (isRnd[15]) is_plant=RAND(0,1);
    if (isRnd[16]) shine=RAND(0,5);
    if (isRnd[17]) point_of_shine_true=pc->GetRand();
    if (isRnd[18]) name_algorithm=RAND(1,3);
    if (isRnd[19]) is_cloud=RAND(0,1);
    if (isRnd[20]) cloud_size=RAND(1,20);
    if (isRnd[21]) cloud_quality=RAND(1,10);
    if (isRnd[22]) cloud_transparent=RAND(0,10);
    if (isRnd[23]) correction=RAND(0,1);
    if (isRnd[24]) cloud_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[25]) is_atmo=RAND(0,1);
    if (isRnd[26]) atmo_transparent=RAND(0,10);
    if (isRnd[27]) atmo_size=RAND(1,10);
    if (isRnd[28]) atmo_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[29]) is_ring=RAND(0,1);
    if (isRnd[30]) R_internal_ring=RAND(1,5);
    if (isRnd[31]) R_external_ring=RAND(1,5);
    if (isRnd[32]) ring_color=QColor(RAND(0,255),RAND(0,255),RAND(0,255));
    if (isRnd[33]) point_of_polar_true=pc->GetRand();
}
