/* DrawingPolygonItem.h
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

#ifndef DRAWINGPOLYGONITEM_H
#define DRAWINGPOLYGONITEM_H

#include <DrawingItem.h>

class DrawingPolygonItem : public DrawingItem
{
private:
	QPolygonF mPolygon;
	QPen mPen;
	QBrush mBrush;

	QRectF mBoundingRect;
	QPainterPath mShape;

public:
	DrawingPolygonItem();
	DrawingPolygonItem(const DrawingPolygonItem& item);
	~DrawingPolygonItem();

	DrawingItem* copy() const;

	void setPolygon(const QPolygonF& polygon);
	QPolygonF polygon() const;

	void setPen(const QPen& pen);
	QPen pen() const;

	void setBrush(const QBrush& brush);
	QBrush brush() const;

	void setProperties(const QHash<QString,QVariant>& properties);
	QHash<QString,QVariant> properties() const;

	QRectF boundingRect() const;
	QPainterPath shape() const;
	bool isValid() const;

	void render(QPainter* painter);

protected:
	void resizeEvent(DrawingItemPoint* itemPoint, const QPointF& scenePos);

	DrawingItemPoint* itemPointToInsert(const QPointF& itemPos, int& index);
	DrawingItemPoint* itemPointToRemove(const QPointF& itemPos);

	void updateGeometry();

	qreal distanceFromPointToLineSegment(const QPointF& point, const QLineF& line) const;
};

#endif
