#ifndef SPECTRUMDIALOG_H
#define SPECTRUMDIALOG_H

#include <QDialog>
#include <QVector>
#include <QWidget>

class QSlider;
class SpectrumWidget;

class SpectrumWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpectrumWidget(QWidget *parent = nullptr);
    void setBands(const QVector<int> &bands);
    QVector<int> bands() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<int> spec;
};

class SpectrumDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SpectrumDialog(const QVector<int> &initial, QWidget *parent = nullptr);
    QVector<int> bands() const;

private:
    void syncPreview();
    QVector<QSlider *> sliders;
    SpectrumWidget *preview;
};

#endif
