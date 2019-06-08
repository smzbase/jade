/* DrawingUndo.cpp
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

#include "DrawingUndo.h"
#include "DrawingWidget.h"
#include "DrawingItem.h"
#include "DrawingItemPoint.h"

DrawingUndoCommand::DrawingUndoCommand(const QString& title, QUndoCommand* parent) :
	QUndoCommand(title, parent) { }

DrawingUndoCommand::DrawingUndoCommand(const DrawingUndoCommand& command, QUndoCommand* parent) :
	QUndoCommand(command.text(), parent)
{
	QList<QUndoCommand*> otherChildren;

	for(int i = 0; i < command.childCount(); i++)
		otherChildren.append(const_cast<QUndoCommand*>(command.child(i)));

	for(auto otherChildIter = otherChildren.begin();
		otherChildIter != otherChildren.end(); otherChildIter++)
	{
		switch ((*otherChildIter)->id())
		{
		case ResizeItemType:
			new DrawingResizeItemCommand(
				*static_cast<DrawingResizeItemCommand*>(*otherChildIter), this);
			break;
		case PointConnectType:
			new DrawingItemPointConnectCommand(
				*static_cast<DrawingItemPointConnectCommand*>(*otherChildIter), this);
			break;
		case PointDisconnectType:
			new DrawingItemPointDisconnectCommand(
				*static_cast<DrawingItemPointDisconnectCommand*>(*otherChildIter), this);
			break;
		default:
			break;
		}
	}
}

DrawingUndoCommand::~DrawingUndoCommand() { }

void DrawingUndoCommand::mergeChildren(const QUndoCommand* command)
{
	bool mergeSuccess;
	QList<QUndoCommand*> children, otherChildren;

	for(int i = 0; i < childCount(); i++)
		children.append(const_cast<QUndoCommand*>(child(i)));
	for(int i = 0; i < command->childCount(); i++)
		otherChildren.append(const_cast<QUndoCommand*>(command->child(i)));

	for(auto otherChildIter = otherChildren.begin();
		otherChildIter != otherChildren.end(); otherChildIter++)
	{
		mergeSuccess = false;
		for(auto childIter = children.begin(); childIter != children.end(); childIter++)
			mergeSuccess = ((*childIter)->mergeWith(*otherChildIter) && mergeSuccess);

		if (!mergeSuccess)
		{
			switch ((*otherChildIter)->id())
			{
			case ResizeItemType:
				new DrawingResizeItemCommand(
					*static_cast<DrawingResizeItemCommand*>(*otherChildIter), this);
				break;
			case PointConnectType:
				new DrawingItemPointConnectCommand(
					*static_cast<DrawingItemPointConnectCommand*>(*otherChildIter), this);
				break;
			case PointDisconnectType:
				new DrawingItemPointDisconnectCommand(
					*static_cast<DrawingItemPointDisconnectCommand*>(*otherChildIter), this);
				break;
			default:
				break;
			}
		}
	}
}

//==================================================================================================

DrawingAddItemsCommand::DrawingAddItemsCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, QUndoCommand* parent)
	: DrawingUndoCommand("Add Items", parent)
{
	mWidget = widget;
	mItems = items;
	mUndone = true;
}

DrawingAddItemsCommand::~DrawingAddItemsCommand()
{
	if (mUndone)
	{
		while (!mItems.isEmpty()) delete mItems.takeFirst();
	}
}

int DrawingAddItemsCommand::id() const
{
	return AddItemsType;
}

void DrawingAddItemsCommand::redo()
{
	mUndone = false;
	if (mWidget) mWidget->addItems(mItems);
	DrawingUndoCommand::redo();
}

void DrawingAddItemsCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->removeItems(mItems);
	mUndone = true;
}

//==================================================================================================

DrawingRemoveItemsCommand::DrawingRemoveItemsCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, QUndoCommand* parent)
	: DrawingUndoCommand("Remove Items", parent)
{
	mWidget = widget;
	mItems = items;
	mUndone = true;

	if (mWidget)
	{
		QList<DrawingItem*> drawingItems = mWidget->items();
		for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
			mItemIndex[*itemIter] = drawingItems.indexOf(*itemIter);
	}
}

DrawingRemoveItemsCommand::~DrawingRemoveItemsCommand()
{
	if (!mUndone)
	{
		while (!mItems.isEmpty()) delete mItems.takeFirst();
	}
}

int DrawingRemoveItemsCommand::id() const
{
	return RemoveItemsType;
}

void DrawingRemoveItemsCommand::redo()
{
	mUndone = false;
	if (mWidget) mWidget->removeItems(mItems);
	DrawingUndoCommand::redo();
}

void DrawingRemoveItemsCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->insertItems(mItems, mItemIndex);
	mUndone = true;
}

//==================================================================================================

DrawingMoveItemsCommand::DrawingMoveItemsCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& newPos, bool finalMove,
	QUndoCommand* parent) : DrawingUndoCommand("Move Items", parent)
{
	mWidget = widget;
	mItems = items;
	mScenePos = newPos;
	mFinalMove = finalMove;

	for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
		mOriginalScenePos[*itemIter] = (*itemIter)->position();
}

DrawingMoveItemsCommand::~DrawingMoveItemsCommand() { }

int DrawingMoveItemsCommand::id() const
{
	return MoveItemsType;
}

bool DrawingMoveItemsCommand::mergeWith(const QUndoCommand* command)
{
	bool mergeSuccess = false;

	if (command && command->id() == MoveItemsType)
	{
		const DrawingMoveItemsCommand* moveCommand =
			static_cast<const DrawingMoveItemsCommand*>(command);

		if (moveCommand && mWidget == moveCommand->mWidget && mItems == moveCommand->mItems && !mFinalMove)
		{
			mScenePos = moveCommand->mScenePos;
			mFinalMove = moveCommand->mFinalMove;
			mergeChildren(moveCommand);
			mergeSuccess = true;
		}
	}

	return mergeSuccess;
}

void DrawingMoveItemsCommand::redo()
{
	if (mWidget) mWidget->moveItems(mItems, mScenePos);
	DrawingUndoCommand::redo();
}

void DrawingMoveItemsCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->moveItems(mItems, mOriginalScenePos);
}

//==================================================================================================

DrawingResizeItemCommand::DrawingResizeItemCommand(DrawingWidget* widget, DrawingItemPoint* point,
	const QPointF& scenePos, bool finalResize, QUndoCommand* parent)
	: DrawingUndoCommand("Resize Item", parent)
{
	mWidget = widget;
	mPoint = point;
	mFinalResize = finalResize;

	if (mPoint && mPoint->item())
	{
		mNewPos = scenePos;
		mOriginalPos = mPoint->item()->mapToScene(mPoint->position());
	}
}

DrawingResizeItemCommand::DrawingResizeItemCommand(const DrawingResizeItemCommand& command,
	QUndoCommand* parent) : DrawingUndoCommand(command, parent)
{
	mWidget = command.mWidget;
	mPoint = command.mPoint;
	mNewPos = command.mNewPos;
	mOriginalPos = command.mOriginalPos;
	mFinalResize = command.mFinalResize;
}

DrawingResizeItemCommand::~DrawingResizeItemCommand() { }

int DrawingResizeItemCommand::id() const
{
	return ResizeItemType;
}

bool DrawingResizeItemCommand::mergeWith(const QUndoCommand* command)
{
	bool mergeSuccess = false;

	if (command && command->id() == ResizeItemType)
	{
		const DrawingResizeItemCommand* resizeCommand =
			static_cast<const DrawingResizeItemCommand*>(command);

		if (resizeCommand && mWidget == resizeCommand->mWidget &&
			mPoint == resizeCommand->mPoint && !mFinalResize)
		{
			mNewPos = resizeCommand->mNewPos;
			mFinalResize = resizeCommand->mFinalResize;
			mergeChildren(resizeCommand);
			mergeSuccess = true;
		}
	}

	return mergeSuccess;
}

void DrawingResizeItemCommand::redo()
{
	if (mWidget) mWidget->resizeItem(mPoint, mNewPos);
	DrawingUndoCommand::redo();
}

void DrawingResizeItemCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->resizeItem(mPoint, mOriginalPos);
}

//==================================================================================================

DrawingRotateItemsCommand::DrawingRotateItemsCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* parent)
	: DrawingUndoCommand("Rotate Items", parent)
{
	mWidget = widget;
	mItems = items;

	for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
		mParentPos[*itemIter] = scenePos;
}

DrawingRotateItemsCommand::~DrawingRotateItemsCommand() { }

int DrawingRotateItemsCommand::id() const
{
	return RotateItemsType;
}

void DrawingRotateItemsCommand::redo()
{
	if (mWidget) mWidget->rotateItems(mItems, mParentPos);
	DrawingUndoCommand::redo();
}

void DrawingRotateItemsCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->rotateBackItems(mItems, mParentPos);
}

//==================================================================================================

DrawingRotateBackItemsCommand::DrawingRotateBackItemsCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* parent)
	: DrawingUndoCommand("Rotate Back Items", parent)
{
	mWidget = widget;
	mItems = items;

	for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
		mParentPos[*itemIter] = scenePos;
}

DrawingRotateBackItemsCommand::~DrawingRotateBackItemsCommand() { }

int DrawingRotateBackItemsCommand::id() const
{
	return RotateBackItemsType;
}

void DrawingRotateBackItemsCommand::redo()
{
	if (mWidget) mWidget->rotateBackItems(mItems, mParentPos);
	DrawingUndoCommand::redo();
}

void DrawingRotateBackItemsCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->rotateItems(mItems, mParentPos);
}

//==================================================================================================

DrawingFlipItemsHorizontalCommand::DrawingFlipItemsHorizontalCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* parent)
	: DrawingUndoCommand("Flip Items Horizontal", parent)
{
	mWidget = widget;
	mItems = items;

	for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
		mParentPos[*itemIter] = scenePos;
}

DrawingFlipItemsHorizontalCommand::~DrawingFlipItemsHorizontalCommand() { }

int DrawingFlipItemsHorizontalCommand::id() const
{
	return FlipItemsHorizontalType;
}

void DrawingFlipItemsHorizontalCommand::redo()
{
	if (mWidget) mWidget->flipItemsHorizontal(mItems, mParentPos);
	DrawingUndoCommand::redo();
}

void DrawingFlipItemsHorizontalCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->flipItemsHorizontal(mItems, mParentPos);
}

//==================================================================================================

DrawingFlipItemsVerticalCommand::DrawingFlipItemsVerticalCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* parent)
	: DrawingUndoCommand("Flip Items Vertical", parent)
{
	mWidget = widget;
	mItems = items;

	for(auto itemIter = mItems.begin(); itemIter != mItems.end(); itemIter++)
		mParentPos[*itemIter] = scenePos;
}

DrawingFlipItemsVerticalCommand::~DrawingFlipItemsVerticalCommand() { }

int DrawingFlipItemsVerticalCommand::id() const
{
	return FlipItemsVerticalType;
}

void DrawingFlipItemsVerticalCommand::redo()
{
	if (mWidget) mWidget->flipItemsVertical(mItems, mParentPos);
	DrawingUndoCommand::redo();
}

void DrawingFlipItemsVerticalCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->flipItemsVertical(mItems, mParentPos);
}

//==================================================================================================

DrawingReorderItemsCommand::DrawingReorderItemsCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& newItemOrder, QUndoCommand* parent)
	: DrawingUndoCommand("Reorder Items", parent)
{
	mWidget = widget;
	mNewItemOrder = newItemOrder;
	if (mWidget) mOriginalItemOrder = mWidget->items();
}

DrawingReorderItemsCommand::~DrawingReorderItemsCommand() { }

int DrawingReorderItemsCommand::id() const
{
	return ReorderItemsType;
}

void DrawingReorderItemsCommand::redo()
{
	if (mWidget) mWidget->reorderItems(mNewItemOrder);
	DrawingUndoCommand::redo();
}

void DrawingReorderItemsCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->reorderItems(mOriginalItemOrder);
}

//==================================================================================================

DrawingSelectItemsCommand::DrawingSelectItemsCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& newSelectedItems, bool finalSelect, QUndoCommand* parent)
	: DrawingUndoCommand("Select Items", parent)
{
	mWidget = widget;
	mSelectedItems = newSelectedItems;
	mFinalSelect = finalSelect;

	if (mWidget) mOriginalSelectedItems = mWidget->selectedItems();
}

DrawingSelectItemsCommand::~DrawingSelectItemsCommand() { }

int DrawingSelectItemsCommand::id() const
{
	return SelectItemsType;
}

bool DrawingSelectItemsCommand::mergeWith(const QUndoCommand* command)
{
	bool mergeSuccess = false;

	if (command && command->id() == SelectItemsType)
	{
		const DrawingSelectItemsCommand* selectCommand =
			static_cast<const DrawingSelectItemsCommand*>(command);

		if (selectCommand && mWidget == selectCommand->mWidget && !mFinalSelect)
		{
			for(auto itemIter = selectCommand->mSelectedItems.begin();
				itemIter != selectCommand->mSelectedItems.end(); itemIter++)
			{
				if (!mSelectedItems.contains(*itemIter)) mSelectedItems.append(*itemIter);
			}

			mFinalSelect = selectCommand->mFinalSelect;
			mergeChildren(selectCommand);
			mergeSuccess = true;
		}
	}

	return mergeSuccess;
}

void DrawingSelectItemsCommand::redo()
{
	if (mWidget) mWidget->selectItems(mSelectedItems);
	DrawingUndoCommand::redo();
}

void DrawingSelectItemsCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->selectItems(mOriginalSelectedItems);
}

//==================================================================================================

DrawingItemInsertPointCommand::DrawingItemInsertPointCommand(DrawingWidget* widget,
	DrawingItem* item, DrawingItemPoint* point, int pointIndex, QUndoCommand* parent)
	: DrawingUndoCommand("Insert Point", parent)
{
	mWidget = widget;
	mItem = item;
	mPoint = point;
	mPointIndex = pointIndex;
	mUndone = true;
}

DrawingItemInsertPointCommand::~DrawingItemInsertPointCommand()
{
	if (mUndone) delete mPoint;
}

int DrawingItemInsertPointCommand::id() const
{
	return InsertItemPointType;
}

void DrawingItemInsertPointCommand::redo()
{
	mUndone = false;
	if (mWidget) mWidget->insertItemPoint(mItem, mPoint, mPointIndex);
	DrawingUndoCommand::redo();
}

void DrawingItemInsertPointCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->removeItemPoint(mItem, mPoint);
	mUndone = true;
}

//==================================================================================================

DrawingItemRemovePointCommand::DrawingItemRemovePointCommand(DrawingWidget* widget,
	DrawingItem* item, DrawingItemPoint* point, QUndoCommand* parent)
	: DrawingUndoCommand("Remove Point", parent)
{
	mWidget = widget;
	mItem = item;
	mPoint = point;
	mUndone = true;

	mPointIndex = (mItem) ? mItem->points().indexOf(mPoint) : -1;
}

DrawingItemRemovePointCommand::~DrawingItemRemovePointCommand()
{
	if (!mUndone) delete mPoint;
}

int DrawingItemRemovePointCommand::id() const
{
	return RemoveItemPointType;
}

void DrawingItemRemovePointCommand::redo()
{
	mUndone = false;
	if (mWidget) mWidget->removeItemPoint(mItem, mPoint);
	DrawingUndoCommand::redo();
}

void DrawingItemRemovePointCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->insertItemPoint(mItem, mPoint, mPointIndex);
	mUndone = true;
}

//==================================================================================================

DrawingItemPointConnectCommand::DrawingItemPointConnectCommand(DrawingWidget* widget,
	DrawingItemPoint* point1, DrawingItemPoint* point2, QUndoCommand* parent)
	: DrawingUndoCommand("Connect Points", parent)
{
	mWidget = widget;
	mPoint1 = point1;
	mPoint2 = point2;
}

DrawingItemPointConnectCommand::DrawingItemPointConnectCommand(
	const DrawingItemPointConnectCommand& command, QUndoCommand* parent)
	: DrawingUndoCommand(command, parent)
{
	mWidget = command.mWidget;
	mPoint1 = command.mPoint1;
	mPoint2 = command.mPoint2;
}

DrawingItemPointConnectCommand::~DrawingItemPointConnectCommand() { }

int DrawingItemPointConnectCommand::id() const
{
	return PointConnectType;
}

void DrawingItemPointConnectCommand::redo()
{
	if (mWidget) mWidget->connectItemPoints(mPoint1, mPoint2);
	DrawingUndoCommand::redo();
}

void DrawingItemPointConnectCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->disconnectItemPoints(mPoint1, mPoint2);
}

//==================================================================================================

DrawingItemPointDisconnectCommand::DrawingItemPointDisconnectCommand(DrawingWidget* widget,
	DrawingItemPoint* point1, DrawingItemPoint* point2, QUndoCommand* parent)
	: DrawingUndoCommand("Disconnect Points", parent)
{
	mWidget = widget;
	mPoint1 = point1;
	mPoint2 = point2;
}

DrawingItemPointDisconnectCommand::DrawingItemPointDisconnectCommand(
	const DrawingItemPointDisconnectCommand& command, QUndoCommand* parent)
	: DrawingUndoCommand(command, parent)
{
	mWidget = command.mWidget;
	mPoint1 = command.mPoint1;
	mPoint2 = command.mPoint2;
}

DrawingItemPointDisconnectCommand::~DrawingItemPointDisconnectCommand() { }

int DrawingItemPointDisconnectCommand::id() const
{
	return PointDisconnectType;
}

void DrawingItemPointDisconnectCommand::redo()
{
	if (mWidget) mWidget->disconnectItemPoints(mPoint1, mPoint2);
	DrawingUndoCommand::redo();
}

void DrawingItemPointDisconnectCommand::undo()
{
	DrawingUndoCommand::undo();
	if (mWidget) mWidget->connectItemPoints(mPoint1, mPoint2);
}

//==================================================================================================

DrawingItemSetPropertiesCommand::DrawingItemSetPropertiesCommand(DrawingWidget* widget,
	const QList<DrawingItem*>& items, const QHash<QString,QVariant>& properties, QUndoCommand* parent) :
	DrawingUndoCommand("Set Items' Properties", parent)
{
	mWidget = widget;

	QHash<QString,QVariant> itemProperties, originalProperties;

	for(auto itemIter = items.begin(), itemEnd = items.end(); itemIter != itemEnd; itemIter++)
	{
		mProperties[*itemIter] = properties;

		itemProperties = (*itemIter)->properties();
		originalProperties.clear();
		for(auto propIter = properties.begin(), propEnd = properties.end(); propIter != propEnd; propIter++)
		{
			if (itemProperties.contains(propIter.key()))
				originalProperties[propIter.key()] = itemProperties[propIter.key()];
		}

		mOriginalProperties[*itemIter] = originalProperties;
	}
}

DrawingItemSetPropertiesCommand::~DrawingItemSetPropertiesCommand() { }

int DrawingItemSetPropertiesCommand::id() const
{
	return SetItemsPropertiesType;
}

void DrawingItemSetPropertiesCommand::redo()
{
	if (mWidget) mWidget->setItemPropertiesPrivate(mProperties);
	QUndoCommand::redo();
}

void DrawingItemSetPropertiesCommand::undo()
{
	QUndoCommand::undo();
	if (mWidget) mWidget->setItemPropertiesPrivate(mOriginalProperties);
}

//==================================================================================================

DrawingSetPropertiesCommand::DrawingSetPropertiesCommand(DrawingWidget* widget,
	const QHash<QString,QVariant>& properties, QUndoCommand* parent) : DrawingUndoCommand("Set Properties", parent)
{
	mWidget = widget;
	mProperties = properties;
	if (mWidget) mOriginalProperties = mWidget->properties();
}

DrawingSetPropertiesCommand::~DrawingSetPropertiesCommand() { }

int DrawingSetPropertiesCommand::id() const
{
	return SetDrawingProperties;
}

void DrawingSetPropertiesCommand::redo()
{
	if (mWidget) mWidget->setDrawingPropertiesPrivate(mProperties);
	QUndoCommand::redo();
}

void DrawingSetPropertiesCommand::undo()
{
	QUndoCommand::undo();
	if (mWidget) mWidget->setDrawingPropertiesPrivate(mOriginalProperties);
}
