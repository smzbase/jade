QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = jade
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

INCLUDEPATH += source

SOURCES += \
    source/DrawingItem.cpp \
    source/DrawingItemContainer.cpp \
    source/DrawingItemPoint.cpp \
    source/MainWindow.cpp \
	source/main.cpp

HEADERS += \
    source/DrawingItem.h \
    source/DrawingItemContainer.h \
    source/DrawingItemPoint.h \
	source/MainWindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
