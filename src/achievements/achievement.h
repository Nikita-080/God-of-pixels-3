#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include <QColor>
#include <QDateTime>
#include <QString>
#include <QVector>

enum class AchievementRarity
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    Special
};

struct AchievementDefaults
{
    QString icon;
    QString iconLocked;
    QString iconSecret;
    QString iconSecretLocked;
};

struct AchievementDef
{
    QString id;
    QString comment;
    AchievementRarity rarity = AchievementRarity::Common;
    bool secret = false;
    QString title;
    QString description;
    QString icon;
    QString iconLocked;

    QString resolvedIcon(bool unlocked, const AchievementDefaults &defaults) const;
};

struct AchievementRecord
{
    bool unlocked = false;
    QDateTime unlockedAt;
};

AchievementRarity achievementRarityFromString(const QString &name);
QColor achievementRarityColor(AchievementRarity rarity);

inline QString achievementMaskedTitle()
{
    return QStringLiteral("????????");
}

inline QString achievementMaskedDescription()
{
    return QStringLiteral("?????????? ????? ???????");
}

#endif
