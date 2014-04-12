#ifndef BEZIER_H
#define BEZIER_H

#include <QPointF>

class Bezier
{
    QPointF controlPoint1;
    QPointF controlPoint2;
    QPointF controlPoint3;
    QPointF controlPoint4;
public:
    Bezier();
    Bezier(const QPointF & pt1, const QPointF & pt2, const QPointF & pt3, const QPointF & pt4);
    QPointF getPointAtU(qreal u);

};

#endif // BEZIER_H
