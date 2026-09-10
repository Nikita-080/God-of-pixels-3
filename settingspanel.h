#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

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

private:
    void wireLiveUpdates();
    void updateAlgoEnabled();
    void notify(bool appearanceOnly);
    template <typename T>
    T *child(const char *name) const;

    QTabWidget *tabs;
    MultiSlider *ms;
    QSlider *sliderShineLat;
    QSlider *sliderShineLon;
    QSlider *sliderPolarLat;
    QSlider *sliderPolarLon;
    QLabel *labelShineLatTitle;
    QLabel *labelShineLonTitle;
    QLabel *labelPolarLatTitle;
    QLabel *labelPolarLonTitle;
    bool updating;
};

#endif
