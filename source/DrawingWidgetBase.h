/* DrawingWidgetBase.h
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

#ifndef DRAWINGWIDGETBASE_H
#define DRAWINGWIDGETBASE_H

#include <QAbstractScrollArea>
#include <QTimer>
#include <QUndoStack>

class DrawingWidgetBase : public QAbstractScrollArea
{
	Q_OBJECT

public:
	enum GridStyle { GridNone, GridDots, GridLines, GridGraphPaper };
	enum Mode { DefaultMode, ScrollMode, ZoomMode, UserMode = 0x1000 };

private:
	QRectF mSceneRect;
	QBrush mBackgroundBrush;

	qreal mGrid;
	GridStyle mGridStyle;
	QBrush mGridBrush;
	int mGridSpacingMajor, mGridSpacingMinor;

	qreal mDynamicGridBaseValue;

	QUndoStack mUndoStack;

	Mode mMode;
	qreal mScale;

	QTransform mViewportTransform;
	QTransform mSceneTransform;

	QPoint mMouseButtonDownPos;
	bool mMouseDragged;

	int mScrollButtonDownHorizontalScrollValue;
	int mScrollButtonDownVerticalScrollValue;

	QRect mZoomRubberBandRect;

	QPoint mPanStartPos;
	QPoint mPanCurrentPos;
	QTimer mPanTimer;

public:
	DrawingWidgetBase(QWidget* parent = nullptr);
	virtual ~DrawingWidgetBase();

	void setSceneRect(const QRectF& rect);
	void setSceneRect(qreal left, qreal top, qreal width, qreal height);
	QRectF sceneRect() const;

	void setBackgroundBrush(const QBrush& brush);
	QBrush backgroundBrush() const;

	void setGrid(qreal grid);
	void setGridStyle(GridStyle style);
	void setGridBrush(const QBrush& brush);
	void setGridSpacing(int majorSpacing, int minorSpacing = 1);
	qreal grid() const;
	GridStyle gridStyle() const;
	QBrush gridBrush() const;
	int gridSpacingMajor() const;
	int gridSpacingMinor() const;
	qreal roundToGrid(qreal value) const;
	QPointF roundToGrid(const QPointF& scenePos) const;

	void setDynamicGrid(qreal baseValue);
	qreal dynamicGrid() const;
	bool isDynamicGridEnabled() const;

	void setUndoLimit(int undoLimit);
	void pushUndoCommand(QUndoCommand* command);
	void setClean();
	int undoLimit() const;
	bool isClean() const;
	bool canUndo() const;
	bool canRedo() const;
	QString undoText() const;
	QString redoText() const;

	Mode mode() const;
	qreal scale() const;

	void centerOn(const QPointF& scenePos);
	void centerOnCursor(const QPointF& scenePos);
	void fitToView(const QRectF& sceneRect);
	void scaleBy(qreal scale);

	QPointF mapToScene(const QPoint& screenPos) const;
	QRectF mapToScene(const QRect& screenRect) const;
	QPoint mapFromScene(const QPointF& scenePos) const;
	QRect mapFromScene(const QRectF& sceneRect) const;
	QRectF visibleRect() const;
	QRectF scrollBarDefinedRect() const;

public slots:
	void setDefaultMode();
	void setScrollMode();
	void setZoomMode();

	void zoomIn();
	void zoomOut();
	void zoomFit();

	void undo();
	void redo();

signals:
	void modeChanged(DrawingWidgetBase::Mode mode);
	void scaleChanged(qreal scale);
	void cleanChanged(bool clean);
	void canUndoChanged(bool canUndo);
	void canRedoChanged(bool canRedo);
	void gridChanged(qreal grid);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void resizeEvent(QResizeEvent* event);

	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);

	virtual void drawBackground(QPainter* painter);
	virtual void drawContents(QPainter* painter);
	virtual void drawRubberBand(QPainter* painter, const QRect& rect);
	virtual void drawForeground(QPainter* painter);

private slots:
	void mousePanEvent();

private:
	void recalculateContentSize(const QRectF& sceneRect = QRectF());
	void updateDynamicGrid();
};

#endif
