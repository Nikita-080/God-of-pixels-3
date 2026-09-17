#include "starspectrum.h"
#include <QCoreApplication>
#include <QPainter>
#include <QtMath>

QVector<int> defaultStarSpectrum()
{
    return {0, 0, 3, 12, 12, 12, 6, 0};
}

void clampStarSpectrum(QVector<int> &bands)
{
    bands.resize(StarBandCount);
    for (int i = 0; i < StarBandCount; ++i)
        bands[i] = qBound(0, bands[i], kStarBandMax);
}

int starBand(const QVector<int> &bands, int i)
{
    if (i < 0 || i >= bands.size())
        return 0;
    return qBound(0, bands[i], kStarBandMax);
}

double starVisible(const QVector<int> &bands)
{
    const double sum = starBand(bands, StarRed) + starBand(bands, StarGreen) + starBand(bands, StarBlue);
    return qBound(0.0, sum / 36.0, 1.0);
}

double starPar(const QVector<int> &bands)
{
    const double sum = starBand(bands, StarBlue) + starBand(bands, StarRed);
    return qBound(0.0, sum / 24.0, 1.0);
}

double starHazard(const QVector<int> &bands)
{
    const double sum = starBand(bands, StarGamma) + starBand(bands, StarUv);
    return qBound(0.0, sum / 24.0, 1.0);
}

QVector3D starLightRgb(const QVector<int> &bands)
{
    const float r = float(starBand(bands, StarRed)) / float(kStarBandMax);
    const float g = float(starBand(bands, StarGreen)) / float(kStarBandMax);
    const float b = float(starBand(bands, StarBlue)) / float(kStarBandMax);
    QVector3D c(r, g, b);
    if (c.lengthSquared() < 1e-8f)
        return QVector3D(1, 1, 1);
    return c;
}

bool isStarBlackHole(const QVector<int> &bands)
{
    for (int i = 0; i < StarBandCount; ++i)
    {
        if (starBand(bands, i) > 0)
            return false;
    }
    return true;
}

int inferStarClass(const QVector<int> &bands)
{
    if (isStarBlackHole(bands))
        return 12;
    const int uv = starBand(bands, StarUv);
    const int r = starBand(bands, StarRed);
    const int g = starBand(bands, StarGreen);
    const int b = starBand(bands, StarBlue);
    const int ir = starBand(bands, StarIr);
    int peak = StarUv;
    int best = uv;
    if (b > best)
    {
        best = b;
        peak = StarBlue;
    }
    if (g > best)
    {
        best = g;
        peak = StarGreen;
    }
    if (r > best)
    {
        best = r;
        peak = StarRed;
    }
    if (ir > best && ir > 6 && (r + g + b) < 12)
        return 9;
    switch (peak)
    {
    case StarUv:
        return uv >= 9 ? 0 : 1;
    case StarBlue:
        return b >= 9 ? 2 : 3;
    case StarGreen:
        return 4;
    default:
        return r >= 9 ? 6 : 5;
    }
}

QColor starBandColor(int band)
{
    switch (band)
    {
    case StarGamma:
        return QColor(220, 80, 255);
    case StarXray:
        return QColor(200, 210, 255);
    case StarUv:
        return QColor(140, 60, 220);
    case StarRed:
        return QColor(220, 40, 40);
    case StarGreen:
        return QColor(40, 180, 50);
    case StarBlue:
        return QColor(50, 90, 230);
    case StarIr:
        return QColor(140, 30, 30);
    case StarRadio:
        return QColor(160, 160, 160);
    default:
        return QColor(180, 180, 180);
    }
}

QString starBandTitle(int band)
{
    switch (band)
    {
    case StarGamma:
        return QCoreApplication::translate("Spectrum", "Gamma");
    case StarXray:
        return QCoreApplication::translate("Spectrum", "X-ray");
    case StarUv:
        return QCoreApplication::translate("Spectrum", "UV");
    case StarRed:
        return QCoreApplication::translate("Spectrum", "Red");
    case StarGreen:
        return QCoreApplication::translate("Spectrum", "Green");
    case StarBlue:
        return QCoreApplication::translate("Spectrum", "Blue");
    case StarIr:
        return QCoreApplication::translate("Spectrum", "IR");
    case StarRadio:
        return QCoreApplication::translate("Spectrum", "Radio");
    default:
        return QString();
    }
}

void paintStarSpectrum(QPainter &p, const QRect &rect, const QVector<int> &bands)
{
    if (!rect.isValid() || rect.width() < 8 || rect.height() < 8)
        return;
    p.fillRect(rect, QColor(8, 12, 16));
    p.setPen(QColor(40, 70, 90));
    p.drawRect(rect.adjusted(0, 0, -1, -1));
    const int n = StarBandCount;
    const int inner = rect.adjusted(2, 2, -2, -2).width();
    const int gap = 2;
    const int barW = qMax(2, (inner - gap * (n - 1)) / n);
    const int baseY = rect.bottom() - 2;
    const int maxH = qMax(1, rect.height() - 6);
    for (int i = 0; i < n; ++i)
    {
        const int h = qRound(maxH * (starBand(bands, i) / double(kStarBandMax)));
        const int x = rect.left() + 2 + i * (barW + gap);
        const QRect bar(x, baseY - h, barW, h);
        p.fillRect(bar, starBandColor(i));
    }
}
