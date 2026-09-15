#ifndef ACHIEVEMENTSTORE_H
#define ACHIEVEMENTSTORE_H

#include "achievement.h"
#include <QHash>

class AchievementStore
{
public:
    static bool isUnlocked(const QString &id);
    static QDateTime unlockedAt(const QString &id);
    static bool unlock(const QString &id);
    static QHash<QString, QDateTime> allUnlocked();
};

#endif
