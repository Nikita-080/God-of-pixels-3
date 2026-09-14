#pragma once

#include <QColor>
#include <QMetaType>

struct EmotionVector
{
    double anger     = 0.0;
    double disgust   = 0.0;
    double fear      = 0.0;
    double joy       = 0.0;
    double sadness   = 0.0;
    double calm      = 0.0;
    double surprise  = 0.0;
};

Q_DECLARE_METATYPE(EmotionVector)

class ColorEmotion
{
public:
    static EmotionVector analyze(const QColor &color);

private:
    static double gaussian(double x, double center, double width);
    static double clamp(double value);
};