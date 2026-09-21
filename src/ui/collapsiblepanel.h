#ifndef COLLAPSIBLEPANEL_H
#define COLLAPSIBLEPANEL_H

#include <QIcon>
#include <QWidget>

class QLabel;
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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void applyExpanded();

    QWidget *header;
    QLabel *iconLabel;
    QToolButton *toggle;
    QLabel *chevron;
    QWidget *content;
};

#endif
