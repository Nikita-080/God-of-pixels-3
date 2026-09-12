#include <QtTest>
#include "planet.h"
#include "planetsettings.h"
#include "spheremath.h"
#include "terrafactory.h"
#include <QJsonObject>

static PlanetSettings testSettings()
{
    PlanetSettings s;
    s.terramode = 0;
    s.randomness = 4;
    s.iterations = 40;
    s.world_size = 5;
    s.temperature = 15;
    s.structure = {40, 95, 150, 205, 260, 315, 370, 425};
    s.ice_color = QColor("#464646");
    s.rock_color = QColor("#808080");
    s.mountain_color = QColor("#b4b4b4");
    s.plain_color = QColor("#b4b480");
    s.beach_color = QColor("#fcdd76");
    s.shallow_color = QColor("#2828ff");
    s.ocean_color = QColor("#000080");
    s.noise = 0;
    s.is_gradient = false;
    s.is_plant = false;
    s.is_civ = false;
    s.civ_color = QColor("#ffcc66");
    s.shine = 5;
    s.shine_lat = 25;
    s.shine_lon = 90;
    s.is_fill_light = true;
    s.polar_lat = 90;
    s.polar_lon = 0;
    s.name_algorithm = 3;
    s.is_cloud = true;
    s.cloud_size = 2;
    s.cloud_quality = 2;
    s.cloud_transparent = 0;
    s.correction = true;
    s.cloud_color = QColor("#d9d9d9");
    s.is_atmo = false;
    s.atmo_transparent = 5;
    s.atmo_size = 5;
    s.atmo_color = QColor("#b7edfd");
    s.is_ring = false;
    s.R_internal_ring = 3;
    s.R_external_ring = 3;
    s.ring_color = QColor("#e6b060");
    s.ring_material = 0;
    s.ring_intensity = 2;
    s.rebuildDerived();
    return s;
}

class TestPlanet : public QObject
{
    Q_OBJECT
private slots:
    void transparentColorLerp();
    void jsonRoundTrip();
    void oldDiamondSquareModeBecomesNoise();
    void sameSeedSameHeight();
    void noiseHasNoMeridianSeam();
    void sphericalFaultFinite();
};

void TestPlanet::transparentColorLerp()
{
    Planet p;
    const QColor c = p.TransparentColor(QColor(0, 0, 0), QColor(100, 50, 0), 0.5);
    QCOMPARE(c, QColor(50, 25, 0));
}

void TestPlanet::jsonRoundTrip()
{
    PlanetSettings a = testSettings();
    PlanetSettings b;
    QVERIFY(b.JSON_deserialize(a.JSON_serialize()));
    QCOMPARE(b.terramode, a.terramode);
    QCOMPARE(b.world_size, a.world_size);
    QCOMPARE(b.structure, a.structure);
    QCOMPARE(b.true_structure.size(), a.true_structure.size());
    QCOMPARE(b.ice_color, a.ice_color);
    QCOMPARE(b.shine_lat, a.shine_lat);
    QCOMPARE(b.shine_lon, a.shine_lon);
    QCOMPARE(b.is_fill_light, a.is_fill_light);
    QCOMPARE(b.seismicity, a.seismicity);
    QCOMPARE(b.polar_lat, a.polar_lat);
    QCOMPARE(b.polar_lon, a.polar_lon);
    QCOMPARE(a.JSON_serialize()["version"].toInt(), 2);
}

void TestPlanet::oldDiamondSquareModeBecomesNoise()
{
    PlanetSettings s = testSettings();
    QJsonObject obj = s.JSON_serialize();
    obj["terramode"] = 0;
    PlanetSettings loaded;
    QVERIFY(loaded.JSON_deserialize(obj));
    QCOMPARE(loaded.terramode, 0);
}

void TestPlanet::sameSeedSameHeight()
{
    Planet a;
    Planet b;
    a.s = testSettings();
    b.s = testSettings();
    a.SetSeed(4242);
    b.SetSeed(4242);
    a.Generate();
    b.Generate();
    QCOMPARE(a.map_w, b.map_w);
    QCOMPARE(a.map_h, b.map_h);
    QCOMPARE(a.map_w, 2 * a.map_h);
    QCOMPARE(a.viewResolution(), 112);
    QVERIFY(a.map_w > 1);
    for (int x = 0; x < a.map_w; ++x)
        for (int y = 0; y < a.map_h; ++y)
            QCOMPARE(a.matrix[x][y], b.matrix[x][y]);
}

void TestPlanet::noiseHasNoMeridianSeam()
{
    TerraFactory factory(64, 32, 99);
    const auto m = factory.sphericalNoise(4.0);
    double maxDiff = 0.0;
    for (int y = 0; y < 32; ++y)
        maxDiff = qMax(maxDiff, qAbs(m[0][y] - m[63][y]));
    QVERIFY(maxDiff < 0.45);
}

void TestPlanet::sphericalFaultFinite()
{
    TerraFactory factory(32, 16, 7);
    const auto m = factory.sphericalFault(25);
    QVERIFY(!m.isEmpty());
    QVERIFY(!qIsNaN(m[0][0]));
    QVERIFY(!qIsInf(m[16][8]));
}

QTEST_MAIN(TestPlanet)
#include "test_planet.moc"
