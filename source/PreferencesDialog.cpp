/* PreferencesDialog.cpp
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

#include "PreferencesDialog.h"
#include "DrawingWidget.h"
#include "HelperWidgets.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>

PreferencesDialog::PreferencesDialog(QWidget* parent) : QDialog(parent)
{
	QFontMetrics fontMetrics(font());
	int width = fontMetrics.width("Drawing Defaults") + 24;
	int height = fontMetrics.height();

	mListWidget = new QListWidget();
	mStackedWidget = new QStackedWidget();

	mListWidget->setIconSize(QSize(2 * height, 2 * height));
	mListWidget->setGridSize(QSize(width, 4 * height));
	mListWidget->setViewMode(QListView::IconMode);
	mListWidget->setMovement(QListView::Static);
	mListWidget->setFixedWidth(width + 4);
	mListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	connect(mListWidget, SIGNAL(currentRowChanged(int)), mStackedWidget, SLOT(setCurrentIndex(int)));

	setupDrawingDefaultsWidget();
	setupExportWidget();

	QWidget *widget = new QWidget();
	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addWidget(mListWidget);
	hLayout->addWidget(mStackedWidget, 100);
	hLayout->setSpacing(8);
	hLayout->setContentsMargins(0, 0, 0, 0);
	widget->setLayout(hLayout);

	QVBoxLayout* vLayout = new QVBoxLayout();
	vLayout->addWidget(widget, 100);
	vLayout->addWidget(createButtonBox());
	setLayout(vLayout);

	setWindowTitle("Preferences");
	resize(460, 420);
}

PreferencesDialog::~PreferencesDialog() { }

//==================================================================================================

void PreferencesDialog::setDrawingProperties(const QHash<QString,QVariant>& properties)
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
		}
	}

	handleGridStyleComboChange();
	handleDynamicGridComboChange();
}

QHash<QString,QVariant> PreferencesDialog::drawingProperties() const
{
	QHash<QString,QVariant> properties;
	bool dynamicGridEnable = (mDrawingDynamicGridCombo->currentIndex() == 0);

	mDrawingGridSpacingMinorWidget->setEnabled(mDrawingGridStyleCombo->currentIndex() == 3);

	properties["scene-rect"] = QRectF(mDrawingTopLeftWidget->position(), mDrawingRectSizeWidget->size());
	properties["background-color"] = mDrawingBackgroundColorWidget->color();
	properties["grid"] = mDrawingGridEdit->size();
	properties["grid-style"] = static_cast<uint>(mDrawingGridStyleCombo->style());
	properties["grid-color"] = mDrawingGridColorWidget->color();
	properties["grid-spacing-major"] = mDrawingGridSpacingMajorWidget->text().toInt();
	properties["grid-spacing-minor"] = mDrawingGridSpacingMinorWidget->text().toInt();

	if (dynamicGridEnable)
	{
		bool ok = false;
		qreal grid = mDrawingDynamicGridEdit->text().toDouble(&ok);
		properties["dynamic-grid"] = (ok) ? grid : 0;
	}
	else properties["dynamic-grid"] = 0;

	return properties;
}

//==================================================================================================

void PreferencesDialog::setExportProperties(qreal pngSvgScale, const QString& vsdxUnits, qreal vsdxScale)
{
	mExportPngSvgScaleEdit->setText(QString::number(pngSvgScale));
	mExportVsdxUnitsCombo->setCurrentIndex((vsdxUnits.toLower() == "mm") ? 1 : 0);
	mExportVsdxScaleEdit->setText(QString::number(vsdxScale));
}

qreal PreferencesDialog::exportPngSvgScale() const
{
	bool ok = false;
	qreal value = mExportPngSvgScaleEdit->text().toDouble(&ok);
	return (ok) ? value : 5;
}

QString PreferencesDialog::exportVsdxUnits() const
{
	return (mExportVsdxUnitsCombo->currentIndex() == 1) ? "mm" : "in";
}

qreal PreferencesDialog::exportVsdxScale() const
{
	bool ok = false;
	qreal value = mExportVsdxScaleEdit->text().toDouble(&ok);
	return (ok) ? value : 1000;
}

//==================================================================================================

void PreferencesDialog::handleGridStyleComboChange()
{
	mDrawingGridSpacingMinorWidget->setEnabled(mDrawingGridStyleCombo->currentIndex() == 3);
}

void PreferencesDialog::handleDynamicGridComboChange()
{
	bool enable = (mDrawingDynamicGridCombo->currentIndex() == 0);
	mDrawingGridEdit->setEnabled(!enable);
	mDrawingDynamicGridEdit->setEnabled(enable);
}

//==================================================================================================

void PreferencesDialog::setupDrawingDefaultsWidget()
{
	QWidget* drawingPropertiesWidget = new QWidget();
	QVBoxLayout* drawingPropertiesLayout = new QVBoxLayout();
	QGroupBox* groupBox;
	QFormLayout* groupLayout;

	int labelWidth = QFontMetrics(font()).width("Background Color:") + 8;

	// Create widgets
	mDrawingTopLeftWidget = new PositionWidget();
	mDrawingRectSizeWidget = new SizeWidget(QSizeF(10000, 7500));
	mDrawingBackgroundColorWidget = new ColorWidget();
	mDrawingGridEdit = new SizeEdit();
	mDrawingGridStyleCombo = new GridStyleCombo();
	mDrawingGridColorWidget = new ColorWidget();
	mDrawingGridSpacingMajorWidget = new QLineEdit();
	mDrawingGridSpacingMinorWidget = new QLineEdit();

	connect(mDrawingGridStyleCombo, SIGNAL(activated(int)), this, SLOT(handleGridStyleComboChange()));
	mDrawingGridSpacingMajorWidget->setValidator(new QIntValidator(1, 1E6));
	mDrawingGridSpacingMinorWidget->setValidator(new QIntValidator(1, 1E6));

	mDrawingDynamicGridCombo = new QComboBox();
	mDrawingDynamicGridCombo->addItem("Enabled");
	mDrawingDynamicGridCombo->addItem("Disabled");
	connect(mDrawingDynamicGridCombo, SIGNAL(activated(int)), this, SLOT(handleDynamicGridComboChange()));

	mDrawingDynamicGridEdit = new SizeEdit();

	// Assemble layout
	groupBox = new QGroupBox("Diagram");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Top Left:", mDrawingTopLeftWidget);
	groupLayout->addRow("Size:", mDrawingRectSizeWidget);
	groupLayout->addRow("Background Color:", mDrawingBackgroundColorWidget);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth);
	groupBox->setLayout(groupLayout);
	drawingPropertiesLayout->addWidget(groupBox);

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
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth);
	groupLayout->addRow("Dynamic Grid:", mDrawingDynamicGridCombo);
	groupLayout->addRow("Dynamic Grid Value:", mDrawingDynamicGridEdit);
	groupBox->setLayout(groupLayout);
	drawingPropertiesLayout->addWidget(groupBox);

	drawingPropertiesLayout->addWidget(new QWidget(), 100);
	drawingPropertiesLayout->setContentsMargins(0, 0, 0, 0);
	drawingPropertiesWidget->setLayout(drawingPropertiesLayout);

	QListWidgetItem *item = new QListWidgetItem("Diagram Defaults");
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item->setTextAlignment(Qt::AlignCenter);
	item->setIcon(QIcon(":/icons/jade/diagram.png"));
	mListWidget->addItem(item);
	mStackedWidget->addWidget(drawingPropertiesWidget);
}

void PreferencesDialog::setupExportWidget()
{
	QWidget* exportPropertiesWidget = new QWidget();
	QVBoxLayout* exportPropertiesLayout = new QVBoxLayout();
	QGroupBox* groupBox;
	QFormLayout* groupLayout;

	int labelWidth = QFontMetrics(font()).width("Background Color:") + 8;

	// Create widgets
	mExportPngSvgScaleEdit = new QLineEdit();
	mExportPngSvgScaleEdit->setValidator(new QDoubleValidator());

	mExportVsdxUnitsCombo = new QComboBox();
	mExportVsdxUnitsCombo->addItem("Inches");
	mExportVsdxUnitsCombo->addItem("Millimeters");

	mExportVsdxScaleEdit = new QLineEdit();
	mExportVsdxScaleEdit->setValidator(new QDoubleValidator());

	// Assemble layout
	groupBox = new QGroupBox("PNG / SVG Export");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Scale:", mExportPngSvgScaleEdit);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth);
	groupBox->setLayout(groupLayout);
	exportPropertiesLayout->addWidget(groupBox);

	groupBox = new QGroupBox("VSDX Export");
	groupLayout = new QFormLayout();
	groupLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	groupLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	groupLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	groupLayout->addRow("Units:", mExportVsdxUnitsCombo);
	groupLayout->addRow("Scale:", mExportVsdxScaleEdit);
	groupLayout->itemAt(0, QFormLayout::LabelRole)->widget()->setMinimumWidth(labelWidth);
	groupBox->setLayout(groupLayout);
	exportPropertiesLayout->addWidget(groupBox);

	exportPropertiesLayout->addWidget(new QWidget(), 100);
	exportPropertiesLayout->setContentsMargins(0, 0, 0, 0);
	exportPropertiesWidget->setLayout(exportPropertiesLayout);

	QListWidgetItem *item = new QListWidgetItem("Export Settings");
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item->setTextAlignment(Qt::AlignCenter);
	item->setIcon(QIcon(":/icons/oxygen/document-export.png"));
	mListWidget->addItem(item);
	mStackedWidget->addWidget(exportPropertiesWidget);
}

//==================================================================================================

QDialogButtonBox* PreferencesDialog::createButtonBox()
{
	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->setCenterButtons(true);

	QPushButton* okButton = buttonBox->addButton("OK", QDialogButtonBox::AcceptRole);
	QPushButton* cancelButton = buttonBox->addButton("Cancel", QDialogButtonBox::RejectRole);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	okButton->setMinimumSize(72, 28);
	cancelButton->setMinimumSize(72, 28);

	return buttonBox;
}
