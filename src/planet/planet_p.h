#ifndef PLANET_P_H
#define PLANET_P_H

#include <QImage>
#include <QSet>
#include <QString>
#include <QVector>
#include <QColor>

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
