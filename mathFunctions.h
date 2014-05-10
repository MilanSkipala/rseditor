/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

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

#ifndef MATHFUNCTIONS_H
#define MATHFUNCTIONS_H

#include "includeHeaders.h"
#include <cmath>

#define PI 3.1415926535897932384626433832795

void rotatePoint(QPointF * pt, qreal angle);
void rotatePoint(QPointF * pt, qreal angle, QPointF *center);
void movePoint(QPointF * pt, qreal dx, qreal dy);
void toCartesian(qreal r, qreal phi, qreal & x, qreal & y);
bool pointsAreCloseEnough(QPointF * pt1, QPointF * pt2, const qreal toleranceHalf);
qreal dist(QPointF * ptA, QPointF * ptB);

#endif // MATHFUNCTIONS_H
