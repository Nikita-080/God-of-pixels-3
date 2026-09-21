#include <QCoreApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>
#include "previewpanel.h"
#include "planetglwidget.h"
#include <QCheckBox>
#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSizePolicy>

namespace {

class CropOverlay : public QWidget
{
public:
    explicit CropOverlay(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_StyledBackground, false);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const int side = qMin(width(), height());
        if (side <= 1)
            return;
        const QRect sq((width() - side) / 2, (height() - side) / 2, side, side);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);
        QRegion dim(rect());
        dim -= QRegion(sq);
        p.setClipRegion(dim);
        p.fillRect(rect(), QColor(255, 255, 255, 36));
        p.setClipping(false);
        p.setPen(QPen(QColor(140, 140, 140), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(sq.adjusted(0, 0, -1, -1));
    }
};

class PreviewChrome : public QWidget
{
public:
    explicit PreviewChrome(QWidget *passTo, QWidget *parent = nullptr)
        : QWidget(parent)
        , target(passTo)
    {
        setObjectName(QStringLiteral("previewChrome"));
        setAttribute(Qt::WA_StyledBackground, false);
        setAutoFillBackground(false);
        setStyleSheet(QStringLiteral("background: transparent;"));
    }

protected:
    bool event(QEvent *e) override
    {
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        {
            auto *me = static_cast<QMouseEvent *>(e);
            if (!childAt(me->pos()) && target)
                return QCoreApplication::sendEvent(target, e);
            break;
        }
        case QEvent::Wheel:
        {
            auto *we = static_cast<QWheelEvent *>(e);
            if (!childAt(we->position().toPoint()) && target)
                return QCoreApplication::sendEvent(target, e);
            break;
        }
        default:
            break;
        }
        return QWidget::event(e);
    }

private:
    QWidget *target;
};

} // namespace

PreviewPanel::PreviewPanel(QWidget *parent)
    : QWidget(parent)
{
    nameLabel = new QLabel(QStringLiteral("..."));
    nameLabel->setObjectName(QStringLiteral("labelPlanetName"));
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setAttribute(Qt::WA_StyledBackground, true);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    QFont nameFont(QStringLiteral("Consolas"), 24);
    nameLabel->setFont(nameFont);
    nameLabel->hide();

    emptyLabel = new QLabel;
    emptyLabel->setObjectName(QStringLiteral("labelEmptyPreview"));
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setWordWrap(true);
    QFont emptyFont(QStringLiteral("Consolas"), 16);
    emptyLabel->setFont(emptyFont);

    gl = new PlanetGLWidget;
    cardView = new QLabel;
    cardView->setAlignment(Qt::AlignCenter);
    cardView->setScaledContents(false);
    cardView->setMinimumSize(257, 257);

    stack = new QStackedWidget;
    stack->addWidget(emptyLabel);
    stack->addWidget(gl);
    stack->addWidget(cardView);
    stack->setMinimumSize(257, 257);
    stack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

    cropOverlay = new CropOverlay;
    cropOverlay->setObjectName(QStringLiteral("cropOverlay"));
    cropOverlay->setAttribute(Qt::WA_TranslucentBackground, true);
    cropOverlay->setAttribute(Qt::WA_NoSystemBackground, true);
    cropOverlay->setAutoFillBackground(false);
    cropOverlay->setStyleSheet(QStringLiteral("background: transparent;"));
    cropOverlay->hide();

    spin = new QCheckBox;
    spin->setObjectName(QStringLiteral("checkGlobeSpin"));
    spin->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    crop = new QCheckBox;
    crop->setObjectName(QStringLiteral("checkCropFrame"));
    crop->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(crop, &QCheckBox::toggled, this, [this](bool) { refreshCropOverlay(); });

    btnResetCamera = new QPushButton;
    btnResetCamera->setObjectName(QStringLiteral("btnResetCamera"));
    connect(btnResetCamera, &QPushButton::clicked, gl, &PlanetGLWidget::resetCamera);

    hud = new QWidget;
    hud->setObjectName(QStringLiteral("previewHud"));
    hud->setAttribute(Qt::WA_StyledBackground, true);
    hud->setAutoFillBackground(false);
    hud->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    auto *hudLayout = new QHBoxLayout(hud);
    hudLayout->setContentsMargins(8, 6, 8, 6);
    hudLayout->setSpacing(8);
    hudLayout->addWidget(spin);
    hudLayout->addWidget(crop);
    hudLayout->addStretch(1);
    hudLayout->addWidget(btnResetCamera);

    auto *chromeLay = new QVBoxLayout;
    chromeLay->setContentsMargins(0, 0, 0, 0);
    chromeLay->setSpacing(0);
    chromeLay->addWidget(nameLabel);
    chromeLay->addStretch(1);
    chromeLay->addWidget(hud);

    chrome = new PreviewChrome(gl);
    chrome->setLayout(chromeLay);

    auto *stage = new QWidget;
    auto *stageLayout = new QGridLayout(stage);
    stageLayout->setContentsMargins(0, 0, 0, 0);
    stageLayout->setSpacing(0);
    stageLayout->addWidget(stack, 0, 0);
    stageLayout->addWidget(cropOverlay, 0, 0);
    stageLayout->addWidget(chrome, 0, 0);
    stageLayout->addWidget(overlay, 0, 0);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(stage, 1);

    retranslate();
    setHasPlanet(false);
}

PlanetGLWidget *PreviewPanel::glWidget() const
{
    return gl;
}

QCheckBox *PreviewPanel::spinCheck() const
{
    return spin;
}

QCheckBox *PreviewPanel::cropCheck() const
{
    return crop;
}

bool PreviewPanel::isGlobeVisible() const
{
    return stack->currentWidget() == gl;
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
        nameLabel->hide();
        refreshCropOverlay();
        return;
    }
    btnResetCamera->setEnabled(true);
    nameLabel->show();
    if (stack->currentWidget() == emptyLabel)
        stack->setCurrentWidget(gl);
    refreshCropOverlay();
}

void PreviewPanel::showGlobe()
{
    if (stack->currentWidget() == emptyLabel)
        return;
    stack->setCurrentWidget(gl);
    refreshCropOverlay();
}

void PreviewPanel::showCard(const QImage &image)
{
    if (stack->currentWidget() == emptyLabel)
        return;
    lastCard = image;
    refreshCardPixmap();
    stack->setCurrentWidget(cardView);
    refreshCropOverlay();
}

void PreviewPanel::setLoading(bool loading)
{
    overlay->setVisible(loading);
    if (loading)
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
    spin->setText(QCoreApplication::translate("MainWindow", "Rotation"));
    crop->setText(QCoreApplication::translate("MainWindow", "Crop frame"));
    btnResetCamera->setText(QCoreApplication::translate("MainWindow", "Reset camera"));
}

void PreviewPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshCardPixmap();
}

void PreviewPanel::refreshCropOverlay()
{
    const bool show = crop->isChecked() && stack->currentWidget() == gl;
    cropOverlay->setVisible(show);
    if (show)
        cropOverlay->raise();
    if (chrome)
        chrome->raise();
    if (overlay && overlay->isVisible())
        overlay->raise();
}

void PreviewPanel::refreshCardPixmap()
{
    if (lastCard.isNull() || !cardView)
        return;
    const QSize box = cardView->size();
    if (box.width() < 2 || box.height() < 2)
        return;
    cardView->setPixmap(QPixmap::fromImage(lastCard).scaled(box, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
