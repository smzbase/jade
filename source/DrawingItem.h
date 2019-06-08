/* DrawingItem.h
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

#ifndef DRAWINGITEM_H
#define DRAWINGITEM_H

#include <QList>
#include <QPen>
#include <QPointF>
#include <QTransform>

class QKeyEvent;
class DrawingItemPoint;

class DrawingItem
{
	friend class DrawingWidget;

public:
	//! \brief Enum used to affect the behavior of the DrawingItem within the scene.
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
	DrawingWidget* mWidget;

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

	DrawingWidget* widget() const;

	void setPosition(const QPointF& pos);
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

	void addPoint(DrawingItemPoint* itemPoint);
	void insertPoint(int index, DrawingItemPoint* itemPoint);
	void removePoint(DrawingItemPoint* itemPoint);
	void clearPoints();
	QList<DrawingItemPoint*> points() const;

	DrawingItemPoint* pointAt(const QPointF& itemPos) const;
	DrawingItemPoint* pointNearest(const QPointF& itemPos) const;

	void setSelected(bool select);
	void setVisible(bool visible);
	bool isSelected() const;
	bool isVisible() const;

	QPointF mapFromScene(const QPointF& point) const;
	QPolygonF mapFromScene(const QRectF& rect) const;
	QPolygonF mapFromScene(const QPolygonF& polygon) const;
	QPainterPath mapFromScene(const QPainterPath& path) const;
	QPointF mapToScene(const QPointF& point) const;
	QPolygonF mapToScene(const QRectF& rect) const;
	QPolygonF mapToScene(const QPolygonF& polygon) const;
	QPainterPath mapToScene(const QPainterPath& path) const;

	virtual void setProperties(const QHash<QString,QVariant>& properties);
	virtual QHash<QString,QVariant> properties() const;

	virtual QRectF boundingRect() const = 0;
	virtual QPainterPath shape() const;
	virtual QPointF centerPos() const;
	virtual bool isValid() const;

	virtual void render(QPainter* painter) = 0;

protected:
	virtual void moveEvent(const QPointF& scenePos);
	virtual void resizeEvent(DrawingItemPoint* itemPoint, const QPointF& scenePos);
	virtual void rotateEvent(const QPointF& scenePos);
	virtual void rotateBackEvent(const QPointF& scenePos);
	virtual void flipHorizontalEvent(const QPointF& scenePos);
	virtual void flipVerticalEvent(const QPointF& scenePos);

	virtual DrawingItemPoint* itemPointToInsert(const QPointF& itemPos, int& index);
	virtual DrawingItemPoint* itemPointToRemove(const QPointF& itemPos);

	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);

	virtual QPainterPath strokePath(const QPainterPath& path, const QPen& pen) const;

public:
	static QList<DrawingItem*> copyItems(const QList<DrawingItem*>& items);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DrawingItem::Flags)

#endif
