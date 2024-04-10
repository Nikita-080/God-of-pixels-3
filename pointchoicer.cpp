#include "pointchoicer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTime>
PointChoicer::PointChoicer(QWidget *parent) : QWidget(parent)
{
    dx=20;
    dy=30;
    R=175;
    x_pos=95+dx;
    y_pos=190+dy;
    capture=false;
}
void PointChoicer::paintEvent(QPaintEvent *){
    QPainter painter(this);
    //pens
    QPen norm_pen=QPen(QColor(110,170,200),2);
    QPen dash_pen=QPen(QColor(0,255,127),2);
    dash_pen.setStyle(Qt::DashLine);
    //static graphic
    painter.setBrush(Qt::NoBrush);
    painter.setPen(norm_pen);
    painter.drawLine(dx,R+dy,2*R+dx,R+dy);
    painter.drawLine(R+dx,dy,R+dx,2*R+dy);
    painter.drawEllipse(QPoint(R+dx,R+dy),R,R);
    //dinamic graphic
    painter.setPen(dash_pen);
    painter.drawLine(dx,y_pos,2*R+dx,y_pos);
    painter.drawLine(x_pos,dy,x_pos,2*R+dy);
    painter.setPen(norm_pen);
    painter.setBrush(QBrush(QColor(0,0,0)));
    painter.drawEllipse(QPoint(x_pos,y_pos),8,8);
}
void PointChoicer::mousePressEvent(QMouseEvent *event){
    if ((event->x()-x_pos)*(event->x()-x_pos)+(event->y()-y_pos)*(event->y()-y_pos)<=65){
        capture=true;
    }
    OldX=event->x();
    OldY=event->y();
}
void PointChoicer::mouseReleaseEvent(QMouseEvent *event){
    capture=false;
}
void PointChoicer::mouseMoveEvent(QMouseEvent *event){
    if (capture){
        int newx=x_pos+event->x()-OldX;
        int newy=y_pos+event->y()-OldY;
        if ((newx-R-dx)*(newx-R-dx)+(newy-R-dy)*(newy-R-dy)<=R*R){
            x_pos=newx;
            y_pos=newy;
        }
    }
    OldX=event->x();
    OldY=event->y();
    update();
}
QVector<int> PointChoicer::GetData(){
    QVector<int> vec={x_pos,y_pos};
    return vec;
}
void PointChoicer::SetData(QVector <int> new_data){
    x_pos=new_data[0];
    y_pos=new_data[1];
    update();
}
QVector <double> PointChoicer::GetTrueData()
{
    QVector <double> vec;
    vec.append((x_pos-dx-R)*1.0/R);
    vec.append((R-(y_pos-dy))*1.0/R);
    return vec;
}
QVector <double> PointChoicer::GetRand(){
    QRandomGenerator rnd;
    QTime midnight(0,0,0);
    rnd.seed(midnight.secsTo(QTime::currentTime()));
    int xR=0;
    int yR=0;
    while ((xR-R-dx)*(xR-R-dx)+(yR-R-dy)*(yR-R-dy)>=R*R){
        xR=rnd.generate()%(2*R)+dx;
        yR=rnd.generate()%(2*R)+dy;
    }
    QVector<double> vec;
    vec.append(1.0*(xR-R-dx)/R);
    vec.append(1.0*(R-(yR-dy))/R);
    return vec;
}

