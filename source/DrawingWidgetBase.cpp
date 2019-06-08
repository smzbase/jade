/* DrawingWidgetBase.cpp
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

#include "DrawingWidgetBase.h"
#include <QtMath>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionRubberBand>

DrawingWidgetBase::DrawingWidgetBase(QWidget* parent) : QAbstractScrollArea(parent)
{
	mSceneRect = QRectF(0, 0, 10000, 7500);
	mBackgroundBrush = Qt::white;

	mGrid = 62.5;
	mGridStyle = GridGraphPaper;
	mGridBrush = QColor(0, 128, 128);
	mGridSpacingMajor = 8;
	mGridSpacingMinor = 2;

	mDynamicGridBaseValue = 0;

	mUndoStack.setUndoLimit(64);
	connect(&mUndoStack, SIGNAL(cleanChanged(bool)), this, SIGNAL(cleanChanged(bool)));
	connect(&mUndoStack, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
	connect(&mUndoStack, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));

	mMode = DefaultMode;
	mScale = 1.0;

	mMouseDragged = false;

	mScrollButtonDownHorizontalScrollValue = 0;
	mScrollButtonDownVerticalScrollValue = 0;

	mPanTimer.setInterval(16);
	connect(&mPanTimer, SIGNAL(timeout()), this, SLOT(mousePanEvent()));
}

DrawingWidgetBase::~DrawingWidgetBase() { }

//==================================================================================================

void DrawingWidgetBase::setSceneRect(const QRectF& rect)
{
	mSceneRect = rect;
}

void DrawingWidgetBase::setSceneRect(qreal left, qreal top, qreal width, qreal height)
{
	mSceneRect = QRectF(left, top, width, height);
}

QRectF DrawingWidgetBase::sceneRect() const
{
	return mSceneRect;
}

//==================================================================================================

void DrawingWidgetBase::setBackgroundBrush(const QBrush& brush)
{
	mBackgroundBrush = brush;
}

QBrush DrawingWidgetBase::backgroundBrush() const
{
	return mBackgroundBrush;
}

//==================================================================================================

void DrawingWidgetBase::setGrid(qreal grid)
{
	mGrid = grid;
}

void DrawingWidgetBase::setGridStyle(GridStyle style)
{
	mGridStyle = style;
}

void DrawingWidgetBase::setGridBrush(const QBrush& brush)
{
	mGridBrush = brush;
}

void DrawingWidgetBase::setGridSpacing(int majorSpacing, int minorSpacing)
{
	mGridSpacingMajor = majorSpacing;
	mGridSpacingMinor = minorSpacing;
}

qreal DrawingWidgetBase::grid() const
{
	return mGrid;
}

DrawingWidgetBase::GridStyle DrawingWidgetBase::gridStyle() const
{
	return mGridStyle;
}

QBrush DrawingWidgetBase::gridBrush() const
{
	return mGridBrush;
}

int DrawingWidgetBase::gridSpacingMajor() const
{
	return mGridSpacingMajor;
}

int DrawingWidgetBase::gridSpacingMinor() const
{
	return mGridSpacingMinor;
}

qreal DrawingWidgetBase::roundToGrid(qreal value) const
{
	qreal result = value;

	if (mGrid > 0)
	{
		qreal mod = fmod(value, mGrid);
		result = value - mod;
		if (mod >= mGrid/2) result += mGrid;
		else if (mod <= -mGrid/2) result -= mGrid;
	}

	return result;
}

QPointF DrawingWidgetBase::roundToGrid(const QPointF& scenePos) const
{
	return QPointF(roundToGrid(scenePos.x()), roundToGrid(scenePos.y()));
}

//==================================================================================================

void DrawingWidgetBase::setDynamicGrid(qreal baseValue)
{
	mDynamicGridBaseValue = baseValue;
	updateDynamicGrid();
}

qreal DrawingWidgetBase::dynamicGrid() const
{
	return mDynamicGridBaseValue;
}

bool DrawingWidgetBase::isDynamicGridEnabled() const
{
	return (mDynamicGridBaseValue > 0);
}

//==================================================================================================

void DrawingWidgetBase::setUndoLimit(int undoLimit)
{
	mUndoStack.setUndoLimit(undoLimit);
}

void DrawingWidgetBase::pushUndoCommand(QUndoCommand* command)
{
	mUndoStack.push(command);
}

void DrawingWidgetBase::setClean()
{
	mUndoStack.setClean();
}

int DrawingWidgetBase::undoLimit() const
{
	return mUndoStack.undoLimit();
}

bool DrawingWidgetBase::isClean() const
{
	return mUndoStack.isClean();
}

bool DrawingWidgetBase::canUndo() const
{
	return mUndoStack.canUndo();
}

bool DrawingWidgetBase::canRedo() const
{
	return mUndoStack.canRedo();
}

QString DrawingWidgetBase::undoText() const
{
	return mUndoStack.undoText();
}

QString DrawingWidgetBase::redoText() const
{
	return mUndoStack.redoText();
}

//==================================================================================================

DrawingWidgetBase::Mode DrawingWidgetBase::mode() const
{
	return mMode;
}

qreal DrawingWidgetBase::scale() const
{
	return mScale;
}

//==================================================================================================

void DrawingWidgetBase::centerOn(const QPointF& scenePos)
{
	QPointF oldScenePos = mapToScene(viewport()->rect().center());

	int horizontalDelta = qRound((scenePos.x() - oldScenePos.x()) * mScale);
	int verticalDelta = qRound((scenePos.y() - oldScenePos.y()) * mScale);

	horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalDelta);
	verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalDelta);
}

void DrawingWidgetBase::centerOnCursor(const QPointF& scenePos)
{
	QPointF oldScenePos = mapToScene(mapFromGlobal(QCursor::pos()));

	int horizontalDelta = qRound((scenePos.x() - oldScenePos.x()) * mScale);
	int verticalDelta = qRound((scenePos.y() - oldScenePos.y()) * mScale);

	horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalDelta);
	verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalDelta);
}

void DrawingWidgetBase::fitToView(const QRectF& sceneRect)
{
	qreal scaleX = (maximumViewportSize().width() - 5) / sceneRect.width();
	qreal scaleY = (maximumViewportSize().height() - 5) / sceneRect.height();

	mScale = qMin(scaleX, scaleY);

	recalculateContentSize(sceneRect);

	centerOn(sceneRect.center());
}

void DrawingWidgetBase::scaleBy(qreal scale)
{
	if (scale > 0)
	{
		QPointF mousePos = mapToScene(mapFromGlobal(QCursor::pos()));
		QRectF scrollBarRect = scrollBarDefinedRect();

		mScale *= scale;

		recalculateContentSize(scrollBarRect);

		if (viewport()->rect().contains(mapFromGlobal(QCursor::pos()))) centerOnCursor(mousePos);
		else centerOn(QPointF());
	}
}

//==================================================================================================

QPointF DrawingWidgetBase::mapToScene(const QPoint& screenPos) const
{
	QPointF p = screenPos;
	p.setX(p.x() + horizontalScrollBar()->value());
	p.setY(p.y() + verticalScrollBar()->value());
	return mSceneTransform.map(p);
}

QRectF DrawingWidgetBase::mapToScene(const QRect& screenRect) const
{
	return QRectF(mapToScene(screenRect.topLeft()), mapToScene(screenRect.bottomRight()));
}

QPoint DrawingWidgetBase::mapFromScene(const QPointF& scenePos) const
{
	QPointF p = mViewportTransform.map(scenePos);
	p.setX(p.x() - horizontalScrollBar()->value());
	p.setY(p.y() - verticalScrollBar()->value());
	return p.toPoint();
}

QRect DrawingWidgetBase::mapFromScene(const QRectF& sceneRect) const
{
	return QRect(mapFromScene(sceneRect.topLeft()), mapFromScene(sceneRect.bottomRight()));
}

QRectF DrawingWidgetBase::visibleRect() const
{
	return QRectF(mapToScene(QPoint(0, 0)), mapToScene(QPoint(viewport()->width(), viewport()->height())));
}

QRectF DrawingWidgetBase::scrollBarDefinedRect() const
{
	QRectF scrollBarRect = mSceneRect;

	if (horizontalScrollBar()->minimum() < horizontalScrollBar()->maximum())
	{
		scrollBarRect.setLeft(horizontalScrollBar()->minimum() / mScale + scrollBarRect.left());
		scrollBarRect.setWidth((horizontalScrollBar()->maximum() - horizontalScrollBar()->minimum() +
			maximumViewportSize().width()) / mScale);
	}

	if (verticalScrollBar()->minimum() < verticalScrollBar()->maximum())
	{
		scrollBarRect.setTop(verticalScrollBar()->minimum() / mScale + scrollBarRect.top());
		scrollBarRect.setHeight((verticalScrollBar()->maximum() - verticalScrollBar()->minimum() +
			maximumViewportSize().height()) / mScale);
	}

	return scrollBarRect;
}

//==================================================================================================

void DrawingWidgetBase::setDefaultMode()
{
	mMode = DefaultMode;
	setCursor(Qt::ArrowCursor);
	emit modeChanged(mMode);
	viewport()->update();
}

void DrawingWidgetBase::setScrollMode()
{
	mMode = ScrollMode;
	setCursor(Qt::OpenHandCursor);
	emit modeChanged(mMode);
	viewport()->update();
}

void DrawingWidgetBase::setZoomMode()
{
	mMode = ZoomMode;
	setCursor(Qt::CrossCursor);
	emit modeChanged(mMode);
	viewport()->update();
}

//==================================================================================================

void DrawingWidgetBase::zoomIn()
{
	scaleBy(qSqrt(2));
	emit scaleChanged(mScale);
	viewport()->update();
}

void DrawingWidgetBase::zoomOut()
{
	scaleBy(qSqrt(2) / 2);
	emit scaleChanged(mScale);
	viewport()->update();
}

void DrawingWidgetBase::zoomFit()
{
	fitToView(mSceneRect);
	emit scaleChanged(mScale);
	viewport()->update();
}

//==================================================================================================

void DrawingWidgetBase::undo()
{
	if (mode() == DefaultMode && mUndoStack.canUndo())
	{
		mUndoStack.undo();
		viewport()->update();
	}
}

void DrawingWidgetBase::redo()
{
	if (mode() == DefaultMode && mUndoStack.canRedo())
	{
		mUndoStack.redo();
		viewport()->update();
	}
}

//==================================================================================================

void DrawingWidgetBase::paintEvent(QPaintEvent* event)
{
	QImage image(viewport()->width(), viewport()->height(), QImage::Format_RGB32);
	image.fill(palette().brush(QPalette::Window).color());

	// Render scene
	QPainter painter(&image);

	painter.translate(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
	painter.setTransform(mViewportTransform, true);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	drawBackground(&painter);
	drawContents(&painter);
	drawRubberBand(&painter, mZoomRubberBandRect);
	drawForeground(&painter);

	painter.end();

	// Render scene image on to widget
	QPainter widgetPainter(viewport());
	widgetPainter.drawImage(0, 0, image);

	Q_UNUSED(event);
}

void DrawingWidgetBase::resizeEvent(QResizeEvent* event)
{
	QAbstractScrollArea::resizeEvent(event);
	recalculateContentSize();
}

//==================================================================================================

void DrawingWidgetBase::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mMouseButtonDownPos = event->pos();
		mMouseDragged = false;

		if (mMode == ScrollMode)
		{
			setCursor(Qt::ClosedHandCursor);
			mScrollButtonDownHorizontalScrollValue = horizontalScrollBar()->value();
			mScrollButtonDownVerticalScrollValue = verticalScrollBar()->value();
		}
	}
	else if (event->button() == Qt::MiddleButton)
	{
		setCursor(Qt::SizeAllCursor);

		mPanStartPos = event->pos();
		mPanCurrentPos = event->pos();
		mPanTimer.start();
	}

	viewport()->update();
}

void DrawingWidgetBase::mouseMoveEvent(QMouseEvent* event)
{
	mMouseDragged = (mMouseDragged |
		((mMouseButtonDownPos - event->pos()).manhattanLength() >= QApplication::startDragDistance()));

	if (mMode == ScrollMode)
	{
		if ((event->buttons() & Qt::LeftButton) && mMouseDragged)
		{
			horizontalScrollBar()->setValue(
				mScrollButtonDownHorizontalScrollValue - (event->pos().x() - mMouseButtonDownPos.x()));
			verticalScrollBar()->setValue(
				mScrollButtonDownVerticalScrollValue - (event->pos().y() - mMouseButtonDownPos.y()));
		}
	}
	else if (mMode == ZoomMode)
	{
		if ((event->buttons() & Qt::LeftButton) && mMouseDragged)
		{
			QPoint p1 = event->pos();
			QPoint p2 = mMouseButtonDownPos;
			mZoomRubberBandRect = QRect(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()), qAbs(p2.x() - p1.x()), qAbs(p2.y() - p1.y()));
		}
	}

	if (mPanTimer.isActive()) mPanCurrentPos = event->pos();

	if (event->buttons() != Qt::NoButton) viewport()->update();
}

void DrawingWidgetBase::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (mMode == ScrollMode)
		{
			setCursor(Qt::OpenHandCursor);
		}
		else if (mMode == ZoomMode)
		{
			if (mZoomRubberBandRect.isValid())
			{
				fitToView(mapToScene(mZoomRubberBandRect));
				emit scaleChanged(mScale);

				mZoomRubberBandRect = QRect();
				setDefaultMode();
			}
		}
	}
	else if (event->button() == Qt::RightButton && mMode != DefaultMode) setDefaultMode();

	mMouseButtonDownPos = QPoint();
	mMouseDragged = false;

	if (mPanTimer.isActive())
	{
		setCursor(Qt::ArrowCursor);
		mPanTimer.stop();
	}

	viewport()->update();
}

void DrawingWidgetBase::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && mMode != DefaultMode) setDefaultMode();
}

void DrawingWidgetBase::wheelEvent(QWheelEvent* event)
{
	if (event->modifiers() && Qt::ControlModifier)
	{
		if (event->delta() > 0) zoomIn();
		else if (event->delta() < 0) zoomOut();
	}
	else QAbstractScrollArea::wheelEvent(event);
}



//==================================================================================================

void DrawingWidgetBase::drawBackground(QPainter* painter)
{
	QRectF visibleRect = DrawingWidgetBase::visibleRect();

	QPainter::RenderHints renderHints = painter->renderHints();
	painter->setRenderHints(renderHints, false);

	// Draw background
	painter->setBrush(mBackgroundBrush);
	painter->setPen(Qt::NoPen);
	painter->drawRect(visibleRect);

	// Draw grid
	QPen gridPen(mGridBrush, devicePixelRatio());
	gridPen.setCosmetic(true);

	if (mGridStyle != GridNone && mGrid >= 0)
	{
		painter->setPen(gridPen);

		if (mGridStyle == GridDots && mGridSpacingMajor > 0)
		{
			qreal spacing = mGrid * mGridSpacingMajor;
			for(qreal y = qCeil(visibleRect.top() / spacing) * spacing; y < visibleRect.bottom(); y += spacing)
			{
				for(qreal x = qCeil(visibleRect.left() / spacing) * spacing; x < visibleRect.right(); x += spacing)
					painter->drawPoint(QPointF(x, y));
			}
		}

		if (mGridStyle == GridGraphPaper && mGridSpacingMinor > 0)
		{
			gridPen.setStyle(Qt::DotLine);
			painter->setPen(gridPen);

			qreal spacing = mGrid * mGridSpacingMinor;
			for(qreal y = qCeil(visibleRect.top() / spacing) * spacing; y < visibleRect.bottom(); y += spacing)
				painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
			for(qreal x = qCeil(visibleRect.left() / spacing) * spacing; x < visibleRect.right(); x += spacing)
				painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
		}

		if ((mGridStyle == GridLines || mGridStyle == GridGraphPaper) && mGridSpacingMajor > 0)
		{
			gridPen.setStyle(Qt::SolidLine);
			painter->setPen(gridPen);

			qreal spacing = mGrid * mGridSpacingMajor;
			for(qreal y = qCeil(visibleRect.top() / spacing) * spacing; y < visibleRect.bottom(); y += spacing)
				painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
			for(qreal x = qCeil(visibleRect.left() / spacing) * spacing; x < visibleRect.right(); x += spacing)
				painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
		}
	}

	// Draw origin
	painter->save();
	painter->setBrush(Qt::transparent);
	painter->setPen(gridPen);
	painter->resetTransform();
	painter->drawEllipse(mapFromScene(QPointF(0, 0)), 4, 4);
	painter->restore();

	// Draw border
	QPen borderPen((mBackgroundBrush == Qt::black) ? Qt::white : Qt::black, devicePixelRatio() * 2);
	borderPen.setCosmetic(true);

	painter->setBrush(Qt::transparent);
	painter->setPen(borderPen);
	painter->drawRect(mSceneRect);

	painter->setRenderHints(renderHints);
	painter->setBrush(mBackgroundBrush);
}

void DrawingWidgetBase::drawContents(QPainter* painter)
{
	Q_UNUSED(painter);
}

void DrawingWidgetBase::drawRubberBand(QPainter* painter, const QRect& rect)
{
	if (rect.isValid())
	{
		QStyleOptionRubberBand option;
		option.initFrom(viewport());
		option.rect = rect;
		option.shape = QRubberBand::Rectangle;

		painter->save();
		painter->resetTransform();

		QStyleHintReturnMask mask;
		if (viewport()->style()->styleHint(QStyle::SH_RubberBand_Mask, &option, viewport(), &mask))
			painter->setClipRegion(mask.region, Qt::IntersectClip);

		viewport()->style()->drawControl(QStyle::CE_RubberBand, &option, painter, viewport());

		painter->restore();
	}
}

void DrawingWidgetBase::drawForeground(QPainter* painter)
{
	Q_UNUSED(painter);
}

//==================================================================================================

void DrawingWidgetBase::mousePanEvent()
{
	QRectF visibleRect = DrawingWidgetBase::visibleRect();

	if (horizontalScrollBar()->maximum() - horizontalScrollBar()->minimum() > 0)
	{
		if (mPanCurrentPos.x() - mPanStartPos.x() < 0)
		{
			int delta = (mPanCurrentPos.x() - mPanStartPos.x()) / 16;

			if (horizontalScrollBar()->value() + delta < horizontalScrollBar()->minimum())
			{
				if (horizontalScrollBar()->minimum() >= horizontalScrollBar()->maximum())
					horizontalScrollBar()->setMinimum(qFloor((visibleRect.left() - mSceneRect.left()) * mScale) + delta);
				else
					horizontalScrollBar()->setMinimum(horizontalScrollBar()->value() + delta);

				horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());
			}
			else horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta);
		}
		else if (mPanCurrentPos.x() - mPanStartPos.x() > 0)
		{
			int delta = (mPanCurrentPos.x() - mPanStartPos.x()) / 16;

			if (horizontalScrollBar()->value() + delta > horizontalScrollBar()->maximum())
			{
				if (horizontalScrollBar()->minimum() > horizontalScrollBar()->maximum())
					horizontalScrollBar()->setMaximum(qFloor((mSceneRect.right() - visibleRect.right()) * mScale) + delta);
				else
					horizontalScrollBar()->setMaximum(horizontalScrollBar()->value() + delta);

				horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
			}
			else horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta);
		}
	}

	if (verticalScrollBar()->maximum() - verticalScrollBar()->minimum() > 0)
	{
		if (mPanCurrentPos.y() - mPanStartPos.y() < 0)
		{
			int delta = (mPanCurrentPos.y() - mPanStartPos.y()) / 16;

			if (verticalScrollBar()->value() + delta < verticalScrollBar()->minimum())
			{
				if (verticalScrollBar()->minimum() >= verticalScrollBar()->maximum())
					verticalScrollBar()->setMinimum(qFloor((visibleRect.top() - mSceneRect.top()) * mScale) + delta);
				else
					verticalScrollBar()->setMinimum(verticalScrollBar()->value() + delta);

				verticalScrollBar()->setValue(verticalScrollBar()->minimum());
			}
			else verticalScrollBar()->setValue(verticalScrollBar()->value() + delta);
		}
		else if (mPanCurrentPos.y() - mPanStartPos.y() > 0)
		{
			int delta = (mPanCurrentPos.y() - mPanStartPos.y()) / 16;

			if (verticalScrollBar()->value() + delta > verticalScrollBar()->maximum())
			{
				if (verticalScrollBar()->minimum() >= verticalScrollBar()->maximum())
					verticalScrollBar()->setMaximum(qFloor((mSceneRect.bottom() - visibleRect.bottom()) * mScale) + delta);
				else
					verticalScrollBar()->setMaximum(verticalScrollBar()->value() + delta);

				verticalScrollBar()->setValue(verticalScrollBar()->maximum());
			}
			else verticalScrollBar()->setValue(verticalScrollBar()->value() + delta);
		}
	}
}

//==================================================================================================

void DrawingWidgetBase::recalculateContentSize(const QRectF& sceneRect)
{
	QRectF targetRect = mSceneRect;
	qreal dx = 0, dy = 0;

	if (sceneRect.isValid())
	{
		targetRect.setLeft(qMin(sceneRect.left(), mSceneRect.left()));
		targetRect.setTop(qMin(sceneRect.top(), mSceneRect.top()));
		targetRect.setRight(qMax(sceneRect.right(), mSceneRect.right()));
		targetRect.setBottom(qMax(sceneRect.bottom(), mSceneRect.bottom()));
	}

	int contentWidth = qRound(targetRect.width() * mScale);
	int contentHeight = qRound(targetRect.height() * mScale);
	int viewportWidth = maximumViewportSize().width();
	int viewportHeight = maximumViewportSize().height();
	int scrollBarExtent = style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, this);

	// Set scroll bar range
	if (contentWidth > viewportWidth)
	{
		int contentLeft = qFloor((targetRect.left() - mSceneRect.left()) * mScale);

		if (verticalScrollBarPolicy() == Qt::ScrollBarAsNeeded) viewportWidth -= scrollBarExtent;

		horizontalScrollBar()->setRange(contentLeft - 1, contentLeft + contentWidth - viewportWidth + 1);
		horizontalScrollBar()->setSingleStep(viewportWidth / 80);
		horizontalScrollBar()->setPageStep(viewportWidth);
	}
	else horizontalScrollBar()->setRange(0, 0);

	if (contentHeight > viewportHeight)
	{
		int contentTop = qFloor((targetRect.top() - mSceneRect.top()) * mScale);

		if (horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded) viewportHeight -= scrollBarExtent;

		verticalScrollBar()->setRange(contentTop - 1, contentTop + contentHeight - viewportHeight + 1);
		verticalScrollBar()->setSingleStep(viewportHeight / 80);
		verticalScrollBar()->setPageStep(viewportHeight);
	}
	else verticalScrollBar()->setRange(0, 0);

	// Recalculate transforms
	dx = -targetRect.left() * mScale;
	dy = -targetRect.top() * mScale;

	if (horizontalScrollBar()->maximum() <= horizontalScrollBar()->minimum())
		dx += -(targetRect.width() * mScale - viewportWidth) / 2;
	if (verticalScrollBar()->maximum() <= verticalScrollBar()->minimum())
		dy += -(targetRect.height() * mScale - viewportHeight) / 2;

	mViewportTransform = QTransform();
	mViewportTransform.translate(qRound(dx), qRound(dy));
	mViewportTransform.scale(mScale, mScale);

	mSceneTransform = mViewportTransform.inverted();

	// Update dynamic grid, if enabled
	updateDynamicGrid();
}

void DrawingWidgetBase::updateDynamicGrid()
{
	if (mDynamicGridBaseValue > 0)
	{
		const int minimumScreenGridValue = devicePixelRatio() * 4;		// pixels

		QPointF gridPoint = mapToScene(QPoint(minimumScreenGridValue, minimumScreenGridValue)) - mapToScene(QPoint(0, 0));
		qreal gridValue = qMax(gridPoint.x(), gridPoint.y());

		qreal baseOffset = qLn(mDynamicGridBaseValue) / qLn(2);
		baseOffset = baseOffset - qFloor(baseOffset);

		mGrid = qPow(2, qCeil(qLn(gridValue) / qLn(2) - baseOffset) + baseOffset);
		emit gridChanged(mGrid);
	}
}
