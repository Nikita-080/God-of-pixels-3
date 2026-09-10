QT += testlib widgets
CONFIG += console testcase c++11
CONFIG -= app_bundle

TARGET = gop3_tests
TEMPLATE = app

INCLUDEPATH += $$PWD/..

SOURCES += \
    test_planet.cpp \
    ../planet.cpp \
    ../terrafactory.cpp \
    ../noise3d.cpp \
    ../cloudfactory.cpp \
    ../planetsettings.cpp \
    ../facts.cpp \
    ../autogensettings.cpp

HEADERS += \
    ../planet.h \
    ../terrafactory.h \
    ../noise3d.h \
    ../cloudfactory.h \
    ../planetsettings.h \
    ../facts.h \
    ../spheremath.h \
    ../global.h

RESOURCES += ../resources.qrc
