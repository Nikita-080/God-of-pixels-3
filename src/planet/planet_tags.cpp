#include "planet_p.h"
#include "planet.h"
#include "starspectrum.h"
#include "coloremotion.h"
#include <QCoreApplication>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPair>
#include <QRandomGenerator>
#include <QSet>
#include <QtMath>
#include <algorithm>

namespace {

const int kTagCols = 28;
const int kTagRows = 13;
const double kValuablePrevalence = 10.0;
const double kOreShareCut = 0.12;
const double kRuggedLandShare = 0.08;

struct PlanetTagDef
{
    QString id;
    QString family;
    int priority;
    QString tone;
    bool easter = false;
    bool civ = false;
    QVector<QString> labels;
};

struct PlacedTag
{
    QString text;
    QColor color;
    int col;
    int row;
};

struct SurfaceScan
{
    int pixels = 1;
    int lava = 0;
    int rifts = 0;
    int cloud = 0;
    int mountain = 0;
    qint64 layerArea[7] = {0, 0, 0, 0, 0, 0, 0};
};

struct TagWorld
{
    SurfaceScan scan;
    int wealth = 0;
    int waterRank = 0;
    int iceRank = 0;
    int floraRank = 0;
    bool rugged = false;
    bool valuable = false;
    bool hazard = false;
    QString emotion;
};

SurfaceScan scanSurface(const Planet &p)
{
    SurfaceScan s;
    const int w = p.map_w;
    const int h = p.map_h;
    s.pixels = qMax(1, w * h);
    if (w <= 0 || h <= 0)
        return s;
    const bool hasLava = !p.lavaHeat.isEmpty() && p.lavaHeat.size() == w;
    const bool hasFault = !p.faultKind.isEmpty() && p.faultKind.size() == w;
    const bool hasCloud = p.s.is_cloud && !p.c_map.isEmpty() && p.c_map.size() == w;
    const bool hasStruct = p.s.true_structure.size() >= 8;
    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            if (x < p.matrix.size() && y < p.matrix[x].size())
            {
                const double z = p.matrix[x][y];
                if (hasStruct)
                {
                    for (int i = 0; i < 7; ++i)
                    {
                        if (p.s.true_structure[i] == p.s.true_structure[i + 1])
                            continue;
                        if (z <= p.s.true_structure[i] && z >= p.s.true_structure[i + 1])
                        {
                            ++s.layerArea[i];
                            if (i == 2)
                                ++s.mountain;
                            break;
                        }
                    }
                }
            }
            if (hasLava && y < p.lavaHeat[x].size() && p.lavaHeat[x][y] > 0.2)
                ++s.lava;
            if (hasFault && y < p.faultKind[x].size() && p.faultKind[x][y] == 1)
                ++s.rifts;
            if (hasCloud && y < p.c_map[x].size() && p.c_map[x][y] > 0.45)
                ++s.cloud;
        }
    }
    return s;
}

int lavaRank(const SurfaceScan &s)
{
    const double f = double(s.lava) / double(s.pixels);
    if (f < 0.001)
        return 0;
    if (f < 0.012)
        return 1;
    if (f < 0.045)
        return 2;
    return 3;
}

int cloudRank(const Planet &p, const SurfaceScan &s)
{
    if (!p.s.is_cloud)
        return 0;
    const double f = double(s.cloud) / double(s.pixels);
    if (f < 0.12)
        return 1;
    if (f < 0.45)
        return 2;
    return 3;
}

int rank5(int v)
{
    v = qBound(0, v, 12);
    if (v <= 1)
        return 0;
    if (v <= 4)
        return 1;
    if (v <= 7)
        return 2;
    if (v <= 10)
        return 3;
    return 4;
}

int fracToRank5(double frac)
{
    return rank5(qRound(qBound(0.0, frac, 1.0) * 12.0));
}

int floraRank(const Planet &p)
{
    if (!p.s.is_plant || !p.s.is_atmo || p.plant_pixel_count <= 0)
        return 0;
    return rank5(p.facts.life);
}

int oreWealthRank(const QVector<PlanetOre> &ores)
{
    const int n = ores.size();
    if (n <= 0)
        return 0;
    if (n <= 2)
        return 1;
    if (n <= 4)
        return 2;
    if (n <= 7)
        return 3;
    return 4;
}

void markOreQuality(const QVector<PlanetOre> &ores, bool &valuable, bool &hazard)
{
    valuable = false;
    hazard = false;
    qint64 total = 0;
    for (const PlanetOre &ore : ores)
        total += ore.area;
    if (total <= 0)
        return;
    for (const PlanetOre &ore : ores)
    {
        const double share = double(ore.area) / double(total);
        if (share < kOreShareCut)
            continue;
        if (ore.prevalence < kValuablePrevalence)
            valuable = true;
        if (ore.radioactive)
            hazard = true;
    }
}

void addEmotion(EmotionVector &acc, const EmotionVector &e, double w)
{
    acc.anger += e.anger * w;
    acc.disgust += e.disgust * w;
    acc.fear += e.fear * w;
    acc.joy += e.joy * w;
    acc.sadness += e.sadness * w;
    acc.calm += e.calm * w;
    acc.surprise += e.surprise * w;
}

QString dominantEmotion(const EmotionVector &e)
{
    const struct
    {
        const char *id;
        double v;
    } items[] = {
        {"anger", e.anger},
        {"disgust", e.disgust},
        {"fear", e.fear},
        {"joy", e.joy},
        {"sadness", e.sadness},
        {"calm", e.calm},
        {"surprise", e.surprise}
    };
    int best = -1;
    double mx = 0.0;
    for (int i = 0; i < 7; ++i)
    {
        if (items[i].v > mx)
        {
            mx = items[i].v;
            best = i;
        }
    }
    if (best < 0 || mx <= 0.0)
        return QString();
    return QString::fromLatin1(items[best].id);
}

QString landEmotionId(const Planet &p, const SurfaceScan &scan)
{
    const QColor layerColor[7] = {
        p.s.ice_color, p.s.rock_color, p.s.mountain_color, p.s.plain_color,
        p.s.beach_color, p.s.shallow_color, p.s.ocean_color
    };
    EmotionVector acc;
    double total = 0.0;
    for (int i = 0; i < 7; ++i)
    {
        if (scan.layerArea[i] <= 0 || !layerColor[i].isValid())
            continue;
        const double w = double(scan.layerArea[i]);
        addEmotion(acc, ColorEmotion::analyze(layerColor[i]), w);
        total += w;
    }
    if (total <= 0.0)
        return QString();
    const double inv = 1.0 / total;
    acc.anger *= inv;
    acc.disgust *= inv;
    acc.fear *= inv;
    acc.joy *= inv;
    acc.sadness *= inv;
    acc.calm *= inv;
    acc.surprise *= inv;
    return dominantEmotion(acc);
}

QString skyEmotionId(const Planet &p)
{
    EmotionVector acc;
    double total = 0.0;
    if (p.s.is_cloud && p.s.cloud_color.isValid())
    {
        addEmotion(acc, ColorEmotion::analyze(p.s.cloud_color), 1.0);
        total += 1.0;
    }
    if (p.s.is_atmo && p.s.atmo_color.isValid())
    {
        addEmotion(acc, ColorEmotion::analyze(p.s.atmo_color), 1.0);
        total += 1.0;
    }
    if (total <= 0.0)
        return QString();
    const double inv = 1.0 / total;
    acc.anger *= inv;
    acc.disgust *= inv;
    acc.fear *= inv;
    acc.joy *= inv;
    acc.sadness *= inv;
    acc.calm *= inv;
    acc.surprise *= inv;
    return dominantEmotion(acc);
}

QString planetEmotionId(const Planet &p, const SurfaceScan &scan)
{
    const bool useSky = (p.s.is_atmo && p.s.atmo_transparent > 5)
        || (p.s.is_cloud && p.s.cloud_transparent > 5);
    if (useSky)
    {
        const QString sky = skyEmotionId(p);
        if (!sky.isEmpty())
            return sky;
    }
    return landEmotionId(p, scan);
}

QString metalEnglishName(const QString &symbol)
{
    static QHash<QString, QString> table;
    static bool loaded = false;
    if (!loaded)
    {
        QFile file(QStringLiteral(":/txt_files/res/txt_files/metal_lex.json"));
        if (file.open(QIODevice::ReadOnly))
        {
            const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
            for (auto it = root.begin(); it != root.end(); ++it)
            {
                const QString en = it.value().isString()
                                       ? it.value().toString()
                                       : it.value().toObject().value(QStringLiteral("en")).toString();
                if (!en.isEmpty())
                    table.insert(it.key(), en);
            }
        }
        loaded = true;
    }
    return table.value(symbol, symbol);
}

QString metalForm(const QString &en, const char *comment)
{
    const QByteArray utf8 = en.toUtf8();
    return QCoreApplication::translate("PlanetTags", utf8.constData(), comment);
}

QString orePhrase(const QString &symbol, int kind, QRandomGenerator &rnd)
{
    const QString en = metalEnglishName(symbol);
    const QString gen = metalForm(en, "metal genitive");
    const QString adj = metalForm(en, "metal adjective");
    const int pick = rnd.bounded(2);
    QString text;
    if (kind <= 0)
    {
        text = pick == 0
                   ? QCoreApplication::translate("PlanetTags", "%1 ores").arg(adj)
                   : QCoreApplication::translate("PlanetTags", "much %1").arg(gen);
    }
    else if (kind == 1)
    {
        text = pick == 0
                   ? QCoreApplication::translate("PlanetTags", "%1 deposits").arg(gen)
                   : QCoreApplication::translate("PlanetTags", "%1 veins").arg(gen);
    }
    else
    {
        text = pick == 0
                   ? QCoreApplication::translate("PlanetTags", "traces of %1").arg(gen)
                   : QCoreApplication::translate("PlanetTags", "a little %1").arg(gen);
    }
    return text;
}

QColor toneColor(const QString &tone)
{
    if (tone == QLatin1String("good"))
        return QColor(0, 200, 0);
    if (tone == QLatin1String("caution"))
        return QColor(200, 200, 0);
    if (tone == QLatin1String("bad"))
        return QColor(200, 0, 0);
    if (tone == QLatin1String("anomaly"))
        return QColor(180, 80, 220);
    if (tone == QLatin1String("easter"))
        return QColor(150, 150, 150);
    return QColor(110, 170, 200);
}

const QVector<PlanetTagDef> &tagCatalog()
{
    static QVector<PlanetTagDef> table;
    static bool loaded = false;
    if (!loaded)
    {
        QFile file(QStringLiteral(":/txt_files/res/txt_files/planet_tags.json"));
        if (file.open(QIODevice::ReadOnly))
        {
            const QJsonArray arr = QJsonDocument::fromJson(file.readAll())
                                       .object()
                                       .value(QStringLiteral("tags"))
                                       .toArray();
            for (const QJsonValue &v : arr)
            {
                if (!v.isObject())
                    continue;
                const QJsonObject o = v.toObject();
                PlanetTagDef d;
                d.id = o.value(QStringLiteral("id")).toString();
                d.family = o.value(QStringLiteral("family")).toString();
                d.priority = o.value(QStringLiteral("priority")).toInt(100);
                d.tone = o.value(QStringLiteral("tone")).toString(QStringLiteral("info"));
                d.easter = o.value(QStringLiteral("easter")).toBool();
                d.civ = o.value(QStringLiteral("civ")).toBool();
                const QJsonArray labs = o.value(QStringLiteral("labels")).toArray();
                for (const QJsonValue &lab : labs)
                {
                    const QString s = lab.toString().trimmed();
                    if (!s.isEmpty() && s.size() <= kTagCols)
                        d.labels.append(s);
                }
                if (!d.id.isEmpty())
                    table.append(d);
            }
        }
        loaded = true;
    }
    return table;
}

TagWorld makeTagWorld(const Planet &planet)
{
    TagWorld world;
    world.scan = scanSurface(planet);
    const QVector<PlanetOre> ores = planetOreInventory(planet);
    world.wealth = oreWealthRank(ores);
    world.waterRank = fracToRank5(double(planet.water_pixel_count) / double(world.scan.pixels));
    world.iceRank = fracToRank5(double(planet.ice_pixel_count) / double(world.scan.pixels));
    world.floraRank = floraRank(planet);
    const int land = qMax(1, world.scan.pixels - planet.water_pixel_count);
    world.rugged = double(world.scan.mountain) / double(land) >= kRuggedLandShare;
    markOreQuality(ores, world.valuable, world.hazard);
    world.emotion = planetEmotionId(planet, world.scan);
    return world;
}

int tagIdRank(const QString &id, const QLatin1String &prefix)
{
    bool ok = false;
    const int n = id.mid(prefix.size()).toInt(&ok);
    return ok ? n : -1;
}

bool tagApplies(const Planet &p, const PlanetTagDef &d, const TagWorld &w)
{
    const QString &id = d.id;
    if (id.startsWith(QLatin1String("res_")))
        return w.wealth == tagIdRank(id, QLatin1String("res_"));
    if (id.startsWith(QLatin1String("rad_")))
        return rank5(p.facts.radiation) == tagIdRank(id, QLatin1String("rad_"));
    if (id.startsWith(QLatin1String("wat_")))
        return w.waterRank == tagIdRank(id, QLatin1String("wat_"));
    if (id.startsWith(QLatin1String("ice_")))
        return w.iceRank == tagIdRank(id, QLatin1String("ice_"));
    if (id.startsWith(QLatin1String("life_")))
        return w.floraRank == tagIdRank(id, QLatin1String("life_"));
    if (id.startsWith(QLatin1String("tmp_")))
        return rank5(p.facts.temperature) == tagIdRank(id, QLatin1String("tmp_"));
    if (id.startsWith(QLatin1String("sei_")))
        return rank5(p.facts.seismicity) == tagIdRank(id, QLatin1String("sei_"));
    if (id.startsWith(QLatin1String("lava_")))
        return lavaRank(w.scan) == tagIdRank(id, QLatin1String("lava_"));
    if (id.startsWith(QLatin1String("cloud_")))
        return cloudRank(p, w.scan) == tagIdRank(id, QLatin1String("cloud_"));
    if (id == QLatin1String("civ_yes"))
        return !p.cities.isEmpty();
    if (id == QLatin1String("toxic_flora"))
        return p.plant_pixel_count > 0 && p.s.hazardLight() >= 0.45;
    if (id == QLatin1String("rifts"))
        return double(w.scan.rifts) / double(w.scan.pixels) >= 0.01;
    if (id == QLatin1String("rugged"))
        return w.rugged;
    if (id == QLatin1String("habitable"))
        return rank5(p.facts.temperature) == 2
            && double(p.water_pixel_count) / double(w.scan.pixels) >= 5.0 / 12.0
            && p.facts.radiation <= 4 && p.plant_pixel_count > 0;
    if (id == QLatin1String("ice_ocean"))
        return double(p.water_pixel_count) / double(w.scan.pixels) >= 8.0 / 12.0
            && double(p.ice_pixel_count) / double(w.scan.pixels) >= 8.0 / 12.0;
    if (id == QLatin1String("dark_flora"))
        return !p.s.has_star && p.plant_pixel_count > 0;
    if (id == QLatin1String("danger"))
        return p.facts.radiation >= 8 || p.facts.seismicity >= 9 || lavaRank(w.scan) >= 3
            || (p.facts.life >= 3 && p.facts.radiation >= 6);
    if (id == QLatin1String("anomaly"))
    {
        const int hi = starBand(p.s.star_spectrum, StarGamma)
                       + starBand(p.s.star_spectrum, StarXray);
        const int radio = starBand(p.s.star_spectrum, StarRadio);
        return hi >= 16 || radio >= 9 || (!p.s.has_star && p.plant_pixel_count > 0);
    }
    if (id == QLatin1String("xrays"))
        return p.s.has_star && !isStarBlackHole(p.s.star_spectrum)
            && starBand(p.s.star_spectrum, StarGamma) + starBand(p.s.star_spectrum, StarXray) >= 14;
    if (id == QLatin1String("radio_loud"))
        return p.s.has_star && !isStarBlackHole(p.s.star_spectrum)
            && starBand(p.s.star_spectrum, StarRadio) >= 8;
    if (id == QLatin1String("dim_star"))
        return p.s.has_star && !isStarBlackHole(p.s.star_spectrum) && p.s.visibleLight() < 0.28;
    if (id == QLatin1String("star_blackhole"))
        return p.s.has_star && isStarBlackHole(p.s.star_spectrum);
    if (id.startsWith(QLatin1String("star_")))
    {
        if (!p.s.has_star || p.starclass.isEmpty() || isStarBlackHole(p.s.star_spectrum))
            return false;
        const int c = p.starclass.first();
        if (id == QLatin1String("star_blue"))
            return c >= 0 && c <= 2;
        if (id == QLatin1String("star_yellow"))
            return c >= 3 && c <= 5;
        if (id == QLatin1String("star_red"))
            return c == 6;
        if (id == QLatin1String("star_brown"))
            return c >= 9 && c <= 11;
        return false;
    }
    if (id == QLatin1String("rings"))
        return p.s.is_ring;
    if (id == QLatin1String("atmo"))
        return p.s.is_atmo && p.s.atmo_size > 3 && p.s.atmo_size < 8;
    if (id == QLatin1String("airless"))
        return !p.s.is_atmo;
    if (id == QLatin1String("thick_atmo"))
        return p.s.is_atmo && p.s.atmo_size >= 8;
    if (id == QLatin1String("thin_atmo"))
        return p.s.is_atmo && p.s.atmo_size <= 3;
    if (id == QLatin1String("has_star"))
        return p.s.has_star;
    if (id == QLatin1String("no_star"))
        return !p.s.has_star;
    if (id == QLatin1String("valuable_ores"))
        return w.valuable;
    if (id == QLatin1String("hazard_ores"))
        return w.hazard;
    if (id.startsWith(QLatin1String("emo_")))
        return !w.emotion.isEmpty() && w.emotion == id.mid(4);
    return false;
}

QChar radioNoiseChar(QRandomGenerator &rnd)
{
    static const char kNoise[] = {'@', '#', '$', '%', '*'};
    return QLatin1Char(kNoise[rnd.bounded(5)]);
}

QString jamCivEasterText(QString text, int radio, QRandomGenerator &rnd)
{
    if (text.isEmpty())
        return text;
    const int n = qBound(0, radio, kStarBandMax);
    if (n <= 0)
        return text;
    const int room = kTagCols - text.size();
    const int toInsert = qMin(n, qMax(0, room));
    for (int i = 0; i < toInsert; ++i)
        text.insert(rnd.bounded(text.size() + 1), radioNoiseChar(rnd));
    return text;
}

QVector<PlacedTag> packTags(const QVector<QPair<QString, QColor>> &picked)
{
    QVector<PlacedTag> out;
    int col = 0;
    int row = 0;
    for (const auto &item : picked)
    {
        const int tw = item.first.size();
        if (tw <= 0 || tw > kTagCols)
            continue;
        if (col > 0 && col + 1 + tw > kTagCols)
        {
            col = 0;
            ++row;
        }
        if (row >= kTagRows)
            break;
        PlacedTag t;
        t.text = item.first;
        t.color = item.second;
        t.col = col;
        t.row = row;
        out.append(t);
        col += tw;
        if (col < kTagCols)
            ++col;
        else
        {
            col = 0;
            ++row;
        }
    }
    return out;
}

}

QSet<QString> planetActiveTagIds(const Planet &planet)
{
    const TagWorld world = makeTagWorld(planet);
    QSet<QString> ids;
    for (const PlanetTagDef &d : tagCatalog())
    {
        if (tagApplies(planet, d, world))
            ids.insert(d.id);
    }
    return ids;
}

void planetPaintTagCard(Planet &planet)
{
    planet.img_sys = planetCachedImage(QStringLiteral(":/images/res/images/window.png")).copy();
    planet.cardTagIds.clear();
    planet.cardLabelKeys.clear();
    const TagWorld world = makeTagWorld(planet);
    const QVector<PlanetOre> ores = planetOreInventory(planet);
    QVector<PlanetTagDef> chosen;
    QSet<QString> families;
    const QVector<PlanetTagDef> &all = tagCatalog();
    QVector<PlanetTagDef> regular;
    QVector<PlanetTagDef> easter;
    QVector<PlanetTagDef> easterCiv;
    for (const PlanetTagDef &d : all)
    {
        if (d.easter && d.civ)
            easterCiv.append(d);
        else if (d.easter)
            easter.append(d);
        else
            regular.append(d);
    }
    std::sort(regular.begin(), regular.end(), [](const PlanetTagDef &a, const PlanetTagDef &b) {
        if (a.priority != b.priority)
            return a.priority < b.priority;
        return a.id < b.id;
    });
    for (const PlanetTagDef &d : regular)
    {
        if (!d.family.isEmpty() && families.contains(d.family))
            continue;
        if (!tagApplies(planet, d, world))
            continue;
        if (!d.family.isEmpty())
            families.insert(d.family);
        chosen.append(d);
    }
    if (!ores.isEmpty())
    {
        qint64 total = 0;
        for (const PlanetOre &ore : ores)
            total += ore.area;
        const int n = qMin(2, ores.size());
        for (int i = 0; i < n && total > 0; ++i)
        {
            const double share = double(ores[i].area) / double(total);
            int kind = 2;
            if (share >= 0.28)
                kind = 0;
            else if (share >= 0.12)
                kind = 1;
            const QString text = orePhrase(ores[i].symbol, kind, planet.rnd);
            if (text.isEmpty() || text.size() > kTagCols)
                continue;
            PlanetTagDef d;
            d.id = QLatin1String("oreq_") + ores[i].symbol;
            d.priority = 16 + i;
            if (ores[i].radioactive)
                d.tone = share >= 0.12 ? QStringLiteral("bad") : QStringLiteral("caution");
            else if (kind == 0)
                d.tone = QStringLiteral("good");
            else
                d.tone = QStringLiteral("info");
            d.labels.append(text);
            chosen.append(d);
        }
    }
    if (!easter.isEmpty() && planet.rnd.bounded(100) < 5)
    {
        const PlanetTagDef &egg = easter[planet.rnd.bounded(easter.size())];
        chosen.append(egg);
    }
    if (!planet.cities.isEmpty() && !easterCiv.isEmpty() && planet.rnd.bounded(100) < 5)
    {
        const PlanetTagDef &egg = easterCiv[planet.rnd.bounded(easterCiv.size())];
        chosen.append(egg);
    }
    std::sort(chosen.begin(), chosen.end(), [](const PlanetTagDef &a, const PlanetTagDef &b) {
        if (a.priority != b.priority)
            return a.priority < b.priority;
        return a.id < b.id;
    });

    QVector<QPair<QString, QColor>> picked;
    for (const PlanetTagDef &d : chosen)
    {
        if (d.labels.isEmpty())
            continue;
        QString text;
        QString labelKey;
        if (d.id.startsWith(QLatin1String("oreq_")))
        {
            text = d.labels.first();
            labelKey = text;
        }
        else
        {
            const int i = planet.rnd.bounded(d.labels.size());
            labelKey = d.labels[i];
            const QByteArray key = labelKey.toUtf8();
            text = QCoreApplication::translate("PlanetTags", key.constData());
        }
        if (d.civ)
        {
            const int radio = planet.s.has_star
                                  ? starBand(planet.s.star_spectrum, StarRadio)
                                  : 0;
            text = jamCivEasterText(text, radio, planet.rnd);
        }
        if (text.size() > kTagCols)
            continue;
        planet.cardTagIds.insert(d.id);
        if (!labelKey.isEmpty())
            planet.cardLabelKeys.insert(labelKey);
        picked.append(qMakePair(text, toneColor(d.tone)));
    }
    const QVector<PlacedTag> placed = packTags(picked);

    QPainter p;
    p.begin(&planet.img_sys);
    QFont font(QStringLiteral("Consolas"), 8);
    p.setFont(font);
    const QFontMetrics fm(font);
    const int x0 = 40;
    const int y0 = 27;
    const int lh = fm.lineSpacing();
    const int cw = qMax(1, fm.averageCharWidth());
    for (const PlacedTag &t : placed)
    {
        const QRect box(x0 + t.col * cw, y0 + t.row * lh, t.text.size() * cw, lh);
        QColor fill = t.color;
        fill.setAlpha(70);
        p.fillRect(box.adjusted(0, 1, 0, -1), fill);
        p.setPen(t.color);
        p.drawText(box, Qt::AlignLeft | Qt::AlignVCenter, t.text);
    }
    p.end();
}
