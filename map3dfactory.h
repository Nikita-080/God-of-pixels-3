#ifndef MAP3DFACTORY_H
#define MAP3DFACTORY_H
#include <QVector>

class Map3DFactory //from internet (from python)
{
public:
    int size;
    int max;
    double randomness;
    double high;
    QVector <double> grid;
public:
    void set(int x, int y, double val);
    void get(int x, int y);
public:
    Map3DFactory();
};

#endif // MAP3DFACTORY_H
