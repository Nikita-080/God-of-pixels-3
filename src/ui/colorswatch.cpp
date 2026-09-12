#include "colorswatch.h"
#include <QPalette>
#include <QVariant>

void ColorSwatch::setColor(QPushButton *button, const QColor &color)
{
    if (!button)
        return;
    button->setProperty("swatchColor", color);
    button->setStyleSheet(QString("QPushButton{background-color: rgb(%1,%2,%3);}")
                              .arg(color.red())
                              .arg(color.green())
                              .arg(color.blue()));
    QPalette palette = button->palette();
    const int luma = (color.red() * 299 + color.green() * 587 + color.blue() * 114) / 1000;
    palette.setColor(QPalette::ButtonText, luma >= 128 ? QColor(0, 0, 0) : QColor(255, 255, 255));
    button->setPalette(palette);
}

QColor ColorSwatch::color(QPushButton *button)
{
    if (!button)
        return QColor();
    const QVariant v = button->property("swatchColor");
    if (v.canConvert<QColor>())
        return v.value<QColor>();
    return QColor();
}
