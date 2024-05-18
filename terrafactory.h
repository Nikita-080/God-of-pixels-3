#ifndef TERRAFACTORY_H
#define TERRAFACTORY_H

#include <QVector>
#include <QRandomGenerator>

class TerraFactory
{
public:
    TerraFactory(int Size,int Seed);
    QVector<QVector<double>> foultformation(int iter);
    QVector<QVector<double>> diamondsquare(double randfactor);
private:
    void FillZero();
private:
    int size;
    QRandomGenerator rnd;
    QVector<QVector<double>> matrix;
};

#endif // TERRAFACTORY_H
