#ifndef ACHIEVEMENTTOAST_H
#define ACHIEVEMENTTOAST_H

#include "achievement.h"
#include <QObject>
#include <QVector>

class QWidget;
class QEvent;
class AchievementToast;

class AchievementToastHost : public QObject
{
    Q_OBJECT
public:
    explicit AchievementToastHost(QWidget *host);
    void enqueue(const QVector<AchievementDef> &unlocked);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void relayout();
    void removeToast(AchievementToast *toast);

    QWidget *hostWindow;
    QVector<AchievementToast *> toasts;
};

#endif
