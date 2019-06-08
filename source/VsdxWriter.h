/* VsdxWriter.h
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

#ifndef VSDXWRITER_H
#define VSDXWRITER_H

#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QPolygonF>

class DrawingItem;
class DrawingCurveItem;
class DrawingEllipseItem;
class DrawingItemGroup;
class DrawingLineItem;
class DrawingPathItem;
class DrawingPolygonItem;
class DrawingPolylineItem;
class DrawingRectItem;
class DrawingTextEllipseItem;
class DrawingTextItem;
class DrawingTextRectItem;
class DrawingWidget;

class QuaZip;

class VsdxWriter
{
private:
	QString mErrorMessage;

	DrawingWidget* mDrawing;
	QString mFilePath;
	QString mUnits;
	qreal mScale;

	QPointF mTranslate;
	qreal mVsdxWidth, mVsdxHeight;
	qreal mVsdxMargin;

public:
	VsdxWriter();
	~VsdxWriter();

	bool write(DrawingWidget* drawing, const QString& filePath, const QString& units, qreal scale);
	QString errorMessage() const;
	bool hasError() const;

private:
	void writeVsdx();
	QString writeContentTypes();
	QString writeRels();
	QString writeApp();
	QString writeCore();
	QString writeCustom();
	QString writePagesRels();
	QString writePages();
	QString writePage1();
	QString writeDocumentRels();
	QString writeDocument();
	QString writeWindows();
	void createFileInZip(QuaZip* zip, const QString& path, const QString& content);

	QString writeItems(const QList<DrawingItem*>& items);
	QString writeLineItem(DrawingLineItem* item, int& index);
	QString writeCurveItem(DrawingCurveItem* item, int& index);
	QString writePolylineItem(DrawingPolylineItem* item, int& index);
	QString writeRectItem(DrawingRectItem* item, int& index);
	QString writeEllipseItem(DrawingEllipseItem* item, int& index);
	QString writePolygonItem(DrawingPolygonItem* item, int& index);
	QString writeTextItem(DrawingTextItem* item, int& index);
	QString writeTextRectItem(DrawingTextRectItem* item, int& index);
	QString writeTextEllipseItem(DrawingTextEllipseItem* item, int& index);
	QString writePathItem(DrawingPathItem* item, int& index);
	QString writeItemGroup(DrawingItemGroup* item, int& index);

	QString writeItemProperties(DrawingItem* item);

	QPointF mapFromScene(const QPointF& pos) const;
	QRectF mapFromScene(const QRectF& rect) const;
	QPolygonF mapFromScene(const QPolygonF& poly) const;
	QString colorToHexString(const QColor& color) const;
};

#endif
