/********************************************************************************
** Form generated from reading UI file 'windowsettings.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WINDOWSETTINGS_H
#define UI_WINDOWSETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_windowsettings
{
public:
    QVBoxLayout *rootLayout;
    QHBoxLayout *mainRow;
    QVBoxLayout *leftCol;
    QTabWidget *tabMode;
    QWidget *tabCollage;
    QVBoxLayout *collageLayout;
    QHBoxLayout *collagePathRow;
    QLineEdit *editCollagePath;
    QPushButton *btnCollageBrowse;
    QFormLayout *collageForm;
    QLabel *labelWidth;
    QSpinBox *spinWidth;
    QLabel *labelHeight;
    QSpinBox *spinHeight;
    QSpacerItem *collageSpacer;
    QWidget *tabImages;
    QVBoxLayout *imagesLayout;
    QHBoxLayout *imagesPathRow;
    QLineEdit *editImagesPath;
    QPushButton *btnImagesBrowse;
    QFormLayout *imagesForm;
    QLabel *labelNumber;
    QSpinBox *spinNumber;
    QSpacerItem *imagesSpacer;
    QCheckBox *checkExtended;
    QVBoxLayout *rightCol;
    QLineEdit *editFlagFilter;
    QTreeWidget *treeFlags;
    QPushButton *btnInvert;
    QHBoxLayout *statusRow;
    QLabel *labelProgress;
    QLabel *labelCount;
    QProgressBar *progressBar;
    QHBoxLayout *buttonRow;
    QPushButton *btnLoad;
    QPushButton *btnSave;
    QPushButton *btnOpenFolder;
    QSpacerItem *buttonSpacer;
    QPushButton *btnRun;
    QPushButton *btnStop;
    QPushButton *btnClose;

    void setupUi(QDialog *windowsettings)
    {
        if (windowsettings->objectName().isEmpty())
            windowsettings->setObjectName(QString::fromUtf8("windowsettings"));
        windowsettings->resize(820, 560);
        windowsettings->setMinimumSize(QSize(720, 480));
        rootLayout = new QVBoxLayout(windowsettings);
        rootLayout->setSpacing(10);
        rootLayout->setObjectName(QString::fromUtf8("rootLayout"));
        mainRow = new QHBoxLayout();
        mainRow->setSpacing(12);
        mainRow->setObjectName(QString::fromUtf8("mainRow"));
        leftCol = new QVBoxLayout();
        leftCol->setObjectName(QString::fromUtf8("leftCol"));
        tabMode = new QTabWidget(windowsettings);
        tabMode->setObjectName(QString::fromUtf8("tabMode"));
        tabCollage = new QWidget();
        tabCollage->setObjectName(QString::fromUtf8("tabCollage"));
        collageLayout = new QVBoxLayout(tabCollage);
        collageLayout->setObjectName(QString::fromUtf8("collageLayout"));
        collagePathRow = new QHBoxLayout();
        collagePathRow->setObjectName(QString::fromUtf8("collagePathRow"));
        editCollagePath = new QLineEdit(tabCollage);
        editCollagePath->setObjectName(QString::fromUtf8("editCollagePath"));

        collagePathRow->addWidget(editCollagePath);

        btnCollageBrowse = new QPushButton(tabCollage);
        btnCollageBrowse->setObjectName(QString::fromUtf8("btnCollageBrowse"));
        btnCollageBrowse->setMaximumWidth(56);

        collagePathRow->addWidget(btnCollageBrowse);


        collageLayout->addLayout(collagePathRow);

        collageForm = new QFormLayout();
        collageForm->setObjectName(QString::fromUtf8("collageForm"));
        labelWidth = new QLabel(tabCollage);
        labelWidth->setObjectName(QString::fromUtf8("labelWidth"));

        collageForm->setWidget(0, QFormLayout::LabelRole, labelWidth);

        spinWidth = new QSpinBox(tabCollage);
        spinWidth->setObjectName(QString::fromUtf8("spinWidth"));
        spinWidth->setMinimum(1);
        spinWidth->setMaximum(99);
        spinWidth->setValue(2);

        collageForm->setWidget(0, QFormLayout::FieldRole, spinWidth);

        labelHeight = new QLabel(tabCollage);
        labelHeight->setObjectName(QString::fromUtf8("labelHeight"));

        collageForm->setWidget(1, QFormLayout::LabelRole, labelHeight);

        spinHeight = new QSpinBox(tabCollage);
        spinHeight->setObjectName(QString::fromUtf8("spinHeight"));
        spinHeight->setMinimum(1);
        spinHeight->setMaximum(99);
        spinHeight->setValue(2);

        collageForm->setWidget(1, QFormLayout::FieldRole, spinHeight);


        collageLayout->addLayout(collageForm);

        collageSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        collageLayout->addItem(collageSpacer);

        tabMode->addTab(tabCollage, QString());
        tabImages = new QWidget();
        tabImages->setObjectName(QString::fromUtf8("tabImages"));
        imagesLayout = new QVBoxLayout(tabImages);
        imagesLayout->setObjectName(QString::fromUtf8("imagesLayout"));
        imagesPathRow = new QHBoxLayout();
        imagesPathRow->setObjectName(QString::fromUtf8("imagesPathRow"));
        editImagesPath = new QLineEdit(tabImages);
        editImagesPath->setObjectName(QString::fromUtf8("editImagesPath"));

        imagesPathRow->addWidget(editImagesPath);

        btnImagesBrowse = new QPushButton(tabImages);
        btnImagesBrowse->setObjectName(QString::fromUtf8("btnImagesBrowse"));
        btnImagesBrowse->setMaximumWidth(56);

        imagesPathRow->addWidget(btnImagesBrowse);


        imagesLayout->addLayout(imagesPathRow);

        imagesForm = new QFormLayout();
        imagesForm->setObjectName(QString::fromUtf8("imagesForm"));
        labelNumber = new QLabel(tabImages);
        labelNumber->setObjectName(QString::fromUtf8("labelNumber"));

        imagesForm->setWidget(0, QFormLayout::LabelRole, labelNumber);

        spinNumber = new QSpinBox(tabImages);
        spinNumber->setObjectName(QString::fromUtf8("spinNumber"));
        spinNumber->setMinimum(1);
        spinNumber->setMaximum(9999);
        spinNumber->setValue(10);

        imagesForm->setWidget(0, QFormLayout::FieldRole, spinNumber);


        imagesLayout->addLayout(imagesForm);

        imagesSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        imagesLayout->addItem(imagesSpacer);

        tabMode->addTab(tabImages, QString());

        leftCol->addWidget(tabMode);

        checkExtended = new QCheckBox(windowsettings);
        checkExtended->setObjectName(QString::fromUtf8("checkExtended"));

        leftCol->addWidget(checkExtended);


        mainRow->addLayout(leftCol);

        rightCol = new QVBoxLayout();
        rightCol->setObjectName(QString::fromUtf8("rightCol"));
        editFlagFilter = new QLineEdit(windowsettings);
        editFlagFilter->setObjectName(QString::fromUtf8("editFlagFilter"));

        rightCol->addWidget(editFlagFilter);

        treeFlags = new QTreeWidget(windowsettings);
        treeFlags->setObjectName(QString::fromUtf8("treeFlags"));
        treeFlags->setHeaderHidden(true);
        treeFlags->setRootIsDecorated(true);
        treeFlags->setItemsExpandable(true);
        treeFlags->setUniformRowHeights(true);

        rightCol->addWidget(treeFlags);

        btnInvert = new QPushButton(windowsettings);
        btnInvert->setObjectName(QString::fromUtf8("btnInvert"));

        rightCol->addWidget(btnInvert);


        mainRow->addLayout(rightCol);

        mainRow->setStretch(0, 3);
        mainRow->setStretch(1, 2);

        rootLayout->addLayout(mainRow);

        statusRow = new QHBoxLayout();
        statusRow->setObjectName(QString::fromUtf8("statusRow"));
        labelProgress = new QLabel(windowsettings);
        labelProgress->setObjectName(QString::fromUtf8("labelProgress"));

        statusRow->addWidget(labelProgress);

        labelCount = new QLabel(windowsettings);
        labelCount->setObjectName(QString::fromUtf8("labelCount"));
        labelCount->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

        statusRow->addWidget(labelCount);


        rootLayout->addLayout(statusRow);

        progressBar = new QProgressBar(windowsettings);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);
        progressBar->setAlignment(Qt::AlignCenter);
        progressBar->setTextVisible(true);

        rootLayout->addWidget(progressBar);

        buttonRow = new QHBoxLayout();
        buttonRow->setObjectName(QString::fromUtf8("buttonRow"));
        btnLoad = new QPushButton(windowsettings);
        btnLoad->setObjectName(QString::fromUtf8("btnLoad"));

        buttonRow->addWidget(btnLoad);

        btnSave = new QPushButton(windowsettings);
        btnSave->setObjectName(QString::fromUtf8("btnSave"));

        buttonRow->addWidget(btnSave);

        btnOpenFolder = new QPushButton(windowsettings);
        btnOpenFolder->setObjectName(QString::fromUtf8("btnOpenFolder"));
        btnOpenFolder->setEnabled(false);

        buttonRow->addWidget(btnOpenFolder);

        buttonSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonRow->addItem(buttonSpacer);

        btnRun = new QPushButton(windowsettings);
        btnRun->setObjectName(QString::fromUtf8("btnRun"));

        buttonRow->addWidget(btnRun);

        btnStop = new QPushButton(windowsettings);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        btnStop->setEnabled(false);

        buttonRow->addWidget(btnStop);

        btnClose = new QPushButton(windowsettings);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));

        buttonRow->addWidget(btnClose);


        rootLayout->addLayout(buttonRow);


        retranslateUi(windowsettings);

        tabMode->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(windowsettings);
    } // setupUi

    void retranslateUi(QDialog *windowsettings)
    {
        windowsettings->setWindowTitle(QCoreApplication::translate("windowsettings", "Autogen", nullptr));
        btnCollageBrowse->setText(QCoreApplication::translate("windowsettings", "...", nullptr));
        labelWidth->setText(QCoreApplication::translate("windowsettings", "width", nullptr));
        labelHeight->setText(QCoreApplication::translate("windowsettings", "height", nullptr));
        tabMode->setTabText(tabMode->indexOf(tabCollage), QCoreApplication::translate("windowsettings", "Collage", nullptr));
        btnImagesBrowse->setText(QCoreApplication::translate("windowsettings", "...", nullptr));
        labelNumber->setText(QCoreApplication::translate("windowsettings", "number", nullptr));
        tabMode->setTabText(tabMode->indexOf(tabImages), QCoreApplication::translate("windowsettings", "Images", nullptr));
        checkExtended->setText(QCoreApplication::translate("windowsettings", "extended format", nullptr));
        editFlagFilter->setPlaceholderText(QCoreApplication::translate("windowsettings", "Filter flags", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeFlags->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("windowsettings", "Do random", nullptr));
        btnInvert->setText(QCoreApplication::translate("windowsettings", "Invert", nullptr));
        labelProgress->setText(QCoreApplication::translate("windowsettings", "Autogen progress", nullptr));
        labelCount->setText(QCoreApplication::translate("windowsettings", "0 / 0", nullptr));
        btnLoad->setText(QCoreApplication::translate("windowsettings", "Load", nullptr));
        btnSave->setText(QCoreApplication::translate("windowsettings", "Save", nullptr));
        btnOpenFolder->setText(QCoreApplication::translate("windowsettings", "Open folder", nullptr));
        btnRun->setText(QCoreApplication::translate("windowsettings", "Run", nullptr));
        btnStop->setText(QCoreApplication::translate("windowsettings", "Stop", nullptr));
        btnClose->setText(QCoreApplication::translate("windowsettings", "Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class windowsettings: public Ui_windowsettings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WINDOWSETTINGS_H
