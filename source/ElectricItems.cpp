/* ElectricItems.cpp
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

#include "ElectricItems.h"
#include "DrawingPathItem.h"

QList<DrawingPathItem*> ElectricItems::items()
{
	QList<DrawingPathItem*> items;

	items << createResistor1() <<
		createResistor2() <<
		createCapacitor1() <<
		createCapacitor2() <<
		createInductor1() <<
		createDiode() <<
		createZenerDiode() <<
		createSchottkyDiode() <<
		createNpnBjt() <<
		createPnpBjt() <<
		createNmosFet() <<
		createPmosFet() <<
		createGround1() <<
		createGround2() <<
		createOpAmp() <<
		createLed() <<
		createVdc() <<
		createVac() <<
		createIdc() <<
		createIac();

	return items;
}

QStringList ElectricItems::icons()
{
	QStringList icons;

	icons << ":/icons/items/resistor1.png" <<
		":/icons/items/resistor2.png" <<
		":/icons/items/capacitor1.png" <<
		":/icons/items/capacitor2.png" <<
		":/icons/items/inductor1.png" <<
		":/icons/items/diode.png" <<
		":/icons/items/zener_diode.png" <<
		":/icons/items/schottky_diode.png" <<
		":/icons/items/npn_bjt.png" <<
		":/icons/items/pnp_bjt.png" <<
		":/icons/items/nmos_fet.png" <<
		":/icons/items/pmos_fet.png" <<
		":/icons/items/ground1.png" <<
		":/icons/items/ground2.png" <<
		":/icons/items/opamp.png" <<
		":/icons/items/led.png" <<
		":/icons/items/vdc.png" <<
		":/icons/items/vac.png" <<
		":/icons/items/idc.png" <<
		":/icons/items/iac.png";

	return icons;
}

//==================================================================================================

DrawingPathItem* ElectricItems::createResistor1()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -62.5, 500, 125);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-187.5, 0);
	path.moveTo(-187.5, 0); path.lineTo(-156.25, -62.5);
	path.moveTo(-156.25, -62.5); path.lineTo(-93.75, 62.5);
	path.moveTo(-93.75, 62.5); path.lineTo(-31.25, -62.5);
	path.moveTo(-31.25, -62.5); path.lineTo(31.25, 62.5);
	path.moveTo(31.25, 62.5); path.lineTo(93.75, -62.5);
	path.moveTo(93.75, -62.5); path.lineTo(156.25, 62.5);
	path.moveTo(156.25, 62.5); path.lineTo(187.5, 0);
	path.moveTo(187.5, 0); path.lineTo(250, 0);

	item->setName("Resistor 1");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* ElectricItems::createResistor2()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -62.5, 500, 125);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-187.5, 0);
	path.moveTo(-187.5, -62.5); path.lineTo(187.5, -62.5);
	path.moveTo(-187.5, 62.5); path.lineTo(187.5, 62.5);
	path.moveTo(-187.5, -62.5); path.lineTo(-187.5, 62.5);
	path.moveTo(187.5, -62.5); path.lineTo(187.5, 62.5);
	path.moveTo(187.5, 0); path.lineTo(250, 0);

	item->setName("Resistor 2");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}
DrawingPathItem* ElectricItems::createCapacitor1()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -62.5, 500, 125);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-31.25, 0);
	path.moveTo(-31.25, -93.75); path.lineTo(-31.25, 93.75);
	path.moveTo(31.25, -93.75); path.lineTo(31.25, 93.75);
	path.moveTo(31.25, 0); path.lineTo(250, 0);

	item->setName("Capacitor 1");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* ElectricItems::createCapacitor2()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -62.5, 500, 125);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-31.25, 0);
	path.moveTo(-31.25, -93.75); path.lineTo(-31.25, 93.75);
	path.moveTo(56.25, -93.75); path.cubicTo(6.25, -75, 6.25, 75, 56.25, 93.75);
	path.moveTo(31.25, 0); path.lineTo(250, 0);

	item->setName("Capacitor 2");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* ElectricItems::createInductor1()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-375, -93.75, 750, 93.75);
	QPainterPath path;
	path.moveTo(-375, 0); path.lineTo(-300, 0);
	path.moveTo(-300, 0); path.cubicTo(-300, -53.0268, -266.4213, -93.75, -225, -93.75);
	path.moveTo(-225, -93.75); path.cubicTo(-183.5788, -93.75, -150, -41.4214, -150, 0);
	path.moveTo(-150, 0); path.cubicTo(-150, -53.0268, -116.4214, -93.75, -75, -93.75);
	path.moveTo(-75, -93.75); path.cubicTo(-33.5786, -93.75, 0, -41.4214, 0, 0);
	path.moveTo(0, 0); path.cubicTo(0, -53.0268, 33.5786, -93.75, 75, -93.75);
	path.moveTo(75, -93.75); path.cubicTo(116.4214, -93.75, 150, -41.4214, 150, 0);
	path.moveTo(150, 0); path.cubicTo(150, -53.0268, 183.5788, -93.75, 225, -93.75);
	path.moveTo(225, -93.75); path.cubicTo(266.4213, -93.75, 300, -41.4214, 300, 0);
	path.moveTo(300, 0); path.lineTo(375, 0);

	item->setName("Inductor 1");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-375, 0));
	item->addConnectionPoint(QPointF(375, 0));

	return item;
}

DrawingPathItem* ElectricItems::createDiode()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -93.75, 500, 187.5);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(62.5, -93.75); path.lineTo(62.5, 93.75);
	path.moveTo(62.5, 0); path.lineTo(250, 0);

	item->setName("Diode");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* ElectricItems::createZenerDiode()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -156.25, 500, 312.5);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(125, -156.25); path.lineTo(62.5, -93.75);
	path.moveTo(62.5, -93.75); path.lineTo(62.5, 93.75);
	path.moveTo(62.5, 93.75); path.lineTo(0, 156.25);
	path.moveTo(62.5, 0); path.lineTo(250, 0);

	item->setName("Zener Diode");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* ElectricItems::createSchottkyDiode()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -125, 500, 250);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(12.5, 75); path.lineTo(12.5, 125);
	path.moveTo(12.5, 125); path.lineTo(62.5, 125);
	path.moveTo(62.5, 125); path.lineTo(62.5, -125);
	path.moveTo(62.5, -125); path.lineTo(112.5, -125);
	path.moveTo(112.5, -125); path.lineTo(112.5, -75);
	path.moveTo(62.5, 0); path.lineTo(250, 0);

	item->setName("Schottky Diode");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* ElectricItems::createNpnBjt()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(218.75, 0);
	path.cubicTo(218.75, -110.457, 129.2075, -200, 18.75, -200);
	path.cubicTo(-91.707, -200, -181.25, -110.457, -181.25, 0);
	path.cubicTo(-181.25, 110.457, -91.707, 200, 18.75, 200);
	path.cubicTo(129.2075, 200, 218.75, 110.457, 218.75, 0);

	path.moveTo(125, -250); path.lineTo(125, -125);
	path.moveTo(125, -125); path.lineTo(-93.75, -62.5);
	path.moveTo(-93.75, -62.5); path.lineTo(-93.75, 62.5);
	path.moveTo(-93.75, 62.5); path.lineTo(125, 125);
	path.moveTo(125, 125); path.lineTo(125, 250);
	path.moveTo(-93.75, -125); path.lineTo(-93.75, 125);
	path.moveTo(-250, 0); path.lineTo(-93.75, 0);

	path.moveTo(88.575, 59.4375); path.lineTo(125, 125);
	path.moveTo(125, 125); path.lineTo(51.9375, 161.425);

	item->setName("NPN BJT");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(125, -250));
	item->addConnectionPoint(QPointF(125, 250));

	return item;
}

DrawingPathItem* ElectricItems::createPnpBjt()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(218.75, 0);
	path.cubicTo(218.75, -110.457, 129.2075, -200, 18.75, -200);
	path.cubicTo(-91.707, -200, -181.25, -110.457, -181.25, 0);
	path.cubicTo(-181.25, 110.457, -91.707, 200, 18.75, 200);
	path.cubicTo(129.2075, 200, 218.75, 110.457, 218.75, 0);

	path.moveTo(125, -250); path.lineTo(125, -125);
	path.moveTo(125, -125); path.lineTo(-93.75, -62.5);
	path.moveTo(-93.75, -62.5); path.lineTo(-93.75, 62.5);
	path.moveTo(-93.75, 62.5); path.lineTo(125, 125);
	path.moveTo(125, 125); path.lineTo(125, 250);
	path.moveTo(-93.75, -125); path.lineTo(-93.75, 125);
	path.moveTo(-250, 0); path.lineTo(-93.75, 0);

	path.moveTo(-28.1875, 26.075); path.lineTo(-93.75, 62.5);
	path.moveTo(-93.75, 62.5); path.lineTo(-57.325, 128.0625);

	item->setName("PNP BJT");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(125, -250));
	item->addConnectionPoint(QPointF(125, 250));

	return item;
}

DrawingPathItem* ElectricItems::createNmosFet()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(218.75, 0);
	path.cubicTo(218.75, -110.457, 129.2075, -200, 18.75, -200);
	path.cubicTo(-91.707, -200, -181.25, -110.457, -181.25, 0);
	path.cubicTo(-181.25, 110.457, -91.707, 200, 18.75, 200);
	path.cubicTo(129.2075, 200, 218.75, 110.457, 218.75, 0);

	path.moveTo(125, -250); path.lineTo(125, -93.75);
	path.moveTo(125, -93.75); path.lineTo(0, -93.75);
	path.moveTo(0, 0); path.lineTo(125, 0);
	path.moveTo(125, 0); path.lineTo(125, 250);
	path.moveTo(0, 93.75); path.lineTo(125, 93.75);
	path.moveTo(0, 125); path.lineTo(0, 62.5);
	path.moveTo(0, 31.25); path.lineTo(0, -31.25);
	path.moveTo(0, -62.5); path.lineTo(0, -125);
	path.moveTo(-93.75, -125); path.lineTo(-93.75, 125);
	path.moveTo(-250, 0); path.lineTo(-93.75, 0);
	path.moveTo(53.0375, -53.0375); path.lineTo(0, 0);
	path.moveTo(0, 0); path.lineTo(53.0375, 53.0375);

	item->setName("NMOS FET");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(125, -250));
	item->addConnectionPoint(QPointF(125, 250));

	return item;
}

DrawingPathItem* ElectricItems::createPmosFet()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -250, 500, 500);
	QPainterPath path;
	path.moveTo(218.75, 0);
	path.cubicTo(218.75, -110.457, 129.2075, -200, 18.75, -200);
	path.cubicTo(-91.707, -200, -181.25, -110.457, -181.25, 0);
	path.cubicTo(-181.25, 110.457, -91.707, 200, 18.75, 200);
	path.cubicTo(129.2075, 200, 218.75, 110.457, 218.75, 0);

	path.moveTo(125, -250); path.lineTo(125, -93.75);
	path.moveTo(125, -93.75); path.lineTo(0, -93.75);
	path.moveTo(0, 0); path.lineTo(125, 0);
	path.moveTo(125, 0); path.lineTo(125, 250);
	path.moveTo(0, 93.75); path.lineTo(125, 93.75);
	path.moveTo(0, 125); path.lineTo(0, 62.5);
	path.moveTo(0, 31.25); path.lineTo(0, -31.25);
	path.moveTo(0, -62.5); path.lineTo(0, -125);
	path.moveTo(-93.75, -125); path.lineTo(-93.75, 125);
	path.moveTo(-250, 0); path.lineTo(-93.75, 0);
	path.moveTo(71.9625, -53.0375); path.lineTo(125, 0);
	path.moveTo(125, 0); path.lineTo(71.9625, 53.0375);

	item->setName("PMOS FET");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(125, -250));
	item->addConnectionPoint(QPointF(125, 250));

	return item;
}

DrawingPathItem* ElectricItems::createGround1()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-125, 0, 250, 156.25);
	QPainterPath path;

	path.moveTo(0, 0); path.lineTo(0, 62.5);
	path.moveTo(-93.75, 62.5); path.lineTo(93.75, 62.5);
	path.moveTo(-62.5, 109.375); path.lineTo(62.5, 109.375);
	path.moveTo(-31.25, 156.25); path.lineTo(31.25, 156.25);

	item->setName("Ground 1");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(0, 0));

	return item;
}

DrawingPathItem* ElectricItems::createGround2()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-125, 0, 250, 156.25);
	QPainterPath path;

	path.moveTo(0, 0); path.lineTo(0, 62.5);
	path.moveTo(-93.75, 62.5); path.lineTo(93.75, 62.5);
	path.moveTo(93.75, 62.5); path.lineTo(0, 156.25);
	path.moveTo(0, 156.25); path.lineTo(-93.75, 62.5);

	item->setName("Ground 2");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(0, 0));

	return item;
}

DrawingPathItem* ElectricItems::createOpAmp()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-375, -250, 750, 500);
	QPainterPath path;

	path.moveTo(-250, -250); path.lineTo(-250, 250); path.lineTo(250, 0); path.lineTo(-250, -250);
	path.moveTo(-375, -125); path.lineTo(-250, -125);
	path.moveTo(-375, 125); path.lineTo(-250, 125);
	path.moveTo(375, 0); path.lineTo(250, 0);
	path.moveTo(-212.5, -125); path.lineTo(-132.5, -125);
	path.moveTo(-172.5, -165); path.lineTo(-172.5, -85);
	path.moveTo(-212.5, 125); path.lineTo(-132.5, 125);

	item->setName("Op Amp");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-375, -125));
	item->addConnectionPoint(QPointF(-375, 125));
	item->addConnectionPoint(QPointF(375, 0));
	item->addConnectionPoint(QPointF(0, -125));
	item->addConnectionPoint(QPointF(0, 125));

	return item;
}

DrawingPathItem* ElectricItems::createLed()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-250, -93.75, 500, 187.5);
	QPainterPath path;
	path.moveTo(-250, 0); path.lineTo(-62.5, 0);
	path.moveTo(62.5, 0); path.lineTo(-62.5, -93.75); path.lineTo(-62.5, 93.75); path.lineTo(62.5, 0);
	path.moveTo(62.5, -93.75); path.lineTo(62.5, 93.75);
	path.moveTo(62.5, 0); path.lineTo(250, 0);

	path.moveTo(87.5, -43.75); path.lineTo(137.5, -93.75);
	path.moveTo(100, -93.75); path.lineTo(137.5, -93.75);
	path.moveTo(137.5, -93.75); path.lineTo(137.5, -56.25);

	path.moveTo(162.5, -43.75); path.lineTo(212.5, -93.75);
	path.moveTo(175, -93.75); path.lineTo(212.5, -93.75);
	path.moveTo(212.5, -93.75); path.lineTo(212.5, -56.25);

	item->setName("LED");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(-250, 0));
	item->addConnectionPoint(QPointF(250, 0));

	return item;
}

DrawingPathItem* ElectricItems::createVdc()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-187.5, -375, 375, 750);
	QPainterPath path;

	path.moveTo(187.5, 0);
	path.cubicTo(187.5, -138.0713, 103.5534, -250, 0, -250);
	path.cubicTo(-103.5534, -250, -187.5, -138.0713, -187.5, 0);
	path.cubicTo(-187.5, 138.0713, -103.5534, 250, 0, 250);
	path.cubicTo(103.5534, 250, 187.5, 138.0713, 187.5, 0);

	path.moveTo(0, -375); path.lineTo(0, -250);
	path.moveTo(0, 250); path.lineTo(0, 375);

	path.moveTo(-50, -81.25); path.lineTo(50, -81.25);
	path.moveTo(0, -131.25); path.lineTo(0, -31.25);
	path.moveTo(-50, 112.5); path.lineTo(50, 112.5);

	item->setName("DC Voltage");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(0, -375));
	item->addConnectionPoint(QPointF(0, 375));

	return item;
}

DrawingPathItem* ElectricItems::createVac()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-187.5, -375, 375, 750);
	QPainterPath path;

	path.moveTo(187.5, 0);
	path.cubicTo(187.5, -138.0713, 103.5534, -250, 0, -250);
	path.cubicTo(-103.5534, -250, -187.5, -138.0713, -187.5, 0);
	path.cubicTo(-187.5, 138.0713, -103.5534, 250, 0, 250);
	path.cubicTo(103.5534, 250, 187.5, 138.0713, 187.5, 0);

	path.moveTo(0, -375); path.lineTo(0, -250);
	path.moveTo(0, 250); path.lineTo(0, 375);

	path.moveTo(-62.5, 0); path.cubicTo(-12.5, -150, 12.5, 150, 62.5, 0);

	item->setName("AC Voltage");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(0, -375));
	item->addConnectionPoint(QPointF(0, 375));

	return item;
}

DrawingPathItem* ElectricItems::createIdc()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-187.5, -375, 375, 750);
	QPainterPath path;

	path.moveTo(187.5, 0);
	path.cubicTo(187.5, -138.0713, 103.5534, -250, 0, -250);
	path.cubicTo(-103.5534, -250, -187.5, -138.0713, -187.5, 0);
	path.cubicTo(-187.5, 138.0713, -103.5534, 250, 0, 250);
	path.cubicTo(103.5534, 250, 187.5, 138.0713, 187.5, 0);

	path.moveTo(0, -375); path.lineTo(0, -250);
	path.moveTo(0, 250); path.lineTo(0, 375);

	path.moveTo(0, 125); path.lineTo(0, -125);
	path.moveTo(-53.0375, -71.9625); path.lineTo(0, -125);
	path.moveTo(0, -125); path.lineTo(53.0375, -71.9625);

	item->setName("DC Current");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(0, -375));
	item->addConnectionPoint(QPointF(0, 375));

	return item;
}

DrawingPathItem* ElectricItems::createIac()
{
	DrawingPathItem* item = new DrawingPathItem();

	QRectF pathRect(-187.5, -375, 375, 750);
	QPainterPath path;

	path.moveTo(187.5, 0);
	path.cubicTo(187.5, -138.0713, 103.5534, -250, 0, -250);
	path.cubicTo(-103.5534, -250, -187.5, -138.0713, -187.5, 0);
	path.cubicTo(-187.5, 138.0713, -103.5534, 250, 0, 250);
	path.cubicTo(103.5534, 250, 187.5, 138.0713, 187.5, 0);

	path.moveTo(0, -375); path.lineTo(0, -250);
	path.moveTo(0, 250); path.lineTo(0, 375);

	path.moveTo(0, 125); path.lineTo(0, -125);
	path.moveTo(-53.0375, -71.9625); path.lineTo(0, -125);
	path.moveTo(0, -125); path.lineTo(53.0375, -71.9625);
	path.moveTo(-62.5, 0); path.cubicTo(-12.5, -150, 12.5, 150, 62.5, 0);

	item->setName("AC Current");
	item->setRect(pathRect);
	item->setPath(path, pathRect);
	item->addConnectionPoint(QPointF(0, -375));
	item->addConnectionPoint(QPointF(0, 375));

	return item;
}
