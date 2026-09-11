#include <QCoreApplication>
#include "settingspanel.h"
#include "multislider.h"
#include "colorswatch.h"
#include <QTabWidget>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QFont>
#include <QSizePolicy>

template <typename T>
T *SettingsPanel::child(const char *name) const
{
    return tabs->findChild<T *>(QLatin1String(name));
}

SettingsPanel::SettingsPanel(QTabWidget *tabs, QWidget *parent)
    : QWidget(parent)
    , tabs(tabs)
    , updating(false)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tabs);
    setFixedWidth(491);
    setMinimumHeight(571);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    tabs->setMinimumSize(491, 571);
    tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    for (int i = 0; i < tabs->count(); ++i)
    {
        QWidget *page = tabs->widget(i);
        if (!page)
            continue;
        const QRect bounds = page->childrenRect();
        page->setMinimumSize(qMax(bounds.right() + 12, 380),
                             qMax(bounds.bottom() + 12, 500));
    }

    tabs->setStyleSheet(QString());
    for (QSlider *slider : tabs->findChildren<QSlider *>())
        slider->setStyleSheet(QString());
    for (QComboBox *box : tabs->findChildren<QComboBox *>())
        box->setStyleSheet(QString());
    if (auto *debugA = child<QPushButton>("pushButton_6"))
        debugA->hide();
    if (auto *debugB = child<QPushButton>("pushButton_7"))
        debugB->hide();

    ms = new MultiSlider(child<QWidget>("tab_2"));
    ms->setGeometry(8, 28, 210, 520);
    ms->show();
    ms->raise();

    auto addLatLon = [this](QWidget *parent, QSlider *&latSlider, QSlider *&lonSlider,
                            QLabel *&latTitle, QLabel *&lonTitle) {
        auto makeTitle = [parent](int y) {
            QLabel *label = new QLabel(parent);
            label->setGeometry(20, y, 200, 24);
            label->setFont(QFont(QStringLiteral("Consolas"), 10));
            return label;
        };
        auto makeValue = [parent](int y) {
            QLabel *label = new QLabel(QStringLiteral("0"), parent);
            label->setGeometry(230, y, 41, 21);
            label->setFont(QFont(QStringLiteral("Consolas"), 10));
            label->setAlignment(Qt::AlignCenter);
            return label;
        };
        auto makeSlider = [parent](int y, int min, int max, int value) {
            QSlider *slider = new QSlider(Qt::Horizontal, parent);
            slider->setGeometry(20, y, 221, 16);
            slider->setRange(min, max);
            slider->setValue(value);
            return slider;
        };
        latTitle = makeTitle(110);
        latSlider = makeSlider(140, -90, 90, 0);
        QLabel *latValue = makeValue(135);
        lonTitle = makeTitle(180);
        lonSlider = makeSlider(210, -180, 180, 0);
        QLabel *lonValue = makeValue(205);
        latValue->setText(QString::number(latSlider->value()));
        lonValue->setText(QString::number(lonSlider->value()));
        connect(latSlider, &QSlider::valueChanged, latValue, QOverload<int>::of(&QLabel::setNum));
        connect(lonSlider, &QSlider::valueChanged, lonValue, QOverload<int>::of(&QLabel::setNum));
    };

    addLatLon(child<QWidget>("tab_5"), sliderShineLat, sliderShineLon,
              labelShineLatTitle, labelShineLonTitle);
    addLatLon(child<QWidget>("tab_10"), sliderPolarLat, sliderPolarLon,
              labelPolarLatTitle, labelPolarLonTitle);

    checkFillLight = new QCheckBox(child<QWidget>("tab_5"));
    checkFillLight->setObjectName(QStringLiteral("checkFillLight"));
    checkFillLight->setGeometry(10, 250, 260, 31);
    checkFillLight->setFont(QFont(QStringLiteral("Consolas"), 10));
    checkFillLight->setChecked(true);

    auto *lifeTab = child<QWidget>("tab_4");
    checkCiv = new QCheckBox(lifeTab);
    checkCiv->setObjectName(QStringLiteral("checkCiv"));
    checkCiv->setGeometry(10, 84, 241, 22);
    checkCiv->setFont(QFont(QStringLiteral("Consolas"), 10));
    btnColorCiv = new QPushButton(lifeTab);
    btnColorCiv->setObjectName(QStringLiteral("btnColorCiv"));
    btnColorCiv->setGeometry(10, 116, 141, 51);
    btnColorCiv->setFont(QFont(QStringLiteral("Consolas"), 10));
    ColorSwatch::setColor(btnColorCiv, QColor(QStringLiteral("#ffcc66")));

    auto *ringTab = child<QWidget>("tab_9");
    labelRingMaterial = new QLabel(ringTab);
    labelRingMaterial->setGeometry(10, 250, 240, 24);
    labelRingMaterial->setFont(QFont(QStringLiteral("Consolas"), 10));
    radioRingGas = new QRadioButton(ringTab);
    radioRingGas->setObjectName(QStringLiteral("radioRingGas"));
    radioRingGas->setGeometry(10, 274, 260, 28);
    radioRingGas->setFont(QFont(QStringLiteral("Consolas"), 10));
    radioRingGas->setChecked(true);
    radioRingMeteor = new QRadioButton(ringTab);
    radioRingMeteor->setObjectName(QStringLiteral("radioRingMeteor"));
    radioRingMeteor->setGeometry(10, 302, 260, 28);
    radioRingMeteor->setFont(QFont(QStringLiteral("Consolas"), 10));
    auto *ringGroup = new QButtonGroup(this);
    ringGroup->addButton(radioRingGas, 0);
    ringGroup->addButton(radioRingMeteor, 1);
    labelRingIntensity = new QLabel(ringTab);
    labelRingIntensity->setGeometry(10, 336, 240, 24);
    labelRingIntensity->setFont(QFont(QStringLiteral("Consolas"), 10));
    sliderRingIntensity = new QSlider(Qt::Horizontal, ringTab);
    sliderRingIntensity->setObjectName(QStringLiteral("sliderRingIntensity"));
    sliderRingIntensity->setGeometry(10, 364, 221, 16);
    sliderRingIntensity->setRange(0, 10);
    sliderRingIntensity->setValue(2);
    QLabel *ringIntValue = new QLabel(QStringLiteral("2"), ringTab);
    ringIntValue->setGeometry(230, 358, 41, 21);
    ringIntValue->setFont(QFont(QStringLiteral("Consolas"), 10));
    ringIntValue->setAlignment(Qt::AlignCenter);
    connect(sliderRingIntensity, &QSlider::valueChanged, ringIntValue, QOverload<int>::of(&QLabel::setNum));
    if (ringTab)
    {
        const QRect bounds = ringTab->childrenRect();
        ringTab->setMinimumSize(qMax(bounds.right() + 12, 380),
                                qMax(bounds.bottom() + 12, 500));
    }

    if (auto *slider = child<QSlider>("sliderIterations"))
        slider->setEnabled(false);
    if (auto *slider = child<QSlider>("sliderCloudSize"))
        slider->setRange(1, 10);
    if (auto *slider = child<QSlider>("sliderCloudQuality"))
        slider->setRange(1, 6);

    const char *valueSliders[] = {
        "sliderWorldSize", "sliderRandomness", "sliderTemperature", "sliderShine",
        "sliderCloudSize", "sliderCloudQuality", "sliderCloudTransparent",
        "sliderAtmoTransparent", "sliderAtmoSize", "sliderRingInner", "sliderRingOuter",
        "sliderIterations", "sliderNoise"
    };
    const char *valueLabels[] = {
        "label_2", "label_4", "label_6", "label_22",
        "label_30", "label_27", "label_29",
        "label_35", "label_34", "label_38", "label_37",
        "label_46", "label_48"
    };
    for (int i = 0; i < 13; ++i)
    {
        QSlider *slider = child<QSlider>(valueSliders[i]);
        QLabel *label = child<QLabel>(valueLabels[i]);
        if (!slider || !label)
            continue;
        connect(slider, &QSlider::valueChanged, this, [label](int v) {
            label->setText(QString::number(v));
        });
    }

    QList<QPushButton *> colors = {
        child<QPushButton>("btnColorIce"), child<QPushButton>("btnColorRock"),
        child<QPushButton>("btnColorMountain"), child<QPushButton>("btnColorPlain"),
        child<QPushButton>("btnColorBeach"), child<QPushButton>("btnColorShallow"),
        child<QPushButton>("btnColorOcean"), child<QPushButton>("btnColorCloud"),
        child<QPushButton>("btnColorAtmo"), child<QPushButton>("btnColorRing"),
        btnColorCiv
    };
    for (QPushButton *b : colors)
    {
        if (!b)
            continue;
        connect(b, &QPushButton::clicked, this, [this, b]() {
            const QColor current = ColorSwatch::color(b);
            const QColor color = QColorDialog::getColor(current, nullptr);
            if (!color.isValid())
                return;
            ColorSwatch::setColor(b, color);
            notify(b == btnColorCiv);
        });
    }

    wireLiveUpdates();
    updateAlgoEnabled();
    retranslate();
}

void SettingsPanel::notify(bool appearanceOnly)
{
    if (!updating)
        emit settingsChanged(appearanceOnly);
}

void SettingsPanel::wireLiveUpdates()
{
    auto requestFull = [this]() { notify(false); };
    auto requestAppearance = [this]() { notify(true); };

    if (auto *c = child<QComboBox>("comboTerraMode"))
        connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
            updateAlgoEnabled();
            notify(false);
        });

    const char *fullSliders[] = {
        "sliderWorldSize", "sliderRandomness", "sliderTemperature", "sliderIterations",
        "sliderNoise", "sliderCloudSize", "sliderCloudQuality"
    };
    for (const char *name : fullSliders)
    {
        if (auto *s = child<QSlider>(name))
            connect(s, &QSlider::valueChanged, this, requestFull);
    }
    const char *appearanceSliders[] = {
        "sliderShine", "sliderAtmoTransparent", "sliderAtmoSize",
        "sliderRingInner", "sliderRingOuter", "sliderCloudTransparent"
    };
    for (const char *name : appearanceSliders)
    {
        if (auto *s = child<QSlider>(name))
            connect(s, &QSlider::valueChanged, this, requestAppearance);
    }

    connect(sliderShineLat, &QSlider::valueChanged, this, requestAppearance);
    connect(sliderShineLon, &QSlider::valueChanged, this, requestAppearance);
    connect(checkFillLight, &QCheckBox::toggled, this, requestAppearance);
    connect(radioRingGas, &QRadioButton::toggled, this, requestAppearance);
    connect(sliderRingIntensity, &QSlider::valueChanged, this, requestAppearance);
    connect(sliderPolarLat, &QSlider::valueChanged, this, requestFull);
    connect(sliderPolarLon, &QSlider::valueChanged, this, requestFull);
    connect(ms, &MultiSlider::valueChanged, this, requestFull);

    const char *checks[] = {
        "checkCloud", "checkCorrection", "checkPlant", "checkGradient", "checkCiv"
    };
    for (const char *name : checks)
    {
        if (auto *b = child<QCheckBox>(name))
            connect(b, &QCheckBox::toggled, this, requestFull);
    }
    if (auto *b = child<QCheckBox>("checkAtmo"))
        connect(b, &QCheckBox::toggled, this, requestAppearance);
    if (auto *b = child<QCheckBox>("checkRing"))
        connect(b, &QCheckBox::toggled, this, requestAppearance);
    if (auto *r = child<QRadioButton>("radioName1"))
        connect(r, &QRadioButton::toggled, this, requestFull);
    if (auto *r = child<QRadioButton>("radioName2"))
        connect(r, &QRadioButton::toggled, this, requestFull);
    if (auto *r = child<QRadioButton>("radioName3"))
        connect(r, &QRadioButton::toggled, this, requestFull);
}

void SettingsPanel::updateAlgoEnabled()
{
    const bool noise = child<QComboBox>("comboTerraMode")
                           && child<QComboBox>("comboTerraMode")->currentIndex() == 0;
    if (auto *s = child<QSlider>("sliderRandomness"))
        s->setEnabled(noise);
    if (auto *s = child<QSlider>("sliderIterations"))
        s->setEnabled(!noise);
}

void SettingsPanel::collect(PlanetSettings &s) const
{
    if (auto *c = child<QComboBox>("comboTerraMode"))
        s.terramode = c->currentIndex();
    if (auto *v = child<QSlider>("sliderRandomness"))
        s.randomness = v->value();
    if (auto *v = child<QSlider>("sliderIterations"))
        s.iterations = v->value();
    if (auto *v = child<QSlider>("sliderWorldSize"))
        s.world_size = v->value();
    if (auto *v = child<QSlider>("sliderTemperature"))
        s.temperature = v->value();
    s.structure = ms->GetData();
    s.true_structure = ms->GetTrueData();
    s.ice_color = ColorSwatch::color(child<QPushButton>("btnColorIce"));
    s.rock_color = ColorSwatch::color(child<QPushButton>("btnColorRock"));
    s.mountain_color = ColorSwatch::color(child<QPushButton>("btnColorMountain"));
    s.plain_color = ColorSwatch::color(child<QPushButton>("btnColorPlain"));
    s.beach_color = ColorSwatch::color(child<QPushButton>("btnColorBeach"));
    s.shallow_color = ColorSwatch::color(child<QPushButton>("btnColorShallow"));
    s.ocean_color = ColorSwatch::color(child<QPushButton>("btnColorOcean"));
    if (auto *v = child<QSlider>("sliderNoise"))
        s.noise = v->value();
    if (auto *v = child<QCheckBox>("checkGradient"))
        s.is_gradient = v->isChecked();
    if (auto *v = child<QCheckBox>("checkPlant"))
        s.is_plant = v->isChecked();
    s.is_civ = checkCiv && checkCiv->isChecked();
    s.civ_color = ColorSwatch::color(btnColorCiv);
    if (auto *v = child<QSlider>("sliderShine"))
        s.shine = v->value();
    s.shine_lat = sliderShineLat->value();
    s.shine_lon = sliderShineLon->value();
    s.is_fill_light = checkFillLight->isChecked();
    if (child<QRadioButton>("radioName1") && child<QRadioButton>("radioName1")->isChecked())
        s.name_algorithm = 1;
    else if (child<QRadioButton>("radioName2") && child<QRadioButton>("radioName2")->isChecked())
        s.name_algorithm = 2;
    else
        s.name_algorithm = 3;
    if (auto *v = child<QCheckBox>("checkCloud"))
        s.is_cloud = v->isChecked();
    if (auto *v = child<QSlider>("sliderCloudSize"))
        s.cloud_size = v->value();
    if (auto *v = child<QSlider>("sliderCloudQuality"))
        s.cloud_quality = v->value();
    if (auto *v = child<QSlider>("sliderCloudTransparent"))
        s.cloud_transparent = v->value();
    if (auto *v = child<QCheckBox>("checkCorrection"))
        s.correction = v->isChecked();
    s.cloud_color = ColorSwatch::color(child<QPushButton>("btnColorCloud"));
    if (auto *v = child<QCheckBox>("checkAtmo"))
        s.is_atmo = v->isChecked();
    if (auto *v = child<QSlider>("sliderAtmoTransparent"))
        s.atmo_transparent = v->value();
    if (auto *v = child<QSlider>("sliderAtmoSize"))
        s.atmo_size = v->value();
    s.atmo_color = ColorSwatch::color(child<QPushButton>("btnColorAtmo"));
    if (auto *v = child<QCheckBox>("checkRing"))
        s.is_ring = v->isChecked();
    if (auto *v = child<QSlider>("sliderRingInner"))
        s.R_internal_ring = v->value();
    if (auto *v = child<QSlider>("sliderRingOuter"))
        s.R_external_ring = v->value();
    s.ring_color = ColorSwatch::color(child<QPushButton>("btnColorRing"));
    s.ring_material = radioRingMeteor->isChecked() ? 1 : 0;
    s.ring_intensity = sliderRingIntensity->value();
    s.polar_lat = sliderPolarLat->value();
    s.polar_lon = sliderPolarLon->value();
}

PlanetSettings SettingsPanel::pull() const
{
    PlanetSettings s;
    collect(s);
    return s;
}

void SettingsPanel::push(const PlanetSettings &s)
{
    updating = true;
    if (auto *c = child<QComboBox>("comboTerraMode"))
        c->setCurrentIndex(s.terramode);
    if (auto *v = child<QSlider>("sliderRandomness"))
        v->setValue(s.randomness);
    if (auto *v = child<QSlider>("sliderIterations"))
        v->setValue(s.iterations);
    if (auto *v = child<QSlider>("sliderWorldSize"))
        v->setValue(s.world_size);
    if (auto *v = child<QSlider>("sliderTemperature"))
        v->setValue(s.temperature);
    ms->SetData(s.structure);
    ColorSwatch::setColor(child<QPushButton>("btnColorIce"), s.ice_color);
    ColorSwatch::setColor(child<QPushButton>("btnColorRock"), s.rock_color);
    ColorSwatch::setColor(child<QPushButton>("btnColorMountain"), s.mountain_color);
    ColorSwatch::setColor(child<QPushButton>("btnColorPlain"), s.plain_color);
    ColorSwatch::setColor(child<QPushButton>("btnColorBeach"), s.beach_color);
    ColorSwatch::setColor(child<QPushButton>("btnColorShallow"), s.shallow_color);
    ColorSwatch::setColor(child<QPushButton>("btnColorOcean"), s.ocean_color);
    if (auto *v = child<QSlider>("sliderNoise"))
        v->setValue(s.noise);
    if (auto *v = child<QCheckBox>("checkGradient"))
        v->setChecked(s.is_gradient);
    if (auto *v = child<QCheckBox>("checkPlant"))
        v->setChecked(s.is_plant);
    if (checkCiv)
        checkCiv->setChecked(s.is_civ);
    ColorSwatch::setColor(btnColorCiv, s.civ_color);
    if (auto *v = child<QSlider>("sliderShine"))
        v->setValue(s.shine);
    sliderShineLat->setValue(s.shine_lat);
    sliderShineLon->setValue(s.shine_lon);
    checkFillLight->setChecked(s.is_fill_light);
    if (s.name_algorithm == 1)
        child<QRadioButton>("radioName1")->setChecked(true);
    else if (s.name_algorithm == 2)
        child<QRadioButton>("radioName2")->setChecked(true);
    else if (auto *r = child<QRadioButton>("radioName3"))
        r->setChecked(true);
    if (auto *v = child<QCheckBox>("checkCloud"))
        v->setChecked(s.is_cloud);
    if (auto *v = child<QSlider>("sliderCloudSize"))
        v->setValue(s.cloud_size);
    if (auto *v = child<QSlider>("sliderCloudQuality"))
        v->setValue(s.cloud_quality);
    if (auto *v = child<QSlider>("sliderCloudTransparent"))
        v->setValue(s.cloud_transparent);
    if (auto *v = child<QCheckBox>("checkCorrection"))
        v->setChecked(s.correction);
    ColorSwatch::setColor(child<QPushButton>("btnColorCloud"), s.cloud_color);
    if (auto *v = child<QCheckBox>("checkAtmo"))
        v->setChecked(s.is_atmo);
    if (auto *v = child<QSlider>("sliderAtmoTransparent"))
        v->setValue(s.atmo_transparent);
    if (auto *v = child<QSlider>("sliderAtmoSize"))
        v->setValue(s.atmo_size);
    ColorSwatch::setColor(child<QPushButton>("btnColorAtmo"), s.atmo_color);
    if (auto *v = child<QCheckBox>("checkRing"))
        v->setChecked(s.is_ring);
    if (auto *v = child<QSlider>("sliderRingInner"))
        v->setValue(s.R_internal_ring);
    if (auto *v = child<QSlider>("sliderRingOuter"))
        v->setValue(s.R_external_ring);
    ColorSwatch::setColor(child<QPushButton>("btnColorRing"), s.ring_color);
    radioRingGas->setChecked(s.ring_material != 1);
    radioRingMeteor->setChecked(s.ring_material == 1);
    sliderRingIntensity->setValue(s.ring_intensity);
    sliderPolarLat->setValue(s.polar_lat);
    sliderPolarLon->setValue(s.polar_lon);
    updateAlgoEnabled();
    updating = false;
}

void SettingsPanel::retranslate()
{
    ms->ReloadText();
    labelShineLatTitle->setText(QCoreApplication::translate("MainWindow", "Latitude"));
    labelShineLonTitle->setText(QCoreApplication::translate("MainWindow", "Longitude"));
    checkFillLight->setText(QCoreApplication::translate("MainWindow", "Fill light"));
    labelRingMaterial->setText(QCoreApplication::translate("MainWindow", "Material"));
    radioRingGas->setText(QCoreApplication::translate("MainWindow", "Gaseous"));
    radioRingMeteor->setText(QCoreApplication::translate("MainWindow", "Meteoritic"));
    labelRingIntensity->setText(QCoreApplication::translate("MainWindow", "Intensity"));
    labelPolarLatTitle->setText(QCoreApplication::translate("MainWindow", "Latitude"));
    labelPolarLonTitle->setText(QCoreApplication::translate("MainWindow", "Longitude"));
    if (checkCiv)
        checkCiv->setText(QCoreApplication::translate("MainWindow", "Intelligence"));
    if (btnColorCiv)
        btnColorCiv->setText(QCoreApplication::translate("MainWindow", "Color"));
}
