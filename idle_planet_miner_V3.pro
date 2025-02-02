# idle_planet_miner_V3.pro
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console
TARGET = idle_planet_miner_V3
TEMPLATE = app

SOURCES += idle_planet_miner_V3.cpp

# If json.hpp is not in the same directory, add the include path:
# INCLUDEPATH += path/to/json
