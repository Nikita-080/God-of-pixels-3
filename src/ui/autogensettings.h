#ifndef AUTOGENSETTINGS_H
#define AUTOGENSETTINGS_H
#include <QVector>
#include <QString>

enum class AutoGenMode
{
    Collage,
    SeparateFiles
};

class AutoGenSettings
{
public:
    AutoGenMode mode;
    bool extendedFormat;
    QString path;
    int height;
    int width;
    int number;
    QVector<bool> isRndList;

    AutoGenSettings();
};

#endif
