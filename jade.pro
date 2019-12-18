QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = jade
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
	source/DrawingItemPoint.cpp \
	source/MainWindow.cpp \
	source/main.cpp

HEADERS += \
	source/DrawingItemPoint.h \
	source/MainWindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
