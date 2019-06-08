/* DrawingReader.h
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

#ifndef DRAWINGREADER_H
#define DRAWINGREADER_H

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

class QXmlStreamReader;
class QuaZip;

class DrawingReader
{
private:
	QString mErrorMessage;

public:
	DrawingReader();
	~DrawingReader();

	void readFile(const QString& filePath, DrawingWidget* drawing);
	void readDrawing(const QString& str, DrawingWidget* drawing);
	QList<DrawingItem*> readItems(const QString& str);
	QString errorMessage() const;
	bool hasError() const;

private:
	QList<DrawingItem*> readItemElements(QXmlStreamReader* xml);

	DrawingLineItem* readLineItem(QXmlStreamReader* xml);
	DrawingCurveItem* readCurveItem(QXmlStreamReader* xml);
	DrawingPolylineItem* readPolylineItem(QXmlStreamReader* xml);
	DrawingRectItem* readRectItem(QXmlStreamReader* xml);
	DrawingEllipseItem* readEllipseItem(QXmlStreamReader* xml);
	DrawingPolygonItem* readPolygonItem(QXmlStreamReader* xml);
	DrawingTextItem* readTextItem(QXmlStreamReader* xml);
	DrawingTextRectItem* readTextRectItem(QXmlStreamReader* xml);
	DrawingTextEllipseItem* readTextEllipseItem(QXmlStreamReader* xml);
	DrawingPathItem* readPathItem(QXmlStreamReader* xml);
	DrawingItemGroup* readItemGroup(QXmlStreamReader* xml);

	void readItemProperties(QXmlStreamReader* xml, DrawingItem* item);

	Qt::Alignment alignmentFromString(const QString& str) const;
	DrawingArrow::Style arrowStyleFromString(const QString& str) const;
	QColor colorFromString(const QString& str) const;
	DrawingWidget::GridStyle gridStyleFromString(const QString& str) const;
	QPainterPath pathFromString(const QString& str) const;
	Qt::PenStyle penStyleFromString(const QString& str) const;
	Qt::PenCapStyle penCapStyleFromString(const QString& str) const;
	Qt::PenJoinStyle penJoinStyleFromString(const QString& str) const;
	QPolygonF pointsFromString(const QString& str) const;
	void transformFromString(const QString& str, DrawingItem* item);

	QString readFileFromZip(QuaZip* zip, const QString& path);
};

#endif
