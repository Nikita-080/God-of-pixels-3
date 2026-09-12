#include "cloudfactory.h"
#include <QtMath>

double CloudFactory::smoothstep(double t)
{
    return t * t * (3.0 - 2.0 * t);
}

CloudFactory::CloudFactory(int Octaves, bool Unbias, int seed)
    : octaves(qMax(1, Octaves))
    , unbias(Unbias)
    , noise(seed)
{
}

double CloudFactory::GetNum(double x, double y, double z)
{
    double ret = noise.fbm(x, y, z, octaves);
    if (unbias)
    {
        double r = (ret + 1.0) / 2.0;
        const int times = qFloor(1.0 * octaves / 2.0 + 0.5);
        for (int i = 0; i < times; ++i)
            r = smoothstep(r);
        ret = r * 2.0 - 1.0;
    }
    return ret;
}
