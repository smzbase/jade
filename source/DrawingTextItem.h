/* DrawingTextItem.h
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

#ifndef DRAWINGTEXTITEM_H
#define DRAWINGTEXTITEM_H

#include <DrawingItem.h>
#include <QFont>

class DrawingTextItem : public DrawingItem
{
private:
	QString mCaption;
	QPen mPen;
	QFont mFont;
	Qt::Alignment mAlignment;

	QRectF mTextRect;
	QPainterPath mTextShape;

public:
	DrawingTextItem();
	DrawingTextItem(const DrawingTextItem& item);
	~DrawingTextItem();

	DrawingItem* copy() const;

	void setCaption(const QString& caption);
	QString caption() const;

	void setTextColor(const QColor& color);
	QColor color() const;

	void setFont(const QFont& font);
	QFont font() const;

	void setAlignment(Qt::Alignment alignment);
	Qt::Alignment alignment() const;

	void setProperties(const QHash<QString,QVariant>& properties);
	QHash<QString,QVariant> properties() const;

	QRectF boundingRect() const;
	QPainterPath shape() const;
	QPointF centerPos() const;
	bool isValid() const;

	void render(QPainter* painter);

protected:
	void updateTextRect(const QFont& font);
};

#endif
