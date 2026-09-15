#include "achievementtoast.h"
#include "achievementcatalog.h"
#include <QCoreApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class AchievementToast : public QWidget
{
public:
    AchievementToast(const AchievementDef &def, QWidget *parent)
        : QWidget(parent)
    {
        setFixedSize(320, 78);

        barColor = achievementRarityColor(def.rarity);

        auto *root = new QHBoxLayout(this);
        root->setContentsMargins(12, 8, 8, 8);
        root->setSpacing(8);

        auto *icon = new QLabel(this);
        icon->setFixedSize(56, 56);
        icon->setScaledContents(true);
        const QPixmap pm(def.resolvedIcon(true, achievementDefaults()));
        if (!pm.isNull())
            icon->setPixmap(pm);
        root->addWidget(icon);

        auto *textCol = new QVBoxLayout;
        textCol->setSpacing(2);
        auto *caption = new QLabel(QCoreApplication::translate("Achievements", "Achievement unlocked"), this);
        caption->setStyleSheet(QStringLiteral("color: rgb(160, 200, 220); font-size: 10px; background: transparent;"));
        auto *title = new QLabel(QCoreApplication::translate("Achievements", def.title.toUtf8().constData()), this);
        title->setWordWrap(true);
        title->setStyleSheet(QStringLiteral("color: rgb(230, 240, 245); font-weight: bold; background: transparent;"));
        textCol->addWidget(caption);
        textCol->addWidget(title);
        textCol->addStretch(1);
        root->addLayout(textCol, 1);

        auto *closeBtn = new QPushButton(QStringLiteral("x"), this);
        closeBtn->setFixedSize(22, 22);
        closeBtn->setFlat(true);
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setStyleSheet(QStringLiteral(
            "QPushButton { color: rgb(110, 170, 200); background: transparent; border: none; }"
            "QPushButton:hover { color: rgb(230, 240, 245); }"));
        connect(closeBtn, &QPushButton::clicked, this, &QWidget::deleteLater);
        root->addWidget(closeBtn, 0, Qt::AlignTop);

        auto *timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(4500);
        connect(timer, &QTimer::timeout, this, &QWidget::deleteLater);
        timer->start();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), QColor(8, 10, 14));
        p.fillRect(QRect(0, 0, 4, height()), barColor);
        p.setPen(QColor(50, 80, 95));
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }

private:
    QColor barColor;
};

QWidget *toastLayer(QWidget *host)
{
    if (auto *mw = qobject_cast<QMainWindow *>(host))
    {
        if (mw->centralWidget())
            return mw->centralWidget();
    }
    return host;
}

AchievementToastHost::AchievementToastHost(QWidget *host)
    : QObject(host)
    , hostWindow(host)
{
    QWidget *layer = toastLayer(hostWindow);
    if (layer)
        layer->installEventFilter(this);
}

void AchievementToastHost::enqueue(const QVector<AchievementDef> &unlocked)
{
    QWidget *layer = toastLayer(hostWindow);
    if (!layer)
        return;
    for (const AchievementDef &def : unlocked)
    {
        auto *toast = new AchievementToast(def, layer);
        connect(toast, &QObject::destroyed, this, [this, toast]() { removeToast(toast); });
        toasts.append(toast);
        toast->show();
        toast->raise();
    }
    relayout();
}

bool AchievementToastHost::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == toastLayer(hostWindow)
        && (event->type() == QEvent::Resize || event->type() == QEvent::Move))
        relayout();
    return QObject::eventFilter(watched, event);
}

void AchievementToastHost::relayout()
{
    QWidget *layer = toastLayer(hostWindow);
    if (!layer)
        return;
    int y = layer->height() - 16;
    for (int i = toasts.size() - 1; i >= 0; --i)
    {
        AchievementToast *t = toasts[i];
        if (!t)
            continue;
        y -= t->height() + 8;
        t->move(layer->width() - t->width() - 16, y);
        t->raise();
    }
}

void AchievementToastHost::removeToast(AchievementToast *toast)
{
    toasts.removeAll(toast);
    relayout();
}

