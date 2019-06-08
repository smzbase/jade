/* DrawingEllipseItem.cpp
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

#include "DrawingEllipseItem.h"
#include "DrawingItemPoint.h"
#include <QPainter>
#include <QVariant>

DrawingEllipseItem::DrawingEllipseItem() : DrawingItem()
{
	mRect = QRectF(0, 0, 0, 0);
	mPen = QPen(Qt::black, 16, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	mBrush = Qt::white;

	setFlags(CanMove | CanResize | CanRotate | CanFlip | CanSelect | PlaceByMousePressAndRelease | AdjustPositionOnResize);

	DrawingItemPoint::Flags flags = (DrawingItemPoint::Control | DrawingItemPoint::Connection);
	for(int i = 0; i < 8; i++) addPoint(new DrawingItemPoint(QPointF(0, 0), flags));

	updateGeometry();
}

DrawingEllipseItem::DrawingEllipseItem(const DrawingEllipseItem& item) : DrawingItem(item)
{
	mRect = item.mRect;
	mPen = item.mPen;
	mBrush = item.mBrush;

	mBoundingRect = item.mBoundingRect;
	mShape = item.mShape;
}

DrawingEllipseItem::~DrawingEllipseItem() { }

//==================================================================================================

DrawingItem* DrawingEllipseItem::copy() const
{
	return new DrawingEllipseItem(*this);
}

//==================================================================================================

void DrawingEllipseItem::setEllipse(const QRectF& rect)
{
	mRect = rect;

	// Update points
	QList<DrawingItemPoint*> points = DrawingEllipseItem::points();
	if (points.size() >= 8)
	{
		points[TopLeft]->setPosition(rect.left(), rect.top());
		points[TopMiddle]->setPosition(rect.center().x(), rect.top());
		points[TopRight]->setPosition(rect.right(), rect.top());
		points[MiddleRight]->setPosition(rect.right(), rect.center().y());
		points[BottomRight]->setPosition(rect.right(), rect.bottom());
		points[BottomMiddle]->setPosition(rect.center().x(), rect.bottom());
		points[BottomLeft]->setPosition(rect.left(), rect.bottom());
		points[MiddleLeft]->setPosition(rect.left(), rect.center().y());
	}

	updateGeometry();
}

void DrawingEllipseItem::setEllipse(qreal left, qreal top, qreal width, qreal height)
{
	setEllipse(QRectF(left, top, width, height));
}

QRectF DrawingEllipseItem::ellipse() const
{
	return mRect;
}

//==================================================================================================

void DrawingEllipseItem::setPen(const QPen& pen)
{
	mPen = pen;
	updateGeometry();
}

QPen DrawingEllipseItem::pen() const
{
	return mPen;
}

//==================================================================================================

void DrawingEllipseItem::setBrush(const QBrush& brush)
{
	mBrush = brush;
	updateGeometry();
}

QBrush DrawingEllipseItem::brush() const
{
	return mBrush;
}

//==================================================================================================

void DrawingEllipseItem::setProperties(const QHash<QString,QVariant>& properties)
{
	if (properties.contains("pen-style"))
	{
		bool ok = false;
		uint value = properties["pen-style"].toUInt(&ok);
		if (ok) mPen.setStyle(static_cast<Qt::PenStyle>(value));
	}

	if (properties.contains("pen-color"))
	{
		QColor color = properties["pen-color"].value<QColor>();
		mPen.setBrush(color);
	}

	if (properties.contains("pen-width"))
	{
		bool ok = false;
		qreal value = properties["pen-width"].toDouble(&ok);
		if (ok) mPen.setWidthF(value);
	}

	if (properties.contains("pen-cap-style"))
	{
		bool ok = false;
		uint value = properties["pen-cap-style"].toUInt(&ok);
		if (ok) mPen.setCapStyle(static_cast<Qt::PenCapStyle>(value));
	}

	if (properties.contains("pen-join-style"))
	{
		bool ok = false;
		uint value = properties["pen-join-style"].toUInt(&ok);
		if (ok) mPen.setJoinStyle(static_cast<Qt::PenJoinStyle>(value));
	}

	if (properties.contains("brush-color"))
	{
		QColor color = properties["brush-color"].value<QColor>();
		mBrush = QBrush(color);
	}

	updateGeometry();
}

QHash<QString,QVariant> DrawingEllipseItem::properties() const
{
	QHash<QString,QVariant> properties;

	properties["pen-style"] = static_cast<uint>(mPen.style());
	properties["pen-color"] = mPen.brush().color();
	properties["pen-width"] = mPen.widthF();
	properties["pen-cap-style"] = static_cast<uint>(mPen.capStyle());
	properties["pen-join-style"] = static_cast<uint>(mPen.joinStyle());

	properties["brush-color"] = mBrush.color();

	return properties;
}

//==================================================================================================

QRectF DrawingEllipseItem::boundingRect() const
{
	return mBoundingRect;
}

QPainterPath DrawingEllipseItem::shape() const
{
	return mShape;
}

bool DrawingEllipseItem::isValid() const
{
	return (mRect.width() != 0 && mRect.height() != 0);
}

//==================================================================================================

void DrawingEllipseItem::render(QPainter* painter)
{
	if (isValid())
	{
		QBrush sceneBrush = painter->brush();
		QPen scenePen = painter->pen();

		// Draw rect
		painter->setBrush(mBrush);
		painter->setPen(mPen);
		painter->drawEllipse(mRect);

		painter->setBrush(sceneBrush);
		painter->setPen(scenePen);
	}
}

//==================================================================================================

void DrawingEllipseItem::resizeEvent(DrawingItemPoint* itemPoint, const QPointF& scenePos)
{
	DrawingItem::resizeEvent(itemPoint, scenePos);

	QList<DrawingItemPoint*> points = DrawingEllipseItem::points();

	if (points.size() >= 8)
	{
		int pointIndex = points.indexOf(itemPoint);

		if (0 <= pointIndex && pointIndex < 8)
		{
			QRectF rect;

			rect.setTopLeft(points[0]->position());
			rect.setBottomRight(points[1]->position());

			switch (pointIndex)
			{
			case TopLeft: rect.setTopLeft(itemPoint->position()); break;
			case TopMiddle:	rect.setTop(itemPoint->y()); break;
			case TopRight: rect.setTopRight(itemPoint->position()); break;
			case MiddleRight: rect.setRight(itemPoint->x()); break;
			case BottomRight: rect.setBottomRight(itemPoint->position()); break;
			case BottomMiddle: rect.setBottom(itemPoint->y()); break;
			case BottomLeft: rect.setBottomLeft(itemPoint->position()); break;
			case MiddleLeft: rect.setLeft(itemPoint->x()); break;
			default: break;
			}

			setEllipse(rect);
		}
	}
}

//==================================================================================================

void DrawingEllipseItem::updateGeometry()
{
	mBoundingRect = QRectF();
	mShape = QPainterPath();

	if (isValid())
	{
		qreal halfPenWidth = mPen.widthF() / 2;
		QRectF normalizedRect = mRect.normalized();
		QPainterPath drawPath;

		// Bounding rect
		mBoundingRect = normalizedRect;
		mBoundingRect.adjust(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);

		// Shape
		drawPath.addEllipse(normalizedRect);

		mShape = strokePath(drawPath, mPen);
		if (mBrush.color().alpha() > 0) mShape.addPath(drawPath);
	}
}
