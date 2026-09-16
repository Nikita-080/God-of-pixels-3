#include "achievementcontext.h"
#include "planet.h"
#include "planet_p.h"
#include <QDate>

bool structureOnlyShelfAndOcean(const QVector<double> &st)
{
    if (st.size() < 8)
        return false;
    for (int i = 0; i < 5; ++i)
    {
        if (st[i] != st[i + 1])
            return false;
    }
    const bool shelf = st[5] != st[6];
    const bool ocean = st[6] != st[7];
    return shelf && ocean;
}

AchievementContext AchievementContext::fromPlanet(const Planet &planet)
{
    AchievementContext ctx;
    ctx.seed = planet.seed;
    ctx.name = planet.name;
    ctx.facts = planet.facts;
    ctx.plantPixelCount = planet.plant_pixel_count;
    ctx.waterPixelCount = planet.water_pixel_count;
    ctx.icePixelCount = planet.ice_pixel_count;
    ctx.cityCount = planet.cities.size();
    ctx.hasCities = !planet.cities.isEmpty();
    ctx.hasStar = planet.s.has_star;
    ctx.hasAtmo = planet.s.is_atmo;
    ctx.hasRings = planet.s.is_ring;
    ctx.isSunday = QDate::currentDate().dayOfWeek() == Qt::Sunday;
    ctx.shelfOceanOnly = structureOnlyShelfAndOcean(planet.s.true_structure);
    ctx.atmoSize = planet.s.atmo_size;
    ctx.hazardLight = planet.s.hazardLight();
    const QVector<PlanetOre> ores = planetOreInventory(planet);
    if (!ores.isEmpty())
        ctx.firstResourceSymbol = ores.first().symbol;
    ctx.tags = planetActiveTagIds(planet);
    ctx.cardTagIds = planet.cardTagIds;
    ctx.cardLabelKeys = planet.cardLabelKeys;
    return ctx;
}
