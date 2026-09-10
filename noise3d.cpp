#include "noise3d.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QtMath>

Noise3D::Noise3D(int seed)
{
    int p[256];
    for (int i = 0; i < 256; ++i)
        p[i] = i;
    QRandomGenerator rnd(static_cast<quint32>(seed));
    for (int i = 255; i > 0; --i)
    {
        const int j = rnd.bounded(i + 1);
        qSwap(p[i], p[j]);
    }
    for (int i = 0; i < 256; ++i)
    {
        perm[i] = p[i];
        perm[i + 256] = p[i];
    }
}

double Noise3D::fade(double t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double Noise3D::lerp(double t, double a, double b)
{
    return a + t * (b - a);
}

double Noise3D::grad(int hash, double x, double y, double z)
{
    const int h = hash & 15;
    const double u = h < 8 ? x : y;
    const double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double Noise3D::noise(double x, double y, double z) const
{
    int X = qFloor(x) & 255;
    int Y = qFloor(y) & 255;
    int Z = qFloor(z) & 255;
    x -= qFloor(x);
    y -= qFloor(y);
    z -= qFloor(z);
    const double u = fade(x);
    const double v = fade(y);
    const double w = fade(z);
    const int A = perm[X] + Y;
    const int AA = perm[A] + Z;
    const int AB = perm[A + 1] + Z;
    const int B = perm[X + 1] + Y;
    const int BA = perm[B] + Z;
    const int BB = perm[B + 1] + Z;
    return lerp(w,
                lerp(v,
                     lerp(u, grad(perm[AA], x, y, z), grad(perm[BA], x - 1, y, z)),
                     lerp(u, grad(perm[AB], x, y - 1, z), grad(perm[BB], x - 1, y - 1, z))),
                lerp(v,
                     lerp(u, grad(perm[AA + 1], x, y, z - 1), grad(perm[BA + 1], x - 1, y, z - 1)),
                     lerp(u, grad(perm[AB + 1], x, y - 1, z - 1), grad(perm[BB + 1], x - 1, y - 1, z - 1))));
}

double Noise3D::fbm(double x, double y, double z, int octaves) const
{
    octaves = qMax(1, octaves);
    double sum = 0.0;
    double amp = 1.0;
    double freq = 1.0;
    double norm = 0.0;
    for (int i = 0; i < octaves; ++i)
    {
        sum += amp * noise(x * freq, y * freq, z * freq);
        norm += amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum / norm;
}
