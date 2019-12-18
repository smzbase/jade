/* DrawingItemContainer.h
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

#ifndef DRAWINGITEMCONTAINER_H
#define DRAWINGITEMCONTAINER_H

#include <QList>

class DrawingItem;

class DrawingItemContainer
{
private:
	QList<DrawingItem*> mItems;

public:
	DrawingItemContainer();
	virtual ~DrawingItemContainer();

	void addItem(DrawingItem* item);
	void insertItem(int index, DrawingItem* item);
	void removeItem(DrawingItem* item);
	void clearItems();
	QList<DrawingItem*> items() const;
};

#endif
