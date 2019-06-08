/* DrawingTextEllipseItem.h
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

#ifndef DRAWINGTEXTELLIPSEITEM_H
#define DRAWINGTEXTELLIPSEITEM_H

#include <DrawingItem.h>
#include <QFont>

class DrawingTextEllipseItem : public DrawingItem
{
private:
	enum PointIndex { TopLeft, BottomRight, TopRight, BottomLeft, TopMiddle, MiddleRight, BottomMiddle, MiddleLeft };

private:
	QRectF mRect;
	QPen mPen;
	QBrush mBrush;

	QString mCaption;
	QPen mTextPen;
	QFont mFont;

	QRectF mBoundingRect;
	QPainterPath mShape;
	QRectF mRectBoundingRect;
	QPainterPath mRectShape;
	QRectF mTextBoundingRect;

public:
	DrawingTextEllipseItem();
	DrawingTextEllipseItem(const DrawingTextEllipseItem& item);
	~DrawingTextEllipseItem();

	DrawingItem* copy() const;

	void setEllipse(const QRectF& rect);
	void setEllipse(qreal left, qreal top, qreal width, qreal height);
	QRectF ellipse() const;

	void setPen(const QPen& pen);
	QPen pen() const;

	void setBrush(const QBrush& brush);
	QBrush brush() const;

	void setCaption(const QString& caption);
	QString caption() const;

	void setTextColor(const QColor& color);
	QColor color() const;

	void setFont(const QFont& font);
	QFont font() const;

	void setProperties(const QHash<QString,QVariant>& properties);
	QHash<QString,QVariant> properties() const;

	QRectF boundingRect() const;
	QPainterPath shape() const;
	bool isValid() const;

	void render(QPainter* painter);

protected:
	void resizeEvent(DrawingItemPoint* itemPoint, const QPointF& scenePos);

	void updateGeometry();
	void updateTextRect(const QFont& font);
};

#endif
