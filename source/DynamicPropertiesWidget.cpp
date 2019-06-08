/* DynamicPropertiesWidget.cpp
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

#include "DynamicPropertiesWidget.h"
#include "HelperWidgets.h"

#include "DrawingCurveItem.h"
#include "DrawingEllipseItem.h"
#include "DrawingItemGroup.h"
#include "DrawingItemPoint.h"
#include "DrawingLineItem.h"
#include "DrawingPathItem.h"
#include "DrawingPolygonItem.h"
#include "DrawingPolylineItem.h"
#include "DrawingRectItem.h"
#include "DrawingTextItem.h"
#include "DrawingTextRectItem.h"
#include "DrawingTextEllipseItem.h"

#include <QCheckBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTextEdit>

DynamicPropertiesWidget::DynamicPropertiesWidget() : QScrollArea()
{
	mStackedWidget = new QStackedWidget();
	clear();

	mTabWidget = new QTabWidget();
	mTabWidget->addTab(createDrawingWidget(), "Diagram");
	mTabWidget->addTab(createItemDefaultsWidget(), "Item Defaults");
	mTabWidget->setTabPosition(QTabWidget::South);
	mStackedWidget->addWidget(mTabWidget);

	setWidget(mStackedWidget);
	setWidgetResizable(true);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	setSelectedItems(QList<DrawingItem*>());
}

DynamicPropertiesWidget::~DynamicPropertiesWidget() { }

//==================================================================================================

QSize DynamicPropertiesWidget::sizeHint() const
{
	return QSize(350, -1);
}

int DynamicPropertiesWidget::labelWidth() const
{
	return QFontMetrics(font()).width("Background Color:") + 8;
}

//==================================================================================================

void DynamicPropertiesWidget::setSelectedItems(const QList<DrawingItem*>& selectedItems)
{
	clear();

	if (selectedItems.size() > 1)
	{
		mItems = selectedItems;

		createPropertiesWidgets();

		mStackedWidget->addWidget(createItemsWidget());
		mStackedWidget->setCurrentIndex(1);
	}
	else if (selectedItems.size() == 1)
	{
		mItems = selectedItems;
		mItem = selectedItems.first();

		createGeometryWidgets();
		createPropertiesWidgets();

		mStackedWidget->addWidget(createItemsWidget());
		mStackedWidget->setCurrentIndex(1);
	}
}

void DynamicPropertiesWidget::setNewItems(const QList<DrawingItem*>& newItems)
{
	setSelectedItems(newItems);
}

void DynamicPropertiesWidget::clear()
{
	QWidget* widget;

	while (mStackedWidget->count() > 1)
	{
		widget = mStackedWidget->widget(1);
		mStackedWidget->removeWidget(widget);
		delete widget;
	}
	mStackedWidget->setCurrentIndex(0);

	mItems.clear();
	mItem = nullptr;

	mPositionWidget = nullptr;
	mStartPositionWidget = nullptr;
	mEndPositionWidget = nullptr;
	mCurveStartPositionWidget = nullptr;
	mCurveStartControlPositionWidget = nullptr;
	mCurveEndPositionWidget = nullptr;
	mCurveEndControlPositionWidget = nullptr;
	mRectTopLeftWidget = nullptr;
	mRectBottomRightWidget = nullptr;
	mCornerRadiusEdit = nullptr;
	mPointPositionStackedWidget = nullptr;
	mPointPositionWidgets.clear();
	mPenStyleCombo = nullptr;
	mPenWidthEdit = nullptr;
	mPenColorWidget = nullptr;
	mBrushColorWidget = nullptr;
	mFontComboBox = nullptr;
	mFontSizeEdit = nullptr;
	mFontStyleWidget = nullptr;
	mTextAlignmentWidget = nullptr;
	mTextColorWidget = nullptr;
	mCaptionEdit = nullptr;
	mStartArrowCombo = nullptr;
	mStartArrowSizeEdit = nullptr;
	mEndArrowCombo = nullptr;
	mEndArrowSizeEdit = nullptr;
	mItemStyleLabels.clear();
}

//==================================================================================================

void DynamicPropertiesWidget::setItemGeometry(const QList<DrawingItem*>& items)
{
	DrawingItem* item = (!items.isEmpty()) ? items.first() : nullptr;

	if (item)
	{
		QList<DrawingItemPoint*> itemPoints = item->points();

		if (mPositionWidget) mPositionWidget->setPosition(item->position());

		if (mStartPositionWidget && itemPoints.size() >= 2)
			mStartPositionWidget->setPosition(item->mapToScene(itemPoints[0]->position()));
		if (mEndPositionWidget && itemPoints.size() >= 2)
			mEndPositionWidget->setPosition(item->mapToScene(itemPoints[1]->position()));

		if (mCurveStartPositionWidget && itemPoints.size() >= 4)
			mCurveStartPositionWidget->setPosition(item->mapToScene(itemPoints[0]->position()));
		if (mCurveEndPositionWidget && itemPoints.size() >= 4)
			mCurveEndPositionWidget->setPosition(item->mapToScene(itemPoints[3]->position()));
		if (mCurveStartControlPositionWidget && itemPoints.size() >= 4)
			mCurveStartControlPositionWidget->setPosition(item->mapToScene(itemPoints[1]->position()));
		if (mCurveEndControlPositionWidget && itemPoints.size() >= 4)
			mCurveEndControlPositionWidget->setPosition(item->mapToScene(itemPoints[2]->position()));

		if (mRectTopLeftWidget && itemPoints.size() >= 8)
			mRectTopLeftWidget->setPosition(item->mapToScene(itemPoints[0]->position()));
		if (mRectBottomRightWidget && itemPoints.size() >= 8)
			mRectBottomRightWidget->setPosition(item->mapToScene(itemPoints[1]->position()));

		if (mPointPositionStackedWidget)
		{
			if (itemPoints.size() != mPointPositionWidgets.size()) fillPointsWidgets();
			for(int i = 0; i < itemPoints.size() && i < mPointPositionWidgets.size(); i++)
				mPointPositionWidgets[i]->setPosition(item->mapToScene(itemPoints[i]->position()));
		}
	}
}

void DynamicPropertiesWidget::setItemsProperties(const QList<DrawingItem*>& items)
{
	if (items.size() > 0)
	{
		QHash<QString,QVariant> properties = items.first()->properties();
		QHash<QString,QVariant> otherItemProperties;
		bool allItemsHaveProperty;
		bool propertiesMatch;
		bool boldPropertiesMatch = false, italicPropertiesMatch = false, underlinePropertiesMatch = false,
			strikeThroughPropertiesMatch = false;
		bool textAlignHorizontalPropertiesMatch = false, textAlignVerticalPropertiesMatch = false;

		for(auto propIter = properties.begin(), propEnd = properties.end(); propIter != propEnd; propIter++)
		{
			allItemsHaveProperty = true;
			propertiesMatch = true;

			for(auto itemIter = items.begin() + 1, itemEnd = items.end(); itemIter != itemEnd; itemIter++)
			{
				otherItemProperties = (*itemIter)->properties();
				if (otherItemProperties.contains(propIter.key()))
				{
					if (propertiesMatch && propIter.value() != otherItemProperties.value(propIter.key()))
						propertiesMatch = false;
				}
				else allItemsHaveProperty = false;
			}

			if (allItemsHaveProperty)
			{
				if (propIter.key() == "pen-style" && mPenStyleCombo)
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mPenStyleCombo->setStyle(static_cast<Qt::PenStyle>(value));
						mPenStyleCombo->setEnabled(propertiesMatch);
						if (mItemStyleLabels.contains(mPenStyleCombo))
							mItemStyleLabels.value(mPenStyleCombo)->setChecked(mPenStyleCombo->isEnabled());
					}
				}
				else if (propIter.key() == "pen-color" && mPenColorWidget)
				{
					QColor color = propIter.value().value<QColor>();
					mPenColorWidget->setColor(color);
					mPenColorWidget->setEnabled(propertiesMatch);
					if (mItemStyleLabels.contains(mPenColorWidget))
						mItemStyleLabels.value(mPenColorWidget)->setChecked(mPenColorWidget->isEnabled());
				}
				else if (propIter.key() == "pen-width" && mPenWidthEdit)
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mPenWidthEdit->setSize(value);
						mPenWidthEdit->setEnabled(propertiesMatch);
						if (mItemStyleLabels.contains(mPenWidthEdit))
							mItemStyleLabels.value(mPenWidthEdit)->setChecked(mPenWidthEdit->isEnabled());
					}
				}
				else if (propIter.key() == "brush-color" && mBrushColorWidget)
				{
					QColor color = propIter.value().value<QColor>();
					mBrushColorWidget->setColor(color);
					mBrushColorWidget->setEnabled(propertiesMatch);
					if (mItemStyleLabels.contains(mBrushColorWidget))
						mItemStyleLabels.value(mBrushColorWidget)->setChecked(mBrushColorWidget->isEnabled());
				}
				else if (propIter.key() == "font-family" && mFontComboBox)
				{
					mFontComboBox->setCurrentFont(QFont(propIter.value().toString()));
					mFontComboBox->setEnabled(propertiesMatch);
					if (mItemStyleLabels.contains(mFontComboBox))
						mItemStyleLabels.value(mFontComboBox)->setChecked(mFontComboBox->isEnabled());
				}
				else if (propIter.key() == "font-size" && mFontSizeEdit)
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mFontSizeEdit->setSize(value);
						mFontSizeEdit->setEnabled(propertiesMatch);
						if (mItemStyleLabels.contains(mFontSizeEdit))
							mItemStyleLabels.value(mFontSizeEdit)->setChecked(mFontSizeEdit->isEnabled());
					}
				}
				else if (propIter.key() == "font-bold" && mFontStyleWidget)
				{
					mFontStyleWidget->setBold(propIter.value().toBool());
					boldPropertiesMatch = propertiesMatch;
				}
				else if (propIter.key() == "font-italic" && mFontStyleWidget)
				{
					mFontStyleWidget->setItalic(propIter.value().toBool());
					italicPropertiesMatch = propertiesMatch;
				}
				else if (propIter.key() == "font-underline" && mFontStyleWidget)
				{
					mFontStyleWidget->setUnderline(propIter.value().toBool());
					underlinePropertiesMatch = propertiesMatch;
				}
				else if (propIter.key() == "font-strike-through" && mFontStyleWidget)
				{
					mFontStyleWidget->setStrikeThrough(propIter.value().toBool());
					strikeThroughPropertiesMatch = propertiesMatch;
				}
				else if (propIter.key() == "text-color" && mTextColorWidget)
				{
					QColor color = propIter.value().value<QColor>();
					mTextColorWidget->setColor(color);
					mTextColorWidget->setEnabled(propertiesMatch);
					if (mItemStyleLabels.contains(mTextColorWidget))
						mItemStyleLabels.value(mTextColorWidget)->setChecked(mTextColorWidget->isEnabled());
				}
				else if (propIter.key() == "text-alignment-horizontal" && mTextAlignmentWidget)
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mTextAlignmentWidget->setAlignment(static_cast<Qt::Alignment>(value), mTextAlignmentWidget->verticalAlignment());
						textAlignHorizontalPropertiesMatch = propertiesMatch;
					}
				}
				else if (propIter.key() == "text-alignment-vertical" && mTextAlignmentWidget)
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mTextAlignmentWidget->setAlignment(mTextAlignmentWidget->horizontalAlignment(), static_cast<Qt::Alignment>(value));
						textAlignVerticalPropertiesMatch = propertiesMatch;
					}
				}
				else if (propIter.key() == "start-arrow-style" && mStartArrowCombo)
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mStartArrowCombo->setStyle(static_cast<DrawingArrow::Style>(value));
						mStartArrowCombo->setEnabled(propertiesMatch);
						if (mItemStyleLabels.contains(mStartArrowCombo))
							mItemStyleLabels.value(mStartArrowCombo)->setChecked(mStartArrowCombo->isEnabled());
					}
				}
				else if (propIter.key() == "start-arrow-size" && mStartArrowSizeEdit)
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mStartArrowSizeEdit->setSize(value);
						mStartArrowSizeEdit->setEnabled(propertiesMatch);
						if (mItemStyleLabels.contains(mStartArrowSizeEdit))
							mItemStyleLabels.value(mStartArrowSizeEdit)->setChecked(mStartArrowSizeEdit->isEnabled());
					}
				}
				else if (propIter.key() == "end-arrow-style" && mEndArrowCombo)
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mEndArrowCombo->setStyle(static_cast<DrawingArrow::Style>(value));
						mEndArrowCombo->setEnabled(propertiesMatch);
						if (mItemStyleLabels.contains(mEndArrowCombo))
							mItemStyleLabels.value(mEndArrowCombo)->setChecked(mEndArrowCombo->isEnabled());
					}
				}
				else if (propIter.key() == "end-arrow-size" && mEndArrowSizeEdit)
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mEndArrowSizeEdit->setSize(value);
						mEndArrowSizeEdit->setEnabled(propertiesMatch);
						if (mItemStyleLabels.contains(mEndArrowSizeEdit))
							mItemStyleLabels.value(mEndArrowSizeEdit)->setChecked(mEndArrowSizeEdit->isEnabled());
					}
				}
			}

			if (items.size() == 1)
			{
				if (propIter.key() == "corner-radius" && mCornerRadiusEdit)
				{
					bool ok = false;
					qreal value = properties["corner-radius"].toDouble(&ok);
					if (ok) mCornerRadiusEdit->setSize(value);
				}
				else if (propIter.key() == "caption" && mCaptionEdit)
				{
					QString caption = properties["caption"].toString();

					if (mCaptionEdit->toPlainText() != caption)
					{
						disconnect(mCaptionEdit, SIGNAL(textChanged()), this, SLOT(handleItemGeometryChange()));
						mCaptionEdit->setPlainText(caption);
						connect(mCaptionEdit, SIGNAL(textChanged()), this, SLOT(handleItemGeometryChange()));
					}
				}
			}
		}

		if (mFontStyleWidget)
		{
			mFontStyleWidget->setEnabled(boldPropertiesMatch &&
				italicPropertiesMatch && underlinePropertiesMatch && strikeThroughPropertiesMatch);
			if (mItemStyleLabels.contains(mFontStyleWidget))
				mItemStyleLabels.value(mFontStyleWidget)->setChecked(mFontStyleWidget->isEnabled());
		}

		if (mTextAlignmentWidget)
		{
			mTextAlignmentWidget->setEnabled(textAlignHorizontalPropertiesMatch && textAlignVerticalPropertiesMatch);
			if (mItemStyleLabels.contains(mTextAlignmentWidget))
				mItemStyleLabels.value(mTextAlignmentWidget)->setChecked(mTextAlignmentWidget->isEnabled());
		}
	}
}

//==================================================================================================

void DynamicPropertiesWidget::setDefaultItemsProperties(const QHash<QString,QVariant>& properties)
{
	for(auto propIter = properties.begin(), propEnd = properties.end(); propIter != propEnd; propIter++)
	{
		if (propIter.key() == "pen-style")
		{
			bool ok = false;
			uint value = propIter.value().toUInt(&ok);
			if (ok) mDefaultPenStyleCombo->setStyle(static_cast<Qt::PenStyle>(value));
		}
		else if (propIter.key() == "pen-color")
		{
			mDefaultPenColorWidget->setColor(propIter.value().value<QColor>());
		}
		else if (propIter.key() == "pen-width")
		{
			bool ok = false;
			qreal value = propIter.value().toDouble(&ok);
			if (ok) mDefaultPenWidthEdit->setSize(value);
		}
		else if (propIter.key() == "brush-color")
		{
			mDefaultBrushColorWidget->setColor(propIter.value().value<QColor>());
		}
		else if (propIter.key() == "font-family")
		{
			mDefaultFontComboBox->setCurrentFont(QFont(propIter.value().toString()));
		}
		else if (propIter.key() == "font-size")
		{
			bool ok = false;
			qreal value = propIter.value().toDouble(&ok);
			if (ok) mDefaultFontSizeEdit->setSize(value);
		}
		else if (propIter.key() == "font-bold")
		{
			mDefaultFontStyleWidget->setBold(propIter.value().toBool());
		}
		else if (propIter.key() == "font-italic")
		{
			mDefaultFontStyleWidget->setItalic(propIter.value().toBool());
		}
		else if (propIter.key() == "font-underline")
		{
			mDefaultFontStyleWidget->setUnderline(propIter.value().toBool());
		}
		else if (propIter.key() == "font-strike-through")
		{
			mDefaultFontStyleWidget->setStrikeThrough(propIter.value().toBool());
		}
		else if (propIter.key() == "text-color")
		{
			mDefaultTextColorWidget->setColor(propIter.value().value<QColor>());
		}
		else if (propIter.key() == "text-alignment-horizontal")
		{
			bool ok = false;
			uint value = propIter.value().toUInt(&ok);
			if (ok) mDefaultTextAlignmentWidget->setAlignment(static_cast<Qt::Alignment>(value), mDefaultTextAlignmentWidget->verticalAlignment());
		}
		else if (propIter.key() == "text-alignment-vertical")
		{
			bool ok = false;
			uint value = propIter.value().toUInt(&ok);
			if (ok) mDefaultTextAlignmentWidget->setAlignment(mDefaultTextAlignmentWidget->horizontalAlignment(), static_cast<Qt::Alignment>(value));
		}
		else if (propIter.key() == "start-arrow-style")
		{
			bool ok = false;
			uint value = propIter.value().toUInt(&ok);
			if (ok) mDefaultStartArrowCombo->setStyle(static_cast<DrawingArrow::Style>(value));
		}
		else if (propIter.key() == "start-arrow-size")
		{
			bool ok = false;
			qreal value = propIter.value().toDouble(&ok);
			if (ok) mDefaultStartArrowSizeEdit->setSize(value);
		}
		else if (propIter.key() == "end-arrow-style")
		{
			bool ok = false;
			uint value = propIter.value().toUInt(&ok);
			if (ok) mDefaultEndArrowCombo->setStyle(static_cast<DrawingArrow::Style>(value));
		}
		else if (propIter.key() == "end-arrow-size")
		{
			bool ok = false;
			qreal value = propIter.value().toDouble(&ok);
			if (ok) mDefaultEndArrowSizeEdit->setSize(value);
		}
	}
}

void DynamicPropertiesWidget::setDrawingProperties(const QHash<QString,QVariant>& properties)
{
	if (properties.contains("scene-rect"))
	{
		QRectF rect = properties["scene-rect"].toRectF();
		if (rect.isValid())
		{
			mDrawingTopLeftWidget->setPosition(rect.topLeft());
			mDrawingRectSizeWidget->setSize(rect.size());
		}
	}

	if (properties.contains("background-color"))
	{
		QColor color = properties["background-color"].value<QColor>();
		mDrawingBackgroundColorWidget->setColor(color);
	}

	if (properties.contains("grid"))
	{
		bool ok = false;
		qreal value = properties["grid"].toDouble(&ok);
		if (ok) mDrawingGridEdit->setSize(value);
	}

	if (properties.contains("grid-style"))
	{
		bool ok = false;
		uint value = properties["grid-style"].toUInt(&ok);
		if (ok) mDrawingGridStyleCombo->setStyle(static_cast<DrawingWidget::GridStyle>(value));
	}

	if (properties.contains("grid-color"))
	{
		QColor color = properties["grid-color"].value<QColor>();
		mDrawingGridColorWidget->setColor(color);
	}

	if (properties.contains("grid-spacing-major"))
	{
		bool ok = false;
		int value = properties["grid-spacing-major"].toInt(&ok);
		if (ok) mDrawingGridSpacingMajorWidget->setText(QString::number(value));
	}

	if (properties.contains("grid-spacing-minor"))
	{
		bool ok = false;
		int value = properties["grid-spacing-minor"].toInt(&ok);
		if (ok) mDrawingGridSpacingMinorWidget->setText(QString::number(value));
	}

	if (properties.contains("dynamic-grid"))
	{
		bool ok = false;
		qreal value = properties["dynamic-grid"].toDouble(&ok);
		if (ok)
		{
			mDrawingDynamicGridCombo->setCurrentIndex((value != 0) ? 0 : 1);
			mDrawingDynamicGridEdit->setSize((value != 0) ? value : 1000);
			mDrawingGridEdit->setEnabled(value == 0);
			mDrawingDynamicGridEdit->setEnabled(value != 0);
		}
	}

	mDrawingGridSpacingMinorWidget->setEnabled(mDrawingGridStyleCombo->currentIndex() == 3);
}

void DynamicPropertiesWidget::setDrawingGrid(qreal grid)
{
	mDrawingGridEdit->setSize(grid);
}

//==================================================================================================

void DynamicPropertiesWidget::handleItemGeometryChange()
{
	QObject* sender = DynamicPropertiesWidget::sender();

	if (mPositionWidget && sender == mPositionWidget && mItems.size() > 0)
		emit selectedItemMoved(mPositionWidget->position() - mItems.first()->position());
	else if (mItem)
	{
		QList<DrawingItemPoint*> itemPoints = mItem->points();

		if (mStartPositionWidget && sender == mStartPositionWidget && itemPoints.size() >= 2)
			emit selectedItemResized(itemPoints[0], mStartPositionWidget->position());
		else if (mEndPositionWidget && sender == mEndPositionWidget && itemPoints.size() >= 2)
			emit selectedItemResized(itemPoints[1], mEndPositionWidget->position());
		else if (mCurveStartPositionWidget && sender == mCurveStartPositionWidget && itemPoints.size() >= 4)
			emit selectedItemResized(itemPoints[0], mCurveStartPositionWidget->position());
		else if (mCurveStartControlPositionWidget && sender == mCurveStartControlPositionWidget && itemPoints.size() >= 4)
			emit selectedItemResized(itemPoints[1], mCurveStartControlPositionWidget->position());
		else if (mCurveEndPositionWidget && sender == mCurveEndPositionWidget && itemPoints.size() >= 4)
			emit selectedItemResized(itemPoints[3], mCurveEndPositionWidget->position());
		else if (mCurveEndControlPositionWidget && sender == mCurveEndControlPositionWidget && itemPoints.size() >= 4)
			emit selectedItemResized(itemPoints[2], mCurveEndControlPositionWidget->position());
		else if (mRectTopLeftWidget && sender == mRectTopLeftWidget && itemPoints.size() >= 8)
			emit selectedItemResized(itemPoints[0], mRectTopLeftWidget->position());
		else if (mRectBottomRightWidget && sender == mRectBottomRightWidget && itemPoints.size() >= 8)
			emit selectedItemResized(itemPoints[1], mRectBottomRightWidget->position());
		else if	(!mPointPositionWidgets.isEmpty())
		{
			for(int i = 0; i < itemPoints.size() && i < mPointPositionWidgets.size(); i++)
			{
				if (sender == mPointPositionWidgets[i])
					emit selectedItemResized(itemPoints[i], mPointPositionWidgets[i]->position());
			}
		}
	}
}

void DynamicPropertiesWidget::handleItemsPropertiesChange()
{
	QHash<QString,QVariant> newProperties;
	QObject* sender = DynamicPropertiesWidget::sender();

	if (mPenStyleCombo && (sender == mPenStyleCombo || (mItemStyleLabels.contains(mPenStyleCombo) && sender == mItemStyleLabels[mPenStyleCombo] && mItemStyleLabels[mPenStyleCombo]->isChecked())))
		newProperties["pen-style"] = static_cast<uint>(mPenStyleCombo->style());
	else if (mPenWidthEdit && (sender == mPenWidthEdit || (mItemStyleLabels.contains(mPenWidthEdit) && sender == mItemStyleLabels[mPenWidthEdit] && mItemStyleLabels[mPenWidthEdit]->isChecked())))
		newProperties["pen-width"] = mPenWidthEdit->size();
	else if (mPenColorWidget && (sender == mPenColorWidget || (mItemStyleLabels.contains(mPenColorWidget) && sender == mItemStyleLabels[mPenColorWidget] && mItemStyleLabels[mPenColorWidget]->isChecked())))
		newProperties["pen-color"] = mPenColorWidget->color();
	else if (mBrushColorWidget && (sender == mBrushColorWidget || (mItemStyleLabels.contains(mBrushColorWidget) && sender == mItemStyleLabels[mBrushColorWidget] && mItemStyleLabels[mBrushColorWidget]->isChecked())))
		newProperties["brush-color"] = mBrushColorWidget->color();
	else if (mFontComboBox && (sender == mFontComboBox || (mItemStyleLabels.contains(mFontComboBox) && sender == mItemStyleLabels[mFontComboBox] && mItemStyleLabels[mFontComboBox]->isChecked())))
		newProperties["font-family"] = mFontComboBox->currentFont().family();
	else if (mFontSizeEdit && (sender == mFontSizeEdit || (mItemStyleLabels.contains(mFontSizeEdit) && sender == mItemStyleLabels[mFontSizeEdit] && mItemStyleLabels[mFontSizeEdit]->isChecked())))
		newProperties["font-size"] = mFontSizeEdit->size();
	else if (mFontStyleWidget && (sender == mFontStyleWidget || (mItemStyleLabels.contains(mFontStyleWidget) && sender == mItemStyleLabels[mFontStyleWidget] && mItemStyleLabels[mFontStyleWidget]->isChecked())))
	{
		newProperties["font-bold"] = mFontStyleWidget->isBold();
		newProperties["font-italic"] = mFontStyleWidget->isItalic();
		newProperties["font-underline"] = mFontStyleWidget->isUnderline();
		newProperties["font-strike-through"] = mFontStyleWidget->isStrikeThrough();
	}
	else if (mTextAlignmentWidget && (sender == mTextAlignmentWidget || (mItemStyleLabels.contains(mTextAlignmentWidget) && sender == mItemStyleLabels[mTextAlignmentWidget] && mItemStyleLabels[mTextAlignmentWidget]->isChecked())))
	{
		newProperties["text-alignment-horizontal"] = static_cast<uint>(mTextAlignmentWidget->horizontalAlignment());
		newProperties["text-alignment-vertical"] = static_cast<uint>(mTextAlignmentWidget->verticalAlignment());
	}
	else if (mTextColorWidget && (sender == mTextColorWidget || (mItemStyleLabels.contains(mTextColorWidget) && sender == mItemStyleLabels[mTextColorWidget] && mItemStyleLabels[mTextColorWidget]->isChecked())))
		newProperties["text-color"] = mTextColorWidget->color();
	else if (mStartArrowCombo && (sender == mStartArrowCombo || (mItemStyleLabels.contains(mStartArrowCombo) && sender == mItemStyleLabels[mStartArrowCombo] && mItemStyleLabels[mStartArrowCombo]->isChecked())))
		newProperties["start-arrow-style"] = static_cast<uint>(mStartArrowCombo->style());
	else if (mStartArrowSizeEdit && (sender == mStartArrowSizeEdit || (mItemStyleLabels.contains(mStartArrowSizeEdit) && sender == mItemStyleLabels[mStartArrowSizeEdit] && mItemStyleLabels[mStartArrowSizeEdit]->isChecked())))
		newProperties["start-arrow-size"] = mStartArrowSizeEdit->size();
	else if (mEndArrowCombo && (sender == mEndArrowCombo || (mItemStyleLabels.contains(mEndArrowCombo) && sender == mItemStyleLabels[mEndArrowCombo] && mItemStyleLabels[mEndArrowCombo]->isChecked())))
		newProperties["end-arrow-style"] = static_cast<uint>(mEndArrowCombo->style());
	else if (mEndArrowSizeEdit && (sender == mEndArrowSizeEdit || (mItemStyleLabels.contains(mEndArrowSizeEdit) && sender == mItemStyleLabels[mEndArrowSizeEdit] && mItemStyleLabels[mEndArrowSizeEdit]->isChecked())))
		newProperties["end-arrow-size"] = mEndArrowSizeEdit->size();
	else if (mCornerRadiusEdit && sender == mCornerRadiusEdit)
		newProperties["corner-radius"] = mCornerRadiusEdit->size();
	else if (mCaptionEdit && sender == mCaptionEdit)
		newProperties["caption"] = mCaptionEdit->toPlainText();

	if (!newProperties.isEmpty()) emit selectedItemsPropertiesChanged(newProperties);
}

void DynamicPropertiesWidget::handleItemDefaultsChange()
{
	QHash<QString,QVariant> properties;

	QObject* sender = DynamicPropertiesWidget::sender();

	if (sender == mDefaultPenStyleCombo)
		properties["pen-style"] = static_cast<uint>(mDefaultPenStyleCombo->style());
	else if (sender == mDefaultPenWidthEdit)
		properties["pen-width"] = mDefaultPenWidthEdit->size();
	else if (sender == mDefaultPenColorWidget)
		properties["pen-color"] = mDefaultPenColorWidget->color();
	else if (sender == mDefaultBrushColorWidget)
		properties["brush-color"] = mDefaultBrushColorWidget->color();
	else if (sender == mDefaultFontComboBox)
		properties["font-family"] = mDefaultFontComboBox->currentFont().family();
	else if (sender == mDefaultFontSizeEdit)
		properties["font-size"] = mDefaultFontSizeEdit->size();
	else if (sender == mDefaultFontStyleWidget)
	{
		properties["font-bold"] = mDefaultFontStyleWidget->isBold();
		properties["font-italic"] = mDefaultFontStyleWidget->isItalic();
		properties["font-underline"] = mDefaultFontStyleWidget->isUnderline();
		properties["font-strike-through"] = mDefaultFontStyleWidget->isStrikeThrough();
	}
	else if (sender == mDefaultTextAlignmentWidget)
	{
		properties["text-alignment-horizontal"] = static_cast<uint>(mDefaultTextAlignmentWidget->horizontalAlignment());
		properties["text-alignment-vertical"] = static_cast<uint>(mDefaultTextAlignmentWidget->verticalAlignment());
	}
	else if (sender == mDefaultTextColorWidget)
		properties["text-color"] = mDefaultTextColorWidget->color();
	else if (sender == mDefaultStartArrowCombo)
		properties["start-arrow-style"] = static_cast<uint>(mDefaultStartArrowCombo->style());
	else if (sender == mDefaultStartArrowSizeEdit)
		properties["start-arrow-size"] = mDefaultStartArrowSizeEdit->size();
	else if (sender == mDefaultEndArrowCombo)
		properties["end-arrow-style"] = static_cast<uint>(mDefaultEndArrowCombo->style());
	else if (sender == mDefaultEndArrowSizeEdit)
		properties["end-arrow-size"] = mDefaultEndArrowSizeEdit->size();

	if (!properties.isEmpty()) emit defaultItemsPropertiesChanged(properties);
}

void DynamicPropertiesWidget::handleDrawingChange()
{
	QHash<QString,QVariant> newProperties;
	QObject* sender = DynamicPropertiesWidget::sender();

	if (sender == mDrawingGridStyleCombo)
		mDrawingGridSpacingMinorWidget->setEnabled(mDrawingGridStyleCombo->currentIndex() == 3);

	if (sender == mDrawingTopLeftWidget || sender == mDrawingRectSizeWidget)
		newProperties["scene-rect"] = QRectF(mDrawingTopLeftWidget->position(), mDrawingRectSizeWidget->size());
	else if (sender == mDrawingBackgroundColorWidget)
		newProperties["background-color"] = mDrawingBackgroundColorWidget->color();
	else if (sender == mDrawingGridEdit)
		newProperties["grid"] = mDrawingGridEdit->size();
	else if (sender == mDrawingGridStyleCombo)
		newProperties["grid-style"] = static_cast<uint>(mDrawingGridStyleCombo->style());
	else if (sender == mDrawingGridColorWidget)
		newProperties["grid-color"] = mDrawingGridColorWidget->color();
	else if (sender == mDrawingGridSpacingMajorWidget)
		newProperties["grid-spacing-major"] = mDrawingGridSpacingMajorWidget->text().toInt();
	else if (sender == mDrawingGridSpacingMinorWidget)
		newProperties["grid-spacing-minor"] = mDrawingGridSpacingMinorWidget->text().toInt();
	else if (sender == mDrawingDynamicGridCombo)
	{
		bool enable = (mDrawingDynamicGridCombo->currentIndex() == 0);
		mDrawingGridEdit->setEnabled(!enable);
		mDrawingDynamicGridEdit->setEnabled(enable);

		if (enable)
		{
			bool ok = false;
			qreal grid = mDrawingDynamicGridEdit->text().toDouble(&ok);
			newProperties["dynamic-grid"] = (ok) ? grid : 0;
		}
		else newProperties["dynamic-grid"] = 0;
	}
	else if (sender == mDrawingDynamicGridCombo || sender == mDrawingDynamicGridEdit)
	{
		bool enable = (mDrawingDynamicGridCombo->currentIndex() == 0);

		bool ok = false;
		qreal grid = mDrawingDynamicGridEdit->text().toDouble(&ok);
		newProperties["dynamic-grid"] = (ok && enable) ? grid : 0;
	}

	if (!newProperties.isEmpty()) emit drawingPropertiesChanged(newProperties);
}

//==================================================================================================

QWidget* DynamicPropertiesWidget::createDrawingWidget()
{
	QWidget* diagramPropertiesWidget = new QWidget();
	QVBoxLayout* diagramPropertiesLayout = new QVBoxLayout();
	QGroupBox* groupBox;
	QFormLayout* groupLayout;

	// Create widgets
	mDrawingTopLeftWidget = new PositionWidget();
	connect(mDrawingTopLeftWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleDrawingChange()));

	mDrawingRectSizeWidget = new SizeWidget(QSizeF(10000, 7500));
	connect(mDrawingRectSizeWidget, SIGNAL(sizeChanged(const QSizeF&)), this, SLOT(handleDrawingChange()));

	mDrawingBackgroundColorWidget = new ColorWidget();
	connect(mDrawingBackgroundColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleDrawingChange()));

	mDrawingGridEdit = new SizeEdit();
	connect(mDrawingGridEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleDrawingChange()));

	mDrawingGridStyleCombo = new GridStyleCombo();
	connect(mDrawingGridStyleCombo, SIGNAL(activated(int)), this, SLOT(handleDrawingChange()));

	mDrawingGridColorWidget = new ColorWidget();
	connect(mDrawingGridColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleDrawingChange()));

	mDrawingGridSpacingMajorWidget = new QLineEdit();
	mDrawingGridSpacingMajorWidget->setValidator(new QIntValidator(1, 1E6));
	//connect(mDrawingGridSpacingMajorWidget, SIGNAL(returnPressed()), this, SLOT(handleDrawingChange()));
	connect(mDrawingGridSpacingMajorWidget, SIGNAL(editingFinished()), this, SLOT(handleDrawingChange()));

	mDrawingGridSpacingMinorWidget = new QLineEdit();
	mDrawingGridSpacingMinorWidget->setValidator(new QIntValidator(1, 1E6));
	//connect(mDrawingGridSpacingMinorWidget, SIGNAL(returnPressed()), this, SLOT(handleDrawingChange()));
	connect(mDrawingGridSpacingMinorWidget, SIGNAL(editingFinished()), this, SLOT(handleDrawingChange()));

	mDrawingDynamicGridCombo = new QComboBox();
	mDrawingDynamicGridCombo->addItem("Enabled");
	mDrawingDynamicGridCombo->addItem("Disabled");
	connect(mDrawingDynamicGridCombo, SIGNAL(activated(int)), this, SLOT(handleDrawingChange()));

	mDrawingDynamicGridEdit = new SizeEdit();
	connect(mDrawingGridEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleDrawingChange()));

	// Assemble layout
	groupBox = new QGroupBox("Diagram");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Top Left:", mDrawingTopLeftWidget);
	groupLayout->addRow("Size:", mDrawingRectSizeWidget);
	groupLayout->addRow("Background Color:", mDrawingBackgroundColorWidget);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth());
	groupBox->setLayout(groupLayout);
	diagramPropertiesLayout->addWidget(groupBox);

	groupBox = new QGroupBox("Grid");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Grid:", mDrawingGridEdit);
	groupLayout->addRow("Grid Style:", mDrawingGridStyleCombo);
	groupLayout->addRow("Grid Color:", mDrawingGridColorWidget);
	groupLayout->addRow("Major Spacing:", mDrawingGridSpacingMajorWidget);
	groupLayout->addRow("Minor Spacing:", mDrawingGridSpacingMinorWidget);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth());
	groupLayout->addRow("Dynamic Grid:", mDrawingDynamicGridCombo);
	groupLayout->addRow("Dynamic Grid Value:", mDrawingDynamicGridEdit);
	groupBox->setLayout(groupLayout);
	diagramPropertiesLayout->addWidget(groupBox);

	diagramPropertiesLayout->addWidget(new QWidget(), 100);
	diagramPropertiesWidget->setLayout(diagramPropertiesLayout);
	return diagramPropertiesWidget;
}

QWidget* DynamicPropertiesWidget::createItemDefaultsWidget()
{
	QWidget* itemDefaultPropertiesWidget = new QWidget();
	QVBoxLayout* itemDefaultPropertiesLayout = new QVBoxLayout();
	QGroupBox* groupBox;
	QFormLayout* groupLayout;

	// Pen / Brush widgets
	mDefaultPenStyleCombo = new PenStyleCombo(Qt::SolidLine);
	connect(mDefaultPenStyleCombo, SIGNAL(activated(int)), this, SLOT(handleItemDefaultsChange()));

	mDefaultPenWidthEdit = new SizeEdit(16.0);
	connect(mDefaultPenWidthEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemDefaultsChange()));

	mDefaultPenColorWidget = new ColorWidget();
	QColor color(0, 0, 0);
	mDefaultPenColorWidget->setColor(color);
	connect(mDefaultPenColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleItemDefaultsChange()));

	mDefaultBrushColorWidget = new ColorWidget();
	color = QColor(255, 255, 255);
	mDefaultBrushColorWidget->setColor(color);
	connect(mDefaultBrushColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleItemDefaultsChange()));

	// Text widgets
	mDefaultFontComboBox = new QFontComboBox();
	mDefaultFontComboBox->setCurrentFont(QFont("Arial"));
	connect(mDefaultFontComboBox, SIGNAL(activated(int)), this, SLOT(handleItemDefaultsChange()));

	mDefaultFontSizeEdit = new SizeEdit(100);
	connect(mDefaultFontSizeEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemDefaultsChange()));

	mDefaultFontStyleWidget = new FontStyleWidget();
	mDefaultFontStyleWidget->setBold(false);
	mDefaultFontStyleWidget->setItalic(false);
	mDefaultFontStyleWidget->setUnderline(false);
	mDefaultFontStyleWidget->setStrikeThrough(false);
	connect(mDefaultFontStyleWidget, SIGNAL(boldChanged(bool)), this, SLOT(handleItemDefaultsChange()));
	connect(mDefaultFontStyleWidget, SIGNAL(italicChanged(bool)), this, SLOT(handleItemDefaultsChange()));
	connect(mDefaultFontStyleWidget, SIGNAL(underlineChanged(bool)), this, SLOT(handleItemDefaultsChange()));
	connect(mDefaultFontStyleWidget, SIGNAL(strikeThroughChanged(bool)), this, SLOT(handleItemDefaultsChange()));

	mDefaultTextAlignmentWidget = new TextAlignmentWidget();
	mDefaultTextAlignmentWidget->setAlignment(Qt::AlignHCenter, Qt::AlignVCenter);
	connect(mDefaultTextAlignmentWidget, SIGNAL(horizontalAlignmentChanged(Qt::Alignment)), this, SLOT(handleItemDefaultsChange()));
	connect(mDefaultTextAlignmentWidget, SIGNAL(verticalAlignmentChanged(Qt::Alignment)), this, SLOT(handleItemDefaultsChange()));

	mDefaultTextColorWidget = new ColorWidget();
	color = QColor(0, 0, 0);
	mDefaultTextColorWidget->setColor(color);
	connect(mDefaultTextColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleItemDefaultsChange()));

	// Arrow widgets
	mDefaultStartArrowCombo = new ArrowStyleCombo(DrawingArrow::None);
	connect(mDefaultStartArrowCombo, SIGNAL(activated(int)), this, SLOT(handleItemDefaultsChange()));

	mDefaultStartArrowSizeEdit = new SizeEdit(100);
	connect(mDefaultStartArrowSizeEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemDefaultsChange()));

	mDefaultEndArrowCombo = new ArrowStyleCombo(DrawingArrow::None);
	connect(mDefaultEndArrowCombo, SIGNAL(activated(int)), this, SLOT(handleItemDefaultsChange()));

	mDefaultEndArrowSizeEdit = new SizeEdit(100);
	connect(mDefaultEndArrowSizeEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemDefaultsChange()));

	// Assemble layout
	groupBox = new QGroupBox("Pen / Brush Defaults");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Pen Style:", mDefaultPenStyleCombo);
	groupLayout->addRow("Pen Width:", mDefaultPenWidthEdit);
	groupLayout->addRow("Pen Color:", mDefaultPenColorWidget);
	groupLayout->addRow("Brush Color:", mDefaultBrushColorWidget);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth());
	groupBox->setLayout(groupLayout);
	itemDefaultPropertiesLayout->addWidget(groupBox);

	groupBox = new QGroupBox("Text Defaults");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Font:", mDefaultFontComboBox);
	groupLayout->addRow("Font Size:", mDefaultFontSizeEdit);
	groupLayout->addRow("Font Style:", mDefaultFontStyleWidget);
	groupLayout->addRow("Alignment:", mDefaultTextAlignmentWidget);
	groupLayout->addRow("Text Color:", mDefaultTextColorWidget);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth());
	groupBox->setLayout(groupLayout);
	itemDefaultPropertiesLayout->addWidget(groupBox);

	groupBox = new QGroupBox("Arrow Defaults");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Start Arrow:", mDefaultStartArrowCombo);
	groupLayout->addRow("Arrow Size:", mDefaultStartArrowSizeEdit);
	groupLayout->addRow("End Arrow:", mDefaultEndArrowCombo);
	groupLayout->addRow("Arrow Size:", mDefaultEndArrowSizeEdit);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth());
	groupBox->setLayout(groupLayout);
	itemDefaultPropertiesLayout->addWidget(groupBox);

	itemDefaultPropertiesLayout->addWidget(new QWidget(), 100);
	itemDefaultPropertiesWidget->setLayout(itemDefaultPropertiesLayout);
	return itemDefaultPropertiesWidget;
}

//==================================================================================================

void DynamicPropertiesWidget::createGeometryWidgets()
{
	DrawingCurveItem* curveItem = dynamic_cast<DrawingCurveItem*>(mItem);
	DrawingEllipseItem* ellipseItem = dynamic_cast<DrawingEllipseItem*>(mItem);
	DrawingLineItem* lineItem = dynamic_cast<DrawingLineItem*>(mItem);
	DrawingPathItem* pathItem = dynamic_cast<DrawingPathItem*>(mItem);
	DrawingPolygonItem* polygonItem = dynamic_cast<DrawingPolygonItem*>(mItem);
	DrawingPolylineItem* polylineItem = dynamic_cast<DrawingPolylineItem*>(mItem);
	DrawingRectItem* rectItem = dynamic_cast<DrawingRectItem*>(mItem);
	DrawingTextEllipseItem* textEllipseItem = dynamic_cast<DrawingTextEllipseItem*>(mItem);
	DrawingTextItem* textItem = dynamic_cast<DrawingTextItem*>(mItem);
	DrawingTextRectItem* textRectItem = dynamic_cast<DrawingTextRectItem*>(mItem);
	DrawingItemGroup* groupItem = dynamic_cast<DrawingItemGroup*>(mItem);

	// Position widget
	if (pathItem || polygonItem || polylineItem || textItem || groupItem)
	{
		mPositionWidget = new PositionWidget(mItem->position());
		connect(mPositionWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));
	}

	// Line/arc widgets
	if (lineItem)
	{
		mStartPositionWidget = new PositionWidget(mItem->mapToScene(mItem->points()[0]->position()));
		connect(mStartPositionWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));

		mEndPositionWidget = new PositionWidget(mItem->mapToScene(mItem->points()[1]->position()));
		connect(mEndPositionWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));
	}

	// Curve widgets
	if (curveItem)
	{
		mCurveStartPositionWidget = new PositionWidget(mItem->mapToScene(mItem->points()[0]->position()));
		connect(mCurveStartPositionWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));

		mCurveEndPositionWidget = new PositionWidget(mItem->mapToScene(mItem->points()[3]->position()));
		connect(mCurveEndPositionWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));

		mCurveStartControlPositionWidget = new PositionWidget(mItem->mapToScene(mItem->points()[1]->position()));
		connect(mCurveStartControlPositionWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));

		mCurveEndControlPositionWidget = new PositionWidget(mItem->mapToScene(mItem->points()[2]->position()));
		connect(mCurveEndControlPositionWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));
	}

	// Rect widgets
	if (ellipseItem || pathItem || rectItem || textEllipseItem || textRectItem)
	{
		mRectTopLeftWidget = new PositionWidget(mItem->mapToScene(mItem->points()[0]->position()));
		connect(mRectTopLeftWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));

		mRectBottomRightWidget = new PositionWidget(mItem->mapToScene(mItem->points()[1]->position()));
		connect(mRectBottomRightWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));
	}

	// Poly widgets
	if (polygonItem || polylineItem)
	{
		mPointPositionStackedWidget = new QStackedWidget();
		fillPointsWidgets();
	}
}

void DynamicPropertiesWidget::createPropertiesWidgets()
{
	if (mItems.size() > 0)
	{
		QHash<QString,QVariant> properties = mItems.first()->properties();
		QHash<QString,QVariant> otherItemProperties;
		bool allItemsHaveProperty;
		bool propertiesMatch;
		bool boldPropertiesMatch = false, italicPropertiesMatch = false, underlinePropertiesMatch = false,
			strikeThroughPropertiesMatch = false;
		bool textAlignHorizontalPropertiesMatch = false, textAlignVerticalPropertiesMatch = false;

		for(auto propIter = properties.begin(), propEnd = properties.end(); propIter != propEnd; propIter++)
		{
			allItemsHaveProperty = true;
			propertiesMatch = true;

			for(auto itemIter = mItems.begin() + 1, itemEnd = mItems.end(); itemIter != itemEnd; itemIter++)
			{
				otherItemProperties = (*itemIter)->properties();
				if (otherItemProperties.contains(propIter.key()))
				{
					if (propertiesMatch && propIter.value() != otherItemProperties.value(propIter.key()))
						propertiesMatch = false;
				}
				else allItemsHaveProperty = false;
			}

			if (allItemsHaveProperty)
			{
				if (propIter.key() == "pen-style")
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mPenStyleCombo = new PenStyleCombo(static_cast<Qt::PenStyle>(value));
						mPenStyleCombo->setEnabled(propertiesMatch);
						connect(mPenStyleCombo, SIGNAL(activated(int)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "pen-color")
				{
					QColor color = propIter.value().value<QColor>();
					mPenColorWidget = new ColorWidget(color);
					mPenColorWidget->setEnabled(propertiesMatch);
					connect(mPenColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "pen-width")
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mPenWidthEdit = new SizeEdit(value);
						mPenWidthEdit->setEnabled(propertiesMatch);
						connect(mPenWidthEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "brush-color")
				{
					QColor color = propIter.value().value<QColor>();
					mBrushColorWidget = new ColorWidget(color);
					mBrushColorWidget->setEnabled(propertiesMatch);
					connect(mBrushColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "font-family")
				{
					mFontComboBox = new QFontComboBox();
					mFontComboBox->setCurrentFont(QFont(propIter.value().toString()));
					mFontComboBox->setEnabled(propertiesMatch);
					connect(mFontComboBox, SIGNAL(activated(int)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "font-size")
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mFontSizeEdit = new SizeEdit(value);
						mFontSizeEdit->setEnabled(propertiesMatch);
						connect(mFontSizeEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "font-bold")
				{
					if (mFontStyleWidget == nullptr) mFontStyleWidget = new FontStyleWidget();
					mFontStyleWidget->setBold(propIter.value().toBool());
					boldPropertiesMatch = propertiesMatch;
					connect(mFontStyleWidget, SIGNAL(boldChanged(bool)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "font-italic")
				{
					if (mFontStyleWidget == nullptr) mFontStyleWidget = new FontStyleWidget();
					mFontStyleWidget->setItalic(propIter.value().toBool());
					italicPropertiesMatch = propertiesMatch;
					connect(mFontStyleWidget, SIGNAL(italicChanged(bool)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "font-underline")
				{
					if (mFontStyleWidget == nullptr) mFontStyleWidget = new FontStyleWidget();
					mFontStyleWidget->setUnderline(propIter.value().toBool());
					underlinePropertiesMatch = propertiesMatch;
					connect(mFontStyleWidget, SIGNAL(underlineChanged(bool)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "font-strike-through")
				{
					if (mFontStyleWidget == nullptr) mFontStyleWidget = new FontStyleWidget();
					mFontStyleWidget->setStrikeThrough(propIter.value().toBool());
					strikeThroughPropertiesMatch = propertiesMatch;
					connect(mFontStyleWidget, SIGNAL(strikeThroughChanged(bool)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "text-color")
				{
					QColor color = propIter.value().value<QColor>();
					mTextColorWidget = new ColorWidget(color);
					mTextColorWidget->setEnabled(propertiesMatch);
					connect(mTextColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleItemsPropertiesChange()));
				}
				else if (propIter.key() == "text-alignment-horizontal")
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						if (mTextAlignmentWidget == nullptr) mTextAlignmentWidget = new TextAlignmentWidget();
						mTextAlignmentWidget->setAlignment(static_cast<Qt::Alignment>(value), mTextAlignmentWidget->verticalAlignment());
						textAlignHorizontalPropertiesMatch = propertiesMatch;
						connect(mTextAlignmentWidget, SIGNAL(horizontalAlignmentChanged(Qt::Alignment)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "text-alignment-vertical")
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						if (mTextAlignmentWidget == nullptr) mTextAlignmentWidget = new TextAlignmentWidget();
						mTextAlignmentWidget->setAlignment(mTextAlignmentWidget->horizontalAlignment(), static_cast<Qt::Alignment>(value));
						textAlignVerticalPropertiesMatch = propertiesMatch;
						connect(mTextAlignmentWidget, SIGNAL(verticalAlignmentChanged(Qt::Alignment)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "start-arrow-style")
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mStartArrowCombo = new ArrowStyleCombo(static_cast<DrawingArrow::Style>(value));
						mStartArrowCombo->setEnabled(propertiesMatch);
						connect(mStartArrowCombo, SIGNAL(activated(int)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "start-arrow-size")
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mStartArrowSizeEdit = new SizeEdit(value);
						mStartArrowSizeEdit->setEnabled(propertiesMatch);
						connect(mStartArrowSizeEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "end-arrow-style")
				{
					bool ok = false;
					uint value = propIter.value().toUInt(&ok);
					if (ok)
					{
						mEndArrowCombo = new ArrowStyleCombo(static_cast<DrawingArrow::Style>(value));
						mEndArrowCombo->setEnabled(propertiesMatch);
						connect(mEndArrowCombo, SIGNAL(activated(int)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "end-arrow-size")
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mEndArrowSizeEdit = new SizeEdit(value);
						mEndArrowSizeEdit->setEnabled(propertiesMatch);
						connect(mEndArrowSizeEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
			}

			if (mItems.size() == 1)
			{
				if (propIter.key() == "corner-radius")
				{
					bool ok = false;
					qreal value = propIter.value().toDouble(&ok);
					if (ok)
					{
						mCornerRadiusEdit = new SizeEdit(value);
						connect(mCornerRadiusEdit, SIGNAL(sizeChanged(qreal)), this, SLOT(handleItemsPropertiesChange()));
					}
				}
				else if (propIter.key() == "caption")
				{
					mCaptionEdit = new QTextEdit();
					mCaptionEdit->setMaximumHeight(QFontMetrics(mCaptionEdit->font()).height() * 6 + 8);
					mCaptionEdit->setPlainText(propIter.value().toString());
					connect(mCaptionEdit, SIGNAL(textChanged()), this, SLOT(handleItemsPropertiesChange()));
				}
			}
		}

		if (mFontStyleWidget)
		{
			mFontStyleWidget->setEnabled(boldPropertiesMatch &&
				italicPropertiesMatch && underlinePropertiesMatch && strikeThroughPropertiesMatch);
		}

		if (mTextAlignmentWidget)
			mTextAlignmentWidget->setEnabled(textAlignHorizontalPropertiesMatch && textAlignVerticalPropertiesMatch);
	}
}

QWidget* DynamicPropertiesWidget::createItemsWidget()
{
	QWidget* mainWidget = new QWidget();
	QVBoxLayout* mainLayout = new QVBoxLayout();
	QGroupBox* groupBox = nullptr;
	QFormLayout* groupLayout = nullptr;

	if (mPositionWidget)
	{
		groupBox = new QGroupBox("Position");
		groupLayout = nullptr;
		addWidget(groupLayout, "Position:", mPositionWidget);
		groupBox->setLayout(groupLayout);
		mainLayout->addWidget(groupBox);
	}

	if (mStartPositionWidget || mEndPositionWidget)
	{
		groupBox = new QGroupBox("Line");
		groupLayout = nullptr;
		addWidget(groupLayout, "Start Point:", mStartPositionWidget);
		addWidget(groupLayout, "End Point:", mEndPositionWidget);
		groupBox->setLayout(groupLayout);
		mainLayout->addWidget(groupBox);
	}

	if (mCurveStartPositionWidget || mCurveStartControlPositionWidget ||
		mCurveEndPositionWidget || mCurveEndControlPositionWidget)
	{
		groupBox = new QGroupBox("Curve");
		groupLayout = nullptr;
		addWidget(groupLayout, "Start Point:", mCurveStartPositionWidget);
		addWidget(groupLayout, "Control Point:", mCurveStartControlPositionWidget);
		addWidget(groupLayout, "End Point:", mCurveEndPositionWidget);
		addWidget(groupLayout, "Control Point:", mCurveEndControlPositionWidget);
		groupBox->setLayout(groupLayout);
		mainLayout->addWidget(groupBox);
	}

	if (mRectTopLeftWidget || mRectBottomRightWidget || mCornerRadiusEdit)
	{
		DrawingEllipseItem* ellipseItem = dynamic_cast<DrawingEllipseItem*>(mItem);
		DrawingTextEllipseItem* textEllipseItem = dynamic_cast<DrawingTextEllipseItem*>(mItem);

		if (ellipseItem || textEllipseItem) groupBox = new QGroupBox("Ellipse");
		else groupBox = new QGroupBox("Rectangle");
		groupLayout = nullptr;
		addWidget(groupLayout, "Top Left:", mRectTopLeftWidget);
		addWidget(groupLayout, "Bottom Right:", mRectBottomRightWidget);
		addWidget(groupLayout, "Corner Radius:", mCornerRadiusEdit);
		groupBox->setLayout(groupLayout);
		mainLayout->addWidget(groupBox);
	}

	if (mPointPositionStackedWidget)
	{
		DrawingPolylineItem* polylineItem = dynamic_cast<DrawingPolylineItem*>(mItem);

		if (polylineItem) groupBox = new QGroupBox("Polyline");
		else groupBox = new QGroupBox("Polygon");
		QVBoxLayout* vLayout = new QVBoxLayout();
		vLayout->addWidget(mPointPositionStackedWidget);
		vLayout->setContentsMargins(0, 0, 0, 0);
		groupBox->setLayout(vLayout);
		mainLayout->addWidget(groupBox);
	}

	if (mPenStyleCombo || mPenWidthEdit || mPenColorWidget || mBrushColorWidget)
	{
		groupBox = new QGroupBox("Pen / Brush");
		groupLayout = nullptr;
		addWidget(groupLayout, "Pen Style:", mPenStyleCombo);
		addWidget(groupLayout, "Pen Width:", mPenWidthEdit);
		addWidget(groupLayout, "Pen Color:", mPenColorWidget);
		addWidget(groupLayout, "Brush Color:", mBrushColorWidget);
		groupBox->setLayout(groupLayout);
		mainLayout->addWidget(groupBox);
	}

	if (mFontComboBox || mFontSizeEdit || mFontStyleWidget || mTextAlignmentWidget ||
		mTextColorWidget || mCaptionEdit)
	{
		groupBox = new QGroupBox("Text");
		groupLayout = nullptr;
		addWidget(groupLayout, "Font:", mFontComboBox);
		addWidget(groupLayout, "Font Size:", mFontSizeEdit);
		addWidget(groupLayout, "Font Style:", mFontStyleWidget);
		addWidget(groupLayout, "Alignment:", mTextAlignmentWidget);
		addWidget(groupLayout, "Text Color:", mTextColorWidget);
		addWidget(groupLayout, "Caption:", mCaptionEdit);
		groupBox->setLayout(groupLayout);
		mainLayout->addWidget(groupBox);
	}

	if (mStartArrowCombo || mStartArrowSizeEdit || mEndArrowCombo || mEndArrowSizeEdit)
	{
		groupBox = new QGroupBox("Arrow");
		groupLayout = nullptr;
		addWidget(groupLayout, "Start Arrow:", mStartArrowCombo);
		addWidget(groupLayout, "Arrow Size:", mStartArrowSizeEdit);
		addWidget(groupLayout, "End Arrow:", mEndArrowCombo);
		addWidget(groupLayout, "Arrow Size:", mEndArrowSizeEdit);
		groupBox->setLayout(groupLayout);
		mainLayout->addWidget(groupBox);
	}

	mainLayout->addWidget(new QWidget(), 100);
	mainWidget->setLayout(mainLayout);
	return mainWidget;
}

void DynamicPropertiesWidget::addWidget(QFormLayout*& formLayout, const QString& label,	QWidget* widget)
{
	if (widget)
	{
		if (formLayout == nullptr)
		{
			formLayout = new QFormLayout();
			formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
			formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
		}

		if (mItems.size() > 1)
		{
			QCheckBox* checkBox = new QCheckBox(label);
			checkBox->setChecked(widget->isEnabled());
			connect(checkBox, SIGNAL(clicked(bool)), widget, SLOT(setEnabled(bool)));
			connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(handleItemsStyleChange()));

			formLayout->addRow(checkBox, widget);
			mItemStyleLabels[widget] = checkBox;
		}
		else formLayout->addRow(label, widget);

		if (formLayout->rowCount() == 1)
			formLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth());
	}
}

//==================================================================================================

void DynamicPropertiesWidget::fillPointsWidgets()
{
	// Clear old point position widgets
	if (mPointPositionStackedWidget)
	{
		QWidget* widget = nullptr;

		while (mPointPositionStackedWidget->count() > 0)
		{
			widget = mPointPositionStackedWidget->widget(0);
			mPointPositionStackedWidget->removeWidget(widget);
			delete widget;
		}
	}

	mPointPositionWidgets.clear();

	// Create new point position widgets
	QList<DrawingItemPoint*> points = mItem->points();
	for(int i = 0; i < points.size(); i++)
	{
		PositionWidget* posWidget = new PositionWidget(mItem->mapToScene(points[i]->position()));
		connect(posWidget, SIGNAL(positionChanged(const QPointF&)), this, SLOT(handleItemGeometryChange()));
		mPointPositionWidgets.append(posWidget);
	}

	// Assemble point position widgets into layout
	if (!mPointPositionWidgets.isEmpty())
	{
		QWidget* pointsWidget = new QWidget();
		QFormLayout* pointsLayout = nullptr;
		pointsLayout = nullptr;
		for(int i = 0; i < mPointPositionWidgets.size(); i++)
		{
			if (i == 0)
				addWidget(pointsLayout, "Start Point:", mPointPositionWidgets[i]);
			else if (i == mPointPositionWidgets.size() - 1)
				addWidget(pointsLayout, "End Point:", mPointPositionWidgets[i]);
			else
				addWidget(pointsLayout, "", mPointPositionWidgets[i]);

		}
		pointsWidget->setLayout(pointsLayout);
		mPointPositionStackedWidget->addWidget(pointsWidget);
	}
}
