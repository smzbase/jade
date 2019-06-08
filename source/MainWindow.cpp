/* MainWindow.cpp
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

#include "MainWindow.h"
#include "AboutDialog.h"
#include "DrawingWidget.h"
#include "DrawingCurveItem.h"
#include "DrawingEllipseItem.h"
#include "DrawingLineItem.h"
#include "DrawingPathItem.h"
#include "DrawingPolygonItem.h"
#include "DrawingPolylineItem.h"
#include "DrawingReader.h"
#include "DrawingRectItem.h"
#include "DrawingTextItem.h"
#include "DrawingTextRectItem.h"
#include "DrawingTextEllipseItem.h"
#include "DrawingWriter.h"
#include "DynamicPropertiesWidget.h"
#include "ElectricItems.h"
#include "LogicItems.h"
#include "PreferencesDialog.h"
#include "VsdxWriter.h"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QSettings>
#include <QShowEvent>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSvgGenerator>
#include <QToolBar>

MainWindow::MainWindow(const QString& filePath) : QMainWindow()
{
	mNewDrawingCount = 0;
#ifndef WIN32
	mWorkingDir = QDir::home();
#endif

	mExportPngSvgScale = 5;
	mExportVsdxUnits = "in";
	mExportVsdxScale = 1000;

	QMainWindow::setWindowTitle("Jade");
	setWindowIcon(QIcon(":/icons/jade/diagram.png"));
	resize(1300, 760);

	mDrawingWidget = new DrawingWidget();

	mStackedWidget = new QStackedWidget();
	mStackedWidget->addWidget(new QWidget());
	mStackedWidget->addWidget(mDrawingWidget);
	mStackedWidget->setCurrentIndex(0);
	setCentralWidget(mStackedWidget);

	createPropertiesDock();
	createStatusBar();

	createActions();
	createMenus();
	createToolBars();

	loadSettings();

	if (!filePath.isEmpty() && loadDrawingFromFile(filePath)) showDrawing();
	else newDrawing();
}

MainWindow::~MainWindow()
{
	while (!mPathItems.isEmpty()) delete mPathItems.takeFirst();
}

//==================================================================================================

void MainWindow::loadSettings()
{
#ifdef RELEASE_BUILD
#ifdef WIN32
	QString configPath("config.ini");
#else
	QString configPath(QDir::home().absoluteFilePath(".jade/config.ini"));
#endif
#else
	QString configPath("config.ini");
#endif

	QSettings settings(configPath, QSettings::IniFormat);

	settings.beginGroup("Window");
	restoreGeometry(settings.value("geometry", QVariant()).toByteArray());
	restoreState(settings.value("state", QVariant()).toByteArray());
	settings.endGroup();

	settings.beginGroup("DrawingDefaults");
	mDrawingDefaultProperties["scene-rect"] = settings.value("sceneRect", QVariant(QRectF(-5000, -3750, 10000, 7500))).toRectF();
	mDrawingDefaultProperties["background-color"] = settings.value("backgroundColor", QVariant(QColor(255, 255, 255))).value<QColor>();
	mDrawingDefaultProperties["grid"] = settings.value("grid", QVariant(50.0)).toDouble();
	mDrawingDefaultProperties["grid-style"] = settings.value("gridStyle", QVariant(static_cast<uint>(DrawingWidget::GridGraphPaper))).toUInt();
	mDrawingDefaultProperties["grid-color"] = settings.value("gridColor", QVariant(QColor(0, 128, 128))).value<QColor>();
	mDrawingDefaultProperties["grid-spacing-major"] = settings.value("gridSpacingMajor", QVariant(8)).toInt();
	mDrawingDefaultProperties["grid-spacing-minor"] = settings.value("gridSpacingMinor", QVariant(2)).toInt();
	mDrawingDefaultProperties["dynamic-grid"] = settings.value("dynamicGrid", QVariant(0.0)).toDouble();
	settings.endGroup();

	settings.beginGroup("ItemDefaults");
	mItemDefaultProperties["pen-style"] = settings.value("penStyle", static_cast<uint>(Qt::SolidLine)).toUInt();
	mItemDefaultProperties["pen-width"] = settings.value("penWidth", 16.0).toDouble();
	mItemDefaultProperties["pen-color"] = settings.value("penColor", QColor(0, 0, 0)).value<QColor>();
	mItemDefaultProperties["brush-color"] = settings.value("brushColor", QColor(255, 255, 255)).value<QColor>();
	mItemDefaultProperties["font-family"] = settings.value("fontFamily", "Arial").toString();
	mItemDefaultProperties["font-size"] = settings.value("fontSize", 100.0).toDouble();
	mItemDefaultProperties["font-bold"] = settings.value("fontBold", false).toBool();
	mItemDefaultProperties["font-italic"] = settings.value("fontItalic", false).toBool();
	mItemDefaultProperties["font-underline"] = settings.value("fontUnderline", false).toBool();
	mItemDefaultProperties["font-strike-through"] = settings.value("fontStrikeThrough", false).toBool();
	mItemDefaultProperties["text-alignment-horizontal"] = settings.value("textAlignHorizontal", static_cast<uint>(Qt::AlignHCenter)).toUInt();
	mItemDefaultProperties["text-alignment-vertical"] = settings.value("textAlignVertical", static_cast<uint>(Qt::AlignVCenter)).toUInt();
	mItemDefaultProperties["text-color"] = settings.value("textColor", QColor(0, 0, 0)).value<QColor>();
	mItemDefaultProperties["start-arrow-style"] = settings.value("startArrowStyle", static_cast<uint>(DrawingArrow::None)).toUInt();
	mItemDefaultProperties["start-arrow-size"] = settings.value("startArrowSize", 100.0).toDouble();
	mItemDefaultProperties["end-arrow-style"] = settings.value("endArrowStyle", static_cast<uint>(DrawingArrow::None)).toUInt();
	mItemDefaultProperties["end-arrow-size"] = settings.value("endArrowSize", 100.0).toDouble();
	settings.endGroup();

	settings.beginGroup("Export");
	mExportPngSvgScale = settings.value("pngSvgScale", 5).toDouble();
	mExportVsdxUnits = settings.value("vsdxUnits", "in").toString();
	mExportVsdxScale = settings.value("vsdxScale", 1000).toDouble();
	settings.endGroup();

	settings.beginGroup("Recent");
	if (settings.contains("workingDir"))
	{
		QDir newDir(settings.value("workingDir").toString());
		if (newDir.exists()) mWorkingDir = newDir;
	}
	settings.endGroup();

	mPropertiesWidget->setDefaultItemsProperties(mItemDefaultProperties);
	mPropertiesWidget->setDrawingProperties(mDrawingDefaultProperties);
}

void MainWindow::saveSettings()
{
#ifdef RELEASE_BUILD
#ifdef WIN32
	QString configPath("config.ini");
#else
	QString configPath(QDir::home().absoluteFilePath(".jade/config.ini"));
#endif
#else
	QString configPath("config.ini");
#endif

	QSettings settings(configPath, QSettings::IniFormat);

	settings.beginGroup("Window");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.endGroup();

	settings.beginGroup("DrawingDefaults");
	settings.setValue("sceneRect", mDrawingDefaultProperties["scene-rect"].toRectF());
	settings.setValue("backgroundColor", mDrawingDefaultProperties["background-color"].value<QColor>());
	settings.setValue("grid", mDrawingDefaultProperties["grid"].toDouble());
	settings.setValue("gridStyle", mDrawingDefaultProperties["grid-style"].toUInt());
	settings.setValue("gridColor", mDrawingDefaultProperties["grid-color"].value<QColor>());
	settings.setValue("gridSpacingMajor", mDrawingDefaultProperties["grid-spacing-major"].toInt());
	settings.setValue("gridSpacingMinor", mDrawingDefaultProperties["grid-spacing-minor"].toInt());
	settings.setValue("dynamicGrid", mDrawingDefaultProperties["dynamic-grid"].toDouble());
	settings.endGroup();

	settings.beginGroup("ItemDefaults");
	settings.setValue("penStyle", mItemDefaultProperties["pen-style"].toUInt());
	settings.setValue("penWidth", mItemDefaultProperties["pen-width"].toDouble());
	settings.setValue("penColor", mItemDefaultProperties["pen-color"].value<QColor>());
	settings.setValue("brushColor", mItemDefaultProperties["brush-color"].value<QColor>());
	settings.setValue("fontFamily", mItemDefaultProperties["font-family"].toString());
	settings.setValue("fontSize", mItemDefaultProperties["font-size"].toDouble());
	settings.setValue("fontBold", mItemDefaultProperties["font-bold"].toBool());
	settings.setValue("fontItalic", mItemDefaultProperties["font-italic"].toBool());
	settings.setValue("fontUnderline", mItemDefaultProperties["font-underline"].toBool());
	settings.setValue("fontStrikeThrough", mItemDefaultProperties["font-strike-through"].toBool());
	settings.setValue("textAlignHorizontal", mItemDefaultProperties["text-alignment-horizontal"].toUInt());
	settings.setValue("textAlignVertical", mItemDefaultProperties["text-alignment-vertical"].toUInt());
	settings.setValue("textColor", mItemDefaultProperties["text-color"].value<QColor>());
	settings.setValue("startArrowStyle", mItemDefaultProperties["start-arrow-style"].toUInt());
	settings.setValue("startArrowSize", mItemDefaultProperties["start-arrow-size"].toDouble());
	settings.setValue("endArrowStyle", mItemDefaultProperties["end-arrow-style"].toUInt());
	settings.setValue("endArrowSize", mItemDefaultProperties["end-arrow-size"].toDouble());
	settings.endGroup();

	settings.beginGroup("Export");
	settings.setValue("pngSvgScale", mExportPngSvgScale);
	settings.setValue("vsdxUnits", mExportVsdxUnits);
	settings.setValue("vsdxScale", mExportVsdxScale);
	settings.endGroup();

	settings.beginGroup("Recent");
	settings.setValue("workingDir", mWorkingDir.absolutePath());
	settings.endGroup();
}

//==================================================================================================

bool MainWindow::newDrawing()
{
	bool drawingCreated = false;

	if (closeDrawing())
	{
		drawingCreated = true;
		mNewDrawingCount++;

		mFilePath = "Untitled " + QString::number(mNewDrawingCount);

		showDrawing();
	}

	return drawingCreated;
}

bool MainWindow::openDrawing()
{
	bool drawingOpened = false;

	QString filePath = mWorkingDir.path();

	filePath = QFileDialog::getOpenFileName(this, "Open File", filePath,
		"Jade Drawings (*.jdm);;All Files (*)", nullptr);
	if (!filePath.isEmpty())
	{
		QFileInfo fileInfo(filePath);
		mWorkingDir = fileInfo.dir();

		if (closeDrawing())
		{
			drawingOpened = loadDrawingFromFile(filePath);

			if (!drawingOpened)
			{
				QMessageBox::critical(this, "Error Reading File",
					"File could not be read. Please ensure that this file is a valid Jade drawing: " + filePath);

				hideDrawing();
			}
			else showDrawing();
		}
	}

	return drawingOpened;
}

bool MainWindow::saveDrawing()
{
	bool drawingSaved = false;

	if (isDrawingVisible())
	{
		if (!mFilePath.startsWith("Untitled"))
		{
			drawingSaved = saveDrawingToFile(mFilePath);
			if (!drawingSaved)
			{
				QMessageBox::critical(this, "Error Saving File",
					"Unable to open file for saving.  File not saved: " + mFilePath);
			}
			else setWindowTitle(mFilePath);
		}

		else drawingSaved = saveDrawingAs();
	}

	return drawingSaved;
}

bool MainWindow::saveDrawingAs()
{
	bool drawingSaved = false;

	if (isDrawingVisible())
	{
		QString filePath = (mFilePath.startsWith("Untitled")) ? mWorkingDir.path() : mFilePath;

		filePath = QFileDialog::getSaveFileName(this, "Save File", filePath,
			"Jade Drawings (*.jdm);;All Files (*)", nullptr);
		if (!filePath.isEmpty())
		{
			QFileInfo fileInfo(filePath);
			mWorkingDir = fileInfo.dir();

			if (!filePath.endsWith(".jdm", Qt::CaseInsensitive)) filePath += ".jdm";

			drawingSaved = saveDrawingToFile(filePath);
			if (!drawingSaved)
			{
				QMessageBox::critical(this, "Error Saving File",
					"Unable to open file for saving.  File not saved: " + mFilePath);
			}
			else setWindowTitle(mFilePath);
		}
	}

	return drawingSaved;
}

bool MainWindow::closeDrawing()
{
	bool drawingClosed = true;

	if (isDrawingVisible())
	{
		QMessageBox::StandardButton button = QMessageBox::Yes;

		if (!mDrawingWidget->isClean())
		{
			QFileInfo fileInfo(mFilePath);

			button = QMessageBox::question(this, "Save Changes",
				"Save changes to " + fileInfo.fileName() + " before closing?",
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);

			if (button == QMessageBox::Yes)
			{
				if (mFilePath.startsWith("Untitled"))
				{
					if (!saveDrawingAs()) button = QMessageBox::Cancel;
				}
				else saveDrawing();
			}
		}

		drawingClosed = (button != QMessageBox::Cancel);
		if (drawingClosed) hideDrawing();
	}

	return drawingClosed;
}

//==================================================================================================

void MainWindow::exportPng()
{
	if (isDrawingVisible())
	{
		QString filePath = mFilePath;

		if (filePath.startsWith("Untitled")) filePath = mWorkingDir.path();
		else filePath = filePath.left(filePath.length() - 4) + ".png";

		filePath = QFileDialog::getSaveFileName(this, "Export PNG", filePath, "Portable Network Graphics (*.png);;All Files (*)", nullptr);
		if (!filePath.isEmpty())
		{
			if (!filePath.endsWith(".png", Qt::CaseInsensitive)) filePath += ".png";

			QRectF visibleRect = mDrawingWidget->sceneRect();
			QSize exportSize(qRound(visibleRect.width() / mExportPngSvgScale), qRound(visibleRect.height() / mExportPngSvgScale));
			QImage pngImage(exportSize, QImage::Format_ARGB32);
			QPainter painter;

			mDrawingWidget->selectNone();

			painter.begin(&pngImage);
			painter.scale(pngImage.width() / visibleRect.width(), pngImage.height() / visibleRect.height());
			painter.translate(-visibleRect.left(), -visibleRect.top());
			painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
			mDrawingWidget->renderExport(&painter);
			painter.end();

			pngImage.save(filePath, "PNG");
		}
	}
}

void MainWindow::exportSvg()
{
	if (isDrawingVisible())
	{
		QString filePath = mFilePath;

		if (filePath.startsWith("Untitled")) filePath = mWorkingDir.path();
		else filePath = filePath.left(filePath.length() - 4) + ".svg";

		filePath = QFileDialog::getSaveFileName(this, "Export SVG", filePath, "Scalable Vector Graphics (*.svg);;All Files (*)", nullptr);
		if (!filePath.isEmpty())
		{
			if (!filePath.endsWith(".svg", Qt::CaseInsensitive)) filePath += ".svg";

			QSvgGenerator svgImage;
			QPainter painter;
			QRectF visibleRect = mDrawingWidget->sceneRect();
			QSize exportSize(qRound(visibleRect.width() / mExportPngSvgScale), qRound(visibleRect.height() / mExportPngSvgScale));

			mDrawingWidget->selectNone();

			svgImage.setFileName(filePath);
			svgImage.setSize(exportSize);
			svgImage.setViewBox(QRect(QPoint(0, 0), exportSize));

			painter.begin(&svgImage);
			painter.scale(svgImage.size().width() / visibleRect.width(), svgImage.size().height() / visibleRect.height());
			painter.translate(-visibleRect.left(), -visibleRect.top());
			painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
			mDrawingWidget->renderExport(&painter);
			painter.end();
		}
	}
}

void MainWindow::exportVsdx()
{
	if (isDrawingVisible())
	{
		QString filePath = mFilePath;

		if (filePath.startsWith("Untitled")) filePath = mWorkingDir.path();
		else filePath = filePath.left(filePath.length() - 4) + ".vsdx";

		filePath = QFileDialog::getSaveFileName(this, "Export to VSDX", filePath, "Visio Drawings (*.vsdx);;All Files (*)", nullptr);
		if (!filePath.isEmpty())
		{
			if (!filePath.endsWith(".vsdx", Qt::CaseInsensitive)) filePath += ".vsdx";

			mDrawingWidget->selectNone();

			VsdxWriter writer;
			if (!writer.write(mDrawingWidget, filePath, mExportVsdxUnits, mExportVsdxScale))
				QMessageBox::critical(this, "VSDX Export Error", writer.errorMessage());
		}
	}
}

//==================================================================================================

void MainWindow::preferences()
{
	PreferencesDialog dialog(this);
	dialog.setDrawingProperties(mDrawingDefaultProperties);
	dialog.setExportProperties(mExportPngSvgScale, mExportVsdxUnits, mExportVsdxScale);

	if (dialog.exec() == QDialog::Accepted)
	{
		mDrawingDefaultProperties = dialog.drawingProperties();
		mExportPngSvgScale = dialog.exportPngSvgScale();
		mExportVsdxUnits = dialog.exportVsdxUnits();
		mExportVsdxScale = dialog.exportVsdxScale();
	}
}

void MainWindow::about()
{
	AboutDialog dialog(this);
	dialog.exec();
}

//==================================================================================================

void MainWindow::setModeFromAction(QAction* action)
{
	if (action->text() == "Scroll Mode") mDrawingWidget->setScrollMode();
	else if (action->text() == "Zoom Mode") mDrawingWidget->setZoomMode();
	else
	{
		QList<DrawingItem*> newItems;

		if (action->text() == "Place Curve") newItems.append(new DrawingCurveItem());
		else if (action->text() == "Place Ellipse") newItems.append(new DrawingEllipseItem());
		else if (action->text() == "Place Line") newItems.append(new DrawingLineItem());
		else if (action->text() == "Place Polygon") newItems.append(new DrawingPolygonItem());
		else if (action->text() == "Place Polyline") newItems.append(new DrawingPolylineItem());
		else if (action->text() == "Place Rect") newItems.append(new DrawingRectItem());
		else if (action->text() == "Place Text") newItems.append(new DrawingTextItem());
		else if (action->text() == "Place Text Rect") newItems.append(new DrawingTextRectItem());
		else if (action->text() == "Place Text Ellipse") newItems.append(new DrawingTextEllipseItem());
		else
		{
			DrawingPathItem* matchItem = nullptr;
			QString text;

			for(auto itemIter = mPathItems.begin(); matchItem == nullptr && itemIter != mPathItems.end(); itemIter++)
			{
				text = action->text();
				if (text.right(text.size() - 6) == (*itemIter)->name()) matchItem = *itemIter;
			}

			if (matchItem)
			{
				DrawingPathItem* newItem = new DrawingPathItem();
				newItem->setName(matchItem->name());
				newItem->setPath(matchItem->path(), matchItem->pathRect());
				newItem->setRect(matchItem->rect());
				newItem->addConnectionPoints(matchItem->connectionPoints());
				newItems.append(newItem);
			}
		}

		if (!newItems.isEmpty())
		{
			for(auto itemIter = newItems.begin(), itemEnd = newItems.end(); itemIter != itemEnd; itemIter++)
				(*itemIter)->setProperties(mItemDefaultProperties);

			mDrawingWidget->setPlaceMode(newItems);
		}
		else mDrawingWidget->setDefaultMode();
	}
}

void MainWindow::setModeFromDrawing(DrawingWidgetBase::Mode mode)
{
	switch (mode)
	{
	case DrawingWidget::ScrollMode:
		mModeActionGroup->actions()[ScrollModeAction]->setChecked(true);
		break;
	case DrawingWidget::ZoomMode:
		mModeActionGroup->actions()[ZoomModeAction]->setChecked(true);
		break;
	case DrawingWidget::DefaultMode:
		mModeActionGroup->actions()[DefaultModeAction]->setChecked(true);
		break;
	default:
		break;
	}
}

//==================================================================================================

void MainWindow::setZoomComboText(qreal scale)
{
	mZoomCombo->setCurrentText(QString::number(1000 * scale, 'g', 4) + "%");
}

void MainWindow::setZoomLevel(const QString& text)
{
	if (text == "Fit to Page")
	{
		mDrawingWidget->zoomFit();
	}
	else
	{
		QString numText = text;
		if (numText.endsWith("%")) numText = numText.left(numText.size() - 1);

		bool ok = false;
		qreal newScale = numText.toDouble(&ok);

		if (ok)
		{
			newScale = newScale / 1000;
			mDrawingWidget->scaleBy(newScale / mDrawingWidget->scale());
			setZoomComboText(newScale);
		}
	}
}

//==================================================================================================

void MainWindow::setItemDefaultProperties(const QHash<QString,QVariant>& properties)
{
	for(auto propIter = properties.begin(), propEnd = properties.end(); propIter != propEnd; propIter++)
		mItemDefaultProperties[propIter.key()] = propIter.value();
}

//==================================================================================================

void MainWindow::setModeText(DrawingWidgetBase::Mode mode)
{
	QString modeText = "Select Mode";

	switch (mode)
	{
	case DrawingWidgetBase::ScrollMode: modeText = "Scroll Mode"; break;
	case DrawingWidgetBase::ZoomMode: modeText = "Zoom Mode"; break;
	default:
		if (mDrawingWidget && !mDrawingWidget->placeItems().isEmpty()) modeText = "Place Mode";
		else modeText = "Select Mode";
		break;
	}

	mModeLabel->setText(modeText);
}

void MainWindow::setModifiedText(bool clean)
{
	mModifiedLabel->setText((clean) ? "" : "Modified");
}

void MainWindow::setNumberOfItemsText(int itemCount)
{
	mNumberOfItemsLabel->setText(QString::number(itemCount));
}

//==================================================================================================

void MainWindow::showEvent(QShowEvent* event)
{
	QMainWindow::showEvent(event);

	if (!event->spontaneous())
	{
		if (mDrawingWidget) mDrawingWidget->zoomFit();
	}
	else if (!mWindowState.isEmpty()) restoreState(mWindowState);
}

void MainWindow::hideEvent(QHideEvent* event)
{
	QMainWindow::hideEvent(event);

	if (event->spontaneous()) mWindowState = saveState();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (closeDrawing())
	{
		saveSettings();
		event->accept();
	}
	else event->ignore();
}

//==================================================================================================

bool MainWindow::saveDrawingToFile(const QString& filePath)
{
	DrawingWriter writer;
	writer.writeFile(mDrawingWidget, filePath);

	if (!writer.hasError())
	{
		mDrawingWidget->setClean();
		mDrawingWidget->viewport()->update();

		mFilePath = filePath;
	}

	return !writer.hasError();
}

bool MainWindow::loadDrawingFromFile(const QString& filePath)
{
	DrawingReader reader;

	mDrawingWidget->clearItems();
	mDrawingWidget->setProperties(mDrawingDefaultProperties);

	reader.readFile(filePath, mDrawingWidget);

	if (!reader.hasError())
	{
		mDrawingWidget->setClean();
		mDrawingWidget->viewport()->update();

		mFilePath = filePath;
	}

	return !reader.hasError();
}

//==================================================================================================

void MainWindow::showDrawing()
{
	mStackedWidget->setCurrentIndex(1);
	setActionsEnabled(true);
	setWindowTitle(mFilePath);

	mDrawingWidget->setDefaultMode();
	mPropertiesWidget->setDrawingProperties(mDrawingWidget->properties());

	mDrawingWidget->zoomFit();

	setModifiedText(mDrawingWidget->isClean());
	setNumberOfItemsText(mDrawingWidget->items().size());
	mMouseInfoLabel->setText("");
}

void MainWindow::hideDrawing()
{
	mStackedWidget->setCurrentIndex(0);
	setActionsEnabled(false);
	setWindowTitle(QString());

	mFilePath = "";

	mDrawingWidget->clearItems();
	mDrawingWidget->setProperties(mDrawingDefaultProperties);

	mModifiedLabel->setText("");
	mNumberOfItemsLabel->setText("");
	mMouseInfoLabel->setText("");
}

bool MainWindow::isDrawingVisible() const
{
	return (mStackedWidget->currentIndex() == 1);
}

//==================================================================================================

void MainWindow::setActionsEnabled(bool enable)
{
	QList<QAction*> actions = MainWindow::actions();
	QList<QAction*> widgetActions = mDrawingWidget->actions();
	QList<QAction*> modeActions = mModeActionGroup->actions();

	for(auto iter = widgetActions.begin(), end = widgetActions.end(); iter != end; iter++)
		(*iter)->setEnabled(enable);
	for(auto iter = modeActions.begin(), end = modeActions.end(); iter != end; iter++)
		(*iter)->setEnabled(enable);

	actions[SaveAction]->setEnabled(enable);
	actions[SaveAsAction]->setEnabled(enable);
	actions[CloseAction]->setEnabled(enable);

	actions[ExportPngAction]->setEnabled(enable);
	actions[ExportSvgAction]->setEnabled(enable);
	actions[ExportVsdxAction]->setEnabled(enable);
}

void MainWindow::setWindowTitle(const QString& filePath)
{
	QFileInfo fileInfo(filePath);
	QString fileName = fileInfo.fileName();
	QMainWindow::setWindowTitle(fileName.isEmpty() ? "Jade" : fileName + " - " + "Jade");
}

//==================================================================================================

void MainWindow::createPropertiesDock()
{
	mPropertiesWidget = new DynamicPropertiesWidget();
	mPropertiesDock = new QDockWidget("Properties");
	mPropertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	mPropertiesDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	mPropertiesDock->setWidget(mPropertiesWidget);
	mPropertiesDock->setObjectName("PropertiesDock");
	addDockWidget(Qt::RightDockWidgetArea, mPropertiesDock);

	connect(mDrawingWidget, SIGNAL(selectionChanged(const QList<DrawingItem*>&)), mPropertiesWidget, SLOT(setSelectedItems(const QList<DrawingItem*>&)));
	connect(mDrawingWidget, SIGNAL(placeItemsChanged(const QList<DrawingItem*>&)), mPropertiesWidget, SLOT(setNewItems(const QList<DrawingItem*>&)));
	connect(mDrawingWidget, SIGNAL(itemsPositionChanged(const QList<DrawingItem*>&)), mPropertiesWidget, SLOT(setItemGeometry(const QList<DrawingItem*>&)));
	connect(mDrawingWidget, SIGNAL(itemsTransformChanged(const QList<DrawingItem*>&)), mPropertiesWidget, SLOT(setItemGeometry(const QList<DrawingItem*>&)));
	connect(mDrawingWidget, SIGNAL(itemsGeometryChanged(const QList<DrawingItem*>&)), mPropertiesWidget, SLOT(setItemGeometry(const QList<DrawingItem*>&)));
	connect(mDrawingWidget, SIGNAL(itemsVisibilityChanged(const QList<DrawingItem*>&)), mPropertiesWidget, SLOT(setItemGeometry(const QList<DrawingItem*>&)));
	connect(mDrawingWidget, SIGNAL(itemsPropertiesChanged(const QList<DrawingItem*>&)), mPropertiesWidget, SLOT(setItemsProperties(const QList<DrawingItem*>&)));
	connect(mDrawingWidget, SIGNAL(drawingPropertiesChanged(const QHash<QString,QVariant>&)), mPropertiesWidget, SLOT(setDrawingProperties(const QHash<QString,QVariant>&)));
	connect(mDrawingWidget, SIGNAL(gridChanged(qreal)), mPropertiesWidget, SLOT(setDrawingGrid(qreal)));

	connect(mPropertiesWidget, SIGNAL(selectedItemMoved(const QPointF&)), mDrawingWidget, SLOT(moveSelection(const QPointF&)));
	connect(mPropertiesWidget, SIGNAL(selectedItemResized(DrawingItemPoint*, const QPointF&)), mDrawingWidget, SLOT(resizeSelection(DrawingItemPoint*, const QPointF&)));
	connect(mPropertiesWidget, SIGNAL(selectedItemsPropertiesChanged(const QHash<QString,QVariant>&)), mDrawingWidget, SLOT(setSelectionProperties(const QHash<QString,QVariant>&)));
	connect(mPropertiesWidget, SIGNAL(drawingPropertiesChanged(const QHash<QString,QVariant>&)), mDrawingWidget, SLOT(setDrawingProperties(const QHash<QString,QVariant>&)));

	connect(mDrawingWidget, SIGNAL(propertiesTriggered()), mPropertiesDock, SLOT(show()));

	connect(mPropertiesWidget, SIGNAL(defaultItemsPropertiesChanged(const QHash<QString,QVariant>&)), this, SLOT(setItemDefaultProperties(const QHash<QString,QVariant>&)));
}

void MainWindow::createStatusBar()
{
	mModeLabel = new QLabel("Select Mode");
	mModifiedLabel = new QLabel("Modified");
	mNumberOfItemsLabel = new QLabel("0");
	mMouseInfoLabel = new QLabel("");
	mModeLabel->setMinimumWidth(QFontMetrics(mModeLabel->font()).width("Select Mode") + 64);
	mModifiedLabel->setMinimumWidth(QFontMetrics(mModifiedLabel->font()).width("Modified") + 64);
	mNumberOfItemsLabel->setMinimumWidth(QFontMetrics(mNumberOfItemsLabel->font()).width("888888") + 48);
	statusBar()->addWidget(mModeLabel);
	statusBar()->addWidget(mModifiedLabel);
	statusBar()->addWidget(mNumberOfItemsLabel);
	statusBar()->addWidget(mMouseInfoLabel, 100);

	connect(mDrawingWidget, SIGNAL(modeChanged(DrawingWidgetBase::Mode)), this, SLOT(setModeText(DrawingWidgetBase::Mode)));
	connect(mDrawingWidget, SIGNAL(cleanChanged(bool)), this, SLOT(setModifiedText(bool)));
	connect(mDrawingWidget, SIGNAL(numberOfItemsChanged(int)), this, SLOT(setNumberOfItemsText(int)));
	connect(mDrawingWidget, SIGNAL(mouseInfoChanged(const QString&)), mMouseInfoLabel, SLOT(setText(const QString&)));
}

//==================================================================================================

void MainWindow::createActions()
{
	addAction("New...", this, SLOT(newDrawing()), ":/icons/oxygen/document-new.png", "Ctrl+N");
	addAction("Open...", this, SLOT(openDrawing()), ":/icons/oxygen/document-open.png", "Ctrl+O");
	addAction("Save", this, SLOT(saveDrawing()), ":/icons/oxygen/document-save.png", "Ctrl+S");
	addAction("Save As...", this, SLOT(saveDrawingAs()), ":/icons/oxygen/document-save-as.png", "Ctrl+Shift+S");
	addAction("Close", this, SLOT(closeDrawing()), ":/icons/oxygen/document-close.png", "Ctrl+W");
	addAction("Export PNG...", this, SLOT(exportPng()), ":/icons/oxygen/image-x-generic.png");
	addAction("Export SVG...", this, SLOT(exportSvg()), ":/icons/oxygen/image-svg+xml.png");
	addAction("Export VSDX...", this, SLOT(exportVsdx()), ":/icons/oxygen/application-msword.png");
	addAction("Preferences...", this, SLOT(preferences()), ":/icons/oxygen/configure.png");
	addAction("Exit", this, SLOT(close()), ":/icons/oxygen/application-exit.png");

	addAction("About...", this, SLOT(about()), ":/icons/oxygen/help-about.png");
	addAction("About Qt...", qApp, SLOT(aboutQt()));

	mModeActionGroup = new QActionGroup(this);
	connect(mModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(setModeFromAction(QAction*)));
	connect(mDrawingWidget, SIGNAL(modeChanged(DrawingWidgetBase::Mode)),
		this, SLOT(setModeFromDrawing(DrawingWidgetBase::Mode)));

	addModeAction("Select Mode", ":/icons/oxygen/edit-select.png", "Escape");
	addModeAction("Scroll Mode", ":/icons/oxygen/transform-move.png", "");
	addModeAction("Zoom Mode", ":/icons/oxygen/page-zoom.png", "");
	addModeAction("Place Curve", ":/icons/oxygen/draw-curve.png", "");
	addModeAction("Place Ellipse", ":/icons/oxygen/draw-ellipse.png", "");
	addModeAction("Place Line", ":/icons/oxygen/draw-line.png", "");
	addModeAction("Place Polygon", ":/icons/oxygen/draw-polygon.png", "");
	addModeAction("Place Polyline", ":/icons/oxygen/draw-polyline.png", "");
	addModeAction("Place Rect", ":/icons/oxygen/draw-rectangle.png", "");
	addModeAction("Place Text", ":/icons/oxygen/draw-text.png", "");
	addModeAction("Place Text Rect", ":/icons/items/textrect.png", "");
	addModeAction("Place Text Ellipse", ":/icons/items/textellipse.png", "");

	mElectricItemsAction = addPathItems("Electric Items", ElectricItems::items(), ElectricItems::icons());
	mLogicItemsAction = addPathItems("Logic Items", LogicItems::items(), LogicItems::icons());

	mModeActionGroup->actions()[DefaultModeAction]->setChecked(true);
}

void MainWindow::createMenus()
{
	QList<QAction*> actions = MainWindow::actions();
	QList<QAction*> widgetActions = mDrawingWidget->actions();
	QList<QAction*> modeActions = mModeActionGroup->actions();
	QMenu* menu;

	menu = menuBar()->addMenu("File");
	menu->addAction(actions[NewAction]);
	menu->addAction(actions[OpenAction]);
	menu->addSeparator();
	menu->addAction(actions[SaveAction]);
	menu->addAction(actions[SaveAsAction]);
	menu->addSeparator();
	menu->addAction(actions[CloseAction]);
	menu->addSeparator();
	menu->addAction(actions[ExportPngAction]);
	menu->addAction(actions[ExportSvgAction]);
	menu->addAction(actions[ExportVsdxAction]);
	menu->addSeparator();
	menu->addAction(actions[PreferencesAction]);
	menu->addSeparator();
	menu->addAction(actions[ExitAction]);

	menu = menuBar()->addMenu("Edit");
	menu->addAction(widgetActions[DrawingWidget::UndoAction]);
	menu->addAction(widgetActions[DrawingWidget::RedoAction]);
	menu->addSeparator();
	menu->addAction(widgetActions[DrawingWidget::CutAction]);
	menu->addAction(widgetActions[DrawingWidget::CopyAction]);
	menu->addAction(widgetActions[DrawingWidget::PasteAction]);
	menu->addAction(widgetActions[DrawingWidget::DeleteAction]);
	menu->addSeparator();
	menu->addAction(widgetActions[DrawingWidget::SelectAllAction]);
	menu->addAction(widgetActions[DrawingWidget::SelectNoneAction]);

	menu = menuBar()->addMenu("Place");
	menu->addAction(modeActions[DefaultModeAction]);
	menu->addAction(modeActions[ScrollModeAction]);
	menu->addAction(modeActions[ZoomModeAction]);
	menu->addSeparator();
	menu->addAction(modeActions[PlaceCurveAction]);
	menu->addAction(modeActions[PlaceEllipseAction]);
	menu->addAction(modeActions[PlaceLineAction]);
	menu->addAction(modeActions[PlacePolylineAction]);
	menu->addAction(modeActions[PlacePolygonAction]);
	menu->addAction(modeActions[PlaceRectAction]);
	menu->addAction(modeActions[PlaceTextAction]);
	menu->addAction(modeActions[PlaceTextEllipseAction]);
	menu->addAction(modeActions[PlaceTextRectAction]);
	menu->addSeparator();
	menu->addAction(mElectricItemsAction);
	menu->addAction(mLogicItemsAction);

	menu = menuBar()->addMenu("Object");
	menu->addAction(widgetActions[DrawingWidget::PropertiesAction]);
	menu->addSeparator();
	menu->addAction(widgetActions[DrawingWidget::RotateAction]);
	menu->addAction(widgetActions[DrawingWidget::RotateBackAction]);
	menu->addAction(widgetActions[DrawingWidget::FlipAction]);
	menu->addSeparator();
	menu->addAction(widgetActions[DrawingWidget::GroupAction]);
	menu->addAction(widgetActions[DrawingWidget::UngroupAction]);
	menu->addSeparator();
	menu->addAction(widgetActions[DrawingWidget::BringForwardAction]);
	menu->addAction(widgetActions[DrawingWidget::SendBackwardAction]);
	menu->addAction(widgetActions[DrawingWidget::BringToFrontAction]);
	menu->addAction(widgetActions[DrawingWidget::SendToBackAction]);

	menu = menuBar()->addMenu("View");
	menu->addAction(widgetActions[DrawingWidget::ZoomInAction]);
	menu->addAction(widgetActions[DrawingWidget::ZoomOutAction]);
	menu->addAction(widgetActions[DrawingWidget::ZoomFitAction]);

	menu = menuBar()->addMenu("About");
	menu->addAction(actions[AboutAction]);
	menu->addAction(actions[AboutQtAction]);
}

void MainWindow::createToolBars()
{
	mZoomCombo = new QComboBox();
	mZoomCombo->setMinimumWidth(QFontMetrics(mZoomCombo->font()).width("XXXXXX.XX%") + 48);
	mZoomCombo->addItems(QStringList() << "Fit to Page" << "25%" << "50%" << "100%" <<
		"150%" << "200%" << "300%" << "400%");
	mZoomCombo->setEditable(true);
	mZoomCombo->setCurrentIndex(3);

	connect(mDrawingWidget, SIGNAL(scaleChanged(qreal)), this, SLOT(setZoomComboText(qreal)));
	connect(mZoomCombo, SIGNAL(activated(const QString&)), this, SLOT(setZoomLevel(const QString&)));

	QWidget* zoomWidget = new QWidget();
	QHBoxLayout* zoomLayout = new QHBoxLayout();
	zoomLayout->addWidget(mZoomCombo);
	zoomLayout->setContentsMargins(2, 0, 2, 0);
	zoomWidget->setLayout(zoomLayout);

	QList<QAction*> actions = MainWindow::actions();
	QList<QAction*> widgetActions = mDrawingWidget->actions();
	QList<QAction*> modeActions = mModeActionGroup->actions();
	QToolBar* toolBar;
	//int size = QFontMetrics(font()).height() * 2;
	int size = mZoomCombo->sizeHint().height();

	toolBar = new QToolBar("File");
	toolBar->setObjectName("FileToolBar");
	toolBar->setIconSize(QSize(size, size));
	toolBar->addAction(actions[NewAction]);
	toolBar->addAction(actions[OpenAction]);
	toolBar->addAction(actions[SaveAction]);
	toolBar->addAction(actions[CloseAction]);
	addToolBar(toolBar);

	toolBar = new QToolBar("Edit");
	toolBar->setObjectName("EditToolBar");
	toolBar->setIconSize(QSize(size, size));
	toolBar->addAction(widgetActions[DrawingWidget::UndoAction]);
	toolBar->addAction(widgetActions[DrawingWidget::RedoAction]);
	toolBar->addSeparator();
	toolBar->addAction(widgetActions[DrawingWidget::CutAction]);
	toolBar->addAction(widgetActions[DrawingWidget::CopyAction]);
	toolBar->addAction(widgetActions[DrawingWidget::PasteAction]);
	addToolBar(toolBar);

	toolBar = new QToolBar("Object");
	toolBar->setObjectName("ObjectToolBar");
	toolBar->setIconSize(QSize(size, size));
	toolBar->addAction(widgetActions[DrawingWidget::RotateAction]);
	toolBar->addAction(widgetActions[DrawingWidget::RotateBackAction]);
	toolBar->addAction(widgetActions[DrawingWidget::FlipAction]);
	toolBar->addSeparator();
	toolBar->addAction(widgetActions[DrawingWidget::BringForwardAction]);
	toolBar->addAction(widgetActions[DrawingWidget::SendBackwardAction]);
	toolBar->addAction(widgetActions[DrawingWidget::BringToFrontAction]);
	toolBar->addAction(widgetActions[DrawingWidget::SendToBackAction]);
	addToolBar(toolBar);

	toolBar = new QToolBar("View");
	toolBar->setObjectName("ViewToolBar");
	toolBar->setIconSize(QSize(size, size));
	toolBar->addAction(widgetActions[DrawingWidget::ZoomInAction]);
	toolBar->addWidget(zoomWidget);
	toolBar->addAction(widgetActions[DrawingWidget::ZoomOutAction]);
	addToolBar(toolBar);

	toolBar = new QToolBar("Place");
	toolBar->setObjectName("PlaceToolBar");
	toolBar->setIconSize(QSize(size + 2, size + 2));
	toolBar->addAction(modeActions[DefaultModeAction]);
	toolBar->addAction(modeActions[ScrollModeAction]);
	toolBar->addAction(modeActions[ZoomModeAction]);
	toolBar->addSeparator();
	toolBar->addAction(modeActions[PlaceLineAction]);
	toolBar->addAction(modeActions[PlacePolylineAction]);
	toolBar->addAction(modeActions[PlaceCurveAction]);
	toolBar->addAction(modeActions[PlaceRectAction]);
	toolBar->addAction(modeActions[PlaceEllipseAction]);
	toolBar->addAction(modeActions[PlacePolygonAction]);
	toolBar->addAction(modeActions[PlaceTextAction]);
	toolBar->addAction(modeActions[PlaceTextRectAction]);
	toolBar->addAction(modeActions[PlaceTextEllipseAction]);
	addToolBar(Qt::LeftToolBarArea, toolBar);
}

void MainWindow::addAction(const QString& text, QObject* slotObj, const char* slotFunction,
	const QString& iconPath, const QString& shortcut)
{
	QAction* action = new QAction(text, this);
	connect(action, SIGNAL(triggered()), slotObj, slotFunction);

	if (!iconPath.isEmpty()) action->setIcon(QIcon(iconPath));
	if (!shortcut.isEmpty()) action->setShortcut(QKeySequence(shortcut));

	QMainWindow::addAction(action);
}

QAction* MainWindow::addModeAction(const QString& text, const QString& iconPath, const QString& shortcut)
{
	QAction* action = new QAction(text, this);

	if (!iconPath.isEmpty()) action->setIcon(QIcon(iconPath));
	if (!shortcut.isEmpty()) action->setShortcut(QKeySequence(shortcut));

	action->setCheckable(true);
	action->setActionGroup(mModeActionGroup);

	return action;
}

QAction* MainWindow::addPathItems(const QString& name, const QList<DrawingPathItem*>& items, const QStringList& icons)
{
	QAction* action = new QAction(QIcon(icons.first()), name, this);
	QMenu* menu = new QMenu(name);

	mPathItems.append(items);
	for(int i = 0; i < items.size() && i < icons.size(); i++)
		menu->addAction(addModeAction("Place " + items[i]->name(), icons[i], ""));

	action->setMenu(menu);
	return action;
}

