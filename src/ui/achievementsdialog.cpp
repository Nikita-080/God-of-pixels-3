#include "achievementsdialog.h"
#include "achievementcatalog.h"
#include "achievementstore.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>

namespace {

QString displayTitle(const AchievementDef &def, bool unlocked)
{
    if (def.secret && !unlocked)
        return achievementMaskedTitle();
    return QCoreApplication::translate("Achievements", def.title.toUtf8().constData());
}

QString displayDescription(const AchievementDef &def, bool unlocked)
{
    if (def.secret && !unlocked)
        return achievementMaskedDescription();
    return QCoreApplication::translate("Achievements", def.description.toUtf8().constData());
}

QWidget *makeCard(const AchievementDef &def, bool unlocked, const QDateTime &when, QWidget *parent)
{
    auto *card = new QWidget(parent);
    const QColor border = unlocked ? achievementRarityColor(def.rarity) : QColor(70, 70, 75);
    card->setStyleSheet(QStringLiteral(
        "QWidget#achCard { background-color: rgb(10, 12, 16); border: 2px solid %1; }"
        "QLabel { background: transparent; }")
                            .arg(border.name()));
    card->setObjectName(QStringLiteral("achCard"));

    auto *row = new QHBoxLayout(card);
    row->setContentsMargins(10, 10, 10, 10);
    row->setSpacing(12);

    auto *icon = new QLabel(card);
    icon->setFixedSize(64, 64);
    icon->setScaledContents(true);
    const QPixmap pm(def.resolvedIcon(unlocked, achievementDefaults()));
    if (!pm.isNull())
        icon->setPixmap(pm);
    if (!unlocked)
        icon->setStyleSheet(QStringLiteral("QLabel { opacity: 0.4; }"));
    row->addWidget(icon);

    auto *col = new QVBoxLayout;
    col->setSpacing(4);
    auto *title = new QLabel(displayTitle(def, unlocked), card);
    title->setWordWrap(true);
    auto *desc = new QLabel(displayDescription(def, unlocked), card);
    desc->setWordWrap(true);
    desc->setMaximumHeight(desc->fontMetrics().lineSpacing() * 3 + 4);
    if (unlocked)
    {
        title->setStyleSheet(QStringLiteral("color: rgb(230, 240, 245); font-weight: bold;"));
        desc->setStyleSheet(QStringLiteral("color: rgb(140, 175, 195);"));
    }
    else
    {
        title->setStyleSheet(QStringLiteral("color: rgb(90, 95, 100); font-weight: bold;"));
        desc->setStyleSheet(QStringLiteral("color: rgb(70, 74, 78);"));
        icon->setEnabled(false);
    }
    col->addWidget(title);
    col->addWidget(desc);
    if (unlocked && when.isValid())
    {
        auto *whenLabel = new QLabel(when.toLocalTime().toString(Qt::DefaultLocaleShortDate), card);
        whenLabel->setStyleSheet(QStringLiteral("color: rgb(90, 120, 140); font-size: 10px;"));
        col->addWidget(whenLabel);
    }
    col->addStretch(1);
    row->addLayout(col, 1);
    return card;
}

} // namespace

AchievementsDialog::AchievementsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QCoreApplication::translate("Achievements", "Achievements"));
    resize(520, 560);
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: rgb(0, 0, 0); color: rgb(110, 170, 200); }"
        "QScrollArea { background-color: rgb(0, 0, 0); border: none; }"));

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto *host = new QWidget(scroll);
    auto *layout = new QVBoxLayout(host);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    struct Row
    {
        AchievementDef def;
        bool unlocked = false;
        QDateTime when;
    };
    QVector<Row> unlocked;
    QVector<Row> locked;
    const QHash<QString, QDateTime> times = AchievementStore::allUnlocked();
    for (const AchievementDef &def : achievementCatalog())
    {
        Row r;
        r.def = def;
        r.when = times.value(def.id);
        r.unlocked = r.when.isValid() || AchievementStore::isUnlocked(def.id);
        if (r.unlocked)
            unlocked.append(r);
        else
            locked.append(r);
    }
    std::sort(unlocked.begin(), unlocked.end(), [](const Row &a, const Row &b) {
        return a.when > b.when;
    });

    for (const Row &r : unlocked)
        layout->addWidget(makeCard(r.def, true, r.when, host));
    for (const Row &r : locked)
        layout->addWidget(makeCard(r.def, false, QDateTime(), host));
    layout->addStretch(1);

    scroll->setWidget(host);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}
