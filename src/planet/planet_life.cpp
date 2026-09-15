#include "planet.h"
#include "planet_p.h"
#include <QPoint>
#include <algorithm>
#include <QtMath>
#include <QRgb>

void Planet::Civilization()
{
    cities.clear();
    if (!s.is_civ || !s.is_plant || !s.is_atmo || !(water_level > 0) || plant_pixel_count <= 0)
        return;
    if (map_w <= 0 || map_h <= 0 || s.true_structure.size() < 5)
        return;

    rnd.seed(static_cast<quint32>(seed) ^ 0xC17u);

    QVector<QVector<double>> waterNear(map_w);
    for (int x = 0; x < map_w; ++x)
    {
        waterNear[x].resize(map_h);
        for (int y = 0; y < map_h; ++y)
            waterNear[x][y] = matrix[x][y] < water_level ? 1.0 : 0.0;
    }
    const int nearR = qMax(2, map_w / 48);
    planetBoxBlurWrapX(waterNear, nearR);
    planetBoxBlurClampY(waterNear, nearR);

    const double plainsHi = s.true_structure[3];
    const double plainsLo = s.true_structure[4];
    const double plainsMid = 0.5 * (plainsHi + plainsLo);
    const double plainsSig = qMax(8.0, 0.5 * qAbs(plainsHi - plainsLo));
    const double tIdeal = 10.0;
    const double tSig = 11.0;
    const double wIdeal = 95.0;
    const double wSig = 70.0;

    struct Cand { int x; int y; float score; };
    QVector<Cand> cand;
    cand.reserve(map_w * map_h / 8);
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (matrix[x][y] <= water_level)
                continue;
            if (!faultKind.isEmpty() && faultKind[x][y] != 0)
                continue;
            const double T = t_map[x][y];
            if (T < -8.0)
                continue;
            const double hScore = qExp(-0.5 * qPow((matrix[x][y] - plainsMid) / plainsSig, 2.0));
            const double waterScore = qBound(0.0, waterNear[x][y] / 0.55, 1.0);
            const double tScore = qExp(-0.5 * qPow((T - tIdeal) / tSig, 2.0));
            const double wScore = qExp(-0.5 * qPow((r_map[x][y] - wIdeal) / wSig, 2.0));
            const float score = float(hScore * waterScore * tScore * wScore);
            if (score > 0.12f)
                cand.append({x, y, score});
        }
    }
    std::sort(cand.begin(), cand.end(), [](const Cand &a, const Cand &b) {
        return a.score > b.score;
    });

    const double lifeMul = 1.0 - s.hazardLight();
    const int maxCities = qBound(0, qRound((qBound(18, map_w / 2, 210)) * lifeMul), 210);
    const int minDist2 = qMax(9, ((map_w / 28) * (map_w / 28)) / 3);
    QVector<QPoint> placed;
    for (const Cand &c : cand)
    {
        if (cities.size() >= maxCities)
            break;
        if (rnd.generateDouble() > qMin(1.0, 3.0 * (0.28 * double(c.score) + 0.04) * lifeMul))
            continue;
        bool far = true;
        for (const QPoint &p : placed)
        {
            int dx = qAbs(c.x - p.x());
            dx = qMin(dx, map_w - dx);
            const int dy = c.y - p.y();
            if (dx * dx + dy * dy < minDist2)
            {
                far = false;
                break;
            }
        }
        if (!far)
            continue;
        placed.append(QPoint(c.x, c.y));
        const SphereVec3 p = TexelXYZ(c.x, c.y);
        CityLight city;
        city.x = float(p.x * 1.004);
        city.y = float(p.y * 1.004);
        city.z = float(p.z * 1.004);
        city.mapX = c.x;
        city.mapY = c.y;
        cities.append(city);

        const QColor glow = s.civ_color;
        for (int ox = -1; ox <= 1; ++ox)
        {
            for (int oy = -1; oy <= 1; ++oy)
            {
                const int px = (c.x + ox + map_w) % map_w;
                const int py = qBound(0, c.y + oy, map_h - 1);
                QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(py));
                if (ox == 0 && oy == 0)
                    line[px] = glow.rgb();
                else
                    line[px] = TransparentColor(QColor::fromRgb(line[px]), glow, 0.55).rgb();
            }
        }
    }
}
