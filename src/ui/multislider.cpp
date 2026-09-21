#include "multislider.h"
#include <QCoreApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QVector>

namespace {
constexpr int kDataMin = 40;
constexpr int kHandle = 15;
constexpr int kPad = 8;
constexpr int kTrackX = 20;

int paintY(int dataY)
{
    return dataY - kDataMin + kPad;
}

int contentHeight()
{
    const int dataMax = kDataMin + 55 * 7;
    return kPad + (dataMax - kDataMin) + kHandle + kPad;
}
}

MultiSlider::MultiSlider(QWidget *parent) : QWidget(parent)
{
    for (int i=0;i<8;i++){data.append(40+55*i);}
    for (int i=0;i<6;i++){capture[i]=false;}
    setFixedSize(210, contentHeight());
    ReloadText();
}

QSize MultiSlider::sizeHint() const
{
    return QSize(210, contentHeight());
}

QSize MultiSlider::minimumSizeHint() const
{
    return sizeHint();
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
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(QPen(QColor(46, 66, 82), 1));
  painter.setBrush(QBrush(QColor(18, 24, 32)));
  const int y0 = paintY(data.first()) + kHandle / 2;
  const int y1 = paintY(data.last()) + kHandle / 2;
  painter.drawLine(kTrackX + kHandle / 2, y0, kTrackX + kHandle / 2, y1);
  painter.setPen(QPen(QColor(94, 234, 212), 1));
  painter.setBrush(QBrush(QColor(18, 24, 32)));
  for (int i=0;i<8;i++){
      painter.drawEllipse(kTrackX, paintY(data[i]), kHandle, kHandle);
  }
  painter.setPen(QPen(QColor(197, 216, 228), 1));
  for (int i=0;i<7;i++){
      int pos=(paintY(data[i])+paintY(data[i+1])+kHandle)/2;
      painter.drawText(QRect(kTrackX + kHandle + 8, pos-15, 120, 30), names[i]);
  }
}
void MultiSlider::mousePressEvent(QMouseEvent *event){
    for (int i=0;i<6;i++){
        const int hx = kTrackX + kHandle / 2;
        const int hy = paintY(data[i+1]) + kHandle / 2;
        const int dx = event->x() - hx;
        const int dy = event->y() - hy;
        if (dx * dx + dy * dy <= 65){
            capture[i]=true;
        }
    }
    OldY=event->y();
}
void MultiSlider::mouseReleaseEvent(QMouseEvent *){
    bool any=false;
    for (int i=0;i<6;i++){
        if (capture[i]) any=true;
        capture[i]=false;
    }
    if (any) emit valueChanged();
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
