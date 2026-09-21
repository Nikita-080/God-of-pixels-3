#ifndef AUTOGENSETTINGS_H
#define AUTOGENSETTINGS_H
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonObject>

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
    QMap<QString, bool> isRndList;

    AutoGenSettings();

    QJsonObject JSON_serialize() const;
    bool JSON_deserialize(const QJsonObject &object);
    bool Save(const QString &path) const;
    bool Load(const QString &path);

    QVector<bool> flagVector() const;
    bool allFlagsOn() const;

    static QStringList canonicalFlagKeys();
    static QStringList loadResourceLines(const QString &resourcePath);
};

#endif
