#ifndef MATHFUNCTIONS_H
#define MATHFUNCTIONS_H

#include "includeHeaders.h"
#include <cmath>

#define PI 3.1415926535897932384626433832795

void rotatePoint(QPointF * pt, qreal angle);
void rotatePoint(QPointF * pt, qreal angle, QPointF *center);
void movePoint(QPointF * pt, qreal dx, qreal dy);
//toPolar(qreal & r, qreal & phi, qreal & x, qreal & y)
void toCartesian(qreal r, qreal phi, qreal & x, qreal & y);

#endif // MATHFUNCTIONS_H
