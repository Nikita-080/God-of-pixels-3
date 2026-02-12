#include "multislider.h"
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QCoreApplication>
MultiSlider::MultiSlider(QWidget *parent) : QWidget(parent)
{
    for (int i=0;i<8;i++){data.append(40+55*i);}
    for (int i=0;i<6;i++){capture[i]=false;}
    ReloadText();
}

void MultiSlider::ReloadText()
{
    names[0]=QCoreApplication::translate("MultiSlider", "rocks");
    names[1]=QCoreApplication::translate("MultiSlider", "mountains");
    names[2]=QCoreApplication::translate("MultiSlider", "plains");
    names[3]=QCoreApplication::translate("MultiSlider", "lowlands");
    names[4]=QCoreApplication::translate("MultiSlider", "beaches");
    names[5]=QCoreApplication::translate("MultiSlider", "shelf");
    names[6]=QCoreApplication::translate("MultiSlider", "ocean");
}

void MultiSlider::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setPen(QPen(QColor(110,170,200),4));
  painter.setBrush(QBrush(QColor(0,0,0)));
  painter.drawLine(28,48,28,433);
  for (int i=0;i<8;i++){
      painter.drawEllipse(20,data[i],15,15);
  }
  for (int i=0;i<7;i++){
      int pos=(data[i]+data[i+1]+14)/2;
      painter.drawText(QRect(35,pos-15,100,30),names[i]);
  }
}
void MultiSlider::mousePressEvent(QMouseEvent *event){
    for (int i=0;i<6;i++){
        if ((event->x()-28)*(event->x()-28)+(event->y()-data[i+1])*(event->y()-data[i+1])<=65){
            capture[i]=true;
        }
    }
    OldY=event->y();
}
void MultiSlider::mouseReleaseEvent(QMouseEvent *event){
    for (int i=0;i<6;i++){capture[i]=false;}
}
void MultiSlider::mouseMoveEvent(QMouseEvent *event){
    for (int i=0;i<6;i++){
        if (capture[i]){
            if (data[i+1]+event->y()-OldY>=data[i] && data[i+1]+event->y()-OldY<=data[i+2]){
                data[i+1]+=event->y()-OldY;
            }
        }    
    }
    OldY=event->y();
    update();
}
QVector<int> MultiSlider::GetData(){
    return data;
}
QVector<double> MultiSlider::GetTrueData()
{
    QVector<double> result;
    for (int i=0;i<8;i++)
    {
        result.append(280-((data[i]-40)*280*1.0/385));
    }
    return result;
}
void MultiSlider::SetData(QVector<int> NewData){
    data=NewData;
    update();
}
