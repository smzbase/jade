/* DrawingWidget.cpp
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

#include "DrawingWidget.h"
#include "DrawingItem.h"
#include "DrawingItemGroup.h"
#include "DrawingItemPoint.h"
#include "DrawingReader.h"
#include "DrawingUndo.h"
#include "DrawingWriter.h"
#include <QtMath>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>
#include <QPainter>

DrawingWidget::DrawingWidget(QWidget* parent) : DrawingWidgetBase(parent)
{
	setMouseTracking(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	mSelectedItemPoint = nullptr;

	mMouseDownItem = nullptr;
	mFocusItem = nullptr;

	mMouseState = MouseReady;
	mMouseDragged = false;

	addActions();
	createContextMenus();

	connect(this, SIGNAL(modeChanged(DrawingWidgetBase::Mode)),	this, SLOT(clearMode(DrawingWidgetBase::Mode)));

	connect(this, SIGNAL(selectionChanged(const QList<DrawingItem*>&)),	this, SLOT(updateSelectionCenter()));
	connect(this, SIGNAL(itemsPositionChanged(const QList<DrawingItem*>&)), this, SLOT(updateSelectionCenter()));
	connect(this, SIGNAL(itemsGeometryChanged(const QList<DrawingItem*>&)), this, SLOT(updateSelectionCenter()));

	connect(this, SIGNAL(selectionChanged(const QList<DrawingItem*>&)),	this, SLOT(updateActionsFromSelection()));
}

DrawingWidget::~DrawingWidget()
{
	clearItems();
}

//==================================================================================================

void DrawingWidget::addItem(DrawingItem* item)
{
	if (item && item->mWidget == nullptr)
	{
		mItems.append(item);
		item->mWidget = this;
	}
}

void DrawingWidget::insertItem(int index, DrawingItem* item)
{
	if (item && item->mWidget == nullptr)
	{
		mItems.insert(index, item);
		item->mWidget = this;
	}
}

void DrawingWidget::removeItem(DrawingItem* item)
{
	if (item && item->mWidget == this)
	{
		mItems.removeAll(item);
		item->mWidget = nullptr;
	}
}

void DrawingWidget::clearItems()
{
	DrawingItem* item = nullptr;

	setDefaultMode();

	mInitialPositions.clear();

	mMouseDownItem = nullptr;
	mFocusItem = nullptr;

	mSelectedItems.clear();
	mSelectedItemPoint = nullptr;

	while (!mItems.empty())
	{
		item = mItems.first();
		removeItem(item);
		delete item;
		item = nullptr;
	}
}

QList<DrawingItem*> DrawingWidget::items() const
{
	return mItems;
}

//==================================================================================================

QList<DrawingItem*> DrawingWidget::items(const QPointF& pos) const
{
	QList<DrawingItem*> items;

	for(auto itemIter = mItems.begin(), itemEnd = mItems.end(); itemIter != itemEnd; itemIter++)
	{
		if ((*itemIter)->isVisible() && itemMatchesPoint(*itemIter, pos)) items.append(*itemIter);
	}

	return items;
}

QList<DrawingItem*> DrawingWidget::items(const QRectF& rect) const
{
	QList<DrawingItem*> items;

	for(auto itemIter = mItems.begin(), itemEnd = mItems.end(); itemIter != itemEnd; itemIter++)
	{
		if ((*itemIter)->isVisible() && itemMatchesRect(*itemIter, rect, Qt::ContainsItemBoundingRect))
			items.append(*itemIter);
	}

	return items;
}

QList<DrawingItem*> DrawingWidget::items(const QPainterPath& path) const
{
	QList<DrawingItem*> items;

	for(auto itemIter = mItems.begin(), itemEnd = mItems.end(); itemIter != itemEnd; itemIter++)
	{
		if ((*itemIter)->isVisible() && itemMatchesPath(*itemIter, path, Qt::ContainsItemBoundingRect))
			items.append(*itemIter);
	}

	return items;
}

DrawingItem* DrawingWidget::itemAt(const QPointF& pos) const
{
	DrawingItem* item = nullptr;

	// Favor selected items
	auto selectedItemIter = mSelectedItems.end();
	while (item == nullptr && selectedItemIter != mSelectedItems.begin())
	{
		selectedItemIter--;
		if (itemMatchesPoint(*selectedItemIter, pos)) item = *selectedItemIter;
	}

	// Search all visible items
	auto itemIter = mItems.constEnd();
	while (item == nullptr && itemIter != mItems.constBegin())
	{
		itemIter--;
		if ((*itemIter)->isVisible() && itemMatchesPoint(*itemIter, pos)) item = *itemIter;
	}

	return item;
}

//==================================================================================================

void DrawingWidget::selectItem(DrawingItem* item)
{
	if (item && !mSelectedItems.contains(item))
	{
		item->setSelected(true);
		mSelectedItems.append(item);
	}
}

void DrawingWidget::deselectItem(DrawingItem* item)
{
	if (item && mSelectedItems.contains(item))
	{
		mSelectedItems.removeAll(item);
		item->setSelected(false);
	}
}

void DrawingWidget::clearSelection()
{
	for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
		(*itemIter)->setSelected(false);

	mSelectedItems.clear();
}

QList<DrawingItem*> DrawingWidget::selectedItems() const
{
	return mSelectedItems;
}

DrawingItemPoint* DrawingWidget::selectedItemPoint() const
{
	return mSelectedItemPoint;
}

QPointF DrawingWidget::selectionCenter() const
{
	return mSelectionCenter;
}

//==================================================================================================

QList<DrawingItem*> DrawingWidget::placeItems() const
{
	return mPlaceItems;
}

DrawingItem* DrawingWidget::mouseDownItem() const
{
	return mMouseDownItem;
}

DrawingItem* DrawingWidget::focusItem() const
{
	return mFocusItem;
}

//==================================================================================================

void DrawingWidget::setProperties(const QHash<QString,QVariant>& properties)
{
	if (properties.contains("scene-rect"))
	{
		QRectF rect = properties["scene-rect"].toRectF();
		if (rect.isValid()) setSceneRect(rect);
	}

	if (properties.contains("background-color"))
	{
		QColor color = properties["background-color"].value<QColor>();
		setBackgroundBrush(color);
	}

	if (properties.contains("grid"))
	{
		bool ok = false;
		qreal value = properties["grid"].toDouble(&ok);
		if (ok) setGrid(value);
	}

	if (properties.contains("grid-style"))
	{
		bool ok = false;
		uint value = properties["grid-style"].toUInt(&ok);
		if (ok) setGridStyle(static_cast<GridStyle>(value));
	}

	if (properties.contains("grid-color"))
	{
		QColor color = properties["grid-color"].value<QColor>();
		setGridBrush(color);
	}

	if (properties.contains("grid-spacing-major"))
	{
		bool ok = false;
		int value = properties["grid-spacing-major"].toInt(&ok);
		if (ok) setGridSpacing(value, gridSpacingMinor());
	}

	if (properties.contains("grid-spacing-minor"))
	{
		bool ok = false;
		int value = properties["grid-spacing-minor"].toInt(&ok);
		if (ok) setGridSpacing(gridSpacingMajor(), value);
	}

	if (properties.contains("dynamic-grid"))
	{
		bool ok = false;
		qreal value = properties["dynamic-grid"].toDouble(&ok);
		if (ok) setDynamicGrid(value);
	}
}

QHash<QString,QVariant> DrawingWidget::properties() const
{
	QHash<QString,QVariant> properties;

	properties["scene-rect"] = sceneRect();
	properties["background-color"] = backgroundBrush().color();
	properties["grid"] = grid();
	properties["grid-style"] = gridStyle();
	properties["grid-color"] = gridBrush().color();
	properties["grid-spacing-major"] = gridSpacingMajor();
	properties["grid-spacing-minor"] = gridSpacingMinor();
	properties["dynamic-grid"] = dynamicGrid();

	return properties;
}

//==================================================================================================

void DrawingWidget::renderExport(QPainter* painter)
{
	painter->setBrush(backgroundBrush());
	painter->setPen(Qt::NoPen);
	painter->drawRect(sceneRect());

	drawItems(painter, mItems);
}

//==================================================================================================

void DrawingWidget::setPlaceMode(const QList<DrawingItem*>& items)
{
	setDefaultMode();

	if (!items.isEmpty())
	{
		QPointF centerPos, deltaPos;

		setCursor(Qt::CrossCursor);

		mPlaceItems = items;

		for(auto itemIter = mPlaceItems.begin(), itemEnd = mPlaceItems.end(); itemIter != itemEnd; itemIter++)
			centerPos += (*itemIter)->mapToScene((*itemIter)->centerPos());
		centerPos /= mPlaceItems.size();

		deltaPos = roundToGrid(mapToScene(mapFromGlobal(QCursor::pos())) - centerPos);

		for(auto itemIter = mPlaceItems.begin(), itemEnd = mPlaceItems.end(); itemIter != itemEnd; itemIter++)
			(*itemIter)->setPosition((*itemIter)->position() + deltaPos);

		emit placeItemsChanged(mPlaceItems);

		viewport()->update();
	}
}

//==================================================================================================

void DrawingWidget::undo()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && canUndo())
		DrawingWidgetBase::undo();
}

void DrawingWidget::redo()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && canRedo())
		DrawingWidgetBase::redo();
}

//==================================================================================================

void DrawingWidget::cut()
{
	copy();
	deleteSelection();
}

void DrawingWidget::copy()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		QClipboard* clipboard = QApplication::clipboard();

		if (clipboard && !mSelectedItems.isEmpty())
		{
			QString xmlItems;

			DrawingWriter writer;
			xmlItems = writer.writeItems(mSelectedItems);

			clipboard->setText(xmlItems);
		}
	}
}

void DrawingWidget::paste()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		QClipboard* clipboard = QApplication::clipboard();
		QList<DrawingItem*> newItems;

		if (clipboard)
		{
			QString xmlItems = clipboard->text();

			DrawingReader reader;
			newItems = reader.readItems(xmlItems);
		}

		if (!newItems.isEmpty())
		{
			for(auto itemIter = newItems.begin(); itemIter != newItems.end(); itemIter++)
				(*itemIter)->setFlags((*itemIter)->flags() & (~DrawingItem::PlaceByMousePressAndRelease));

			selectNone();
			setPlaceMode(newItems);
		}
	}
}

void DrawingWidget::deleteSelection()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		if (!mSelectedItems.isEmpty())
		{
			QList<DrawingItem*> itemsToRemove = mSelectedItems;

			QUndoCommand* deleteCommand = new QUndoCommand("Delete Items");

			selectItemsCommand(QList<DrawingItem*>(), deleteCommand);
			removeItemsCommand(itemsToRemove, deleteCommand);

			pushUndoCommand(deleteCommand);

			viewport()->update();
		}
	}
	else setDefaultMode();
}

//==================================================================================================

void DrawingWidget::selectItems(const QList<DrawingItem*>& items)
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			(*itemIter)->setSelected(false);

		mSelectedItems = items;

		for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			(*itemIter)->setSelected(true);

		emit selectionChanged(mSelectedItems);
	}
}

void DrawingWidget::selectArea(const QRectF& rect)
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		QList<DrawingItem*> foundItems = items(rect);
		QList<DrawingItem*> itemsToSelect;

		for(auto itemIter = foundItems.begin(); itemIter != foundItems.end(); itemIter++)
		{
			if ((*itemIter)->flags() & DrawingItem::CanSelect) itemsToSelect.append(*itemIter);
		}

		if (mSelectedItems != itemsToSelect)
		{
			selectItemsCommand(itemsToSelect, true);
			viewport()->update();
		}
	}
}

void DrawingWidget::selectArea(const QPainterPath& path)
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		QList<DrawingItem*> foundItems = items(path);
		QList<DrawingItem*> itemsToSelect;

		for(auto itemIter = foundItems.begin(); itemIter != foundItems.end(); itemIter++)
		{
			if ((*itemIter)->flags() & DrawingItem::CanSelect) itemsToSelect.append(*itemIter);
		}

		if (mSelectedItems != itemsToSelect)
		{
			selectItemsCommand(itemsToSelect, true);
			viewport()->update();
		}
	}
}

void DrawingWidget::selectAll()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		QList<DrawingItem*> foundItems = items();
		QList<DrawingItem*> itemsToSelect;

		for(auto itemIter = foundItems.begin(); itemIter != foundItems.end(); itemIter++)
		{
			if ((*itemIter)->flags() & DrawingItem::CanSelect) itemsToSelect.append(*itemIter);
		}
		if (mSelectedItems != itemsToSelect)
		{
			selectItemsCommand(itemsToSelect, true);
			viewport()->update();
		}
	}
}

void DrawingWidget::selectNone()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		selectItemsCommand(QList<DrawingItem*>(), true);
		viewport()->update();
	}
}

//==================================================================================================

void DrawingWidget::moveSelection(const QPointF& deltaPos)
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
	{
		QList<DrawingItem*> itemsToMove;
		QHash<DrawingItem*,QPointF> originalPositions, newPositions;

		for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			originalPositions[*itemIter] = (*itemIter)->position();

		for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
		{
			if ((*itemIter)->flags() & DrawingItem::CanMove)
			{
				itemsToMove.append(*itemIter);
				newPositions[*itemIter] = (*itemIter)->position() + deltaPos;
				(*itemIter)->setPosition(newPositions[*itemIter]);
			}
		}

		for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			(*itemIter)->setPosition(originalPositions[*itemIter]);

		if (!itemsToMove.isEmpty())
		{
			moveItemsCommand(itemsToMove, newPositions, true);
			viewport()->update();
		}
	}
}

void DrawingWidget::resizeSelection(DrawingItemPoint* itemPoint, const QPointF& pos)
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && mSelectedItems.size() == 1 &&
		(mSelectedItems.first()->flags() & DrawingItem::CanResize) &&
		mSelectedItems.first()->points().contains(itemPoint))
	{
		resizeItemCommand(itemPoint, pos, true, true);
		viewport()->update();
	}
}

void DrawingWidget::rotateSelection()
{
	if (mode() == DefaultMode)
	{
		if (mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
		{
			QList<DrawingItem*> itemsToRotate;
			for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanRotate) itemsToRotate.append(*itemIter);
			}

			if (!itemsToRotate.isEmpty())
			{
				rotateItemsCommand(itemsToRotate, roundToGrid(mSelectionCenter));
				viewport()->update();
			}
		}
		else if (!mPlaceItems.isEmpty())
		{
			QList<DrawingItem*> itemsToRotate;
			for(auto itemIter = mPlaceItems.begin(); itemIter != mPlaceItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanRotate) itemsToRotate.append(*itemIter);
			}

			if (!itemsToRotate.isEmpty())
			{
				QPointF scenePos = roundToGrid(mapToScene(mapFromGlobal(QCursor::pos())));
				QHash<DrawingItem*,QPointF> parentPos;

				for(auto itemIter = itemsToRotate.begin(); itemIter != itemsToRotate.end(); itemIter++)
					parentPos[*itemIter] = scenePos;

				rotateItems(itemsToRotate, parentPos);
				viewport()->update();
			}
		}
	}
}

void DrawingWidget::rotateBackSelection()
{
	if (mode() == DefaultMode)
	{
		if (mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
		{
			QList<DrawingItem*> itemsToRotate;
			for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanRotate) itemsToRotate.append(*itemIter);
			}

			if (!itemsToRotate.isEmpty())
			{
				rotateBackItemsCommand(itemsToRotate, roundToGrid(mSelectionCenter));
				viewport()->update();
			}
		}
		else if (!mPlaceItems.isEmpty())
		{
			QList<DrawingItem*> itemsToRotate;
			for(auto itemIter = mPlaceItems.begin(); itemIter != mPlaceItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanRotate) itemsToRotate.append(*itemIter);
			}

			if (!itemsToRotate.isEmpty())
			{
				QPointF scenePos = roundToGrid(mapToScene(mapFromGlobal(QCursor::pos())));
				QHash<DrawingItem*,QPointF> parentPos;

				for(auto itemIter = itemsToRotate.begin(); itemIter != itemsToRotate.end(); itemIter++)
					parentPos[*itemIter] = scenePos;

				rotateBackItems(itemsToRotate, parentPos);
				viewport()->update();
			}
		}
	}
}

void DrawingWidget::flipSelectionHorizontal()
{
	if (mode() == DefaultMode)
	{
		if (mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
		{
			QList<DrawingItem*> itemsToFlip;
			for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanFlip) itemsToFlip.append(*itemIter);
			}

			if (!itemsToFlip.isEmpty())
			{
				flipItemsHorizontalCommand(itemsToFlip, roundToGrid(mSelectionCenter));
				viewport()->update();
			}
		}
		else if (!mPlaceItems.isEmpty())
		{
			QList<DrawingItem*> itemsToFlip;
			for(auto itemIter = mPlaceItems.begin(); itemIter != mPlaceItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanFlip) itemsToFlip.append(*itemIter);
			}

			if (!itemsToFlip.isEmpty())
			{
				QPointF scenePos = roundToGrid(mapToScene(mapFromGlobal(QCursor::pos())));
				QHash<DrawingItem*,QPointF> parentPos;

				for(auto itemIter = itemsToFlip.begin(); itemIter != itemsToFlip.end(); itemIter++)
					parentPos[*itemIter] = scenePos;

				flipItemsHorizontal(itemsToFlip, parentPos);
				viewport()->update();
			}
		}
	}
}

void DrawingWidget::flipSelectionVertical()
{
	if (mode() == DefaultMode)
	{
		if (mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
		{
			QList<DrawingItem*> itemsToFlip;
			for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanFlip) itemsToFlip.append(*itemIter);
			}

			if (!itemsToFlip.isEmpty())
			{
				flipItemsVerticalCommand(itemsToFlip, roundToGrid(mSelectionCenter));
				viewport()->update();
			}
		}
		else if (!mPlaceItems.isEmpty())
		{
			QList<DrawingItem*> itemsToFlip;
			for(auto itemIter = mPlaceItems.begin(); itemIter != mPlaceItems.end(); itemIter++)
			{
				if ((*itemIter)->flags() & DrawingItem::CanFlip) itemsToFlip.append(*itemIter);
			}

			if (!itemsToFlip.isEmpty())
			{
				QPointF scenePos = roundToGrid(mapToScene(mapFromGlobal(QCursor::pos())));
				QHash<DrawingItem*,QPointF> parentPos;

				for(auto itemIter = itemsToFlip.begin(); itemIter != itemsToFlip.end(); itemIter++)
					parentPos[*itemIter] = scenePos;

				flipItemsVertical(itemsToFlip, parentPos);
				viewport()->update();
			}
		}
	}
}

//==================================================================================================

void DrawingWidget::bringForward()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
	{
		QList<DrawingItem*> itemsToReorder = mSelectedItems;
		QList<DrawingItem*> itemsOrdered = mItems;
		DrawingItem* item;
		int itemIndex;

		while (!itemsToReorder.empty())
		{
			item = itemsToReorder.takeLast();

			itemIndex = itemsOrdered.indexOf(item);
			if (itemIndex >= 0)
			{
				itemsOrdered.removeAll(item);
				itemsOrdered.insert(itemIndex + 1, item);
			}
		}

		reorderItemsCommand(itemsOrdered);
		viewport()->update();
	}
}

void DrawingWidget::sendBackward()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
	{
		QList<DrawingItem*> itemsToReorder = mSelectedItems;
		QList<DrawingItem*> itemsOrdered = mItems;
		DrawingItem* item;
		int itemIndex;

		while (!itemsToReorder.empty())
		{
			item = itemsToReorder.takeLast();

			itemIndex = itemsOrdered.indexOf(item);
			if (itemIndex >= 0)
			{
				itemsOrdered.removeAll(item);
				itemsOrdered.insert(itemIndex - 1, item);
			}
		}

		reorderItemsCommand(itemsOrdered);
		viewport()->update();
	}
}

void DrawingWidget::bringToFront()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
	{
		QList<DrawingItem*> itemsToReorder = mSelectedItems;
		QList<DrawingItem*> itemsOrdered = mItems;
		DrawingItem* item;
		int itemIndex;

		while (!itemsToReorder.empty())
		{
			item = itemsToReorder.takeLast();

			itemIndex = itemsOrdered.indexOf(item);
			if (itemIndex >= 0)
			{
				itemsOrdered.removeAll(item);
				itemsOrdered.append(item);
			}
		}

		reorderItemsCommand(itemsOrdered);
		viewport()->update();
	}
}

void DrawingWidget::sendToBack()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && !mSelectedItems.isEmpty())
	{
		QList<DrawingItem*> itemsToReorder = mSelectedItems;
		QList<DrawingItem*> itemsOrdered = mItems;
		DrawingItem* item;
		int itemIndex;

		while (!mSelectedItems.empty())
		{
			item = mSelectedItems.takeLast();

			itemIndex = itemsOrdered.indexOf(item);
			if (itemIndex >= 0)
			{
				itemsOrdered.removeAll(item);
				itemsOrdered.prepend(item);
			}
		}

		reorderItemsCommand(itemsOrdered);
		viewport()->update();
	}
}


//==================================================================================================

void DrawingWidget::insertItemPoint()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		DrawingItem* item = nullptr;
		if (mSelectedItems.size() == 1) item = mSelectedItems.first();

		if (item && (item->flags() & DrawingItem::CanInsertPoints))
		{
			int index = 0;
			DrawingItemPoint* pointToInsert = item->itemPointToInsert(item->mapFromScene(roundToGrid(mMouseButtonDownScenePos)), index);
			if (pointToInsert)
			{
				pushUndoCommand(new DrawingItemInsertPointCommand(this, item, pointToInsert, index));
				viewport()->update();
			}
		}
	}
}

void DrawingWidget::removeItemPoint()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty())
	{
		DrawingItem* item = nullptr;
		if (mSelectedItems.size() == 1) item = mSelectedItems.first();

		if (item && (item->flags() & DrawingItem::CanRemovePoints))
		{
			DrawingItemPoint* pointToRemove = item->itemPointToRemove(item->mapFromScene(roundToGrid(mMouseButtonDownScenePos)));
			if (pointToRemove)
			{
				QUndoCommand* removeCommand = new QUndoCommand("Remove Point");

				disconnectAll(pointToRemove, removeCommand);
				new DrawingItemRemovePointCommand(this, item, pointToRemove, removeCommand);

				pushUndoCommand(removeCommand);
				viewport()->update();
			}
		}
	}
}

//==================================================================================================

void DrawingWidget::group()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && mSelectedItems.size() > 1)
	{
		QUndoCommand* command = new QUndoCommand("Group Items");

		QList<DrawingItem*> itemsToGroup = mSelectedItems;
		QList<DrawingItem*> items = DrawingItem::copyItems(itemsToGroup);
		DrawingItemGroup* itemGroup = new DrawingItemGroup();
		QList<DrawingItem*> itemsToAdd;

		itemGroup->setPosition(items.first()->position());
		for(auto iter = items.begin(); iter != items.end(); iter++)
			(*iter)->setPosition(itemGroup->mapFromScene((*iter)->position()));
		itemGroup->setItems(items);
		itemsToAdd.append(itemGroup);

		selectItemsCommand(QList<DrawingItem*>(), true, command);
		removeItemsCommand(itemsToGroup, command);
		addItemsCommand(itemsToAdd, false, command);
		selectItemsCommand(itemsToAdd, true, command);

		pushUndoCommand(command);
		viewport()->update();
	}
}

void DrawingWidget::ungroup()
{
	if (mode() == DefaultMode && mPlaceItems.isEmpty() && mSelectedItems.size() == 1)
	{
		DrawingItemGroup* itemGroup = dynamic_cast<DrawingItemGroup*>(mSelectedItems.first());
		if (itemGroup)
		{
			QUndoCommand* command = new QUndoCommand("Ungroup Items");
			QList<DrawingItem*> itemsToRemove;
			itemsToRemove.append(itemGroup);

			QList<DrawingItem*> items = DrawingItem::copyItems(itemGroup->items());
			for(auto iter = items.begin(); iter != items.end(); iter++)
			{
				(*iter)->setPosition(itemGroup->mapToScene((*iter)->position()));
				(*iter)->setTransform(itemGroup->transform(), true);
			}

			selectItemsCommand(QList<DrawingItem*>(), true, command);
			removeItemsCommand(itemsToRemove, command);
			addItemsCommand(items, false, command);
			selectItemsCommand(items, true, command);

			pushUndoCommand(command);
			viewport()->update();
		}
	}
}

//==================================================================================================

void DrawingWidget::setSelectionProperties(const QHash<QString,QVariant>& properties)
{
	if (!mSelectedItems.isEmpty() && !properties.isEmpty())
	{
		pushUndoCommand(new DrawingItemSetPropertiesCommand(this, mSelectedItems, properties));
		viewport()->update();
	}
}

void DrawingWidget::setDrawingProperties(const QHash<QString,QVariant>& properties)
{
	if (!properties.isEmpty())
	{
		pushUndoCommand(new DrawingSetPropertiesCommand(this, properties));
		viewport()->update();
	}
}

//==================================================================================================

void DrawingWidget::clearMode(DrawingWidgetBase::Mode mode)
{
	Q_UNUSED(mode);

	while (!mPlaceItems.isEmpty()) delete mPlaceItems.takeFirst();
	emit placeItemsChanged(mPlaceItems);

	clearSelection();
	emit selectionChanged(mSelectedItems);
}

void DrawingWidget::updateSelectionCenter()
{
	mSelectionCenter = QPointF();

	if (!mSelectedItems.isEmpty())
	{
		for(auto itemIter = mSelectedItems.begin(), itemEnd = mSelectedItems.end(); itemIter != itemEnd; itemIter++)
			mSelectionCenter += (*itemIter)->mapToScene((*itemIter)->centerPos());

		mSelectionCenter /= mSelectedItems.size();
	}
}

void DrawingWidget::updateActionsFromSelection()
{
	QList<QAction*> actions = DrawingWidget::actions();

	bool canInsertRemovePoints = false;
	bool canGroup = (mSelectedItems.size() > 1);
	bool canUngroup = false;

	if (mSelectedItems.size() == 1)
	{
		DrawingItem* item = mSelectedItems.first();
		DrawingItemGroup* groupItem = dynamic_cast<DrawingItemGroup*>(item);

		canInsertRemovePoints = ((item->flags() & DrawingItem::CanInsertPoints) ||
			(item->flags() & DrawingItem::CanRemovePoints));
		canUngroup = (groupItem);
	}

	actions[InsertPointAction]->setEnabled(canInsertRemovePoints);
	actions[RemovePointAction]->setEnabled(canInsertRemovePoints);
	actions[GroupAction]->setEnabled(canGroup);
	actions[UngroupAction]->setEnabled(canUngroup);
}

//==================================================================================================

void DrawingWidget::mousePressEvent(QMouseEvent* event)
{
	DrawingWidgetBase::mousePressEvent(event);

	if (event->button() == Qt::LeftButton)
	{
		mMouseButtonDownPos = event->pos();
		mMouseButtonDownScenePos = mapToScene(mMouseButtonDownPos);
		mMouseDragged = false;

		if (mode() == DefaultMode && mPlaceItems.isEmpty())
		{
			mMouseState = MouseSelect;

			mMouseDownItem = itemAt(mMouseButtonDownScenePos);
			if (mMouseDownItem)
			{
				mInitialPositions.clear();
				for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
					mInitialPositions[*itemIter] = (*itemIter)->position();

				if (mMouseDownItem->isSelected() && mSelectedItems.size() == 1)
				{
					mSelectedItemPoint = pointAt(mMouseDownItem, mMouseDownItem->mapFromScene(mMouseButtonDownScenePos));
					if (mSelectedItemPoint)
					{
						if (mSelectedItemPoint->flags() & DrawingItemPoint::Control)
							mInitialItemPointPosition = mMouseDownItem->mapToScene(mSelectedItemPoint->position());
						else
							mSelectedItemPoint = nullptr;
					}
				}
			}

			mFocusItem = mMouseDownItem;
		}
	}
	else if (event->button() == Qt::RightButton)
	{
		mMouseButtonDownPos = event->pos();
		mMouseButtonDownScenePos = mapToScene(event->pos());
		mMouseDownItem = itemAt(mMouseButtonDownScenePos);
	}

	viewport()->update();
	emit mouseInfoChanged("");
}

void DrawingWidget::mouseMoveEvent(QMouseEvent* event)
{
	DrawingWidgetBase::mouseMoveEvent(event);

	QPointF scenePos = mapToScene(event->pos());

	mMouseDragged = (mMouseDragged |
		((mMouseButtonDownPos - event->pos()).manhattanLength() >= QApplication::startDragDistance()));

	if (mode() == DefaultMode)
	{
		if (!mPlaceItems.isEmpty())
		{
			if (event->buttons() & Qt::LeftButton &&
				mPlaceItems.size() == 1 && (mPlaceItems.first()->flags() & DrawingItem::PlaceByMousePressAndRelease)
				&& mPlaceItems.first()->points().size() >= 2)
			{
				resizeItem(mPlaceItems.first()->points()[1], roundToGrid(scenePos));
			}
			else
			{
				QPointF centerPos, deltaPos;

				for(auto itemIter = mPlaceItems.begin(); itemIter != mPlaceItems.end(); itemIter++)
					centerPos += (*itemIter)->mapToScene((*itemIter)->centerPos());
				if (!mPlaceItems.isEmpty()) centerPos /= mPlaceItems.size();

				deltaPos = roundToGrid(scenePos - centerPos);

				for(auto itemIter = mPlaceItems.begin(); itemIter != mPlaceItems.end(); itemIter++)
					(*itemIter)->setPosition((*itemIter)->position() + deltaPos);

				emit itemsGeometryChanged(mPlaceItems);
			}

			if (event->buttons() & Qt::LeftButton)
				sendMouseInfoText(roundToGrid(mMouseButtonDownScenePos), roundToGrid(scenePos));
			else
				sendMouseInfoText(roundToGrid(scenePos));
		}
		else
		{
			if (event->buttons() & Qt::LeftButton)
			{
				QPointF deltaScenePos;
				QList<DrawingItem*> itemsToMove;
				QHash<DrawingItem*,QPointF> originalPositions, newPositions;

				switch (mMouseState)
				{
				case MouseSelect:
					if (mMouseDragged)
					{
						if (mMouseDownItem && mMouseDownItem->isSelected())
						{
							bool resizeItem = (mSelectedItems.size() == 1 &&
								(mSelectedItems.first()->flags() & DrawingItem::CanResize) &&
								mSelectedItemPoint && (mSelectedItemPoint->flags() & DrawingItemPoint::Control));
							mMouseState = (resizeItem) ? MouseResizeItem : MouseMoveItems;
						}
						else mMouseState = MouseRubberBand;
					}
					sendMouseInfoText(scenePos);
					break;

				case MouseMoveItems:
					for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
						originalPositions[*itemIter] = (*itemIter)->position();

					deltaScenePos = roundToGrid(scenePos - mMouseButtonDownScenePos);
					for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
					{
						if ((*itemIter)->flags() & DrawingItem::CanMove)
						{
							itemsToMove.append(*itemIter);
							newPositions[*itemIter] = mInitialPositions[*itemIter] + deltaScenePos;
							(*itemIter)->setPosition(newPositions[*itemIter]);
						}
					}

					for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
						(*itemIter)->setPosition(originalPositions[*itemIter]);

					if (!itemsToMove.isEmpty())
					{
						moveItemsCommand(itemsToMove, newPositions, false);
						viewport()->update();
					}

					sendMouseInfoText(mInitialPositions[mMouseDownItem],
						mInitialPositions[mMouseDownItem] + roundToGrid(scenePos - mMouseButtonDownScenePos));
					break;

				case MouseResizeItem:
					resizeItemCommand(mSelectedItemPoint, roundToGrid(scenePos), false, true);
					sendMouseInfoText(mInitialItemPointPosition, roundToGrid(scenePos));
					break;

				case MouseRubberBand:
					mRubberBandRect = QRect(event->pos(), mMouseButtonDownPos).normalized();
					sendMouseInfoText(mMouseButtonDownScenePos, scenePos);
					break;

				default:
					sendMouseInfoText(mMouseButtonDownScenePos, scenePos);
					break;
				}
			}
			else sendMouseInfoText(scenePos);
		}
	}

	if (event->buttons() != Qt::NoButton || (mode() == DefaultMode && !mPlaceItems.isEmpty())) viewport()->update();
}

void DrawingWidget::mouseReleaseEvent(QMouseEvent* event)
{
	DrawingWidgetBase::mouseReleaseEvent(event);

	if (event->button() == Qt::LeftButton)
	{
		QPointF scenePos = mapToScene(event->pos());

		if (mode() == DefaultMode)
		{
			if (!mPlaceItems.isEmpty())
			{
				if (mPlaceItems.size() > 1 ||
					(mPlaceItems.size() == 1 && mPlaceItems.first()->isValid()))
				{
					QList<DrawingItem*> newItems;
					DrawingItem* newItem;
					QList<DrawingItemPoint*> points;

					addItemsCommand(mPlaceItems, true);

					for(auto itemIter = mPlaceItems.begin(); itemIter != mPlaceItems.end(); itemIter++)
					{
						newItem = (*itemIter)->copy();
						if (newItem->flags() & DrawingItem::PlaceByMousePressAndRelease)
						{
							points = newItem->points();
							for(auto pointIter = points.begin(); pointIter != points.end(); pointIter++)
								resizeItem(*pointIter, newItem->position());
						}

						newItems.append(newItem);
					}
					mPlaceItems.clear();

					setPlaceMode(newItems);
				}
			}
			else
			{
				bool controlDown = ((event->modifiers() & Qt::ControlModifier) != 0);
				QList<DrawingItem*> newSelection = (controlDown) ? mSelectedItems : QList<DrawingItem*>();
				QList<DrawingItem*> children;
				QPointF deltaScenePos;
				QList<DrawingItem*> itemsToMove;
				QList<DrawingItem*> foundItems;
				QHash<DrawingItem*,QPointF> originalPositions, newPositions;

				switch (mMouseState)
				{
				case MouseSelect:
					if (mMouseDownItem)
					{
						if (controlDown && mMouseDownItem->isSelected())
							newSelection.removeAll(mMouseDownItem);
						else if (mMouseDownItem->flags() & DrawingItem::CanSelect)
							newSelection.append(mMouseDownItem);
					}
					if (mSelectedItems != newSelection) selectItemsCommand(newSelection, true);
					break;

				case MouseMoveItems:
					for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
						originalPositions[*itemIter] = (*itemIter)->position();

					deltaScenePos = roundToGrid(scenePos - mMouseButtonDownScenePos);
					for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
					{
						if ((*itemIter)->flags() & DrawingItem::CanMove)
						{
							itemsToMove.append(*itemIter);
							newPositions[*itemIter] = mInitialPositions[*itemIter] + deltaScenePos;
							(*itemIter)->setPosition(newPositions[*itemIter]);
						}
					}

					for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
						(*itemIter)->setPosition(originalPositions[*itemIter]);

					if (!itemsToMove.isEmpty())
					{
						moveItemsCommand(itemsToMove, newPositions, true);
						viewport()->update();
					}
					break;

				case MouseResizeItem:
					resizeItemCommand(mSelectedItemPoint, roundToGrid(scenePos), true, true);
					break;

				case MouseRubberBand:
					//selectArea(mapToScene(mRubberBandRect));
					foundItems = items(mapToScene(mRubberBandRect));

					for(auto itemIter = foundItems.begin(); itemIter != foundItems.end(); itemIter++)
					{
						if (!newSelection.contains((*itemIter)) &&
							(*itemIter)->flags() & DrawingItem::CanSelect) newSelection.append(*itemIter);
					}

					if (mSelectedItems != newSelection) selectItemsCommand(newSelection, true);

					mRubberBandRect = QRect();
					break;

				default:
					break;
				}

				mMouseState = MouseReady;
				mMouseButtonDownPos = QPoint();
				mMouseButtonDownScenePos = QPointF();
				mMouseDragged = false;
				mInitialPositions.clear();
				mInitialItemPointPosition = QPointF();

				updateSelectionCenter();
			}
		}
	}
	else if (event->button() == Qt::RightButton)
	{
		if (mode() == DefaultMode && mPlaceItems.isEmpty())
		{
			if (mMouseDownItem && mMouseDownItem->isSelected() && mSelectedItems.size() == 1)
			{
				if (actions()[InsertPointAction]->isEnabled())
					mSinglePolyItemContextMenu.popup(event->globalPos() + QPoint(2, 2));
				else
					mSingleItemContextMenu.popup(event->globalPos() + QPoint(2, 2));
			}
			else if (mMouseDownItem && mMouseDownItem->isSelected())
			{
				mMultipleItemContextMenu.popup(event->globalPos() + QPoint(2, 2));
			}
			else
			{
				if (mMouseDownItem == nullptr) clearSelection();
				mDrawingContextMenu.popup(event->globalPos() + QPoint(2, 2));
			}
		}
		else setDefaultMode();

		viewport()->update();
	}

	viewport()->update();
	emit mouseInfoChanged("");
}

void DrawingWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	DrawingWidgetBase::mouseDoubleClickEvent(event);

	if (event->button() == Qt::LeftButton)
	{
		mMouseButtonDownPos = event->pos();
		mMouseButtonDownScenePos = mapToScene(mMouseButtonDownPos);
		mMouseDragged = false;

		if (mode() == DefaultMode && mPlaceItems.isEmpty())
		{
			mMouseState = MouseSelect;

			mMouseDownItem = itemAt(mMouseButtonDownScenePos);
			if (mMouseDownItem)
			{
				mInitialPositions.clear();
				for(auto itemIter = mSelectedItems.begin(); itemIter != mSelectedItems.end(); itemIter++)
					mInitialPositions[*itemIter] = (*itemIter)->position();

				if (mMouseDownItem->isSelected() && mSelectedItems.size() == 1)
				{
					mSelectedItemPoint = pointAt(mMouseDownItem, mMouseDownItem->mapFromScene(mMouseButtonDownScenePos));
					if (mSelectedItemPoint)
					{
						if (mSelectedItemPoint->flags() & DrawingItemPoint::Control)
							mInitialItemPointPosition = mMouseDownItem->mapToScene(mSelectedItemPoint->position());
						else
							mSelectedItemPoint = nullptr;
					}
				}

				emit propertiesTriggered();
			}

			mFocusItem = mMouseDownItem;
		}
	}

	viewport()->update();
	emit mouseInfoChanged("");
}

//==================================================================================================

void DrawingWidget::keyPressEvent(QKeyEvent* event)
{
	if (mFocusItem) mFocusItem->keyPressEvent(event);
}

void DrawingWidget::keyReleaseEvent(QKeyEvent* event)
{
	if (mFocusItem) mFocusItem->keyReleaseEvent(event);
}

//==================================================================================================

void DrawingWidget::drawContents(QPainter* painter)
{
	drawItems(painter, mItems);
}

void DrawingWidget::drawForeground(QPainter* painter)
{
	drawItems(painter, mPlaceItems);
	drawItemPoints(painter, mSelectedItems);
	drawHotpoints(painter, mSelectedItems + mPlaceItems);
	drawRubberBand(painter, mRubberBandRect);
}

//==================================================================================================

void DrawingWidget::drawItems(QPainter* painter, const QList<DrawingItem*>& items)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		if ((*itemIter)->isVisible())
		{
			painter->translate((*itemIter)->position());
			painter->setTransform((*itemIter)->transformInverted(), true);

			(*itemIter)->render(painter);

			//painter->save();
			//painter->setBrush(QColor(255, 0, 255, 128));
			//painter->setPen(QPen(QColor(255, 0, 255, 128), 1));
			//painter->drawPath(itemAdjustedShape(*itemIter));
			//painter->restore();

			painter->setTransform((*itemIter)->transform(), true);
			painter->translate(-(*itemIter)->position());
		}
	}
}

void DrawingWidget::drawItemPoints(QPainter* painter, const QList<DrawingItem*>& items)
{
	QColor color = backgroundBrush().color();
	color.setRed(255 - color.red());
	color.setGreen(255 - color.green());
	color.setBlue(255 - color.blue());

	painter->save();

	painter->resetTransform();
	painter->setRenderHints(QPainter::Antialiasing, false);
	painter->setPen(QPen(color, 1));

	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		if ((*itemIter)->isVisible())
		{
			QList<DrawingItemPoint*> itemPoints = (*itemIter)->points();

			for(auto pointIter = itemPoints.begin(), pointEnd = itemPoints.end(); pointIter != pointEnd; pointIter++)
			{
				bool controlPoint = (((*pointIter)->flags() & DrawingItemPoint::Control) ||
					((*pointIter)->flags() == DrawingItemPoint::NoFlags));
				bool connectionPoint = ((*pointIter)->flags() & DrawingItemPoint::Connection);

				if (controlPoint || connectionPoint)
				{
					QRect pointRect = DrawingWidget::pointRect(*pointIter).adjusted(1, 1, -2, -2);

					if (connectionPoint && !controlPoint)
						painter->setBrush(QColor(255, 255, 0));
					else
						painter->setBrush(QColor(0, 224, 0));

					painter->drawRect(pointRect);
				}
			}
		}
	}

	painter->restore();
}

void DrawingWidget::drawHotpoints(QPainter* painter, const QList<DrawingItem*>& items)
{
	painter->save();

	painter->resetTransform();
	painter->setRenderHints(QPainter::Antialiasing, false);
	painter->setBrush(QColor(255, 128, 0, 128));
	painter->setPen(QPen(painter->brush(), 1));

	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		QList<DrawingItemPoint*> itemPoints = (*itemIter)->points();

		for(auto pointIter = itemPoints.begin(), pointEnd = itemPoints.end(); pointIter != pointEnd; pointIter++)
		{
			for(auto otherItemIter = mItems.begin(), otherItemEnd = mItems.end();
				otherItemIter != otherItemEnd; otherItemIter++)
			{
				if ((*itemIter) != (*otherItemIter))
				{
					QList<DrawingItemPoint*> otherItemPoints = (*otherItemIter)->points();

					for(auto otherItemPointIter = otherItemPoints.begin(), otherItemPointEnd = otherItemPoints.end();
						otherItemPointIter != otherItemPointEnd; otherItemPointIter++)
					{
						if (shouldConnect(*pointIter, *otherItemPointIter))
						{
							QRect pointRect = DrawingWidget::pointRect(*pointIter);
							pointRect.adjust(-pointRect.width() / 2, -pointRect.width() / 2,
								pointRect.width() / 2, pointRect.width() / 2);
							painter->drawEllipse(pointRect);
						}
					}
				}
			}
		}
	}

	painter->restore();
}

//==================================================================================================

void DrawingWidget::addItemsCommand(const QList<DrawingItem*>& items, bool place, QUndoCommand* command)
{
	DrawingAddItemsCommand* addCommand = new DrawingAddItemsCommand(this, items, command);

	addCommand->redo();
	if (place) placeItems(items, addCommand);
	addCommand->undo();

	if (!command) pushUndoCommand(addCommand);
}

void DrawingWidget::removeItemsCommand(const QList<DrawingItem*>& items, QUndoCommand* command)
{
	DrawingRemoveItemsCommand* removeCommand = new DrawingRemoveItemsCommand(this, items, command);

	removeCommand->redo();
	unplaceItems(items, removeCommand);
	removeCommand->undo();

	if (!command) pushUndoCommand(removeCommand);
}

void DrawingWidget::moveItemsCommand(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& newPos,
	bool place, QUndoCommand* command)
{
	DrawingMoveItemsCommand* moveCommand =
		new DrawingMoveItemsCommand(this, items, newPos, place, command);

	moveCommand->redo();
	tryToMaintainConnections(items, true, true, nullptr, moveCommand);
	if (place) placeItems(items, moveCommand);
	moveCommand->undo();

	if (!command) pushUndoCommand(moveCommand);
}

void DrawingWidget::resizeItemCommand(DrawingItemPoint* itemPoint, const QPointF& scenePos,
	bool place, bool disconnect, QUndoCommand* command)
{
	if (itemPoint && itemPoint->item())
	{
		DrawingResizeItemCommand* resizeCommand =
			new DrawingResizeItemCommand(this, itemPoint, scenePos, place, command);
		QList<DrawingItem*> resizeItems;
		resizeItems.append(itemPoint->item());

		resizeCommand->redo();
		if (disconnect) disconnectAll(itemPoint, resizeCommand);
		tryToMaintainConnections(resizeItems, true,
			!(itemPoint->flags() & DrawingItemPoint::Free), itemPoint, resizeCommand);
		if (place) placeItems(resizeItems, resizeCommand);
		resizeCommand->undo();

		if (!command) pushUndoCommand(resizeCommand);
	}
}

void DrawingWidget::rotateItemsCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command)
{
	DrawingRotateItemsCommand* rotateCommand =
		new DrawingRotateItemsCommand(this, items, scenePos, command);

	rotateCommand->redo();
	tryToMaintainConnections(items, true, true, nullptr, rotateCommand);
	rotateCommand->undo();

	if (!command) pushUndoCommand(rotateCommand);
}

void DrawingWidget::rotateBackItemsCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command)
{
	DrawingRotateBackItemsCommand* rotateCommand =
		new DrawingRotateBackItemsCommand(this, items, scenePos, command);

	rotateCommand->redo();
	tryToMaintainConnections(items, true, true, nullptr, rotateCommand);
	rotateCommand->undo();

	if (!command) pushUndoCommand(rotateCommand);
}

void DrawingWidget::flipItemsHorizontalCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command)
{
	DrawingFlipItemsHorizontalCommand* flipCommand =
		new DrawingFlipItemsHorizontalCommand(this, items, scenePos, command);

	flipCommand->redo();
	tryToMaintainConnections(items, true, true, nullptr, flipCommand);
	flipCommand->undo();

	if (!command) pushUndoCommand(flipCommand);
}

void DrawingWidget::flipItemsVerticalCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command)
{
	DrawingFlipItemsVerticalCommand* flipCommand =
		new DrawingFlipItemsVerticalCommand(this, items, scenePos, command);

	flipCommand->redo();
	tryToMaintainConnections(items, true, true, nullptr, flipCommand);
	flipCommand->undo();

	if (!command) pushUndoCommand(flipCommand);
}

void DrawingWidget::reorderItemsCommand(const QList<DrawingItem*>& itemsOrdered, QUndoCommand* command)
{
	DrawingReorderItemsCommand* selectCommand =
		new DrawingReorderItemsCommand(this, itemsOrdered, command);

	if (!command) pushUndoCommand(selectCommand);
}

void DrawingWidget::selectItemsCommand(const QList<DrawingItem*>& items, bool finalSelect,
	QUndoCommand* command)
{
	DrawingSelectItemsCommand* selectCommand =
		new DrawingSelectItemsCommand(this, items, finalSelect, command);

	if (!command) pushUndoCommand(selectCommand);

	//selectItems(items);
}

void DrawingWidget::connectItemPointsCommand(DrawingItemPoint* point1, DrawingItemPoint* point2,
	QUndoCommand* command)
{
	DrawingItemPointConnectCommand* connectCommand =
		new DrawingItemPointConnectCommand(this, point1, point2, command);

	QPointF point0Pos = point1->item()->mapToScene(point1->position());
	QPointF point1Pos = point2->item()->mapToScene(point2->position());

	if (point0Pos != point1Pos)
	{
		if ((point2->flags() & DrawingItemPoint::Control) && (point2->item()->flags() & DrawingItem::CanResize))
			resizeItemCommand(point2, point0Pos, false, true, connectCommand);
		else if ((point1->flags() & DrawingItemPoint::Control) && (point1->item()->flags() & DrawingItem::CanResize))
			resizeItemCommand(point1, point1Pos, false, true, connectCommand);
	}

	if (!command) pushUndoCommand(connectCommand);
}

void DrawingWidget::disconnectItemPointsCommand(DrawingItemPoint* point1, DrawingItemPoint* point2,
	QUndoCommand* command)
{
	DrawingItemPointDisconnectCommand* disconnectCommand =
		new DrawingItemPointDisconnectCommand(this, point1, point2, command);

	if (!command) pushUndoCommand(disconnectCommand);
}

//==================================================================================================

void DrawingWidget::placeItems(const QList<DrawingItem*>& items, QUndoCommand* command)
{
	QList<DrawingItemPoint*> itemPoints, otherItemPoints;

	for(auto itemIter = items.begin(); itemIter != items.end(); itemIter++)
	{
		for(auto otherItemIter = mItems.begin(); otherItemIter != mItems.end(); otherItemIter++)
		{
			if (!items.contains(*otherItemIter) && !mPlaceItems.contains(*otherItemIter))
			{
				itemPoints = (*itemIter)->points();
				otherItemPoints = (*otherItemIter)->points();

				for(auto itemPointIter = itemPoints.begin(); itemPointIter != itemPoints.end(); itemPointIter++)
				{
					for(auto otherItemPointIter = otherItemPoints.begin(); otherItemPointIter != otherItemPoints.end(); otherItemPointIter++)
					{
						if (shouldConnect(*itemPointIter, *otherItemPointIter))
							connectItemPointsCommand(*itemPointIter, *otherItemPointIter, command);
					}
				}
			}
		}
	}
}

void DrawingWidget::unplaceItems(const QList<DrawingItem*>& items, QUndoCommand* command)
{
	DrawingItem* item;
	DrawingItemPoint* itemPoint;
	QList<DrawingItemPoint*> itemPoints, targetPoints;

	for(auto itemIter = items.begin(); itemIter != items.end(); itemIter++)
	{
		item = *itemIter;
		itemPoints = item->points();

		for(auto itemPointIter = itemPoints.begin(); itemPointIter != itemPoints.end(); itemPointIter++)
		{
			itemPoint = *itemPointIter;
			targetPoints = itemPoint->connections();
			for(auto targetPointIter = targetPoints.begin(); targetPointIter != targetPoints.end(); targetPointIter++)
			{
				if (!items.contains((*targetPointIter)->item()))
					disconnectItemPointsCommand(itemPoint, *targetPointIter, command);
			}
		}
	}
}

void DrawingWidget::tryToMaintainConnections(const QList<DrawingItem*>& items, bool allowResize,
	bool checkControlPoints, DrawingItemPoint* pointToSkip, QUndoCommand* command)
{
	QList<DrawingItemPoint*> itemPoints, targetPoints;
	DrawingItem* item;
	DrawingItem* targetItem;
	DrawingItemPoint* itemPoint;
	DrawingItemPoint* targetItemPoint;

	for(auto itemIter = items.begin(); itemIter != items.end(); itemIter++)
	{
		item = *itemIter;
		itemPoints = item->points();

		for(auto itemPointIter = itemPoints.begin(); itemPointIter != itemPoints.end(); itemPointIter++)
		{
			itemPoint = *itemPointIter;
			if (itemPoint != pointToSkip && (checkControlPoints || !(itemPoint->flags() & DrawingItemPoint::Control)))
			{
				targetPoints = itemPoint->connections();
				for(auto targetPointIter = targetPoints.begin(); targetPointIter != targetPoints.end(); targetPointIter++)
				{
					targetItemPoint = *targetPointIter;
					targetItem = targetItemPoint->item();

					if (item->mapToScene(itemPoint->position()) != targetItem->mapToScene(targetItemPoint->position()))
					{
						// Try to maintain the connection by resizing targetPoint if possible
						if (allowResize && (targetItem->flags() & DrawingItem::CanResize) &&
							(targetItemPoint->flags() & DrawingItemPoint::Free) &&
							!shouldDisconnect(itemPoint, targetItemPoint))
						{
							resizeItemCommand(targetItemPoint, item->mapToScene(itemPoint->position()),
								false, false, command);
						}
						else
							disconnectItemPointsCommand(itemPoint, targetItemPoint, command);
					}
				}
			}
		}
	}
}

void DrawingWidget::disconnectAll(DrawingItemPoint* itemPoint, QUndoCommand* command)
{
	QList<DrawingItemPoint*> targetPoints;

	if (itemPoint)
	{
		targetPoints = itemPoint->connections();
		for(auto targetPointIter = targetPoints.begin(); targetPointIter != targetPoints.end(); targetPointIter++)
			disconnectItemPointsCommand(itemPoint, *targetPointIter, command);
	}
}

//==================================================================================================

void DrawingWidget::addItems(const QList<DrawingItem*>& items)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
		addItem(*itemIter);

	emit numberOfItemsChanged(mItems.size());
}

void DrawingWidget::insertItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,int>& indices)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
		insertItem(indices[*itemIter], *itemIter);

	emit numberOfItemsChanged(mItems.size());
}

void DrawingWidget::removeItems(const QList<DrawingItem*>& items)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
		removeItem(*itemIter);

	emit numberOfItemsChanged(mItems.size());
}

void DrawingWidget::moveItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		(*itemIter)->setPosition(scenePos[*itemIter]);
		(*itemIter)->moveEvent((*itemIter)->position());
	}

	emit itemsPositionChanged(items);
}

void DrawingWidget::resizeItem(DrawingItemPoint* point, const QPointF& scenePos)
{
	if (point && point->item())
	{
		DrawingItem* item = point->item();

		point->setPosition(item->mapFromScene(scenePos));

		if (item->flags() & DrawingItem::AdjustPositionOnResize)
		{
			QList<DrawingItemPoint*> points = item->points();

			if (points.size() > 0)
			{
				// Adjust position of item and item points so that point(0)->position() == QPointF(0, 0)
				QPointF deltaPos = -points.first()->position();
				QPointF pointParentPos = item->mapToScene(points.first()->position());

				for(auto pointIter = points.begin(), pointEnd = points.end(); pointIter != pointEnd; pointIter++)
					(*pointIter)->setPosition((*pointIter)->position() + deltaPos);

				item->setPosition(pointParentPos);
			}
		}

		item->resizeEvent(point, scenePos);

		QList<DrawingItem*> items;
		items.append(item);
		emit itemsGeometryChanged(items);
	}
}

void DrawingWidget::rotateItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		QPointF pos = scenePos[*itemIter];

		// Calculate new position of reference point
		QPointF difference((*itemIter)->position() - pos);
		(*itemIter)->setPosition(QPointF(pos.x() + difference.y(), pos.y() - difference.x()));

		// Update orientation
		QTransform transform = (*itemIter)->transform();
		transform.rotate(90);
		(*itemIter)->setTransform(transform);

		(*itemIter)->rotateEvent(pos);
	}

	emit itemsTransformChanged(items);
}

void DrawingWidget::rotateBackItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		QPointF pos = scenePos[*itemIter];

		// Calculate new position of reference point
		QPointF difference((*itemIter)->position() - pos);
		(*itemIter)->setPosition(QPointF(pos.x() - difference.y(), pos.y() + difference.x()));

		// Update orientation
		QTransform transform = (*itemIter)->transform();
		transform.rotate(-90);
		(*itemIter)->setTransform(transform);

		(*itemIter)->rotateBackEvent(pos);
	}

	emit itemsTransformChanged(items);
}

void DrawingWidget::flipItemsHorizontal(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		QPointF pos = scenePos[*itemIter];

		// Calculate new position of reference point
		(*itemIter)->setX(2 * pos.x() - (*itemIter)->x());

		// Update orientation
		QTransform transform = (*itemIter)->transform();
		transform.scale(-1, 1);
		(*itemIter)->setTransform(transform);

		(*itemIter)->flipHorizontalEvent(pos);
	}

	emit itemsTransformChanged(items);
}

void DrawingWidget::flipItemsVertical(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos)
{
	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		QPointF pos = scenePos[*itemIter];

		// Calculate new position of reference point
		(*itemIter)->setY(2 * pos.y() - (*itemIter)->y());

		// Update orientation
		QTransform transform = (*itemIter)->transform();
		transform.scale(1, -1);
		(*itemIter)->setTransform(transform);

		(*itemIter)->flipVerticalEvent(pos);
	}

	emit itemsTransformChanged(items);
}

void DrawingWidget::insertItemPoint(DrawingItem* item, DrawingItemPoint* point, int index)
{
	if (item && point)
	{
		item->insertPoint(index, point);
		item->resizeEvent(point, item->mapToScene(point->position()));

		QList<DrawingItem*> items;
		items.append(item);
		emit itemsGeometryChanged(items);
	}
}

void DrawingWidget::removeItemPoint(DrawingItem* item, DrawingItemPoint* point)
{
	if (item && point)
	{
		item->removePoint(point);
		item->resizeEvent(point, item->mapToScene(point->position()));

		QList<DrawingItem*> items;
		items.append(item);
		emit itemsGeometryChanged(items);
	}
}

void DrawingWidget::connectItemPoints(DrawingItemPoint* point1, DrawingItemPoint* point2)
{
	if (point1 && point2)
	{
		point1->addConnection(point2);
		point2->addConnection(point1);
	}
}

void DrawingWidget::disconnectItemPoints(DrawingItemPoint* point1, DrawingItemPoint* point2)
{
	if (point1 && point2)
	{
		point1->removeConnection(point2);
		point2->removeConnection(point1);
	}
}

void DrawingWidget::reorderItems(const QList<DrawingItem*>& items)
{
	for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
	{
		(*itemIter)->mWidget = nullptr;
		if (!items.contains(*itemIter)) delete *itemIter;
	}

	mItems = items;

	for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
		(*itemIter)->mWidget = this;
}

void DrawingWidget::setItemPropertiesPrivate(const QHash< DrawingItem*, QHash<QString,QVariant> >& properties)
{
	for(auto propIter = properties.begin(), propEnd = properties.end(); propIter != propEnd; propIter++)
		propIter.key()->setProperties(propIter.value());

	emit itemsPropertiesChanged(properties.keys());
}

void DrawingWidget::setDrawingPropertiesPrivate(const QHash<QString,QVariant>& properties)
{
	setProperties(properties);
	emit drawingPropertiesChanged(properties);
	if (properties.contains("scene-rect")) zoomFit();
}

//==================================================================================================

bool DrawingWidget::itemMatchesPoint(DrawingItem* item, const QPointF& pos) const
{
	bool match = false;

	if (item)
	{
		// Check item shape
		match = itemAdjustedShape(item).contains(item->mapFromScene(pos));

		// Check item points
		if (!match && item->isSelected())
		{
			QList<DrawingItemPoint*> itemPoints = item->points();
			QRectF pointSceneRect;

			for(auto pointIter = itemPoints.begin(), pointEnd = itemPoints.end();
				!match && pointIter != pointEnd; pointIter++)
			{
				pointSceneRect = mapToScene(pointRect(*pointIter));
				match = pointSceneRect.contains(pos);
			}
		}
	}

	return match;
}

bool DrawingWidget::itemMatchesRect(DrawingItem* item, const QRectF& rect, Qt::ItemSelectionMode mode) const
{
	bool match = false;

	if (item)
	{
		// Check item boundingRect or shape
		switch (mode)
		{
		case Qt::IntersectsItemShape:
			match = item->shape().intersects(item->mapFromScene(rect).boundingRect());
			break;
		case Qt::ContainsItemShape:
			match = rect.contains(item->mapToScene(item->shape().boundingRect()).boundingRect());
			break;
		case Qt::IntersectsItemBoundingRect:
			match = rect.intersects(item->mapToScene(item->boundingRect()).boundingRect());
			break;
		default:	// Qt::ContainsItemBoundingRect
			match = rect.contains(item->mapToScene(item->boundingRect()).boundingRect());
			break;
		}

		// Check item points
		if (!match && item->isSelected())
		{
			QList<DrawingItemPoint*> itemPoints = item->points();
			QRectF pointSceneRect;

			for(auto pointIter = itemPoints.begin(), pointEnd = itemPoints.end();
				!match && pointIter != pointEnd; pointIter++)
			{
				pointSceneRect = mapToScene(pointRect(*pointIter));

				if (mode == Qt::IntersectsItemBoundingRect || mode == Qt::IntersectsItemShape)
					match = rect.intersects(pointSceneRect);
				else
					match = rect.contains(pointSceneRect);
			}
		}
	}

	return match;
}

bool DrawingWidget::itemMatchesPath(DrawingItem* item, const QPainterPath& path, Qt::ItemSelectionMode mode) const
{
	bool match = false;

	if (item && item->isVisible())
	{
		// Check item boundingRect or shape
		switch (mode)
		{
		case Qt::IntersectsItemShape:
			match = item->shape().intersects(item->mapFromScene(path));
			break;
		case Qt::ContainsItemShape:
			match = path.contains(item->mapToScene(item->shape().boundingRect()).boundingRect());
			break;
		case Qt::IntersectsItemBoundingRect:
			match = path.intersects(item->mapToScene(item->boundingRect()).boundingRect());
			break;
		default:	// Qt::ContainsItemBoundingRect
			match = path.contains(item->mapToScene(item->boundingRect()).boundingRect());
			break;
		}

		// Check item points
		if (!match && item->isSelected())
		{
			QList<DrawingItemPoint*> itemPoints = item->points();
			QRectF pointSceneRect;

			for(auto pointIter = itemPoints.begin(), pointEnd = itemPoints.end();
				!match && pointIter != pointEnd; pointIter++)
			{
				pointSceneRect = mapToScene(pointRect(*pointIter));

				if (mode == Qt::IntersectsItemBoundingRect || mode == Qt::IntersectsItemShape)
					match = path.intersects(pointSceneRect);
				else
					match = path.contains(pointSceneRect);
			}
		}
	}

	return match;
}

QPainterPath DrawingWidget::itemAdjustedShape(DrawingItem* item) const
{
	QPainterPath adjustedShape;

	if (item)
	{
		QHash<QString,QVariant> properties = item->properties();

		if (properties.contains("pen-width"))
		{
			bool ok = false;
			qreal penWidth = properties.value("pen-width").toDouble(&ok);

			if (ok)
			{
				// Determine minimum pen width
				qreal minimumPenWidth = DrawingWidget::minimumPenWidth(item);

				if (0 < penWidth && penWidth < minimumPenWidth)
				{
					properties["pen-width"] = QVariant(minimumPenWidth);
					item->setProperties(properties);

					adjustedShape = item->shape();

					properties["pen-width"] = QVariant(penWidth);
					item->setProperties(properties);
				}
				else adjustedShape = item->shape();
			}
			else adjustedShape = item->shape();
		}
		else adjustedShape = item->shape();
	}

	return adjustedShape;
}

qreal DrawingWidget::minimumPenWidth(DrawingItem* item) const
{
	const int penWidthHint = 8;

	qreal minimumPenWidth = 0;

	if (item)
	{
		QPointF mappedPenSize = item->mapFromScene(item->position() +
			mapToScene(QPoint(penWidthHint, penWidthHint)) - mapToScene(QPoint(0, 0)));

		minimumPenWidth = qMax(qAbs(mappedPenSize.x()), qAbs(mappedPenSize.y()));
	}

	return minimumPenWidth;
}

//==================================================================================================

bool DrawingWidget::shouldConnect(DrawingItemPoint* point1, DrawingItemPoint* point2) const
{
	bool shouldConnect = false;

	if (point1 && point1->item() && point2 && point2->item() && point1->item() != point2->item())
	{
		qreal threshold = grid() / 4000;
		QPointF vec = point1->item()->mapToScene(point1->position()) - point2->item()->mapToScene(point2->position());
		qreal distance = qSqrt(vec.x() * vec.x() + vec.y() * vec.y());

		shouldConnect = ((point1->flags() & DrawingItemPoint::Connection) && (point2->flags() & DrawingItemPoint::Connection) &&
			((point1->flags() & DrawingItemPoint::Free) || (point2->flags() & DrawingItemPoint::Free)) &&
			!point1->isConnected(point2) && !point1->isConnected(point2->item()) && distance <= threshold);
	}

	return shouldConnect;
}

bool DrawingWidget::shouldDisconnect(DrawingItemPoint* point1, DrawingItemPoint* point2) const
{
	bool shouldDisconnect = true;

	if (point1 && point1->item() && point2 && point2->item())
	{
		shouldDisconnect = (
			point1->item()->mapToScene(point1->position()) != point2->item()->mapToScene(point2->position()) &&
			!(point2->flags() & DrawingItemPoint::Control));
	}

	return shouldDisconnect;
}

DrawingItemPoint* DrawingWidget::pointAt(DrawingItem* item, const QPointF& itemPos) const
{
	DrawingItemPoint* itemPoint = nullptr;

	if (item)
	{
		QList<DrawingItemPoint*> itemPoints = item->points();
		QRectF pointItemRect;

		for(auto pointIter = itemPoints.begin(); itemPoint == nullptr && pointIter != itemPoints.end(); pointIter++)
		{
			pointItemRect = item->mapFromScene(mapToScene(pointRect(*pointIter))).boundingRect();
			if (pointItemRect.contains(itemPos)) itemPoint = *pointIter;
		}
	}

	return itemPoint;
}

QRect DrawingWidget::pointRect(DrawingItemPoint* point) const
{
	const QSize pointSizeHint(8 * devicePixelRatio(), 8 * devicePixelRatio());

	QRect viewRect;

	if (point && point->item())
	{
		QPoint centerPoint = mapFromScene(point->item()->mapToScene(point->position()));

		int hDelta = pointSizeHint.width() / 2 * devicePixelRatio();
		int vDelta = pointSizeHint.height() / 2 * devicePixelRatio();
		QPoint deltaPoint(hDelta, vDelta);

		viewRect = QRect(centerPoint - deltaPoint, centerPoint + deltaPoint);
	}

	return viewRect;
}

//==================================================================================================

void DrawingWidget::sendMouseInfoText(const QPointF& pos)
{
	emit mouseInfoChanged("(" + QString::number(pos.x()) + "," + QString::number(pos.y()) + ")");
}

void DrawingWidget::sendMouseInfoText(const QPointF& p1, const QPointF& p2)
{
	QString mouseInfoText;
	QPointF delta = p2 - p1;

	mouseInfoText += "(" + QString::number(p1.x()) + "," + QString::number(p1.y()) + ")";
	mouseInfoText += " - (" + QString::number(p2.x()) + "," + QString::number(p2.y()) + ")";
	mouseInfoText += "  " + QString(QChar(0x394)) + " = (" +
		QString::number(delta.x()) + "," + QString::number(delta.y()) + ")";

	emit mouseInfoChanged(mouseInfoText);
}

//==================================================================================================

void DrawingWidget::addActions()
{
	addAction("Undo", this, SLOT(undo()), ":/icons/oxygen/edit-undo.png", "Ctrl+Z");
	addAction("Redo", this, SLOT(redo()), ":/icons/oxygen/edit-redo.png", "Ctrl+Shift+Z");

	addAction("Cut", this, SLOT(cut()), ":/icons/oxygen/edit-cut.png", "Ctrl+X");
	addAction("Copy", this, SLOT(copy()), ":/icons/oxygen/edit-copy.png", "Ctrl+C");
	addAction("Paste", this, SLOT(paste()), ":/icons/oxygen/edit-paste.png", "Ctrl+V");
	addAction("Delete", this, SLOT(deleteSelection()), ":/icons/oxygen/edit-delete.png", "Delete");

	addAction("Select All", this, SLOT(selectAll()), ":/icons/oxygen/edit-select-all.png", "Ctrl+A");
	addAction("Select None", this, SLOT(selectNone()), "", "Ctrl+Shift+A");

	addAction("Rotate", this, SLOT(rotateSelection()), ":/icons/oxygen/object-rotate-right.png", "R");
	addAction("Rotate Back", this, SLOT(rotateBackSelection()), ":/icons/oxygen/object-rotate-left.png", "Shift+R");
	addAction("Flip", this, SLOT(flipSelectionHorizontal()), ":/icons/oxygen/object-flip-horizontal.png", "F");

	addAction("Bring Forward", this, SLOT(bringForward()), ":/icons/oxygen/object-bring-forward.png");
	addAction("Send Backward", this, SLOT(sendBackward()), ":/icons/oxygen/object-send-backward.png");
	addAction("Bring to Front", this, SLOT(bringToFront()), ":/icons/oxygen/object-bring-to-front.png");
	addAction("Send to Back", this, SLOT(sendToBack()), ":/icons/oxygen/object-send-to-back.png");

	addAction("Insert Point", this, SLOT(insertItemPoint()), "");
	addAction("Remove Point", this, SLOT(removeItemPoint()), "");

	addAction("Group", this, SLOT(group()), ":/icons/oxygen/merge.png", "Ctrl+G");
	addAction("Ungroup", this, SLOT(ungroup()), ":/icons/oxygen/split.png", "Ctrl+Shift+G");

	addAction("Zoom In", this, SLOT(zoomIn()), ":/icons/oxygen/zoom-in.png", ".");
	addAction("Zoom Out", this, SLOT(zoomOut()), ":/icons/oxygen/zoom-out.png", ",");
	addAction("Zoom Fit", this, SLOT(zoomFit()), ":/icons/oxygen/zoom-fit-best.png", "/");

	addAction("Properties...", this, SIGNAL(propertiesTriggered()), ":/icons/oxygen/games-config-board.png");
}

void DrawingWidget::createContextMenus()
{
	QList<QAction*> actions = DrawingWidget::actions();

	mSingleItemContextMenu.addAction(actions[CutAction]);
	mSingleItemContextMenu.addAction(actions[CopyAction]);
	mSingleItemContextMenu.addAction(actions[PasteAction]);
	mSingleItemContextMenu.addSeparator();
	mSingleItemContextMenu.addAction(actions[RotateAction]);
	mSingleItemContextMenu.addAction(actions[RotateBackAction]);
	mSingleItemContextMenu.addAction(actions[FlipAction]);
	mSingleItemContextMenu.addAction(actions[DeleteAction]);
	mSingleItemContextMenu.addSeparator();
	mSingleItemContextMenu.addAction(actions[BringForwardAction]);
	mSingleItemContextMenu.addAction(actions[SendBackwardAction]);
	mSingleItemContextMenu.addAction(actions[BringToFrontAction]);
	mSingleItemContextMenu.addAction(actions[SendToBackAction]);
	mSingleItemContextMenu.addSeparator();
	mSingleItemContextMenu.addAction(actions[GroupAction]);
	mSingleItemContextMenu.addAction(actions[UngroupAction]);

	mSinglePolyItemContextMenu.addAction(actions[CutAction]);
	mSinglePolyItemContextMenu.addAction(actions[CopyAction]);
	mSinglePolyItemContextMenu.addAction(actions[PasteAction]);
	mSinglePolyItemContextMenu.addSeparator();
	mSinglePolyItemContextMenu.addAction(actions[InsertPointAction]);
	mSinglePolyItemContextMenu.addAction(actions[RemovePointAction]);
	mSinglePolyItemContextMenu.addSeparator();
	mSinglePolyItemContextMenu.addAction(actions[RotateAction]);
	mSinglePolyItemContextMenu.addAction(actions[RotateBackAction]);
	mSinglePolyItemContextMenu.addAction(actions[FlipAction]);
	mSinglePolyItemContextMenu.addAction(actions[DeleteAction]);
	mSinglePolyItemContextMenu.addSeparator();
	mSinglePolyItemContextMenu.addAction(actions[BringForwardAction]);
	mSinglePolyItemContextMenu.addAction(actions[SendBackwardAction]);
	mSinglePolyItemContextMenu.addAction(actions[BringToFrontAction]);
	mSinglePolyItemContextMenu.addAction(actions[SendToBackAction]);
	mSinglePolyItemContextMenu.addSeparator();
	mSinglePolyItemContextMenu.addAction(actions[GroupAction]);
	mSinglePolyItemContextMenu.addAction(actions[UngroupAction]);

	mMultipleItemContextMenu.addAction(actions[CutAction]);
	mMultipleItemContextMenu.addAction(actions[CopyAction]);
	mMultipleItemContextMenu.addAction(actions[PasteAction]);
	mMultipleItemContextMenu.addSeparator();
	mMultipleItemContextMenu.addAction(actions[RotateAction]);
	mMultipleItemContextMenu.addAction(actions[RotateBackAction]);
	mMultipleItemContextMenu.addAction(actions[FlipAction]);
	mMultipleItemContextMenu.addAction(actions[DeleteAction]);
	mMultipleItemContextMenu.addSeparator();
	mMultipleItemContextMenu.addAction(actions[BringForwardAction]);
	mMultipleItemContextMenu.addAction(actions[SendBackwardAction]);
	mMultipleItemContextMenu.addAction(actions[BringToFrontAction]);
	mMultipleItemContextMenu.addAction(actions[SendToBackAction]);
	mMultipleItemContextMenu.addSeparator();
	mMultipleItemContextMenu.addAction(actions[GroupAction]);
	mMultipleItemContextMenu.addAction(actions[UngroupAction]);

	mDrawingContextMenu.addAction(actions[UndoAction]);
	mDrawingContextMenu.addAction(actions[RedoAction]);
	mDrawingContextMenu.addSeparator();
	mDrawingContextMenu.addAction(actions[CutAction]);
	mDrawingContextMenu.addAction(actions[CopyAction]);
	mDrawingContextMenu.addAction(actions[PasteAction]);
	mDrawingContextMenu.addSeparator();
	mDrawingContextMenu.addAction(actions[ZoomInAction]);
	mDrawingContextMenu.addAction(actions[ZoomOutAction]);
	mDrawingContextMenu.addAction(actions[ZoomFitAction]);
}

QAction* DrawingWidget::addAction(const QString& text, QObject* slotObj, const char* slotFunction,
	const QString& iconPath, const QString& shortcut)
{
	QAction* action = new QAction(text, this);
	if (slotObj) connect(action, SIGNAL(triggered()), slotObj, slotFunction);

	if (!iconPath.isEmpty()) action->setIcon(QIcon(iconPath));
	if (!shortcut.isEmpty()) action->setShortcut(QKeySequence(shortcut));

	DrawingWidgetBase::addAction(action);
	return action;
}
