#include "planet.h"
#include "planet_p.h"
#include <algorithm>
#include <QtMath>
#include <QRgb>

QColor Planet::TransparentColor(QColor color1, QColor color2, double koef)
{
    int r = qRound((1 - koef) * color1.red() + koef * color2.red());
    int g = qRound((1 - koef) * color1.green() + koef * color2.green());
    int b = qRound((1 - koef) * color1.blue() + koef * color2.blue());
    return QColor(r, g, b);
}

QColor Planet::DispersionColor(QColor color, int disp)
{
    const int r = qBound(0, color.red() + RAND(-disp, +disp), 255);
    const int g = qBound(0, color.green() + RAND(-disp, +disp), 255);
    const int b = qBound(0, color.blue() + RAND(-disp, +disp), 255);
    return QColor(r, g, b);
}

QColor Planet::LowerColor(QColor color, double koef)
{
    return QColor(qRound(color.red() * koef), qRound(color.green() * koef), qRound(color.blue() * koef));
}

void Planet::ImageCreating()
{
    img = QImage(map_w, map_h, QImage::Format_RGB32);
    img.fill(color_black);
    QVector<double> level_aver;
    if (s.is_gradient)
    {
        for (int i = 0; i < level_up.length(); i++)
            level_aver.append((level_up[i] + level_down[i]) / 2);
        level_aver.append(0);
    }
    for (int i = 0; i < map_w; i++)
    {
        for (int k = 0; k < map_h; k++)
        {
            if (matrix[i][k] < water_level)
                water_pixel_count++;
            QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(k));
            if (s.is_gradient)
            {
                int index = 0;
                while (index < level_aver.length() && matrix[i][k] < level_aver[index])
                    index++;
                if (index == 0)
                    line[i] = level_color[0].rgb();
                else if (index == level_aver.length() - 1)
                    line[i] = level_color.last().rgb();
                else
                {
                    double koef = (matrix[i][k] - level_aver[index]) / (level_aver[index - 1] - level_aver[index]);
                    line[i] = TransparentColor(level_color[index], level_color[index - 1], koef).rgb();
                }
            }
            else
            {
                for (int j = 0; j < level_up.length(); j++)
                {
                    if (matrix[i][k] <= level_up[j] and matrix[i][k] >= level_down[j])
                    {
                        line[i] = level_color[j].rgb();
                        break;
                    }
                }
            }
        }
    }
}

void Planet::Plant()
{
    plant_pixel_count = 0;
    if (!s.is_plant || !(water_level > 0) || !s.is_atmo)
        return;
    const double keep = qBound(0.0, s.parLight() * (1.0 - s.hazardLight()), 1.0);
    if (keep <= 0.0)
        return;
    const QImage &diagram = planetCachedImage(s.is_gradient
        ? QStringLiteral(":/images/res/images/plantmatrixblur.png")
        : QStringLiteral(":/images/res/images/plantmatrix.png"));
    if (diagram.isNull())
        return;
    const int dw = diagram.width() - 1;
    const int dh = diagram.height() - 1;
    for (int i = 0; i < map_w; i++)
    {
        for (int k = 0; k < map_h; k++)
        {
            if (matrix[i][k] <= water_level)
                continue;
            if (!faultKind.isEmpty() && faultKind[i][k] != 0)
                continue;
            const double T = t_map[i][k];
            const double W = r_map[i][k];
            if (W < 0 || W > 450 || T < -15 || T > 35)
                continue;
            const double tFit = qExp(-0.5 * qPow((T - 12.0) / 22.0, 2.0));
            const double wFit = qBound(0.0, W / 70.0, 1.0);
            if (tFit * (0.4 + 0.6 * wFit) < 1.0 - keep)
                continue;
            int x = qRound(-1.18 * T + 41.3);
            int y = qRound(-0.13 * W + 59);
            x = qBound(0, x, dw);
            y = qBound(0, y, dh);
            QColor color = diagram.pixelColor(x, y);
            if (color.alpha() < 16)
                continue;
            QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(k));
            if (s.is_gradient)
            {
                const double edge = qMin(qMin(x, y), qMin(dw - x, dh - y));
                if (edge < 11)
                    color = TransparentColor(QColor::fromRgb(line[i]), color, edge / 10.0);
            }
            line[i] = color.rgb();
            plant_pixel_count++;
        }
    }
}

void Planet::Polar()
{
    if (water_level == 0)
        return;
    QColor color = QColor(255, 255, 255);
    QColor lowcolor = QColor(150, 150, 150);
    QVector<QColor> polar_color;
    if (!s.is_gradient)
    {
        polar_color.append(color);
        if (level_color.length() > 1)
        {
            for (int i = 1; i < level_color.length(); i++)
            {
                if (level_up[i] < water_level)
                    polar_color.append(polar_color[i - 1]);
                else
                    polar_color.append(LowerColor(polar_color[i - 1], 0.9));
            }
        }
    }
    for (int i = 0; i < map_w; i++)
    {
        for (int j = 0; j < map_h; j++)
        {
            double T = t_map[i][j];
            if (T < -15)
            {
                if (!faultKind.isEmpty() && faultKind[i][j] != 0)
                    continue;
                ice_pixel_count++;
                QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(j));
                if (s.is_gradient)
                {
                    double koef = (matrix[i][j] - water_level) / (280 - water_level);
                    QColor cur_color = TransparentColor(lowcolor, color, koef);
                    koef = (-T - 15) / (1 + abs(-T - 15));
                    cur_color = TransparentColor(QColor::fromRgb(line[i]), cur_color, koef);
                    line[i] = cur_color.rgb();
                }
                else
                {
                    for (int q = 0; q < level_color.length(); q++)
                    {
                        if (level_up[q] >= matrix[i][j] and matrix[i][j] >= level_down[q])
                        {
                            line[i] = polar_color[q].rgb();
                            break;
                        }
                    }
                }
            }
        }
    }
}

void Planet::Noise()
{
    if (s.noise == 0)
        return;
    rnd.seed(static_cast<quint32>(seed));
    for (int i = 0; i < map_w; i++)
    {
        for (int j = 0; j < map_h; j++)
        {
            QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(j));
            line[i] = DispersionColor(QColor::fromRgb(line[i]), s.noise).rgb();
        }
    }
}

QImage Planet::ImageReport(const QVector<QVector<double>> &data, QColor lowcolor, QColor highcolor)
{
    QImage r_img = QImage(map_w, map_h, QImage::Format_RGB32);
    double minv = 1000000;
    double maxv = -1000000;
    for (const QVector<double> &row : data)
    {
        minv = qMin(minv, *std::min_element(row.begin(), row.end()));
        maxv = qMax(maxv, *std::max_element(row.begin(), row.end()));
    }
    for (int i = 0; i < map_w; i++)
    {
        for (int k = 0; k < map_h; k++)
        {
            double koef = (data[i][k] - minv) / (maxv - minv);
            QRgb *line = reinterpret_cast<QRgb *>(r_img.scanLine(k));
            line[i] = TransparentColor(lowcolor, highcolor, koef).rgb();
        }
    }
    return r_img;
}
