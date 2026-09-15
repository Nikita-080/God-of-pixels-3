#ifndef ACHIEVEMENTRULES_H
#define ACHIEVEMENTRULES_H

class AchievementContext;
class QString;

bool achievementHasRule(const QString &id);
bool achievementRuleMatches(const QString &id, const AchievementContext &ctx);

#endif
