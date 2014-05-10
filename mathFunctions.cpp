/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

    This file is a part of Rail & Slot Editor.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mathFunctions.h"
#include "globalVariables.h"

void rotatePoint(QPointF *pt, qreal angle)
{
    qreal angleR = angle*PI/180;
    qreal x = pt->x()*cos(angleR)-pt->y()*sin(angleR);
    qreal y = pt->x()*sin(angleR)+pt->y()*cos(angleR);

    pt->setX(x);
    pt->setY(y);
}

void rotatePoint(QPointF *pt, qreal angle, QPointF * center)
{
    qreal angleR = angle*PI/180;
    qreal x = (pt->x()-center->x())*cos(angleR)-(pt->y()-center->y())*sin(angleR);
    qreal y = (pt->x()-center->x())*sin(angleR)+(pt->y()-center->y())*cos(angleR);

    pt->setX(x+center->x());
    pt->setY(y+center->y());
}

void movePoint(QPointF *pt, qreal dx, qreal dy)
{
    pt->setX(pt->x()+dx);
    pt->setY(pt->y()+dy);
}


void toCartesian(qreal r, qreal phi, qreal &x, qreal &y)
{
    x = r*sin(phi*PI/180);
    y = r*cos(phi*PI/180);
}


bool pointsAreCloseEnough(QPointF *pt1, QPointF *pt2, const qreal toleranceHalf)
{
    return QRectF(pt1->x()-toleranceHalf,pt1->y()-toleranceHalf,2*toleranceHalf,2*toleranceHalf).contains(*pt2);
}


qreal dist(QPointF *ptA, QPointF *ptB)
{
    qreal dx = ptA->x()-ptB->x();
    qreal dy = ptA->y()-ptB->y();
    return sqrt(dx*dx+dy*dy);
}
