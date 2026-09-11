#include <QCoreApplication>
#include <QPixmap>
#include "previewpanel.h"
#include "planetglwidget.h"
#include <QCheckBox>
#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QGridLayout>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSizePolicy>

class SquareBox : public QWidget
{
public:
    explicit SquareBox(QWidget *child, QWidget *parent = nullptr)
        : QWidget(parent)
        , inner(child)
    {
        inner->setParent(this);
        setMinimumSize(257, 257);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

protected:
    void resizeEvent(QResizeEvent *) override
    {
        const int side = qMin(width(), height());
        inner->setGeometry((width() - side) / 2, (height() - side) / 2, side, side);
    }

private:
    QWidget *inner;
};

PreviewPanel::PreviewPanel(QWidget *parent)
    : QWidget(parent)
{
    nameLabel = new QLabel(QStringLiteral("..."));
    nameLabel->setObjectName(QStringLiteral("labelPlanetName"));
    nameLabel->setAlignment(Qt::AlignCenter);
    QFont nameFont(QStringLiteral("Consolas"), 24);
    nameLabel->setFont(nameFont);

    emptyLabel = new QLabel;
    emptyLabel->setObjectName(QStringLiteral("labelEmptyPreview"));
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setWordWrap(true);
    QFont emptyFont(QStringLiteral("Consolas"), 16);
    emptyLabel->setFont(emptyFont);

    gl = new PlanetGLWidget;
    cardView = new QLabel;
    cardView->setAlignment(Qt::AlignCenter);
    cardView->setScaledContents(true);
    cardView->setMinimumSize(257, 257);

    stack = new QStackedWidget;
    stack->addWidget(emptyLabel);
    stack->addWidget(gl);
    stack->addWidget(cardView);
    stack->setMinimumSize(257, 257);

    loadingMovie = new QMovie(QStringLiteral(":/images/res/images/loading.gif"), QByteArray(), this);
    loadingLabel = new QLabel;
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setMovie(loadingMovie);
    loadingLabel->setScaledContents(false);

    overlay = new QWidget;
    overlay->setObjectName(QStringLiteral("loadingOverlay"));
    overlay->setAttribute(Qt::WA_StyledBackground, true);
    overlay->setStyleSheet(QStringLiteral(
        "QWidget#loadingOverlay { background-color: rgba(0, 0, 0, 180); }"));
    auto *overlayLayout = new QVBoxLayout(overlay);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    overlayLayout->addStretch();
    overlayLayout->addWidget(loadingLabel, 0, Qt::AlignCenter);
    overlayLayout->addStretch();
    overlay->hide();

    auto *stage = new QWidget;
    auto *stageLayout = new QGridLayout(stage);
    stageLayout->setContentsMargins(0, 0, 0, 0);
    stageLayout->setSpacing(0);
    stageLayout->addWidget(stack, 0, 0);
    stageLayout->addWidget(overlay, 0, 0);

    previewBox = new SquareBox(stage);

    live = new QCheckBox;
    live->setObjectName(QStringLiteral("checkLivePreview"));
    live->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    live->setMinimumHeight(32);

    btnResetCamera = new QPushButton;
    btnResetCamera->setObjectName(QStringLiteral("btnResetCamera"));
    connect(btnResetCamera, &QPushButton::clicked, gl, &PlanetGLWidget::resetCamera);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 8);
    layout->setSpacing(10);
    layout->addWidget(nameLabel);
    layout->addWidget(previewBox, 1);
    layout->addWidget(live);
    layout->addWidget(btnResetCamera);

    retranslate();
    setHasPlanet(false);
}

PlanetGLWidget *PreviewPanel::glWidget() const
{
    return gl;
}

QCheckBox *PreviewPanel::liveCheck() const
{
    return live;
}

void PreviewPanel::setPlanetName(const QString &name)
{
    nameLabel->setText(name);
}

void PreviewPanel::setHasPlanet(bool hasPlanet)
{
    if (!hasPlanet)
    {
        stack->setCurrentWidget(emptyLabel);
        btnResetCamera->setEnabled(false);
        return;
    }
    btnResetCamera->setEnabled(true);
    if (stack->currentWidget() == emptyLabel)
        stack->setCurrentWidget(gl);
}

void PreviewPanel::showGlobe()
{
    if (stack->currentWidget() == emptyLabel)
        return;
    stack->setCurrentWidget(gl);
}

void PreviewPanel::showCard(const QImage &image)
{
    if (stack->currentWidget() == emptyLabel)
        return;
    cardView->setPixmap(QPixmap::fromImage(image));
    stack->setCurrentWidget(cardView);
}

void PreviewPanel::setLoading(bool loading)
{
    overlay->setVisible(loading);
    overlay->raise();
    if (loading)
    {
        if (loadingMovie->state() != QMovie::Running)
            loadingMovie->start();
    }
    else
    {
        loadingMovie->stop();
        loadingMovie->jumpToFrame(0);
    }
}

void PreviewPanel::retranslate()
{
    emptyLabel->setText(QCoreApplication::translate("MainWindow", "Create a planet to see the preview"));
    live->setText(QCoreApplication::translate("MainWindow", "Live preview"));
    btnResetCamera->setText(QCoreApplication::translate("MainWindow", "Reset camera"));
}
