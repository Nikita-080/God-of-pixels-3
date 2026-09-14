#include "coloremotion.h"

#include <QtMath>
#include <algorithm>

double ColorEmotion::clamp(double value)
{
    return qBound(0.0, value, 1.0);
}

double ColorEmotion::gaussian(double x, double center, double width)
{
    const double d = (x - center) / width;
    return qExp(-0.5 * d * d);
}

EmotionVector ColorEmotion::analyze(const QColor &color)
{
    EmotionVector e;

    // HSV:
    // H = 0..359
    // S = 0..1
    // V = 0..1

    const double h = color.hsvHue() < 0
                         ? 0.0
                         : color.hsvHue();

    const double s = color.hsvSaturationF();
    const double v = color.valueF();

    // Ахроматические цвета не имеют выраженного оттенка.
    const bool colorful = s > 0.08;

    // Нормализованный оттенок: 0..1
    const double hue = h / 360.0;

    /*
     * Базовые эмоциональные области.
     *
     * Это НЕ психологическая модель.
     * Это простая эвристика, которую можно легко менять.
     */

    // ---------------------------------------------------------
    // RED -> ANGER
    // ---------------------------------------------------------

    const double red =
        std::max(
            gaussian(hue, 0.0, 0.055),
            gaussian(hue, 1.0, 0.055)
        );

    e.anger =
        red
        * (0.35 + 0.65 * s)
        * (0.25 + 0.75 * v);

    // ---------------------------------------------------------
    // YELLOW-GREEN -> DISGUST
    // ---------------------------------------------------------

    // ~60-100 градусов
    const double yellowGreen =
        gaussian(hue, 0.22, 0.10);

    e.disgust =
        yellowGreen
        * (0.25 + 0.75 * s)
        * (0.35 + 0.65 * v);

    // ---------------------------------------------------------
    // BLUE / DARK BLUE -> SADNESS
    // ---------------------------------------------------------

    const double blue =
        gaussian(hue, 0.62, 0.13);

    e.sadness =
        blue
        * (0.25 + 0.75 * s)
        * (1.0 - 0.55 * v);

    // ---------------------------------------------------------
    // GREEN / CYAN -> CALM
    // ---------------------------------------------------------

    const double green =
        gaussian(hue, 0.40, 0.13);

    e.calm =
        green
        * (0.35 + 0.65 * s)
        * (0.30 + 0.70 * v);

    // ---------------------------------------------------------
    // YELLOW / ORANGE -> JOY
    // ---------------------------------------------------------

    const double warm =
        std::max(
            gaussian(hue, 0.12, 0.08), // yellow
            gaussian(hue, 0.07, 0.055) // orange
        );

    e.joy =
        warm
        * (0.35 + 0.65 * s)
        * (0.35 + 0.65 * v);

    // ---------------------------------------------------------
    // PURPLE / MAGENTA -> SURPRISE
    // ---------------------------------------------------------

    const double purple =
        gaussian(hue, 0.80, 0.12);

    e.surprise =
        purple
        * (0.35 + 0.65 * s)
        * (0.30 + 0.70 * v);

    // ---------------------------------------------------------
    // FEAR
    // ---------------------------------------------------------

    // Страх здесь определяется не столько оттенком,
    // сколько комбинацией высокой насыщенности
    // и низкой яркости.
    //
    // Дополнительно усиливаем его для красно-фиолетовой
    // части спектра.

    const double dangerousHue =
        std::max(red, purple);

    const double darkness = 1.0 - v;

    e.fear =
        dangerousHue
        * s
        * darkness;

    // ---------------------------------------------------------
    // Снижаем эмоции для почти серых цветов
    // ---------------------------------------------------------

    if (!colorful)
    {
        e.anger    *= 0.25;
        e.disgust  *= 0.25;
        e.fear     *= 0.35;
        e.joy      *= 0.35;
        e.sadness  *= 0.65;
        e.calm     *= 0.85;
        e.surprise *= 0.20;
    }

    // ---------------------------------------------------------
    // Очень тёмные цвета дополнительно усиливают sadness/fear
    // ---------------------------------------------------------

    if (v < 0.15)
    {
        e.sadness = clamp(e.sadness + 0.25 * (1.0 - v));
        e.fear    = clamp(e.fear    + 0.20 * (1.0 - v));
    }

    // ---------------------------------------------------------
    // Нормализация
    // ---------------------------------------------------------

    e.anger    = clamp(e.anger);
    e.disgust  = clamp(e.disgust);
    e.fear     = clamp(e.fear);
    e.joy      = clamp(e.joy);
    e.sadness  = clamp(e.sadness);
    e.calm     = clamp(e.calm);
    e.surprise = clamp(e.surprise);

    return e;
}