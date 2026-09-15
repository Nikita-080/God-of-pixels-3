#include "achievementengine.h"
#include "achievementcatalog.h"
#include "achievementcontext.h"
#include "achievementrules.h"
#include "achievementstore.h"
#include <QDebug>

QVector<AchievementDef> AchievementEngine::evaluate(const AchievementContext &ctx)
{
    static bool warned = false;
    const QVector<AchievementDef> &catalog = achievementCatalog();
    if (!warned)
    {
        for (const AchievementDef &def : catalog)
        {
            if (!achievementHasRule(def.id))
                qDebug() << "Achievement" << def.id << "has no rule";
        }
        warned = true;
    }

    QVector<AchievementDef> newly;
    for (const AchievementDef &def : catalog)
    {
        if (AchievementStore::isUnlocked(def.id))
            continue;
        if (!achievementHasRule(def.id))
            continue;
        if (!achievementRuleMatches(def.id, ctx))
            continue;
        if (AchievementStore::unlock(def.id))
            newly.append(def);
    }
    return newly;
}
