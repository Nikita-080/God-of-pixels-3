#include "planet.h"
#include "cloudfactory.h"
#include <QtMath>
#include <QRgb>

void Planet::CloudMapCreating()
{
    c_map.clear();
    c_map.resize(map_w);
    if (!s.is_cloud)
    {
        for (int x = 0; x < map_w; x++)
            c_map[x].fill(0.0, map_h);
        return;
    }
    const double t = (qBound(1, s.cloud_size, 10) - 1) / 9.0;
    const double freq = 8.5 - 7.7 * t;
    CloudFactory pnf(qBound(1, s.cloud_quality, 6), s.correction, seed);
    for (int x = 0; x < map_w; x++)
    {
        c_map[x].resize(map_h);
        for (int y = 0; y < map_h; y++)
        {
            const SphereVec3 p = TexelXYZ(x, y);
            double n = pnf.GetNum(p.x * freq, p.y * freq, p.z * freq);
            c_map[x][y] = n * 0.5 + 0.5;
        }
    }
}

void Planet::CloudImageCreating()
{
    img_clouds = QImage(map_w, map_h, QImage::Format_ARGB32);
    img_clouds.fill(Qt::transparent);
    if (!s.is_cloud)
        return;
    for (int x = 0; x < map_w; x++)
    {
        for (int y = 0; y < map_h; y++)
        {
            const double n = c_map[x][y];
            double a = (n - 0.40) / 0.32;
            a = qBound(0.0, a, 1.0);
            a = a * a * (3.0 - 2.0 * a);
            QColor c = s.cloud_color;
            c.setAlpha(qBound(0, qRound(a * 255.0), 255));
            QRgb *line = reinterpret_cast<QRgb *>(img_clouds.scanLine(y));
            line[x] = c.rgba();
        }
    }
}
