/* DrawingItem.cpp
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

#include "DrawingItem.h"
#include "DrawingItemPoint.h"
#include <QKeyEvent>

DrawingItem::DrawingItem()
{
	mWidget = nullptr;

	mFlags = (CanMove | CanResize | CanRotate | CanFlip | CanSelect);

	mSelected = false;
	mVisible = true;
}

DrawingItem::DrawingItem(const DrawingItem& item)
{
	mWidget = nullptr;

	mPosition = item.mPosition;
	mTransform = item.mTransform;
	mTransformInverse = item.mTransformInverse;

	mFlags = item.mFlags;

	for(auto pointIter = item.mPoints.begin(), pointEnd = item.mPoints.end();
		pointIter != pointEnd; pointIter++)
	{
		addPoint(new DrawingItemPoint(**pointIter));
	}

	mSelected = false;
	mVisible = true;
}

DrawingItem::~DrawingItem()
{
	clearPoints();
	mWidget = nullptr;
}

//==================================================================================================

DrawingWidget* DrawingItem::widget() const
{
	return mWidget;
}

//==================================================================================================

void DrawingItem::setPosition(const QPointF& pos)
{
	mPosition = pos;
}

void DrawingItem::setPosition(qreal x, qreal y)
{
	mPosition.setX(x);
	mPosition.setY(y);
}

void DrawingItem::setX(qreal x)
{
	mPosition.setX(x);
}

void DrawingItem::setY(qreal y)
{
	mPosition.setY(y);
}

QPointF DrawingItem::position() const
{
	return mPosition;
}

qreal DrawingItem::x() const
{
	return mPosition.x();
}

qreal DrawingItem::y() const
{
	return mPosition.y();
}

//==================================================================================================

void DrawingItem::setTransform(const QTransform& transform, bool combine)
{
	if (combine) mTransform = mTransform * transform;
	else mTransform = transform;

	mTransformInverse = mTransform.inverted();
}

QTransform DrawingItem::transform() const
{
	return mTransform;
}

QTransform DrawingItem::transformInverted() const
{
	return mTransformInverse;
}

//==================================================================================================

void DrawingItem::setFlags(Flags flags)
{
	mFlags = flags;
}

DrawingItem::Flags DrawingItem::flags() const
{
	return mFlags;
}

//==================================================================================================

void DrawingItem::addPoint(DrawingItemPoint* itemPoint)
{
	if (itemPoint && itemPoint->mItem == nullptr)
	{
		mPoints.append(itemPoint);
		itemPoint->mItem = this;
	}
}

void DrawingItem::insertPoint(int index, DrawingItemPoint* itemPoint)
{
	if (itemPoint && itemPoint->mItem == nullptr)
	{
		mPoints.insert(index, itemPoint);
		itemPoint->mItem = this;
	}
}

void DrawingItem::removePoint(DrawingItemPoint* itemPoint)
{
	if (itemPoint && itemPoint->mItem == this)
	{
		mPoints.removeAll(itemPoint);
		itemPoint->mItem = nullptr;
	}
}

void DrawingItem::clearPoints()
{
	DrawingItemPoint* itemPoint = nullptr;

	while (!mPoints.empty())
	{
		itemPoint = mPoints.first();
		removePoint(itemPoint);
		delete itemPoint;
		itemPoint = nullptr;
	}
}

QList<DrawingItemPoint*> DrawingItem::points() const
{
	return mPoints;
}

//==================================================================================================

DrawingItemPoint* DrawingItem::pointAt(const QPointF& itemPos) const
{
	DrawingItemPoint* itemPoint = nullptr;

	for(auto pointIter = mPoints.begin(), pointEnd = mPoints.end();
		itemPoint == nullptr && pointIter != pointEnd; pointIter++)
	{
		if (itemPos == (*pointIter)->position()) itemPoint = *pointIter;
	}

	return itemPoint;
}

DrawingItemPoint* DrawingItem::pointNearest(const QPointF& itemPos) const
{
	DrawingItemPoint *itemPoint = nullptr;

	if (!mPoints.isEmpty())
	{
		itemPoint = mPoints.first();

		QPointF vec = (itemPoint->position() - itemPos);
		qreal minimumDistanceSq = vec.x() * vec.x() + vec.y() * vec.y();
		qreal distanceSq;

		for(auto pointIter = mPoints.begin() + 1, pointEnd = mPoints.end();
			pointIter != pointEnd; pointIter++)
		{
			vec = ((*pointIter)->position() - itemPos);
			distanceSq = vec.x() * vec.x() + vec.y() * vec.y();

			if (distanceSq < minimumDistanceSq)
			{
				itemPoint = *pointIter;
				minimumDistanceSq = distanceSq;
			}
		}
	}

	return itemPoint;
}

//==================================================================================================

void DrawingItem::setSelected(bool selected)
{
	mSelected = selected;
}

void DrawingItem::setVisible(bool visible)
{
	mVisible = visible;
}

bool DrawingItem::isSelected() const
{
	return mSelected;
}

bool DrawingItem::isVisible() const
{
	return mVisible;
}

//==================================================================================================

QPointF DrawingItem::mapFromScene(const QPointF& point) const
{
	return mTransform.map(point - mPosition);
}

QPolygonF DrawingItem::mapFromScene(const QRectF& rect) const
{
	return mapFromScene(QPolygonF(rect));
}

QPolygonF DrawingItem::mapFromScene(const QPolygonF& polygon) const
{
	QPolygonF poly = polygon;
	poly.translate(-mPosition);
	return mTransform.map(poly);
}

QPainterPath DrawingItem::mapFromScene(const QPainterPath& path) const
{
	QPainterPath painterPath = path;
	painterPath.translate(-mPosition);
	return mTransform.map(painterPath);
}

QPointF DrawingItem::mapToScene(const QPointF& point) const
{
	return mTransformInverse.map(point) + mPosition;
}

QPolygonF DrawingItem::mapToScene(const QRectF& rect) const
{
	return mapToScene(QPolygonF(rect));
}

QPolygonF DrawingItem::mapToScene(const QPolygonF& polygon) const
{
	QPolygonF poly = mTransformInverse.map(polygon);
	poly.translate(mPosition);
	return poly;
}

QPainterPath DrawingItem::mapToScene(const QPainterPath& path) const
{
	QPainterPath painterPath = mTransformInverse.map(path);
	painterPath.translate(mPosition);
	return painterPath;
}

//==================================================================================================

void DrawingItem::setProperties(const QHash<QString,QVariant>& properties)
{
	Q_UNUSED(properties);
}

QHash<QString,QVariant> DrawingItem::properties() const
{
	return QHash<QString,QVariant>();
}

//==================================================================================================

QPainterPath DrawingItem::shape() const
{
	QPainterPath path;
	path.addRect(boundingRect());
	return path;
}

QPointF DrawingItem::centerPos() const
{
	return boundingRect().center();
}

bool DrawingItem::isValid() const
{
	return boundingRect().isValid();
}

//==================================================================================================

void DrawingItem::moveEvent(const QPointF& scenePos)
{
	Q_UNUSED(scenePos);
}

void DrawingItem::resizeEvent(DrawingItemPoint* itemPoint, const QPointF& scenePos)
{
	Q_UNUSED(itemPoint);
	Q_UNUSED(scenePos);
}

void DrawingItem::rotateEvent(const QPointF& scenePos)
{
	Q_UNUSED(scenePos);
}

void DrawingItem::rotateBackEvent(const QPointF& scenePos)
{
	Q_UNUSED(scenePos);
}

void DrawingItem::flipHorizontalEvent(const QPointF& scenePos)
{
	Q_UNUSED(scenePos);
}

void DrawingItem::flipVerticalEvent(const QPointF& scenePos)
{
	Q_UNUSED(scenePos);
}

//==================================================================================================

DrawingItemPoint* DrawingItem::itemPointToInsert(const QPointF& itemPos, int& index)
{
	Q_UNUSED(itemPos);
	Q_UNUSED(index);
	return nullptr;
}

DrawingItemPoint* DrawingItem::itemPointToRemove(const QPointF& itemPos)
{
	Q_UNUSED(itemPos);
	return nullptr;
}

//==================================================================================================

void DrawingItem::keyPressEvent(QKeyEvent* event)
{
	Q_UNUSED(event);
}

void DrawingItem::keyReleaseEvent(QKeyEvent* event)
{
	Q_UNUSED(event);
}

//==================================================================================================

QPainterPath DrawingItem::strokePath(const QPainterPath& path, const QPen& pen) const
{
	if (path == QPainterPath()) return path;

	QPainterPathStroker ps;
	const qreal penWidthZero = qreal(0.00000001);

	if (pen.widthF() <= 0.0)
		ps.setWidth(penWidthZero);
	else
		ps.setWidth(pen.widthF());

	//ps.setCapStyle(pen.capStyle());
	//ps.setJoinStyle(pen.joinStyle());
	//ps.setMiterLimit(pen.miterLimit());

	ps.setCapStyle(Qt::SquareCap);
	ps.setJoinStyle(Qt::BevelJoin);

	return ps.createStroke(path);
}

//==================================================================================================

QList<DrawingItem*> DrawingItem::copyItems(const QList<DrawingItem*>& items)
{
	QList<DrawingItem*> copiedItems;
	QList<DrawingItemPoint*> itemPoints;
	QList<DrawingItemPoint*> targetPoints;
	DrawingItem* targetItem;
	DrawingItem* copiedTargetItem;
	DrawingItemPoint* copiedTargetPoint;
	DrawingItemPoint* copiedPoint;

	// Copy items
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
		copiedItems.append((*itemIter)->copy());

	// Maintain connections to other items in this list
	for(int itemIndex = 0, itemSize = items.size(); itemIndex < itemSize; itemIndex++)
	{
		itemPoints = items[itemIndex]->points();
		for(int pointIndex = 0, pointSize = itemPoints.size(); pointIndex < pointSize; pointIndex++)
		{
			targetPoints = itemPoints[pointIndex]->connections();
			for(auto targetIter = targetPoints.begin(), targetEnd = targetPoints.end();
				targetIter != targetEnd; targetIter++)
			{
				targetItem = (*targetIter)->item();
				if (items.contains(targetItem))
				{
					// There is a connection here that must be maintained in the copied items
					copiedPoint = copiedItems[itemIndex]->points().at(pointIndex);

					copiedTargetItem = copiedItems[items.indexOf(targetItem)];
					copiedTargetPoint =
						copiedTargetItem->points().at(targetItem->points().indexOf(*targetIter));

					if (copiedPoint && copiedTargetPoint)
					{
						copiedPoint->addConnection(copiedTargetPoint);
						copiedTargetPoint->addConnection(copiedPoint);
					}
				}
			}
		}
	}

	return copiedItems;
}
