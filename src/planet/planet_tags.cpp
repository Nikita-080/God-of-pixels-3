#include "planet_p.h"
#include "planet.h"
#include "starspectrum.h"
#include <QCoreApplication>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
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

struct PlanetTagDef
{
    QString id;
    QString family;
    int priority;
    QString tone;
    bool easter;
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
    double relief = 0.0;
};

SurfaceScan scanSurface(const Planet &p)
{
    SurfaceScan s;
    const int w = p.map_w;
    const int h = p.map_h;
    s.pixels = qMax(1, w * h);
    if (w <= 0 || h <= 0)
        return s;
    double mn = 1e100;
    double mx = -1e100;
    const bool hasLava = !p.lavaHeat.isEmpty() && p.lavaHeat.size() == w;
    const bool hasFault = !p.faultKind.isEmpty() && p.faultKind.size() == w;
    const bool hasCloud = p.s.is_cloud && !p.c_map.isEmpty() && p.c_map.size() == w;
    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            if (x < p.matrix.size() && y < p.matrix[x].size())
            {
                const double z = p.matrix[x][y];
                mn = qMin(mn, z);
                mx = qMax(mx, z);
            }
            if (hasLava && y < p.lavaHeat[x].size() && p.lavaHeat[x][y] > 0.2)
                ++s.lava;
            if (hasFault && y < p.faultKind[x].size() && p.faultKind[x][y] == 1)
                ++s.rifts;
            if (hasCloud && y < p.c_map[x].size() && p.c_map[x][y] > 0.45)
                ++s.cloud;
        }
    }
    if (mx > mn)
        s.relief = mx - mn;
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

int floraRank(const Planet &p)
{
    if (p.plant_pixel_count <= 0)
        return 0;
    const int pix = qMax(1, p.map_w * p.map_h);
    const double ofMap = double(p.plant_pixel_count) / double(pix);
    if (ofMap < 0.03)
        return 1;
    if (ofMap < 0.10)
        return 2;
    if (ofMap < 0.22)
        return 3;
    return 4;
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

struct MetalLex
{
    const char *sym;
    const char *en;
    const char *ruGen;
    const char *ruAdj;
};

const MetalLex *metalLex(const QString &symbol)
{
    static const MetalLex kTable[] = {
        {"Li", "lithium", "лития", "литиевые"},
        {"Be", "beryllium", "бериллия", "бериллиевые"},
        {"Na", "sodium", "натрия", "натриевые"},
        {"Mg", "magnesium", "магния", "магниевые"},
        {"Al", "aluminium", "алюминия", "алюминиевые"},
        {"K", "potassium", "калия", "калиевые"},
        {"Ca", "calcium", "кальция", "кальциевые"},
        {"Sc", "scandium", "скандия", "скандиевые"},
        {"Ti", "titanium", "титана", "титановые"},
        {"V", "vanadium", "ванадия", "ванадиевые"},
        {"Cr", "chromium", "хрома", "хромовые"},
        {"Mn", "manganese", "марганца", "марганцевые"},
        {"Fe", "iron", "железа", "железные"},
        {"Co", "cobalt", "кобальта", "кобальтовые"},
        {"Ni", "nickel", "никеля", "никелевые"},
        {"Cu", "copper", "меди", "медные"},
        {"Zn", "zinc", "цинка", "цинковые"},
        {"Ga", "gallium", "галлия", "галлиевые"},
        {"Rb", "rubidium", "рубидия", "рубидиевые"},
        {"Sr", "strontium", "стронция", "стронциевые"},
        {"Y", "yttrium", "иттрия", "иттриевые"},
        {"Zr", "zirconium", "циркония", "циркониевые"},
        {"Nb", "niobium", "ниобия", "ниобиевые"},
        {"Mo", "molybdenum", "молибдена", "молибденовые"},
        {"Ru", "ruthenium", "рутения", "рутениевые"},
        {"Rh", "rhodium", "родия", "родиевые"},
        {"Pd", "palladium", "палладия", "палладиевые"},
        {"Ag", "silver", "серебра", "серебряные"},
        {"Cd", "cadmium", "кадмия", "кадмиевые"},
        {"In", "indium", "индия", "индиевые"},
        {"Sn", "tin", "олова", "оловянные"},
        {"Cs", "caesium", "цезия", "цезиевые"},
        {"Ba", "barium", "бария", "бариевые"},
        {"La", "lanthanum", "лантана", "лантановые"},
        {"Ce", "cerium", "церия", "цериевые"},
        {"Pr", "praseodymium", "празеодима", "празеодимовые"},
        {"Nd", "neodymium", "неодима", "неодимовые"},
        {"Sm", "samarium", "самария", "самариевые"},
        {"Eu", "europium", "европия", "европиевые"},
        {"Gd", "gadolinium", "гадолиния", "гадолиниевые"},
        {"Tb", "terbium", "тербия", "тербиевые"},
        {"Dy", "dysprosium", "диспрозия", "диспрозиевые"},
        {"Ho", "holmium", "гольмия", "гольмиевые"},
        {"Er", "erbium", "эрбия", "эрбиевые"},
        {"Tm", "thulium", "тулия", "тулиевые"},
        {"Yb", "ytterbium", "иттербия", "иттербиевые"},
        {"Lu", "lutetium", "лютеция", "лютециевые"},
        {"Hf", "hafnium", "гафния", "гафниевые"},
        {"Ta", "tantalum", "тантала", "танталовые"},
        {"W", "tungsten", "вольфрама", "вольфрамовые"},
        {"Re", "rhenium", "рения", "рениевые"},
        {"Os", "osmium", "осмия", "осмиевые"},
        {"Ir", "iridium", "иридия", "иридиевые"},
        {"Pt", "platinum", "платины", "платиновые"},
        {"Au", "gold", "золота", "золотые"},
        {"Hg", "mercury", "ртути", "ртутные"},
        {"Tl", "thallium", "таллия", "таллиевые"},
        {"Pb", "lead", "свинца", "свинцовые"},
        {"Bi", "bismuth", "висмута", "висмутовые"},
        {"Th", "thorium", "тория", "ториевые"},
        {"U", "uranium", "урана", "урановые"},
        {nullptr, nullptr, nullptr, nullptr}
    };
    for (int i = 0; kTable[i].sym; ++i)
    {
        if (symbol == QLatin1String(kTable[i].sym))
            return &kTable[i];
    }
    return nullptr;
}

QString orePhrase(const QString &symbol, int kind, QRandomGenerator &rnd)
{
    const MetalLex *m = metalLex(symbol);
    const bool ru = QCoreApplication::translate("PlanetTags", "rings")
                    == QString::fromUtf8("кольца");
    const QString en = m ? QString::fromUtf8(m->en) : symbol;
    const QString gen = (ru && m) ? QString::fromUtf8(m->ruGen) : en;
    const QString adj = (ru && m) ? QString::fromUtf8(m->ruAdj) : en;
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
                const QJsonArray labs = o.value(QStringLiteral("labels")).toArray();
                for (const QJsonValue &lab : labs)
                {
                    const QString s = lab.toString().trimmed();
                    if (!s.isEmpty() && s.size() <= kTagCols)
                        d.labels.append(s);
                }
                if (!d.id.isEmpty() && !d.labels.isEmpty())
                    table.append(d);
            }
        }
        loaded = true;
    }
    return table;
}

bool tagApplies(const Planet &p, const PlanetTagDef &d, const SurfaceScan &scan, int wealth)
{
    const QString &id = d.id;
    if (id.startsWith(QLatin1String("res_")))
        return wealth == id.mid(4).toInt();
    if (id.startsWith(QLatin1String("rad_")))
        return rank5(p.facts.radiation) == id.mid(4).toInt();
    if (id.startsWith(QLatin1String("wat_")))
        return rank5(p.facts.water) == id.mid(4).toInt();
    if (id.startsWith(QLatin1String("ice_")))
        return rank5(p.facts.ice) == id.mid(4).toInt();
    if (id.startsWith(QLatin1String("life_")))
        return floraRank(p) == id.mid(4).toInt();
    if (id.startsWith(QLatin1String("tmp_")))
        return rank5(p.facts.temperature) == id.mid(4).toInt();
    if (id.startsWith(QLatin1String("sei_")))
        return rank5(p.facts.seismicity) == id.mid(4).toInt();
    if (id.startsWith(QLatin1String("lava_")))
        return lavaRank(scan) == id.mid(5).toInt();
    if (id.startsWith(QLatin1String("cloud_")))
        return cloudRank(p, scan) == id.mid(6).toInt();
    if (id == QLatin1String("civ_yes"))
        return !p.cities.isEmpty();
    if (id == QLatin1String("toxic_flora"))
        return p.plant_pixel_count > 0 && p.s.hazardLight() >= 0.45;
    if (id == QLatin1String("rifts"))
        return double(scan.rifts) / double(scan.pixels) >= 0.01;
    if (id == QLatin1String("rugged"))
    {
        if (p.s.true_structure.size() < 2)
            return false;
        const double span = qAbs(p.s.true_structure.first() - p.s.true_structure.last());
        return span >= 1.0 && scan.relief / span >= 0.78;
    }
    if (id == QLatin1String("habitable"))
        return rank5(p.facts.temperature) == 2 && p.facts.water >= 5
            && p.facts.radiation <= 4 && p.plant_pixel_count > 0;
    if (id == QLatin1String("ice_ocean"))
        return p.facts.water >= 8 && p.facts.ice >= 8;
    if (id == QLatin1String("dark_flora"))
        return !p.s.has_star && p.plant_pixel_count > 0;
    if (id == QLatin1String("danger"))
        return p.facts.radiation >= 8 || p.facts.seismicity >= 9 || lavaRank(scan) >= 3
            || (p.facts.life >= 3 && p.facts.radiation >= 6);
    if (id == QLatin1String("anomaly"))
    {
        const int hi = starBand(p.s.star_spectrum, StarGamma)
                       + starBand(p.s.star_spectrum, StarXray);
        const int radio = starBand(p.s.star_spectrum, StarRadio);
        return hi >= 16 || radio >= 9 || (!p.s.has_star && p.plant_pixel_count > 0);
    }
    if (id == QLatin1String("xrays"))
        return p.s.has_star
            && starBand(p.s.star_spectrum, StarGamma) + starBand(p.s.star_spectrum, StarXray) >= 14;
    if (id == QLatin1String("radio_loud"))
        return p.s.has_star && starBand(p.s.star_spectrum, StarRadio) >= 8;
    if (id == QLatin1String("dim_star"))
        return p.s.has_star && p.s.visibleLight() < 0.28;
    if (id.startsWith(QLatin1String("star_")))
    {
        if (!p.s.has_star || p.starclass.isEmpty())
            return false;
        const int c = p.starclass.first();
        if (id == QLatin1String("star_blue"))
            return c >= 0 && c <= 2;
        if (id == QLatin1String("star_yellow"))
            return c >= 3 && c <= 5;
        if (id == QLatin1String("star_red"))
            return c == 6;
        if (id == QLatin1String("star_brown"))
            return c >= 9;
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
    return false;
}

QVector<PlacedTag> packTags(const QVector<QPair<QString, QColor>> &picked)
{
    QVector<PlacedTag> out;
    int col = 0;
    int row = 0;
    for (const auto &item : picked)
    {
        const int w = item.first.size();
        if (w <= 0 || w > kTagCols)
            continue;
        if (col > 0 && col + 1 + w > kTagCols)
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
        col += w;
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

void planetPaintTagCard(Planet &planet)
{
    planet.img_sys = planetCachedImage(QStringLiteral(":/images/res/images/window.png")).copy();
    const SurfaceScan scan = scanSurface(planet);
    const QVector<PlanetOre> ores = planetOreInventory(planet);
    const int wealth = oreWealthRank(ores);
    QVector<PlanetTagDef> chosen;
    QSet<QString> families;
    const QVector<PlanetTagDef> &all = tagCatalog();
    QVector<PlanetTagDef> regular;
    QVector<PlanetTagDef> easter;
    for (const PlanetTagDef &d : all)
    {
        if (d.easter)
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
        if (!tagApplies(planet, d, scan, wealth))
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
    if (!easter.isEmpty() && planet.rnd.bounded(100) < 2)
    {
        const PlanetTagDef &egg = easter[planet.rnd.bounded(easter.size())];
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
        if (d.id.startsWith(QLatin1String("oreq_")))
            text = d.labels.first();
        else
        {
            const int i = planet.rnd.bounded(d.labels.size());
            text = QCoreApplication::translate("PlanetTags", d.labels[i].toUtf8().constData());
        }
        if (text.size() > kTagCols)
            continue;
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
