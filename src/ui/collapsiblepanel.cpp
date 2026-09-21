#include "collapsiblepanel.h"
#include <QIcon>
#include <QToolButton>
#include <QVBoxLayout>
#include <QSizePolicy>

CollapsiblePanel::CollapsiblePanel(const QString &title, const QIcon &icon, QWidget *contentWidget, QWidget *parent)
    : QWidget(parent)
    , toggle(new QToolButton(this))
    , content(contentWidget)
{
    toggle->setObjectName(QStringLiteral("accordionToggle"));
    toggle->setCheckable(true);
    toggle->setChecked(true);
    toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    toggle->setIcon(icon);
    toggle->setIconSize(QSize(28, 28));
    toggle->setText(title);
    toggle->setArrowType(Qt::DownArrow);

    if (content)
        content->setParent(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(toggle);
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

void CollapsiblePanel::applyExpanded()
{
    const bool on = toggle->isChecked();
    toggle->setArrowType(on ? Qt::DownArrow : Qt::RightArrow);
    if (content)
        content->setVisible(on);
}
