#include "terrafactory.h"

TerraFactory::TerraFactory(int Size,QRandomGenerator RND)
{
    rnd=RND;
    size=Size;
    FillZero();
}
void TerraFactory::FillZero()
{
    matrix.fill(QVector<double>().fill(0,size),size);
}
QVector<QVector<double>> TerraFactory::diamondsquare(double randfactor)
{
    FillZero();
    int step=size-1;
    int halfstep=step/2;
    double noise=randfactor*size;
    double height;
    while (step!=1)
    {
        //square
        for (int i=halfstep;i<size;i+=step)
        {
            for (int j=halfstep;j<size;j+=step)
            {
                height=(matrix[i-halfstep][j-halfstep]+
                        matrix[i-halfstep][j+halfstep]+
                        matrix[i+halfstep][j-halfstep]+
                        matrix[i+halfstep][j+halfstep])/4;
                height+=rnd.generateDouble()*noise*2-noise;
                matrix[i][j]=height;
            }
        }
        //diamond
        for (int i=0;i<size;i+=halfstep)
        {
            for (int j=(i+halfstep)%step;j<size;j+=step)
            {
                height=0;
                if (i>0) height+=matrix[i-halfstep][j];
                if (i<size-1) height+=matrix[i+halfstep][j];
                if (j>0) height+=matrix[i][j-halfstep];
                if (j<size-1) height+=matrix[i][j+halfstep];
                height/=4;
                height+=rnd.generateDouble()*noise*2-noise;
                matrix[i][j]=height;
            }
        }
        step/=2;
        halfstep=step/2;
        noise/=2;
    }
    return matrix;
}
QVector<QVector<double>> TerraFactory::foultformation(int iter)
{
    FillZero();
    int x1,x2,y1,y2,a,b,c;
    for (int i=iter;i>0;i--)
    {
        x1=rnd.bounded(0,size);
        x2=rnd.bounded(0,size);
        y1=rnd.bounded(0,size);
        y2=rnd.bounded(0,size);
        a=y1-y2;
        b=x2-x1;
        c=-x2*y1+x1*y2;
        for (int j=0;j<size;j++)
        {
            for (int k=0;k<size;k++)
            {
                // -+ iter maybe
                if (a*j+b*k+c>0) matrix[k][j]+=1;
                else matrix[k][j]-=1;
            }
        }
    }
    return matrix;
}
