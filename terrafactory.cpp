#include "terrafactory.h"
#include "noise3d.h"
#include "spheremath.h"
#include <QtMath>

TerraFactory::TerraFactory(int width, int height, int seed)
    : w(width)
    , h(height)
    , seed(seed)
{
    rnd.seed(static_cast<quint32>(seed));
}

QVector<QVector<double>> TerraFactory::makeZero() const
{
    QVector<QVector<double>> m;
    m.resize(w);
    for (int x = 0; x < w; ++x)
        m[x].fill(0.0, h);
    return m;
}

QVector<QVector<double>> TerraFactory::sphericalNoise(double amplitude)
{
    QVector<QVector<double>> m = makeZero();
    Noise3D n(seed);
    const double freq = 1.5 + amplitude * 0.35;
    const int octaves = 4 + qBound(0, qRound(amplitude / 3.0), 3);
    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            const SphereVec3 p = equirectToSphere(x, y, w, h);
            m[x][y] = n.fbm(p.x * freq, p.y * freq, p.z * freq, octaves);
        }
    }
    return m;
}

QVector<QVector<double>> TerraFactory::sphericalFault(int iterations)
{
    QVector<QVector<double>> m = makeZero();
    const int iter = qMax(1, iterations);
    for (int i = 0; i < iter; ++i)
    {
        const double ax = rnd.generateDouble() * 2.0 - 1.0;
        const double ay = rnd.generateDouble() * 2.0 - 1.0;
        const double az = rnd.generateDouble() * 2.0 - 1.0;
        SphereVec3 n = sphereNorm({ax, ay, az});
        for (int x = 0; x < w; ++x)
        {
            for (int y = 0; y < h; ++y)
            {
                const SphereVec3 p = equirectToSphere(x, y, w, h);
                m[x][y] += (sphereDot(n, p) > 0.0) ? 1.0 : -1.0;
            }
        }
    }
    return m;
}
