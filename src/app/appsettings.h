#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QString>

namespace AppKeys {
const char language[] = "ui/language";
const char livePreview[] = "ui/livePreview";
const char globeSpin[] = "ui/globeSpin";
const char dirImage[] = "paths/image";
const char dirPlanet[] = "paths/planet";
const char dirSettings[] = "paths/settings";
const char dirAutogen[] = "paths/autogen";
const char windowGeometry[] = "ui/windowGeometry";
const char achievementUnlockPrefix[] = "achievements/unlock/";
const char achievementCreatedCount[] = "achievements/createdCount";
}

inline QString appSettingsFile()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir);
    return QDir(dir).filePath(QStringLiteral("GodOfPixels3.ini"));
}

inline void migrateAppSettingsFromRegistry()
{
    const QString file = appSettingsFile();
    if (QFile::exists(file))
        return;
    const QSettings registry(QSettings::NativeFormat, QSettings::UserScope,
                             QStringLiteral("NikitaRiabovSoft"),
                             QStringLiteral("GodOfPixels3"));
    const QStringList keys = registry.allKeys();
    if (keys.isEmpty())
        return;
    QSettings ini(file, QSettings::IniFormat);
    for (const QString &key : keys)
        ini.setValue(key, registry.value(key));
}

inline QString startPath(const char *dirKey, const QString &fileName = QString())
{
    const QString dir = QSettings(appSettingsFile(), QSettings::IniFormat).value(dirKey).toString();
    if (dir.isEmpty() || !QDir(dir).exists())
        return fileName.isEmpty() ? QStringLiteral("./") : fileName;
    if (fileName.isEmpty())
        return dir;
    return QDir(dir).filePath(fileName);
}

inline void rememberPath(const char *dirKey, const QString &selected)
{
    if (selected.isEmpty())
        return;
    const QFileInfo info(selected);
    const QString dir = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    if (!dir.isEmpty())
        QSettings(appSettingsFile(), QSettings::IniFormat).setValue(dirKey, dir);
}

#endif
