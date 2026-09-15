#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QDir>
#include <QFileInfo>
#include <QSettings>
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
}

inline QString startPath(const char *dirKey, const QString &fileName = QString())
{
    const QString dir = QSettings().value(dirKey).toString();
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
        QSettings().setValue(dirKey, dir);
}

#endif
