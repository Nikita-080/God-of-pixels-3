#include "planet_p.h"
#include <QFile>
#include <QHash>
#include <QTextStream>

const QImage &planetCachedImage(const QString &path)
{
    static QHash<QString, QImage> cache;
    auto it = cache.find(path);
    if (it == cache.end())
        it = cache.insert(path, QImage(path));
    return it.value();
}

const QVector<QVector<int>> &planetNameMatrix()
{
    static QVector<QVector<int>> table;
    static bool loaded = false;
    if (!loaded)
    {
        QFile file(QStringLiteral(":/txt_files/res/txt_files/NameMatrix.txt"));
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            while (!stream.atEnd())
            {
                const QStringList data = stream.readLine().split(QLatin1Char(' '));
                QVector<int> row;
                row.reserve(data.size());
                for (const QString &item : data)
                    row.append(item.toInt());
                table.append(row);
            }
        }
        loaded = true;
    }
    return table;
}
