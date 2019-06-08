TEMPLATE = app

win32:TARGET = Jade
!win32:TARGET = jade

DESTDIR = bin
INCLUDEPATH += source

CONFIG += release warn_on embed_manifest_dll c++11 qt
CONFIG -= debug
QT += widgets svg

!win32:MOC_DIR = release
!win32:OBJECTS_DIR = release
!win32:RCC_DIR = release

# QuaZIP
INCLUDEPATH += C:/Development/quazip-0.7.1/quazip $$[QT_INSTALL_HEADERS]/QtZlib
LIBS += -LC:/Development/quazip-0.7.1/quazip/release/ -lquazip

# --------------------------------------------------------------------------------------------------

SOURCES += \
	source/AboutDialog.cpp \
	source/DrawingArrow.cpp \
	source/DrawingCurveItem.cpp \
	source/DrawingEllipseItem.cpp \
	source/DrawingItem.cpp \
	source/DrawingItemGroup.cpp \
	source/DrawingItemPoint.cpp \
	source/DrawingLineItem.cpp \
	source/DrawingPathItem.cpp \
	source/DrawingPolygonItem.cpp \
	source/DrawingPolylineItem.cpp \
	source/DrawingReader.cpp \
	source/DrawingRectItem.cpp \
	source/DrawingTextEllipseItem.cpp \
	source/DrawingTextItem.cpp \
	source/DrawingTextRectItem.cpp \
	source/DrawingUndo.cpp \
	source/DrawingWidget.cpp \
	source/DrawingWidgetBase.cpp \
	source/DrawingWriter.cpp \
	source/DynamicPropertiesWidget.cpp \
	source/ElectricItems.cpp \
	source/HelperWidgets.cpp \
	source/LogicItems.cpp \
	source/MainWindow.cpp \
	source/PreferencesDialog.cpp \
	source/VsdxWriter.cpp \
	source/main.cpp

HEADERS += \
	source/AboutDialog.h \
	source/DrawingArrow.h \
	source/DrawingCurveItem.h \
	source/DrawingEllipseItem.h \
	source/DrawingItem.h \
	source/DrawingItemGroup.h \
	source/DrawingItemPoint.h \
	source/DrawingLineItem.h \
	source/DrawingPathItem.h \
	source/DrawingPolygonItem.h \
	source/DrawingPolylineItem.h \
	source/DrawingReader.h \
	source/DrawingRectItem.h \
	source/DrawingTextEllipseItem.h \
	source/DrawingTextItem.h \
	source/DrawingTextRectItem.h \
	source/DrawingUndo.h \
	source/DrawingWidget.h \
	source/DrawingWidgetBase.h \
	source/DrawingWriter.h \
	source/DynamicPropertiesWidget.h \
	source/ElectricItems.h \
	source/HelperWidgets.h \
	source/LogicItems.h \
	source/MainWindow.h \
	source/PreferencesDialog.h \
	source/VsdxWriter.h

RESOURCES += icons/icons.qrc

RC_FILE = icons/icons.rc
OTHER_FILES += icons/icons.rc
