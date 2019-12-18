/* DrawingItem.cpp
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

#include "DrawingItem.h"
#include "DrawingItemPoint.h"

DrawingItem::DrawingItem() : DrawingItemContainer()
{
	mParent = nullptr;
	mFlags = (CanMove | CanResize | CanRotate | CanFlip | CanSelect);
	mSelected = false;
	mVisible = true;
}

DrawingItem::DrawingItem(const DrawingItem& item) : DrawingItemContainer()
{
	mParent = nullptr;

	mPosition = item.mPosition;
	mTransform = item.mTransform;
	mTransformInverse = item.mTransformInverse;
	mFlags = item.mFlags;

	for(auto pointIter = item.mPoints.begin(), pointEnd = item.mPoints.end();
		pointIter != pointEnd; pointIter++)
	{
		addPoint(new DrawingItemPoint(**pointIter));
	}

	QList<DrawingItem*> otherChildItems = item.items();
	for(auto childIter = otherChildItems.begin(), childEnd = otherChildItems.end();
		childIter != childEnd; childIter++)
	{
		addItem((*childIter)->copy());
	}

	mSelected = false;
	mVisible = true;
}

DrawingItem::~DrawingItem()
{
	clearPoints();
	mParent = nullptr;
}

//==================================================================================================

DrawingItemContainer* DrawingItem::parent() const
{
	return mParent;
}

//==================================================================================================

void DrawingItem::setPosition(const QPointF& position)
{
	mPosition = position;
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

void DrawingItem::addPoint(DrawingItemPoint* point)
{
	if (point && point->mItem == nullptr)
	{
		mPoints.append(point);
		point->mItem = this;
	}
}

void DrawingItem::insertPoint(int index, DrawingItemPoint* point)
{
	if (point && point->mItem == nullptr)
	{
		mPoints.insert(index, point);
		point->mItem = this;
	}
}

void DrawingItem::removePoint(DrawingItemPoint* point)
{
	if (point && point->mItem == this)
	{
		mPoints.removeAll(point);
		point->mItem = nullptr;
	}
}

void DrawingItem::clearPoints()
{
	DrawingItemPoint* point = nullptr;

	while (!mPoints.empty())
	{
		point = mPoints.first();
		removePoint(point);
		delete point;
		point = nullptr;
	}
}

QList<DrawingItemPoint*> DrawingItem::points() const
{
	return mPoints;
}

//==================================================================================================

DrawingItemPoint* DrawingItem::pointAt(const QPointF& position) const
{
	DrawingItemPoint* point = nullptr;

	for(auto pointIter = mPoints.begin(), pointEnd = mPoints.end();
		point == nullptr && pointIter != pointEnd; pointIter++)
	{
		if (position == (*pointIter)->position()) point = *pointIter;
	}

	return point;
}

DrawingItemPoint* DrawingItem::pointNearest(const QPointF& position) const
{
	DrawingItemPoint *point = nullptr;

	if (!mPoints.isEmpty())
	{
		point = mPoints.first();

		QPointF vec = (point->position() - position);
		qreal minimumDistanceSq = vec.x() * vec.x() + vec.y() * vec.y();
		qreal distanceSq;

		for(auto pointIter = mPoints.begin() + 1, pointEnd = mPoints.end();
			pointIter != pointEnd; pointIter++)
		{
			vec = ((*pointIter)->position() - position);
			distanceSq = vec.x() * vec.x() + vec.y() * vec.y();

			if (distanceSq < minimumDistanceSq)
			{
				point = *pointIter;
				minimumDistanceSq = distanceSq;
			}
		}
	}

	return point;
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

QPointF DrawingItem::mapFromParent(const QPointF& point) const
{
	return mTransform.map(point - mPosition);
}

QPolygonF DrawingItem::mapFromParent(const QRectF& rect) const
{
	return mapFromParent(QPolygonF(rect));
}

QPolygonF DrawingItem::mapFromParent(const QPolygonF& polygon) const
{
	QPolygonF poly = polygon;
	poly.translate(-mPosition);
	return mTransform.map(poly);
}

QPainterPath DrawingItem::mapFromParent(const QPainterPath& path) const
{
	QPainterPath painterPath = path;
	painterPath.translate(-mPosition);
	return mTransform.map(painterPath);
}

QPointF DrawingItem::mapToParent(const QPointF& point) const
{
	return mTransformInverse.map(point) + mPosition;
}

QPolygonF DrawingItem::mapToParent(const QRectF& rect) const
{
	return mapToParent(QPolygonF(rect));
}

QPolygonF DrawingItem::mapToParent(const QPolygonF& polygon) const
{
	QPolygonF poly = mTransformInverse.map(polygon);
	poly.translate(mPosition);
	return poly;
}

QPainterPath DrawingItem::mapToParent(const QPainterPath& path) const
{
	QPainterPath painterPath = mTransformInverse.map(path);
	painterPath.translate(mPosition);
	return painterPath;
}

//==================================================================================================

QPainterPath DrawingItem::shape() const
{
	QPainterPath path;
	path.addRect(boundingRect());
	return path;
}

QPointF DrawingItem::center() const
{
	return boundingRect().center();
}

bool DrawingItem::isValid() const
{
	return boundingRect().isValid();
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
