#ifndef ACHIEVEMENTENGINE_H
#define ACHIEVEMENTENGINE_H

#include "achievement.h"

struct AchievementContext;

class AchievementEngine
{
public:
    static QVector<AchievementDef> evaluate(const AchievementContext &ctx);
};

#endif
