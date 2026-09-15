#ifndef SPHEREMATH_H
#define SPHEREMATH_H

#include <QtMath>
#include <cmath>

struct SphereVec3
{
    double x;
    double y;
    double z;
};

inline SphereVec3 equirectToSphere(int x, int y, int mapW, int mapH)
{
    const double lon = 2.0 * M_PI * (x + 0.5) / mapW;
    const double lat = M_PI * (0.5 - (y + 0.5) / mapH);
    const double cl = cos(lat);
    SphereVec3 p;
    p.x = cl * cos(lon);
    p.y = sin(lat);
    p.z = cl * sin(lon);
    return p;
}

inline double sphereDot(const SphereVec3 &a, const SphereVec3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline double sphereLen(const SphereVec3 &a)
{
    return sqrt(sphereDot(a, a));
}

inline SphereVec3 sphereNorm(const SphereVec3 &a)
{
    const double l = sphereLen(a);
    if (l <= 1e-12)
        return {0.0, 1.0, 0.0};
    return {a.x / l, a.y / l, a.z / l};
}

inline double sphereAtanh(double x)
{
    x = qBound(-0.999999, x, 0.999999);
    return 0.5 * log((1.0 + x) / (1.0 - x));
}

inline bool equirectToMercatorPixel(int x, int y, int mapW, int mapH,
                                    int outW, int outH, int *mx, int *my)
{
    if (mapW <= 0 || mapH <= 0 || outW <= 0 || outH <= 0 || !mx || !my)
        return false;
    const double lon = 2.0 * M_PI * (x + 0.5) / mapW;
    const double lat = M_PI * (0.5 - (y + 0.5) / mapH);
    const double maxLat = 85.0 * M_PI / 180.0;
    if (qAbs(lat) > maxLat)
        return false;
    const double maxY = sphereAtanh(sin(maxLat));
    const double mer = sphereAtanh(sin(lat));
    *mx = int(qBound(0.0, (lon / (2.0 * M_PI)) * outW, double(outW - 1)));
    *my = int(qBound(0.0, (maxY - mer) / (2.0 * maxY) * outH, double(outH - 1)));
    return true;
}

inline SphereVec3 latLonDegToSphere(int latDeg, int lonDeg)
{
    const double lat = qDegreesToRadians(double(qBound(-90, latDeg, 90)));
    const double lon = qDegreesToRadians(double(lonDeg));
    const double cl = cos(lat);
    SphereVec3 p;
    p.x = cl * cos(lon);
    p.y = sin(lat);
    p.z = cl * sin(lon);
    return sphereNorm(p);
}

#endif
