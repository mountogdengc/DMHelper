# Standalone Qt Test project for VisionCalculator.
#
# Usage:
#   cd DMHelper/test/vision
#   qmake && make
#   ./vision
#
# Intentionally separate from DMHelper.pro so the geometry engine can be
# validated without linking the full desktop app.

QT       += testlib core gui
CONFIG   += c++11 testcase console
CONFIG   -= app_bundle

TARGET   = vision
TEMPLATE = app

SOURCES += \
    tst_visioncalculator.cpp \
    ../../src/visioncalculator.cpp

HEADERS += \
    ../../src/visioncalculator.h

INCLUDEPATH += ../../src
