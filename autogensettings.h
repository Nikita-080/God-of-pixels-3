#ifndef AUTOGENSETTINGS_H
#define AUTOGENSETTINGS_H
#include <QVector>
#include <QString>

class AutoGenSettings
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
    AutoGenSettings();
};

#endif // AUTOGENSETTINGS_H
