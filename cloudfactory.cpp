#include "cloudfactory.h"
#include<math.h>
#include <QtMath>
double CloudFactory::smoothstep(double t)
{
    return t * t * (3.0 - 2.0 * t);
}
double CloudFactory::lerp(double t, double a, double b)
{
    return a + t * (b - a);
}
CloudFactory::CloudFactory(int Octaves, bool Unbias,QRandomGenerator Rnd)
{
    octaves = Octaves;
    unbias = Unbias;
    scale_factor = 1.41421;
    rnd=Rnd;
    gauss_next=NULL;
}
double CloudFactory::RandomGauss()
{
    //в оригинале python: random.gauss(0,1)
    double z = gauss_next;
    gauss_next = NULL;
    if (z==NULL)
    {
        double x2pi = rnd.generateDouble() * 6.283;
        double g2rad = sqrt(-2.0 * qLn(1.0 - rnd.generateDouble()));
        z = cos(x2pi) * g2rad;
        gauss_next = sin(x2pi) * g2rad;
    }
    return z;
}
QVector <double> CloudFactory::_generate_gradient()
{
    QVector <double> rp; //random_point
    rp.append(RandomGauss());
    rp.append(RandomGauss());
    double scale=pow(rp[0]*rp[0]+rp[1]*rp[1],-0.5);
    rp[0]*=scale;
    rp[1]*=scale;
    return rp;
}
double CloudFactory::get_plain_noise(double x,double y)
{
    double xmin=qFloor(x);
    double xmax=xmin+1;
    double ymin=qFloor(y);
    double ymax=ymin+1;
    QVector <QVector<double>> grid_co_orig={{xmin,xmax},{ymin,ymax}};
    QVector <QVector<double>> grid_coords={{xmin,ymin},
                                           {xmin,ymax},
                                           {xmax,ymin},
                                           {xmax,ymax}};
    QVector <double> dots;
    for (int i=0;i<4;i++)
    {
        if (!gradient.contains(grid_coords[i]))
        {
            gradient[grid_coords[i]] = _generate_gradient();
        }
        QVector <double> grad = gradient[grid_coords[i]];
        double dot = 0;
        dot+=grad[0]*(x-grid_coords[i][0]);
        dot+=grad[1]*(y-grid_coords[i][1]);
        dots.append(dot);
    }
    QVector <double> point={x,y};
    int dim = 2;
    while (dots.length() > 1)
    {
        dim--;
        double s = smoothstep(point[dim] - grid_co_orig[dim][0]);
        QVector <double> next_dots;
        while (dots.length()!=0)
        {
            double el1=dots[0];
            dots.pop_front();
            double el2=dots[0];
            dots.pop_front();
            next_dots.append(lerp(s, el1, el2));
        }
        dots = next_dots;
    }
    return dots[0] * scale_factor;
}
double CloudFactory::GetNum(double x,double y)
{
    QVector <double> point={x,y};
    double ret = 0;
    for (int o=0;o<octaves;o++)
    {
        int o2 = 1 << o;
        ret += get_plain_noise(point[0]*o2,point[1]*o2) / o2;
    }
    ret /= 2 - pow(2,1 - octaves);
    if (unbias)
    {
        double r = (ret + 1) / 2;
        for (int i=0;i<floor(1.0*octaves/2+0.5);i++)
        {
            r=smoothstep(r);
        }
        ret = r * 2 - 1;
    }
    return ret;
}
