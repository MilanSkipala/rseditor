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

#include <QList>
#include "bezier.h"




Bezier::Bezier()
{
}

Bezier::Bezier(const QPointF &pt1, const QPointF &pt2, const QPointF &pt3, const QPointF &pt4)
{
    this->controlPoint1=pt1;
    this->controlPoint2=pt2;
    this->controlPoint3=pt3;
    this->controlPoint4=pt4;
}

QPointF Bezier::getPointAtU(qreal u)
{
    /*SOURCE:
     * http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/de-casteljau.html

      First, all given control points are arranged into a column, which is the left-most one in the figure. For each pair of adjacent control points, draw a south-east bound arrow and a north-east bound arrow, and write down a new point at the intersection of the two adjacent arrows. For example, if the two adjacent points are ij and i(j+1), the new point is (i+1)j. The south-east (resp., north-east) bound arrow means multiplying 1 - u (resp., u) to the point at its tail, ij (resp., i(j+1)), and the new point is the sum.
      Thus, from the initial column, column 0, we compute column 1; from column 1 we obtain column 2 and so on. Eventually, after n applications we shall arrive at a single point n0 and this is the point on the curve. The following algorithm summarizes what we have discussed. It takes an array P of n+1 points and a u in the range of 0 and 1, and returns a point on the Bézier curve C(u).

      Input: array P[0:n] of n+1 points and real number u in [0,1]
      Output: point on curve, C(u)
      Working: point array Q[0:n]

      for i := 0 to n do
          Q[i] := P[i]; // save input
      for k := 1 to n do
          for i := 0 to n - k do
              Q[i] := (1 - u)Q[i] + u Q[i + 1]; return Q[0];

    */

    QList<QPointF> Q;
    Q.push_back(this->controlPoint1);
    Q.push_back(this->controlPoint2);
    Q.push_back(this->controlPoint3);
    Q.push_back(this->controlPoint4);

    int n = 4;
    for (int k = 1; k < n; k++)
    {
        for (int i = 0; i < (n - k); i++)
            Q[i] = (1 - u)*Q[i] + u*Q[i + 1];
    }

    return Q[0];

}



