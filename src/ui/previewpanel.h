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
    QCheckBox *liveCheck() const;
    void setPlanetName(const QString &name);
    void setHasPlanet(bool hasPlanet);
    void showGlobe();
    void showCard(const QImage &image);
    void setLoading(bool loading);
    void retranslate();

private:
    class SquareBox *previewBox;
    QStackedWidget *stack;
    QWidget *overlay;
    QLabel *emptyLabel;
    QLabel *nameLabel;
    QLabel *cardView;
    QLabel *loadingLabel;
    class QMovie *loadingMovie;
    PlanetGLWidget *gl;
    QCheckBox *live;
    QPushButton *btnResetCamera;
};

#endif
