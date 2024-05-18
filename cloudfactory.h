#ifndef CLOUDFACTORY_H
#define CLOUDFACTORY_H
#include <QVector>
#include <QRandomGenerator>
#include <QMap>
class CloudFactory
{
public:
    int dimension;
    int octaves;
    bool unbias;
    double scale_factor;
    QMap <QVector<double>,QVector<double>> gradient;
    QRandomGenerator rnd;
    double gauss_next;
public:
    double smoothstep(double t);
    double lerp(double t, double a, double b);
    CloudFactory(int octaves, bool unbias, int Seed);
    QVector <double> _generate_gradient();
    double RandomGauss();
    double get_plain_noise(double x,double y);
    double GetNum(double x,double y);
};

#endif // CLOUDFACTORY_H
