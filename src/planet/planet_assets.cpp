#include "planet_p.h"
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

static QColor mineralColor(const QJsonArray &arr)
{
    if (arr.size() < 3)
        return QColor();
    return QColor(qBound(0, arr.at(0).toInt(), 255),
                  qBound(0, arr.at(1).toInt(), 255),
                  qBound(0, arr.at(2).toInt(), 255));
}

const QVector<MineralSalt> &planetMinerals()
{
    static QVector<MineralSalt> table;
    static bool loaded = false;
    if (!loaded)
    {
        QFile file(QStringLiteral(":/txt_files/res/txt_files/minerals.json"));
        if (file.open(QIODevice::ReadOnly))
        {
            const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
            for (auto it = root.begin(); it != root.end(); ++it)
            {
                if (it.key().startsWith(QLatin1Char('_')))
                    continue;
                if (!it.value().isObject())
                    continue;
                const QJsonObject obj = it.value().toObject();
                MineralSalt m;
                m.symbol = it.key();
                m.soluble = mineralColor(obj.value(QStringLiteral("solubleSaltColor")).toArray());
                m.insoluble = mineralColor(obj.value(QStringLiteral("inSolubleSaltColor")).toArray());
                m.radioactive = obj.value(QStringLiteral("radioactivity")).toBool();
                if (m.soluble.isValid() && m.insoluble.isValid())
                    table.append(m);
            }
        }
        loaded = true;
    }
    return table;
}
