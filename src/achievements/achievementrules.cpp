#include "achievementrules.h"
#include "achievementcontext.h"
#include <QHash>
#include <QString>
#include <functional>

// Register a predicate per achievement id from achievements.json, e.g.:
// table.insert(QStringLiteral("habitable"), [](const AchievementContext &c) {
//     return c.hasTag(QStringLiteral("habitable"));
// });

namespace {

using AchievementPredicate = std::function<bool(const AchievementContext &)>;

const QHash<QString, AchievementPredicate> &ruleTable()
{
    static const QHash<QString, AchievementPredicate> table;
    return table;
}

} // namespace

bool achievementHasRule(const QString &id)
{
    return ruleTable().contains(id);
}

bool achievementRuleMatches(const QString &id, const AchievementContext &ctx)
{
    const auto it = ruleTable().constFind(id);
    if (it == ruleTable().cend())
        return false;
    return (*it)(ctx);
}
