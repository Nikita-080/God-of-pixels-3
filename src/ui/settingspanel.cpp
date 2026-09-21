#include <QCoreApplication>
#include "settingspanel.h"
#include "collapsiblepanel.h"
#include "multislider.h"
#include "colorswatch.h"
#include "spectrumdialog.h"
#include "starspectrum.h"
#include <QTabWidget>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDialog>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QList>
#include <QVBoxLayout>
#include <QFont>
#include <QIcon>
#include <QScrollArea>
#include <QFrame>
#include <QSizePolicy>
#include <QColorDialog>
#include <QTabBar>

namespace {
QColor averageRgb(const QList<QColor> &colors)
{
    qint64 r = 0;
    qint64 g = 0;
    qint64 b = 0;
    int n = 0;
    for (const QColor &c : colors)
    {
        if (!c.isValid())
            continue;
        r += c.red();
        g += c.green();
        b += c.blue();
        ++n;
    }
    if (n == 0)
        return QColor(66, 66, 66);
    return QColor(int(r / n), int(g / n), int(b / n));
}

QColor scaleRgb(const QColor &c, double k)
{
    return QColor(qBound(0, qRound(c.red() * k), 255),
                  qBound(0, qRound(c.green() * k), 255),
                  qBound(0, qRound(c.blue() * k), 255));
}
}

template <typename T>
T *SettingsPanel::child(const char *name) const
{
    return host->findChild<T *>(QLatin1String(name));
}

SettingsPanel::SettingsPanel(QTabWidget *tabs, QWidget *parent)
    : QWidget(parent)
    , host(tabs)
    , labelAvgColor(nullptr)
    , btnAvgLand(nullptr)
    , btnAvgWater(nullptr)
    , labelSeismicity(nullptr)
    , sliderSeismicity(nullptr)
    , updating(false)
    , sliderDrag(false)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
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
                            QLabel *&latTitle, QLabel *&lonTitle, int startY) {
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
        latTitle = makeTitle(startY);
        latSlider = makeSlider(startY + 30, -90, 90, 0);
        QLabel *latValue = makeValue(startY + 25);
        lonTitle = makeTitle(startY + 70);
        lonSlider = makeSlider(startY + 100, -180, 180, 0);
        QLabel *lonValue = makeValue(startY + 95);
        latValue->setText(QString::number(latSlider->value()));
        lonValue->setText(QString::number(lonSlider->value()));
        connect(latSlider, &QSlider::valueChanged, latValue, QOverload<int>::of(&QLabel::setNum));
        connect(lonSlider, &QSlider::valueChanged, lonValue, QOverload<int>::of(&QLabel::setNum));
    };

    auto *lightTab = child<QWidget>("tab_5");
    checkHasStar = new QCheckBox(lightTab);
    checkHasStar->setObjectName(QStringLiteral("checkHasStar"));
    checkHasStar->setGeometry(10, 42, 260, 24);
    checkHasStar->setFont(QFont(QStringLiteral("Consolas"), 10));
    checkHasStar->setChecked(true);
    if (auto *sizeTitle = child<QLabel>("label_13"))
    {
        labelStarSize = sizeTitle;
        sizeTitle->setGeometry(10, 70, 171, 24);
    }
    else
        labelStarSize = nullptr;
    if (auto *sizeSlider = child<QSlider>("sliderShine"))
        sizeSlider->setGeometry(10, 98, 221, 16);
    if (auto *sizeValue = child<QLabel>("label_22"))
        sizeValue->setGeometry(230, 90, 41, 21);

    addLatLon(lightTab, sliderShineLat, sliderShineLon,
              labelShineLatTitle, labelShineLonTitle, 128);
    addLatLon(child<QWidget>("tab_10"), sliderPolarLat, sliderPolarLon,
              labelPolarLatTitle, labelPolarLonTitle, 110);
    sliderPolarLat->setValue(90);

    checkFillLight = new QCheckBox(lightTab);
    checkFillLight->setObjectName(QStringLiteral("checkFillLight"));
    checkFillLight->setGeometry(10, 268, 260, 24);
    checkFillLight->setFont(QFont(QStringLiteral("Consolas"), 10));
    checkFillLight->setChecked(true);

    checkStarfield = new QCheckBox(lightTab);
    checkStarfield->setObjectName(QStringLiteral("checkStarfield"));
    checkStarfield->setGeometry(10, 296, 260, 24);
    checkStarfield->setFont(QFont(QStringLiteral("Consolas"), 10));

    spectrumPreview = new SpectrumWidget(lightTab);
    spectrumPreview->setGeometry(10, 328, 160, 52);
    spectrumPreview->setBands(defaultStarSpectrum());
    btnSpectrum = new QPushButton(lightTab);
    btnSpectrum->setObjectName(QStringLiteral("btnSpectrum"));
    btnSpectrum->setGeometry(178, 328, 92, 52);
    btnSpectrum->setFont(QFont(QStringLiteral("Consolas"), 10));
    btnSpectrum->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  color: rgb(110, 170, 200);"
        "  background-color: rgb(0, 0, 0);"
        "  border-width: 2px;"
        "  border-style: solid;"
        "  border-color: rgb(110, 170, 200);"
        "}"
        "QPushButton:disabled {"
        "  color: rgb(55, 75, 85);"
        "  border-color: rgb(45, 65, 75);"
        "}"));
    btnSpectrum->raise();
    btnSpectrum->show();

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

    if (auto *slider = child<QSlider>("sliderIterations"))
        slider->setEnabled(false);
    if (auto *slider = child<QSlider>("sliderCloudSize"))
        slider->setRange(1, 10);
    if (auto *slider = child<QSlider>("sliderCloudQuality"))
        slider->setRange(1, 6);

    if (QWidget *mainTab = child<QWidget>("tab"))
    {
        labelSeismicity = new QLabel(mainTab);
        labelSeismicity->setGeometry(20, 360, 171, 31);
        labelSeismicity->setFont(QFont(QStringLiteral("Consolas"), 10));
        sliderSeismicity = new QSlider(Qt::Horizontal, mainTab);
        sliderSeismicity->setObjectName(QStringLiteral("sliderSeismicity"));
        sliderSeismicity->setGeometry(20, 400, 221, 16);
        sliderSeismicity->setRange(0, 12);
        sliderSeismicity->setValue(0);
        QLabel *seisValue = new QLabel(QStringLiteral("0"), mainTab);
        seisValue->setObjectName(QStringLiteral("labelSeismicityValue"));
        seisValue->setGeometry(240, 390, 41, 21);
        seisValue->setFont(QFont(QStringLiteral("Consolas"), 10));
        seisValue->setAlignment(Qt::AlignCenter);
        connect(sliderSeismicity, &QSlider::valueChanged, seisValue, QOverload<int>::of(&QLabel::setNum));
        labelSeismicity->show();
        sliderSeismicity->show();
        seisValue->show();
    }

    if (QWidget *colorTab = child<QWidget>("tab_3"))
    {
        labelAvgColor = new QLabel(colorTab);
        labelAvgColor->setGeometry(290, 40, 80, 31);
        labelAvgColor->setFont(QFont(QStringLiteral("Consolas")));
        labelAvgColor->setAlignment(Qt::AlignCenter);
        btnAvgLand = new QPushButton(colorTab);
        btnAvgLand->setObjectName(QStringLiteral("btnAvgLand"));
        btnAvgLand->setGeometry(310, 75, 40, 220);
        btnAvgWater = new QPushButton(colorTab);
        btnAvgWater->setObjectName(QStringLiteral("btnAvgWater"));
        btnAvgWater->setGeometry(310, 300, 40, 85);
        labelAvgColor->show();
        btnAvgLand->show();
        btnAvgWater->show();
    }

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
            refreshAverageSwatches();
            notify(b == btnColorCiv);
        });
    }
    if (btnAvgLand)
    {
        connect(btnAvgLand, &QPushButton::clicked, this, [this]() {
            const QColor current = ColorSwatch::color(btnAvgLand);
            const QColor color = QColorDialog::getColor(current, nullptr);
            if (!color.isValid())
                return;
            applyLandFromAverage(color);
            refreshAverageSwatches();
            notify(false);
        });
    }
    if (btnAvgWater)
    {
        connect(btnAvgWater, &QPushButton::clicked, this, [this]() {
            const QColor current = ColorSwatch::color(btnAvgWater);
            const QColor color = QColorDialog::getColor(current, nullptr);
            if (!color.isValid())
                return;
            applyWaterFromAverage(color);
            refreshAverageSwatches();
            notify(false);
        });
    }
    refreshAverageSwatches();

    const char *headingLabels[] = {
        "label_10", "label_11", "label_12", "label_32", "label_39",
        "label_40", "label_41", "label_43", "label_42", "label_9"
    };
    for (const char *name : headingLabels)
    {
        if (auto *label = child<QLabel>(name))
            label->hide();
    }

    wireLiveUpdates();
    updateAlgoEnabled();
    updateStarDependentUi();
    buildAccordion(tabs);
    retranslate();
    fitToContents();
}

void SettingsPanel::notify(bool appearanceOnly)
{
    if (updating)
        return;
    emit settingsChanged(appearanceOnly);
    if (!sliderDrag)
        emit settingsCommitted(appearanceOnly);
}

void SettingsPanel::notifyLive(bool appearanceOnly)
{
    if (!updating)
        emit settingsChanged(appearanceOnly);
}

void SettingsPanel::bindSlider(QSlider *slider, bool appearanceOnly)
{
    if (!slider)
        return;
    connect(slider, &QSlider::sliderPressed, this, [this]() { sliderDrag = true; });
    connect(slider, &QSlider::valueChanged, this, [this, appearanceOnly](int) {
        notifyLive(appearanceOnly);
    });
    connect(slider, &QSlider::sliderReleased, this, [this, appearanceOnly]() {
        sliderDrag = false;
        notify(appearanceOnly);
    });
}

void SettingsPanel::buildAccordion(QTabWidget *tabs)
{
    static const char *kTitles[] = {
        QT_TRANSLATE_NOOP("MainWindow", "Main"),
        QT_TRANSLATE_NOOP("MainWindow", "Structure"),
        QT_TRANSLATE_NOOP("MainWindow", "Colors"),
        QT_TRANSLATE_NOOP("MainWindow", "Life"),
        QT_TRANSLATE_NOOP("MainWindow", "Light"),
        QT_TRANSLATE_NOOP("MainWindow", "Name"),
        QT_TRANSLATE_NOOP("MainWindow", "Clouds"),
        QT_TRANSLATE_NOOP("MainWindow", "Atmosphere"),
        QT_TRANSLATE_NOOP("MainWindow", "Rings"),
        QT_TRANSLATE_NOOP("MainWindow", "North")
    };

    auto *inner = new QWidget;
    auto *col = new QVBoxLayout(inner);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(4);

    const int count = tabs->count();
    for (int i = 0; i < count; ++i)
    {
        QWidget *page = tabs->widget(0);
        tabs->removeTab(0);
        const QString title = QCoreApplication::translate("MainWindow", kTitles[i]);
        const QIcon icon(QStringLiteral(":/images/res/images/TabIcon%1.png").arg(i + 1));
        auto *section = new CollapsiblePanel(title, icon, page, inner);
        section->setExpanded(i == 0);
        sections.append(section);
        col->addWidget(section);
    }
    col->addStretch(1);

    tabs->hide();
    tabs->setParent(this);

    host = inner;
    auto *scroll = new QScrollArea;
    scroll->setObjectName(QStringLiteral("settingsScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(inner);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);
}

void SettingsPanel::fitToContents()
{
    int contentRight = 260;
    for (CollapsiblePanel *section : sections)
    {
        QWidget *page = section ? section->contentWidget() : nullptr;
        if (!page)
            continue;
        const QRect bounds = page->childrenRect();
        const int w = qMax(bounds.right() + 12, 1);
        const int h = qMax(bounds.bottom() + 12, 80);
        page->setMinimumSize(w, h);
        contentRight = qMax(contentRight, w);
    }
    setMinimumWidth(contentRight + 24);
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
        "sliderNoise", "sliderCloudSize", "sliderCloudQuality", "sliderSeismicity"
    };
    for (const char *name : fullSliders)
        bindSlider(child<QSlider>(name), false);
    const char *appearanceSliders[] = {
        "sliderShine", "sliderAtmoTransparent", "sliderAtmoSize",
        "sliderRingInner", "sliderRingOuter", "sliderCloudTransparent"
    };
    for (const char *name : appearanceSliders)
        bindSlider(child<QSlider>(name), true);

    bindSlider(sliderShineLat, true);
    bindSlider(sliderShineLon, true);
    bindSlider(sliderRingIntensity, true);
    bindSlider(sliderPolarLat, false);
    bindSlider(sliderPolarLon, false);
    connect(checkFillLight, &QCheckBox::toggled, this, requestAppearance);
    connect(checkStarfield, &QCheckBox::toggled, this, requestAppearance);
    connect(checkHasStar, &QCheckBox::toggled, this, [this](bool) {
        updateStarDependentUi();
        notify(false);
    });
    connect(btnSpectrum, &QPushButton::clicked, this, [this]() {
        SpectrumDialog dlg(spectrumPreview->bands(), this);
        if (dlg.exec() != QDialog::Accepted)
            return;
        spectrumPreview->setBands(dlg.bands());
        notify(false);
    });
    connect(radioRingGas, &QRadioButton::toggled, this, requestAppearance);
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
        connect(b, &QCheckBox::toggled, this, requestFull);
    if (auto *b = child<QCheckBox>("checkRing"))
        connect(b, &QCheckBox::toggled, this, requestAppearance);
    if (auto *r = child<QRadioButton>("radioName1"))
        connect(r, &QRadioButton::toggled, this, requestFull);
    if (auto *r = child<QRadioButton>("radioName2"))
        connect(r, &QRadioButton::toggled, this, requestFull);
    if (auto *r = child<QRadioButton>("radioName3"))
        connect(r, &QRadioButton::toggled, this, requestFull);
}

void SettingsPanel::updateStarDependentUi()
{
    const bool on = checkHasStar && checkHasStar->isChecked();
    if (auto *v = child<QSlider>("sliderShine"))
        v->setEnabled(on);
    if (sliderShineLat)
        sliderShineLat->setEnabled(on);
    if (sliderShineLon)
        sliderShineLon->setEnabled(on);
    if (spectrumPreview)
        spectrumPreview->setEnabled(on);
    if (btnSpectrum)
        btnSpectrum->setEnabled(on);
    if (auto *t = child<QSlider>("sliderTemperature"))
        t->setEnabled(on);
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
    if (sliderSeismicity)
        s.seismicity = sliderSeismicity->value();
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
    s.has_star = checkHasStar && checkHasStar->isChecked();
    if (auto *v = child<QSlider>("sliderShine"))
        s.star_size = v->value();
    s.shine_lat = sliderShineLat->value();
    s.shine_lon = sliderShineLon->value();
    s.is_fill_light = checkFillLight->isChecked();
    s.is_starfield = checkStarfield && checkStarfield->isChecked();
    if (spectrumPreview)
        s.star_spectrum = spectrumPreview->bands();
    clampStarSpectrum(s.star_spectrum);
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
    if (sliderSeismicity)
        sliderSeismicity->setValue(s.seismicity);
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
    if (checkHasStar)
        checkHasStar->setChecked(s.has_star);
    if (auto *v = child<QSlider>("sliderShine"))
        v->setValue(s.star_size);
    sliderShineLat->setValue(s.shine_lat);
    sliderShineLon->setValue(s.shine_lon);
    checkFillLight->setChecked(s.is_fill_light);
    if (checkStarfield)
        checkStarfield->setChecked(s.is_starfield);
    if (spectrumPreview)
        spectrumPreview->setBands(s.star_spectrum);
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
    refreshAverageSwatches();
    updateAlgoEnabled();
    updateStarDependentUi();
    updating = false;
}

void SettingsPanel::retranslate()
{
    static const char *kTitles[] = {
        QT_TRANSLATE_NOOP("MainWindow", "Main"),
        QT_TRANSLATE_NOOP("MainWindow", "Structure"),
        QT_TRANSLATE_NOOP("MainWindow", "Colors"),
        QT_TRANSLATE_NOOP("MainWindow", "Life"),
        QT_TRANSLATE_NOOP("MainWindow", "Light"),
        QT_TRANSLATE_NOOP("MainWindow", "Name"),
        QT_TRANSLATE_NOOP("MainWindow", "Clouds"),
        QT_TRANSLATE_NOOP("MainWindow", "Atmosphere"),
        QT_TRANSLATE_NOOP("MainWindow", "Rings"),
        QT_TRANSLATE_NOOP("MainWindow", "North")
    };
    for (int i = 0; i < sections.size() && i < 10; ++i)
        sections.at(i)->setTitle(QCoreApplication::translate("MainWindow", kTitles[i]));
    ms->ReloadText();
    if (labelStarSize)
        labelStarSize->setText(QCoreApplication::translate("MainWindow", "Size"));
    if (checkHasStar)
        checkHasStar->setText(QCoreApplication::translate("MainWindow", "Star"));
    if (checkStarfield)
        checkStarfield->setText(QCoreApplication::translate("MainWindow", "Stars"));
    if (btnSpectrum)
        btnSpectrum->setText(QCoreApplication::translate("MainWindow", "Spectrum"));
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
    if (labelAvgColor)
        labelAvgColor->setText(QCoreApplication::translate("MainWindow", "Average"));
    if (labelSeismicity)
        labelSeismicity->setText(QCoreApplication::translate("MainWindow", "Seismicity"));
}

void SettingsPanel::refreshAverageSwatches()
{
    if (btnAvgLand)
    {
        ColorSwatch::setColor(btnAvgLand, averageRgb({
            ColorSwatch::color(child<QPushButton>("btnColorIce")),
            ColorSwatch::color(child<QPushButton>("btnColorRock")),
            ColorSwatch::color(child<QPushButton>("btnColorMountain")),
            ColorSwatch::color(child<QPushButton>("btnColorPlain")),
            ColorSwatch::color(child<QPushButton>("btnColorBeach"))
        }));
    }
    if (btnAvgWater)
    {
        ColorSwatch::setColor(btnAvgWater, averageRgb({
            ColorSwatch::color(child<QPushButton>("btnColorShallow")),
            ColorSwatch::color(child<QPushButton>("btnColorOcean"))
        }));
    }
}

void SettingsPanel::applyLandFromAverage(const QColor &center)
{
    ColorSwatch::setColor(child<QPushButton>("btnColorIce"), scaleRgb(center, 0.70));
    ColorSwatch::setColor(child<QPushButton>("btnColorRock"), scaleRgb(center, 0.85));
    ColorSwatch::setColor(child<QPushButton>("btnColorMountain"), scaleRgb(center, 1.00));
    ColorSwatch::setColor(child<QPushButton>("btnColorPlain"), scaleRgb(center, 1.15));
    ColorSwatch::setColor(child<QPushButton>("btnColorBeach"), scaleRgb(center, 1.30));
}

void SettingsPanel::applyWaterFromAverage(const QColor &center)
{
    ColorSwatch::setColor(child<QPushButton>("btnColorShallow"), scaleRgb(center, 1.18));
    ColorSwatch::setColor(child<QPushButton>("btnColorOcean"), scaleRgb(center, 0.82));
}
