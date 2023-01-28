#ifndef AUTOGODSETTINGS_H
#define AUTOGODSETTINGS_H
#include <QVector>
#include <QString>

class AutoGodSettings
{
public:
    bool save_type;
    bool picturetype;
    QString path;
    int height;
    int width;
    int number;
    QVector <bool> isRndList;
public:
    AutoGodSettings();
};

#endif // AUTOGODSETTINGS_H
