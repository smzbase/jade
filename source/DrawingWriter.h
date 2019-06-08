/* DrawingWriter.h
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

#ifndef DRAWINGWRITER_H
#define DRAWINGWRITER_H

#include <QString>
#include "DrawingArrow.h"
#include "DrawingWidget.h"

class DrawingCurveItem;
class DrawingEllipseItem;
class DrawingItem;
class DrawingItemGroup;
class DrawingLineItem;
class DrawingPathItem;
class DrawingPolygonItem;
class DrawingPolylineItem;
class DrawingRectItem;
class DrawingTextItem;
class DrawingTextRectItem;
class DrawingTextEllipseItem;

class QXmlStreamWriter;
class QuaZip;

class DrawingWriter
{
private:
	QString mErrorMessage;

public:
	DrawingWriter();
	~DrawingWriter();

	void writeFile(DrawingWidget* drawing, const QString& filePath);
	QString writeDrawing(DrawingWidget* drawing);
	QString writeItems(const QList<DrawingItem*>& items);
	QString errorMessage() const;
	bool hasError() const;

private:
	void writeItemElements(QXmlStreamWriter* xml, const QList<DrawingItem*>& items);

	void writeLineItem(QXmlStreamWriter* xml, DrawingLineItem* item);
	void writeCurveItem(QXmlStreamWriter* xml, DrawingCurveItem* item);
	void writePolylineItem(QXmlStreamWriter* xml, DrawingPolylineItem* item);
	void writeRectItem(QXmlStreamWriter* xml, DrawingRectItem* item);
	void writeEllipseItem(QXmlStreamWriter* xml, DrawingEllipseItem* item);
	void writePolygonItem(QXmlStreamWriter* xml, DrawingPolygonItem* item);
	void writeTextItem(QXmlStreamWriter* xml, DrawingTextItem* item);
	void writeTextRectItem(QXmlStreamWriter* xml, DrawingTextRectItem* item);
	void writeTextEllipseItem(QXmlStreamWriter* xml, DrawingTextEllipseItem* item);
	void writePathItem(QXmlStreamWriter* xml, DrawingPathItem* item);
	void writeItemGroup(QXmlStreamWriter* xml, DrawingItemGroup* item);

	void writeItemProperties(QXmlStreamWriter* xml, DrawingItem* item);

	QString alignmentToString(Qt::Alignment align) const;
	QString arrowStyleToString(DrawingArrow::Style style) const;
	QString colorToString(const QColor& color) const;
	QString gridStyleToString(DrawingWidget::GridStyle gridStyle) const;
	QString pathToString(const QPainterPath& path) const;
	QString penStyleToString(Qt::PenStyle style) const;
	QString penCapStyleToString(Qt::PenCapStyle style) const;
	QString penJoinStyleToString(Qt::PenJoinStyle style) const;
	QString pointsToString(const QPolygonF& points) const;
	QString transformToString(DrawingItem* item) const;

	void createFileInZip(QuaZip* zip, const QString& path, const QString& content);
};

#endif
