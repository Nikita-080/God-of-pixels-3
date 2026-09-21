#include "autogensettings.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>
#include <QTextStream>
#include <QtGlobal>

namespace {

QStringList readLines(const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    QByteArray bytes = file.readAll();
    file.close();
    if (bytes.startsWith("\xEF\xBB\xBF"))
        bytes = bytes.mid(3);
    QStringList lines;
    const QStringList raw = QString::fromUtf8(bytes).split(QRegExp(QStringLiteral("\\r?\\n")), Qt::SkipEmptyParts);
    for (QString line : raw)
    {
        line = line.trimmed();
        if (!line.isEmpty())
            lines.append(line);
    }
    return lines;
}

} // namespace

AutoGenSettings::AutoGenSettings()
    : mode(AutoGenMode::Collage)
    , extendedFormat(false)
    , height(2)
    , width(2)
    , number(10)
{
}

QStringList AutoGenSettings::loadResourceLines(const QString &resourcePath)
{
    return readLines(resourcePath);
}

QStringList AutoGenSettings::canonicalFlagKeys()
{
    static const QStringList keys = readLines(QStringLiteral(":/txt_files/res/txt_files/randomsettings_en.txt"));
    return keys;
}

QVector<bool> AutoGenSettings::flagVector() const
{
    QVector<bool> flags;
    const QStringList keys = canonicalFlagKeys();
    flags.reserve(keys.size());
    for (const QString &key : keys)
        flags.append(isRndList.value(key, false));
    return flags;
}

bool AutoGenSettings::allFlagsOn() const
{
    const QStringList keys = canonicalFlagKeys();
    if (keys.isEmpty())
        return false;
    for (const QString &key : keys)
    {
        if (!isRndList.value(key, false))
            return false;
    }
    return true;
}

QJsonObject AutoGenSettings::JSON_serialize() const
{
    QJsonObject object;
    object.insert(QStringLiteral("version"), 1);
    object.insert(QStringLiteral("mode"), mode == AutoGenMode::Collage
                                              ? QStringLiteral("collage")
                                              : QStringLiteral("separate"));
    object.insert(QStringLiteral("extendedFormat"), extendedFormat);
    object.insert(QStringLiteral("path"), path);
    object.insert(QStringLiteral("height"), height);
    object.insert(QStringLiteral("width"), width);
    object.insert(QStringLiteral("number"), number);
    QJsonObject flags;
    const QStringList keys = canonicalFlagKeys();
    if (!keys.isEmpty())
    {
        for (const QString &key : keys)
            flags.insert(key, isRndList.value(key, false));
    }
    else
    {
        for (auto it = isRndList.constBegin(); it != isRndList.constEnd(); ++it)
            flags.insert(it.key(), it.value());
    }
    object.insert(QStringLiteral("isRndList"), flags);
    return object;
}

bool AutoGenSettings::JSON_deserialize(const QJsonObject &object)
{
    const QString modeName = object.value(QStringLiteral("mode")).toString(QStringLiteral("collage"));
    mode = (modeName == QStringLiteral("separate") || modeName == QStringLiteral("images"))
               ? AutoGenMode::SeparateFiles
               : AutoGenMode::Collage;
    extendedFormat = object.value(QStringLiteral("extendedFormat")).toBool(false);
    path = object.value(QStringLiteral("path")).toString();
    height = qMax(1, object.value(QStringLiteral("height")).toInt(2));
    width = qMax(1, object.value(QStringLiteral("width")).toInt(2));
    number = qMax(1, object.value(QStringLiteral("number")).toInt(10));
    isRndList.clear();
    const QJsonObject flags = object.value(QStringLiteral("isRndList")).toObject();
    const QStringList keys = canonicalFlagKeys();
    if (flags.isEmpty() && object.value(QStringLiteral("isRndList")).isArray())
    {
        const auto arr = object.value(QStringLiteral("isRndList")).toArray();
        for (int i = 0; i < keys.size(); ++i)
            isRndList.insert(keys.at(i), i < arr.size() && arr.at(i).toBool());
        return true;
    }
    for (const QString &key : keys)
        isRndList.insert(key, flags.value(key).toBool(false));
    return true;
}

bool AutoGenSettings::Save(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;
    QTextStream stream(&file);
    stream << QJsonDocument(JSON_serialize()).toJson();
    return true;
}

bool AutoGenSettings::Load(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return false;
    return JSON_deserialize(doc.object());
}
