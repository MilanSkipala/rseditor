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

#ifndef HEIGHTPATHITEM_H
#define HEIGHTPATHITEM_H


#include "includeHeaders.h"
#include <QDialog>
#include <QSpinBox>
#include "partsRelated.h"

//is it ok that HPI inherits QObject && QGPI?
class HeightPathItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
    ModelItem * parentItem;
    QDialog * slotTrackDialog;
    QSpinBox * longSpinBox;
    QSpinBox * latSpinBox;
    int lastLongSBValue;
    //int lastLatSBValue;
    int * lastLatSBValue;
    qreal * latAngle;

public:
    HeightPathItem(ModelItem * item, QGraphicsItem * parent = 0);
    HeightPathItem(ModelItem * item, const QPainterPath & path, QGraphicsItem * parent = 0);
    //copies only HPI attributes - paths, brushes, etc. has to be copied manually thus it should be moved in copy c.
    HeightPathItem(const HeightPathItem &hpi);
    QDialog *initSlotTrackDialog();
    void setAngle(qreal angle);
    qreal getAngle() const;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

protected slots:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void adjustHeightOfParentItem();
};

#endif // HEIGHTPATHITEM_H
