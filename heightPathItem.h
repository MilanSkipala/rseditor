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
    int lastLatSBValue;

public:
    HeightPathItem(ModelItem * item, QGraphicsItem * parent = 0);
    HeightPathItem(ModelItem * item, const QPainterPath & path, QGraphicsItem * parent = 0);
    //copies only HPI attributes - paths, brushes, etc. has to be copied manually thus it should be moved in copy c.
    HeightPathItem(const HeightPathItem &hpi);
    QDialog *initSlotTrackDialog();
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void adjustHeightOfParentItem();
};

#endif // HEIGHTPATHITEM_H
