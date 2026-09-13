#ifndef STARSPECTRUM_H
#define STARSPECTRUM_H

#include <QColor>
#include <QString>
#include <QVector>
#include <QVector3D>

class QPainter;
class QRect;

enum StarBand
{
    StarGamma = 0,
    StarXray,
    StarUv,
    StarRed,
    StarGreen,
    StarBlue,
    StarIr,
    StarRadio,
    StarBandCount
};

const int kStarTempMin = -90;
const int kStarBandMax = 12;

QVector<int> defaultStarSpectrum();
void clampStarSpectrum(QVector<int> &bands);
int starBand(const QVector<int> &bands, int i);
double starVisible(const QVector<int> &bands);
double starPar(const QVector<int> &bands);
double starHazard(const QVector<int> &bands);
QVector3D starLightRgb(const QVector<int> &bands);
int inferStarClass(const QVector<int> &bands);
QColor starBandColor(int band);
QString starBandTitle(int band);
void paintStarSpectrum(QPainter &p, const QRect &rect, const QVector<int> &bands);

#endif
