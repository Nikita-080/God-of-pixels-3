#include "planet.h"
#include "planet_p.h"
#include "starspectrum.h"
#include <QCoreApplication>
#include <QPainter>
#include <QtMath>
#include <QHash>
#include <algorithm>

namespace {
const double kMineralColorMaxDist = 88.0;
const int kCardLineWidth = 28;
const int kNearestPerLayer = 5;

double colorDist(const QColor &a, const QColor &b)
{
    const double dr = a.red() - b.red();
    const double dg = a.green() - b.green();
    const double db = a.blue() - b.blue();
    return sqrt(dr * dr + dg * dg + db * db);
}

QVector<QString> nearestMinerals(const QColor &layerColor, bool soluble)
{
    struct Hit
    {
        QString symbol;
        double dist;
    };
    QVector<Hit> hits;
    for (const MineralSalt &m : planetMinerals())
    {
        const QColor salt = soluble ? m.soluble : m.insoluble;
        const double d = colorDist(layerColor, salt);
        if (d <= kMineralColorMaxDist)
            hits.append({m.symbol, d});
    }
    std::sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) {
        if (a.dist != b.dist)
            return a.dist < b.dist;
        return a.symbol < b.symbol;
    });
    QVector<QString> names;
    const int n = qMin(kNearestPerLayer, hits.size());
    names.reserve(n);
    for (int i = 0; i < n; ++i)
        names.append(hits[i].symbol);
    return names;
}

QVector<PlanetOre> collectOres(const Planet &p)
{
    QVector<PlanetOre> out;
    if (p.map_w <= 0 || p.map_h <= 0 || p.s.true_structure.size() < 8)
        return out;

    const QColor layerColor[7] = {
        p.s.ice_color, p.s.rock_color, p.s.mountain_color, p.s.plain_color,
        p.s.beach_color, p.s.shallow_color, p.s.ocean_color
    };
    const bool solubleLayer[7] = {false, false, false, false, false, true, true};
    qint64 area[7] = {0, 0, 0, 0, 0, 0, 0};
    for (int x = 0; x < p.map_w; ++x)
    {
        for (int y = 0; y < p.map_h; ++y)
        {
            const double h = p.matrix[x][y];
            for (int i = 0; i < 7; ++i)
            {
                if (p.s.true_structure[i] == p.s.true_structure[i + 1])
                    continue;
                if (h <= p.s.true_structure[i] && h >= p.s.true_structure[i + 1])
                {
                    ++area[i];
                    break;
                }
            }
        }
    }

    QHash<QString, qint64> score;
    for (int i = 0; i < 7; ++i)
    {
        if (area[i] <= 0 || !layerColor[i].isValid())
            continue;
        const QVector<QString> hits = nearestMinerals(layerColor[i], solubleLayer[i]);
        for (const QString &symbol : hits)
            score[symbol] += area[i];
    }
    if (score.isEmpty())
        return out;

    QHash<QString, bool> radioactive;
    QHash<QString, double> prevalence;
    for (const MineralSalt &m : planetMinerals())
    {
        radioactive.insert(m.symbol, m.radioactive);
        prevalence.insert(m.symbol, m.prevalence);
    }

    out.reserve(score.size());
    for (auto it = score.begin(); it != score.end(); ++it)
        out.append({it.key(), it.value(), radioactive.value(it.key()), prevalence.value(it.key())});
    std::sort(out.begin(), out.end(), [](const PlanetOre &a, const PlanetOre &b) {
        if (a.area != b.area)
            return a.area > b.area;
        return a.symbol < b.symbol;
    });
    return out;
}
}

QVector<PlanetOre> planetOreInventory(const Planet &planet)
{
    return collectOres(planet);
}

void Planet::GenerateDescription()
{
    facts.resources = Resources();
    facts.seismicity = qBound(0, s.seismicity, 12);
    if (s.has_star)
    {
        const double hi = (starBand(s.star_spectrum, StarGamma)
                           + starBand(s.star_spectrum, StarXray)
                           + starBand(s.star_spectrum, StarUv)) / 36.0;
        facts.radiation = qBound(0, facts.radiation + qRound(hi * 12.0), 12);
    }
}

void Planet::CalculateDescription()
{
    int pixel_count = map_w * map_h;
    if (pixel_count <= 0)
        pixel_count = 1;
    facts.life = qRound(plant_pixel_count * 12.0 / qMax(1, pixel_count - water_pixel_count));
    facts.ice = qRound(ice_pixel_count * 12.0 / pixel_count);
    facts.water = qRound(water_pixel_count * 12.0 / pixel_count);
    facts.temperature = qRound((s.effectiveTemperature() + 90) * 12.0 / 230);
}

void Planet::Level(QString start, int string, int lvl, QString type, QPainter &p)
{
    p.setPen(QPen(QColor(110, 170, 200)));
    QString s;
    s.fill('\n', string - 1);
    s += start + "|            |";
    p.drawText(QRect(40, 27, 400, 400), s);
    QColor color;
    if (type == "good")
    {
        if (lvl <= 4)
            color = QColor(200, 0, 0);
        else if (lvl >= 9)
            color = QColor(0, 200, 0);
        else
            color = QColor(200, 200, 0);
    }
    else if (type == "neutral")
    {
        if (lvl <= 2 || lvl >= 11)
            color = QColor(200, 0, 0);
        else if (lvl <= 4 || lvl >= 9)
            color = QColor(200, 200, 0);
        else
            color = QColor(0, 200, 0);
    }
    if (type == "bad")
    {
        if (lvl <= 4)
            color = QColor(0, 200, 0);
        else if (lvl >= 9)
            color = QColor(200, 0, 0);
        else
            color = QColor(200, 200, 0);
    }
    p.setPen(QPen(color));
    s.fill('\n', string - 1);
    QString a(start.length() + 1, ' ');
    QString b(lvl, '#');
    s += a + b;
    p.drawText(QRect(40, 27, 400, 400), s);
}

void Planet::DrawDescription()
{
    img_dsc = planetCachedImage(QStringLiteral(":/images/res/images/window.png"));
    const QString classes = QStringLiteral("OBAFGKMCSLTY");

    QPainter p;
    p.begin(&img_dsc);
    p.setPen(QPen(QColor(110, 170, 200)));
    p.setFont(QFont("Consolas", 8));

    QString head;
    head += QCoreApplication::translate("Planet", "name       - ") + name + "\n";
    head += QCoreApplication::translate("Planet", "resources  - ") + facts.resources;
    if (starclass.isEmpty())
        head += QCoreApplication::translate("Planet", "star       - [not found]");
    else
    {
        head += QCoreApplication::translate("Planet", "star       - ");
        for (int i = 0; i < starclass.length(); i++)
        {
            head += classes[starclass[i]];
            head += QLatin1Char(' ');
        }
    }
    head += QLatin1Char('\n');
    head += QCoreApplication::translate("Planet", "spectrum   - ") + QLatin1Char('\n');
    p.drawText(QRect(40, 27, 400, 400), head);
    const int specY = 27 + p.fontMetrics().lineSpacing() * 4;
    paintStarSpectrum(p, QRect(40, specY, 220, 36),
                      this->s.has_star ? this->s.star_spectrum : QVector<int>(StarBandCount, 0));

    Level(QCoreApplication::translate("Planet", "life         "), 9, facts.life, "good", p);
    Level(QCoreApplication::translate("Planet", "water        ", nullptr), 10, facts.water, "neutral", p);
    Level(QCoreApplication::translate("Planet", "ice          "), 11, facts.ice, "bad", p);
    Level(QCoreApplication::translate("Planet", "radiation    "), 12, facts.radiation, "bad", p);
    Level(QCoreApplication::translate("Planet", "temperature  "), 13, facts.temperature, "neutral", p);
    Level(QCoreApplication::translate("Planet", "seismicity   "), 14, facts.seismicity, "bad", p);
    p.end();
}

QString Planet::Resources()
{
    const QString notFound = QCoreApplication::translate("Planet", "[not found]\n");
    facts.radiation = 0;
    const QVector<PlanetOre> ranked = planetOreInventory(*this);
    if (ranked.isEmpty())
        return notFound;

    qint64 radioScore = 0;
    qint64 totalScore = 0;
    for (const PlanetOre &ore : ranked)
    {
        totalScore += ore.area;
        if (ore.radioactive)
            radioScore += ore.area;
    }
    if (totalScore > 0)
        facts.radiation = qBound(0, qRound(12.0 * double(radioScore) / double(totalScore)), 12);

    const QString prefix = QCoreApplication::translate("Planet", "resources  - ");
    const int budget = qMax(1, kCardLineWidth - prefix.size());
    QString res;
    for (const PlanetOre &item : ranked)
    {
        const QString next = res.isEmpty() ? item.symbol : (res + QLatin1Char(' ') + item.symbol);
        if (next.size() > budget)
            break;
        res = next;
    }
    if (res.isEmpty())
        return notFound;
    return res + QLatin1Char('\n');
}

void Planet::SystemMap()
{
    planetPaintTagCard(*this);
}

void Planet::GalaxyMap()
{
    QImage img_ptr = planetCachedImage(QStringLiteral(":/images/res/images/icon ptr.png"));
    QImage img_map = planetCachedImage(QStringLiteral(":/images/res/images/GalaxyMap.png"));
    img_gal = planetCachedImage(QStringLiteral(":/images/res/images/window.png"));
    rnd.seed(static_cast<quint32>(seed));
    int x = RAND(0, 218);
    int y = RAND(0, 218);

    QPainter p;
    p.begin(&img_gal);

    for (int i = 0; i < 38; i++)
    {
        for (int k = 0; k < 38; k++)
        {
            QColor pc = img_ptr.pixelColor(i, k);
            if (!(pc.red() == 0 && pc.green() == 0 && pc.blue() == 0))
                img_map.setPixelColor(x + i, y + k, pc);
        }
    }
    p.drawImage(QRect(36, 36, 257, 257), img_map);
    p.end();
}

void Planet::FinalImage()
{
    img_final = QImage(658, 658, QImage::Format_RGB32);
    const QImage &img_window = planetCachedImage(QStringLiteral(":/images/res/images/window.png"));
    QPainter p;
    p.begin(&img_final);
    p.drawImage(QRect(0, 0, 329, 329), img_window);
    QImage globe = img_view.isNull() ? img : img_view;
    p.drawImage(QRect(36, 36, 257, 257), globe.scaled(257, 257, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    p.drawImage(QRect(329, 0, 329, 329), img_dsc);
    p.drawImage(QRect(0, 329, 329, 329), img_sys);
    p.drawImage(QRect(329, 329, 329, 329), img_gal);
    p.end();
}

void Planet::ImagesScale()
{
    img_dsc = img_dsc.scaled(658, 658);
    img_gal = img_gal.scaled(658, 658);
    img_sys = img_sys.scaled(658, 658);
}
