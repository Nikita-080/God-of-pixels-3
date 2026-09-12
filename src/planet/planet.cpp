#include "planet.h"
#include "terrafactory.h"
#include <QRandomGenerator>
#include <QtMath>

Planet::Planet()
{
    color_black = QColor(0, 0, 0);
    facts = Facts();
    map_w = 0;
    map_h = 0;
    world_size = 0;
    seed = 0;
}

void Planet::SetSeed(int value)
{
    if (value == 0)
        seed = int(QRandomGenerator::global()->generate() & 0x7fffffffu);
    else
        seed = value;
    rnd.seed(static_cast<quint32>(seed));
}

int Planet::RAND(int a, int b)
{
    return rnd.bounded(a, b + 1);
}

void Planet::Generate()
{
    CreateMatrixNew();
    Calculator();
    FixMatrix();
    LevelCreating();
    ImageCreating();
    TMapCreating();
    RMapCreating();
    Plant();
    Polar();
    Noise();
    Civilization();
    CloudMapCreating();
    CloudImageCreating();
    PrepareRings();
    rnd.seed(static_cast<quint32>(seed));
    Name();
    rnd.seed(static_cast<quint32>(seed));
    GenerateDescription();
    CalculateDescription();
    DrawDescription();
    rnd.seed(static_cast<quint32>(seed));
    SystemMap();
    rnd.seed(static_cast<quint32>(seed));
    GalaxyMap();
    ImagesScale();
    FinalImage();
}

SphereVec3 Planet::TexelXYZ(int x, int y) const
{
    return equirectToSphere(x, y, map_w, map_h);
}

int Planet::viewResolution() const
{
    const int q = qBound(1, s.world_size, 20);
    return 16 + 24 * (q - 1);
}

void Planet::Calculator()
{
    plant_pixel_count = 0;
    ice_pixel_count = 0;
    water_pixel_count = 0;

    rnd.seed(static_cast<quint32>(seed));
    starclass.clear();
    int numstars;
    if (s.temperature == -90)
        numstars = RAND(0, 2);
    else
        numstars = RAND(1, 2);
    for (int i = 0; i < numstars; i++)
        starclass.append(RAND(0, 11));

    R_planet = 1.0;
    if (s.is_atmo)
        R_atmo = R_planet * (81 + s.atmo_size) / 81.0;
    else
        R_atmo = R_planet;
    R_final = qMax(R_atmo, R_planet);
    const SphereVec3 shine = latLonDegToSphere(s.shine_lat, s.shine_lon);
    x_shine = shine.x;
    y_shine = shine.y;
    z_shine = shine.z;
    const SphereVec3 polar = latLonDegToSphere(s.polar_lat, s.polar_lon);
    x_polar = polar.x;
    y_polar = polar.y;
    z_polar = polar.z;
    if (s.true_structure.size() > 5)
        water_level = s.true_structure[5];
    else
        water_level = 140;

    world_deep = matrix[0][0];
    world_heighth = matrix[0][0];
    for (int i = 0; i < map_w; i++)
        for (int k = 0; k < map_h; k++)
        {
            world_deep = fmin(world_deep, matrix[i][k]);
            world_heighth = fmax(world_heighth, matrix[i][k]);
        }
    if (qFuzzyCompare(world_heighth, world_deep))
        world_heighth = world_deep + 1.0;
}

double Planet::ArcPolarDistance(int x, int y) const
{
    SphereVec3 p = TexelXYZ(x, y);
    SphereVec3 pole = {x_polar, y_polar, z_polar};
    pole = sphereNorm(pole);
    const double den = sphereLen(p) * sphereLen(pole);
    if (den <= 1e-12)
        return 0.0;
    return sphereDot(p, pole) / den;
}
