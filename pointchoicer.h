#ifndef POINTCHOICER_H
#define POINTCHOICER_H
#include <QVector>
#include <QWidget>
#include <QRandomGenerator>
class PointChoicer : public QWidget
{
    Q_OBJECT
public:
    int x_pos,y_pos;
    int dx,dy,R;
    int OldX,OldY;
    bool capture;
public:
    explicit PointChoicer(QWidget *parent = NULL);
    QVector <int> GetData();
    void SetData(QVector <int>);
    QVector <double> GetTrueData();
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    QVector <double> GetRand();
signals:

};

#endif // POINTCHOICER_H
