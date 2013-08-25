#ifndef MATHFUNCTIONS_H
#define MATHFUNCTIONS_H

#include "includeHeaders.h"
#include <cmath>

#define PI 3.14159265

void rotatePoint(QPointF * pt, qreal angle);//alloc new, rotate new, change old, delete new
void movePoint(QPointF * pt);//alloc new, move new, change old, delete new
//toPolar(qreal & r, qreal & phi, qreal & x, qreal & y)
//toCartesian(qreal & r, qreal & phi, qreal & x, qreal & y)

#endif // MATHFUNCTIONS_H
