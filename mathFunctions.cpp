#include "mathFunctions.h"

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
