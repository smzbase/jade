/* DrawingItemPoint.h
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

#ifndef DRAWINGITEMPOINT_H
#define DRAWINGITEMPOINT_H

#include <QList>
#include <QPointF>

class DrawingItem;

class DrawingItemPoint
{
	friend class DrawingItem;

public:
	enum Flag
	{
		NoFlags = 0x00,
		Control = 0x01,
		Connection = 0x02,
		Free = 0x04
	};
	Q_DECLARE_FLAGS(Flags, Flag)

private:
	DrawingItem* mItem;

	QPointF mPosition;
	Flags mFlags;

	QList<DrawingItemPoint*> mConnections;

public:
	DrawingItemPoint(const QPointF& pos = QPointF(), Flags flags = Control);
	DrawingItemPoint(const DrawingItemPoint& point);
	virtual ~DrawingItemPoint();

	DrawingItem* item() const;

	void setPosition(const QPointF& position);
	void setPosition(qreal x, qreal y);
	void setX(qreal x);
	void setY(qreal y);
	QPointF position() const;
	qreal x() const;
	qreal y() const;

	void setFlags(Flags flags);
	Flags flags() const;

	void addConnection(DrawingItemPoint* itemPoint);
	void removeConnection(DrawingItemPoint* itemPoint);
	void clearConnections();
	QList<DrawingItemPoint*> connections() const;

	bool isConnected(DrawingItemPoint* itemPoint) const;
	bool isConnected(DrawingItem* item) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DrawingItemPoint::Flags)

#endif
