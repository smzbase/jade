/* DynamicPropertiesWidget.h
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

#ifndef DYNAMICPROPERTIESWIDGET_H
#define DYNAMICPROPERTIESWIDGET_H

#include <QScrollArea>

class DrawingItem;
class DrawingItemPoint;

class PositionWidget;
class SizeWidget;
class SizeEdit;
class ColorWidget;
class ArrowStyleCombo;
class PenStyleCombo;
class GridStyleCombo;
class FontStyleWidget;
class TextAlignmentWidget;

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QFormLayout;
class QLineEdit;
class QStackedWidget;
class QTabWidget;
class QTextEdit;

class DynamicPropertiesWidget : public QScrollArea
{
	Q_OBJECT

protected:
	QStackedWidget* mStackedWidget;
	QList<DrawingItem*> mItems;
	DrawingItem* mItem;

	// Drawing properties
	QTabWidget* mTabWidget;

	PositionWidget* mDrawingTopLeftWidget;
	SizeWidget* mDrawingRectSizeWidget;
	ColorWidget* mDrawingBackgroundColorWidget;
	SizeEdit* mDrawingGridEdit;
	GridStyleCombo* mDrawingGridStyleCombo;
	ColorWidget* mDrawingGridColorWidget;
	QLineEdit* mDrawingGridSpacingMajorWidget;
	QLineEdit* mDrawingGridSpacingMinorWidget;

	QComboBox* mDrawingDynamicGridCombo;
	SizeEdit* mDrawingDynamicGridEdit;

	// Item default properties
	PenStyleCombo* mDefaultPenStyleCombo;
	SizeEdit* mDefaultPenWidthEdit;
	ColorWidget* mDefaultPenColorWidget;
	ColorWidget* mDefaultBrushColorWidget;

	QFontComboBox* mDefaultFontComboBox;
	SizeEdit* mDefaultFontSizeEdit;
	FontStyleWidget* mDefaultFontStyleWidget;
	TextAlignmentWidget* mDefaultTextAlignmentWidget;
	ColorWidget* mDefaultTextColorWidget;

	ArrowStyleCombo* mDefaultStartArrowCombo;
	SizeEdit* mDefaultStartArrowSizeEdit;
	ArrowStyleCombo* mDefaultEndArrowCombo;
	SizeEdit* mDefaultEndArrowSizeEdit;

	// Item properties
	PositionWidget* mPositionWidget;

	PositionWidget* mStartPositionWidget;
	PositionWidget* mEndPositionWidget;

	PositionWidget* mCurveStartPositionWidget;
	PositionWidget* mCurveStartControlPositionWidget;
	PositionWidget* mCurveEndPositionWidget;
	PositionWidget* mCurveEndControlPositionWidget;

	PositionWidget* mRectTopLeftWidget;
	PositionWidget* mRectBottomRightWidget;
	SizeEdit* mCornerRadiusEdit;

	QStackedWidget* mPointPositionStackedWidget;
	QList<PositionWidget*> mPointPositionWidgets;

	PenStyleCombo* mPenStyleCombo;
	SizeEdit* mPenWidthEdit;
	ColorWidget* mPenColorWidget;
	ColorWidget* mBrushColorWidget;

	QFontComboBox* mFontComboBox;
	SizeEdit* mFontSizeEdit;
	FontStyleWidget* mFontStyleWidget;
	TextAlignmentWidget* mTextAlignmentWidget;
	ColorWidget* mTextColorWidget;
	QTextEdit* mCaptionEdit;

	ArrowStyleCombo* mStartArrowCombo;
	SizeEdit* mStartArrowSizeEdit;
	ArrowStyleCombo* mEndArrowCombo;
	SizeEdit* mEndArrowSizeEdit;

	QHash<QWidget*,QCheckBox*> mItemStyleLabels;

public:
	DynamicPropertiesWidget();
	virtual ~DynamicPropertiesWidget();

	QSize sizeHint() const;
	int labelWidth() const;

public slots:
	void setSelectedItems(const QList<DrawingItem*>& selectedItems);
	void setNewItems(const QList<DrawingItem*>& newItems);
	void clear();

	void setItemGeometry(const QList<DrawingItem*>& item);
	void setItemsProperties(const QList<DrawingItem*>& items);

	void setDefaultItemsProperties(const QHash<QString,QVariant>& properties);
	void setDrawingProperties(const QHash<QString,QVariant>& properties);
	void setDrawingGrid(qreal grid);

signals:
	void selectedItemMoved(const QPointF& scenePos);
	void selectedItemResized(DrawingItemPoint* itemPoint, const QPointF& scenePos);
	void selectedItemsPropertiesChanged(const QHash<QString,QVariant>& properties);
	void defaultItemsPropertiesChanged(const QHash<QString,QVariant>& properties);
	void drawingPropertiesChanged(const QHash<QString,QVariant>& properties);

private slots:
	void handleItemGeometryChange();
	void handleItemsPropertiesChange();
	void handleItemDefaultsChange();
	void handleDrawingChange();

private:
	QWidget* createDrawingWidget();
	QWidget* createItemDefaultsWidget();

	void createGeometryWidgets();
	void createPropertiesWidgets();
	QWidget* createItemsWidget();
	void addWidget(QFormLayout*& formLayout, const QString& label, QWidget* widget);

	void fillPointsWidgets();
};

#endif
