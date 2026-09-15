#include "achievementrules.h"
#include "achievementcontext.h"
#include "planet_p.h"
#include <QHash>
#include <QString>
#include <functional>

namespace {

using AchievementPredicate = std::function<bool(const AchievementContext &)>;

const QHash<QString, AchievementPredicate> &ruleTable()
{
    static QHash<QString, AchievementPredicate> table;
    static bool ready = false;
    if (!ready)
    {
        table.insert(QStringLiteral("a_1_planet"), [](const AchievementContext &c) {
            return c.createdCount >= 1;
        });
        table.insert(QStringLiteral("a_50_planet"), [](const AchievementContext &c) {
            return c.createdCount >= 50;
        });
        table.insert(QStringLiteral("a_100_planet"), [](const AchievementContext &c) {
            return c.createdCount >= 100;
        });
        table.insert(QStringLiteral("a_200_planet"), [](const AchievementContext &c) {
            return c.createdCount >= 200;
        });
        table.insert(QStringLiteral("a_300_planet"), [](const AchievementContext &c) {
            return c.createdCount >= 300;
        });
        table.insert(QStringLiteral("a_kzzzkt"), [](const AchievementContext &c) {
            return c.cardLabelKeys.contains(QStringLiteral("kzzzkt"));
        });
        table.insert(QStringLiteral("a_sunday"), [](const AchievementContext &c) {
            return c.isSunday;
        });
        table.insert(QStringLiteral("a_green"), [](const AchievementContext &c) {
            return planetDescriptionBarsAll(c.facts, DescriptionBarColor::Green);
        });
        table.insert(QStringLiteral("a_red"), [](const AchievementContext &c) {
            return planetDescriptionBarsAll(c.facts, DescriptionBarColor::Red);
        });
        table.insert(QStringLiteral("a_1_city"), [](const AchievementContext &c) {
            return c.cityCount >= 1;
        });
        table.insert(QStringLiteral("a_100_cities"), [](const AchievementContext &c) {
            return c.cityCount >= 100;
        });
        table.insert(QStringLiteral("a_no_star"), [](const AchievementContext &c) {
            return !c.hasStar;
        });
        table.insert(QStringLiteral("a_message"), [](const AchievementContext &c) {
            return c.cardTagIds.contains(QStringLiteral("easter_civ"));
        });
        table.insert(QStringLiteral("a_plant"), [](const AchievementContext &c) {
            return c.plantPixelCount > 0;
        });
        ready = true;
    }
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
