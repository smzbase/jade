/* DrawingLineItem.h
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

#ifndef DRAWINGLINEITEM_H
#define DRAWINGLINEITEM_H

#include <DrawingItem.h>
#include <DrawingArrow.h>

class DrawingLineItem : public DrawingItem
{
private:
	QLineF mLine;
	QPen mPen;
	DrawingArrow mStartArrow;
	DrawingArrow mEndArrow;

	QRectF mBoundingRect;
	QPainterPath mShape;

public:
	DrawingLineItem();
	DrawingLineItem(const DrawingLineItem& item);
	~DrawingLineItem();

	DrawingItem* copy() const;

	void setLine(const QLineF& line);
	void setLine(qreal x1, qreal y1, qreal x2, qreal y2);
	QLineF line() const;

	void setPen(const QPen& pen);
	QPen pen() const;

	void setStartArrow(const DrawingArrow& arrow);
	void setEndArrow(const DrawingArrow& arrow);
	DrawingArrow startArrow() const;
	DrawingArrow endArrow() const;

	void setProperties(const QHash<QString,QVariant>& properties);
	QHash<QString,QVariant> properties() const;

	QRectF boundingRect() const;
	QPainterPath shape() const;
	bool isValid() const;

	void render(QPainter* painter);

protected:
	void resizeEvent(DrawingItemPoint* itemPoint, const QPointF& scenePos);

	void updateGeometry();
};

#endif
