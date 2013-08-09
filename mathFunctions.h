#ifndef MATHFUNCTIONS_H
#define MATHFUNCTIONS_H

#include "includeHeaders.h"

QPoint * rotatePoint(QPoint * pt);//alloc new, rotate new, change old, delete new
QPoint * movePoint(QPoint * pt);//alloc new, move new, change old, delete new
//toPolar(qreal & r, qreal & phi, qreal & x, qreal & y)
//toCartesian(qreal & r, qreal & phi, qreal & x, qreal & y)

#endif // MATHFUNCTIONS_H
