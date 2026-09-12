#include "planet.h"
#include "planet_p.h"
#include <QCoreApplication>
#include <QPainter>
#include <QtMath>

void Planet::GenerateDescription()
{
    facts.day = QString::number(RAND(5, 100));
    facts.year = QString::number(RAND(1, 50));
    facts.gravitation = QString::number(RAND(0, 2)) + "." + QString::number(RAND(0, 9));
    facts.resources = Resources();
    facts.radiation = RAND(0, 12);
    facts.seismicity = RAND(0, 12);
}

void Planet::CalculateDescription()
{
    int pixel_count = map_w * map_h;
    if (pixel_count <= 0)
        pixel_count = 1;
    facts.life = qRound(plant_pixel_count * 12.0 / qMax(1, pixel_count - water_pixel_count));
    facts.ice = qRound(ice_pixel_count * 12.0 / pixel_count);
    facts.water = qRound(water_pixel_count * 12.0 / pixel_count);
    facts.temperature = qRound((s.temperature + 90) * 12.0 / 230);
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
    QString classes = "OBAFGKMCSLTY";
    QString s = "";

    QPainter p;
    p.begin(&img_dsc);
    p.setPen(QPen(QColor(110, 170, 200)));
    p.setFont(QFont("Consolas", 8));

    s += QCoreApplication::translate("Planet", "name       - ") + name + "\n";

    if (starclass.isEmpty())
        s += QCoreApplication::translate("Planet", "day        - [not found]") + "\n";
    else
        s += QCoreApplication::translate("Planet", "day        - ") + facts.day + QCoreApplication::translate("Planet", "h") + "\n";

    if (starclass.isEmpty())
        s += QCoreApplication::translate("Planet", "year       - [not found]") + "\n";
    else
        s += QCoreApplication::translate("Planet", "year       - ") + facts.year + QCoreApplication::translate("Planet", "y") + "\n";

    s += QCoreApplication::translate("Planet", "gravity    - ") + facts.gravitation + "g\n";

    if (starclass.isEmpty())
        s += QCoreApplication::translate("Planet", "star       - [not found]") + "\n";
    else
    {
        s += QCoreApplication::translate("Planet", "star       - ");
        for (int i = 0; i < starclass.length(); i++)
        {
            s += classes[starclass[i]];
            s += ' ';
        }
        s += '\n';
    }

    s += QCoreApplication::translate("Planet", "resources  - ") + facts.resources;
    p.drawText(QRect(40, 27, 400, 400), s);
    Level(QCoreApplication::translate("Planet", "life         "), 8, facts.life, "good", p);
    Level(QCoreApplication::translate("Planet", "water        ", nullptr), 9, facts.water, "neutral", p);
    Level(QCoreApplication::translate("Planet", "ice          "), 10, facts.ice, "bad", p);
    Level(QCoreApplication::translate("Planet", "radiation    "), 11, facts.radiation, "bad", p);
    Level(QCoreApplication::translate("Planet", "temperature  "), 12, facts.temperature, "neutral", p);
    Level(QCoreApplication::translate("Planet", "seismicity   "), 13, facts.seismicity, "bad", p);
    p.end();
}

QString Planet::Resources()
{
    QVector<QString> mas = {"He", "Li", "Mg", "Al", "Si", "Cl", "Ar",
                            "Ca", "Ti", "Cr", "Fe", "Co", "Ni", "Cu",
                            "Zn", "As", "Ag", "Cd", "Sn", "Xe", "Cs",
                            "Nd", "Pt", "Au", "Hg", "Pb", "Rn", "Pu"};
    QString res = "";
    QVector<int> indexes = {-1};
    int x = RAND(0, 5);
    if (x == 0)
        return QCoreApplication::translate("Planet", "[not found]\n");
    for (int i = 0; i < x; i++)
    {
        int index = RAND(0, 14);
        while (indexes.contains(index))
            index = RAND(0, 14);
        indexes.append(index);
        if (i == x - 1)
            res += mas[index] + "\n";
        else
            res += mas[index] + " ";
    }
    return res;
}

bool Planet::Collis(int r_o, int r, const QVector<QVector<int>> &planets)
{
    for (int i = 0; i < planets.length(); i++)
    {
        bool f1 = planets[i][0] <= r_o - r and r_o - r <= planets[i][1];
        bool f2 = planets[i][0] <= r_o + r and r_o + r <= planets[i][1];
        bool f3 = r_o - r <= planets[i][0] and planets[i][0] <= r_o + r;
        bool f4 = r_o - r <= planets[i][1] and planets[i][1] <= r_o + r;
        if (f1 or f2 or f3 or f4)
            return false;
    }
    return true;
}

void Planet::SystemMap()
{
    if (starclass.length() == 0)
        SystemMap_0star();
    else if (starclass.length() == 1)
        SystemMap_1star();
    else
        SystemMap_2star();
}

void Planet::SystemMap_0star()
{
    img_sys = planetCachedImage(QStringLiteral(":/images/res/images/window.png"));
    QPainter p;
    p.begin(&img_sys);
    int r;
    r = RAND(4, 8);
    p.setPen(Qt::PenStyle::NoPen);
    p.setBrush(QBrush(QColor("#ff0000")));
    p.drawEllipse(165 - r, 165 - r, 2 * r, 2 * r);
    p.end();
}

void Planet::SystemMap_1star()
{
    img_sys = planetCachedImage(QStringLiteral(":/images/res/images/window.png"));
    QVector<QString> color_star = {"#E7ECFE", "#F5F7FF", "#FEFEFE", "#FFFBE5",
                                   "#FFF3BD", "#FFD48A", "#FFA38A", "#F7805F",
                                   "#EE4F3A", "#DF3C26", "#C53320", "#AF3627"};
    QPainter p;
    p.begin(&img_sys);

    int r_star = RAND(8, 25);

    p.setPen(Qt::PenStyle::NoPen);
    p.setBrush(QBrush(QColor(color_star[starclass[0]])));
    p.drawEllipse(165 - r_star, 165 - r_star, r_star * 2, r_star * 2);

    int r = RAND(4, 8);
    int r_o = qRound((140 - s.temperature) * (125 - r - r - r_star) * 1.0 / 230 + r + r_star);
    DrawPlanets(&p, 165, 165, r_star, 125, 4, 8, r_o, r);

    p.end();
}

void Planet::SystemMap_2star()
{
    img_sys = planetCachedImage(QStringLiteral(":/images/res/images/window.png"));
    QVector<QString> color_star = {"#E7ECFE", "#F5F7FF", "#FEFEFE", "#FFFBE5",
                                   "#FFF3BD", "#FFD48A", "#FFA38A", "#F7805F",
                                   "#EE4F3A", "#DF3C26", "#C53320", "#AF3627"};
    QPainter p;
    p.begin(&img_sys);

    double angle = rnd.bounded(6.283);
    int r_o_s = 27;
    int r_star = 8;
    int x_s1 = qRound(r_o_s * cos(angle)) + 165;
    int y_s1 = 165 - qRound(r_o_s * sin(angle));
    p.setPen(Qt::PenStyle::NoPen);
    p.setBrush(QBrush(QColor(color_star[starclass[0]])));
    p.drawEllipse(x_s1 - r_star, y_s1 - r_star, r_star * 2, r_star * 2);

    angle += 3.1415;
    int x_s2 = qRound(r_o_s * cos(angle)) + 165;
    int y_s2 = 165 - qRound(r_o_s * sin(angle));
    p.setBrush(QBrush(QColor(color_star[starclass[1]])));
    p.drawEllipse(x_s2 - r_star, y_s2 - r_star, r_star * 2, r_star * 2);

    int r_o = -1;
    int r_p = 4;
    int r_o_max = 27;
    if (s.temperature == -90)
        r_o = RAND(55 + r_p, 111 - r_p);
    DrawPlanets(&p, 165, 165, 55, 111, 4, 4, r_o, r_p);
    if (s.temperature == -90)
        r_o = -1;
    else
        r_o = qRound((140 - s.temperature) * (r_o_max - r_p - r_p - r_star) * 1.0 / 230 + r_p + r_star);
    DrawPlanets(&p, x_s1, y_s1, r_star, r_o_max, 4, 4, r_o, r_p);
    r_o = -1;
    DrawPlanets(&p, x_s2, y_s2, r_star, r_o_max, 4, 4, r_o, r_p);

    p.end();
}

void Planet::DrawPlanets(QPainter *p, int x, int y, int r_o_min, int r_o_max, int r_p_min, int r_p_max, int r_o, int r_p)
{
    int x_p, y_p;
    double angle;
    QVector<QVector<int>> planets;
    QVector<int> vec;
    if (r_o != -1)
    {
        double angle = rnd.bounded(6.283);
        int x_p = qRound(r_o * cos(angle)) + x;
        int y_p = y - qRound(r_o * sin(angle));
        vec = {r_o - r_p, r_o + r_p};
        planets.append(vec);
        p->setPen(QPen(QColor("#f2e8c9"), 2));
        p->setBrush(Qt::BrushStyle::NoBrush);
        p->drawEllipse(x - r_o, y - r_o, 2 * r_o, 2 * r_o);

        p->setPen(Qt::PenStyle::NoPen);
        p->setBrush(QBrush(QColor("#ff0000")));
        p->drawEllipse(x_p - r_p, y_p - r_p, 2 * r_p, 2 * r_p);
    }

    int num_planet = RAND(0, 9);
    for (int i = 0; i < num_planet; i++)
    {
        x_p = 0;
        y_p = 0;
        r_p = 0;
        r_o = 0;
        int count = 0;
        while (r_o + r_p > r_o_max or r_o - r_p < r_o_min or !Collis(r_o, r_p, planets))
        {
            r_p = RAND(r_p_min, r_p_max);
            r_o = RAND(r_o_min, r_o_max);
            count++;
            if (count > 15)
                break;
        }
        if (count > 15)
            break;
        angle = rnd.bounded(6.283);
        x_p = qRound(r_o * cos(angle)) + x;
        y_p = y - qRound(r_o * sin(angle));
        vec = {r_o - r_p, r_o + r_p};
        planets.append(vec);

        p->setPen(QPen(QColor("#f2e8c9"), 2));
        p->setBrush(Qt::BrushStyle::NoBrush);
        p->drawEllipse(x - r_o, y - r_o, 2 * r_o, 2 * r_o);

        p->setPen(Qt::PenStyle::NoPen);
        p->setBrush(QBrush(QColor("#808080")));
        p->drawEllipse(x_p - r_p, y_p - r_p, 2 * r_p, 2 * r_p);
    }
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
