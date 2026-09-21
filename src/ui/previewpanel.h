#ifndef PREVIEWPANEL_H
#define PREVIEWPANEL_H

#include <QWidget>
#include <QImage>

class PlanetGLWidget;
class QLabel;
class QCheckBox;
class QPushButton;
class QStackedWidget;

class PreviewPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewPanel(QWidget *parent = nullptr);

    PlanetGLWidget *glWidget() const;
    QCheckBox *spinCheck() const;
    QCheckBox *cropCheck() const;
    bool isGlobeVisible() const;
    void setPlanetName(const QString &name);
    void setHasPlanet(bool hasPlanet);
    void showGlobe();
    void showCard(const QImage &image);
    void setLoading(bool loading);
    void retranslate();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void refreshCropOverlay();
    void refreshCardPixmap();

    QStackedWidget *stack;
    QWidget *overlay;
    QWidget *cropOverlay;
    QWidget *hud;
    QWidget *chrome;
    QLabel *emptyLabel;
    QLabel *nameLabel;
    QLabel *cardView;
    QLabel *loadingLabel;
    class QMovie *loadingMovie;
    PlanetGLWidget *gl;
    QCheckBox *spin;
    QCheckBox *crop;
    QPushButton *btnResetCamera;
    QImage lastCard;
};

#endif
