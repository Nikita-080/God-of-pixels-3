#include "planet.h"
#include "planet_p.h"
#include "starspectrum.h"
#include "facts.h"
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

DescriptionBarColor planetDescriptionBarColor(int lvl, const QString &type)
{
    lvl = qBound(0, lvl, 12);
    if (lvl < 1)
        return DescriptionBarColor::Empty;
    if (type == QLatin1String("good"))
    {
        if (lvl <= 4)
            return DescriptionBarColor::Red;
        if (lvl >= 9)
            return DescriptionBarColor::Green;
        return DescriptionBarColor::Yellow;
    }
    if (type == QLatin1String("neutral"))
    {
        if (lvl <= 2 || lvl >= 11)
            return DescriptionBarColor::Red;
        if (lvl <= 4 || lvl >= 9)
            return DescriptionBarColor::Yellow;
        return DescriptionBarColor::Green;
    }
    if (type == QLatin1String("bad"))
    {
        if (lvl <= 4)
            return DescriptionBarColor::Green;
        if (lvl >= 9)
            return DescriptionBarColor::Red;
        return DescriptionBarColor::Yellow;
    }
    return DescriptionBarColor::Empty;
}

bool planetDescriptionBarsAll(const Facts &facts, DescriptionBarColor want)
{
    struct Bar
    {
        int lvl;
        const char *type;
    };
    const Bar bars[] = {
        {facts.life, "good"},
        {facts.water, "neutral"},
        {facts.ice, "bad"},
        {facts.radiation, "bad"},
        {facts.temperature, "neutral"},
        {facts.seismicity, "bad"},
    };
    for (const Bar &bar : bars)
    {
        const int lvl = qBound(0, bar.lvl, 12);
        if (lvl < 1)
            return false;
        if (planetDescriptionBarColor(lvl, QLatin1String(bar.type)) != want)
            return false;
    }
    return true;
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
    facts.life = qBound(0, qRound(plant_pixel_count * 12.0 / qMax(1, pixel_count - water_pixel_count)), 12);
    if (plant_pixel_count > 0 || !cities.isEmpty())
        facts.life = qMax(1, facts.life);
    else
        facts.life = 0;
    facts.ice = qBound(0, qRound(ice_pixel_count * 12.0 / pixel_count), 12);
    if (ice_pixel_count > 0)
        facts.ice = qMax(1, facts.ice);
    facts.water = qBound(0, qRound(water_pixel_count * 12.0 / pixel_count), 12);
    if (water_pixel_count > 0)
        facts.water = qMax(1, facts.water);
    facts.temperature = qBound(0, qRound((s.effectiveTemperature() + 90) * 12.0 / 230), 12);
}

void Planet::Level(QString start, int string, int lvl, QString type, QPainter &p)
{
    lvl = qBound(0, lvl, 12);
    p.setPen(QPen(QColor(110, 170, 200)));
    QString s;
    s.fill('\n', string - 1);
    s += start + "|            |";
    p.drawText(QRect(40, 27, 400, 400), s);
    const DescriptionBarColor tone = planetDescriptionBarColor(lvl, type);
    QColor color(110, 170, 200);
    if (tone == DescriptionBarColor::Red)
        color = QColor(200, 0, 0);
    else if (tone == DescriptionBarColor::Green)
        color = QColor(0, 200, 0);
    else if (tone == DescriptionBarColor::Yellow)
        color = QColor(200, 200, 0);
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
    head += QCoreApplication::translate("Planet", "spectrum:") + QLatin1Char('\n');
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
    img_gal = planetCachedImage(QStringLiteral(":/images/res/images/window.png")).copy();
    const int outW = 257;
    const int outH = 257;
    QImage atlas(outW, outH, QImage::Format_ARGB32);
    atlas.fill(Qt::transparent);

    QPainter ap;
    ap.begin(&atlas);
    ap.setRenderHint(QPainter::Antialiasing, false);

    const double maxLat = 85.0 * M_PI / 180.0;
    const double maxY = sphereAtanh(sin(maxLat));
    ap.setPen(QPen(QColor(140, 140, 140, 180), 1));
    for (int deg = 0; deg < 360; deg += 30)
    {
        const int gx = int(qBound(0.0, (deg / 360.0) * outW, double(outW - 1)));
        ap.drawLine(gx, 0, gx, outH - 1);
    }
    for (int deg = -60; deg <= 60; deg += 30)
    {
        const double lat = qDegreesToRadians(double(deg));
        const double mer = sphereAtanh(sin(lat));
        const int gy = int(qBound(0.0, (maxY - mer) / (2.0 * maxY) * outH, double(outH - 1)));
        ap.drawLine(0, gy, outW - 1, gy);
    }
    ap.end();

    auto wrapX = [this](int x) {
        const int w = map_w;
        if (w <= 0)
            return 0;
        return (x % w + w) % w;
    };
    auto inY = [this](int y) {
        return y >= 0 && y < map_h;
    };
    auto isLand = [&](int x, int y) {
        if (!inY(y) || map_w <= 0 || matrix.isEmpty())
            return false;
        x = wrapX(x);
        if (x >= matrix.size() || y >= matrix[x].size())
            return false;
        return matrix[x][y] >= water_level;
    };
    auto isIce = [&](int x, int y) {
        if (!inY(y) || t_map.isEmpty())
            return false;
        x = wrapX(x);
        if (x >= t_map.size() || y >= t_map[x].size())
            return false;
        if (!faultKind.isEmpty() && x < faultKind.size() && y < faultKind[x].size()
            && faultKind[x][y] != 0)
            return false;
        return t_map[x][y] < -15.0;
    };
    auto plot = [&](int px, int py, QRgb rgb) {
        if (px < 0 || py < 0 || px >= outW || py >= outH)
            return;
        atlas.setPixel(px, py, rgb);
        if (px + 1 < outW)
            atlas.setPixel(px + 1, py, rgb);
        if (py + 1 < outH)
            atlas.setPixel(px, py + 1, rgb);
    };

    const QRgb coastRgb = s.beach_color.isValid() ? s.beach_color.rgb() : qRgb(210, 190, 140);
    const QRgb iceRgb = qRgb(255, 255, 255);
    if (map_w > 0 && map_h > 0 && !matrix.isEmpty())
    {
        const int ndx[4] = {1, -1, 0, 0};
        const int ndy[4] = {0, 0, 1, -1};
        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                int mx = 0;
                int my = 0;
                if (!equirectToMercatorPixel(x, y, map_w, map_h, outW, outH, &mx, &my))
                    continue;
                if (isLand(x, y))
                {
                    bool edge = false;
                    for (int i = 0; i < 4; ++i)
                    {
                        if (!isLand(x + ndx[i], y + ndy[i]))
                        {
                            edge = true;
                            break;
                        }
                    }
                    if (edge)
                        plot(mx, my, coastRgb);
                }
                if (isIce(x, y))
                {
                    bool edge = false;
                    for (int i = 0; i < 4; ++i)
                    {
                        if (!isIce(x + ndx[i], y + ndy[i]))
                        {
                            edge = true;
                            break;
                        }
                    }
                    if (edge)
                        plot(mx, my, iceRgb);
                }
            }
        }
    }

    ap.begin(&atlas);
    ap.setPen(QPen(QColor(255, 0, 0), 1));
    ap.setBrush(QColor(255, 0, 0));
    for (const CityLight &c : cities)
    {
        int mx = 0;
        int my = 0;
        if (!equirectToMercatorPixel(c.mapX, c.mapY, map_w, map_h, outW, outH, &mx, &my))
            continue;
        ap.drawLine(mx - 2, my, mx + 2, my);
        ap.drawLine(mx, my - 2, mx, my + 2);
    }
    ap.end();

    QPainter p;
    p.begin(&img_gal);
    p.drawImage(QRect(36, 36, 257, 257), atlas);
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
