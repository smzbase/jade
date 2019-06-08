/* DrawingUndo.h
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

#ifndef DRAWINGUNDO_H
#define DRAWINGUNDO_H

#include <QHash>
#include <QPointF>
#include <QUndoCommand>

class DrawingWidget;
class DrawingItem;
class DrawingItemPoint;

class DrawingUndoCommand : public QUndoCommand
{
public:
	enum Type { AddItemsType, RemoveItemsType, MoveItemsType, ResizeItemType,
		RotateItemsType, RotateBackItemsType, FlipItemsHorizontalType, FlipItemsVerticalType,
		ReorderItemsType, SelectItemsType,
		InsertItemPointType, RemoveItemPointType,
		PointConnectType, PointDisconnectType,
		SetItemsPropertiesType, SetDrawingProperties, NumberOfCommands };

public:
	DrawingUndoCommand(const QString& title = QString(), QUndoCommand* parent = nullptr);
	DrawingUndoCommand(const DrawingUndoCommand& command, QUndoCommand* parent = nullptr);
	virtual ~DrawingUndoCommand();

protected:
	virtual void mergeChildren(const QUndoCommand* command);
};

//==================================================================================================

class DrawingAddItemsCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mItems;
	bool mUndone;

public:
	DrawingAddItemsCommand(DrawingWidget* widget, const QList<DrawingItem*>& items, QUndoCommand* parent = nullptr);
	~DrawingAddItemsCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingRemoveItemsCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mItems;
	bool mUndone;
	QHash<DrawingItem*,int> mItemIndex;

public:
	DrawingRemoveItemsCommand(DrawingWidget* widget, const QList<DrawingItem*>& items,
		QUndoCommand* parent = nullptr);
	~DrawingRemoveItemsCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingMoveItemsCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mItems;
	QHash<DrawingItem*,QPointF> mScenePos;
	QHash<DrawingItem*,QPointF> mOriginalScenePos;
	bool mFinalMove;

public:
	DrawingMoveItemsCommand(DrawingWidget* widget, const QList<DrawingItem*>& items,
		const QHash<DrawingItem*,QPointF>& newPos, bool finalMove, QUndoCommand* parent = nullptr);
	~DrawingMoveItemsCommand();

	int id() const;
	bool mergeWith(const QUndoCommand* command);

	void redo();
	void undo();
};

//==================================================================================================

class DrawingResizeItemCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	DrawingItemPoint* mPoint;
	QPointF mNewPos;
	QPointF mOriginalPos;
	bool mFinalResize;

public:
	DrawingResizeItemCommand(DrawingWidget* widget, DrawingItemPoint* point,
		const QPointF& scenePos, bool finalResize, QUndoCommand* parent = nullptr);
	DrawingResizeItemCommand(const DrawingResizeItemCommand& command, QUndoCommand* parent = nullptr);
	~DrawingResizeItemCommand();

	int id() const;
	bool mergeWith(const QUndoCommand* command);

	void redo();
	void undo();
};

//==================================================================================================

class DrawingRotateItemsCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mItems;
	QHash<DrawingItem*,QPointF> mParentPos;

public:
	DrawingRotateItemsCommand(DrawingWidget* widget, const QList<DrawingItem*>& items,
		const QPointF& scenePos, QUndoCommand* parent = nullptr);
	~DrawingRotateItemsCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingRotateBackItemsCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mItems;
	QHash<DrawingItem*,QPointF> mParentPos;

public:
	DrawingRotateBackItemsCommand(DrawingWidget* widget, const QList<DrawingItem*>& items,
		const QPointF& scenePos, QUndoCommand* parent = nullptr);
	~DrawingRotateBackItemsCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingFlipItemsHorizontalCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mItems;
	QHash<DrawingItem*,QPointF> mParentPos;

public:
	DrawingFlipItemsHorizontalCommand(DrawingWidget* widget, const QList<DrawingItem*>& items,
		const QPointF& scenePos, QUndoCommand* parent = nullptr);
	~DrawingFlipItemsHorizontalCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingFlipItemsVerticalCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mItems;
	QHash<DrawingItem*,QPointF> mParentPos;

public:
	DrawingFlipItemsVerticalCommand(DrawingWidget* widget, const QList<DrawingItem*>& items,
		const QPointF& scenePos, QUndoCommand* parent = nullptr);
	~DrawingFlipItemsVerticalCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingReorderItemsCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mNewItemOrder;
	QList<DrawingItem*> mOriginalItemOrder;

public:
	DrawingReorderItemsCommand(DrawingWidget* widget,
		const QList<DrawingItem*>& newItemOrder, QUndoCommand* parent = nullptr);
	~DrawingReorderItemsCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingSelectItemsCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QList<DrawingItem*> mSelectedItems;
	QList<DrawingItem*> mOriginalSelectedItems;
	bool mFinalSelect;

public:
	DrawingSelectItemsCommand(DrawingWidget* widget, const QList<DrawingItem*>& newSelectedItems,
		bool finalSelect, QUndoCommand* parent = nullptr);
	~DrawingSelectItemsCommand();

	int id() const;
	bool mergeWith(const QUndoCommand* command);

	void redo();
	void undo();
};

//==================================================================================================

class DrawingItemInsertPointCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	DrawingItem* mItem;
	DrawingItemPoint* mPoint;
	int mPointIndex;
	bool mUndone;

public:
	DrawingItemInsertPointCommand(DrawingWidget* widget, DrawingItem* item, DrawingItemPoint* point,
		int pointIndex, QUndoCommand* parent = nullptr);
	~DrawingItemInsertPointCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingItemRemovePointCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	DrawingItem* mItem;
	DrawingItemPoint* mPoint;
	int mPointIndex;
	bool mUndone;

public:
	DrawingItemRemovePointCommand(DrawingWidget* widget, DrawingItem* item, DrawingItemPoint* point,
		QUndoCommand* parent = nullptr);
	~DrawingItemRemovePointCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingItemPointConnectCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	DrawingItemPoint* mPoint1;
	DrawingItemPoint* mPoint2;

public:
	DrawingItemPointConnectCommand(DrawingWidget* widget, DrawingItemPoint* point1,
		DrawingItemPoint* point2, QUndoCommand* parent = nullptr);
	DrawingItemPointConnectCommand(const DrawingItemPointConnectCommand& command,
		QUndoCommand* parent = nullptr);
	~DrawingItemPointConnectCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingItemPointDisconnectCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	DrawingItemPoint* mPoint1;
	DrawingItemPoint* mPoint2;

public:
	DrawingItemPointDisconnectCommand(DrawingWidget* widget, DrawingItemPoint* point1,
		DrawingItemPoint* point2, QUndoCommand* parent = nullptr);
	DrawingItemPointDisconnectCommand(const DrawingItemPointDisconnectCommand& command,
		QUndoCommand* parent = nullptr);
	~DrawingItemPointDisconnectCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingItemSetPropertiesCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QHash< DrawingItem*, QHash<QString,QVariant> > mProperties;
	QHash< DrawingItem*, QHash<QString,QVariant> > mOriginalProperties;

public:
	DrawingItemSetPropertiesCommand(DrawingWidget* widget, const QList<DrawingItem*>& items,
		const QHash<QString,QVariant>& properties, QUndoCommand* parent = nullptr);
	~DrawingItemSetPropertiesCommand();

	int id() const;

	void redo();
	void undo();
};

//==================================================================================================

class DrawingSetPropertiesCommand : public DrawingUndoCommand
{
private:
	DrawingWidget* mWidget;
	QHash<QString,QVariant> mProperties;
	QHash<QString,QVariant> mOriginalProperties;

public:
	DrawingSetPropertiesCommand(DrawingWidget* widget, const QHash<QString,QVariant>& properties,
		QUndoCommand* parent = nullptr);
	~DrawingSetPropertiesCommand();

	int id() const;

	void redo();
	void undo();
};

#endif
