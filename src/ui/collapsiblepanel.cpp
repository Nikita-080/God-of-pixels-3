#include "collapsiblepanel.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QSize>
#include <QSizePolicy>
#include <QToolButton>
#include <QVBoxLayout>

CollapsiblePanel::CollapsiblePanel(const QString &title, const QIcon &icon, QWidget *contentWidget, QWidget *parent)
    : QWidget(parent)
    , header(new QWidget(this))
    , iconLabel(new QLabel(this))
    , toggle(new QToolButton(this))
    , chevron(new QLabel(this))
    , content(contentWidget)
{
    header->setObjectName(QStringLiteral("accordionHeader"));
    header->setAttribute(Qt::WA_StyledBackground, true);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->installEventFilter(this);

    iconLabel->setObjectName(QStringLiteral("accordionIcon"));
    iconLabel->setFixedSize(28, 28);
    iconLabel->setScaledContents(true);
    iconLabel->setPixmap(icon.pixmap(QSize(28, 28)));
    iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    toggle->setObjectName(QStringLiteral("accordionToggle"));
    toggle->setCheckable(true);
    toggle->setChecked(true);
    toggle->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toggle->setArrowType(Qt::NoArrow);
    toggle->setAutoRaise(true);
    toggle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toggle->setText(title);
    toggle->setFocusPolicy(Qt::NoFocus);

    chevron->setObjectName(QStringLiteral("accordionChevron"));
    chevron->setAlignment(Qt::AlignCenter);
    chevron->setFixedWidth(18);
    chevron->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    auto *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(8, 6, 8, 6);
    headerLay->setSpacing(8);
    headerLay->addWidget(iconLabel, 0, Qt::AlignVCenter);
    headerLay->addWidget(toggle, 1);
    headerLay->addWidget(chevron, 0, Qt::AlignVCenter);

    if (content)
    {
        content->setParent(this);
        content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(header);
    if (content)
        layout->addWidget(content);

    connect(toggle, &QToolButton::toggled, this, [this](bool) { applyExpanded(); });
    applyExpanded();
}

void CollapsiblePanel::setExpanded(bool expanded)
{
    toggle->setChecked(expanded);
}

bool CollapsiblePanel::isExpanded() const
{
    return toggle->isChecked();
}

void CollapsiblePanel::setTitle(const QString &title)
{
    toggle->setText(title);
}

QWidget *CollapsiblePanel::contentWidget() const
{
    return content;
}

bool CollapsiblePanel::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == header && event->type() == QEvent::MouseButtonRelease)
    {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton && header->childAt(me->pos()) != toggle)
            toggle->toggle();
    }
    return QWidget::eventFilter(watched, event);
}

void CollapsiblePanel::applyExpanded()
{
    const bool on = toggle->isChecked();
    chevron->setText(on ? QStringLiteral("▾") : QStringLiteral("▸"));
    if (content)
        content->setVisible(on);
}
