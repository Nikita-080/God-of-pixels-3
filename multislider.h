#ifndef MULTISLIDER_H
#define MULTISLIDER_H
#include <QVector>
#include <QWidget>

class MultiSlider : public QWidget
{
    Q_OBJECT
public:
    QVector <int> data;
    bool capture[6];
    QString names[7];
    int OldY;
public:
    explicit MultiSlider(QWidget *parent = nullptr);
    QVector <int> GetData();
    QVector <double> GetTrueData();
    void SetData(QVector<int>);
    void ReloadText();
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
signals:

};

#endif // MULTISLIDER_H
