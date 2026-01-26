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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "component.h"
#include "e-element.h"
#include "e-resistor.h"
#include "pin.h"

class LibraryItem;

class MAINMODULE_EXPORT Joystick
    : public Component
    , public eElement
{
    Q_OBJECT
    Q_PROPERTY(
        double Resistance READ res WRITE setRes DESIGNABLE true USER true)
    Q_PROPERTY(int Pos_X READ posX WRITE setPosX DESIGNABLE true USER true)
    Q_PROPERTY(int Pos_Y READ posY WRITE setPosY DESIGNABLE true USER true)
    Q_PROPERTY(
        bool Pressed READ pressed WRITE setPressed DESIGNABLE true USER true)

  public:
    Joystick(QObject *parent, QString type, QString id);
    ~Joystick();

    static Component *construct(QObject *parent, QString type, QString id);
    static LibraryItem *libraryItem();

    virtual void initialize();
    virtual void attach();
    virtual void stamp();
    virtual void updateStep();

    double res() const
    {
        return m_resist;
    }
    void setRes(double v);

    int posX() const
    {
        return m_posX;
    }
    void setPosX(int x);

    int posY() const
    {
        return m_posY;
    }
    void setPosY(int y);

    bool pressed() const
    {
        return m_pressed;
    }
    void setPressed(bool p);

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *option,
                       QWidget *widget);

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

  private:
    void updateState();

    double m_resist;
    int m_posX; // 0-1000, 500 is center
    int m_posY; // 0-1000, 500 is center
    bool m_pressed;
    bool m_changed;

    Pin m_pinGND;
    Pin m_pinVCC;
    Pin m_pinVRx;
    Pin m_pinVRy;
    Pin m_pinSW;

    ePin m_ePinPotX_VCC;
    ePin m_ePinPotX_GND;
    ePin m_ePinPotY_VCC;
    ePin m_ePinPotY_GND;

    ePin m_ePinSW_GND;

    eResistor m_resXA;
    eResistor m_resXB;
    eResistor m_resYA;
    eResistor m_resYB;
    eResistor m_resSW;

    eNode *m_nodeVCC;
    eNode *m_nodeGND;

    bool m_isDragging;
};

#endif
