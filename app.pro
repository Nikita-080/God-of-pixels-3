QT       += core gui widgets

CONFIG += c++11

TARGET = GodOfPixels3
TEMPLATE = app

SOURCES += \
    autogensettings.cpp \
    cloudfactory.cpp \
    colorswatch.cpp \
    facts.cpp \
    main.cpp \
    mainwindow.cpp \
    multislider.cpp \
    noise3d.cpp \
    planet.cpp \
    planetglwidget.cpp \
    planetsettings.cpp \
    previewpanel.cpp \
    settingspanel.cpp \
    terrafactory.cpp \
    windowsettings.cpp

HEADERS += \
    appsettings.h \
    autogensettings.h \
    cloudfactory.h \
    colorswatch.h \
    facts.h \
    global.h \
    mainwindow.h \
    multislider.h \
    noise3d.h \
    planet.h \
    planetglwidget.h \
    planetsettings.h \
    previewpanel.h \
    settingspanel.h \
    spheremath.h \
    terrafactory.h \
    windowsettings.h

TRANSLATIONS += QtLanguage_ru.ts

FORMS += \
    mainwindow.ui \
    windowsettings.ui

RESOURCES += \
    resources.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
