#ifndef PLANET_P_H
#define PLANET_P_H

#include <QImage>
#include <QString>
#include <QVector>

void planetBoxBlurWrapX(QVector<QVector<double>> &field, int radius);
void planetBoxBlurClampY(QVector<QVector<double>> &field, int radius);
const QImage &planetCachedImage(const QString &path);
const QVector<QVector<int>> &planetNameMatrix();

#endif
