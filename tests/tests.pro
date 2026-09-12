QT += testlib widgets
CONFIG += console testcase c++11
CONFIG -= app_bundle

TARGET = gop3_tests
TEMPLATE = app

INCLUDEPATH += \
    $$PWD/.. \
    $$PWD/../src/app \
    $$PWD/../src/ui \
    $$PWD/../src/planet \
    $$PWD/../src/gen \
    $$PWD/../src/render

SOURCES += \
    test_planet.cpp \
    ../src/planet/planet.cpp \
    ../src/planet/planet_assets.cpp \
    ../src/planet/planet_cards.cpp \
    ../src/planet/planet_climate.cpp \
    ../src/planet/planet_clouds.cpp \
    ../src/planet/planet_life.cpp \
    ../src/planet/planet_name.cpp \
    ../src/planet/planet_rings.cpp \
    ../src/planet/planet_surface.cpp \
    ../src/planet/planet_terrain.cpp \
    ../src/gen/terrafactory.cpp \
    ../src/gen/noise3d.cpp \
    ../src/gen/cloudfactory.cpp \
    ../src/gen/planetsettings.cpp \
    ../src/gen/facts.cpp \
    ../src/ui/autogensettings.cpp

HEADERS += \
    ../src/planet/planet.h \
    ../src/planet/planet_p.h \
    ../src/gen/terrafactory.h \
    ../src/gen/noise3d.h \
    ../src/gen/cloudfactory.h \
    ../src/gen/planetsettings.h \
    ../src/gen/facts.h \
    ../src/gen/spheremath.h \
    ../src/app/global.h \
    ../src/ui/autogensettings.h

RESOURCES += ../resources.qrc
