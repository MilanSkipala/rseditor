#include "mathFunctions.h"

void rotatePoint(QPointF *pt, qreal angle)
{
    qreal angleR = angle*PI/180;
    qreal x = pt->x()*cos(angleR)-pt->y()*sin(angleR);
    qreal y = pt->x()*sin(angleR)-pt->y()*cos(angleR);

    pt->setX(x);
    pt->setY(y);
}
