#include "planet.h"
#include <QVector>
#include <QRandomGenerator>
#include "cloudfactory.h"
#include <QFile>
#include "terrafactory.h"
#include <QTextStream>
#include <QtMath>
#include <QPainter>
#include <QFontDatabase>
Planet::Planet(QRandomGenerator Rnd,QString Currentpath)
{
    currentpath=Currentpath;
    rnd=Rnd;
    color_black=QColor(0,0,0);
}
Planet::Planet()
{

}
int Planet::RAND(int a, int b){
    return rnd.generate()%((b+1)-a)+a;
}

void Planet::CreateMatrixNew()
{
    world_size=pow(2,s.world_size)+1;
    TerraFactory factory(world_size,rnd);
    if (s.terramode==0)
    {
        matrix=factory.diamondsquare(s.randomness*1.0/10);
        ImageReport("ds",matrix,color_black,QColor(255,255,255));
    }
    else if (s.terramode==1) matrix=factory.foultformation(s.iterations);
}
void Planet::Name()
{
    if (s.name_algorithm==1) name=Name_gop2();
    else if (s.name_algorithm==2) name=Name_readable();
    else name=Name_random();
}
QString Planet::Name_random()
{
    QString alfa="qwertyuiopasdfghjklzxcvbnm1234567890";
    QString s="";
    int len=RAND(3,10);
    for (int i=0;i<len;i++) s+=alfa[RAND(0,35)];
    return s;
}
QString Planet::Name_gop2()
{
    QString sogl="rtpsdfghkljzxcvbnmy";
    QString glas="euioa";
    QString alfa=sogl+glas;
    QString s="";
    int len =RAND(3,8);
    s+=alfa[RAND(0,23)];
    s+=alfa[RAND(0,23)];
    for (int i=0;i<len-2;i++)
    {
        if (sogl.contains(s[i+1]) and sogl.contains(s[i])) s+=glas[RAND(0,4)];
        else if (glas.contains(s[i+1]) and glas.contains(s[i])) s+=sogl[RAND(0,18)];
        else s+=alfa[RAND(0,23)];
    }
    int num=RAND(1,1000);
    return s+"-"+QString::number(num);
}
QString Planet::Name_readable()
{
    QFile file(currentpath+"NameMatrix.txt");
    file.open(QIODevice::ReadOnly);
    QTextStream fileStream(&file);
    QString str;
    QVector<QVector<int>> word_table;
    for (str = fileStream.readLine(); !str.isNull(); str = fileStream.readLine()) {
        QVector <int> curr;
        QStringList data=str.split(" ");
        for (int i=0;i<data.length();i++)
        {
             int number=data[i].toInt();
             curr.append(number);
        }
        word_table.append(curr);
    }
    QString alfa="abcdefghijklmnopqrstuvwxyz";
    QString sogl="rtpsdfghkljzxcvbnmy";
    QString glas="euioa";
    QString s="";
    s+=alfa[RAND(0,25)];
    s+=char2char(s[0],word_table);
    int len=RAND(1,6);
    for (int i=0;i<len;i++)
    {
        if ((sogl.contains(s[i+1]) and sogl.contains(s[i])) or s[i+1]=='x')
        {
            QChar c=sogl[0];
            while (sogl.contains(c)) c=char2char(s[i+1],word_table);
            s+=c;
        }
        else if (glas.contains(s[i+1]) and glas.contains(s[i]))
        {
            QChar c=glas[0];
            while (glas.contains(c)) c=char2char(s[i+1],word_table);
            s+=c;
        }
        else s+=char2char(s[i+1],word_table);
    }
    return s[0].toUpper()+s.mid(1,len+1);
}
QChar Planet::char2char(QChar a, QVector<QVector<int>> word_table)
{
    QString alfa="abcdefghijklmnopqrstuvwxyz";
    QVector <int> work_arr=word_table[alfa.indexOf(a)];
    int work_arr_sum=0;
    for (int i=0;i<26;i++) work_arr_sum+=work_arr[i];
    int v=RAND(1,work_arr_sum);
    int ptr=0;
    int summa=0;
    while (summa<v)
    {
        summa+=work_arr[ptr];
        ptr++;
    }
    ptr-=1;
    return alfa[ptr];
}
void Planet::FixMatrix()
{
    for (int i=0; i<world_size;i++)
    {
        for (int k=0;k<world_size;k++)
        {
            matrix[i][k]=(matrix[i][k]-world_deep)*280/(world_heighth-world_deep);
        }
    }
}
void Planet::LevelCreating()
{
    QVector <QColor> colors={s.ice_color,s.rock_color,s.mountain_color,s.plain_color,s.beach_color,s.shallow_color,s.ocean_color};
    level_up.clear();
    level_down.clear();
    level_color.clear();
    for (int i=0;i<7;i++)
    {
        if (s.true_structure[i]!=s.true_structure[i+1])
        {
            level_up.append(s.true_structure[i]);
            level_down.append(s.true_structure[i+1]);
            level_color.append(colors[i]);
        }
    }
}
void Planet::TMapCreating()
{
    t_map.clear();
    for (int i=0;i<world_size;i++)
    {
        QVector <double> dataline;
        for (int k=0;k<world_size;k++)
        {
            if (!isBlack(img.pixelColor(i,k)))
            {
                double e_len=2*3.1415*R_planet/4;//четверть длины экватора
                double tmp_len=ArcDistance(i,k,"polar"); //предполагаемое расстояние до полюса
                double r_e; //расстояние до полюса
                if (tmp_len>e_len) r_e=(2*e_len-tmp_len)/e_len;
                else r_e=tmp_len/e_len;
                double r_w=matrix[i][k]-water_level; //относительная высота
                r_w*=31.6; //преобразование в метры 70.86
                double T=56*r_e-28; //среднегодовая температура у поверхности
                T-=0.6*(r_w)/100; //падение температуры с высотой
                T+=s.temperature-15; //настройки пользователя преобразованная из средней в корректирующую
                dataline.append(T);
            }
            else dataline.append(0);
        }
        t_map.append(dataline);
    }
}
void Planet::RMapCreating()
{
    r_map.clear();
    for (int i=0;i<world_size;i++)
    {
        QVector <double> dataline;
        for (int k=0;k<world_size;k++)
        {
            if (!isBlack(img.pixelColor(i,k)))
            {
                double r_w=matrix[i][k]-water_level; //относительная высота
                double W=1.37*r_w+0.32*(t_map[i][k])+51.53; //влажность
                dataline.append(W);
            }
            else dataline.append(0);
        }
        r_map.append(dataline);
    }
}
void Planet::Plant()
{
    green_square=0;
    QImage diagram=QImage(currentpath+"plantmatrix.png");
    for (int i=0;i<world_size;i++)
    {
        for (int k=0;k<world_size;k++)
        {
            if (!isBlack(img.pixelColor(i,k)) && matrix[i][k]>water_level)
            {
                double T=t_map[i][k];
                double W=r_map[i][k];
                if (W>=0 && W<=450 && T>=-15 && T<=35)
                {
                    int x=qRound(-1.2*T+42);
                    int y=qRound(-0.13*W+60);
                    if (x<0) x=0;
                    if (x>59) x=59;
                    if (y<0) y=0;
                    if (y>59) y=59;
                    QColor color=diagram.pixelColor(x,y);
                    if (color.red()!=252 || color.green()!=253 || color.blue()!=163)
                    { //игнорирование цвета пустыни
                        img.setPixelColor(i,k,color);
                        green_square++;
                    }
                }
            }
        }
    }
}
void Planet::Polar()
{
    QColor color=QColor(255,255,255);
    QVector <QColor> polar_color;
    polar_color.append(color);
    if (level_color.length()>1)
    {
        for (int i=1;i<level_color.length();i++)
        {
            if (level_up[i]<water_level) polar_color.append(polar_color[i-1]);
            else polar_color.append(LowerColor(polar_color[i-1],0.9));
        }
    }
    for (int i=0;i<world_size;i++)
    {
        for (int j=0;j<world_size;j++)
        {
            if (!isBlack(img.pixelColor(i,j)))
            {
                double T=t_map[i][j];
                if (T<-15)
                {
                    for (int q=0;q<level_color.length();q++)
                    {
                        if (level_up[q]>=matrix[i][j] and matrix[i][j]>=level_down[q])
                        {
                            img.setPixelColor(i,j,polar_color[q]);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void Planet::CloudMapCreating()
{
    c_map.clear();
    int res=qRound(1.0*s.cloud_size*world_size/20);
    CloudFactory pnf(s.cloud_quality,s.correction,rnd);
    for (int x=0;x<world_size;x++)
    {
        QVector <double> cloud_line;
        for (int y=0;y<world_size;y++)
        {
            double n = pnf.GetNum(1.0*x/res, 1.0*y/res);
            cloud_line.append(n*0.5+0.5);//qFloor((n + 1) / 2 * 255 + 0.5));
        }
        c_map.append(cloud_line);
    }

}
void Planet::UMapCreating()
{
    u_map.clear();
    for (int i=0;i<world_size;i++)
    {
        QVector<QVector<int>> data_line;
        for (int k=0;k<world_size;k++)
        {
            double xn;
            double yn;
            if (i==k and k==world_size/2) //?
            {
                xn=i;
                yn=k;
            }
            else
            {
                xn=i-world_size/2;
                yn=world_size/2-k;
                double r=sqrt(xn*xn+yn*yn);
                double l=sin(r/R_planet)*R_planet;
                xn=xn*l/r;
                yn=yn*l/r;
                xn=xn+world_size/2;
                yn=world_size/2-yn;
            }
            QVector <int> result={qRound(xn),qRound(yn)};
            data_line.append(result);
        }
        u_map.append(data_line);
    }
}
double Planet::ArcDistance(int x, int y, QString mode)
{
    double xc,yc,zc;
    if (mode=="polar")
    {
        xc=x_polar;
        yc=y_polar;
        zc=z_polar;
    }
    else if (mode=="shine")
    {
        xc=x_shine;
        yc=y_shine;
        zc=z_shine;
    }
    else
    {
        return 0;
    }
    double xn=u_map[x][y][0];
    double yn=u_map[x][y][1];
    xn-=world_size/2;
    yn=world_size/2-yn;
    double zn=sqrt(R_planet*R_planet-xn*xn-yn*yn);
    double angle_cos=(xn*xc+yn*yc+zn*zc)/sqrt(xn*xn+yn*yn+zn*zn)/sqrt(xc*xc+yc*yc+zc*zc);
    return acos(angle_cos)*R_planet;
}
void Planet::Atmosphere(QString mode)
{
    double l_min=0;//R_atmo-R_planet;
    double l_max=2*sqrt(R_planet*(R_atmo-R_planet));
    double q=0.1;//коэффициент при l_max и transparent=10
    double kmin=-0.1*s.atmo_transparent+1; //k при l_min
    double kmax=(q-1)/10*s.atmo_transparent+1; //k при l_max
    double a=(kmax-kmin)/(l_max-l_min); //k=a*l+b
    double b=kmin-(l_min*a);
    for (int i=0;i<world_size;i++)
    {
        for (int j=0;j<world_size;j++)
        {
            bool isCell;
            if (mode=="in")
            {
                isCell=!isBlack(img.pixelColor(i,j));
            }
            else
            {
                isCell=isBlack(img.pixelColor(i,j)) && (i-world_size/2)*(i-world_size/2)+(j-world_size/2)*(j-world_size/2)<=R_atmo*R_atmo;
            }
            if (isCell)
            {
                int x=i-world_size/2; //координаты относительно центра картинки
                int y=world_size/2-j;
                double r=sqrt(x*x+y*y);
                double l;//приведенная толщина атмосферы
                if (R_planet<=r and R_atmo>=r) l=2*sqrt(R_atmo*r-r*r);
                else l=sqrt(r*R_atmo-r*r)-sqrt(r*R_planet-r*r);
                double k=a*l+b;
                if (mode=="out")
                {
                    double shadowK=Shadow_step(i,j,R_atmo);
                    if (shadowK<0) shadowK=0;
                    QColor setcolor;
                    setcolor=TransparentColor(img.pixelColor(i,j),s.atmo_color,k);
                    setcolor=LowerColor(setcolor,shadowK);
                    img.setPixelColor(i,j,setcolor);
                }
                else
                {
                    QColor setcolor;
                    setcolor=TransparentColor(img.pixelColor(i,j),s.atmo_color,k);
                    img.setPixelColor(i,j,setcolor);
                }
            }
        }
    }
}
void Planet::UV()
{
    QImage img2=img;
    img.fill(color_black);
    for (int i=0;i<world_size;i++)
    {
        for (int k=0;k<world_size;k++)
        {
            if (!isBlack(img2.pixelColor(i,k)))
            {
                QVector <int> v=u_map[i][k];
                img.setPixelColor(v[0],v[1],img2.pixelColor(i,k));
            }
        }
    }
}
void Planet::Shadow()
{
    for (int i=0;i<world_size;i++)
    {
        for (int j=0;j<world_size;j++)
        {
            if (!isBlack(img.pixelColor(i,j)))
            {
                double k=Shadow_step(i,j,R_planet);
                if (k<0) img.setPixelColor(i,j,color_black);
                else
                {
                    k*=s.shine*0.1+0.5;
                    //k=pow(k,-0.2*s.shine+2);
                    img.setPixelColor(i,j,LowerColor(img.pixelColor(i,j),k));
                }
            }
        }
    }
}
double Planet::Shadow_step(int a, int b, int R)
{
    int x=a-world_size/2;
    int y=world_size/2-b;
    if (R*R-x*x-y*y<0) return 0;
    double z=sqrt(R*R-x*x-y*y);
    double angle_cos=(x*x_shine+y*y_shine+z*z_shine)/sqrt(x*x+y*y+z*z)/sqrt(x_shine*x_shine+y_shine*y_shine+z_shine*z_shine);
    return angle_cos;
}
void Planet::Cloud()
{
    double k1=-0.09*s.cloud_transparent+1;
    for (int i=0;i<world_size;i++)
    {
        for (int j=0;j<world_size;j++)
        {
            if (!isBlack(img.pixelColor(i,j)))
            {
                double koef=c_map[i][j]*k1;
                img.setPixelColor(i,j,TransparentColor(img.pixelColor(i,j),s.cloud_color,koef));
            }
        }
    }
}
bool Planet::isBlack(QColor color)
{
    return color.red()==0 and color.blue()==0 and color.green()==0;
}
void Planet::Calculator()
{
    green_square=0;
    if (s.temperature==-90) starclass=RAND(0,12);
    else starclass=RAND(0,11);
    R_planet=world_size/2*2*1.0/3.1415;
    if (s.is_atmo) R_atmo=R_planet*(81+s.atmo_size)/81;
    else R_atmo=R_planet;
    if (R_atmo>R_planet) R_final=R_atmo;
    else R_final=R_planet;
    x_shine=s.point_of_shine_true[0]*R_planet;
    y_shine=s.point_of_shine_true[1]*R_planet;
    z_shine=sqrt(R_planet*R_planet-x_shine*x_shine-y_shine*y_shine);
    x_polar=s.point_of_polar_true[0]*R_planet;
    y_polar=s.point_of_polar_true[1]*R_planet;
    z_polar=sqrt(R_planet*R_planet-x_polar*x_polar-y_polar*y_polar);
    A=x_polar; //плоскость колец
    B=y_polar;
    C=z_polar;
    water_level=s.true_structure[5];

    world_deep=matrix[0][0];
    world_heighth=matrix[0][0];
    for (int i=0;i<world_size;i++) for (int k=0;k<world_size;k++)
    {
        world_deep=fmin(world_deep,matrix[i][k]);
        world_heighth=fmax(world_heighth,matrix[i][k]);
    }
}
/*
void Planet::ImageCreating()
{
    img=QImage(world_size,world_size,QImage::Format_RGB32);
    img.fill(color_black);
    for (int i=0;i<world_size;i++)
    {
        for (int k=0;k<world_size;k++)
        {
            double Rad1=(i-world_size/2-1)*(i-world_size/2-1)+(k-world_size/2-1)*(k-world_size/2-1);
            double Rad2=(world_size/2)*(world_size/2);
            if (Rad1<=Rad2)
            {
                for (int j=0;j<level_up.length();j++)
                {
                    if (matrix[i][k]<=level_up[j] and matrix[i][k]>=level_down[j])
                    {
                        img.setPixelColor(i,k,level_color[j]);
                        break;
                    }
                }
            }
        }
    }
}
*/
void Planet::ImageCreating() //gradient version
{
    img=QImage(world_size,world_size,QImage::Format_RGB32);
    img.fill(color_black);
    QVector<double> level_aver;
    for (int i=0;i<level_up.length();i++)
    {
        level_aver.append((level_up[i]+level_down[i])/2);
    }
    level_aver.append(0);
    for (int i=0;i<world_size;i++)
    {
        for (int k=0;k<world_size;k++)
        {
            double Rad1=(i-world_size/2-1)*(i-world_size/2-1)+(k-world_size/2-1)*(k-world_size/2-1);
            double Rad2=(world_size/2)*(world_size/2);
            if (Rad1<=Rad2)
            {
                int index=0;
                while (matrix[i][k]<level_aver[index]) index++;
                if (index==0) img.setPixelColor(i,k,level_color[0]);
                else if (index==level_aver.length()-1) img.setPixelColor(i,k,level_color.last());
                else
                {
                    img.setPixelColor(i,k,TransparentColor(level_color[index],level_color[index-1],(matrix[i][k]-level_aver[index])/(level_aver[index-1]-level_aver[index])));
                }


            }
        }
    }
}
QColor Planet::TransparentColor(QColor color1, QColor color2, double koef)
{
    int r=qRound((1-koef)*color1.red()+koef*color2.red());
    int g=qRound((1-koef)*color1.green()+koef*color2.green());
    int b=qRound((1-koef)*color1.blue()+koef*color2.blue());

    return QColor(r,g,b);
}
QColor Planet::DispersionColor(QColor color, int disp)
{
    QVector <int> new_c={0,0,0};
    new_c[0]=color.red()+RAND(-disp,+disp);
    new_c[1]=color.green()+RAND(-disp,+disp);
    new_c[2]=color.blue()+RAND(-disp,+disp);

    new_c[0]= (new_c[0]<0) ? 0 : new_c[0];
    new_c[1]= (new_c[1]<0) ? 0 : new_c[1];
    new_c[2]= (new_c[2]<0) ? 0 : new_c[2];

    new_c[0]= (new_c[0]>255) ? 255 : new_c[0];
    new_c[1]= (new_c[1]>255) ? 255 : new_c[1];
    new_c[2]= (new_c[2]>255) ? 255 : new_c[2];

    QColor result=QColor(new_c[0],new_c[1],new_c[2]);
    return result;
}
QColor Planet::LowerColor(QColor color, double koef)
{
    int a=qRound(color.red()*koef);
    int b=qRound(color.green()*koef);
    int c=qRound(color.blue()*koef);
    QColor result=QColor(a,b,c);
    return result;
}
void Planet::Ring()
{
    double rmin=R_final+(world_size/2-R_final)/10*s.R_internal_ring;
    double rmax=(world_size/2+R_final)/2+(world_size/2-R_final)/10*s.R_external_ring;
    int color_num=RAND(1,6);
    QVector <QColor> color_mas;
    for (int i=0;i<color_num;i++) color_mas.append(DispersionColor(s.ring_color,20));
    QVector <QColor> color_mas_dark;
    for (int i=0;i<color_num;i++) color_mas_dark.append(LowerColor(color_mas[i],0.5));
    for (int i=0;i<world_size;i++)
    {
        for (int j=0;j<world_size;j++)
        {
            double a=i-world_size/2; //координаты относительно центра картинки
            double b=world_size/2-j;
            double c=(-A*a-B*b)/C; //точка на плоскости кольца
            double r=sqrt(a*a+b*b+c*c); //расстояние до центра
            QColor final_color;
            if (rmin<=r and r<=rmax and ((c<0 and a*a+b*b>R_planet*R_planet) or c>=0))
            {//если принадлежит кольцу и не за планетой
                if (sqrt((a*y_shine-b*x_shine)*(a*y_shine-b*x_shine)+(b*z_shine-c*y_shine)*(b*z_shine-c*y_shine)+(a*z_shine-c*x_shine)*(a*z_shine-c*x_shine))/R_planet<=R_planet and (a*x_shine+b*y_shine+c*z_shine)/r/R_planet<=0)
                {
                    //если в тени планеты
                    int index=qFloor((r-rmin)/((rmax-rmin)/color_num));
                    if (index>=color_num) index=color_num-1;
                    final_color=color_mas_dark[index];
                }
                else
                {
                    //иначе
                    int index=qFloor((r-rmin)/((rmax-rmin)/color_num));
                    if (index>=color_num) index=color_num-1;
                    final_color=color_mas[index];
                }
                if (R_planet!=R_final and R_planet*R_planet<a*a+b*b and a*a+b*b<R_final*R_final and c<0)
                {
                    double koef=1.0*img.pixelColor(i,j).red()/s.atmo_color.red();
                    final_color=TransparentColor(final_color,s.atmo_color,koef);
                }
                img.setPixelColor(i,j,final_color);
            }
        }
    }
}
void Planet::Description()
{
    img_dsc=QImage(currentpath+"window.png");
    QString classes="OBAFGKMCSLTY";
    QString s="";

    QPainter p;
    p.begin(&img_dsc);
    p.setPen(QPen(QColor(110,170,200)));
    p.setFont(QFont("Consolas",8));

    s+="имя        - "+name+"\n";

    if (starclass==12) s+="день       - [не найдено]\n";
    else s+="день       - "+QString::number(RAND(5,100))+"ч\n";

    if (starclass==12) s+="год        - [не найдено]\n";
    else s+="год        - "+QString::number(RAND(1,50))+"г\n";

    s+="гравитация - "+QString::number(RAND(0,2))+"."+QString::number(RAND(0,9))+"g\n";

    if (starclass==12) s+="звезда     - [не найдено]\n";
    else s+="звезда     - "+classes[starclass]+"\n";

    s+="ресурсы    - "+Resources();
    s+="\n";
    QString level1;
    QString level2;
    int x;
    int y=qFloor(green_square*10.0/(3.1415*(world_size*1.0/2)*(world_size*1.0/2)));
    level1.fill('#',y);
    level2.fill(' ',10-y);
    s+="жизнь:\n";
    s+="   |"+level1+level2+"|\n";
    x=RAND(0,10);
    level1.fill('#',x);
    level2.fill(' ',10-x);
    s+="сейсмичность:\n";
    s+="   |"+level1+level2+"|\n";
    x=RAND(0,10);
    level1.fill('#',x);
    level2.fill(' ',10-x);
    s+="радиация:\n";
    s+="   |"+level1+level2+"|\n";

    p.drawText(QRect(40,27,400,400), s);
    p.end();
}
QString Planet::Resources()
{
    QVector <QString> mas={"He","Li","Al","Si","Ti",
                           "Fe","Co","Cu","Ag","Au",
                           "Hg","Pb","Pu","Ni","Zn"};
    QString res="";
    QVector <int> indexes={-1};
    int x=RAND(0,5);
    if (x==0) return "отсутствуют\n";
    for (int i=0;i<x;i++)
    {
        int index=RAND(0,14);
        while (indexes.contains(index)) index=RAND(0,14);
        indexes.append(index);
        if (i==x-1) res+=mas[index]+"\n";
        else res+=mas[index]+" ";
    }
    return res;
}
bool Planet::Collis(int r_o, int r, QVector<QVector<int>> planets)
{
    for (int i=0;i<planets.length();i++)
    {
        bool f1=planets[i][0]<=r_o-r and r_o-r<=planets[i][1];
        bool f2=planets[i][0]<=r_o+r and r_o+r<=planets[i][1];
        bool f3=r_o-r<=planets[i][0] and planets[i][0]<=r_o+r;
        bool f4=r_o-r<=planets[i][1] and planets[i][1]<=r_o+r;
        if (f1 or f2 or f3 or f4) return false;
    }
    return true;
}
void Planet::SystemMap()
{
    img_sys=QImage(currentpath+"window.png");
    if (starclass==12)
    {
        QPainter p;
        p.begin(&img_sys);
        int r;
        r=RAND(4,8);
        p.setPen(Qt::PenStyle::NoPen);
        p.setBrush(QBrush(QColor("#ff0000")));
        p.drawEllipse(165-r,165-r,2*r,2*r);
        p.end();
    }
    else
    {
        QVector <QString> color_star={"#E7ECFE","#F5F7FF","#FEFEFE","#FFFBE5",
                                      "#FFF3BD","#FFD48A","#FFA38A","#F7805F",
                                      "#EE4F3A","#DF3C26","#C53320","#AF3627"};
        QVector <QVector<int>> planets;

        QPainter p;
        p.begin(&img_sys);

        int r_star=RAND(8,25); //радиус звезды

        p.setPen(Qt::PenStyle::NoPen);
        p.setBrush(QBrush(QColor(color_star[starclass])));
        p.drawEllipse(165-r_star,165-r_star,r_star*2,r_star*2);

        //main planet begin
        int x,y,r;
        r=RAND(4,8);
        int r_o=qRound((140-s.temperature)*(125-r-r-r_star)*1.0/230+r+r_star);
        int angle=RAND(0,7);
        x=qRound(r_o*cos(angle))+165;
        y=165-qRound(r_o*sin(angle));
        QVector<int> vec={r_o-r,r_o+r};
        planets.append(vec);

        p.setPen(QPen(QColor("#f2e8c9"),2));
        p.setBrush(Qt::BrushStyle::NoBrush);
        p.drawEllipse(165-r_o,165-r_o,2*r_o,2*r_o);

        p.setPen(Qt::PenStyle::NoPen);
        p.setBrush(QBrush(QColor("#ff0000")));
        p.drawEllipse(x-r,y-r,2*r,2*r);

        //main planet end
        int num_planet=RAND(0,9);
        for (int i=0;i<num_planet;i++)
        {
            x=0;
            y=0;
            r=0;
            r_o=0;
            int count=0;
            while (r_o+r>125 or r_o-r<r_star or Collis(r_o,r,planets)==false)
            {
                r=RAND(4,8);
                x=RAND(r,328-r);
                y=RAND(r,328-r);
                r_o=qRound(sqrt(1.0*(x-165)*(x-165)+(y-165)*(y-165)));
                count++;
                if (count>15) break;
            }
            if (count>15) continue;
            vec={r_o-r,r_o+r};
            planets.append(vec);

            p.setPen(QPen(QColor("#f2e8c9"),2));
            p.setBrush(Qt::BrushStyle::NoBrush);
            p.drawEllipse(165-r_o,165-r_o,2*r_o,2*r_o);

            p.setPen(Qt::PenStyle::NoPen);
            p.setBrush(QBrush(QColor("#808080")));
            p.drawEllipse(x-r,y-r,2*r,2*r);
        }
        p.end();
    }
}
void Planet::GalaxyMap()
{
    QImage img_ptr=QImage(currentpath+"icon ptr.png");
    QImage img_map=QImage(currentpath+"GalaxyMap.png");
    img_gal=QImage(currentpath+"window.png");
    int x=RAND(0,218);
    int y=RAND(0,218);

    QPainter p;
    p.begin(&img_gal);

    for (int i=0;i<38;i++)
    {
        for (int k=0;k<38;k++)
        {
            if (!isBlack(img_ptr.pixelColor(i,k)))
            {
                img_map.setPixelColor(x+i,y+k,img_ptr.pixelColor(i,k));
            }
        }
    }
    p.drawImage(QRect(36,36,257,257),img_map);
    p.end();
}
void Planet::FinalImage()
{
    img_final=QImage(currentpath+"pattern.png");
    QImage img_window=QImage(currentpath+"window.png");
    QPainter p;
    p.begin(&img_final);
    p.drawImage(QRect(0,0,329,329),img_window);
    p.drawImage(QRect(36,36,257,257),img.scaled(257,257));
    p.drawImage(QRect(329,0,329,329),img_dsc);
    p.drawImage(QRect(0,329,329,329),img_sys);
    p.drawImage(QRect(329,329,329,329),img_gal);
    p.end();
}
void Planet::ImagesScale()
{
    img_nonscale=img.scaled(257,257);
    img=img.scaled(514,514);
    img_dsc=img_dsc.scaled(658,658);
    img_gal=img_gal.scaled(658,658);
    img_sys=img_sys.scaled(658,658);
}
void Planet::ImageReport(QString name, QVector<QVector<double>> data,QColor lowcolor,QColor highcolor)
{
    //служебная функция
    QString path="C:/Users/Никита/Desktop/report/"+name+".png";
    QImage r_img=QImage(world_size,world_size,QImage::Format_RGB32);
    double minv=1000000;
    double maxv=-1000000;
    foreach(QVector<double> i,data)
    {
        minv=qMin(minv,*std::min_element(i.begin(),i.end()));
        maxv=qMax(maxv,*std::max_element(i.begin(),i.end()));
    }
    for (int i=0;i<world_size;i++)
    {
        for (int k=0;k<world_size;k++)
        {
            double koef=(data[i][k]-minv)/(maxv-minv);
            r_img.setPixelColor(i,k,TransparentColor(lowcolor,highcolor,koef));
        }
    }
    r_img.save(path);
}
