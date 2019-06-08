/* MainWindow.h
 *
 * Copyright (C) 2019- Jason Allen
 *
 * This file is part of the jade application.
 *
 * jade is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * jade is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with jade.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <DrawingWidgetBase.h>

class DynamicPropertiesWidget;

class DrawingPathItem;
class DrawingWidget;

class QActionGroup;
class QCloseEvent;
class QComboBox;
class QHideEvent;
class QLabel;
class QShowEvent;
class QStackedWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	enum ActionIndex { NewAction, OpenAction, SaveAction, SaveAsAction, CloseAction,
		ExportPngAction, ExportSvgAction, ExportVsdxAction,
		PreferencesAction, ExitAction,
		AboutAction, AboutQtAction, NumberOfActions };
	enum ModeActionIndex { DefaultModeAction, ScrollModeAction, ZoomModeAction,
	   PlaceCurveAction, PlaceEllipseAction, PlaceLineAction,
	   PlacePolygonAction, PlacePolylineAction, PlaceRectAction, PlaceTextAction,
	   PlaceTextRectAction, PlaceTextEllipseAction };

private:
	DrawingWidget* mDrawingWidget;
	QStackedWidget* mStackedWidget;

	QComboBox* mZoomCombo;

	DynamicPropertiesWidget* mPropertiesWidget;
	QDockWidget* mPropertiesDock;

	QLabel* mModeLabel;
	QLabel* mModifiedLabel;
	QLabel* mNumberOfItemsLabel;
	QLabel* mMouseInfoLabel;

	QActionGroup* mModeActionGroup;
	QList<DrawingPathItem*> mPathItems;
	QAction* mElectricItemsAction;
	QAction* mLogicItemsAction;

	QHash<QString,QVariant> mDrawingDefaultProperties;
	QHash<QString,QVariant> mItemDefaultProperties;

	QString mFilePath;
	int mNewDrawingCount;
	QDir mWorkingDir;
	QByteArray mWindowState;

	qreal mExportPngSvgScale;
	QString mExportVsdxUnits;
	qreal mExportVsdxScale;

public:
	MainWindow(const QString& filePath = QString());
	~MainWindow();

	void loadSettings();
	void saveSettings();

public slots:
	bool newDrawing();
	bool openDrawing();
	bool saveDrawing();
	bool saveDrawingAs();
	bool closeDrawing();

	void exportPng();
	void exportSvg();
	void exportVsdx();

	void preferences();
	void about();

private slots:
	void setModeFromAction(QAction* action);
	void setModeFromDrawing(DrawingWidgetBase::Mode mode);

	void setZoomComboText(qreal scale);
	void setZoomLevel(const QString& text);

	void setItemDefaultProperties(const QHash<QString,QVariant>& properties);

	void setModeText(DrawingWidgetBase::Mode mode);
	void setModifiedText(bool clean);
	void setNumberOfItemsText(int itemCount);

private:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
	void closeEvent(QCloseEvent* event);

	bool saveDrawingToFile(const QString& filePath);
	bool loadDrawingFromFile(const QString& filePath);

	void showDrawing();
	void hideDrawing();
	bool isDrawingVisible() const;

	void setActionsEnabled(bool enable);
	void setWindowTitle(const QString& filePath);

	void createPropertiesDock();
	void createStatusBar();

	void createActions();
	void createMenus();
	void createToolBars();
	void addAction(const QString& text, QObject* slotObj, const char* slotFunction,
		const QString& iconPath = QString(), const QString& shortcut = QString());
	QAction* addModeAction(const QString& text,
		const QString& iconPath = QString(), const QString& shortcut = QString());
	QAction* addPathItems(const QString& name, const QList<DrawingPathItem*>& items,
		const QStringList& icons);
};

#endif
