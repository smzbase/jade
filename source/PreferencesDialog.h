/* PreferencesDialog.h
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

class ColorWidget;
class GridStyleCombo;
class PositionWidget;
class SizeEdit;
class SizeWidget;

class QComboBox;
class QDialogButtonBox;
class QListWidget;
class QLineEdit;
class QStackedWidget;

class PreferencesDialog : public QDialog
{
	Q_OBJECT

private:
	QListWidget* mListWidget;
	QStackedWidget* mStackedWidget;

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

	QLineEdit* mExportPngSvgScaleEdit;
	QComboBox* mExportVsdxUnitsCombo;
	QLineEdit* mExportVsdxScaleEdit;

public:
	PreferencesDialog(QWidget* parent = nullptr);
	~PreferencesDialog();

	void setDrawingProperties(const QHash<QString,QVariant>& properties);
	QHash<QString,QVariant> drawingProperties() const;

	void setExportProperties(qreal pngSvgScale, const QString& vsdxUnits, qreal vsdxScale);
	qreal exportPngSvgScale() const;
	QString exportVsdxUnits() const;
	qreal exportVsdxScale() const;

private slots:
	void handleGridStyleComboChange();
	void handleDynamicGridComboChange();

private:
	void setupDrawingDefaultsWidget();
	void setupExportWidget();

	QDialogButtonBox* createButtonBox();
};


#endif
