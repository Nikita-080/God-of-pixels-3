#ifndef PLANET_P_H
#define PLANET_P_H

#include <QImage>
#include <QSet>
#include <QString>
#include <QVector>
#include <QColor>
#include <QFont>
#include <QRect>
#include <QtMath>

class QPainter;

constexpr int kPlanetCardSize = 329;
constexpr int kPlanetCardBarSlots = 16;
constexpr int kPlanetCardBarMax = 12;
constexpr int kPlanetCardResourceMax = 5;
constexpr int kPlanetCardTagRowGap = 3;

inline QRect planetCardViewport()
{
    return QRect(20, 20, 289, 289);
}

inline QColor planetCardInk()
{
    return QColor(110, 170, 200);
}

inline QRect planetCardContentRect()
{
    return planetCardViewport().adjusted(2, 2, -2, -2);
}

inline int planetCardBarFill(int lvl, int maxLvl, int barSlots)
{
    lvl = qBound(0, lvl, maxLvl);
    if (lvl <= 0 || maxLvl <= 0 || barSlots <= 0)
        return 0;
    return qBound(0, qRound(lvl * double(barSlots) / double(maxLvl)), barSlots);
}

QFont planetCardFont();
QImage planetCardCanvas();
void planetPaintCardFrame(QPainter &p);
int planetCardCharWidth();
int planetCardLineHeight();
int planetCardTagCols();
int planetCardTagRowPitch();
int planetCardTagRows();

void planetBoxBlurWrapX(QVector<QVector<double>> &field, int radius);
void planetBoxBlurClampY(QVector<QVector<double>> &field, int radius);
const QImage &planetCachedImage(const QString &path);
const QVector<QVector<int>> &planetNameMatrix();

struct MineralSalt
{
    QString symbol;
    QColor soluble;
    QColor insoluble;
    bool radioactive;
    double prevalence;
};

const QVector<MineralSalt> &planetMinerals();

struct PlanetOre
{
    QString symbol;
    qint64 area;
    bool radioactive;
    double prevalence;
};

QVector<PlanetOre> planetOreInventory(const class Planet &planet);
void planetPaintTagCard(class Planet &planet);
QSet<QString> planetActiveTagIds(const class Planet &planet);

enum class DescriptionBarColor
{
    Empty,
    Green,
    Yellow,
    Red
};

DescriptionBarColor planetDescriptionBarColor(int lvl, const QString &type);
bool planetDescriptionBarsAll(const class Facts &facts, DescriptionBarColor want);

#endif
