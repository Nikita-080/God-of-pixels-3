#include "spectrumdialog.h"
#include "starspectrum.h"
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>

SpectrumWidget::SpectrumWidget(QWidget *parent)
    : QWidget(parent)
    , spec(defaultStarSpectrum())
{
    setMinimumSize(160, 40);
}

void SpectrumWidget::setBands(const QVector<int> &bands)
{
    spec = bands;
    clampStarSpectrum(spec);
    update();
}

QVector<int> SpectrumWidget::bands() const
{
    return spec;
}

void SpectrumWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    paintStarSpectrum(p, rect(), spec);
}

SpectrumDialog::SpectrumDialog(const QVector<int> &initial, QWidget *parent)
    : QDialog(parent)
    , preview(new SpectrumWidget(this))
{
    setWindowTitle(QCoreApplication::translate("Spectrum", "Spectrum"));
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: rgb(0, 0, 0); color: rgb(110, 170, 200); }"
        "QLabel { color: rgb(110, 170, 200); background-color: transparent; }"
        "QPushButton {"
        "  color: rgb(110, 170, 200);"
        "  background-color: rgb(0, 0, 0);"
        "  border-width: 2px;"
        "  border-style: solid;"
        "  border-color: rgb(110, 170, 200);"
        "  padding: 6px 18px;"
        "  min-height: 28px;"
        "}"
        "QScrollArea { background-color: rgb(0, 0, 0); border: none; }"));
    QVector<int> bands = initial;
    clampStarSpectrum(bands);

    auto *formHost = new QWidget(this);
    auto *form = new QFormLayout(formHost);
    form->setContentsMargins(0, 0, 8, 0);
    for (int i = 0; i < StarBandCount; ++i)
    {
        auto *slider = new QSlider(Qt::Horizontal, formHost);
        slider->setRange(0, kStarBandMax);
        slider->setValue(starBand(bands, i));
        auto *value = new QLabel(QString::number(slider->value()), formHost);
        value->setMinimumWidth(24);
        connect(slider, &QSlider::valueChanged, this, [this, value](int v) {
            value->setText(QString::number(v));
            syncPreview();
        });
        auto *row = new QWidget(formHost);
        auto *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->addWidget(slider, 1);
        hl->addWidget(value);
        form->addRow(starBandTitle(i), row);
        sliders.append(slider);
    }

    auto *scroll = new QScrollArea(this);
    scroll->setWidget(formHost);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setMinimumHeight(280);

    preview->setMinimumHeight(56);
    preview->setBands(bands);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setCenterButtons(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(scroll, 1);
    layout->addWidget(preview);
    layout->addWidget(buttons, 0);
    setMinimumSize(400, 520);
    resize(420, 560);
}

void SpectrumDialog::syncPreview()
{
    preview->setBands(bands());
}

QVector<int> SpectrumDialog::bands() const
{
    QVector<int> out;
    out.reserve(sliders.size());
    for (QSlider *s : sliders)
        out.append(s ? s->value() : 0);
    clampStarSpectrum(out);
    return out;
}
