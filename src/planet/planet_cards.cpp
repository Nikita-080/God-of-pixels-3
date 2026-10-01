#include "planet.h"
#include "planet_p.h"
#include "starspectrum.h"
#include "facts.h"
#include <QCoreApplication>
#include <QPainter>
#include <QPen>
#include <QtMath>
#include <QHash>
#include <QFont>
#include <QFontMetrics>
#include <algorithm>

QFont planetCardFont()
{
    QFont font(QStringLiteral("Consolas"));
    font.setPixelSize(11);
    font.setStyleHint(QFont::TypeWriter);
    font.setFixedPitch(true);
    return font;
}

int planetCardCharWidth()
{
    return qMax(1, QFontMetrics(planetCardFont()).horizontalAdvance(QLatin1Char('M')));
}

int planetCardLineHeight()
{
    return qMax(1, QFontMetrics(planetCardFont()).lineSpacing());
}

int planetCardTagCols()
{
    return qMax(1, planetCardContentRect().width() / planetCardCharWidth());
}

int planetCardTagRowPitch()
{
    return planetCardLineHeight() + kPlanetCardTagRowGap;
}

int planetCardTagRows()
{
    return qMax(1, planetCardContentRect().height() / planetCardTagRowPitch());
}

void planetPaintCardFrame(QPainter &p)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    QPen pen(planetCardInk(), 1);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    const int L = 8;
    const int T = 8;
    const int R = kPlanetCardSize - 9;
    const int B = kPlanetCardSize - 9;
    const int gap = 26;
    const int inset = 4;

    auto hEdge = [&](int y, int x0, int x1) {
        p.drawLine(x0, y, x1, y);
    };
    auto vEdge = [&](int x, int y0, int y1) {
        p.drawLine(x, y0, x, y1);
    };
    for (int d : {0, inset})
    {
        hEdge(T + d, L + gap, R - gap);
        hEdge(B - d, L + gap, R - gap);
        vEdge(L + d, T + gap, B - gap);
        vEdge(R - d, T + gap, B - gap);
    }

    auto corner = [&](int x, int y, int sx, int sy) {
        const int arm = 22;
        const int cut = 10;
        for (int d : {0, inset})
        {
            const int cx = x + sx * d;
            const int cy = y + sy * d;
            p.drawLine(QPoint(cx, cy + sy * arm), QPoint(cx, cy + sy * cut));
            p.drawLine(QPoint(cx, cy + sy * cut), QPoint(cx + sx * cut, cy));
            p.drawLine(QPoint(cx + sx * cut, cy), QPoint(cx + sx * arm, cy));
        }
    };
    corner(L, T, +1, +1);
    corner(R, T, -1, +1);
    corner(L, B, +1, -1);
    corner(R, B, -1, -1);

    const int midY = kPlanetCardSize / 2;
    for (int i = -2; i <= 1; ++i)
    {
        const int y = midY + i * 5;
        p.drawLine(L + 1, y, L + 8, y);
        p.drawLine(R - 8, y, R - 1, y);
    }

    p.restore();
}

QImage planetCardCanvas()
{
    QImage img(kPlanetCardSize, kPlanetCardSize, QImage::Format_RGB32);
    img.fill(QColor(0, 0, 0));
    QPainter p(&img);
    planetPaintCardFrame(p);
    return img;
}

namespace {
const double kMineralColorMaxDist = 88.0;
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

namespace {
void paintAsciiBar(QPainter &p, const QRect &row, const QString &label,
                   int lvl, int maxLvl, int barSlots, const QColor &fill)
{
    if (!row.isValid() || row.height() <= 0 || row.width() <= 0 || barSlots <= 0)
        return;
    const QString track = QLatin1Char('|') + QString(barSlots, QLatin1Char(' ')) + QLatin1Char('|');
    p.setPen(QPen(planetCardInk()));
    p.drawText(row, Qt::AlignLeft | Qt::AlignVCenter, label + track);
    const int filled = planetCardBarFill(lvl, maxLvl, barSlots);
    if (filled <= 0)
        return;
    p.setPen(QPen(fill));
    const QString hashes = QString(label.size() + 1, QLatin1Char(' ')) + QString(filled, QLatin1Char('#'));
    p.drawText(row, Qt::AlignLeft | Qt::AlignVCenter, hashes);
}

QString paddedBandLabel(const QString &label)
{
    return label.leftJustified(3, QLatin1Char(' '));
}
}

void Planet::Level(const QString &start, const QRect &row, int lvl, const QString &type, QPainter &p)
{
    if (!row.isValid() || row.height() <= 0 || row.width() <= 0)
        return;
    lvl = qBound(0, lvl, kPlanetCardBarMax);
    const DescriptionBarColor tone = planetDescriptionBarColor(lvl, type);
    QColor color = planetCardInk();
    if (tone == DescriptionBarColor::Red)
        color = QColor(200, 0, 0);
    else if (tone == DescriptionBarColor::Green)
        color = QColor(0, 200, 0);
    else if (tone == DescriptionBarColor::Yellow)
        color = QColor(200, 200, 0);
    paintAsciiBar(p, row, start, lvl, kPlanetCardBarMax, kPlanetCardBarSlots, color);
}

void Planet::DrawDescription()
{
    img_dsc = planetCardCanvas();
    const QString classes = QStringLiteral("OBAFGKMCSLTY");
    const QRect content = planetCardContentRect();

    QPainter p;
    p.begin(&img_dsc);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(QPen(planetCardInk()));
    p.setFont(planetCardFont());
    p.setClipRect(content);

    const QFontMetrics fm = p.fontMetrics();
    const int lh = fm.lineSpacing();
    const int cw = qMax(1, fm.horizontalAdvance(QLatin1Char('M')));
    int y = content.top();
    auto drawLine = [&](const QString &text) {
        if (y + lh > content.top() + content.height())
            return false;
        p.drawText(QRect(content.left(), y, content.width(), lh),
                   Qt::AlignLeft | Qt::AlignVCenter, text);
        y += lh;
        return true;
    };

    drawLine(QCoreApplication::translate("Planet", "name       - ") + name);
    QString resources = facts.resources;
    if (resources.endsWith(QLatin1Char('\n')))
        resources.chop(1);
    drawLine(QCoreApplication::translate("Planet", "resources  - ") + resources);

    QString starLine;
    if (starclass.isEmpty())
        starLine = QCoreApplication::translate("Planet", "star       - [not found]");
    else if (s.has_star && isStarBlackHole(s.star_spectrum))
        starLine = QCoreApplication::translate("Planet", "star       - black hole");
    else
    {
        starLine = QCoreApplication::translate("Planet", "star       - ");
        for (int i = 0; i < starclass.length(); i++)
        {
            if (starclass[i] < 0 || starclass[i] >= classes.size())
                continue;
            starLine += classes[starclass[i]];
            starLine += QLatin1Char(' ');
        }
    }
    drawLine(starLine);
    drawLine(QCoreApplication::translate("Planet", "spectrum:"));

    const bool blackHole = s.has_star && isStarBlackHole(s.star_spectrum);
    if (blackHole)
    {
        p.setPen(QColor(220, 20, 20));
        drawLine(QCoreApplication::translate("Planet", "[ERROR]"));
        p.setPen(QPen(planetCardInk()));
    }
    else
    {
        const QVector<int> bands = s.has_star ? s.star_spectrum : QVector<int>(StarBandCount, 0);
        const int colW = content.width() / 2;
        const int colChars = qMax(1, colW / cw);
        const int specSlots = qMax(1, colChars - 3 - 2);
        const int leftBands[4] = {StarGamma, StarXray, StarUv, StarIr};
        const int rightBands[4] = {StarRed, StarGreen, StarBlue, StarRadio};
        const QString leftLabels[4] = {
            QString(QChar(0x03B3)), QStringLiteral("X"), QStringLiteral("UV"), QStringLiteral("IR")
        };
        const QString rightLabels[4] = {
            QStringLiteral("R"), QStringLiteral("G"), QStringLiteral("B"), QStringLiteral("Ra")
        };
        for (int i = 0; i < 4; ++i)
        {
            if (y + lh > content.top() + content.height())
                break;
            const QRect left(content.left(), y, colW, lh);
            const QRect right(content.left() + colW, y, content.width() - colW, lh);
            paintAsciiBar(p, left, paddedBandLabel(leftLabels[i]),
                          starBand(bands, leftBands[i]), kStarBandMax, specSlots, planetCardInk());
            paintAsciiBar(p, right, paddedBandLabel(rightLabels[i]),
                          starBand(bands, rightBands[i]), kStarBandMax, specSlots, planetCardInk());
            y += lh;
        }
        p.setPen(QPen(planetCardInk()));
    }

    drawLine(QString(qMax(1, content.width() / cw), QLatin1Char('-')));

    auto nextRow = [&]() {
        if (y + lh > content.top() + content.height())
            return QRect();
        const QRect row(content.left(), y, content.width(), lh);
        y += lh;
        return row;
    };
    Level(QCoreApplication::translate("Planet", "life         "), nextRow(), facts.life, QStringLiteral("good"), p);
    Level(QCoreApplication::translate("Planet", "water        "), nextRow(), facts.water, QStringLiteral("neutral"), p);
    Level(QCoreApplication::translate("Planet", "ice          "), nextRow(), facts.ice, QStringLiteral("bad"), p);
    Level(QCoreApplication::translate("Planet", "radiation    "), nextRow(), facts.radiation, QStringLiteral("bad"), p);
    Level(QCoreApplication::translate("Planet", "temperature  "), nextRow(), facts.temperature, QStringLiteral("neutral"), p);
    Level(QCoreApplication::translate("Planet", "seismicity   "), nextRow(), facts.seismicity, QStringLiteral("bad"), p);
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
    const int cols = qMax(1, planetCardContentRect().width() / qMax(1,
        QFontMetrics(planetCardFont()).horizontalAdvance(QLatin1Char('W'))));
    const int budget = qMax(1, cols - prefix.size());
    QString res;
    int shown = 0;
    for (const PlanetOre &item : ranked)
    {
        if (shown >= kPlanetCardResourceMax)
            break;
        const QString next = res.isEmpty() ? item.symbol : (res + QLatin1Char(' ') + item.symbol);
        if (next.size() > budget)
            break;
        res = next;
        ++shown;
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
    img_gal = planetCardCanvas();
    const QRect view = planetCardViewport();
    const int outW = view.width();
    const int outH = view.height();
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
    p.drawImage(view, atlas);
    p.end();
}

void Planet::FinalImage()
{
    const int size = kPlanetCardSize;
    img_final = QImage(size * 2, size * 2, QImage::Format_RGB32);
    img_final.fill(QColor(0, 0, 0));
    QImage globeCard = planetCardCanvas();
    {
        QPainter gp(&globeCard);
        QImage globe = img_view.isNull() ? img : img_view;
        gp.drawImage(planetCardViewport(),
                     globe.scaled(planetCardViewport().size(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }
    QPainter p;
    p.begin(&img_final);
    p.drawImage(QRect(0, 0, size, size), globeCard);
    p.drawImage(QRect(size, 0, size, size), img_dsc);
    p.drawImage(QRect(0, size, size, size), img_sys);
    p.drawImage(QRect(size, size, size, size), img_gal);
    p.end();
}

void Planet::ImagesScale()
{
    const QSize hi(kPlanetCardSize * 2, kPlanetCardSize * 2);
    img_dsc = img_dsc.scaled(hi, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    img_gal = img_gal.scaled(hi, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    img_sys = img_sys.scaled(hi, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}
