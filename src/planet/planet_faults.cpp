#include "planet.h"
#include "noise3d.h"
#include <QQueue>
#include <QPair>
#include <QtMath>
#include <QPoint>
#include <queue>

namespace {
const quint8 kFaultNone = 0;
const quint8 kFaultDark = 1;
const quint8 kFaultWater = 2;
const quint8 kFaultLava = 3;

SphereVec3 randomSite(QRandomGenerator &rnd)
{
    double x, y, z, len2;
    do
    {
        x = rnd.generateDouble() * 2.0 - 1.0;
        y = rnd.generateDouble() * 2.0 - 1.0;
        z = rnd.generateDouble() * 2.0 - 1.0;
        len2 = x * x + y * y + z * z;
    } while (len2 < 0.04 || len2 > 1.0);
    return sphereNorm({x, y, z});
}

int wrapX(int x, int w)
{
    return (x % w + w) % w;
}

struct LavaHeatNode
{
    double d;
    int x;
    int y;
    double fall;
    bool operator<(const LavaHeatNode &o) const { return d > o.d; }
};
}

void Planet::ApplyFaults()
{
    faultKind.clear();
    lavaHeat.clear();
    if (map_w <= 0 || map_h <= 0)
        return;
    faultKind.resize(map_w);
    lavaHeat.resize(map_w);
    for (int x = 0; x < map_w; ++x)
    {
        faultKind[x].fill(kFaultNone, map_h);
        lavaHeat[x].fill(0.0, map_h);
    }

    const int S = qBound(0, s.seismicity, 12);
    if (S <= 0)
        return;

    const double t = S / 12.0;
    const int nSites = qBound(64, (map_w * 3) / 4, 180);
    QVector<SphereVec3> sites;
    sites.reserve(nSites);
    rnd.seed(static_cast<quint32>(seed) ^ 0x5E15u);
    for (int i = 0; i < nSites; ++i)
        sites.append(randomSite(rnd));

    Noise3D n(seed ^ 0xFA17);
    const double warpAmp = 0.07;
    const double pixelAng = 2.0 * M_PI / double(map_w);
    const double baseW = pixelAng * 1.15;
    const double freq = 2.2;

    QVector<QVector<float>> strength(map_w);
    const float crackCut = 0.35f;
    for (int x = 0; x < map_w; ++x)
    {
        strength[x].fill(0.0f, map_h);
        for (int y = 0; y < map_h; ++y)
        {
            const SphereVec3 p = TexelXYZ(x, y);
            const double nx = n.fbm(p.x * freq, p.y * freq, p.z * freq, 3);
            const double ny = n.fbm(p.x * freq + 17.1, p.y * freq + 4.3, p.z * freq + 9.7, 3);
            const double nz = n.fbm(p.x * freq + 3.8, p.y * freq + 21.4, p.z * freq + 1.2, 3);
            const SphereVec3 warped = sphereNorm({
                p.x + warpAmp * nx,
                p.y + warpAmp * ny,
                p.z + warpAmp * nz
            });

            int i1 = 0;
            int i2 = 0;
            double d1 = 4.0;
            double d2 = 4.0;
            for (int i = 0; i < sites.size(); ++i)
            {
                const double d = acos(qBound(-1.0, sphereDot(warped, sites[i]), 1.0));
                if (d < d1)
                {
                    d2 = d1;
                    i2 = i1;
                    d1 = d;
                    i1 = i;
                }
                else if (d < d2)
                {
                    d2 = d;
                    i2 = i;
                }
            }
            if (i1 == i2)
                continue;
            const double widthMod = 1.0 + 0.25 * n.fbm(p.x * 3.1, p.y * 3.1, p.z * 3.1, 2);
            const double w = baseW * widthMod;
            const double gap = d2 - d1;
            if (gap >= w)
                continue;
            strength[x][y] = float(1.0 - gap / w);
        }
    }

    const int ndx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    const int ndy[8] = {0, 0, 1, -1, 1, -1, 1, -1};

    QVector<QPoint> skeleton;
    skeleton.reserve(map_w * 4);
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (strength[x][y] >= crackCut)
                skeleton.append(QPoint(x, y));
        }
    }
    if (skeleton.isEmpty())
        return;

    const int seedCount = qMax(1, qRound(0.7 * S + 8.0 * t * t));
    const int maxDist = (S >= 12)
                            ? (map_w + map_h) * 3
                            : qMax(5, qRound(7.0 + map_w * 0.55 * t * t));
    QVector<QVector<int>> dist(map_w);
    for (int x = 0; x < map_w; ++x)
        dist[x].fill(-1, map_h);

    QQueue<QPair<int, int>> q;
    int placed = 0;
    int attempts = 0;
    while (placed < seedCount && attempts < seedCount * 8)
    {
        ++attempts;
        const QPoint sp = skeleton[rnd.bounded(skeleton.size())];
        if (dist[sp.x()][sp.y()] >= 0)
            continue;
        dist[sp.x()][sp.y()] = 0;
        q.enqueue(qMakePair(sp.x(), sp.y()));
        ++placed;
    }
    if (q.isEmpty())
    {
        const QPoint sp = skeleton.first();
        dist[sp.x()][sp.y()] = 0;
        q.enqueue(qMakePair(sp.x(), sp.y()));
    }

    while (!q.isEmpty())
    {
        const QPair<int, int> cur = q.dequeue();
        const int cd = dist[cur.first][cur.second];
        if (cd >= maxDist)
            continue;
        for (int i = 0; i < 8; ++i)
        {
            const int nx = wrapX(cur.first + ndx[i], map_w);
            const int ny = cur.second + ndy[i];
            if (ny < 0 || ny >= map_h)
                continue;
            if (strength[nx][ny] < crackCut)
                continue;
            if (dist[nx][ny] >= 0)
                continue;
            dist[nx][ny] = cd + 1;
            q.enqueue(qMakePair(nx, ny));
        }
    }

    QVector<QVector<int>> comp(map_w);
    for (int x = 0; x < map_w; ++x)
        comp[x].fill(-1, map_h);
    QVector<int> compSize;
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (dist[x][y] < 0 || comp[x][y] >= 0)
                continue;
            const int id = compSize.size();
            compSize.append(0);
            QQueue<QPair<int, int>> cq;
            cq.enqueue(qMakePair(x, y));
            comp[x][y] = id;
            while (!cq.isEmpty())
            {
                const QPair<int, int> cur = cq.dequeue();
                ++compSize[id];
                for (int i = 0; i < 8; ++i)
                {
                    const int nx = wrapX(cur.first + ndx[i], map_w);
                    const int ny = cur.second + ndy[i];
                    if (ny < 0 || ny >= map_h)
                        continue;
                    if (dist[nx][ny] < 0 || comp[nx][ny] >= 0)
                        continue;
                    comp[nx][ny] = id;
                    cq.enqueue(qMakePair(nx, ny));
                }
            }
        }
    }
    const bool allowLava = S >= 7;
    const int lavaMin = qMax(map_w, qRound(map_w * (2.4 - 1.6 * t)));

    QVector<QVector<quint8>> isCrack(map_w);
    for (int x = 0; x < map_w; ++x)
    {
        isCrack[x].fill(kFaultNone, map_h);
        for (int y = 0; y < map_h; ++y)
        {
            if (dist[x][y] < 0)
                continue;
            const int id = comp[x][y];
            const bool lava = allowLava && id >= 0 && compSize[id] >= lavaMin;
            isCrack[x][y] = lava ? kFaultLava : kFaultDark;
        }
    }

    QVector<QVector<quint8>> waterReach(map_w);
    for (int x = 0; x < map_w; ++x)
        waterReach[x].fill(0, map_h);
    QQueue<QPair<int, int>> wq;
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (matrix[x][y] >= water_level)
                continue;
            waterReach[x][y] = 1;
            wq.enqueue(qMakePair(x, y));
        }
    }
    while (!wq.isEmpty())
    {
        const QPair<int, int> cur = wq.dequeue();
        for (int i = 0; i < 8; ++i)
        {
            const int nx = wrapX(cur.first + ndx[i], map_w);
            const int ny = cur.second + ndy[i];
            if (ny < 0 || ny >= map_h)
                continue;
            if (waterReach[nx][ny])
                continue;
            if (isCrack[nx][ny] != kFaultDark)
                continue;
            waterReach[nx][ny] = 1;
            wq.enqueue(qMakePair(nx, ny));
        }
    }

    const double smallDepth = 6.0 + 10.0 * t;
    const double lavaDepth = 2.5 + 3.5 * t;
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const quint8 kind = isCrack[x][y];
            if (kind == kFaultNone)
                continue;
            const double fall = qMax(0.35, double(strength[x][y]));
            if (kind == kFaultLava)
            {
                faultKind[x][y] = kFaultLava;
                matrix[x][y] = qMax(water_level + 1.5, matrix[x][y] - lavaDepth * fall);
                lavaHeat[x][y] = fall;
            }
            else if (waterReach[x][y])
            {
                faultKind[x][y] = kFaultWater;
                matrix[x][y] = qMin(matrix[x][y], water_level - 2.0 - 4.0 * fall);
            }
            else
            {
                faultKind[x][y] = kFaultDark;
                matrix[x][y] -= smallDepth * fall;
            }
        }
    }
}

void Planet::ApplyLavaClimate()
{
    if (map_w <= 0 || lavaHeat.isEmpty() || t_map.isEmpty())
        return;

    const int radius = qMax(1, map_w / 28);
    const double r = double(radius);
    const double tBoost = 50.0 * (qBound(0, s.seismicity, 12) / 12.0);

    QVector<QVector<double>> dist(map_w);
    QVector<QVector<double>> heat(map_w);
    for (int x = 0; x < map_w; ++x)
    {
        dist[x].fill(1.0e9, map_h);
        heat[x].fill(0.0, map_h);
    }

    std::priority_queue<LavaHeatNode> pq;
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const double fall = lavaHeat[x][y];
            if (fall <= 0.0)
                continue;
            dist[x][y] = 0.0;
            heat[x][y] = fall;
            pq.push({0.0, x, y, fall});
        }
    }

    const int ndx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    const int ndy[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    const double nstep[8] = {1.0, 1.0, 1.0, 1.0, M_SQRT2, M_SQRT2, M_SQRT2, M_SQRT2};

    while (!pq.empty())
    {
        const LavaHeatNode cur = pq.top();
        pq.pop();
        if (cur.d > dist[cur.x][cur.y] + 1.0e-9)
            continue;
        if (cur.d >= r)
            continue;
        for (int i = 0; i < 8; ++i)
        {
            const int nx = wrapX(cur.x + ndx[i], map_w);
            const int ny = cur.y + ndy[i];
            if (ny < 0 || ny >= map_h)
                continue;
            const double nd = cur.d + nstep[i];
            if (nd > r || nd >= dist[nx][ny])
                continue;
            dist[nx][ny] = nd;
            heat[nx][ny] = cur.fall * (1.0 - nd / r);
            pq.push({nd, nx, ny, cur.fall});
        }
    }

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            lavaHeat[x][y] = heat[x][y];
            t_map[x][y] += tBoost * heat[x][y];
        }
    }
}

void Planet::PaintFaults()
{
    if (map_w <= 0 || faultKind.isEmpty() || img.isNull())
        return;
    rnd.seed(static_cast<quint32>(seed) ^ 0x1A7Au);
    const QColor magma(255, 96, 24);
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const quint8 kind = faultKind[x][y];
            if (kind == kFaultNone)
                continue;
            QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
            if (kind == kFaultLava)
                line[x] = DispersionColor(magma, 22).rgb();
            else if (kind == kFaultWater)
            {
                const QColor water = s.shallow_color.isValid() ? s.shallow_color : s.ocean_color;
                line[x] = water.rgb();
            }
            else
                line[x] = LowerColor(QColor::fromRgb(line[x]), 0.62).rgb();
        }
    }
}
