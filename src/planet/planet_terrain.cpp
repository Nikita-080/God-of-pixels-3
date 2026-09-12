#include "planet.h"
#include "terrafactory.h"

void Planet::CreateMatrixNew()
{
    const int view = viewResolution();
    map_h = qMax(32, view);
    map_w = 2 * map_h;
    world_size = view;
    TerraFactory factory(map_w, map_h, seed);
    if (s.terramode == 0)
        matrix = factory.sphericalNoise(s.randomness);
    else
        matrix = factory.sphericalFault(s.iterations);
}

void Planet::FixMatrix()
{
    for (int i = 0; i < map_w; i++)
    {
        for (int k = 0; k < map_h; k++)
        {
            matrix[i][k] = (matrix[i][k] - world_deep) * 280 / (world_heighth - world_deep);
        }
    }
}

void Planet::LevelCreating()
{
    QVector<QColor> colors = {s.ice_color, s.rock_color, s.mountain_color, s.plain_color,
                             s.beach_color, s.shallow_color, s.ocean_color};
    level_up.clear();
    level_down.clear();
    level_color.clear();
    for (int i = 0; i < 7; i++)
    {
        if (s.true_structure[i] != s.true_structure[i + 1])
        {
            level_up.append(s.true_structure[i]);
            level_down.append(s.true_structure[i + 1]);
            level_color.append(colors[i]);
        }
    }
}
