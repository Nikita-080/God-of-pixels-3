#ifndef TERRAFACTORY_H
#define TERRAFACTORY_H

#include <QVector>
#include <QRandomGenerator>

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
    QVector<QVector<double>> makeZero() const;
};

#endif
