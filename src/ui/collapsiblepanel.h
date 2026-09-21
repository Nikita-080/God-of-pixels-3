#ifndef COLLAPSIBLEPANEL_H
#define COLLAPSIBLEPANEL_H

#include <QIcon>
#include <QWidget>

class QToolButton;

class CollapsiblePanel : public QWidget
{
    Q_OBJECT
public:
    CollapsiblePanel(const QString &title, const QIcon &icon, QWidget *content, QWidget *parent = nullptr);

    void setExpanded(bool expanded);
    bool isExpanded() const;
    void setTitle(const QString &title);
    QWidget *contentWidget() const;

private:
    void applyExpanded();

    QToolButton *toggle;
    QWidget *content;
};

#endif
