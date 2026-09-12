#ifndef NOISE3D_H
#define NOISE3D_H

class Noise3D
{
public:
    explicit Noise3D(int seed);
    double noise(double x, double y, double z) const;
    double fbm(double x, double y, double z, int octaves) const;

private:
    int perm[512];
    static double fade(double t);
    static double lerp(double t, double a, double b);
    static double grad(int hash, double x, double y, double z);
};

#endif
