#include "achievement.h"

AchievementRarity achievementRarityFromString(const QString &name)
{
    if (name == QLatin1String("uncommon"))
        return AchievementRarity::Uncommon;
    if (name == QLatin1String("rare"))
        return AchievementRarity::Rare;
    if (name == QLatin1String("epic"))
        return AchievementRarity::Epic;
    if (name == QLatin1String("legendary"))
        return AchievementRarity::Legendary;
    if (name == QLatin1String("special"))
        return AchievementRarity::Special;
    return AchievementRarity::Common;
}

QColor achievementRarityColor(AchievementRarity rarity)
{
    switch (rarity)
    {
    case AchievementRarity::Uncommon:
        return QColor(80, 180, 80);
    case AchievementRarity::Rare:
        return QColor(70, 120, 220);
    case AchievementRarity::Epic:
        return QColor(160, 80, 220);
    case AchievementRarity::Legendary:
        return QColor(220, 180, 50);
    case AchievementRarity::Special:
        return QColor(200, 50, 50);
    case AchievementRarity::Common:
    default:
        return QColor(140, 140, 150);
    }
}

QString AchievementDef::resolvedIcon(bool unlocked, const AchievementDefaults &defaults) const
{
    if (unlocked)
    {
        if (!icon.isEmpty())
            return icon;
        return secret ? defaults.iconSecret : defaults.icon;
    }
    if (!iconLocked.isEmpty())
        return iconLocked;
    return secret ? defaults.iconSecretLocked : defaults.iconLocked;
}
