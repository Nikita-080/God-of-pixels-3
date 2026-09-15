#include "achievementcatalog.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

struct CatalogData
{
    AchievementDefaults defaults;
    QVector<AchievementDef> list;
    bool loaded = false;
};

CatalogData &catalogData()
{
    static CatalogData data;
    if (!data.loaded)
    {
        QFile file(QStringLiteral(":/txt_files/res/txt_files/achievements.json"));
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isObject())
            {
                const QJsonObject root = doc.object();
                const QJsonObject defObj = root.value(QLatin1String("defaults")).toObject();
                data.defaults.icon = defObj.value(QLatin1String("icon")).toString();
                data.defaults.iconLocked = defObj.value(QLatin1String("icon_locked")).toString();
                data.defaults.iconSecret = defObj.value(QLatin1String("icon_secret")).toString();
                data.defaults.iconSecretLocked = defObj.value(QLatin1String("icon_secret_locked")).toString();

                const QJsonArray arr = root.value(QLatin1String("achievements")).toArray();
                for (const QJsonValue &v : arr)
                {
                    const QJsonObject o = v.toObject();
                    AchievementDef d;
                    d.id = o.value(QLatin1String("id")).toString().trimmed();
                    d.comment = o.value(QLatin1String("_comment")).toString().trimmed();
                    if (d.id.isEmpty() || d.comment.isEmpty())
                        continue;
                    d.rarity = achievementRarityFromString(o.value(QLatin1String("rarity")).toString().trimmed());
                    d.secret = o.value(QLatin1String("secret")).toBool(false);
                    d.title = o.value(QLatin1String("title")).toString();
                    d.description = o.value(QLatin1String("description")).toString();
                    d.icon = o.value(QLatin1String("icon")).toString().trimmed();
                    d.iconLocked = o.value(QLatin1String("icon_locked")).toString().trimmed();
                    data.list.append(d);
                }
            }
        }
        data.loaded = true;
    }
    return data;
}

} // namespace

const AchievementDefaults &achievementDefaults()
{
    return catalogData().defaults;
}

const QVector<AchievementDef> &achievementCatalog()
{
    return catalogData().list;
}
