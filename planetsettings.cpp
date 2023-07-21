#include "planetsettings.h"
#include <QFile>
#include <QTextStream>
PlanetSettings::PlanetSettings()
{

}
PlanetSettings::PlanetSettings(QRandomGenerator gen)
{
    rnd=gen;
}
int PlanetSettings::RAND(int a, int b)
{
    return rnd.generate()%((b+1)-a)+a;
}
bool PlanetSettings::Save(QString path){

    QFile file(path);
    if (file.open(QFile::WriteOnly|QFile::Text)){
        QTextStream stream(&file);
        stream<<QString::number(terramode)<<"\n";
        stream<<QString::number(randomness)<<"\n";
        stream<<QString::number(iterations)<<"\n";
        stream<<QString::number(world_size)<<"\n";
        stream<<QString::number(temperature)<<"\n";
        for (int i=0;i<8;i++){
            stream<<QString::number(structure[i])<<"\n";
        }
        stream<<ice_color.name()<<"\n";
        stream<<rock_color.name()<<"\n";
        stream<<mountain_color.name()<<"\n";
        stream<<plain_color.name()<<"\n";
        stream<<beach_color.name()<<"\n";
        stream<<shallow_color.name()<<"\n";
        stream<<ocean_color.name()<<"\n";
        stream<<QString::number(noise)<<"\n";
        stream<<QString::number(is_gradient)<<"\n";
        stream<<QString::number(is_plant)<<"\n";
        stream<<QString::number(shine)<<"\n";
        stream<<QString::number(point_of_shine[0])<<"\n";
        stream<<QString::number(point_of_shine[1])<<"\n";
        stream<<QString::number(name_algorithm)<<"\n";
        stream<<QString::number(is_cloud)<<"\n";
        stream<<QString::number(cloud_size)<<"\n";
        stream<<QString::number(cloud_quality)<<"\n";
        stream<<QString::number(cloud_transparent)<<"\n";
        stream<<QString::number(correction)<<"\n";
        stream<<cloud_color.name()<<"\n";
        stream<<QString::number(is_atmo)<<"\n";
        stream<<QString::number(atmo_transparent)<<"\n";
        stream<<QString::number(atmo_size)<<"\n";
        stream<<atmo_color.name()<<"\n";
        stream<<QString::number(is_ring)<<"\n";
        stream<<QString::number(R_internal_ring)<<"\n";
        stream<<QString::number(R_external_ring)<<"\n";
        stream<<ring_color.name()<<"\n";
        stream<<QString::number(point_of_polar[0])<<"\n";
        stream<<QString::number(point_of_polar[1])<<"\n";
        file.close();
        return true;
    }
    return false;
}
bool PlanetSettings::Load(QString path){
    QFile file(path);
    if(file.open(QFile::ReadOnly|QFile::Text)){
        QTextStream stream(&file);
        QString a;
        stream>>a; terramode=a.toInt();
        stream>>a; randomness=a.toInt();
        stream>>a; iterations=a.toInt();
        stream>>a; world_size=a.toInt();
        stream>>a; temperature=a.toInt();
        for (int i=0;i<8;i++){
            stream>>a;
            structure[i]=a.toInt();
        }
        stream>>a; ice_color.setNamedColor(a);
        stream>>a; rock_color.setNamedColor(a);
        stream>>a; mountain_color.setNamedColor(a);
        stream>>a; plain_color.setNamedColor(a);
        stream>>a; beach_color.setNamedColor(a);
        stream>>a; shallow_color.setNamedColor(a);
        stream>>a; ocean_color.setNamedColor(a);
        stream>>a; noise=a.toInt();
        stream>>a; is_gradient=a.toInt();
        stream>>a; is_plant=a.toInt();
        stream>>a; shine=a.toInt();
        stream>>a; point_of_shine[0]=a.toDouble();
        stream>>a; point_of_shine[1]=a.toDouble();
        stream>>a; name_algorithm=a.toInt();
        stream>>a; is_cloud=a.toInt();
        stream>>a; cloud_size=a.toInt();
        stream>>a; cloud_quality=a.toInt();
        stream>>a; cloud_transparent=a.toInt();
        stream>>a; correction=a.toInt();
        stream>>a; cloud_color.setNamedColor(a);
        stream>>a; is_atmo=a.toInt();
        stream>>a; atmo_transparent=a.toInt();
        stream>>a; atmo_size=a.toInt();
        stream>>a; atmo_color.setNamedColor(a);
        stream>>a; is_ring=a.toInt();
        stream>>a; R_internal_ring=a.toInt();
        stream>>a; R_external_ring=a.toInt();
        stream>>a; ring_color.setNamedColor(a);
        stream>>a; point_of_polar[0]=a.toDouble();
        stream>>a; point_of_polar[1]=a.toDouble();
        return true;
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
