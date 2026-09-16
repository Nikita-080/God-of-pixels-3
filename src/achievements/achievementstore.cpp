#include "achievementstore.h"
#include "appsettings.h"
#include <QSettings>

namespace {

QString unlockKey(const QString &id)
{
    return QLatin1String(AppKeys::achievementUnlockPrefix) + id;
}

} // namespace

bool AchievementStore::isUnlocked(const QString &id)
{
    if (id.isEmpty())
        return false;
    return QSettings(appSettingsFile(), QSettings::IniFormat).contains(unlockKey(id));
}

QDateTime AchievementStore::unlockedAt(const QString &id)
{
    if (id.isEmpty())
        return QDateTime();
    return QDateTime::fromString(
        QSettings(appSettingsFile(), QSettings::IniFormat).value(unlockKey(id)).toString(), Qt::ISODate);
}

bool AchievementStore::unlock(const QString &id)
{
    if (id.isEmpty() || isUnlocked(id))
        return false;
    QSettings(appSettingsFile(), QSettings::IniFormat)
        .setValue(unlockKey(id), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    return true;
}

QHash<QString, QDateTime> AchievementStore::allUnlocked()
{
    QHash<QString, QDateTime> out;
    QSettings st(appSettingsFile(), QSettings::IniFormat);
    st.beginGroup(QStringLiteral("achievements/unlock"));
    const QStringList keys = st.childKeys();
    for (const QString &id : keys)
        out.insert(id, QDateTime::fromString(st.value(id).toString(), Qt::ISODate));
    return out;
}

int AchievementStore::createdCount()
{
    return QSettings(appSettingsFile(), QSettings::IniFormat)
        .value(QLatin1String(AppKeys::achievementCreatedCount), 0)
        .toInt();
}

int AchievementStore::addCreatedPlanet()
{
    QSettings st(appSettingsFile(), QSettings::IniFormat);
    const int n = st.value(QLatin1String(AppKeys::achievementCreatedCount), 0).toInt() + 1;
    st.setValue(QLatin1String(AppKeys::achievementCreatedCount), n);
    return n;
}
