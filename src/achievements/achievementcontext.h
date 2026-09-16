#ifndef ACHIEVEMENTCONTEXT_H
#define ACHIEVEMENTCONTEXT_H

#include <facts.h>
#include <QSet>
#include <QString>

class Planet;

struct AchievementContext
{
    int seed = 0;
    QString name;
    Facts facts;
    int plantPixelCount = 0;
    int waterPixelCount = 0;
    int icePixelCount = 0;
    int cityCount = 0;
    int createdCount = 0;
    bool hasCities = false;
    bool hasStar = false;
    bool hasAtmo = false;
    bool hasRings = false;
    bool isSunday = false;
    bool planetSaved = false;
    bool autogenAllRandom = false;
    bool shelfOceanOnly = false;
    int atmoSize = 0;
    double hazardLight = 0.0;
    QString firstResourceSymbol;
    QSet<QString> tags;
    QSet<QString> cardTagIds;
    QSet<QString> cardLabelKeys;

    bool hasTag(const QString &id) const { return tags.contains(id); }

    static AchievementContext fromPlanet(const Planet &planet);
};

#endif
