/* DrawingItem.h
 *
 * Copyright 2019 Jason Allen
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
 * along with jade.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DRAWINGITEM_H
#define DRAWINGITEM_H

#include <DrawingItemContainer.h>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QTransform>

class DrawingItemPoint;

class DrawingItem : public DrawingItemContainer
{
	friend class DrawingItemContainer;

public:
	enum Flag
	{
		CanMove = 0x01,
		CanResize = 0x02,
		CanRotate = 0x04,
		CanFlip = 0x08,
		CanSelect = 0x10,
		CanInsertPoints = 0x20,
		CanRemovePoints = 0x40,
		PlaceByMousePressAndRelease = 0x1000,
		AdjustPositionOnResize = 0x2000
	};
	Q_DECLARE_FLAGS(Flags, Flag)

private:
	DrawingItemContainer* mParent;

	QPointF mPosition;

	QTransform mTransform;
	QTransform mTransformInverse;

	Flags mFlags;

	QList<DrawingItemPoint*> mPoints;

	bool mSelected;
	bool mVisible;

public:
	DrawingItem();
	DrawingItem(const DrawingItem& item);
	virtual ~DrawingItem();

	virtual DrawingItem* copy() const = 0;

	DrawingItemContainer* parent() const;

	void setPosition(const QPointF& position);
	void setPosition(qreal x, qreal y);
	void setX(qreal x);
	void setY(qreal y);
	QPointF position() const;
	qreal x() const;
	qreal y() const;

	void setTransform(const QTransform& transform, bool combine = false);
	QTransform transform() const;
	QTransform transformInverted() const;

	void setFlags(Flags flags);
	Flags flags() const;

	void addPoint(DrawingItemPoint* point);
	void insertPoint(int index, DrawingItemPoint* point);
	void removePoint(DrawingItemPoint* point);
	void clearPoints();
	QList<DrawingItemPoint*> points() const;

	DrawingItemPoint* pointAt(const QPointF& position) const;
	DrawingItemPoint* pointNearest(const QPointF& position) const;

	void setSelected(bool select);
	void setVisible(bool visible);
	bool isSelected() const;
	bool isVisible() const;

	QPointF mapFromParent(const QPointF& point) const;
	QPolygonF mapFromParent(const QRectF& rect) const;
	QPolygonF mapFromParent(const QPolygonF& polygon) const;
	QPainterPath mapFromParent(const QPainterPath& path) const;
	QPointF mapToParent(const QPointF& point) const;
	QPolygonF mapToParent(const QRectF& rect) const;
	QPolygonF mapToParent(const QPolygonF& polygon) const;
	QPainterPath mapToParent(const QPainterPath& path) const;

	virtual QRectF boundingRect() const = 0;
	virtual QPainterPath shape() const;
	virtual QPointF center() const;
	virtual bool isValid() const;

	virtual void render(QPainter* painter) = 0;

protected:


	virtual QPainterPath strokePath(const QPainterPath& path, const QPen& pen) const;

public:
	static QList<DrawingItem*> copyItems(const QList<DrawingItem*>& items);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DrawingItem::Flags)

#endif
