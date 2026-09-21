#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QVector>
#include <QWidget>
#include "planetsettings.h"

class QTabWidget;
class MultiSlider;
class QSlider;
class QLabel;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QPushButton;
class SpectrumWidget;
class CollapsiblePanel;

class SettingsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPanel(QTabWidget *tabs, QWidget *parent = nullptr);

    PlanetSettings pull() const;
    void collect(PlanetSettings &s) const;
    void push(const PlanetSettings &s);
    void retranslate();

signals:
    void settingsChanged(bool appearanceOnly);
    void settingsCommitted(bool appearanceOnly);

private:
    void wireLiveUpdates();
    void updateAlgoEnabled();
    void notify(bool appearanceOnly);
    void notifyLive(bool appearanceOnly);
    void bindSlider(QSlider *slider, bool appearanceOnly);
    void refreshAverageSwatches();
    void applyLandFromAverage(const QColor &center);
    void applyWaterFromAverage(const QColor &center);
    void updateStarDependentUi();
    void fitToContents();
    void buildAccordion(QTabWidget *tabs);
    template <typename T>
    T *child(const char *name) const;

    QWidget *host;
    MultiSlider *ms;
    QSlider *sliderShineLat;
    QSlider *sliderShineLon;
    QSlider *sliderPolarLat;
    QSlider *sliderPolarLon;
    QCheckBox *checkHasStar;
    QCheckBox *checkFillLight;
    QCheckBox *checkStarfield;
    SpectrumWidget *spectrumPreview;
    QPushButton *btnSpectrum;
    QCheckBox *checkCiv;
    QPushButton *btnColorCiv;
    QRadioButton *radioRingGas;
    QRadioButton *radioRingMeteor;
    QSlider *sliderRingIntensity;
    QLabel *labelRingMaterial;
    QLabel *labelRingIntensity;
    QLabel *labelShineLatTitle;
    QLabel *labelShineLonTitle;
    QLabel *labelStarSize;
    QLabel *labelPolarLatTitle;
    QLabel *labelPolarLonTitle;
    QLabel *labelAvgColor;
    QPushButton *btnAvgLand;
    QPushButton *btnAvgWater;
    QLabel *labelSeismicity;
    QSlider *sliderSeismicity;
    QVector<CollapsiblePanel *> sections;
    bool updating;
    bool sliderDrag;
};

#endif
