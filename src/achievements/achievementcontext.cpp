#include "achievementcontext.h"
#include "planet.h"
#include "planet_p.h"

AchievementContext AchievementContext::fromPlanet(const Planet &planet)
{
    AchievementContext ctx;
    ctx.seed = planet.seed;
    ctx.name = planet.name;
    ctx.facts = planet.facts;
    ctx.plantPixelCount = planet.plant_pixel_count;
    ctx.waterPixelCount = planet.water_pixel_count;
    ctx.icePixelCount = planet.ice_pixel_count;
    ctx.hasCities = !planet.cities.isEmpty();
    ctx.hasStar = planet.s.has_star;
    ctx.hasAtmo = planet.s.is_atmo;
    ctx.hasRings = planet.s.is_ring;
    ctx.atmoSize = planet.s.atmo_size;
    ctx.hazardLight = planet.s.hazardLight();
    ctx.tags = planetActiveTagIds(planet);
    return ctx;
}
