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
    bool hasCities = false;
    bool hasStar = false;
    bool hasAtmo = false;
    bool hasRings = false;
    int atmoSize = 0;
    double hazardLight = 0.0;
    QSet<QString> tags;

    bool hasTag(const QString &id) const { return tags.contains(id); }

    static AchievementContext fromPlanet(const Planet &planet);
};

#endif
