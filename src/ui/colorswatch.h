#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <QColor>
#include <QPushButton>

class ColorSwatch
{
public:
    static void setColor(QPushButton *button, const QColor &color);
    static QColor color(QPushButton *button);
};

#endif
