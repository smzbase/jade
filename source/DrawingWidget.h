/* DrawingWidget.h
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

#ifndef DRAWINGWIDGET_H
#define DRAWINGWIDGET_H

#include <DrawingWidgetBase.h>
#include <QMenu>

class DrawingItem;
class DrawingItemPoint;

class DrawingWidget : public DrawingWidgetBase
{
	Q_OBJECT

	friend class DrawingAddItemsCommand;
	friend class DrawingRemoveItemsCommand;
	friend class DrawingMoveItemsCommand;
	friend class DrawingResizeItemCommand;
	friend class DrawingRotateItemsCommand;
	friend class DrawingRotateBackItemsCommand;
	friend class DrawingFlipItemsHorizontalCommand;
	friend class DrawingFlipItemsVerticalCommand;
	friend class DrawingReorderItemsCommand;
	friend class DrawingSelectItemsCommand;
	friend class DrawingItemInsertPointCommand;
	friend class DrawingItemRemovePointCommand;
	friend class DrawingItemPointConnectCommand;
	friend class DrawingItemPointDisconnectCommand;
	friend class DrawingItemSetVisibilityCommand;
	friend class DrawingItemSetPropertiesCommand;
	friend class DrawingSetPropertiesCommand;

public:
	enum ActionIndex { UndoAction, RedoAction, CutAction, CopyAction, PasteAction, DeleteAction,
		SelectAllAction, SelectNoneAction, RotateAction, RotateBackAction, FlipAction,
		BringForwardAction, SendBackwardAction, BringToFrontAction, SendToBackAction,
		InsertPointAction, RemovePointAction, GroupAction, UngroupAction,
		ZoomInAction, ZoomOutAction, ZoomFitAction, PropertiesAction, NumberOfActions };

private:
	enum MouseState { MouseReady, MouseSelect, MouseMoveItems, MouseResizeItem, MouseRubberBand };

private:
	QList<DrawingItem*> mItems;

	QList<DrawingItem*> mSelectedItems;
	DrawingItemPoint* mSelectedItemPoint;
	QPointF mSelectionCenter;

	QList<DrawingItem*> mPlaceItems;
	DrawingItem* mMouseDownItem;
	DrawingItem* mFocusItem;

	MouseState mMouseState;
	QPoint mMouseButtonDownPos;
	QPointF mMouseButtonDownScenePos;
	bool mMouseDragged;
	QHash<DrawingItem*,QPointF> mInitialPositions;
	QPointF mInitialItemPointPosition;
	QRect mRubberBandRect;

	QMenu mSingleItemContextMenu;
	QMenu mSinglePolyItemContextMenu;
	QMenu mMultipleItemContextMenu;
	QMenu mDrawingContextMenu;

public:
	DrawingWidget(QWidget* parent = nullptr);
	~DrawingWidget();

	void addItem(DrawingItem* item);
	void insertItem(int index, DrawingItem* item);
	void removeItem(DrawingItem* item);
	void clearItems();
	QList<DrawingItem*> items() const;

	QList<DrawingItem*> items(const QPointF& pos) const;
	QList<DrawingItem*> items(const QRectF& rect) const;
	QList<DrawingItem*> items(const QPainterPath& path) const;
	DrawingItem* itemAt(const QPointF& pos) const;

	void selectItem(DrawingItem* item);
	void deselectItem(DrawingItem* item);
	void clearSelection();
	QList<DrawingItem*> selectedItems() const;
	DrawingItemPoint* selectedItemPoint() const;
	QPointF selectionCenter() const;

	QList<DrawingItem*> placeItems() const;
	DrawingItem* mouseDownItem() const;
	DrawingItem* focusItem() const;

	void setProperties(const QHash<QString,QVariant>& properties);
	QHash<QString,QVariant> properties() const;

	void renderExport(QPainter* painter);

public slots:
	void setPlaceMode(const QList<DrawingItem*>& items);

	void undo();
	void redo();

	void cut();
	void copy();
	void paste();
	void deleteSelection();

	void selectItems(const QList<DrawingItem*>& items);
	void selectArea(const QRectF& rect);
	void selectArea(const QPainterPath& path);
	void selectAll();
	void selectNone();

	void moveSelection(const QPointF& deltaPos);
	void resizeSelection(DrawingItemPoint* itemPoint, const QPointF& pos);
	void rotateSelection();
	void rotateBackSelection();
	void flipSelectionHorizontal();
	void flipSelectionVertical();

	void bringForward();
	void sendBackward();
	void bringToFront();
	void sendToBack();

	void insertItemPoint();
	void removeItemPoint();

	void group();
	void ungroup();

	void setSelectionProperties(const QHash<QString,QVariant>& properties);
	void setDrawingProperties(const QHash<QString,QVariant>& properties);

signals:
	void numberOfItemsChanged(int itemCount);
	void itemsPositionChanged(const QList<DrawingItem*>& items);
	void itemsTransformChanged(const QList<DrawingItem*>& items);
	void itemsGeometryChanged(const QList<DrawingItem*>& items);
	void itemsVisibilityChanged(const QList<DrawingItem*>& items);
	void itemsPropertiesChanged(const QList<DrawingItem*>& items);
	void selectionChanged(const QList<DrawingItem*>& items);
	void placeItemsChanged(const QList<DrawingItem*>& items);
	void propertiesTriggered();
	void drawingPropertiesChanged(const QHash<QString,QVariant>& properties);
	void mouseInfoChanged(const QString& info);

private slots:
	void clearMode(DrawingWidgetBase::Mode mode);
	void updateSelectionCenter();
	void updateActionsFromSelection();

private:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

	void drawContents(QPainter* painter);
	void drawForeground(QPainter* painter);

	void drawItems(QPainter* painter, const QList<DrawingItem*>& items);
	void drawItemPoints(QPainter* painter, const QList<DrawingItem*>& items);
	void drawHotpoints(QPainter* painter, const QList<DrawingItem*>& items);

	void addItemsCommand(const QList<DrawingItem*>& items, bool place, QUndoCommand* command = nullptr);
	void removeItemsCommand(const QList<DrawingItem*>& items, QUndoCommand* command = nullptr);
	void moveItemsCommand(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& newPos,
		bool place, QUndoCommand* command = nullptr);
	void resizeItemCommand(DrawingItemPoint* itemPoint, const QPointF& scenePos,
		bool place, bool disconnect, QUndoCommand* command = nullptr);
	void rotateItemsCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command = nullptr);
	void rotateBackItemsCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command = nullptr);
	void flipItemsHorizontalCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command = nullptr);
	void flipItemsVerticalCommand(const QList<DrawingItem*>& items, const QPointF& scenePos, QUndoCommand* command = nullptr);
	void reorderItemsCommand(const QList<DrawingItem*>& itemsOrdered, QUndoCommand* command = nullptr);
	void selectItemsCommand(const QList<DrawingItem*>& items, bool finalSelect = true, QUndoCommand* command = nullptr);
	void connectItemPointsCommand(DrawingItemPoint* point1, DrawingItemPoint* point2, QUndoCommand* command = nullptr);
	void disconnectItemPointsCommand(DrawingItemPoint* point1, DrawingItemPoint* point2, QUndoCommand* command = nullptr);

	void placeItems(const QList<DrawingItem*>& items, QUndoCommand* command);
	void unplaceItems(const QList<DrawingItem*>& items, QUndoCommand* command);
	void tryToMaintainConnections(const QList<DrawingItem*>& items, bool allowResize,
		bool checkControlPoints, DrawingItemPoint* pointToSkip, QUndoCommand* command);
	void disconnectAll(DrawingItemPoint* itemPoint, QUndoCommand* command);

	void addItems(const QList<DrawingItem*>& items);
	void insertItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,int>& indices);
	void removeItems(const QList<DrawingItem*>& items);
	void moveItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos);
	void resizeItem(DrawingItemPoint* point, const QPointF& scenePos);
	void rotateItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos);
	void rotateBackItems(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos);
	void flipItemsHorizontal(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos);
	void flipItemsVertical(const QList<DrawingItem*>& items, const QHash<DrawingItem*,QPointF>& scenePos);
	void insertItemPoint(DrawingItem* item, DrawingItemPoint* point, int index);
	void removeItemPoint(DrawingItem* item, DrawingItemPoint* point);
	void connectItemPoints(DrawingItemPoint* point1, DrawingItemPoint* point2);
	void disconnectItemPoints(DrawingItemPoint* point1, DrawingItemPoint* point2);
	void reorderItems(const QList<DrawingItem*>& items);
	void setItemPropertiesPrivate(const QHash< DrawingItem*, QHash<QString,QVariant> >& properties);
	void setDrawingPropertiesPrivate(const QHash<QString,QVariant>& properties);

	bool itemMatchesPoint(DrawingItem* item, const QPointF& pos) const;
	bool itemMatchesRect(DrawingItem* item, const QRectF& rect, Qt::ItemSelectionMode mode) const;
	bool itemMatchesPath(DrawingItem* item, const QPainterPath& path, Qt::ItemSelectionMode mode) const;
	QPainterPath itemAdjustedShape(DrawingItem* item) const;
	qreal minimumPenWidth(DrawingItem* item) const;

	bool shouldConnect(DrawingItemPoint* point1, DrawingItemPoint* point2) const;
	bool shouldDisconnect(DrawingItemPoint* point1, DrawingItemPoint* point2) const;
	DrawingItemPoint* pointAt(DrawingItem* item, const QPointF& itemPos) const;
	QRect pointRect(DrawingItemPoint* point) const;

	void sendMouseInfoText(const QPointF& pos);
	void sendMouseInfoText(const QPointF& p1, const QPointF& p2);

	void addActions();
	void createContextMenus();
	QAction* addAction(const QString& text, QObject* slotObj, const char* slotFunction,
		const QString& iconPath = QString(), const QString& shortcut = QString());
};

#endif
