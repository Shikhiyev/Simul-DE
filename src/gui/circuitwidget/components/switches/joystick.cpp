/***************************************************************************
 *   Copyright (C) 2026 by Antigravity                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "joystick.h"

#include "circuit.h"
#include "connector.h"
#include "itemlibrary.h"


Component *Joystick::construct(QObject *parent, QString type, QString id)
{
    return new Joystick(parent, type, id);
}

LibraryItem *Joystick::libraryItem()
{
    return new LibraryItem(tr("Joystick"), tr("Switches"), "joystick.png",
                           "Joystick", Joystick::construct);
}

Joystick::Joystick(QObject *parent, QString type, QString id) :
    Component(parent, type, id), eElement((id + "-eElement").toStdString()),
    m_pinGND(270, QPoint(-16, 24), id + "-PinGND", 0, this),
    m_pinVCC(270, QPoint(-8, 24), id + "-PinVCC", 0, this),
    m_pinVRx(270, QPoint(0, 24), id + "-PinVRx", 0, this),
    m_pinVRy(270, QPoint(8, 24), id + "-PinVRy", 0, this),
    m_pinSW(270, QPoint(16, 24), id + "-PinSW", 0, this),
    m_ePinPotX_VCC((id + "-ePinPX_VCC").toStdString(), 1),
    m_ePinPotX_GND((id + "-ePinPX_GND").toStdString(), 1),
    m_ePinPotY_VCC((id + "-ePinPY_VCC").toStdString(), 1),
    m_ePinPotY_GND((id + "-ePinPY_GND").toStdString(), 1),
    m_ePinSW_GND((id + "-ePinSW_GND").toStdString(), 1),
    m_resXA((id + "-resXA").toStdString()),
    m_resXB((id + "-resXB").toStdString()),
    m_resYA((id + "-resYA").toStdString()),
    m_resYB((id + "-resYB").toStdString()),
    m_resSW((id + "-resSW").toStdString())
{
    m_area = QRectF(-20, -24, 40, 48);

    setLabelPos(-20, -40, 0);

    m_posX       = 500;
    m_posY       = 500;
    m_pressed    = false;
    m_changed    = true;
    m_isDragging = false;
    m_resist     = 10000; // 10k default

    // X Pot
    m_resXA.setEpin(0, &m_pinVRx);
    m_resXA.setEpin(1, &m_ePinPotX_VCC);
    m_resXB.setEpin(0, &m_pinVRx);
    m_resXB.setEpin(1, &m_ePinPotX_GND);

    // Y Pot
    m_resYA.setEpin(0, &m_pinVRy);
    m_resYA.setEpin(1, &m_ePinPotY_VCC);
    m_resYB.setEpin(0, &m_pinVRy);
    m_resYB.setEpin(1, &m_ePinPotY_GND);

    // SW
    m_resSW.setEpin(0, &m_pinSW);
    m_resSW.setEpin(1, &m_ePinSW_GND);

    m_pinGND.setLabelText("GND");
    m_pinVCC.setLabelText("5V");
    m_pinVRx.setLabelText("X");
    m_pinVRy.setLabelText("Y");
    m_pinSW.setLabelText("SW");

    Simulator::self()->addToUpdateList(this);
}

Joystick::~Joystick() {}

void Joystick::initialize() {}

void Joystick::attach()
{
    m_nodeGND = m_pinGND.getEnode();
    m_nodeVCC = m_pinVCC.getEnode();

    if (!m_nodeGND) {
        m_nodeGND = new eNode(m_id + "-gndNode");
        m_pinGND.setEnode(m_nodeGND);
    }
    if (!m_nodeVCC) {
        m_nodeVCC = new eNode(m_id + "-vccNode");
        m_pinVCC.setEnode(m_nodeVCC);
    }

    m_ePinPotX_GND.setEnode(m_nodeGND);
    m_ePinPotX_VCC.setEnode(m_nodeVCC);
    m_ePinPotY_GND.setEnode(m_nodeGND);
    m_ePinPotY_VCC.setEnode(m_nodeVCC);

    m_ePinSW_GND.setEnode(m_nodeGND);

    m_changed = true;
    updateStep();
}

void Joystick::stamp()
{
    m_changed = true;
    updateStep();
}

void Joystick::updateStep()
{
    if (m_changed) {
        // X Pot
        double resX1 = double(m_resist * m_posX / 1000.0);
        double resX2 = m_resist - resX1;
        if (resX1 < 1e-6)
            resX1 = 1e-6;
        if (resX2 < 1e-6)
            resX2 = 1e-6;
        m_resXA.setRes(resX2); // VCC to X
        m_resXB.setRes(resX1); // GND to X

        // Y Pot
        double resY1 = double(m_resist * m_posY / 1000.0);
        double resY2 = m_resist - resY1;
        if (resY1 < 1e-6)
            resY1 = 1e-6;
        if (resY2 < 1e-6)
            resY2 = 1e-6;
        m_resYA.setRes(resY2); // VCC to Y
        m_resYB.setRes(resY1); // GND to Y

        // SW
        m_resSW.setRes(m_pressed ? 0.01 : 1e9);

        m_changed = false;
    }
}

void Joystick::setRes(double res)
{
    if (res <= 0)
        res = 1;
    m_resist  = res;
    m_changed = true;
}

void Joystick::setPosX(int x)
{
    if (x < 0)
        x = 0;
    if (x > 1000)
        x = 1000;
    if (m_posX == x)
        return;
    m_posX    = x;
    m_changed = true;
}

void Joystick::setPosY(int y)
{
    if (y < 0)
        y = 0;
    if (y > 1000)
        y = 1000;
    if (m_posY == y)
        return;
    m_posY    = y;
    m_changed = true;
}

void Joystick::setPressed(bool p)
{
    if (m_pressed == p)
        return;
    m_pressed = p;
    m_changed = true;
}

void Joystick::paint(QPainter *p, const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Component::paint(p, option, widget);

    p->setRenderHint(QPainter::Antialiasing);

    // Draw PCB
    p->setBrush(QColor(30, 40, 100)); // Premium Blue PCB
    p->setPen(QPen(Qt::black, 1));
    p->drawRoundedRect(-20, -24, 40, 48, 4, 4);

    // Draw Silk Screen
    p->setPen(QPen(Qt::white, 1));
    p->drawRect(-18, -22, 36, 44);

    // Draw Outer Circle
    p->setBrush(QColor(20, 20, 20));
    p->setPen(QPen(QColor(60, 60, 60), 2));
    p->drawEllipse(QPointF(0, -4), 15, 15);

    // Draw Handle
    double hx = (m_posX - 500) * 12.0 / 500.0;
    double hy =
        -(m_posY - 500) * 12.0 / 500.0; // Y is inverted in screen coords

    QRadialGradient grad(hx, -4 + hy, 10);
    grad.setColorAt(0, m_pressed ? Qt::red : QColor(80, 80, 80));
    grad.setColorAt(1, QColor(40, 40, 40));

    p->setBrush(grad);
    p->setPen(QPen(Qt::black, 1));
    p->drawEllipse(QPointF(hx, -4 + hy), 10, 10);

    // Draw shine on handle
    p->setBrush(QColor(255, 255, 255, 100));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(hx - 3, -4 + hy - 3), 3, 3);
}

void Joystick::updateState() {}

void Joystick::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        double hx = (m_posX - 500) * 12.0 / 500.0;
        double hy = -(m_posY - 500) * 12.0 / 500.0;
        QPointF handlePos(hx, -4 + hy);

        if (QLineF(event->pos(), handlePos).length() < 12) {
            m_isDragging = true;
            setPressed(true);
            event->accept();
            return;
        }
    }
    Component::mousePressEvent(event);
}

void Joystick::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging) {
        QPointF pos = event->pos();
        pos.setY(pos.y() + 4); // Center on outer circle

        double x = pos.x() * 500.0 / 12.0 + 500;
        double y = -pos.y() * 500.0 / 12.0 + 500;

        setPosX(qBound(0.0, x, 1000.0));
        setPosY(qBound(0.0, y, 1000.0));

        update();
        event->accept();
    } else
        Component::mouseMoveEvent(event);
}

void Joystick::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging) {
        m_isDragging = false;
        setPressed(false);
        // Return to center if released? Normal joysticks are spring-loaded.
        setPosX(500);
        setPosY(500);
        update();
        event->accept();
    } else
        Component::mouseReleaseEvent(event);
}

#include "moc_joystick.cpp"
