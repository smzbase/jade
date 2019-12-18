/* DrawingItemContainer.cpp
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

#include "DrawingItemContainer.h"
#include "DrawingItem.h"

DrawingItemContainer::DrawingItemContainer() { }

DrawingItemContainer::~DrawingItemContainer()
{
	clearItems();
}

//==================================================================================================

void DrawingItemContainer::addItem(DrawingItem* item)
{
	if (item && item->mParent == nullptr)
	{
		mItems.append(item);
		item->mParent = this;
	}
}

void DrawingItemContainer::insertItem(int index, DrawingItem* item)
{
	if (item && item->mParent == nullptr)
	{
		mItems.insert(index, item);
		item->mParent = this;
	}
}

void DrawingItemContainer::removeItem(DrawingItem* item)
{
	if (item && item->mParent == this)
	{
		mItems.removeAll(item);
		item->mParent = nullptr;
	}
}

void DrawingItemContainer::clearItems()
{
	DrawingItem* item = nullptr;

	while (!mItems.empty())
	{
		item = mItems.first();
		removeItem(item);
		delete item;
		item = nullptr;
	}
}

QList<DrawingItem*> DrawingItemContainer::items() const
{
	return mItems;
}
