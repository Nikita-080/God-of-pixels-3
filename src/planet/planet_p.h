#ifndef PLANET_P_H
#define PLANET_P_H

#include <QImage>
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
};

const QVector<MineralSalt> &planetMinerals();
void planetPaintTagCard(class Planet &planet);

#endif
