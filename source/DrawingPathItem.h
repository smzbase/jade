/* DrawingPathItem.h
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

#ifndef DRAWINGPATHITEM_H
#define DRAWINGPATHITEM_H

#include <DrawingItem.h>

class DrawingPathItem : public DrawingItem
{
private:
	enum PointIndex { TopLeft, BottomRight, TopRight, BottomLeft, TopMiddle, MiddleRight, BottomMiddle, MiddleLeft };

private:
	QRectF mRect;
	QPen mPen;

	QString mName;
	QPainterPath mPath;
	QRectF mPathRect;
	QHash<DrawingItemPoint*,QPointF> mPathConnectionPoints;

	QRectF mBoundingRect;
	QPainterPath mShape;
	QPainterPath mTransformedPath;

public:
	DrawingPathItem();
	DrawingPathItem(const DrawingPathItem& item);
	~DrawingPathItem();

	DrawingItem* copy() const;

	void setRect(const QRectF& rect);
	void setRect(qreal left, qreal top, qreal width, qreal height);
	QRectF rect() const;

	void setPen(const QPen& pen);
	QPen pen() const;

	void setProperties(const QHash<QString,QVariant>& properties);
	QHash<QString,QVariant> properties() const;

	void setName(const QString& name);
	QString name() const;

	void setPath(const QPainterPath& path, const QRectF& pathRect);
	QPainterPath path() const;
	QRectF pathRect() const;

	void addConnectionPoint(const QPointF& pathPos);
	void addConnectionPoints(const QPolygonF& pathPos);
	QPolygonF connectionPoints() const;

	QPointF mapToPath(const QPointF& itemPos) const;
	QRectF mapToPath(const QRectF& itemRect) const;
	QPointF mapFromPath(const QPointF& pathPos) const;
	QRectF mapFromPath(const QRectF& pathRect) const;

	QRectF boundingRect() const;
	QPainterPath shape() const;
	bool isValid() const;

	void render(QPainter* painter);

protected:
	void resizeEvent(DrawingItemPoint* itemPoint, const QPointF& scenePos);

	void updateGeometry();

	QPainterPath transformedPath() const;
};

#endif
