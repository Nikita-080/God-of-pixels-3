#ifndef TERRAFACTORY_H
#define TERRAFACTORY_H

#include <QVector>
#include <QRandomGenerator>
#include "spheremath.h"

class TerraFactory
{
public:
    TerraFactory(int width, int height, int seed);
    QVector<QVector<double>> sphericalNoise(double amplitude);
    QVector<QVector<double>> sphericalFault(int iterations);

private:
    int w;
    int h;
    int seed;
    QRandomGenerator rnd;
    QVector<SphereVec3> xyz;
    QVector<QVector<double>> makeZero() const;
};

#endif
