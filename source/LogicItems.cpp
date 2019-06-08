/* LogicItems.cpp
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

#include "LogicItems.h"
#include "DrawingPathItem.h"

QList<DrawingPathItem*> LogicItems::items()
{
	QList<DrawingPathItem*> items;

	items << createAndGate() <<
		createOrGate() <<
		createXorGate() <<
		createXnorGate() <<
		createNandGate() <<
		createNorGate() <<
		createMultiplexer() <<
		createDemultiplexer() <<
		createBuffer() <<
		createNotGate() <<
		createTristateBuffer1() <<
		createTristateBuffer2() <<
		createFlipFlop1() <<
		createFlipFlop2();

	return items;
}

QStringList LogicItems::icons()
{
	QStringList icons;

	icons << ":/icons/items/and_gate.png" <<
		":/icons/items/or_gate.png" <<
		":/icons/items/xor_gate.png" <<
		":/icons/items/xnor_gate.png" <<
		":/icons/items/nand_gate.png" <<
		":/icons/items/nor_gate.png" <<
		":/icons/items/multiplexer.png" <<
		":/icons/items/demultiplexer.png" <<
		":/icons/items/buffer.png" <<
		":/icons/items/not_gate.png" <<
		":/icons/items/tristate_buffer.png" <<
		":/icons/items/tristate_buffer2.png" <<
		":/icons/items/flip_flop.png" <<
		":/icons/items/flip_flop2.png";

	return icons;
}

//==================================================================================================

DrawingPathItem* LogicItems::createAndGate()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -125, 500, 250);
	QPainterPath path;
	path.moveTo(-187.5, -125);
	path.cubicTo(312.5, -125, 312.5, 125, -187.5, 125);
	//path.lineTo(-187.5, -125);

	//path.moveTo(-250, -62.5); path.lineTo(-187.5, -62.5);
	//path.moveTo(-250, 62.5); path.lineTo(-187.5, 62.5);
	//path.moveTo(187.5, 0); path.lineTo(250, 0);

	item->setName("AND Gate");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -62.5));
	item->addConnectionPoint(QPointF(-250, 62.5));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createOrGate()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -125, 500, 250);
	QPainterPath path;
	path.moveTo(-187.5, -125);
	path.cubicTo(312.5, -125, 312.5, 125, -187.5, 125);
	path.cubicTo(-87.5, 125, -87.5, -125, -187.5, -125);

	path.moveTo(-250, -62.5); path.lineTo(-125, -62.5);
	path.moveTo(-250, 62.5); path.lineTo(-125, 62.5);
	path.moveTo(187.5, 0); path.lineTo(250, 0);

	item->setName("OR Gate");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -62.5));
	item->addConnectionPoint(QPointF(-250, 62.5));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createXorGate()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -125, 500, 250);
	QPainterPath path;
	path.moveTo(-187.5, -125);
	path.cubicTo(312.5, -125, 312.5, 125, -187.5, 125);
	path.cubicTo(-87.5, 125, -87.5, -125, -187.5, -125);

	path.moveTo(-250, -125);
	path.cubicTo(-150, -125, -150, 125, -250, 125);

	path.moveTo(-250, -62.5); path.lineTo(-125, -62.5);
	path.moveTo(-250, 62.5); path.lineTo(-125, 62.5);
	path.moveTo(187.5, 0); path.lineTo(250, 0);

	item->setName("XOR Gate");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -62.5));
	item->addConnectionPoint(QPointF(-250, 62.5));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createXnorGate()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -125, 500, 250);
	QPainterPath path;
	path.moveTo(-187.5, -125);
	path.cubicTo(312.5, -125, 312.5, 125, -187.5, 125);
	path.cubicTo(-87.5, 125, -87.5, -125, -187.5, -125);

	path.moveTo(-250, -125);
	path.cubicTo(-150, -125, -150, 125, -250, 125);

	path.moveTo(-250, -62.5); path.lineTo(-125, -62.5);
	path.moveTo(-250, 62.5); path.lineTo(-125, 62.5);
	path.moveTo(237.5, 0); path.lineTo(250, 0);

	path.moveTo(237.5, 0);
	path.cubicTo(237.5, -13.8071, 226.3075, -25, 212.5, -25);
	path.cubicTo(198.6925, -25, 187.5, -13.8071, 187.5, 0);
	path.cubicTo(187.5, 13.8071, 198.6925, 25, 212.5, 25);
	path.cubicTo(226.3075, 25, 237.5, 13.8071, 237.5, 0);

	item->setName("XNOR Gate");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -62.5));
	item->addConnectionPoint(QPointF(-250, 62.5));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createNandGate()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -125, 500, 250);
	QPainterPath path;
	path.moveTo(-187.5, -125);
	path.cubicTo(312.5, -125, 312.5, 125, -187.5, 125);
	path.lineTo(-187.5, -125);

	path.moveTo(-250, -62.5); path.lineTo(-187.5, -62.5);
	path.moveTo(-250, 62.5); path.lineTo(-187.5, 62.5);
	path.moveTo(237.5, 0); path.lineTo(250, 0);

	path.moveTo(237.5, 0);
	path.cubicTo(237.5, -13.8071, 226.3075, -25, 212.5, -25);
	path.cubicTo(198.6925, -25, 187.5, -13.8071, 187.5, 0);
	path.cubicTo(187.5, 13.8071, 198.6925, 25, 212.5, 25);
	path.cubicTo(226.3075, 25, 237.5, 13.8071, 237.5, 0);

	item->setName("NAND Gate");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -62.5));
	item->addConnectionPoint(QPointF(-250, 62.5));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createNorGate()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -125, 500, 250);
	QPainterPath path;
	path.moveTo(-187.5, -125);
	path.cubicTo(312.5, -125, 312.5, 125, -187.5, 125);
	path.cubicTo(-87.5, 125, -87.5, -125, -187.5, -125);

	path.moveTo(-250, -62.5); path.lineTo(-125, -62.5);
	path.moveTo(-250, 62.5); path.lineTo(-125, 62.5);
	path.moveTo(237.5, 0); path.lineTo(250, 0);

	path.moveTo(237.5, 0);
	path.cubicTo(237.5, -13.8071, 226.3075, -25, 212.5, -25);
	path.cubicTo(198.6925, -25, 187.5, -13.8071, 187.5, 0);
	path.cubicTo(187.5, 13.8071, 198.6925, 25, 212.5, 25);
	path.cubicTo(226.3075, 25, 237.5, 13.8071, 237.5, 0);

	item->setName("NOR Gate");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -62.5));
	item->addConnectionPoint(QPointF(-250, 62.5));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createMultiplexer()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(-125, -250);
	path.lineTo(-125, 250);
	path.lineTo(125, 125);
	path.lineTo(125, -125);
	path.lineTo(-125, -250);

	path.moveTo(-250, -125); path.lineTo(-125, -125);
	path.moveTo(-250, 125); path.lineTo(-125, 125);
	path.moveTo(125, 0); path.lineTo(250, 0);

	item->setName("Multiplexer");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -125));
	item->addConnectionPoint(QPointF(-250, 125));
	item->addConnectionPoint(QPointF(250, 0));
	item->addConnectionPoint(QPointF(0, -187.5));
	item->addConnectionPoint(QPointF(0, 187.5));

	return item;
}

DrawingPathItem* LogicItems::createDemultiplexer()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(125, -250);
	path.lineTo(125, 250);
	path.lineTo(-125, 125);
	path.lineTo(-125, -125);
	path.lineTo(125, -250);

	path.moveTo(250, -125); path.lineTo(125, -125);
	path.moveTo(250, 125); path.lineTo(125, 125);
	path.moveTo(-125, 0); path.lineTo(-250, 0);

	item->setName("Demultiplexer");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(250, -125));
	item->addConnectionPoint(QPointF(250, 125));
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(0, -187.5));
	item->addConnectionPoint(QPointF(0, 187.5));

	return item;
}

DrawingPathItem* LogicItems::createBuffer()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -93.75, 500, 187.5);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(250, 0);

	item->setName("Buffer");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createNotGate()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -93.75, 500, 187.5);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(112.5, 0); path.lineTo(250, 0);

	path.moveTo(112.5, 0);
	path.cubicTo(112.5, -13.8071, 101.3075, -25, 87.5, -25);
	path.cubicTo(73.6925, -25, 62.5, -13.8071, 62.5, 0);
	path.cubicTo(62.5, 13.8071, 73.6925, 25, 87.5, 25);
	path.cubicTo(101.3075, 25, 112.5, 13.8071, 112.5, 0);

	item->setName("NOT Gate");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* LogicItems::createTristateBuffer1()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -93.75, 500, 187.5);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(250, 0);

	path.moveTo(0, -46.875); path.lineTo(0, -93.75);

	item->setName("Tristate Buffer 1");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));
	item->addConnectionPoint(QPointF(0, -93.75));

	return item;
}

DrawingPathItem* LogicItems::createTristateBuffer2()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -93.75, 500, 187.5);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(112.5, 0); path.lineTo(250, 0);

	path.moveTo(112.5, 0);
	path.cubicTo(112.5, -13.8071, 101.3075, -25, 87.5, -25);
	path.cubicTo(73.6925, -25, 62.5, -13.8071, 62.5, 0);
	path.cubicTo(62.5, 13.8071, 73.6925, 25, 87.5, 25);
	path.cubicTo(101.3075, 25, 112.5, 13.8071, 112.5, 0);

	path.moveTo(0, -46.875); path.lineTo(0, -93.75);

	item->setName("Tristate Buffer 2");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));
	item->addConnectionPoint(QPointF(0, -93.75));

	return item;
}

DrawingPathItem* LogicItems::createFlipFlop1()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(-187.5, -250); path.lineTo(-187.5, 250); path.lineTo(187.5, 250); path.lineTo(187.5, -250); path.lineTo(-187.5, -250);

	path.moveTo(-250, -125); path.lineTo(-187.5, -125);
	path.moveTo(-250, 125); path.lineTo(-187.5, 125);
	path.moveTo(250, -125); path.lineTo(187.5, -125);
	path.moveTo(250, 125); path.lineTo(187.5, 125);

	item->setName("Flip-Flop 1");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -125));
	item->addConnectionPoint(QPointF(-250, 125));
	item->addConnectionPoint(QPointF(0, -250));
	item->addConnectionPoint(QPointF(0, 250));
	item->addConnectionPoint(QPointF(250, -125));
	item->addConnectionPoint(QPointF(250, 125));

	return item;
}

DrawingPathItem* LogicItems::createFlipFlop2()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(-187.5, -250); path.lineTo(-187.5, 250); path.lineTo(187.5, 250); path.lineTo(187.5, -250); path.lineTo(-187.5, -250);

	path.moveTo(-250, -125); path.lineTo(-187.5, -125);
	path.moveTo(-250, 125); path.lineTo(-187.5, 125);
	path.moveTo(250, -125); path.lineTo(187.5, -125);
	path.moveTo(250, 125); path.lineTo(187.5, 125);

	path.moveTo(-187.5, 62.5); path.lineTo(-125, 125);
	path.moveTo(-125, 125); path.lineTo(-187.5, 187.5);

	item->setName("Flip-Flop 2");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, -125));
	item->addConnectionPoint(QPointF(-250, 125));
	item->addConnectionPoint(QPointF(0, -250));
	item->addConnectionPoint(QPointF(0, 250));
	item->addConnectionPoint(QPointF(250, -125));
	item->addConnectionPoint(QPointF(250, 125));

	return item;
}
