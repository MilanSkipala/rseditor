#include <QFormLayout>
#include <QGraphicsWidget>
#include <QSpinBox>
#include "heightPathItem.h"
#include "globalVariables.h"
#include "mathFunctions.h"

HeightPathItem::HeightPathItem(ModelItem *item, QGraphicsItem *parent) : QGraphicsPathItem(parent), QObject()
{
    this->parentItem=item;
    this->slotTrackDialog=NULL;
    this->latSpinBox=NULL;
    this->longSpinBox=NULL;
    this->lastLatSBValue=new int;
    *this->lastLatSBValue=0;
    this->lastLongSBValue=0;
    this->slotTrackDialog = this->initSlotTrackDialog();
    this->latAngle = new qreal;
    *this->latAngle = 0;
    this->setFlag(QGraphicsItem::ItemIsMovable,true);


}

HeightPathItem::HeightPathItem(ModelItem *item, const QPainterPath &path, QGraphicsItem *parent) : QGraphicsPathItem(path,parent), QObject()
{
    this->parentItem=item;
    this->slotTrackDialog=NULL;
    this->latSpinBox=NULL;
    this->longSpinBox=NULL;
    this->lastLatSBValue=new int;
    *this->lastLatSBValue=0;
    this->lastLongSBValue=0;
    this->slotTrackDialog = this->initSlotTrackDialog();
    this->latAngle = new qreal;
    *this->latAngle = 0;
    this->setFlag(QGraphicsItem::ItemIsMovable,true);
}

HeightPathItem::HeightPathItem(const HeightPathItem &hpi)
{
    this->parentItem=hpi.parentItem;
    this->slotTrackDialog=hpi.slotTrackDialog;
    this->latSpinBox=hpi.latSpinBox;
    this->longSpinBox=hpi.longSpinBox;
    this->lastLongSBValue=hpi.lastLongSBValue;
    this->lastLatSBValue=hpi.lastLatSBValue;
    this->latAngle = hpi.latAngle;
    this->setFlag(QGraphicsItem::ItemIsMovable,true);
}

QDialog * HeightPathItem::initSlotTrackDialog()
{
    QDialog * pointer = NULL;

    if (app!=NULL && this->parentItem->get2DModelNoText()!=NULL && this->slotTrackDialog==NULL)
    {
        pointer = new QDialog(app->getWindow()->getWorkspaceWidget());
        pointer->setWindowTitle("Adjust height");
        QFormLayout * layout = new QFormLayout(pointer);
        QGraphicsView * preview = new QGraphicsView(pointer);
        preview->setScene(new QGraphicsScene);
        //scene is the new parent of 2D Model thus it disappears from workspace scene
        QGraphicsPathItem * qgpi = new QGraphicsPathItem();
        qgpi->setPath(this->parentItem->get2DModelNoText()->path());

        qgpi->setBrush(this->parentItem->get2DModelNoText()->brush());
        qgpi->setPen(this->parentItem->get2DModelNoText()->pen());
        qgpi->moveBy(this->parentItem->get2DModelNoText()->scenePos().x(),this->parentItem->get2DModelNoText()->scenePos().y());
        preview->scene()->addItem(qgpi);
        preview->scale(0.3,0.3);

        layout->addRow(preview);

        QLabel * partNoLabel = new QLabel(*this->parentItem->getPartNo());
        layout->addRow("Part number",partNoLabel);

        this->longSpinBox = new QSpinBox(this->slotTrackDialog);
        //this->longSpinBox->setValue(this->parentItem->getHeightProfileAt(this->parentItem->getEndPoint(1))-this->parentItem->getHeightProfileAt(this->parentItem->getEndPoint(0)));
        this->longSpinBox->setValue(this->lastLongSBValue);
        this->longSpinBox->setMaximum(512);
        this->longSpinBox->setMinimum(-512);
        layout->addRow("Longitudinal climb",this->longSpinBox);

        if (this->parentItem->getSlotTrackInfo()->numberOfLanes>1)
        {
            this->latSpinBox = new QSpinBox(this->slotTrackDialog);
            //this->latSpinBox->setValue(this->parentItem->getHeightProfileAt(this->parentItem->getEndPoint(0))-this->parentItem->getHeightProfileAt(this->parentItem->getEndPoint(2)));
            this->latSpinBox->setValue(*this->lastLatSBValue);

            this->latSpinBox->setMaximum(512);
            this->latSpinBox->setMinimum(-512);
            layout->addRow("Lateral climb",this->latSpinBox);
        }

        QPushButton * accept = new QPushButton("OK",pointer);
        QPushButton * cancel = new QPushButton("Cancel",pointer);


        connect(accept,SIGNAL(clicked()),this,SLOT(adjustHeightOfParentItem()));
        connect(cancel,SIGNAL(clicked()),pointer,SLOT(close()));

        //cout << "this" << endl;



        layout->addRow(accept);
        layout->addRow(cancel);


        pointer->setLayout(layout);
    }
    return pointer;
}

void HeightPathItem::setAngle(qreal angle)
{

    qreal latClimb = 0;
    if ((this->parentItem->getType()==S1 ||
        this->parentItem->getType()==J5 ||
        this->parentItem->getType()==X1 ||
        this->parentItem->getType()==X2 ||
        this->parentItem->getType()==JM ||
        this->parentItem->getType()==HE ||
        this->parentItem->getType()==HS))
        latClimb = sin(angle*PI/180)*(2*abs(this->parentItem->getRadius()));
    else
        latClimb = sin(angle*PI/180)*(abs(this->parentItem->getRadius()-this->parentItem->getSecondRadius()));

    *this->latAngle=angle;
    *this->lastLatSBValue=latClimb;
    this->latSpinBox->setValue(latClimb);

    /*cout << (int)this->parentItem << ", PtAddr:" << (int)this << ", lastVal:"<<(int)this->lastLatSBValue<< "||||" << endl;
    cout << this->parentItem->getType() << endl;*/
}

qreal HeightPathItem::getAngle() const
{
    return *this->latAngle;
}


void HeightPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    QGraphicsPathItem::mousePressEvent(event);
    /*
    if (this->boundingRect().contains(event->pos()))
        this->slotTrackDialog->exec();
    */
}

void HeightPathItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mouseDoubleClickEvent(event);
    if (this->boundingRect().contains(event->pos()))
        this->slotTrackDialog->exec();
}

/**
  TODO
  -modify it for HE,HS, JUMP parts
*/
void HeightPathItem::adjustHeightOfParentItem()
{
    //get longitudinal climb


    cout << (int)this->parentItem << ", PtAddr:" << (int)this << ", lastVal:"<<(int)this->lastLatSBValue<<endl;

    int longClimb = this->longSpinBox->value()-this->lastLongSBValue;
    ModelItem * mi = this->parentItem;

    int index = 0;
    //get lateral climb if there are 2 lanes or more
    if (this->parentItem->getSlotTrackInfo()->numberOfLanes>=2)
    {


        int latClimb = this->latSpinBox->value();///-this->lastLatSBValue;
        //add if for curved parts
        QPointF point;//(abs(this->parentItem->getRadius()),0);
        QPointF point2(abs(this->parentItem->getRadius())-this->parentItem->getSlotTrackInfo()->fstLaneDist,0);
        qreal angle;
        qreal previousAngle = 0;



        if ((this->parentItem->getType()==S1 ||
            this->parentItem->getType()==J5 ||
            this->parentItem->getType()==X1 ||
            this->parentItem->getType()==X2 ||
            this->parentItem->getType()==JM ||
            this->parentItem->getType()==HE ||
            this->parentItem->getType()==HS
             )
              ) //modify X1 and add remaining "straight" parts
        {
            //point.setX(2*point.x());
            point2=QPointF(abs(this->parentItem->getRadius()*2)-this->parentItem->getSlotTrackInfo()->fstLaneDist,0);
            angle = 180/PI*asin(latClimb/(2*abs(this->parentItem->getRadius())))-*this->latAngle;
        }
        else
        {
            //point.setX(point.x()-abs(this->parentItem->getSecondRadius()));
            point2.setX(point2.x()-abs(this->parentItem->getSecondRadius()));
            angle = 180/PI*asin(latClimb/(abs(this->parentItem->getRadius()-this->parentItem->getSecondRadius())))-*this->latAngle;


        }

        point = point2;

        if (((int)angle-(int)*this->latAngle)!=0)//this->latSpinBox->value()-this->lastLatSBValue!=0)
            *this->latAngle=angle+*this->latAngle;

/**
  TODO
  -FOLLOWING FOR HAS TO BE MODIFIED TO HANDLE HE AND HS TRACK PARTS
*/

        if (this->latSpinBox->value()-*this->lastLatSBValue!=0)//(((int)angle-(int)*this->latAngle)!=0)//
        {
            for (int i = 0; i < this->parentItem->getSlotTrackInfo()->numberOfLanes*2; i+=2)
            {
                rotatePoint(&point,*this->latAngle-angle);
                qreal yOld = point.y();
                rotatePoint(&point,angle);

                    if (this->parentItem->getSlotTrackInfo()->numberOfLanes>1)
                    {
                        *this->lastLatSBValue=this->latSpinBox->value();
                    }



                    int currZ = this->parentItem->getHeightProfileAt(this->parentItem->getEndPoint(i));
                    this->parentItem->adjustHeightProfile((int)point.y()-yOld,this->parentItem->getEndPoint(i));
                    //this->parentItem->adjustHeightProfile((int)point.y()-currZ,this->parentItem->getEndPoint(i));
                    currZ = this->parentItem->getHeightProfileAt(this->parentItem->getEndPoint(i+1));
                    this->parentItem->adjustHeightProfile((int)point.y()-yOld,this->parentItem->getEndPoint(i+1));
                    //this->parentItem->adjustHeightProfile((int)point.y()-currZ,this->parentItem->getEndPoint(i+1));

                point2.setX(point2.x()-this->parentItem->getSlotTrackInfo()->lanesGauge);
                point=point2;
            }
        }



        QPointF ptDebug(156,0);
        rotatePoint(&ptDebug,angle);

    }

    index = 1;
    //adjust height by long. climb
    while (mi->getEndPoint(index)!=NULL)
    {
        mi->adjustHeightProfile(longClimb,mi->getEndPoint(index));
        index+=2;
    }



    this->lastLongSBValue=this->longSpinBox->value();
    this->slotTrackDialog->close();
}


