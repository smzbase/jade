/* DrawingWriter.cpp
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

#include "DrawingWriter.h"
#include "DrawingCurveItem.h"
#include "DrawingEllipseItem.h"
#include "DrawingItemGroup.h"
#include "DrawingLineItem.h"
#include "DrawingPathItem.h"
#include "DrawingPolygonItem.h"
#include "DrawingPolylineItem.h"
#include "DrawingRectItem.h"
#include "DrawingTextItem.h"
#include "DrawingTextRectItem.h"
#include "DrawingTextEllipseItem.h"

#include <QTextStream>
#include <QXmlStreamWriter>
#include <QtMath>
#include <quazip.h>
#include <quazipfile.h>

DrawingWriter::DrawingWriter() { }

DrawingWriter::~DrawingWriter() { }

//==================================================================================================

void DrawingWriter::writeFile(DrawingWidget* drawing, const QString& filePath)
{
	mErrorMessage.clear();

	QuaZip jdmFile(filePath);

	if (jdmFile.open(QuaZip::mdCreate))
	{
		createFileInZip(&jdmFile, "drawing.xml", writeDrawing(drawing));
		jdmFile.close();
	}
	else mErrorMessage = "Error creating file: " + filePath;
}

QString DrawingWriter::writeDrawing(DrawingWidget* drawing)
{
	QString str;

	QXmlStreamWriter xml(&str);
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(2);

	mErrorMessage.clear();

	xml.writeStartDocument();
	xml.writeStartElement("jade-drawing");
	xml.writeAttribute("version", "1.1");
	xml.writeStartElement("page");

	if (drawing)
	{
		QRectF sceneRect = drawing->sceneRect();
		xml.writeAttribute("view-left", QString::number(sceneRect.left()));
		xml.writeAttribute("view-top", QString::number(sceneRect.top()));
		xml.writeAttribute("view-width", QString::number(sceneRect.width()));
		xml.writeAttribute("view-height", QString::number(sceneRect.height()));

		xml.writeAttribute("background-color", colorToString(drawing->backgroundBrush().color()));

		xml.writeAttribute("grid", QString::number(drawing->grid()));
		xml.writeAttribute("grid-color", colorToString(drawing->gridBrush().color()));
		xml.writeAttribute("grid-style", gridStyleToString(drawing->gridStyle()));
		xml.writeAttribute("grid-spacing-major", QString::number(drawing->gridSpacingMajor()));
		xml.writeAttribute("grid-spacing-minor", QString::number(drawing->gridSpacingMinor()));

		xml.writeAttribute("dynamic-grid", QString::number(drawing->dynamicGrid()));

		xml.writeStartElement("items");
		writeItemElements(&xml, drawing->items());
		xml.writeEndElement();
	}

	xml.writeEndElement();
	xml.writeEndElement();
	xml.writeEndDocument();

	return str;
}

QString DrawingWriter::writeItems(const QList<DrawingItem*>& items)
{
	QString str;

	QXmlStreamWriter xml(&str);
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(2);

	mErrorMessage.clear();

	xml.writeStartDocument();
	xml.writeStartElement("jade-items");
	xml.writeAttribute("version", "1.1");

	xml.writeStartElement("items");
	writeItemElements(&xml, items);
	xml.writeEndElement();

	xml.writeEndElement();
	xml.writeEndDocument();

	return str;
}

QString DrawingWriter::errorMessage() const
{
	return mErrorMessage;
}

bool DrawingWriter::hasError() const
{
	return !mErrorMessage.isEmpty();
}

//==================================================================================================

void DrawingWriter::writeItemElements(QXmlStreamWriter* xml, const QList<DrawingItem*>& items)
{
	for(auto itemIter = items.begin(); itemIter != items.end(); itemIter++)
	{
		DrawingLineItem* lineItem = dynamic_cast<DrawingLineItem*>(*itemIter);
		DrawingCurveItem* curveItem = dynamic_cast<DrawingCurveItem*>(*itemIter);
		DrawingPolylineItem* polylineItem = dynamic_cast<DrawingPolylineItem*>(*itemIter);
		DrawingRectItem* rectItem = dynamic_cast<DrawingRectItem*>(*itemIter);
		DrawingEllipseItem* ellipseItem = dynamic_cast<DrawingEllipseItem*>(*itemIter);
		DrawingPolygonItem* polygonItem = dynamic_cast<DrawingPolygonItem*>(*itemIter);
		DrawingTextItem* textItem = dynamic_cast<DrawingTextItem*>(*itemIter);
		DrawingTextRectItem* textRectItem = dynamic_cast<DrawingTextRectItem*>(*itemIter);
		DrawingTextEllipseItem* textEllipseItem = dynamic_cast<DrawingTextEllipseItem*>(*itemIter);
		DrawingPathItem* pathItem = dynamic_cast<DrawingPathItem*>(*itemIter);
		DrawingItemGroup* groupItem = dynamic_cast<DrawingItemGroup*>(*itemIter);

		if (lineItem) writeLineItem(xml, lineItem);
		else if (curveItem) writeCurveItem(xml, curveItem);
		else if (polylineItem) writePolylineItem(xml, polylineItem);
		else if (rectItem) writeRectItem(xml, rectItem);
		else if (ellipseItem) writeEllipseItem(xml, ellipseItem);
		else if (polygonItem) writePolygonItem(xml, polygonItem);
		else if (textItem) writeTextItem(xml, textItem);
		else if (textRectItem) writeTextRectItem(xml, textRectItem);
		else if (textEllipseItem) writeTextEllipseItem(xml, textEllipseItem);
		else if (pathItem) writePathItem(xml, pathItem);
		else if (groupItem) writeItemGroup(xml, groupItem);
	}
}

//==================================================================================================

void DrawingWriter::writeLineItem(QXmlStreamWriter* xml, DrawingLineItem* item)
{
	xml->writeStartElement("line");

	xml->writeAttribute("transform", transformToString(item));

	QLineF line = item->line();
	xml->writeAttribute("x1", QString::number(line.x1()));
	xml->writeAttribute("y1", QString::number(line.y1()));
	xml->writeAttribute("x2", QString::number(line.x2()));
	xml->writeAttribute("y2", QString::number(line.y2()));

	writeItemProperties(xml, item);

	xml->writeEndElement();
}

void DrawingWriter::writeCurveItem(QXmlStreamWriter* xml, DrawingCurveItem* item)
{
	xml->writeStartElement("curve");

	xml->writeAttribute("transform", transformToString(item));

	xml->writeAttribute("x1", QString::number(item->curveStartPos().x()));
	xml->writeAttribute("y1", QString::number(item->curveStartPos().y()));
	xml->writeAttribute("cx1", QString::number(item->curveStartControlPos().x()));
	xml->writeAttribute("cy1", QString::number(item->curveStartControlPos().y()));
	xml->writeAttribute("cx2", QString::number(item->curveEndControlPos().x()));
	xml->writeAttribute("cy2", QString::number(item->curveEndControlPos().y()));
	xml->writeAttribute("x2", QString::number(item->curveEndPos().x()));
	xml->writeAttribute("y2", QString::number(item->curveEndPos().y()));

	writeItemProperties(xml, item);

	xml->writeEndElement();
}

void DrawingWriter::writePolylineItem(QXmlStreamWriter* xml, DrawingPolylineItem* item)
{
	xml->writeStartElement("polyline");

	xml->writeAttribute("transform", transformToString(item));

	xml->writeAttribute("points", pointsToString(item->polyline()));

	writeItemProperties(xml, item);

	xml->writeEndElement();
}

void DrawingWriter::writeRectItem(QXmlStreamWriter* xml, DrawingRectItem* item)
{
	xml->writeStartElement("rect");

	xml->writeAttribute("transform", transformToString(item));

	QRectF rect = item->rect();
	xml->writeAttribute("left", QString::number(rect.left()));
	xml->writeAttribute("top", QString::number(rect.top()));
	xml->writeAttribute("width", QString::number(rect.width()));
	xml->writeAttribute("height", QString::number(rect.height()));

	if (item->cornerRadius() != 0) xml->writeAttribute("r", QString::number(item->cornerRadius()));

	writeItemProperties(xml, item);

	xml->writeEndElement();
}

void DrawingWriter::writeEllipseItem(QXmlStreamWriter* xml, DrawingEllipseItem* item)
{
	xml->writeStartElement("ellipse");

	xml->writeAttribute("transform", transformToString(item));

	QRectF rect = item->ellipse();
	xml->writeAttribute("left", QString::number(rect.left()));
	xml->writeAttribute("top", QString::number(rect.top()));
	xml->writeAttribute("width", QString::number(rect.width()));
	xml->writeAttribute("height", QString::number(rect.height()));

	writeItemProperties(xml, item);

	xml->writeEndElement();
}

void DrawingWriter::writePolygonItem(QXmlStreamWriter* xml, DrawingPolygonItem* item)
{
	xml->writeStartElement("polygon");

	xml->writeAttribute("transform", transformToString(item));

	xml->writeAttribute("points", pointsToString(item->polygon()));

	writeItemProperties(xml, item);

	xml->writeEndElement();
}

void DrawingWriter::writeTextItem(QXmlStreamWriter* xml, DrawingTextItem* item)
{
	xml->writeStartElement("text");

	xml->writeAttribute("transform", transformToString(item));

	writeItemProperties(xml, item);

	xml->writeCharacters(item->caption());

	xml->writeEndElement();
}

void DrawingWriter::writeTextRectItem(QXmlStreamWriter* xml, DrawingTextRectItem* item)
{
	xml->writeStartElement("text-rect");

	xml->writeAttribute("transform", transformToString(item));

	QRectF rect = item->rect();
	xml->writeAttribute("left", QString::number(rect.left()));
	xml->writeAttribute("top", QString::number(rect.top()));
	xml->writeAttribute("width", QString::number(rect.width()));
	xml->writeAttribute("height", QString::number(rect.height()));

	if (item->cornerRadius() != 0) xml->writeAttribute("r", QString::number(item->cornerRadius()));

	writeItemProperties(xml, item);

	xml->writeCharacters(item->caption());

	xml->writeEndElement();
}

void DrawingWriter::writeTextEllipseItem(QXmlStreamWriter* xml, DrawingTextEllipseItem* item)
{
	xml->writeStartElement("text-ellipse");

	xml->writeAttribute("transform", transformToString(item));

	QRectF rect = item->ellipse();
	xml->writeAttribute("left", QString::number(rect.left()));
	xml->writeAttribute("top", QString::number(rect.top()));
	xml->writeAttribute("width", QString::number(rect.width()));
	xml->writeAttribute("height", QString::number(rect.height()));

	writeItemProperties(xml, item);

	xml->writeCharacters(item->caption());

	xml->writeEndElement();
}

void DrawingWriter::writePathItem(QXmlStreamWriter* xml, DrawingPathItem* item)
{
	xml->writeStartElement("path");

	xml->writeAttribute("name", item->name());

	xml->writeAttribute("transform", transformToString(item));

	QRectF rect = item->rect();
	xml->writeAttribute("left", QString::number(rect.left()));
	xml->writeAttribute("top", QString::number(rect.top()));
	xml->writeAttribute("width", QString::number(rect.width()));
	xml->writeAttribute("height", QString::number(rect.height()));

	writeItemProperties(xml, item);

	QRectF pathRect = item->pathRect();
	xml->writeAttribute("view-left", QString::number(pathRect.left()));
	xml->writeAttribute("view-top", QString::number(pathRect.top()));
	xml->writeAttribute("view-width", QString::number(pathRect.width()));
	xml->writeAttribute("view-height", QString::number(pathRect.height()));

	xml->writeAttribute("d", pathToString(item->path()));

	QString connectionStr = pointsToString(item->connectionPoints());
	if (!connectionStr.isEmpty()) xml->writeAttribute("connection-points", connectionStr);

	xml->writeEndElement();
}

void DrawingWriter::writeItemGroup(QXmlStreamWriter* xml, DrawingItemGroup* item)
{
	xml->writeStartElement("group");

	xml->writeAttribute("transform", transformToString(item));

	writeItemElements(xml, item->items());

	xml->writeEndElement();
}

//==================================================================================================

void DrawingWriter::writeItemProperties(QXmlStreamWriter* xml, DrawingItem* item)
{
	QHash<QString,QVariant> properties = item->properties();

	if (properties.contains("pen-style"))
	{
		bool ok = false;
		uint value = properties["pen-style"].toUInt(&ok);
		if (ok && value != Qt::SolidLine) xml->writeAttribute("pen-style", penStyleToString(static_cast<Qt::PenStyle>(value)));
	}

	if (properties.contains("pen-color"))
	{
		QColor color = properties["pen-color"].value<QColor>();
		xml->writeAttribute("pen-color", colorToString(color));
		if (color.alphaF() != 1.0) xml->writeAttribute("pen-opacity", QString::number(color.alphaF()));
	}

	if (properties.contains("pen-width"))
	{
		bool ok = false;
		qreal value = properties["pen-width"].toDouble(&ok);
		if (ok) xml->writeAttribute("pen-width", QString::number(value));
	}

	if (properties.contains("brush-color"))
	{
		QColor color = properties["brush-color"].value<QColor>();
		xml->writeAttribute("brush-color", colorToString(color));
		if (color.alphaF() != 1.0) xml->writeAttribute("brush-opacity", QString::number(color.alphaF()));
	}

	if (properties.contains("font-family"))
		xml->writeAttribute("font-family", properties["font-family"].toString());

	if (properties.contains("font-size"))
	{
		bool ok = false;
		qreal value = properties["font-size"].toDouble(&ok);
		if (ok) xml->writeAttribute("font-size", QString::number(value));
	}

	if (properties.contains("font-bold") && properties["font-bold"].toBool())
		xml->writeAttribute("font-bold", "true");

	if (properties.contains("font-italic") && properties["font-italic"].toBool())
		xml->writeAttribute("font-italic", "true");

	if (properties.contains("font-underline") && properties["font-underline"].toBool())
		xml->writeAttribute("font-underline", "true");

	if (properties.contains("font-strike-through") && properties["font-strike-through"].toBool())
		xml->writeAttribute("font-strike-through", "true");

	// Text
	if (properties.contains("text-color"))
	{
		QColor color = properties["text-color"].value<QColor>();
		xml->writeAttribute("text-color", colorToString(color));
		if (color.alphaF() != 1.0) xml->writeAttribute("text-opacity", QString::number(color.alphaF()));
	}

	if (properties.contains("text-alignment-horizontal"))
	{
		bool ok = false;
		uint value = properties["text-alignment-horizontal"].toUInt(&ok);
		if (ok) xml->writeAttribute("text-alignment-horizontal", alignmentToString(static_cast<Qt::Alignment>(value)));
	}

	if (properties.contains("text-alignment-vertical"))
	{
		bool ok = false;
		uint value = properties["text-alignment-vertical"].toUInt(&ok);
		if (ok) xml->writeAttribute("text-alignment-vertical", alignmentToString(static_cast<Qt::Alignment>(value)));
	}

	// Arrows
	if (properties.contains("start-arrow-style"))
	{
		bool ok = false;
		uint value = properties["start-arrow-style"].toUInt(&ok);
		if (ok) xml->writeAttribute("start-arrow-style", arrowStyleToString(static_cast<DrawingArrow::Style>(value)));
	}

	if (properties.contains("start-arrow-size"))
	{
		bool ok = false;
		qreal value = properties["start-arrow-size"].toDouble(&ok);
		if (ok) xml->writeAttribute("start-arrow-size", QString::number(value));
	}

	if (properties.contains("end-arrow-style"))
	{
		bool ok = false;
		uint value = properties["end-arrow-style"].toUInt(&ok);
		if (ok) xml->writeAttribute("end-arrow-style", arrowStyleToString(static_cast<DrawingArrow::Style>(value)));
	}

	if (properties.contains("end-arrow-size"))
	{
		bool ok = false;
		qreal value = properties["end-arrow-size"].toDouble(&ok);
		if (ok) xml->writeAttribute("end-arrow-size", QString::number(value));
	}
}

//==================================================================================================

QString DrawingWriter::alignmentToString(Qt::Alignment align) const
{
	QString str;

	if (align & Qt::AlignLeft) str = "left";
	else if (align & Qt::AlignRight) str = "right";
	else if (align & Qt::AlignHCenter) str = "center";
	else if (align & Qt::AlignTop) str = "top";
	else if (align & Qt::AlignBottom) str = "bottom";
	else if (align & Qt::AlignVCenter) str = "middle";

	return str;
}

QString DrawingWriter::arrowStyleToString(DrawingArrow::Style style) const
{
	QString str;

	switch (style)
	{
	case DrawingArrow::Normal: str = "normal"; break;
	case DrawingArrow::Triangle: str = "triangle"; break;
	case DrawingArrow::TriangleFilled: str = "triangle-filled"; break;
	case DrawingArrow::Circle: str = "circle"; break;
	case DrawingArrow::CircleFilled: str = "circle-filled"; break;
	case DrawingArrow::Concave: str = "concave"; break;
	case DrawingArrow::ConcaveFilled: str = "concave-filled"; break;
	default: str = "none"; break;
	}

	return str;
}

QString DrawingWriter::colorToString(const QColor& color) const
{
	QString str = "#";

	if (color.red() < 16) str += "0";
	str += QString::number(color.red(), 16).toLower();
	if (color.green() < 16) str += "0";
	str += QString::number(color.green(), 16).toLower();
	if (color.blue() < 16) str += "0";
	str += QString::number(color.blue(), 16).toLower();

	return str;
}

QString DrawingWriter::gridStyleToString(DrawingWidget::GridStyle gridStyle) const
{
	QString str;

	switch (gridStyle)
	{
	case DrawingWidget::GridDots: str = "dotted"; break;
	case DrawingWidget::GridLines: str = "lined"; break;
	case DrawingWidget::GridGraphPaper: str = "graph-paper"; break;
	default: str = "none"; break;
	}

	return str;
}

QString DrawingWriter::pathToString(const QPainterPath& path) const
{
	QString pathStr;

	for(int i = 0; i < path.elementCount(); i++)
	{
		QPainterPath::Element element = path.elementAt(i);

		switch (element.type)
		{
		case QPainterPath::MoveToElement:
			pathStr += "M " + QString::number(element.x) + " " + QString::number(element.y) + " ";
			break;
		case QPainterPath::LineToElement:
			pathStr += "L " + QString::number(element.x) + " " + QString::number(element.y) + " ";
			break;
		case QPainterPath::CurveToElement:
			pathStr += "C " + QString::number(element.x) + " " + QString::number(element.y) + " ";
			break;
		case QPainterPath::CurveToDataElement:
			pathStr += QString::number(element.x) + " " + QString::number(element.y) + " ";
			break;
		}
	}

	return pathStr.trimmed();
}

QString DrawingWriter::penStyleToString(Qt::PenStyle style) const
{
	QString str;

	switch (style)
	{
	case Qt::NoPen: str = "none"; break;
	case Qt::DashLine: str = "dash"; break;
	case Qt::DotLine: str = "dot"; break;
	case Qt::DashDotLine: str = "dash-dot"; break;
	case Qt::DashDotDotLine: str = "dash-dot-dot"; break;
	default: str = "solid"; break;
	}

	return str;
}

QString DrawingWriter::penCapStyleToString(Qt::PenCapStyle style) const
{
	QString str;

	switch (style)
	{
	case Qt::FlatCap: str = "flat"; break;
	case Qt::SquareCap: str = "square"; break;
	default: str = "round"; break;
	}

	return str;
}

QString DrawingWriter::penJoinStyleToString(Qt::PenJoinStyle style) const
{
	QString str;

	switch (style)
	{
	case Qt::SvgMiterJoin:
	case Qt::MiterJoin: str = "miter"; break;
	case Qt::BevelJoin: str = "bevel"; break;
	default: str = "round"; break;
	}

	return str;
}

QString DrawingWriter::pointsToString(const QPolygonF& points) const
{
	QString pointsStr;

	for(auto pointIter = points.begin(); pointIter != points.end(); pointIter++)
		pointsStr += QString::number((*pointIter).x()) + "," + QString::number((*pointIter).y()) + " ";

	return pointsStr.trimmed();
}

QString DrawingWriter::transformToString(DrawingItem* item) const
{
	QPointF pos = item->position();
	QTransform transform = item->transform();

	qreal rotation = qAsin(transform.m12()) * 180 / 3.141592654;
	transform.rotate(-rotation);

	qreal hScale = transform.m11();
	qreal vScale = transform.m22();

	QString str = "translate(" + QString::number(pos.x()) + "," + QString::number(pos.y()) + ")";
	str += (rotation != 0) ? " rotate(" + QString::number(rotation) + ")" : "";
	str += (hScale != 1.0 || vScale != 1.0) ? " scale(" + QString::number(hScale) + "," + QString::number(vScale) + ")" : "";
	return str;
}

//==================================================================================================

void DrawingWriter::createFileInZip(QuaZip* zip, const QString& path, const QString& content)
{
	QuaZipFile outputFile(zip);

	if (outputFile.open(QIODevice::WriteOnly, QuaZipNewInfo(path)))
	{
		QTextStream outputStream(&outputFile);
		outputStream << content;
		outputStream.flush();
		outputFile.close();
	}
	else mErrorMessage = "Error creating " + path + " in file: " + zip->getZipName();
}
