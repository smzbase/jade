/* DrawingReader.cpp
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

#include "DrawingReader.h"
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

#include <QTextStream>
#include <QXmlStreamReader>
#include <QtMath>
#include <quazip.h>
#include <quazipfile.h>

DrawingReader::DrawingReader() { }

DrawingReader::~DrawingReader() { }

//==================================================================================================

void DrawingReader::readFile(const QString& filePath, DrawingWidget* drawing)
{
	mErrorMessage.clear();

	QuaZip jdmFile(filePath);

	if (jdmFile.open(QuaZip::mdUnzip))
	{
		readDrawing(readFileFromZip(&jdmFile, "drawing.xml"), drawing);
		jdmFile.close();
	}
	else mErrorMessage = "Error creating file: " + filePath;
}

void DrawingReader::readDrawing(const QString& str, DrawingWidget* drawing)
{
	QXmlStreamReader xml(str);

	mErrorMessage.clear();

	QList<DrawingItem*> items;
	bool pageFound = false;

	if (drawing)
	{
		while (xml.readNextStartElement())
		{
			if (xml.name() == "jade-drawing")
			{
				while (xml.readNextStartElement())
				{
					if (xml.name() == "page" && !pageFound)
					{
						// Read scene properties
						QXmlStreamAttributes attr = xml.attributes();

						QRectF sceneRect = drawing->sceneRect();
						if (attr.hasAttribute("view-left")) sceneRect.setLeft(attr.value("view-left").toDouble());
						if (attr.hasAttribute("view-top")) sceneRect.setTop(attr.value("view-top").toDouble());
						if (attr.hasAttribute("view-width")) sceneRect.setWidth(attr.value("view-width").toDouble());
						if (attr.hasAttribute("view-height")) sceneRect.setHeight(attr.value("view-height").toDouble());
						drawing->setSceneRect(sceneRect);

						if (attr.hasAttribute("background-color"))
							drawing->setBackgroundBrush(colorFromString(attr.value("background-color").toString()));

						if (attr.hasAttribute("grid"))
							drawing->setGrid(attr.value("grid").toDouble());

						if (attr.hasAttribute("grid-color"))
							drawing->setGridBrush(colorFromString(attr.value("grid-color").toString()));
						if (attr.hasAttribute("grid-style"))
							drawing->setGridStyle(gridStyleFromString(attr.value("grid-style").toString()));
						if (attr.hasAttribute("grid-spacing-major"))
							drawing->setGridSpacing(attr.value("grid-spacing-major").toInt(), drawing->gridSpacingMinor());
						if (attr.hasAttribute("grid-spacing-minor"))
							drawing->setGridSpacing(drawing->gridSpacingMajor(), attr.value("grid-spacing-minor").toInt());

						if (attr.hasAttribute("dynamic-grid"))
							drawing->setDynamicGrid(attr.value("dynamic-grid").toDouble());

						// Read items
						while (xml.readNextStartElement())
						{
							if (xml.name() == "items")
							{
								items = readItemElements(&xml);
								for(auto itemIter = items.begin(); itemIter != items.end(); itemIter++)
									drawing->addItem(*itemIter);
							}
							else xml.skipCurrentElement();
						}

						pageFound = true;
					}
					else xml.skipCurrentElement();
				}
			}
			else xml.skipCurrentElement();
		}
	}
}

QList<DrawingItem*> DrawingReader::readItems(const QString& str)
{
	QList<DrawingItem*> items;

	QXmlStreamReader xml(str);

	mErrorMessage.clear();

	while (xml.readNextStartElement())
	{
		if (xml.name() == "jade-items")
		{
			while (xml.readNextStartElement())
			{
				if (xml.name() == "items") items.append(readItemElements(&xml));
				else xml.skipCurrentElement();
			}
		}
		else xml.skipCurrentElement();
	}

	return items;
}

QString DrawingReader::errorMessage() const
{
	return mErrorMessage;
}

bool DrawingReader::hasError() const
{
	return !mErrorMessage.isEmpty();
}

//==================================================================================================

QList<DrawingItem*> DrawingReader::readItemElements(QXmlStreamReader* xml)
{
	QList<DrawingItem*> items;

	DrawingItem* newItem = nullptr;
	QString itemName;

	while (xml->readNextStartElement())
	{
		newItem = nullptr;
		itemName = xml->name().toString();

		if (itemName == "line") newItem = readLineItem(xml);
		else if (itemName == "curve") newItem = readCurveItem(xml);
		else if (itemName == "polyline") newItem = readPolylineItem(xml);
		else if (itemName == "rect") newItem = readRectItem(xml);
		else if (itemName == "ellipse") newItem = readEllipseItem(xml);
		else if (itemName == "polygon") newItem = readPolygonItem(xml);
		else if (itemName == "text") newItem = readTextItem(xml);
		else if (itemName == "text-rect") newItem = readTextRectItem(xml);
		else if (itemName == "text-ellipse") newItem = readTextEllipseItem(xml);
		else if (itemName == "path") newItem = readPathItem(xml);
		else if (itemName == "group") newItem = readItemGroup(xml);
		else xml->skipCurrentElement();

		if (newItem) items.append(newItem);
	}

	// Connect items together
	QList<DrawingItemPoint*> itemPoints, otherItemPoints;
	qreal distance, threshold = 0.01;
	QPointF vec;
	for(auto itemIter = items.begin(); itemIter != items.end(); itemIter++)
	{
		for(auto otherItemIter = itemIter + 1; otherItemIter != items.end(); otherItemIter++)
		{
			itemPoints = (*itemIter)->points();
			otherItemPoints = (*otherItemIter)->points();

			for(auto itemPointIter = itemPoints.begin(); itemPointIter != itemPoints.end(); itemPointIter++)
			{
				for(auto otherItemPointIter = otherItemPoints.begin();
					otherItemPointIter != otherItemPoints.end(); otherItemPointIter++)
				{
					if (((*itemPointIter)->flags() & DrawingItemPoint::Connection) && ((*otherItemPointIter)->flags() & DrawingItemPoint::Connection) &&
						(((*itemPointIter)->flags() & DrawingItemPoint::Free) || ((*otherItemPointIter)->flags() & DrawingItemPoint::Free)))
					{
						vec = (*itemIter)->mapToScene((*itemPointIter)->position()) -
							(*otherItemIter)->mapToScene((*otherItemPointIter)->position());
						distance = qSqrt(vec.x() * vec.x() + vec.y() * vec.y());

						if (distance <= threshold)
						{
							(*itemPointIter)->addConnection(*otherItemPointIter);
							(*otherItemPointIter)->addConnection(*itemPointIter);
						}
					}
				}
			}
		}
	}

	return items;
}

//==================================================================================================

DrawingLineItem* DrawingReader::readLineItem(QXmlStreamReader* xml)
{
	DrawingLineItem* item = new DrawingLineItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	QLineF line = item->line();
	QPointF p1 = line.p1();
	QPointF p2 = line.p2();
	if (attr.hasAttribute("x1")) p1.setX(attr.value("x1").toDouble());
	if (attr.hasAttribute("y1")) p1.setY(attr.value("y1").toDouble());
	if (attr.hasAttribute("x2")) p2.setX(attr.value("x2").toDouble());
	if (attr.hasAttribute("y2")) p2.setY(attr.value("y2").toDouble());
	item->setLine(QLineF(p1, p2));

	readItemProperties(xml, item);

	xml->skipCurrentElement();

	return item;
}

DrawingCurveItem* DrawingReader::readCurveItem(QXmlStreamReader* xml)
{
	DrawingCurveItem* item = new DrawingCurveItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	QPointF p1 = item->curveStartPos(), p2 = item->curveEndPos();
	QPointF cp1 = item->curveStartControlPos(), cp2 = item->curveEndControlPos();
	if (attr.hasAttribute("x1")) p1.setX(attr.value("x1").toDouble());
	if (attr.hasAttribute("y1")) p1.setY(attr.value("y1").toDouble());
	if (attr.hasAttribute("cx1")) cp1.setX(attr.value("cx1").toDouble());
	if (attr.hasAttribute("cy1")) cp1.setY(attr.value("cy1").toDouble());
	if (attr.hasAttribute("cx2")) cp2.setX(attr.value("cx2").toDouble());
	if (attr.hasAttribute("cy2")) cp2.setY(attr.value("cy2").toDouble());
	if (attr.hasAttribute("x2")) p2.setX(attr.value("x2").toDouble());
	if (attr.hasAttribute("y2")) p2.setY(attr.value("y2").toDouble());
	item->setCurve(p1, cp1, cp2, p2);

	readItemProperties(xml, item);

	xml->skipCurrentElement();

	return item;
}

DrawingPolylineItem* DrawingReader::readPolylineItem(QXmlStreamReader* xml)
{
	DrawingPolylineItem* item = new DrawingPolylineItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	if (attr.hasAttribute("points"))
		item->setPolyline(pointsFromString(attr.value("points").toString()));

	readItemProperties(xml, item);

	xml->skipCurrentElement();

	return item;
}

DrawingRectItem* DrawingReader::readRectItem(QXmlStreamReader* xml)
{
	DrawingRectItem* item = new DrawingRectItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	QRectF rect = item->rect();
	if (attr.hasAttribute("left")) rect.setLeft(attr.value("left").toDouble());
	if (attr.hasAttribute("top")) rect.setTop(attr.value("top").toDouble());
	if (attr.hasAttribute("width")) rect.setWidth(attr.value("width").toDouble());
	if (attr.hasAttribute("height")) rect.setHeight(attr.value("height").toDouble());
	item->setRect(rect);

	if (attr.hasAttribute("r")) item->setCornerRadius(attr.value("r").toDouble());

	readItemProperties(xml, item);

	xml->skipCurrentElement();

	return item;
}

DrawingEllipseItem* DrawingReader::readEllipseItem(QXmlStreamReader* xml)
{
	DrawingEllipseItem* item = new DrawingEllipseItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	QRectF rect = item->ellipse();
	if (attr.hasAttribute("left")) rect.setLeft(attr.value("left").toDouble());
	if (attr.hasAttribute("top")) rect.setTop(attr.value("top").toDouble());
	if (attr.hasAttribute("width")) rect.setWidth(attr.value("width").toDouble());
	if (attr.hasAttribute("height")) rect.setHeight(attr.value("height").toDouble());
	item->setEllipse(rect);

	readItemProperties(xml, item);

	xml->skipCurrentElement();

	return item;
}

DrawingPolygonItem* DrawingReader::readPolygonItem(QXmlStreamReader* xml)
{
	DrawingPolygonItem* item = new DrawingPolygonItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	if (attr.hasAttribute("points"))
		item->setPolygon(pointsFromString(attr.value("points").toString()));

	readItemProperties(xml, item);

	xml->skipCurrentElement();

	return item;
}

DrawingTextItem* DrawingReader::readTextItem(QXmlStreamReader* xml)
{
	DrawingTextItem* item = new DrawingTextItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	readItemProperties(xml, item);

	if (xml->readNext() == QXmlStreamReader::Characters)
	{
		item->setCaption(xml->text().toString());
		xml->skipCurrentElement();
	}

	return item;
}

DrawingTextRectItem* DrawingReader::readTextRectItem(QXmlStreamReader* xml)
{
	DrawingTextRectItem* item = new DrawingTextRectItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	QRectF rect = item->rect();
	if (attr.hasAttribute("left")) rect.setLeft(attr.value("left").toDouble());
	if (attr.hasAttribute("top")) rect.setTop(attr.value("top").toDouble());
	if (attr.hasAttribute("width")) rect.setWidth(attr.value("width").toDouble());
	if (attr.hasAttribute("height")) rect.setHeight(attr.value("height").toDouble());
	item->setRect(rect);

	if (attr.hasAttribute("r")) item->setCornerRadius(attr.value("r").toDouble());

	readItemProperties(xml, item);

	if (xml->readNext() == QXmlStreamReader::Characters)
	{
		item->setCaption(xml->text().toString());
		xml->skipCurrentElement();
	}

	return item;
}

DrawingTextEllipseItem* DrawingReader::readTextEllipseItem(QXmlStreamReader* xml)
{
	DrawingTextEllipseItem* item = new DrawingTextEllipseItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	QRectF rect = item->ellipse();
	if (attr.hasAttribute("left")) rect.setLeft(attr.value("left").toDouble());
	if (attr.hasAttribute("top")) rect.setTop(attr.value("top").toDouble());
	if (attr.hasAttribute("width")) rect.setWidth(attr.value("width").toDouble());
	if (attr.hasAttribute("height")) rect.setHeight(attr.value("height").toDouble());
	item->setEllipse(rect);

	readItemProperties(xml, item);

	if (xml->readNext() == QXmlStreamReader::Characters)
	{
		item->setCaption(xml->text().toString());
		xml->skipCurrentElement();
	}

	return item;
}

DrawingPathItem* DrawingReader::readPathItem(QXmlStreamReader* xml)
{
	DrawingPathItem* item = new DrawingPathItem();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("name")) item->setName(attr.value("name").toString());

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	QRectF pathRect = item->pathRect();
	if (attr.hasAttribute("view-left")) pathRect.setLeft(attr.value("view-left").toDouble());
	if (attr.hasAttribute("view-top")) pathRect.setTop(attr.value("view-top").toDouble());
	if (attr.hasAttribute("view-width")) pathRect.setWidth(attr.value("view-width").toDouble());
	if (attr.hasAttribute("view-height")) pathRect.setHeight(attr.value("view-height").toDouble());
	item->setPath(item->path(), pathRect);

	if (attr.hasAttribute("d"))
		item->setPath(pathFromString(attr.value("d").toString()), pathRect);

	QRectF rect = item->rect();
	if (attr.hasAttribute("left")) rect.setLeft(attr.value("left").toDouble());
	if (attr.hasAttribute("top")) rect.setTop(attr.value("top").toDouble());
	if (attr.hasAttribute("width")) rect.setWidth(attr.value("width").toDouble());
	if (attr.hasAttribute("height")) rect.setHeight(attr.value("height").toDouble());
	item->setRect(rect);

	if (attr.hasAttribute("connection-points"))
		item->addConnectionPoints(pointsFromString(attr.value("connection-points").toString()));

	readItemProperties(xml, item);

	xml->skipCurrentElement();

	return item;
}

DrawingItemGroup* DrawingReader::readItemGroup(QXmlStreamReader* xml)
{
	DrawingItemGroup* item = new DrawingItemGroup();
	QXmlStreamAttributes attr = xml->attributes();

	if (attr.hasAttribute("transform")) transformFromString(attr.value("transform").toString(), item);

	item->setItems(readItemElements(xml));

	return item;
}

//==================================================================================================

void DrawingReader::readItemProperties(QXmlStreamReader* xml, DrawingItem* item)
{
	QXmlStreamAttributes attr = xml->attributes();

	QHash<QString,QVariant> properties = item->properties();

	// Pen
	if (properties.contains("pen-style"))
	{
		Qt::PenStyle penStyle = Qt::SolidLine;

		if (attr.hasAttribute("pen-style"))
		{
			bool ok = false;
			uint value = attr.value("pen-style").toUInt(&ok);
			if (ok) penStyle = static_cast<Qt::PenStyle>(value);
		}

		properties["pen-style"] = static_cast<uint>(penStyle);
	}

	if (properties.contains("pen-color"))
	{
		QColor color(0, 0, 0);

		if (attr.hasAttribute("pen-color"))
			color = colorFromString(attr.value("pen-color").toString());
		if (attr.hasAttribute("pen-opacity"))
		{
			bool ok = false;
			qreal opacity = attr.value("pen-opacity").toDouble(&ok);
			if (ok) color.setAlphaF(opacity);
		}

		properties["pen-color"] = color;
	}

	if (properties.contains("pen-width"))
	{
		qreal penWidth = 1.0;

		if (attr.hasAttribute("pen-width"))
		{
			bool ok = false;
			qreal value = attr.value("pen-width").toDouble(&ok);
			if (ok) penWidth = value;
		}

		properties["pen-width"] = penWidth;
	}

	// Brush
	if (properties.contains("brush-color"))
	{
		QColor color(255, 255, 255);

		if (attr.hasAttribute("brush-color"))
			color = colorFromString(attr.value("brush-color").toString());
		if (attr.hasAttribute("brush-opacity"))
		{
			bool ok = false;
			qreal opacity = attr.value("brush-opacity").toDouble(&ok);
			if (ok) color.setAlphaF(opacity);
		}

		properties["brush-color"] = color;
	}

	// Font
	if (properties.contains("font-family"))
	{
		QString name = "Arial";

		if (attr.hasAttribute("font-family"))
			name = attr.value("font-family").toString();

		properties["font-family"] = name;
	}

	if (properties.contains("font-size"))
	{
		qreal size = 1.0;

		if (attr.hasAttribute("font-size"))
		{
			bool ok = false;
			qreal value = attr.value("font-size").toDouble(&ok);
			if (ok) size = value;
		}

		properties["font-size"] = size;
	}

	if (properties.contains("font-bold"))
	{
		bool value = false;

		if (attr.hasAttribute("font-bold"))
			value = (attr.value("font-bold").toString().toLower() == "true");

		properties["font-bold"] = value;
	}

	if (properties.contains("font-italic"))
	{
		bool value = false;

		if (attr.hasAttribute("font-italic"))
			value = (attr.value("font-italic").toString().toLower() == "true");

		properties["font-italic"] = value;
	}

	if (properties.contains("font-underline"))
	{
		bool value = false;

		if (attr.hasAttribute("font-underline"))
			value = (attr.value("font-underline").toString().toLower() == "true");

		properties["font-underline"] = value;
	}

	if (properties.contains("font-strike-through"))
	{
		bool value = false;

		if (attr.hasAttribute("font-strike-through"))
			value = (attr.value("font-strike-through").toString().toLower() == "true");

		properties["font-strike-through"] = value;
	}

	// Text
	if (properties.contains("text-color"))
	{
		QColor color(0, 0, 0);

		if (attr.hasAttribute("text-color"))
			color = colorFromString(attr.value("text-color").toString());
		if (attr.hasAttribute("text-opacity"))
		{
			bool ok = false;
			qreal opacity = attr.value("text-opacity").toDouble(&ok);
			if (ok) color.setAlphaF(opacity);
		}

		properties["text-color"] = color;
	}

	if (properties.contains("text-alignment-horizontal"))
	{
		Qt::Alignment align = Qt::AlignHCenter;

		if (attr.hasAttribute("text-alignment-horizontal"))
		{
			bool ok = false;
			uint value = attr.value("text-alignment-horizontal").toUInt(&ok);
			if (ok) align = static_cast<Qt::Alignment>(value);
		}

		properties["text-alignment-horizontal"] = static_cast<uint>(align);
	}

	if (properties.contains("text-alignment-vertical"))
	{
		Qt::Alignment align = Qt::AlignVCenter;

		if (attr.hasAttribute("text-alignment-vertical"))
		{
			bool ok = false;
			uint value = attr.value("text-alignment-vertical").toUInt(&ok);
			if (ok) align = static_cast<Qt::Alignment>(value);
		}

		properties["text-alignment-vertical"] = static_cast<uint>(align);
	}

	// Arrows
	if (properties.contains("start-arrow-style"))
	{
		DrawingArrow::Style style = DrawingArrow::None;

		if (attr.hasAttribute("start-arrow-style"))
		{
			bool ok = false;
			uint value = attr.value("start-arrow-style").toUInt(&ok);
			if (ok) style = static_cast<DrawingArrow::Style>(value);
		}

		properties["start-arrow-style"] = static_cast<uint>(style);
	}

	if (properties.contains("start-arrow-size"))
	{
		qreal size = 0.0;

		if (attr.hasAttribute("start-arrow-size"))
		{
			bool ok = false;
			qreal value = attr.value("start-arrow-size").toDouble(&ok);
			if (ok) size = value;
		}

		properties["start-arrow-size"] = size;
	}

	if (properties.contains("end-arrow-style"))
	{
		DrawingArrow::Style style = DrawingArrow::None;

		if (attr.hasAttribute("end-arrow-style"))
		{
			bool ok = false;
			uint value = attr.value("end-arrow-style").toUInt(&ok);
			if (ok) style = static_cast<DrawingArrow::Style>(value);
		}

		properties["end-arrow-style"] = static_cast<uint>(style);
	}

	if (properties.contains("end-arrow-size"))
	{
		qreal size = 0.0;

		if (attr.hasAttribute("end-arrow-size"))
		{
			bool ok = false;
			qreal value = attr.value("end-arrow-size").toDouble(&ok);
			if (ok) size = value;
		}

		properties["end-arrow-size"] = size;
	}
}

//==================================================================================================

Qt::Alignment DrawingReader::alignmentFromString(const QString& str) const
{
	Qt::Alignment align;

	if (str == "left") align = Qt::AlignLeft;
	else if (str == "right") align = Qt::AlignRight;
	else if (str == "center") align = Qt::AlignHCenter;
	else if (str == "top") align = Qt::AlignTop;
	else if (str == "bottom") align = Qt::AlignBottom;
	else if (str == "middle") align = Qt::AlignVCenter;

	return align;
}

DrawingArrow::Style DrawingReader::arrowStyleFromString(const QString& str) const
{
	DrawingArrow::Style style = DrawingArrow::None;

	if (str == "normal") style = DrawingArrow::Normal;
	else if (str == "triangle") style = DrawingArrow::Triangle;
	else if (str == "triangle-filled") style = DrawingArrow::TriangleFilled;
	else if (str == "circle") style = DrawingArrow::Circle;
	else if (str == "circle-filled") style = DrawingArrow::CircleFilled;
	else if (str == "concave") style = DrawingArrow::Concave;
	else if (str == "concave-filled") style = DrawingArrow::ConcaveFilled;

	return style;
}

QColor DrawingReader::colorFromString(const QString& str) const
{
	QColor color;

	color.setRed(str.mid(1,2).toUInt(nullptr, 16));
	color.setGreen(str.mid(3,2).toUInt(nullptr, 16));
	color.setBlue(str.mid(5,2).toUInt(nullptr, 16));

	return color;
}

DrawingWidget::GridStyle DrawingReader::gridStyleFromString(const QString& str) const
{
	DrawingWidget::GridStyle style = DrawingWidget::GridNone;

	if (str == "dotted") style = DrawingWidget::GridDots;
	else if (str == "lined") style = DrawingWidget::GridLines;
	else if (str == "graph-paper") style = DrawingWidget::GridGraphPaper;

	return style;
}

QPainterPath DrawingReader::pathFromString(const QString& str) const
{
	QPainterPath path;

	QStringList tokenList = str.split(" ", QString::SkipEmptyParts);
	qreal x, y, x1, y1, x2, y2;
	bool xOk = false, yOk = false, x1Ok = false, x2Ok = false, y1Ok = false, y2Ok = false;

	for(int i = 0; i < tokenList.size(); i++)
	{
		if (tokenList[i] == "M" && i+2 < tokenList.size())
		{
			x = tokenList[i+1].toDouble(&xOk);
			y = tokenList[i+2].toDouble(&yOk);
			if (xOk && yOk) path.moveTo(x, y);
		}
		else if (tokenList[i] == "L" && i+2 < tokenList.size())
		{
			x = tokenList[i+1].toDouble(&xOk);
			y = tokenList[i+2].toDouble(&yOk);
			if (xOk && yOk) path.lineTo(x, y);
		}
		else if (tokenList[i] == "C" && i+6 < tokenList.size())
		{
			x1 = tokenList[i+1].toDouble(&x1Ok);
			y1 = tokenList[i+2].toDouble(&y1Ok);
			x2 = tokenList[i+3].toDouble(&x2Ok);
			y2 = tokenList[i+4].toDouble(&y2Ok);
			x = tokenList[i+5].toDouble(&xOk);
			y = tokenList[i+6].toDouble(&yOk);
			if (x1Ok && y1Ok && x2Ok && y2Ok && xOk && yOk) path.cubicTo(x1, y1, x2, y2, x, y);
		}
	}

	return path;
}

Qt::PenStyle DrawingReader::penStyleFromString(const QString& str) const
{
	Qt::PenStyle style = Qt::SolidLine;

	if (str == "none") style = Qt::NoPen;
	else if (str == "dash") style = Qt::DashLine;
	else if (str == "dot") style = Qt::DotLine;
	else if (str == "dash-dot") style = Qt::DashDotLine;
	else if (str == "dash-dot-dot") style = Qt::DashDotDotLine;

	return style;
}

Qt::PenCapStyle DrawingReader::penCapStyleFromString(const QString& str) const
{
	Qt::PenCapStyle style = Qt::RoundCap;

	if (str == "flat") style = Qt::FlatCap;
	else if (str == "square") style = Qt::SquareCap;

	return style;
}

Qt::PenJoinStyle DrawingReader::penJoinStyleFromString(const QString& str) const
{
	Qt::PenJoinStyle style = Qt::RoundJoin;

	if (str == "miter") style = Qt::MiterJoin;
	else if (str == "bevel") style = Qt::BevelJoin;

	return style;
}

QPolygonF DrawingReader::pointsFromString(const QString& str) const
{
	QPolygonF points;

	QStringList tokenList = str.split(" ");
	QStringList coordList;
	qreal x, y;
	bool xOk = false, yOk = false;

	for(int i = 0; i < tokenList.size(); i++)
	{
		coordList = tokenList[i].split(",");
		if (coordList.size() == 2)
		{
			x = coordList[0].toDouble(&xOk);
			y = coordList[1].toDouble(&yOk);
			if (xOk && yOk) points.append(QPointF(x, y));
		}
	}

	return points;
}

void DrawingReader::transformFromString(const QString& str, DrawingItem* item)
{
	QStringList tokens = str.split(QRegExp("\\s+"));
	for(auto tokenIter = tokens.begin(); tokenIter != tokens.end(); tokenIter++)
	{
		if (tokenIter->startsWith("translate("))
		{
			int endIndex = tokenIter->indexOf(")");
			QStringList coords = tokenIter->mid(10, endIndex - 10).split(",");

			if (coords.size() == 2)
			{
				item->setX(coords.first().toDouble());
				item->setY(coords.last().toDouble());
			}
		}
		else if (tokenIter->startsWith("rotate("))
		{
			int endIndex = tokenIter->indexOf(")");
			QString angle = tokenIter->mid(7, endIndex - 7);

			QTransform transform;
			transform.rotate(angle.toDouble());
			item->setTransform(transform, true);
		}
		else if (tokenIter->startsWith("scale("))
		{
			int endIndex = tokenIter->indexOf(")");
			QStringList coords = tokenIter->mid(6, endIndex - 6).split(",");
			if (coords.size() == 2)
			{
				QTransform transform;
				transform.scale(coords.first().toDouble(), coords.last().toDouble());
				item->setTransform(transform, true);
			}
		}
	}
}

//==================================================================================================

QString DrawingReader::readFileFromZip(QuaZip* zip, const QString& path)
{
	QString content;
	QuaZipFile inputFile(zip);

	zip->setCurrentFile(path);

	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream inputStream(&inputFile);
		content = inputStream.readAll();
		inputStream.flush();
		inputFile.close();
	}
	else mErrorMessage = "Error reading " + path + " in file: " + zip->getZipName();

	return content;
}
