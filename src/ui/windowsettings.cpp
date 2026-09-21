#include "windowsettings.h"
#include "ui_windowsettings.h"
#include "appsettings.h"
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QtGlobal>

namespace {
const int FlagKeyRole = Qt::UserRole;
}

windowsettings::windowsettings(const QString &language, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::windowsettings)
    , language(language)
{
    ui->setupUi(this);
    setWindowTitle(tr("Autogen"));
    applyDialogStyle();
    progressCaptionTimer = new QTimer(this);
    progressCaptionTimer->setInterval(1000);
    connect(progressCaptionTimer, &QTimer::timeout, this, &windowsettings::showRandomProgressCaption);
    loadProgressMessages();
    buildFlagTree();
    fillDefaultPaths();
    restoreSession();
    ui->labelProgress->setText(tr("Autogen progress"));
    ui->progressBar->setValue(0);
    ui->btnStop->setEnabled(false);
    ui->btnOpenFolder->setEnabled(false);
    updateWindowTitle();

    connect(ui->btnCollageBrowse, &QPushButton::clicked, this, &windowsettings::askCollageFile);
    connect(ui->btnImagesBrowse, &QPushButton::clicked, this, &windowsettings::askImagesDir);
    connect(ui->btnInvert, &QPushButton::clicked, this, &windowsettings::invertFlags);
    connect(ui->editFlagFilter, &QLineEdit::textChanged, this, &windowsettings::filterFlags);
    connect(ui->btnLoad, &QPushButton::clicked, this, &windowsettings::loadPreset);
    connect(ui->btnSave, &QPushButton::clicked, this, &windowsettings::savePreset);
    connect(ui->btnOpenFolder, &QPushButton::clicked, this, &windowsettings::openOutputFolder);
    connect(ui->btnRun, &QPushButton::clicked, this, &windowsettings::onRun);
    connect(ui->btnStop, &QPushButton::clicked, this, &windowsettings::onStop);
    connect(ui->btnClose, &QPushButton::clicked, this, &windowsettings::onClose);
}

windowsettings::~windowsettings()
{
    saveSession();
    delete ui;
}

void windowsettings::applyDialogStyle()
{
    setStyleSheet(QStringLiteral(
        "QLineEdit, QSpinBox {"
        "  color: rgb(0, 255, 127);"
        "  background-color: rgb(0, 0, 0);"
        "  border-width: 3px;"
        "  border-style: solid;"
        "  border-color: rgb(110, 170, 200);"
        "  min-height: 28px;"
        "  padding: 2px 6px;"
        "  selection-background-color: rgb(0, 70, 70);"
        "  selection-color: rgb(160, 210, 230);"
        "}"
        "QLineEdit::placeholder { color: rgb(70, 110, 90); }"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "  background: rgb(0, 30, 40);"
        "  border-left: 1px solid rgb(110, 170, 200);"
        "  width: 18px;"
        "}"
        "QSpinBox::up-arrow, QSpinBox::down-arrow {"
        "  width: 8px;"
        "  height: 8px;"
        "  background-color: rgb(0, 255, 127);"
        "}"
        "QPushButton {"
        "  color: rgb(110, 170, 200);"
        "  background-color: rgb(0, 0, 0);"
        "  border-width: 3px;"
        "  border-style: solid;"
        "  border-color: rgb(110, 170, 200);"
        "  min-height: 28px;"
        "  padding: 4px 10px;"
        "}"
        "QTreeWidget {"
        "  color: rgb(110, 170, 200);"
        "  background-color: rgb(0, 0, 0);"
        "  border: 3px solid rgb(110, 170, 200);"
        "  font-family: Consolas;"
        "  font-size: 12pt;"
        "  outline: none;"
        "}"
        "QTreeWidget::item { color: rgb(110, 170, 200); }"
        "QTreeWidget::item:selected { background-color: rgb(30, 50, 60); color: rgb(0, 255, 127); }"
        "QTreeWidget::branch { background: rgb(0, 0, 0); }"
        "QTreeWidget::branch:has-children:!has-siblings:closed,"
        "QTreeWidget::branch:closed:has-children:has-siblings {"
        "  image: none;"
        "  border: 5px solid rgb(0, 0, 0);"
        "  border-left: 7px solid rgb(0, 255, 127);"
        "}"
        "QTreeWidget::branch:open:has-children:!has-siblings,"
        "QTreeWidget::branch:open:has-children:has-siblings {"
        "  image: none;"
        "  border: 5px solid rgb(0, 0, 0);"
        "  border-top: 7px solid rgb(0, 255, 127);"
        "}"
        "QTreeWidget::indicator {"
        "  width: 14px;"
        "  height: 14px;"
        "  border: 2px solid rgb(110, 170, 200);"
        "  background: rgb(0, 0, 0);"
        "}"
        "QTreeWidget::indicator:checked { background: rgb(0, 255, 127); }"
        "QTreeWidget::indicator:indeterminate { background: rgb(110, 170, 200); }"
        "QHeaderView { background-color: rgb(0, 0, 0); color: rgb(110, 170, 200); }"
        "QLabel { color: rgb(110, 170, 200); font-family: Consolas; font-size: 12pt; }"
        "QCheckBox { color: rgb(110, 170, 200); font-family: Consolas; font-size: 12pt; }"
        "QCheckBox::indicator {"
        "  width: 14px;"
        "  height: 14px;"
        "  border: 2px solid rgb(110, 170, 200);"
        "  background: rgb(0, 0, 0);"
        "}"
        "QCheckBox::indicator:checked { background: rgb(0, 255, 127); }"
        "QTabBar::tab {"
        "  font-family: Consolas;"
        "  font-size: 12pt;"
        "  color: rgb(110, 170, 200);"
        "  background-color: rgb(0, 0, 0);"
        "  padding: 6px 14px;"
        "  min-width: 80px;"
        "}"
        "QTabBar::tab:selected { color: rgb(0, 255, 127); border: 1px solid rgb(255, 0, 0); }"
        "QTabBar::tab:hover { border: 1px solid rgb(0, 255, 0); }"));
    const QFont uiFont(QStringLiteral("Consolas"), 12);
    setFont(uiFont);
    ui->btnCollageBrowse->setFixedWidth(52);
    ui->btnImagesBrowse->setFixedWidth(52);
}

void windowsettings::loadProgressMessages()
{
    progressMessages.clear();
    QFile file(QStringLiteral(":/txt_files/res/txt_files/autogen_progress.json"));
    if (!file.open(QIODevice::ReadOnly))
        return;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonArray messages = doc.object().value(QStringLiteral("messages")).toArray();
    for (const QJsonValue &value : messages)
    {
        const QString line = value.toString().trimmed();
        if (!line.isEmpty())
            progressMessages.append(line);
    }
}

void windowsettings::buildFlagTree()
{
    flagKeys = AutoGenSettings::canonicalFlagKeys();
    const QString labelsPath = (language == QStringLiteral("ru"))
                                   ? QStringLiteral(":/txt_files/res/txt_files/randomsettings_ru.txt")
                                   : QStringLiteral(":/txt_files/res/txt_files/randomsettings_en.txt");
    flagLabels = AutoGenSettings::loadResourceLines(labelsPath);
    while (flagLabels.size() < flagKeys.size())
        flagLabels.append(flagKeys.value(flagLabels.size()));

    ui->treeFlags->clear();
    leafItems.clear();
    ui->treeFlags->setColumnCount(1);
    ui->treeFlags->setHeaderHidden(true);

    rootItem = new QTreeWidgetItem(ui->treeFlags);
    rootItem->setText(0, tr("Do random"));
    rootItem->setFlags(rootItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    rootItem->setCheckState(0, Qt::Unchecked);

    auto addGroup = [this](const QString &title, const QStringList &keys) {
        makeGroup(rootItem, title, keys, flagLabels);
    };
    addGroup(tr("Terrain"), QStringList()
                                << QStringLiteral("algorithm") << QStringLiteral("randomness")
                                << QStringLiteral("detalization") << QStringLiteral("quality")
                                << QStringLiteral("temperature") << QStringLiteral("structure")
                                << QStringLiteral("noise") << QStringLiteral("gradient")
                                << QStringLiteral("seismicity") << QStringLiteral("north"));
    addGroup(tr("Colors"), QStringList()
                               << QStringLiteral("ice color") << QStringLiteral("rock color")
                               << QStringLiteral("mountain color") << QStringLiteral("plain color")
                               << QStringLiteral("sand color") << QStringLiteral("shelf color")
                               << QStringLiteral("ocean color"));
    addGroup(tr("Clouds"), QStringList()
                               << QStringLiteral("is clouds") << QStringLiteral("cloud size")
                               << QStringLiteral("cloud detalization") << QStringLiteral("cloud transparent")
                               << QStringLiteral("cloud corection") << QStringLiteral("cloud color"));
    addGroup(tr("Atmosphere"), QStringList()
                                   << QStringLiteral("is atmosphere") << QStringLiteral("atmosphere transparent")
                                   << QStringLiteral("atmosphere size") << QStringLiteral("atposphere color"));
    addGroup(tr("Rings"), QStringList()
                              << QStringLiteral("is ring") << QStringLiteral("ring internal R")
                              << QStringLiteral("ring external R") << QStringLiteral("ring color")
                              << QStringLiteral("ring material") << QStringLiteral("ring intensity"));
    addGroup(tr("Life"), QStringList()
                             << QStringLiteral("is plant") << QStringLiteral("intelligence")
                             << QStringLiteral("civilization color"));
    addGroup(tr("Star"), QStringList()
                             << QStringLiteral("star size") << QStringLiteral("star position")
                             << QStringLiteral("name algorithm") << QStringLiteral("has star")
                             << QStringLiteral("stars") << QStringLiteral("spectrum"));
    ui->treeFlags->expandAll();
}

QTreeWidgetItem *windowsettings::makeGroup(QTreeWidgetItem *parent, const QString &title,
                                           const QStringList &keys, const QStringList &labels)
{
    auto *group = new QTreeWidgetItem(parent);
    group->setText(0, title);
    group->setFlags(group->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    group->setCheckState(0, Qt::Unchecked);
    for (const QString &key : keys)
    {
        const int index = flagKeys.indexOf(key);
        auto *leaf = new QTreeWidgetItem(group);
        leaf->setText(0, (index >= 0 && index < labels.size()) ? labels.at(index) : key);
        leaf->setData(0, FlagKeyRole, key);
        leaf->setFlags((leaf->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable)
                       & ~Qt::ItemIsAutoTristate);
        leaf->setCheckState(0, Qt::Unchecked);
        leafItems.append(leaf);
    }
    return group;
}

void windowsettings::fillDefaultPaths()
{
    if (ui->editCollagePath->text().trimmed().isEmpty())
        ui->editCollagePath->setText(startPath(AppKeys::dirAutogen, QStringLiteral("collage.png")));
    if (ui->editImagesPath->text().trimmed().isEmpty())
        ui->editImagesPath->setText(startPath(AppKeys::dirAutogen));
}

AutoGenSettings windowsettings::settings() const
{
    return resultSettings;
}

AutoGenSettings windowsettings::collectSettings() const
{
    AutoGenSettings out;
    const bool collage = ui->tabMode->currentIndex() == 0;
    out.mode = collage ? AutoGenMode::Collage : AutoGenMode::SeparateFiles;
    out.extendedFormat = ui->checkExtended->isChecked();
    out.path = collage ? ui->editCollagePath->text().trimmed() : ui->editImagesPath->text().trimmed();
    out.height = ui->spinHeight->value();
    out.width = ui->spinWidth->value();
    out.number = ui->spinNumber->value();
    out.isRndList = leafFlags();
    return out;
}

QMap<QString, bool> windowsettings::leafFlags() const
{
    QMap<QString, bool> flags;
    for (QTreeWidgetItem *leaf : leafItems)
    {
        const QString key = leaf->data(0, FlagKeyRole).toString();
        if (!key.isEmpty())
            flags.insert(key, leaf->checkState(0) == Qt::Checked);
    }
    return flags;
}

void windowsettings::setLeafFlags(const QMap<QString, bool> &flags)
{
    ui->treeFlags->blockSignals(true);
    for (QTreeWidgetItem *leaf : leafItems)
    {
        const QString key = leaf->data(0, FlagKeyRole).toString();
        leaf->setCheckState(0, flags.value(key, false) ? Qt::Checked : Qt::Unchecked);
    }
    ui->treeFlags->blockSignals(false);
}

void windowsettings::applySettings(const AutoGenSettings &settings)
{
    ui->tabMode->setCurrentIndex(settings.mode == AutoGenMode::Collage ? 0 : 1);
    ui->checkExtended->setChecked(settings.extendedFormat);
    if (settings.mode == AutoGenMode::Collage)
        ui->editCollagePath->setText(settings.path);
    else
        ui->editImagesPath->setText(settings.path);
    ui->spinHeight->setValue(qMax(1, settings.height));
    ui->spinWidth->setValue(qMax(1, settings.width));
    ui->spinNumber->setValue(qMax(1, settings.number));
    setLeafFlags(settings.isRndList);
    fillDefaultPaths();
}

void windowsettings::updateWindowTitle()
{
    if (presetName.isEmpty())
        setWindowTitle(tr("Autogen"));
    else
        setWindowTitle(tr("Autogen — %1").arg(presetName));
}

void windowsettings::filterFlags(const QString &query)
{
    const QString q = query.trimmed();
    for (QTreeWidgetItem *leaf : leafItems)
    {
        const QString key = leaf->data(0, FlagKeyRole).toString();
        const bool match = q.isEmpty()
                           || leaf->text(0).contains(q, Qt::CaseInsensitive)
                           || key.contains(q, Qt::CaseInsensitive);
        leaf->setHidden(!match);
    }
    if (!rootItem)
        return;
    for (int g = 0; g < rootItem->childCount(); ++g)
    {
        QTreeWidgetItem *group = rootItem->child(g);
        bool anyVisible = false;
        for (int i = 0; i < group->childCount(); ++i)
        {
            if (!group->child(i)->isHidden())
                anyVisible = true;
        }
        group->setHidden(!q.isEmpty() && !anyVisible);
    }
    rootItem->setHidden(false);
}

void windowsettings::invertFlags()
{
    for (QTreeWidgetItem *leaf : leafItems)
    {
        const bool on = leaf->checkState(0) == Qt::Checked;
        leaf->setCheckState(0, on ? Qt::Unchecked : Qt::Checked);
    }
}

void windowsettings::askCollageFile()
{
    const QString chosen = QFileDialog::getSaveFileName(
        this, tr("Save collage"),
        ui->editCollagePath->text().isEmpty()
            ? startPath(AppKeys::dirAutogen, QStringLiteral("collage.png"))
            : ui->editCollagePath->text(),
        tr("Image (*.png);;All files (*.*)"));
    if (chosen.isEmpty())
        return;
    rememberPath(AppKeys::dirAutogen, chosen);
    ui->editCollagePath->setText(chosen);
}

void windowsettings::askImagesDir()
{
    const QString chosen = QFileDialog::getExistingDirectory(
        this, tr("Save images"),
        ui->editImagesPath->text().isEmpty() ? startPath(AppKeys::dirAutogen)
                                             : ui->editImagesPath->text());
    if (chosen.isEmpty())
        return;
    rememberPath(AppKeys::dirAutogen, chosen);
    ui->editImagesPath->setText(chosen);
}

void windowsettings::loadPreset()
{
    const QString chosen = QFileDialog::getOpenFileName(
        this, tr("Load autogen preset"),
        startPath(AppKeys::dirAutogen),
        tr("Autogen (*.autogen);;All files (*.*)"));
    if (chosen.isEmpty())
        return;
    AutoGenSettings loaded;
    if (!loaded.Load(chosen))
    {
        QMessageBox::warning(this, tr("Autogen"), tr("Unable to load autogen preset."));
        return;
    }
    rememberPath(AppKeys::dirAutogen, chosen);
    applySettings(loaded);
    presetName = QFileInfo(chosen).fileName();
    updateWindowTitle();
}

void windowsettings::savePreset()
{
    QString chosen = QFileDialog::getSaveFileName(
        this, tr("Save autogen preset"),
        startPath(AppKeys::dirAutogen, presetName.isEmpty() ? QStringLiteral("preset.autogen") : presetName),
        tr("Autogen (*.autogen);;All files (*.*)"));
    if (chosen.isEmpty())
        return;
    if (!chosen.endsWith(QStringLiteral(".autogen"), Qt::CaseInsensitive))
        chosen += QStringLiteral(".autogen");
    const AutoGenSettings current = collectSettings();
    if (!current.Save(chosen))
    {
        QMessageBox::warning(this, tr("Autogen"), tr("Unable to save autogen preset."));
        return;
    }
    rememberPath(AppKeys::dirAutogen, chosen);
    presetName = QFileInfo(chosen).fileName();
    updateWindowTitle();
}

void windowsettings::openOutputFolder()
{
    if (lastOutputPath.isEmpty())
        return;
    QFileInfo info(lastOutputPath);
    const QString folder = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
}

void windowsettings::onRun()
{
    if (busy)
        return;
    resultSettings = collectSettings();
    if (resultSettings.path.isEmpty())
    {
        fillDefaultPaths();
        resultSettings = collectSettings();
    }
    if (resultSettings.path.isEmpty())
    {
        QMessageBox::warning(this, tr("Autogen"),
                             resultSettings.mode == AutoGenMode::Collage
                                 ? tr("Choose a file for the collage.")
                                 : tr("Choose a folder for the images."));
        return;
    }
    emit runRequested();
}

void windowsettings::onStop()
{
    emit stopRequested();
}

void windowsettings::onClose()
{
    if (busy)
        return;
    saveSession();
    reject();
}

void windowsettings::setBusy(bool on)
{
    busy = on;
    ui->tabMode->setEnabled(!on);
    ui->checkExtended->setEnabled(!on);
    ui->editFlagFilter->setEnabled(!on);
    ui->treeFlags->setEnabled(!on);
    ui->btnInvert->setEnabled(!on);
    ui->btnLoad->setEnabled(!on);
    ui->btnSave->setEnabled(!on);
    ui->btnRun->setEnabled(!on);
    ui->btnClose->setEnabled(!on);
    ui->btnOpenFolder->setEnabled(!on && !lastOutputPath.isEmpty());
    ui->btnStop->setEnabled(on);
    if (on)
    {
        ui->progressBar->setValue(0);
        lastProgressCaption.clear();
        showRandomProgressCaption();
        if (progressMessages.isEmpty())
            ui->labelProgress->setText(tr("Autogen progress"));
        else if (progressCaptionTimer)
            progressCaptionTimer->start();
    }
    else if (progressCaptionTimer)
    {
        progressCaptionTimer->stop();
    }
}

void windowsettings::setProgress(int done, int total)
{
    const int t = qMax(0, total);
    ui->labelCount->setText(QStringLiteral("%1 / %2").arg(done).arg(t));
    if (t <= 0)
        ui->progressBar->setValue(0);
    else
        ui->progressBar->setValue(qBound(0, qRound(100.0 * done / t), 100));
}

void windowsettings::showRandomProgressCaption()
{
    if (progressMessages.isEmpty())
        return;
    QString next = progressMessages.at(QRandomGenerator::global()->bounded(progressMessages.size()));
    if (progressMessages.size() > 1)
    {
        int guard = 0;
        while (next == lastProgressCaption && guard++ < 8)
            next = progressMessages.at(QRandomGenerator::global()->bounded(progressMessages.size()));
    }
    lastProgressCaption = next;
    ui->labelProgress->setText(QCoreApplication::translate("AutogenProgress", next.toUtf8().constData()));
}

void windowsettings::markFinished(bool ok)
{
    setBusy(false);
    if (ok)
    {
        ui->progressBar->setValue(100);
        ui->labelProgress->setText(tr("Ready"));
        ui->btnOpenFolder->setEnabled(!lastOutputPath.isEmpty());
    }
}

void windowsettings::setLastOutputPath(const QString &path)
{
    lastOutputPath = path;
}

void windowsettings::saveSession() const
{
    const AutoGenSettings current = collectSettings();
    QJsonObject object = current.JSON_serialize();
    object.insert(QStringLiteral("presetName"), presetName);
    object.insert(QStringLiteral("collagePath"), ui->editCollagePath->text());
    object.insert(QStringLiteral("imagesPath"), ui->editImagesPath->text());
    QSettings st(appSettingsFile(), QSettings::IniFormat);
    st.setValue(QLatin1String(AppKeys::autogenSession),
                QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact)));
}

void windowsettings::restoreSession()
{
    const QString raw = QSettings(appSettingsFile(), QSettings::IniFormat)
                            .value(QLatin1String(AppKeys::autogenSession)).toString();
    if (raw.isEmpty())
        return;
    const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
    if (!doc.isObject())
        return;
    AutoGenSettings loaded;
    loaded.JSON_deserialize(doc.object());
    applySettings(loaded);
    const QString collagePath = doc.object().value(QStringLiteral("collagePath")).toString();
    const QString imagesPath = doc.object().value(QStringLiteral("imagesPath")).toString();
    if (!collagePath.isEmpty())
        ui->editCollagePath->setText(collagePath);
    if (!imagesPath.isEmpty())
        ui->editImagesPath->setText(imagesPath);
    presetName = doc.object().value(QStringLiteral("presetName")).toString();
    updateWindowTitle();
}

void windowsettings::closeEvent(QCloseEvent *event)
{
    if (busy)
    {
        event->ignore();
        return;
    }
    saveSession();
    QDialog::closeEvent(event);
}

void windowsettings::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QDialog::changeEvent(event);
}
