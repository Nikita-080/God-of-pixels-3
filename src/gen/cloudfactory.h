#ifndef CLOUDFACTORY_H
#define CLOUDFACTORY_H

#include "noise3d.h"

class CloudFactory
{
public:
    CloudFactory(int octaves, bool unbias, int seed);
    double GetNum(double x, double y, double z);

private:
    int octaves;
    bool unbias;
    Noise3D noise;
    static double smoothstep(double t);
};

#endif
