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
    return QSettings().contains(unlockKey(id));
}

QDateTime AchievementStore::unlockedAt(const QString &id)
{
    if (id.isEmpty())
        return QDateTime();
    return QDateTime::fromString(QSettings().value(unlockKey(id)).toString(), Qt::ISODate);
}

bool AchievementStore::unlock(const QString &id)
{
    if (id.isEmpty() || isUnlocked(id))
        return false;
    QSettings().setValue(unlockKey(id), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    return true;
}

QHash<QString, QDateTime> AchievementStore::allUnlocked()
{
    QHash<QString, QDateTime> out;
    QSettings st;
    st.beginGroup(QStringLiteral("achievements/unlock"));
    const QStringList keys = st.childKeys();
    for (const QString &id : keys)
        out.insert(id, QDateTime::fromString(st.value(id).toString(), Qt::ISODate));
    return out;
}

int AchievementStore::createdCount()
{
    return QSettings().value(QLatin1String(AppKeys::achievementCreatedCount), 0).toInt();
}

int AchievementStore::addCreatedPlanet()
{
    QSettings st;
    const int n = st.value(QLatin1String(AppKeys::achievementCreatedCount), 0).toInt() + 1;
    st.setValue(QLatin1String(AppKeys::achievementCreatedCount), n);
    return n;
}
