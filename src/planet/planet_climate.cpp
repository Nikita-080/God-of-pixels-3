#include "planet.h"
#include "planet_p.h"
#include <QtMath>

void planetBoxBlurWrapX(QVector<QVector<double>> &field, int radius)
{
    const int w = field.size();
    if (w <= 0 || radius <= 0)
        return;
    const int h = field[0].size();
    const int span = 2 * radius + 1;
    QVector<QVector<double>> out(w);
    for (int x = 0; x < w; ++x)
        out[x].resize(h);
    const double inv = 1.0 / span;
    for (int y = 0; y < h; ++y)
    {
        double sum = 0.0;
        for (int i = -radius; i <= radius; ++i)
            sum += field[(i % w + w) % w][y];
        out[0][y] = sum * inv;
        for (int x = 1; x < w; ++x)
        {
            sum -= field[((x - 1 - radius) % w + w) % w][y];
            sum += field[(x + radius) % w][y];
            out[x][y] = sum * inv;
        }
    }
    field.swap(out);
}

void planetBoxBlurClampY(QVector<QVector<double>> &field, int radius)
{
    const int w = field.size();
    if (w <= 0 || radius <= 0)
        return;
    const int h = field[0].size();
    QVector<double> prefix(h + 1);
    QVector<QVector<double>> out(w);
    for (int x = 0; x < w; ++x)
        out[x].resize(h);
    for (int x = 0; x < w; ++x)
    {
        prefix[0] = 0.0;
        for (int y = 0; y < h; ++y)
            prefix[y + 1] = prefix[y] + field[x][y];
        for (int y = 0; y < h; ++y)
        {
            const int y0 = qMax(0, y - radius);
            const int y1 = qMin(h - 1, y + radius);
            out[x][y] = (prefix[y1 + 1] - prefix[y0]) / double(y1 - y0 + 1);
        }
    }
    field.swap(out);
}

void Planet::TMapCreating()
{
    t_map.clear();
    t_map.resize(map_w);
    for (int i = 0; i < map_w; i++)
    {
        t_map[i].resize(map_h);
        for (int k = 0; k < map_h; k++)
        {
            double angle_cos = ArcPolarDistance(i, k);
            angle_cos = qBound(-1.0, angle_cos, 1.0);
            double angle_sin = sqrt(1 - angle_cos * angle_cos);
            double r_w = matrix[i][k] - water_level;
            r_w *= 31.6;
            double T = 56 * angle_sin - 28;
            T -= 0.6 * (r_w) / 100;
            T += s.effectiveTemperature() - 15;
            t_map[i][k] = T;
        }
    }
}

void Planet::RMapCreating()
{
    r_map.clear();
    r_map.resize(map_w);
    QVector<QVector<double>> waterNear(map_w);
    for (int i = 0; i < map_w; ++i)
    {
        r_map[i].resize(map_h);
        waterNear[i].resize(map_h);
        for (int k = 0; k < map_h; ++k)
        {
            const double r_w = matrix[i][k] - water_level;
            r_map[i][k] = 1.37 * r_w + 0.32 * t_map[i][k] + 51.53;
            waterNear[i][k] = matrix[i][k] < water_level ? 1.0 : 0.0;
        }
    }

    const int radius = qMax(1, map_w / 12);
    planetBoxBlurWrapX(waterNear, radius);
    planetBoxBlurClampY(waterNear, radius);

    const double pCoast = 0.5;
    const double pCenter = 1.0 - M_PI / 4.0;
    const double invSpan = 1.0 / (pCoast - pCenter);
    for (int i = 0; i < map_w; ++i)
    {
        for (int k = 0; k < map_h; ++k)
        {
            if (matrix[i][k] <= water_level)
                continue;
            const double p = waterNear[i][k];
            const double wet = qMin(1.0, 0.75 + 0.25 * (p - pCenter) * invSpan);
            r_map[i][k] *= wet;
        }
    }
    if (!lavaHeat.isEmpty() && lavaHeat.size() == map_w)
    {
        for (int i = 0; i < map_w; ++i)
        {
            for (int k = 0; k < map_h; ++k)
                r_map[i][k] *= (1.0 - 0.85 * qBound(0.0, lavaHeat[i][k], 1.0));
        }
    }
}
