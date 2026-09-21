QT       += core gui widgets

CONFIG += c++11

TARGET = GodOfPixels3
TEMPLATE = app

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/app \
    $$PWD/src/ui \
    $$PWD/src/planet \
    $$PWD/src/gen \
    $$PWD/src/render \
    $$PWD/src/achievements

SOURCES += \
    src/app/main.cpp \
    src/app/mainwindow.cpp \
    src/app/planetcommands.cpp \
    src/achievements/achievement.cpp \
    src/achievements/achievementcatalog.cpp \
    src/achievements/achievementcontext.cpp \
    src/achievements/achievementengine.cpp \
    src/achievements/achievementrules.cpp \
    src/achievements/achievementstore.cpp \
    src/ui/autogensettings.cpp \
    src/ui/achievementsdialog.cpp \
    src/ui/achievementtoast.cpp \
    src/ui/colorswatch.cpp \
    src/ui/collapsiblepanel.cpp \
    src/ui/multislider.cpp \
    src/ui/previewpanel.cpp \
    src/ui/programsettingsdialog.cpp \
    src/ui/settingspanel.cpp \
    src/ui/spectrumdialog.cpp \
    src/ui/windowsettings.cpp \
    src/planet/planet.cpp \
    src/planet/planet_assets.cpp \
    src/planet/planet_cards.cpp \
    src/planet/planet_climate.cpp \
    src/planet/planet_clouds.cpp \
    src/planet/planet_faults.cpp \
    src/planet/planet_life.cpp \
    src/planet/planet_name.cpp \
    src/planet/planet_rings.cpp \
    src/planet/planet_surface.cpp \
    src/planet/planet_tags.cpp \
    src/planet/planet_terrain.cpp \
    src/gen/coloremotion.cpp \
    src/gen/cloudfactory.cpp \
    src/gen/facts.cpp \
    src/gen/noise3d.cpp \
    src/gen/planetsettings.cpp \
    src/gen/starspectrum.cpp \
    src/gen/terrafactory.cpp \
    src/render/planetglwidget.cpp

HEADERS += \
    src/app/appsettings.h \
    src/app/global.h \
    src/app/mainwindow.h \
    src/app/planetcommands.h \
    src/achievements/achievement.h \
    src/achievements/achievementcatalog.h \
    src/achievements/achievementcontext.h \
    src/achievements/achievementengine.h \
    src/achievements/achievementrules.h \
    src/achievements/achievementstore.h \
    src/ui/autogensettings.h \
    src/ui/achievementsdialog.h \
    src/ui/achievementtoast.h \
    src/ui/colorswatch.h \
    src/ui/collapsiblepanel.h \
    src/ui/multislider.h \
    src/ui/previewpanel.h \
    src/ui/programsettingsdialog.h \
    src/ui/settingspanel.h \
    src/ui/spectrumdialog.h \
    src/ui/windowsettings.h \
    src/planet/planet.h \
    src/planet/planet_p.h \
    src/gen/coloremotion.h \
    src/gen/cloudfactory.h \
    src/gen/facts.h \
    src/gen/noise3d.h \
    src/gen/planetsettings.h \
    src/gen/starspectrum.h \
    src/gen/spheremath.h \
    src/gen/terrafactory.h \
    src/render/planetglwidget.h

TRANSLATIONS += translations/QtLanguage_ru.ts

FORMS += \
    src/app/mainwindow.ui \
    src/ui/windowsettings.ui

RESOURCES += \
    resources.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
