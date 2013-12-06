/**
  TODO:
  -ModelItem::getIndexOfPoint(QPointF) const;
*/
#include "QFormLayout"
#include "QListWidget"
#include "QSpinBox"
#include "QTextEdit"
#include "application.h"
#include "partsRelated.h"
#include "heightPathItem.h"
#include "globalVariables.h"
#include "itemTypeEnums.h"
#include "mathFunctions.h"

//NOTE: parts of commented code (deleteFragmentItem, recursivelyAdd, etc.) have been removed.

//ModelItem::ModelItem(QString * partNumber, QString * partNameEn, QString * partNameCs,
//                     QPointF * start, QPointF * end ,qreal degree, qreal turnRadius,
//                     void * appPointer, QWidget * parentWidg, QGraphicsItem * contourParent)
ModelItem::ModelItem(QString & partNumber, QString & partNameEn, QString & partNameCs,
                     QPointF &start, QPointF &end ,qreal degree,
                     qreal turnRadius, qreal width, qreal height, ProductLine * prodLine, QWidget * parentWidg)/// : QWidget (parentWidg)
{
    logFile << "    ModelItem constructor" << endl;
    this->partNo = new QString(partNumber);
    this->nameEn = new QString(partNameEn);
    this->nameCs = new QString(partNameCs);
    ////this->startPoint = new QPointF(start);
    this->endPoints = new QList <QPointF*>();
    this->endPoints->append(new QPointF(start));
    this->endPoints->append(new QPointF(end));
    ////this->turnDegree=degree;
    this->endPointsAngles = new QList <qreal>();
    this->endPointsAngles->append(-degree/2.0);
    this->endPointsAngles->append(degree/2.0);
    this->neighbours = new QList<ModelItem*>();
    this->neighbours->append(NULL);
    this->neighbours->append(NULL);
    this->endPointsHeight = new QList<int>();
    this->endPointsHeight->append(0);
    this->endPointsHeight->append(0);
    this->endPointsHeightGraphics = new QList<QGraphicsPathItem*>();
    this->endPointsHeightGraphics->push_back(NULL);
    this->endPointsHeightGraphics->push_back(NULL);

    this->radius=turnRadius;
    this->radius2=0;
    this->itemWidth=width;
    this->itemHeight=height;//+=prodLine->getScaleEnum();




    this->availableCount=10;
    this->prodLine=prodLine;
    this->parentFragment=NULL;
    this->parentWidget = parentWidg;

    this->contourModel=NULL;
    this->contourModelNoText=NULL;
    this->glModel=NULL;
    this->recursionStopper=false;
    this->recursionStopperAdj=false;
    this->slotTrackInfo = NULL;

    if (!prodLine->getType())
        this->slotTrackInfo = new SlotTrackInfo();

    /*
    if (app_ptr->getUserPreferences()->getLocale()="Cs")
        this->label->setText(this->nameCs);
    else
        this->label->setText(this->nameEn);
*/

}

ModelItem::ModelItem(QString &partNumber, QString &partNameEn, QString &partNameCs, QList<QPointF> &endPoints, QList<qreal> angles, qreal turnRadius, qreal width, qreal height, ItemType type, ProductLine *prodLine, QWidget *parentWidg)
{
    logFile << "    ModelItem constructor #2" << endl;
    this->partNo = new QString(partNumber);
    this->nameEn = new QString(partNameEn);
    this->nameCs = new QString(partNameCs);
    this->endPoints = new QList <QPointF*>();
    this->neighbours = new QList<ModelItem*>();
    this->endPointsHeight = new QList<int>();
    this->endPointsHeightGraphics = new QList<QGraphicsPathItem*>();




    this->slotTrackInfo = NULL;

    QList<QPointF>::Iterator epIter = endPoints.begin();
    while(epIter!=endPoints.end())
    {
        this->endPoints->push_back(new QPointF(*epIter));
        epIter++;
        this->neighbours->push_back(NULL);
        this->endPointsHeight->push_back(0);
        this->endPointsHeightGraphics->push_back(NULL);
    }

    this->endPointsAngles = new QList <qreal>();
    QList<qreal>::Iterator aIter = angles.begin();
    int i = 0;
    while(aIter!=angles.end())
    {
        this->endPointsAngles->push_back(*aIter);

        logFile<< "        Inserting angle of " << *aIter << " at index " << i << endl;

        aIter++;
        i++;
    }



    this->radius=turnRadius;
    this->itemWidth=width;
    this->itemHeight=height+=prodLine->getScaleEnum()/2.0;

    this->availableCount=10;
    this->prodLine=prodLine;
    this->parentFragment=NULL;
    this->parentWidget = parentWidg;

    this->contourModel=NULL;
    this->contourModelNoText=NULL;
    this->glModel=NULL;
    this->recursionStopper=false;
    this->recursionStopperAdj=false;
    this->slotTrackInfo = NULL;

    this->t=type;

    if (!prodLine->getType())
            this->slotTrackInfo = new SlotTrackInfo();

}

ModelItem::~ModelItem()
{
    logFile << "~ModelItem" << endl;// << this->partNo->toStdString() << " position of point [0] is:" << this->getEndPoint(0) << endl;
    delete this->partNo;
    delete this->nameCs;
    delete this->nameEn;
    this->partNo=NULL;
    this->nameCs=NULL;
    this->nameEn=NULL;


    if (this->contourModel!=NULL)
    {
        this->contourModel->scene()->removeItem(this->contourModel);
        delete this->contourModel;
        this->contourModel=NULL;
    }

    if (this->contourModelNoText!=NULL)
    {
        this->contourModelNoText->scene()->removeItem(this->contourModelNoText);
        delete this->contourModelNoText;
        this->contourModelNoText=NULL;
    }

    ///
    ///QGLWidget * glModel

    ////qreal turnDegree;
    int c = this->endPoints->count();
    for (int i = 0; i < c; i++)
    {
        delete this->endPoints->at(i);
        //delete this->neighbours->at(i);
        app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(this->endPointsHeightGraphics->at(i));
        delete this->endPointsHeightGraphics->at(i);

    }
    this->endPointsHeightGraphics=NULL;
    delete this->neighbours;
    this->neighbours=NULL;
    delete this->endPoints;
    this->endPoints=NULL;
    delete this->endPointsAngles;
    this->endPointsAngles=NULL;
    delete this->endPointsHeight;
    this->endPointsHeight=NULL;
    delete this->endPointsHeightGraphics;
    this->endPointsHeightGraphics=NULL;

    if (this->slotTrackInfo!=NULL)
    {
        delete this->slotTrackInfo;
        this->slotTrackInfo=NULL;
    }

}



QString * ModelItem::getPartNo() const
{
    return this->partNo;
}
QString * ModelItem::getNameEn() const
{
    return this->nameEn;
}
QString * ModelItem::getNameCs() const
{
    return this->nameCs;
}
QPointF * ModelItem::getEndPoint(int index) const
{
    if (index < this->endPoints->count())
    {
        QPointF * pt = this->endPoints->at(index);
        return pt;
    }
    else
        return NULL;
}

qreal ModelItem::getTurnAngle(int index) const
{
    if (index < this->endPointsAngles->count())
        return this->endPointsAngles->at(index);
    else
        return -9.87654321;
}

qreal ModelItem::getTurnAngle(QPointF *pt) const
{
    if (pt==NULL)
        return -9.87654321;
    int index = 0;
    while (index < this->endPoints->count())
    {
        /**
          TODO:
          -tolerance
*/
        if (*pt==*this->endPoints->at(index))
            break;
        index++;
    }

    if (index>=this->endPoints->count()) //??
        return -9.87654321;

    return this->getTurnAngle(index);
}
qreal ModelItem::getRadius() const
{
    return this->radius;
}

qreal ModelItem::getItemWidth() const
{
    return this->itemWidth;
}

qreal ModelItem::getItemHeight() const
{
    return this->itemHeight;
}

GraphicsPathItem * ModelItem::get2DModel() const
{
    return this->contourModel;
}

int ModelItem::set2DModel(GraphicsPathItem *model)
{
    if (model==NULL)
        return 1;
    this->contourModel=model;
    return 0;
}

GraphicsPathItem *ModelItem::get2DModelNoText() const
{
    return this->contourModelNoText;
}

int ModelItem::set2DModelNoText(GraphicsPathItem *model)
{
    if (model==NULL)
        return 1;
    this->contourModelNoText=model;
    return 0;
}

int ModelItem::generate2DModel(bool text)
{
    QRectF rectOuter;
    QRectF rectInner;

    qreal gauge = 0;
    if (this->slotTrackInfo==NULL)
        gauge = this->prodLine->getScaleEnum()/2.0;
    else
        gauge = abs(this->radius-this->radius2)/2.0;

    if (this->radius>0 && this->slotTrackInfo==NULL)
    {

        rectOuter = QRectF(this->radius+gauge/1.0,-this->radius-gauge/1.0,-this->radius*2-2*gauge,this->radius*2+2*gauge);
        rectInner = QRectF(-this->radius+gauge/1.0,-this->radius+gauge/1.0,this->radius*2-2*gauge,this->radius*2-2*gauge);
    }
    else if (this->radius<0 && this->slotTrackInfo==NULL)
    {
        //rectOuter = QRectF(-this->radius+gauge/2.0,-this->radius+gauge/2.0,this->radius*2-gauge,this->radius*2-gauge);
        rectOuter = QRectF(this->radius-gauge/1.0,-this->radius+gauge/1.0,-this->radius*2+2*gauge,this->radius*2-2*gauge);
        rectInner = QRectF(-this->radius-gauge/1.0,-this->radius-gauge/1.0,this->radius*2+2*gauge,this->radius*2+2*gauge);
    }
    else if (this->radius>0 && this->slotTrackInfo!=NULL)
    {
        rectOuter = QRectF(this->radius,-this->radius,-this->radius*2,this->radius*2);
        rectInner = QRectF(-this->radius2,-this->radius2+0*gauge/1.0,this->radius2*2-0*gauge,this->radius2*2-0*gauge);
    }
    else if (this->radius<0 && this->slotTrackInfo!=NULL)
    {
        rectOuter = QRectF(this->radius,-this->radius,-this->radius*2,this->radius*2);
        rectInner = QRectF(-this->radius2,-this->radius2+0*gauge/1.0,this->radius2*2-0*gauge,this->radius2*2-0*gauge);
    }



    QPainterPath itemPath;

    qreal startAngle = 90-(this->getTurnAngle());


    QFont font;
    font.setPixelSize(20);
    QString label(*this->partNo);
    label.append(" ");
    QString s("");
    if (this->getTurnAngle()!=0)
        s.setNum(2*this->getTurnAngle(),'f',1);
    else
        s.setNum(2*this->getTurnAngle(),'f',0);

    label.append(s);
    label.append("° ");
    s.setNum(this->radius);
    label.append(s);



    if (text)// && this->getTurnDegree()!=0)
        itemPath.addText(-(12*this->partNo->length())/2,24-this->radius,font,*this->partNo);
    //else
        //itemPath.addText(-(6*this->partNo->length())/2,24,font,*this->partNo);

    if (this->t==C1 || this->t==C2)
    {
        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,2*this->getTurnAngle());



        //itemPath.arcMoveTo(rectInner,startAngle);
        itemPath.arcTo(rectInner,startAngle,2*this->getTurnAngle());


        itemPath.closeSubpath();

        if (this->slotTrackInfo!=NULL)
            itemPath.translate(0,-0*gauge);

    }
    else if (this->t==E1 || this->t==S1 || this->t==HS || this->t==HE || this->t==SC || ((this->t==X2 || this->t==J5 || this->t==J1 || this->t==J2) && this->slotTrackInfo!=NULL))
    {

        QPolygonF line;
        QPolygonF line2;
        if (this->slotTrackInfo!=NULL)
        {
            line << QPointF(-this->radius2,-this->radius);
            line << QPointF(this->radius2,-this->radius);
            line << QPointF(this->radius2,this->radius);

            line2 << QPointF(this->radius2,this->radius);
            line2 << QPointF(-this->radius2,this->radius);
            line2 << QPointF(-this->radius2,-this->radius);

            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);

            if (this->t==SC)
            {
                qreal d1 = this->slotTrackInfo->fstLaneDist;
                if (this->radius<0)
                    d1*=-1;

                QPolygonF line3;
                line3 << QPointF(-d1,-this->radius);
                line3 << QPointF(-d1,-this->radius-d1);
                line3 << QPointF(d1,-this->radius-d1);
                line3 << QPointF(d1,-this->radius);
                line3 << QPointF(-d1,-this->radius);



                line3 << QPointF(-d1+d1/4,-this->radius-d1/4);
                line3 << QPointF(-d1+d1/4,-this->radius-d1+d1/4);
                line3 << QPointF(d1-d1/4,-this->radius-d1+d1/4);
                line3 << QPointF(d1-d1/4,-this->radius-d1/4);
                line3 << QPointF(-d1+d1/4,-this->radius-d1/4);


                itemPath.addPolygon(line3);
            }

        }
        else if (this->radius>0)
        {
                line << QPointF(-this->radius,-this->radius-gauge/1.0);
                line << QPointF(this->radius,-this->radius-gauge/1.0);
                line << QPointF(this->radius,-this->radius+gauge/1.0);

                line2 << QPointF(-this->radius,-this->radius-gauge/1.0);
                line2 << QPointF(-this->radius,-this->radius+gauge/1.0);
                line2 << QPointF(this->radius,-this->radius+gauge/1.0);




            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);
            itemPath.translate(0,gauge);
        }
        else
        {
            QPolygonF line;
            line << QPointF(-this->radius,-this->radius+gauge/1.0);
            line << QPointF(this->radius,-this->radius+gauge/1.0);
            line << QPointF(this->radius,-this->radius-gauge/1.0);
            QPolygonF line2;
            line2 << QPointF(-this->radius,-this->radius-gauge/1.0);
            line2 << QPointF(this->radius,-this->radius-gauge/1.0);
            line << QPointF(this->radius,-this->radius+gauge/1.0);
            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);
            itemPath.translate(0,-gauge);
        }
        if (this->t==E1)
        {

            QPolygonF line;
            if (this->radius>0)
            {
                line << QPointF(this->radius-2*gauge,-this->radius+2*gauge);
                line << QPointF(this->radius,-this->radius);
                line << QPointF(this->radius,-this->radius+2*gauge);
                line << QPointF(this->radius-2*gauge,-this->radius);
            }
            else
            {
                line << QPointF(-this->radius-2*gauge,-this->radius-2*gauge);
                line << QPointF(-this->radius,-this->radius);
                line << QPointF(-this->radius,-this->radius-2*gauge);
                line << QPointF(-this->radius-2*gauge,-this->radius);
            }

            itemPath.addPolygon(line);
        }

        if (this->t==J1 && this->slotTrackInfo!=NULL)
        {
            qreal d1 = this->slotTrackInfo->fstLaneDist+this->slotTrackInfo->lanesGauge/2;
            if (this->radius<0)
                d1*=-1;
            itemPath.moveTo(-this->radius2,this->radius);
            itemPath.cubicTo(0,this->radius,
                             0,this->radius+d1,
                             this->radius2,this->radius+d1);
            QPolygonF turnoutLane;
            turnoutLane << QPointF(this->radius2,this->radius+d1);
            turnoutLane << QPointF(this->radius2,this->radius);
            turnoutLane << QPointF(-this->radius2,this->radius);
            itemPath.addPolygon(turnoutLane);

        }
        else if (this->t==J2 && this->slotTrackInfo!=NULL)
        {
            qreal d1 = -this->slotTrackInfo->fstLaneDist-this->slotTrackInfo->lanesGauge/2;
            if (this->radius<0)
                d1*=-1;
            itemPath.moveTo(-this->radius2,-this->radius);
            itemPath.cubicTo(0,-this->radius,
                             0,-this->radius+d1,
                             this->radius2,-this->radius+d1);
            QPolygonF turnoutLane;
            turnoutLane << QPointF(this->radius2,-this->radius+d1);
            turnoutLane << QPointF(this->radius2,-this->radius);
            turnoutLane << QPointF(-this->radius2,-this->radius);
            itemPath.addPolygon(turnoutLane);

        }

    }
    else if (this->t==J1 || this->t==J2)
    {
        /*
         *right turnout:
                0--1 is straight
                1--2 is straight

         * radius > 0
                     __====1
             _____---_____
            0_____---_____ 2

            radius < 0

            0===___
              _____---_____
             2_____---_____ 1



        */

        startAngle=90-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(2)))/2.0;

        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,abs(this->getTurnAngle(0))+abs(this->getTurnAngle(2)));
        itemPath.arcTo(rectInner,startAngle,abs(this->getTurnAngle(0))+abs(this->getTurnAngle(2)));


        itemPath.closeSubpath();
        if (this->parentWidget!=NULL)
            itemPath.translate(0,-this->itemHeight+gauge);
        else
            itemPath.translate(0,-this->itemHeight);




        if (this->radius>0 )
        {
            int diff = abs((this->endPointsAngles->at(2)+this->endPointsAngles->at(1)));
            if (diff==0 || diff==180)
            {

                QPolygonF line;
                QPolygonF line2;

                if(this->t==J1)
                {
                    line << QPointF(this->endPoints->at(0)->x(),-radius+this->endPoints->at(0)->y()-gauge);
                    line << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()-gauge);
                    line << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()+gauge);


                    line2 << QPointF(this->endPoints->at(0)->x(),-radius+this->endPoints->at(0)->y()+gauge);
                    line2 << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()+gauge);
                }
                else
                {
                    line << QPointF(this->endPoints->at(2)->x(),-radius+this->endPoints->at(2)->y()-gauge);
                    line << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()-gauge);
                    line << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()+gauge);


                    line2 << QPointF(this->endPoints->at(2)->x(),-radius+this->endPoints->at(2)->y()+gauge);
                    line2 << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()+gauge);
                }



                itemPath.addPolygon(line);
                itemPath.addPolygon(line2);

                itemPath.translate(0,(this->itemHeight-gauge));

            }
            else
            {
                if (this->parentWidget!=NULL)
                    itemPath.translate(0,this->itemHeight-gauge);
                else
                    itemPath.translate(0,this->itemHeight);




                QRectF rectOuter2;
                QRectF rectInner2;

                if (this->t==J1)
                {
                    QMatrix mat1;
                    mat1.rotate(abs(this->getTurnAngle(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(this->radius2+gauge/1.0,-this->radius2-gauge/1.0,-this->radius2*2-2*gauge,this->radius2*2+2*gauge);
                    rectInner2 = QRectF(-this->radius2+gauge/1.0,-this->radius2+gauge/1.0,this->radius2*2-2*gauge,this->radius2*2-2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    startAngle = 90-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1)));

                    itemPath.arcTo(rectInner2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(-abs(this->getTurnAngle(0)));
                    itemPath = mat2.map(itemPath);
                }
                else
                {
                    QMatrix mat1;
                    mat1.rotate(-abs(this->getTurnAngle(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(-this->radius2+gauge/1.0,-this->radius2-gauge/1.0,this->radius2*2-2*gauge,this->radius2*2+2*gauge);
                    rectInner2 = QRectF(this->radius2+gauge/1.0,-this->radius2+gauge/1.0,-this->radius2*2-2*gauge,this->radius2*2-2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    startAngle = 90-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1)));

                    itemPath.arcTo(rectInner2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(abs(this->getTurnAngle(0)));
                    itemPath = mat2.map(itemPath);
                }



            }

        }
        else
        {
            int diff = abs((this->endPointsAngles->at(2)+this->endPointsAngles->at(1)));
            if (diff==0 || diff==180)
            {
                (this->endPoints->at(1))->setX((-1)*(this->endPoints->at(1)->x()));
                (this->endPoints->at(1))->setY((-1)*(this->endPoints->at(1)->y()));

                (*(++this->endPointsAngles->begin()))-=180;
                (*(++this->endPointsAngles->begin()))*=-1;



                QPolygonF line;
                QPolygonF line2;

                if (this->t==J1)
                {
                    line << QPointF(this->endPoints->at(2)->x(),-this->radius+this->endPoints->at(2)->y()-this->itemHeight-(this->itemHeight-gauge));
                    line << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge));
                    line << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);

                    line2 << QPointF(this->endPoints->at(2)->x(),-this->radius+this->endPoints->at(2)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);
                    line2 << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);
                }
                else
                {
                    line << QPointF(this->endPoints->at(0)->x(),-this->radius+this->endPoints->at(0)->y()-this->itemHeight-(this->itemHeight-gauge));
                    line << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge));
                    line << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);

                    line2 << QPointF(this->endPoints->at(0)->x(),-this->radius+this->endPoints->at(0)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);
                    line2 << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);
                }

                //itemPath.translate(0,-(this->itemHeight-gauge));
                itemPath.addPolygon(line);
                itemPath.addPolygon(line2);
                //itemPath.translate(0,(this->itemHeight-gauge));
                itemPath.translate(0,(this->itemHeight-gauge));
            }
            else
            {
                if (this->parentWidget!=NULL)
                    itemPath.translate(0,this->itemHeight-gauge);
                else
                    itemPath.translate(0,this->itemHeight);





                QRectF rectOuter2;
                QRectF rectInner2;

                if (this->t==J1)
                {
                    QMatrix mat1;
                    mat1.rotate(abs(this->getTurnAngle(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(this->radius2+gauge/1.0,this->radius2+gauge/1.0,-this->radius2*2-2*gauge,-this->radius2*2-2*gauge);
                    rectInner2 = QRectF(-this->radius2+gauge/1.0,this->radius2-gauge/1.0,this->radius2*2-2*gauge,-this->radius2*2+2*gauge);

                    startAngle = 270;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    startAngle = 270+(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1)));

                    itemPath.arcTo(rectInner2,startAngle,-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(-abs(this->getTurnAngle(0)));
                    itemPath = mat2.map(itemPath);
                }
                else
                {
                    QMatrix mat1;
                    mat1.rotate(-abs(this->getTurnAngle(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(-this->radius2+gauge/1.0,-this->radius2-gauge/1.0,this->radius2*2-2*gauge,this->radius2*2+2*gauge);
                    rectInner2 = QRectF(this->radius2+gauge/1.0,-this->radius2+gauge/1.0,-this->radius2*2-2*gauge,this->radius2*2-2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    startAngle = 90-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1)));

                    itemPath.arcTo(rectInner2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(abs(this->getTurnAngle(0)));
                    itemPath = mat2.map(itemPath);
                }


                (this->endPoints->at(1))->setX((-1)*(this->endPoints->at(1)->x()));
                (this->endPoints->at(1))->setY((-1)*(this->endPoints->at(1)->y()));

                (*(++this->endPointsAngles->begin()))-=180;
                (*(++this->endPointsAngles->begin()))*=-1;



            }
        }



    }
    else if (this->t==J3)
    {
        /*        /2
         *0______/__________1
         *       \
         *        \3
         */


        startAngle = 270;

        QRectF rectOuterMinus = QRectF(this->radius-gauge/1.0,-this->radius+gauge/1.0,-this->radius*2+2*gauge,this->radius*2-2*gauge);
        QRectF rectInnerMinus = QRectF(-this->radius-gauge/1.0,-this->radius-gauge/1.0,this->radius*2+2*gauge,this->radius*2+2*gauge);

        //QRectF rectOuterMinus = QRectF(this->radius+gauge/1.0,-this->radius-gauge/1.0,-this->radius*2-2*gauge,this->radius*2+2*gauge);
        //QRectF rectInnerMinus = QRectF(-this->radius+gauge/1.0,-this->radius+gauge/1.0,this->radius*2-2*gauge,this->radius*2-2*gauge);


        if (radius > 0)
        {
            itemPath.translate(-this->getEndPoint(0)->x(),2*radius);
            itemPath.arcMoveTo(rectOuterMinus,startAngle);
            itemPath.arcTo(rectOuterMinus,startAngle,(this->getTurnAngle(2)));
            itemPath.arcMoveTo(rectInnerMinus,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,-(this->getTurnAngle(2)));
            //itemPath.closeSubpath();
            itemPath.translate(this->getEndPoint(0)->x(),-2*radius);

            //replace radius with itemWidth
            QPolygonF line;
            line << QPointF(this->getEndPoint(0)->x(),-this->radius-gauge/1.0);
            line << QPointF(this->getEndPoint()->x(),-this->radius-gauge/1.0);
            line << QPointF(this->getEndPoint()->x(),-this->radius+gauge/1.0);
            QPolygonF line2;
            line2 << QPointF(this->getEndPoint()->x(),-this->radius+gauge/1.0);
            line2 << QPointF(this->getEndPoint(0)->x(),-this->radius+gauge/1.0);
            line2 << QPointF(this->getEndPoint(0)->x(),-this->radius-gauge/1.0);
            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);

            itemPath.translate(-this->getEndPoint(0)->x(),0);

            startAngle=90;
            itemPath.arcMoveTo(rectOuter,startAngle);
            itemPath.arcTo(rectOuterMinus,startAngle,(this->getTurnAngle(3)));
            itemPath.arcMoveTo(rectInner,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,-(this->getTurnAngle(3)));

            itemPath.translate(this->getEndPoint(0)->x(),0);

            itemPath.translate(0,gauge);
        }
        else
        {
            //rotatePoint(this->getEndPoint(0),180);
            //rotatePoint(this->getEndPoint(1),180);
            rotatePoint(this->getEndPoint(2),180);
            rotatePoint(this->getEndPoint(3),180);

            itemPath.translate(this->getEndPoint(0)->x(),2*radius);
            itemPath.arcMoveTo(rectOuterMinus,startAngle);
            itemPath.arcTo(rectOuterMinus,startAngle,-(this->getTurnAngle(2)));
            itemPath.arcMoveTo(rectInnerMinus,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,(this->getTurnAngle(2)));
            //itemPath.closeSubpath();
            itemPath.translate(-this->getEndPoint(0)->x(),-2*radius);

            QPolygonF line;
            line << QPointF(this->getEndPoint(0)->x(),-this->radius-gauge/1.0);
            line << QPointF(this->getEndPoint()->x(),-this->radius-gauge/1.0);
            line << QPointF(this->getEndPoint()->x(),-this->radius+gauge/1.0);
            QPolygonF line2;
            line2 << QPointF(this->getEndPoint()->x(),-this->radius+gauge/1.0);
            line2 << QPointF(this->getEndPoint(0)->x(),-this->radius+gauge/1.0);
            line2 << QPointF(this->getEndPoint(0)->x(),-this->radius-gauge/1.0);
            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);

            itemPath.translate(this->getEndPoint(0)->x(),0);

            startAngle=90;
            itemPath.arcMoveTo(rectOuter,startAngle);
            itemPath.arcTo(rectOuterMinus,startAngle,-(this->getTurnAngle(3)));
            itemPath.arcMoveTo(rectInner,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,(this->getTurnAngle(3)));

            itemPath.translate(-this->getEndPoint(0)->x(),0);

            itemPath.translate(0,-gauge+1);

            this->setEndPointAngle(2,180-this->getTurnAngle(2));
            this->setEndPointAngle(3,180-this->getTurnAngle(3));
        }

    }
    //other types of turnouts except J4
    //{}
    else if (this->t==X1 || this->t==J4)
    {
        QPainterPath itemPath2;

        if (this->slotTrackInfo==NULL)
        {
            itemPath.translate(0,this->radius);
            if (this->radius>0)
            {
                QPolygonF line;
                line << QPointF(this->getEndPoint(0)->x(),-0-gauge/1.0);
                line << QPointF(this->getEndPoint()->x(),-0-gauge/1.0);
                line << QPointF(this->getEndPoint()->x(),-0+gauge/1.0);
                QPolygonF line2;
                line2 << QPointF(this->getEndPoint(0)->x(),-0-gauge/1.0);
                line2 << QPointF(this->getEndPoint(0)->x(),-0+gauge/1.0);
                line2 << QPointF(this->getEndPoint()->x(),-0+gauge/1.0);
                itemPath2.addPolygon(line);
                itemPath2.addPolygon(line2);
                //itemPath2.translate(0,gauge);
            }
            else
            {
                QPolygonF line;
                line << QPointF(this->getEndPoint(0)->x(),-0+gauge/1.0);
                line << QPointF(this->getEndPoint()->x(),-0+gauge/1.0);
                line << QPointF(this->getEndPoint()->x(),-0-gauge/1.0);
                QPolygonF line2;
                line2 << QPointF(this->getEndPoint()->x(),-0-gauge/1.0);
                line2 << QPointF(this->getEndPoint(0)->x(),-0-gauge/1.0);
                line2 << QPointF(this->getEndPoint(0)->x(),-0+gauge/1.0);
                itemPath2.addPolygon(line);
                itemPath2.addPolygon(line2);
                //itemPath2.translate(0,-gauge);
            }


            QMatrix matrix;
            matrix.rotate(-this->endPointsAngles->at(2));
            itemPath2 = matrix.map(itemPath2);




            itemPath.addPath(itemPath2);
            itemPath.translate(0,-this->radius);


            if (this->radius>0)
            {
                QPolygonF line3;
                line3 << QPointF(this->getEndPoint(0)->x(),-this->radius-gauge/1.0);
                line3 << QPointF(this->getEndPoint()->x(),-this->radius-gauge/1.0);
                line3 << QPointF(this->getEndPoint()->x(),-this->radius+gauge/1.0);
                QPolygonF line4;
                line4 << QPointF(this->getEndPoint(0)->x(),-this->radius-gauge/1.0);
                line4 << QPointF(this->getEndPoint(0)->x(),-this->radius+gauge/1.0);
                line4 << QPointF(this->getEndPoint()->x(),-this->radius+gauge/1.0);
                itemPath.addPolygon(line3);
                itemPath.addPolygon(line4);
                //itemPath.translate(0,gauge);
            }
            else
            {
                QPolygonF line3;
                line3 << QPointF(this->getEndPoint(0)->x(),-this->radius+gauge/1.0);
                line3 << QPointF(this->getEndPoint()->x(),-this->radius+gauge/1.0);
                line3 << QPointF(this->getEndPoint()->x(),-this->radius-gauge/1.0);
                QPolygonF line4;
                line4 << QPointF(this->getEndPoint()->x(),-this->radius-gauge/1.0);
                line4 << QPointF(this->getEndPoint(0)->x(),-this->radius-gauge/1.0);
                line4 << QPointF(this->getEndPoint(0)->x(),-this->radius+gauge/1.0);
                itemPath.addPolygon(line3);
                itemPath.addPolygon(line4);
                //itemPath.translate(0,-gauge);

                this->getEndPoint(2)->setY(-this->getEndPoint(2)->y());
                this->getEndPoint(3)->setY(-this->getEndPoint(3)->y());

            }




            if (this->t==J4)
            {


                startAngle = 90;
                itemPath.translate(this->getEndPoint(0)->x(),0);
                itemPath.arcMoveTo(rectOuter,startAngle);
                itemPath.arcTo(rectOuter,startAngle,-(this->getTurnAngle(2)));
                itemPath.arcMoveTo(rectInner,startAngle);
                itemPath.arcTo(rectInner,startAngle,(this->getTurnAngle(2)));

                startAngle = 270;
                itemPath.translate(-this->getEndPoint(0)->x(),2*radius);
                itemPath.translate(-this->getEndPoint(0)->x(),0);

                itemPath.arcMoveTo(rectOuter,startAngle);
                itemPath.arcTo(rectOuter,startAngle,(this->getTurnAngle(3)));
                itemPath.arcMoveTo(rectInner,startAngle);
                itemPath.arcTo(rectInner,startAngle,-(this->getTurnAngle(3)));
                itemPath.translate(this->getEndPoint(0)->x(),0);
                itemPath.translate(0,-2*radius);
            }

            if (this->radius>0)
                itemPath.translate(0,gauge);
            else
                itemPath.translate(0,-gauge);

        }
        else
        {
            qreal d1 = this->slotTrackInfo->fstLaneDist;
            qreal d2 = (this->slotTrackInfo->numberOfLanes-1)*this->slotTrackInfo->lanesGauge;

            if (this->radius<0)
            {
                d1*=-1;
                d2*=-1;
            }


            QPolygonF part;
            part << QPointF(-this->radius,      -this->radius2);
            part << QPointF(0,                  -this->radius2);
            part << QPointF(0,                  -this->radius2-d1);
            //part << QPointF(this->radius-d1,  -this->radius2-d1);
            //part << QPointF(this->radius-d1,  -this->radius2);
            part << QPointF(0+d1*2+d2,  -this->radius2-d1);
            part << QPointF(0+d1*2+d2,  -this->radius2);
            part << QPointF(this->radius,       -this->radius2);
            part << QPointF(this->radius,      -this->radius2+2*d1+d2);
            part << QPointF(0+d1*2+d2,  -this->radius2+2*d1+d2);
            part << QPointF(0+d1*2+d2,  this->radius2-d1);
            part << QPointF(0,                  this->radius2-d1);
            part << QPointF(0,                  -this->radius2+2*d1+d2);
            part << QPointF(-this->radius,      -this->radius2+2*d1+d2);
            part << QPointF(-this->radius,      -this->radius2+d1);
            itemPath.addPolygon(part);

        }




    }
    else if (this->t==J5)
    {

        itemPath.translate(0,4.5*this->radius2);

        QPolygonF lineUpper;
        QPolygonF lineLower;
        lineUpper << QPointF(this->getEndPoint(0)->x(),this->getEndPoint(0)->y()+gauge);
        lineUpper << QPointF(this->getEndPoint(1)->x(),this->getEndPoint(1)->y()+gauge);
        lineUpper << QPointF(this->getEndPoint(1)->x(),this->getEndPoint(1)->y()-gauge);
        lineUpper << QPointF(this->getEndPoint(0)->x(),this->getEndPoint(0)->y()-gauge);


        lineLower << QPointF(this->getEndPoint(2)->x(),this->getEndPoint(2)->y()+gauge);
        lineLower << QPointF(this->getEndPoint(3)->x(),this->getEndPoint(3)->y()+gauge);
        lineLower << QPointF(this->getEndPoint(3)->x(),this->getEndPoint(3)->y()-gauge);
        lineLower << QPointF(this->getEndPoint(2)->x(),this->getEndPoint(2)->y()-gauge);

        itemPath.addPolygon(lineUpper);
        itemPath.addPolygon(lineLower);

        itemPath.moveTo(this->getEndPoint(0)->x(),this->getEndPoint(0)->y()+gauge);
        itemPath.cubicTo(0,this->getEndPoint(0)->y()+gauge,
                         0,this->getEndPoint(3)->y()+gauge,
                         this->getEndPoint(3)->x(),this->getEndPoint(3)->y()+gauge);

        itemPath.moveTo(this->getEndPoint(0)->x(),this->getEndPoint(0)->y()-gauge);
        itemPath.cubicTo(0,this->getEndPoint(0)->y()-gauge,
                         0,this->getEndPoint(3)->y()-gauge,
                         this->getEndPoint(3)->x(),this->getEndPoint(3)->y()-gauge);

        itemPath.moveTo(this->getEndPoint(2)->x(),this->getEndPoint(2)->y()-gauge);
        itemPath.cubicTo(0,this->getEndPoint(2)->y()-gauge,
                         0,this->getEndPoint(1)->y()-gauge,
                         this->getEndPoint(1)->x(),this->getEndPoint(1)->y()-gauge);

        itemPath.moveTo(this->getEndPoint(2)->x(),this->getEndPoint(2)->y()+gauge);
        itemPath.cubicTo(0,this->getEndPoint(2)->y()+gauge,
                         0,this->getEndPoint(1)->y()+gauge,
                         this->getEndPoint(1)->x(),this->getEndPoint(1)->y()+gauge);


        if (this->radius>0)
            itemPath.translate(0,-this->radius);
        else
            itemPath.translate(0,-this->radius);



    }
    else if (t==T1)
    {
        itemPath.translate(0,2*this->radius);
        QRectF rect(-this->radius,-this->radius,2*this->radius,2*this->radius);
        itemPath.arcMoveTo(rect,0);
        itemPath.arcTo(rect,0,360);

        QMatrix mat1;
        mat1.rotate(19.450);
        itemPath = mat1.map(itemPath);

        QPolygonF line;
        line << QPointF(-this->radius,+gauge/1.0);
        line << QPointF(this->radius,+gauge/1.0);
        line << QPointF(this->radius,-gauge/1.0);
        line << QPointF(-this->radius,-gauge/1.0);
        line << QPointF(-this->radius,+gauge/1.0);
        itemPath.addPolygon(line);

        QPolygonF line2;
        line2 << QPointF(-gauge,-gauge);
        line2 << QPointF(-4*gauge,-gauge);
        line2 << QPointF(-4*gauge,-3*gauge/1.0);
        line2 << QPointF(-gauge,-3*gauge/1.0);
        line2 << QPointF(-gauge,-gauge);
        line2 << QPointF(-4*gauge,-3*gauge/1.0);
        line2 << QPointF(-4*gauge,-gauge);
        line2 << QPointF(-gauge,-3*gauge/1.0);
        itemPath.addPolygon(line2);

        QPolygonF line3;
        line3 << QPointF(this->radius,-gauge/1.0);
        line3 << QPointF(3*gauge,-4*gauge/1.0);
        line3 << QPointF(-3*gauge,-4*gauge/1.0);
        line3 << QPointF(-this->radius,-gauge/1.0);

        itemPath.addPolygon(line3);

        QMatrix mat2;
        mat2.rotate(-19.450);
        itemPath = mat2.map(itemPath);

        if  (this->radius>0)
            itemPath.translate(0,this->radius+gauge);
        else
            itemPath.translate(0,this->radius-gauge);



    }


    else if (t>=T2 && t<=T10)
    {
        qreal dy = 0;
        if (this->parentWidget!=NULL)
            dy = -gauge+this->itemHeight-gauge;
        else
        {
            if (this->radius>0)
            {
                dy = this->itemHeight-2*gauge;
                itemPath.translate(0,this->itemHeight+2*gauge);
            }
            else
            {
                //uprav to
                dy = -this->itemHeight+2*gauge;
                //itemPath.translate(0,this->itemHeight+2*gauge);
            }
        }

            QPolygonF poly;
            poly << QPointF(-4*gauge,-this->radius-gauge+this->itemHeight-gauge);
            poly << QPointF(-2*gauge,-this->radius-gauge+this->itemHeight-5*gauge);
            poly << QPointF(2*gauge,-this->radius-gauge+this->itemHeight-5*gauge);
            poly << QPointF(4*gauge,-this->radius-gauge+this->itemHeight-gauge);
            itemPath.addPolygon(poly);

            QPolygonF poly2;
            poly2 << QPointF(-gauge,-this->radius+dy-1*gauge);
            poly2 << QPointF(-gauge,-this->radius+dy-3*gauge);
            poly2 << QPointF(2*gauge,-this->radius+dy-3*gauge);
            poly2 << QPointF(2*gauge,-this->radius+dy-1*gauge);
            poly2 << QPointF(-gauge,-this->radius+dy-1*gauge);
            poly2 << QPointF(2*gauge,-this->radius+dy-3*gauge);
            poly2 << QPointF(2*gauge,-this->radius+dy-1*gauge);
            poly2 << QPointF(-gauge,-this->radius+dy-3*gauge);
            itemPath.addPolygon(poly2);

            itemPath.addRect(-this->radius,-this->radius+dy,2*this->radius,this->itemHeight-gauge);
            itemPath.addRect(-this->radius,-this->radius+dy+gauge,2*this->radius,2*gauge);


            if (this->parentWidget==NULL)
                itemPath.translate(0,2*gauge);

            if (this->radius<0)
                itemPath.translate(0,-2*this->itemHeight+2*gauge);



    }

    else if (t==JM)
    {
        QPolygonF line;
        QPolygonF line2;
        if (this->slotTrackInfo!=NULL)
        {
            /*
            line << QPointF(-this->radius2,-this->radius);
            line << QPointF(0,-this->radius);
            line << QPointF(0,this->radius);
            line << QPointF(-this->radius2,this->radius);
            line << QPointF(-this->radius2,-this->radius);

            qreal dx = this->slotTrackInfo->lanesGaugeEnd;
            if (this->radius<0)
                dx*=-1;

            line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,-this->radius);
            line2 << QPointF(this->radius2+dx,-this->radius);
            line2 << QPointF(this->radius2+dx,this->radius);
            line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,this->radius);
*/
            line << QPointF(-abs(this->radius2),-abs(this->radius));
            line << QPointF(0,-abs(this->radius));
            line << QPointF(0,abs(this->radius));
            line << QPointF(-abs(this->radius2),abs(this->radius));
            line << QPointF(-abs(this->radius2),-abs(this->radius));

            qreal dx = this->slotTrackInfo->lanesGaugeEnd;
            if (this->radius<0)
                dx*=1;

            line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,-abs(this->radius));
            line2 << QPointF(abs(this->radius2)+dx,-abs(this->radius));
            line2 << QPointF(abs(this->radius2)+dx,abs(this->radius));
            line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,abs(this->radius));

            qreal y = this->slotTrackInfo->fstLaneDist/2.0;

            if (this->radius<0)
            {
                y*=-1;
            }

            for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i++)
            {
                line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,this->radius-y);
                if (this->radius>0)
                    y += this->slotTrackInfo->fstLaneDist/2.0;
                else
                    y -= this->slotTrackInfo->fstLaneDist/2.0;
                line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd+36,this->radius-y);
                if (this->radius>0)
                    y += this->slotTrackInfo->fstLaneDist/2.0;
                else
                    y -= this->slotTrackInfo->fstLaneDist/2.0;
                line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,this->radius-y);
                if (this->radius>0)
                    y += this->slotTrackInfo->lanesGauge-this->slotTrackInfo->fstLaneDist;
                else
                    y -= this->slotTrackInfo->lanesGauge-this->slotTrackInfo->fstLaneDist;
            }

            line2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,-abs(this->radius));
            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);
        }
    }



    GraphicsPathItem * gpi = new GraphicsPathItem(this);
    gpi->setFlag(QGraphicsItem::ItemIsMovable,false);
    gpi->setFlag(QGraphicsItem::ItemIsSelectable,true);

    if (this->slotTrackInfo!=NULL)
    {

        QBrush b = gpi->brush();
        QColor col;
        col.setRed(64);
        col.setBlue(72);
        col.setGreen(64);
        b.setColor(col);
        b.setStyle(Qt::SolidPattern);
        gpi->setBrush(b);

        //make list of items instead of this
        QGraphicsPathItem * lane = new QGraphicsPathItem(gpi);

        QPainterPath lanePath;

        if (this->t==C1)
        {

            qreal rectParam = this->slotTrackInfo->fstLaneDist;
            if (radius <0)
                rectParam *=-1;
            QRectF rectLane;
            for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i ++)
            {
                rectLane = QRectF(this->radius-rectParam,
                                -this->radius+rectParam,
                                -this->radius*2+2*rectParam,
                                this->radius*2-2*rectParam);

                lanePath.arcMoveTo(rectLane,startAngle);
                lanePath.arcTo(rectLane,startAngle,2*this->getTurnAngle());

                if (radius > 0)
                    rectParam += this->slotTrackInfo->lanesGauge;
                else
                    rectParam -= this->slotTrackInfo->lanesGauge;

            }
            lanePath.translate(0,this->radius);
        }
        else if (this->t==S1 || this->t==X1 || this->t==SC || this->t==J5 || this->t==J1 || this->t==J2)
        {
            qreal yCoord = this->slotTrackInfo->fstLaneDist;
            if (this->radius<0)
                yCoord *=-1;
            for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i++)
            {
                QPolygonF lane;
                lane << QPointF(-this->radius2,yCoord);
                lane << QPointF(this->radius2,yCoord);


                lanePath.addPolygon(lane);
                if (this->radius>0)
                    yCoord+=this->slotTrackInfo->lanesGauge;
                else
                    yCoord-=this->slotTrackInfo->lanesGauge;

            }
            if (this->t==X1)
            {
                qreal d1 = this->slotTrackInfo->fstLaneDist;
                qreal d2 = (this->slotTrackInfo->numberOfLanes-1)*this->slotTrackInfo->lanesGauge;

                if (this->radius<0)
                {
                    d1*=-1;
                    d2*=-1;
                }

                qreal xCoord = this->slotTrackInfo->fstLaneDist;
                if (this->radius<0)
                    xCoord *=-1;
                for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i++)
                {
                    QPolygonF lane;
                    lane << QPointF(xCoord,-d1);
                    lane << QPointF(xCoord,2*this->radius2-d1);



                    lanePath.addPolygon(lane);

                    if (this->radius>0)
                        xCoord+=this->slotTrackInfo->lanesGauge;
                    else
                        xCoord-=this->slotTrackInfo->lanesGauge;

                }
            }

        }
        else if (this->t==C2)
        {

            /*works just for two lanes
*/
            qreal rectParam = this->slotTrackInfo->fstLaneDist;
            if (radius <0)
                rectParam *=-1;
            QRectF rectLane;
            int pointIndex = 0;

            //for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i ++)
            //{
                rectLane = QRectF(this->radius-rectParam,
                                -this->radius+rectParam,
                                -this->radius*2+2*rectParam,
                                this->radius*2-2*rectParam);

                lanePath.moveTo(*this->getEndPoint(0));
                lanePath.cubicTo(this->getEndPoint(0)->x()/2,rectParam,
                                 0,rectParam,
                                 this->getEndPoint(3)->x(),this->getEndPoint(3)->y());
                lanePath.moveTo(*this->getEndPoint(1));
                lanePath.cubicTo(this->getEndPoint(1)->x()/2,rectParam,
                                 0,rectParam,
                                 this->getEndPoint(2)->x(),this->getEndPoint(2)->y());

                if (radius > 0)
                    rectParam += this->slotTrackInfo->lanesGauge;
                else
                    rectParam -= this->slotTrackInfo->lanesGauge;

            //}
            //lanePath.translate(0,this->radius);
        }

        /*else if (this->t==X3)
        {}*/
        else if (this->t==HS || this->t==HE)
        {
            qreal yCoord = this->slotTrackInfo->fstLaneDist;
            qreal yCoord2 = this->slotTrackInfo->fstLaneDist+(this->slotTrackInfo->lanesGauge-this->slotTrackInfo->lanesGaugeEnd);
            if (this->radius<0)
            {
                yCoord *=-1;
                yCoord2 *=-1;
            }

            QPolygonF lane;
            lane << QPointF(-this->radius2,yCoord);
            lane << QPointF(this->radius2,yCoord);
            //lanePath.addPolygon(lane);

            for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i++)
            {



                //lanePath.addPolygon(lane);
                if (this->t==HS)
                {
                    lanePath.moveTo(-this->radius2,yCoord);
                    lanePath.cubicTo(-this->slotTrackInfo->fstLaneDist,yCoord,
                                     this->slotTrackInfo->fstLaneDist,yCoord2,
                                     this->radius2,yCoord2);
                }
                else
                {
                    lanePath.moveTo(-this->radius2,yCoord2);
                    lanePath.cubicTo(-this->slotTrackInfo->fstLaneDist,yCoord2,
                                     this->slotTrackInfo->fstLaneDist,yCoord,
                                     this->radius2,yCoord);
                }

                if (this->radius>0)
                {
                    yCoord+=this->slotTrackInfo->lanesGauge;
                    yCoord2+=this->slotTrackInfo->lanesGaugeEnd;
                }
                else
                {
                    yCoord-=this->slotTrackInfo->lanesGauge;
                    yCoord2-=this->slotTrackInfo->lanesGaugeEnd;
                }

            }
        }

        else if (this->t==JM)
        {
            qreal yCoord = this->slotTrackInfo->fstLaneDist;
            if (this->radius<0)
                yCoord *=-1;
            for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i++)
            {
                QPolygonF lane;
                if (this->radius2>0)
                    lane << QPointF(-this->radius2,yCoord);
                else
                    lane << QPointF(this->radius2,yCoord);
                lane << QPointF(0,yCoord);

                QPolygonF lane2;
                lane2 << QPointF(this->slotTrackInfo->lanesGaugeEnd,yCoord);
                if (this->radius>0)
                    lane2 << QPointF(this->radius2+this->slotTrackInfo->lanesGaugeEnd,yCoord);
                else
                    lane2 << QPointF(-this->radius2+this->slotTrackInfo->lanesGaugeEnd,yCoord);


                lanePath.addPolygon(lane);
                lanePath.addPolygon(lane2);

                if (this->radius>0)
                    yCoord+=this->slotTrackInfo->lanesGauge;
                else
                    yCoord-=this->slotTrackInfo->lanesGauge;

            }
        }

        //don't change this "if" to "else if" - it has to be like that!!!
        if (this->t==X2 || this->t==J5)
        {
            qreal yCoord = this->slotTrackInfo->fstLaneDist;
            qreal laneDist = this->slotTrackInfo->lanesGauge;
            if (this->radius<0)
            {
                yCoord *=-1;
                laneDist*=-1;
            }
            for (int i = 0; i < this->slotTrackInfo->numberOfLanes; i+=2)
            {
                lanePath.moveTo(-this->radius2,yCoord);
                lanePath.cubicTo(0,yCoord,
                                 0,yCoord+laneDist,
                                 this->radius2,yCoord+laneDist);

                lanePath.moveTo(-this->radius2,yCoord+laneDist);
                lanePath.cubicTo(0,yCoord+laneDist,
                                 0,yCoord,
                                 this->radius2,yCoord);


                if (this->radius>0)
                    yCoord+=2*this->slotTrackInfo->lanesGauge;
                else
                    yCoord-=2*this->slotTrackInfo->lanesGauge;
            }
        }

        else if (this->t==J1)
        {
            qreal d1 = this->slotTrackInfo->fstLaneDist+(this->slotTrackInfo->numberOfLanes-1)*this->slotTrackInfo->lanesGauge;
            qreal d2 = this->slotTrackInfo->fstLaneDist+(this->slotTrackInfo->numberOfLanes)*this->slotTrackInfo->lanesGauge;
            if (this->radius<0)
            {
                d1*=-1;
                d2*=-1;
            }

            lanePath.moveTo(-this->radius2,d1);
            lanePath.cubicTo(0,d1,
                             0,d2,
                             this->radius2,d2);


        }
        else if (this->t==J2)
        {
            qreal d1 =this->slotTrackInfo->fstLaneDist;
            qreal d2 =-this->slotTrackInfo->lanesGauge+this->slotTrackInfo->fstLaneDist;
            if (this->radius<0)
            {
                d1*=-1;
                d2*=-1;
            }

            lanePath.moveTo(-this->radius2,d1);
            lanePath.cubicTo(0,d1,
                             0,d2,
                             this->radius2,d2);


        }

        col.setRed(255);
        col.setBlue(0);
        col.setGreen(255);
        col = QColor(255,255,0);

        QPen p = lane->pen();
        p.setColor(col);
        p.setWidth(3);
        lane->setPen(p);

        lane->setPath(lanePath);

    }



    //qreal num = 0*(this->itemHeight-this->prodLine->getScaleEnum()/2);
    qreal num = 0;
    if (this->t!=J5)
        num = (this->itemHeight-this->prodLine->getScaleEnum()/2.0);

    if (this->slotTrackInfo!=NULL)
        num = 0;

    if (this->radius>0)
        itemPath.translate(0,this->radius-num);
    else
        itemPath.translate(0,this->radius+num);




    gpi->setPath(itemPath);
    gpi->changeCountPath(10,this->radius);
    gpi->setToolTip(label);





    if (text)
    {
        this->contourModel=gpi;
    }
    else
    {
        this->contourModelNoText=gpi;
    }

    this->updateEndPointsHeightGraphics();

    return 0;
}

unsigned int ModelItem::getAvailableCount() const
{
    return this->availableCount;
}

void ModelItem::setAvailableCount(unsigned int count)
{
    this->availableCount=count;
}

void ModelItem::incrAvailableCount()
{
    ++this->availableCount;
}

void ModelItem::decrAvailableCount()
{
    if (this->availableCount>0)
        --this->availableCount;
}

ItemType ModelItem::getType() const
{
    return this->t;
}

ProductLine *ModelItem::getProdLine() const
{
    return this->prodLine;
}

ModelFragment *ModelItem::getParentFragment() const
{
    return this->parentFragment;
}

int ModelItem::setParentFragment(ModelFragment *frag)
{
    if (frag==NULL)
        return 1;

    this->parentFragment=frag;
    return 0;
}

void ModelItem::rotate(qreal angle)
{

    if (angle!=0)
        logFile << "    Rotating item " << this->getPartNo()->toStdString() << " by " << angle << endl;

    QTransform rot;
    QTransform rot2;
    rot2 = rot.rotate(angle);
    this->contourModelNoText->setTransform(rot,true);
    QList<qreal>::Iterator iter = this->endPointsAngles->begin();
    while(iter!=this->endPointsAngles->end())
    {
        *iter+=angle;
        iter++;
    }

    int index = 0;
    while (this->getEndPoint(index)!=NULL)
    {
        rotatePoint(this->getEndPoint(index),angle);
        index++;
    }
    /*
    QPointF p1(*this->getEndPoint(0));
    QPointF p2(*this->getEndPoint());
    */
    QPointF s(this->itemWidth,this->itemHeight);
/*
    rotatePoint(&p1,angle);
    rotatePoint(&p2,angle);*/
    rotatePoint(&s,angle);

    this->itemHeight=s.y();
    this->itemWidth=s.x();
/*
    QList<QPointF*>::Iterator iter2 = this->endPoints->begin();
    **iter2=p1;
    iter2++;
    **iter2=p2;
*/

    return;

}

void ModelItem::rotate(qreal angle, QPointF *center)
{
    if (angle!=0)
        logFile << "    Rotating item " << this->getPartNo()->toStdString() << " by " << angle << " around point [" << center->x() << "," << center->y() << "]"  << endl;

    QTransform mat;
    QPointF newPointAfterRotation(this->get2DModelNoText()->scenePos().x(),this->get2DModelNoText()->scenePos().y());

    rotatePoint(&newPointAfterRotation,angle,center);

    //rotate 2D
    //set screen position of rotated 2D
    mat.reset();
    mat.rotate(angle);
    this->get2DModelNoText()->setTransform(mat,true);
    this->get2DModelNoText()->setPos(newPointAfterRotation.x(),newPointAfterRotation.y());

    QList<qreal>::Iterator iter = this->endPointsAngles->begin();
    while(iter!=this->endPointsAngles->end())
    {
        *iter+=angle;
        iter++;
    }

    int index = 0;
    while (this->getEndPoint(index)!=NULL)
    {
        //this->getEndPoint(index)->setX(this->getEndPoint(index)->x()-this->get2DModelNoText()->scenePos().x());
        //this->getEndPoint(index)->setY(this->getEndPoint(index)->y()-this->get2DModelNoText()->scenePos().y());
        this->getEndPoint(index)->setX(this->getEndPoint(index)->x());
        this->getEndPoint(index)->setY(this->getEndPoint(index)->y());
        rotatePoint(this->getEndPoint(index),angle,center);
        index++;
    }
    QPointF s(this->itemWidth,this->itemHeight);
    rotatePoint(&s,angle);

    this->itemHeight=s.y();
    this->itemWidth=s.x();

}

void ModelItem::moveBy(qreal dx, qreal dy)
{
    if (dx!=0 && dy!=0)
        logFile << "    Moving item " << this->getPartNo()->toStdString() << " by [" << dx << "," << dy << "]" << endl;
    QTransform mat;
    mat.translate(-dx,-dy);

    this->get2DModelNoText()->moveBy(dx,dy);
    //this->get2DModelNoText()->setTransform(mat,true);
    for(int i = 0; i < this->endPoints->count(); i++)
    {
        this->endPoints->at(i)->setX(this->endPoints->at(i)->x()+dx);
        this->endPoints->at(i)->setY(this->endPoints->at(i)->y()+dy);
        if (this->endPointsHeightGraphics->at(i)!=NULL)
            this->endPointsHeightGraphics->at(i)->moveBy(dx,dy);
    }
}

qreal ModelItem::getSecondRadius() const
{
    return this->radius2;
}

void ModelItem::setSecondRadius(qreal radi2)
{
    this->radius2=radi2;
}

int ModelItem::adjustHeightProfile(int dz, QPointF *point)//, bool ignoreRecursionStopper)
{/*
    if (!ignoreRecursionStopper && this->recursionStopper)
        return 0;
    if (ignoreRecursionStopper && this->recursionStopper)
        this->recursionStopper=false;*/

    if (this->recursionStopperAdj)
        return 0;

    adjustCallCount++;
    if (point==NULL)
        return 1;

    if (dz!=0)
        logFile << "Raising item " << this->getPartNo()->toStdString() << "by " << dz << endl;

    //find the index of the point
    int index = 0;
    QList<QPointF*>::Iterator ePIter = this->endPoints->begin();
    QList<int>::Iterator heightIter = this->endPointsHeight->begin();
    QList<ModelItem*>::Iterator neighIter = this->neighbours->begin();

    //rectangle approximation of point
    QRectF rect(0,0,0,0);

    while (ePIter!=this->endPoints->end())
    {
        /**
          check for correctness - (-x,-y) etc.
*/
        rect = QRectF((*ePIter)->x()-2.5,(*ePIter)->y()-2.5,5,5);
        if (rect.contains(*point))
            break;

        index++;
        ePIter++;
        heightIter++;
        neighIter++;
    }
    if (index==this->endPoints->count())
        return 2;


    //adjustHeight of all endPoints and all neighbours

    if (((this->t>=T1 && this->t<=T10) || (this->t>=J1 && this->t<=J5) || this->t==X1) && this->slotTrackInfo==NULL)
    {
        heightIter=this->endPointsHeight->begin();
        ePIter=this->endPoints->begin();
        neighIter=this->neighbours->begin();
        for (int i = 0; i < this->neighbours->count();i++)
        {
            if (!this->recursionStopperAdj)
            {
                if ((*neighIter)!=NULL)
                {
                    this->recursionStopperAdj=true;
                    (*neighIter)->adjustHeightProfile(dz,*ePIter);


                }
                *heightIter+=dz;
                this->recursionStopperAdj=false;

            }


            ePIter++;
            heightIter++;
            neighIter++;

        }
    }
    //or adjust height at "point" and neighbour connected at point
    else if (this->slotTrackInfo==NULL)
    {

        if (!this->recursionStopperAdj)
        {
            this->recursionStopperAdj=true;
            if ((*neighIter)!=NULL)
                (*neighIter)->adjustHeightProfile(dz,point);
            *heightIter+=dz;
            this->recursionStopperAdj=false;
        }
    }
    else
    {
        if (!this->recursionStopperAdj)
            *heightIter+=dz;


        //find neighbours´ lane which is connected at point = find neig. index of point => odd(i)=i&i-1
        //for both endpoints of that lane call adjust (DONT FORGET TO TURN ON STOPPER)
        int nEPIndex = 0;
        if ((*neighIter)!=NULL)
        {
            while ((*neighIter)->getEndPoint(nEPIndex)!=NULL)
            {
                rect = QRectF((*neighIter)->getEndPoint(nEPIndex)->x()-2.5,(*neighIter)->getEndPoint(nEPIndex)->y()-2.5,5,5);
                if (rect.contains(*point) && !this->recursionStopperAdj )
                {
                    this->recursionStopperAdj=true;
                    qreal newDz = dz;//-(*neighIter)->getHeightProfileAt((*neighIter)->getEndPoint(nEPIndex));


                    HeightPathItem * hpi = (HeightPathItem*)(*neighIter)->endPointsHeightGraphics->at(0);



                    int i = 0;

                    HeightPathItem * hpi2 = (HeightPathItem*)this->endPointsHeightGraphics->at(0);

                    QList<HeightPathItem*> myList;
                    for (int ijk = 0; ijk < this->endPoints->count(); ijk++)
                        myList.push_back((HeightPathItem*)this->endPointsHeightGraphics->at(ijk));


                    qreal angle = hpi2->getAngle();
                    while (this->getEndPoint(i)!=NULL)
                    {
                        hpi2 = (HeightPathItem*)this->endPointsHeightGraphics->at(i);
                        hpi2->setAngle(hpi2->getAngle());
                        i++;
                    }
                    ((HeightPathItem*)(*neighIter)->endPointsHeightGraphics->at(0))->setAngle(angle);

                    (*neighIter)->adjustHeightProfile(newDz,(*neighIter)->getEndPoint(nEPIndex));


                    if ((int)((HeightPathItem*)this->endPointsHeightGraphics->at(0))->getAngle()!=0)
                    {


                        if (nEPIndex%2==0)
                        {
                            (*neighIter)->adjustHeightProfile(newDz,(*neighIter)->getEndPoint(nEPIndex+1));
                        }
                        else
                        {
                            (*neighIter)->adjustHeightProfile(newDz,(*neighIter)->getEndPoint(nEPIndex-1));
                        }
                    }

                    this->recursionStopperAdj=false;
                }
                nEPIndex++;
            }
        }

    }

    this->recursionStopper=false;
    this->updateEndPointsHeightGraphics();

    return 0;
}

void ModelItem::updateEndPointsHeightGraphics(bool forceUpdate)
{
    if (!forceUpdate && this->recursionStopper)
        return;



    if (app!=NULL)
        updateCallCount++;
    qreal gaugeHalf = 8;
    /*this->prodLine->getScaleEnum()/2.0;
    if (!this->prodLine->getType())
        gaugeHalf = this->slotTrackInfo->fstLaneDist/2.0;*/
    QFont font2;
    font2.setPixelSize(1.5*gaugeHalf);
    //font2.setStyleStrategy(QFont::ForceOutline);

    QList<QGraphicsPathItem*>::Iterator grIter = this->endPointsHeightGraphics->begin();
    for (int i = 0; i < this->endPointsHeightGraphics->count();i++)
    {
        QPainterPath heightInfoEllipse;
        QPainterPath heightInfoText;
        heightInfoEllipse.addEllipse(*this->getEndPoint(i),gaugeHalf,gaugeHalf);
        QString number;
        heightInfoText.addText(this->getEndPoint(i)->x()-gaugeHalf/2.0,this->getEndPoint(i)->y()+gaugeHalf/2,font2,number.setNum(this->endPointsHeight->at(i)));


        QGraphicsPathItem * gpiProfile = new QGraphicsPathItem();
        QGraphicsPathItem * gpiProfileNumber = new QGraphicsPathItem(gpiProfile);
        //QGraphicsPathItem * gpiProfileNumber = new QGraphicsPathItem();
        //QGraphicsPathItem * gpiProfile = new QGraphicsPathItem(gpiProfileNumber);

        gpiProfile->setPath(heightInfoEllipse);
        gpiProfileNumber->setPath(heightInfoText);
        QBrush b = gpiProfile->brush();
        if (app!=NULL)
        {
            if (*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()==*this->endPoints->at(i))
            {
                b.setColor(QColor(0,200,255));

            }
            else
                b.setColor(Qt::blue);

            ModelItem * n = this->getNeighbour(i);
            if (n!=NULL)// && !this->recursionStopper)
            {
                this->recursionStopper=true;
                n->updateEndPointsHeightGraphics();
                this->recursionStopper=false;
            }
        }


        b.setStyle(Qt::SolidPattern);
        gpiProfile->setBrush(b);
        b.setColor(Qt::white);
        QPen p = gpiProfileNumber->pen();
        p.setColor(Qt::white);
        gpiProfileNumber->setBrush(b);
        gpiProfileNumber->setPen(p);
        gpiProfileNumber->setZValue(1);
        gpiProfile->setZValue(5);
        //gpiProfile->setFlag(QGraphicsItem::ItemIsMovable);

        QGraphicsPathItem * toDelete = (*grIter);


        //if ((*grIter)!=NULL && app!=NULL)
        //    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(*grIter);

        if (app!=NULL)
        {
            if (!app->getWindow()->getWorkspaceWidget()->getHeightProfileMode())
                gpiProfile->setVisible(false);
        }

        if (this->slotTrackInfo!=NULL)
        {
            HeightPathItem * hpi = NULL;

            if (app!=NULL)
            {
                if(this->parentWidget==app->getWindow()->getWorkspaceWidget())
                {
                    if ((*grIter)==NULL && (grIter)==this->endPointsHeightGraphics->begin())
                        hpi = new HeightPathItem(this);
                    else
                        hpi = new HeightPathItem(*(HeightPathItem*)(*this->endPointsHeightGraphics->begin()));

                    if (this->slotTrackInfo->numberOfLanes>1)
                    {
                        //if there is any neighbour on "the left side"
                        //here
                    }

                    //gpiProfile->setParentItem(hpi);
                    hpi->setPath(gpiProfile->path());
                    hpi->setBrush(gpiProfile->brush());
                    hpi->setPen(gpiProfile->pen());
                    gpiProfileNumber->setParentItem(hpi);
                    hpi->setVisible(gpiProfile->isVisible());
                    hpi->setFlag(QGraphicsItem::ItemIsSelectable,true);
                    (*grIter)=hpi;
                    //if (app!=NULL)
                    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(hpi);

                }

            }


        }
        else
        {
            (*grIter)=gpiProfile;
            if (app!=NULL)
                app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(*grIter);

        }

        if (app!=NULL)//(*grIter)!=NULL)//call scene to delete item
        {
            if (((toDelete)!=NULL))
                app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem((toDelete));

        }

        grIter++;
    }
    if (app!=NULL)
    {
        QPointF * pt = app->getWindow()->getWorkspaceWidget()->getActiveEndPoint();


    }

    this->recursionStopper=true;

}

int ModelItem::getHeightProfileAt(QPointF *point)
{
    //find the index of the point
    int index = 0;
    QList<QPointF*>::Iterator ePIter = this->endPoints->begin();
    QList<int>::Iterator heightIter = this->endPointsHeight->begin();

    //rectangle approximation of point
    QRectF rect(0,0,0,0);

    while (ePIter!=this->endPoints->end())
    {
        /**
          check for correctness - (-x,-y) etc.
*/
        rect = QRectF((*ePIter)->x()-2.5,(*ePIter)->y()-2.5,5,5);
        if (rect.contains(*point))
            break;

        index++;
        ePIter++;
        heightIter++;
    }
    if (index==this->endPoints->count())
        return -12345;

    return *heightIter;
}

void ModelItem::setEndPointAngle(int index, qreal angle)
{
    if (index>=this->endPointsAngles->count())
        return;

    QList<qreal>::Iterator it = this->endPointsAngles->begin();
    for (int k = 0; k < index; k++)
        it++;

    *it=angle;

}

void ModelItem::setEndPointAngle(QPointF *pt, qreal angle)
{
    if (pt==NULL)
        return;
    int index = 0;
    while (index < this->endPoints->count())
    {
        /**
          TODO:
          -tolerance
*/
        if (*pt==*this->endPoints->at(index))
            break;
        index++;
    }

    if (index>=this->endPoints->count()) //??
        return;

    this->setEndPointAngle(index,angle);
}

SlotTrackInfo *ModelItem::getSlotTrackInfo()
{
    return this->slotTrackInfo;
}

int ModelItem::setSlotTrackInfo(SlotTrackInfo *s)
{
    if (s==NULL)
        return 1;
    this->slotTrackInfo= new SlotTrackInfo(*s);
    return 0;
}

ModelItem *ModelItem::getNeighbour(int index)
{
    if (index >= this->neighbours->count() || index <0)
        return NULL;
    return this->neighbours->at(index);
}

ModelItem *ModelItem::getNeighbour(QPointF *pos)
{
    /*
      this is wrong, because pointer may not be found!
      you need to search for a value of pos
*/
    int ptIndex = 0;//this->endPoints->indexOf(pos,0);

    //correct version
    QList<QPointF*>::Iterator iter = this->endPoints->begin();
    while (iter!=this->endPoints->end())
    {
        if (*pos==**iter)
            break;
        ptIndex++;
        iter++;
    }

    if (ptIndex>=this->endPoints->count())
        return NULL;
    return this->neighbours->at(ptIndex);
}

int ModelItem::setNeighbour(ModelItem *neighbour, int index)
{
    /*
    if (neighbour==NULL)
        return 1;*/
    if (index >=this->neighbours->count() || index < 0)
        return 2;

    QList<ModelItem*>::Iterator i = this->neighbours->begin();
    while (*i != this->neighbours->at(index))
        i++;
    *i=neighbour;
    return 0;
}

int ModelItem::setNeighbour(ModelItem *neighbour, QPointF *pos)
{
    //if (neighbour==NULL || pos==NULL)
    if (pos==NULL)
        return 1;

    int index = -1;
    QList<ModelItem*>::Iterator iter = this->neighbours->begin();

    for (int i = 0; i < this->endPoints->count();i++)
    {
        if (*this->endPoints->at(i)==*pos)
        {
            index = i;
            break;
        }
        iter++;
    }

    if (index < 0 || index>=this->endPoints->count())
        return 2;

    *iter=neighbour;

    if (neighbour!=NULL)
    {
        if (this->slotTrackInfo!=NULL)
        {
            int nIndex = 0;
            for (int i = 0; i < (neighbour)->endPoints->count();i++)
            {
                if (*(neighbour)->endPoints->at(i)==*pos)
                {
                    nIndex = i;
                    break;
                }
            }

            HeightPathItem * hpi = (HeightPathItem*)neighbour->endPointsHeightGraphics->at(nIndex);
            HeightPathItem * hpi2 = (HeightPathItem*)this->endPointsHeightGraphics->at(index);
            hpi->setAngle(hpi2->getAngle());
        }
    }


    return 0;
}

bool ModelItem::leftRightDifference180(int index1, int index2) const
{
    /**
      BUG:
      (-45) -> (-67.5) left turn (=> radius is less than 0) returns true!!
*/

    //following algorithm works well for items with at least one angle > 0

    if (index1 >= this->endPoints->count() || index2 >= this->endPoints->count())
        return false;
    qreal dA = ((this->getTurnAngle(index2))-(this->getTurnAngle(index1)));
    dA = dA;/// /2.0;
    dA = abs(dA);
    //left < right
    if ((abs(this->getTurnAngle(index2))-abs(this->getTurnAngle(index1)))>=0)//(dA>=0)//(this->getTurnDegree(index2)>this->getTurnDegree(index1))
    {

        dA = abs(dA);
        qreal result =((abs(this->getTurnAngle(index1))+dA)-abs(this->getTurnAngle(index2)));
        if (result==0)
            return false;
    }
    ///is this condition always correct?
    else if (dA<180)///(dA<90) //dA=90 means it was =180
    {
        //qreal dA = (this->getTurnDegree(index2)-this->getTurnDegree(index1));
        //dA = dA /2.0;
        dA = abs(dA);
        qreal result =((abs(this->getTurnAngle(index2))+dA)-abs(this->getTurnAngle(index1)));
        if ((result==0 && this->radius<0) || (result==0 && this->getTurnAngle(index2)<0 && this->getTurnAngle(index1)<0))
            return false;
    }
    return true;

    /*V1
    qreal dA = (this->getTurnDegree(index2)-this->getTurnDegree(index1));
    dA = dA /2.0;
    dA = abs(dA);
    //left < right
    if ((abs(this->getTurnDegree(index2))-abs(this->getTurnDegree(index1)))>=0)//(dA>=0)//(this->getTurnDegree(index2)>this->getTurnDegree(index1))
    {

        dA = abs(dA);
        qreal result =((this->getTurnDegree(index1)+2*dA)-this->getTurnDegree(index2));
        if (result==0)/// && this->radius>0)
            return false;
    }
    ///is this condition always correct?
    else if (dA<90) //dA=90 means it was =180
    {
        //qreal dA = (this->getTurnDegree(index2)-this->getTurnDegree(index1));
        //dA = dA /2.0;
        dA = abs(dA);
        qreal result =((this->getTurnDegree(index2)+2*dA)-this->getTurnDegree(index1));
        if (result==0 && this->radius<0)
            return false;
    }
    return true;*/
}

bool ModelItem::leftRightDifference180(QPointF *pt1, QPointF *pt2) const
{
    int i1 = 0;
    int i2 = 0;

    while (this->getEndPoint(i1)!=NULL)
    {
        if (this->getEndPoint(i1)==pt1)
            break;
        i1++;
    }
    while (this->getEndPoint(i2)!=NULL)
    {
        if (this->getEndPoint(i2)==pt2)
            break;
        i2++;
    }
    return leftRightDifference180(i1,i2);

}



QWidget *ModelItem::getParentWidget() const
{
    return this->parentWidget;
}

GraphicsPathItem::GraphicsPathItem(ModelItem * item, QGraphicsItem * parent) : QGraphicsPathItem(parent)
{
    this->setFlag(QGraphicsItem::ItemIsMovable,true);
    this->setFlag(QGraphicsItem::ItemIsSelectable,true);
    this->restrictedCountPath = NULL;
    this->parentItem=item;
    this->dialog=NULL;
    this->contextMenuSBW = NULL;
    this->contextMenuWSW = NULL;
    this->mousePressed=false;
}
GraphicsPathItem::GraphicsPathItem(ModelItem * item, const QPainterPath & path, QGraphicsItem * parent) : QGraphicsPathItem(path,parent)
{
    this->restrictedCountPath = NULL;
    this->parentItem=item;
    this->dialog=NULL;
    this->contextMenuSBW = NULL;
    this->contextMenuWSW = NULL;
    this->mousePressed=false;



}

GraphicsPathItem::~GraphicsPathItem()
{

}



void GraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=true;

    QGraphicsPathItem::mousePressEvent(event);
}
void GraphicsPathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=false;
    QGraphicsPathItem::mouseReleaseEvent(event);
    if (this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget())
    app->getWindow()->getWorkspaceWidget()->connectFragments(this->parentItem->getParentFragment());
}
void GraphicsPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((this->mousePressed) && this->parentItem->getParentFragment()!=NULL)
    {
        if (this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget() && app->getWindow()->getWorkspaceWidget()->getRotationMode())
        {
            //somehow get the angle of rotation
            QPointF center(this->scenePos().x(),this->scenePos().y());
            QPointF * newPoint = new QPointF(*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
            qreal angle = event->scenePos().x()-event->lastScenePos().x();

            rotatePoint(newPoint,angle,&center);
            app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);

            this->parentItem->getParentFragment()->rotate(angle,&center);



        }
        else
        {
            QPointF diff = event->lastScenePos()-event->scenePos();
            this->parentItem->getParentFragment()->moveBy(-diff.x(),-diff.y());

            //QPointF * newPoint = new QPointF(*);
            //newPoint->setX(newPoint->x()-diff.x());
            //newPoint->setY(newPoint->y()-diff.y());
            if (this->parentItem->getParentFragment()==app->getWindow()->getWorkspaceWidget()->getActiveFragment())
            {
                QPointF * newPoint = new QPointF(*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
                newPoint->setX(newPoint->x()-diff.x());
                newPoint->setY(newPoint->y()-diff.y());
                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);

                //app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
            }
        }
    }
    QGraphicsPathItem::mouseMoveEvent(event);
}

bool GraphicsPathItem::contains(const QPointF &point) const
{
    return this->path().boundingRect().contains(point);

}

QRectF GraphicsPathItem::boundingRect() const
{
    return this->path().controlPointRect();
}

QPainterPath GraphicsPathItem::shape() const
{
    QPainterPath * p = new QPainterPath();
    p->addRect(this->path().controlPointRect());
    return *p;
}

void GraphicsPathItem::changeCountPath(unsigned int count, qreal radius)
{
    GraphicsPathItem * newItem = new GraphicsPathItem(this->parentItem,this);

    QPainterPath pp;
    QRectF r(-50+5,5+38-(0*radius),15,13);
    pp.addRect(r);
    QFont font;
    font.setPixelSize(10);
    QString text;
    text.setNum(count);

    pp.addText(-49+5,5+49-(0*radius),font,text);
    QBrush b = newItem->brush();
    if (count==1)
        b.setColor(Qt::yellow);
    else if (count==0)
        b.setColor(Qt::red);
    else
        b.setColor(Qt::green);
    b.setStyle(Qt::SolidPattern);
    newItem->setBrush(b);
    newItem->setPath(pp);

    if (this->restrictedCountPath!=NULL)
        delete this->restrictedCountPath;

    this->restrictedCountPath=newItem;
    //this->restrictedCountPath->setVisible(false);

}

int GraphicsPathItem::initDialog()
{
    this->dialog = new QDialog (app->getWindow());
    this->dialog->setWindowTitle("Change available count");
    QFormLayout * layout = new QFormLayout(this->dialog);

    QLabel * lineTextBox = new QLabel(*(this->parentItem->getProdLine()->getName()),this->dialog);
    layout->addRow("Manufacturer's name:", lineTextBox);
    lineTextBox->setFixedHeight(30);
    QLabel * partNoTextBox = new QLabel(*(this->parentItem->getPartNo()),this->dialog);
    partNoTextBox->setFixedHeight(30);
    layout->addRow("Part No:",partNoTextBox);

    QSpinBox * countSpinBox = new QSpinBox(this->dialog);
    countSpinBox->setRange(0,999);
    countSpinBox->setValue(this->parentItem->getAvailableCount());


    layout->addRow("Available count:",countSpinBox);

    QPushButton * confirm = new QPushButton("Confirm",this->dialog);
    QPushButton * discard = new QPushButton("Discard",this->dialog);

    QWidget::connect(discard,SIGNAL(clicked()),this->dialog,SLOT(close()));
    connect(confirm,SIGNAL(clicked()),this,SLOT(updateItem()));

    layout->addRow(confirm,discard);

    this->dialog->setLayout(layout);

    return 0;
}

int GraphicsPathItem::initMenus()
{
    this->contextMenuSBW = new QMenu(app->getWindow()->getSideBarWidget());
    QAction * action = new QAction ("Change model part info",this->contextMenuSBW);
    this->contextMenuSBW->addAction(action);

    this->contextMenuWSW = new QMenu(app->getWindow()->getWorkspaceWidget());
    QAction * action2 = new QAction ("Fragment info",this->contextMenuWSW);
    this->contextMenuWSW->addAction(action2);

    ///IF NULL RETURN 1
    return 0;
}

void GraphicsPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (this->parentItem->getParentWidget()!=app->getWindow()->getWorkspaceWidget() && (app->getRestrictedInventoryMode() && this->restrictedCountPath!=NULL))
    {
        this->restrictedCountPath->setVisible(true);
    }
    else if ((!app->getRestrictedInventoryMode() || this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget()) && this->restrictedCountPath!=NULL)
        this->restrictedCountPath->setVisible(false);
    QGraphicsPathItem::paint(painter,option,widget);
}


void makeNewItem(QPointF eventPos, GraphicsPathItem * gpi, ModelItem * parentItem, ModelItem * toMake, bool key)
{

    bool enable = true;

    if (parentItem->getParentWidget()!=app->getWindow()->getWorkspaceWidget() || key)
    {
        if (app->getRestrictedInventoryMode())
        {
            if (parentItem->getAvailableCount()<1)
                enable = false;
            parentItem->decrAvailableCount();
            gpi->changeCountPath(parentItem->getAvailableCount(),parentItem->getRadius());
        }

        if (enable)
        {
            ModelItem * it = NULL;

            QList<qreal> * endDegrees = new QList<qreal>();
            QList<QPointF> * endPoints = new QList<QPointF>();
            int index = 0;

            while(toMake->getEndPoint(index)!=NULL)
            {
                if (eventPos.x()<gpi->pos().x()/2)
                {
                    if ((toMake->getType()==HS || toMake->getType()==HE))
                    {
                        if (toMake->getSlotTrackInfo()!=NULL && index > 1 && index%2==0 &&
                            app->getWindow()->getWorkspaceWidget()->getActiveFragment()!=NULL)
                            endDegrees->push_back(-toMake->getTurnAngle(index));
                        else
                            endDegrees->push_back(180-toMake->getTurnAngle(index));
                    }
                    else
                    {
                        if (index%2==0 && toMake->getSlotTrackInfo()!=NULL && index > 1 &&
                            app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                            endDegrees->push_back(180-toMake->getTurnAngle(index));
                        else
                            endDegrees->push_back(-toMake->getTurnAngle(index));
                    }
                }
                else
                {
                    if (index%2==0 && toMake->getSlotTrackInfo()!=NULL && index > 1
                        && app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                        endDegrees->push_back(180+toMake->getTurnAngle(index));
                    else
                        endDegrees->push_back(toMake->getTurnAngle(index));
                }

                if (toMake->getSlotTrackInfo()!=NULL && eventPos.x()<gpi->pos().x()/2)
                {
                    QPointF pt;
                    if (((toMake->getType()==X1 && index>=toMake->getSlotTrackInfo()->numberOfLanes*2)) || ((toMake->getType()==J1 || toMake->getType()==J2) && index==2*toMake->getSlotTrackInfo()->numberOfLanes))
                        pt = QPointF(-toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y());
                    else
                    {
                        pt = QPointF(toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y());
                        if (toMake->getType()==HS || toMake->getType()==HE)
                        {
                            rotatePoint(&pt,180);
                            pt.setY(-pt.y());
                        }
                    }
                    endPoints->push_back(pt);
                }
                else
                    endPoints->push_back(*toMake->getEndPoint(index));


                index++;
            }





/*
            endPoints->push_back(*toMake->getEndPoint(0));
            endPoints->push_back(*toMake->getEndPoint());
*/
            if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
            {


                if (eventPos.x()<gpi->pos().x()/2)
                {
                    //it = new ModelItem(*this->parentItem->getPartNo(),*this->parentItem->getNameEn(),*this->parentItem->getNameCs(),
//                                       *this->parentItem->getEndPoint(0),*this->parentItem->getEndPoint(),-2*this->parentItem->getTurnDegree(),
//                                       -this->parentItem->getRadius(),this->parentItem->getItemWidth(),this->parentItem->getItemHeight(),
//                                       this->parentItem->getProdLine(),app->getWindow()->getWorkspaceWidget());


                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,-toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(-toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                        it->setSlotTrackInfo(toMake->getSlotTrackInfo());
                }
                else
                {
                    //it = new ModelItem(*this->parentItem->getPartNo(),*this->parentItem->getNameEn(),*this->parentItem->getNameCs(),
                    //                   *this->parentItem->getEndPoint(0),*this->parentItem->getEndPoint(),2*this->parentItem->getTurnDegree()
                    //                   ,this->parentItem->getRadius(),this->parentItem->getItemWidth(),this->parentItem->getItemHeight(),
                    //                   this->parentItem->getProdLine(),app->getWindow()->getWorkspaceWidget());

                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                        it->setSlotTrackInfo(toMake->getSlotTrackInfo());
                }

                //no need to generate model with text, because text isn't needed in workspace scene
                it->generate2DModel(false);


                if (eventPos.x()<gpi->pos().x()/2)
                {
                    /*
                    if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                        it->setEndPointAngle(0,180+it->getTurnDegree(0));
                        */
                    if ((!(it->getType()>=T1 && it->getType()<=T10) && app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL))
                        it->setEndPointAngle(0,180+it->getTurnAngle(0));
                }
                else
                {
                    if ((!(it->getType()>=T1 && it->getType()<=T10) && app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL))
                        it->setEndPointAngle(0,180+it->getTurnAngle(0));
                }




                //!eventPos???
                if (toMake->getType()==J2 && toMake->getSlotTrackInfo()==NULL)
                {
                    it->setEndPointAngle(1,180-it->getTurnAngle());
                    /*
                    if (it->getRadius()>0)

                    else
                        it->setEndPointAngle(1,it->getTurnDegree());
                        */
                }

                if(toMake->getType()==J1 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()>gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->numberOfLanes*2,180-it->getTurnAngle(it->getSlotTrackInfo()->numberOfLanes*2));


                if(toMake->getType()==J2 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()>gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->numberOfLanes*2,180-it->getTurnAngle(it->getSlotTrackInfo()->numberOfLanes*2));



                if ((toMake->getType()==X1 || toMake->getType()==J4) && toMake->getSlotTrackInfo()==NULL)
                {
                    if (it->getRadius()<0)
                    {
                    }
                    it->setEndPointAngle(2,180-it->getTurnAngle(2));

                }
                if (toMake->getType()==J5)
                    it->setEndPointAngle(2,180);
                if (it->getType()>=T2 && it->getType()<=T10)
                {
                    int index = 0;
                    while (index!=(it->getType()-8))
                    {
                        it->setEndPointAngle(index,180+it->getTurnAngle(index));
                        index++;
                    }


                }



                //ModelFragmentWidget * fragment = new ModelFragmentWidget(it,this->parentItem->getProdLine(),this->parentItem->getEndPoint(0), endPoints, endDegrees);
                ModelFragment * fragment = new ModelFragment(it);
                /*
                fragment->addEndPoint(it->getEndPoint(0));
                fragment->getEndPointsAngles()->append(it->getTurnDegree(0));

                QPointF * newPoint = app->getWindow()->getWorkspaceWidget()->getActiveEndPoint();
                *newPoint += *it->getEndPoint(0);
                fragment->addEndPoint(newPoint);
                fragment->getEndPointsAngles()->append(it->getTurnDegree(0));

                fragment->addFragmentItem(it,app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
                */

                app->getWindow()->getWorkspaceWidget()->addFragment(fragment);
                app->getWindow()->getWorkspaceWidget()->setActiveFragment(fragment);

                fragment->moveBy(app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->x(),app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->y());

                QPointF * newActive = NULL;

                if (fragment->getEndPoints()->count()>1)
                    newActive = new QPointF(*fragment->getEndPoints()->at(1));
                else
                    newActive = new QPointF(*fragment->getEndPoints()->at(0));

                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newActive);


                //QPointF * newPoint = app->getWindow()->getWorkspaceWidget()->getActiveEndPoint();
                //fragment->moveBy(newPoint->x(),newPoint->y());

                /**
                QPointF * newPoint = app->getWindow()->getWorkspaceWidget()->getActiveEndPoint();
                newPoint->setX(newPoint->x()-diff.x());
                newPoint->setY(newPoint->y()-diff.y());
                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);
                */


                //QPointF * pt = new QPointF(fragment->getEndPoints()->at(1)->x()-newPoint->x(),fragment->getEndPoints()->at(1)->y()-newPoint->y());
                //app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pt);


            }
            ///else activeFragment => addItem at activeEndPoint
            else
            {

                ModelItem * it = NULL;
                if (eventPos.x()<gpi->pos().x()/2)
                {
                    //it = new ModelItem(*this->parentItem->getPartNo(),*this->parentItem->getNameEn(),*this->parentItem->getNameCs(),
//                                       *this->parentItem->getEndPoint(0),*this->parentItem->getEndPoint(),-2*this->parentItem->getTurnDegree(),
//                                       -this->parentItem->getRadius(),this->parentItem->getItemWidth(),this->parentItem->getItemHeight(),
//                                       this->parentItem->getProdLine(),app->getWindow()->getWorkspaceWidget());

                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,-toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(-toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                        it->setSlotTrackInfo(toMake->getSlotTrackInfo());
                }
                else
                {
                    //it = new ModelItem(*this->parentItem->getPartNo(),*this->parentItem->getNameEn(),*this->parentItem->getNameCs(),
                    //                   *this->parentItem->getEndPoint(0),*this->parentItem->getEndPoint(),2*this->parentItem->getTurnDegree()
                    //                   ,this->parentItem->getRadius(),this->parentItem->getItemWidth(),this->parentItem->getItemHeight(),
                    //                   this->parentItem->getProdLine(),app->getWindow()->getWorkspaceWidget());

                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                        it->setSlotTrackInfo(toMake->getSlotTrackInfo());
                }

                //no need to generate model with text, because text isn't needed in workspace scene
                it->generate2DModel(false);

                if (toMake->getType()==J2 && toMake->getSlotTrackInfo()==NULL)
                {
                    if (toMake->getRadius()>0)
                        it->setEndPointAngle(1,180-it->getTurnAngle());
                    else
                        it->setEndPointAngle(1,it->getTurnAngle());
                }

                if(toMake->getType()==J1 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()>gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->numberOfLanes*2,180-it->getTurnAngle(it->getSlotTrackInfo()->numberOfLanes*2));


                if(toMake->getType()==J2 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()>gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->numberOfLanes*2,180-it->getTurnAngle(it->getSlotTrackInfo()->numberOfLanes*2));


                if (toMake->getType()==X1 || toMake->getType()==J4)
                {
                    if (it->getRadius()<0)
                    {
                    }
                    it->setEndPointAngle(2,180-it->getTurnAngle(2));

                }

                if (toMake->getType()==J5)
                    it->setEndPointAngle(2,180);


                if (toMake->getType()==T1)
                    it->rotate(180);

                if (it->getType()>=T2 && it->getType()<=T10)
                {
                    int index = 1;
                    while (index!=(it->getType()-8))
                    {
                        it->setEndPointAngle(index,180+it->getTurnAngle(index));
                        index++;
                    }


                }



                app->getWindow()->getWorkspaceWidget()->getActiveFragment()->addFragmentItem(it,app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());

            }

            app->getWindow()->getWorkspaceWidget()->setLastUsedPart(parentItem);
            app->getWindow()->getWorkspaceWidget()->setLastEventPos(eventPos);
        }
    }


}

void GraphicsPathItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!app->getWindow()->getWorkspaceWidget()->getHeightProfileMode())
        makeNewItem(event->scenePos(),this,this->parentItem,this->parentItem, false);

    //app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->invalidate(this->scene()->sceneRect());

    //this fixes the bug with "no background" of SBW scene
    QBrush b = app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->backgroundBrush();
    b.setColor(QColor(255,255,255));
    app->getWindow()->getSideBarWidget()->getCurrentScene()->setBackgroundBrush(b);

    QGraphicsPathItem::mouseDoubleClickEvent(event);


}

void GraphicsPathItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{

    if (this->dialog==NULL)
        this->initDialog();


    if(this->contextMenuSBW==NULL && this->contextMenuWSW==NULL)
        this->initMenus();



    QPointF pt = event->screenPos();
    QAction * actionSelected = NULL;
    if (this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget())
        actionSelected = this->contextMenuWSW->exec(QPoint(pt.x(),pt.y()));
    else
        actionSelected = this->contextMenuSBW->exec(QPoint(pt.x(),pt.y()));

    if (actionSelected != NULL)
    {
        if (this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget())
        {
            if (this->parentItem->getParentFragment()!=NULL)
                this->parentItem->getParentFragment()->showInfoDialog();
        }
        else
            this->dialog->show();

    }
}

void GraphicsPathItem::keyPressEvent(QKeyEvent *event)
{
    app->sendEvent(app->getWindow()->getWorkspaceWidget(),event);
}

void GraphicsPathItem::updateItem()
{

    unsigned int count = ((QSpinBox*)this->dialog->layout()->itemAt(5)->widget())->value();
    this->parentItem->setAvailableCount(count);
    this->changeCountPath(count,this->parentItem->getRadius());
    this->dialog->close();
}




ProductLine::ProductLine(QString &name,QString &scale, ScaleEnum s, QString &gauge, bool type, QList<ModelItem*> &items)
{
    this->name=new QString(name);
    this->scale=new QString(scale);
    this->scaleE=s;
    this->type=type;
    this->gauge=new QString(gauge);
    this->items=new QList<ModelItem*>(items);
    this->maxTrackRadius=0;
    this->minTrackRadius=50000;

}
ProductLine::ProductLine(QString &name,QString &scale, ScaleEnum s, QString &gauge, bool type)
{
    this->name=new QString(name);
    this->scale=new QString(scale);
    this->scaleE=s;
    this->type=type;
    this->items = new QList<ModelItem*>();
    this->gauge=new QString(gauge);
    this->maxTrackRadius=0;
    this->minTrackRadius=50000;
}


QString * ProductLine::getName() const
{
    return this->name;
}

QString * ProductLine::getScale() const
{
    return this->scale;
}

ScaleEnum ProductLine::getScaleEnum() const
{
    return this->scaleE;
}

bool ProductLine::getType() const
{
    return this->type;
}

QList<ModelItem*> * ProductLine::getItemsList() const
{
    return this->items;
}

int ProductLine::setItemsList(QList<ModelItem*>* list)
{
    return 0;
}

int ProductLine::addItem(ModelItem* item)
{
    if (item==NULL)
        return 1;
    this->items->push_back(item);
    if (item->getRadius()>this->maxTrackRadius)
        this->maxTrackRadius=item->getRadius();

    if(item->getRadius()<this->minTrackRadius)
        this->minTrackRadius=item->getRadius();

    return 0;
}

qreal pointStress(QPointF * point1, QPointF * point2)
{
    qreal dx = point1->x()-point2->x();
    qreal dy = point1->y()-point2->y();
    qreal result = sqrt(dx*dx+dy*dy);
    return result;
}

qreal angleStress(qreal a1, qreal a2)
{

    int x = a1;
    int y = a2;
    int a = x % 360;
    int b = y % 360;
    int result = abs(a-b) % 360;
    if (a>b)
        return result;
    else
        return -result;
}

ModelFragment::ModelFragment()
{
    this->endPoints = new QList<QPointF *>();
    this->endPointsAngles = new QList <qreal>();
    this->fragmentItems = new QList <ModelItem*>();
    this->lines = new QList <ProductLine*>();
    //this->endPointsGraphics=new QList<QGraphicsPathItem*>();
    this->endPointsGraphics=new QList<QGraphicsEllipseItem*>();
    this->infoDialog = NULL;


}

ModelFragment::ModelFragment(ModelItem *item)
{
    logFile << "ModelFragment constructor" << endl;
    this->endPoints = new QList<QPointF*>();


    /**
      TODO
      -switch type
     */

    this->endPointsGraphics=new QList<QGraphicsEllipseItem*>();
    this->endPointsAngles = new QList<qreal>();
    this->endPointsItems = new QList<ModelItem*>();

    int index = 0;
    while (item->getEndPoint(index)!=NULL)
    {
        this->addEndPoint(item->getEndPoint(index));
        this->endPointsAngles->push_back(item->getTurnAngle(index));
        this->endPointsItems->push_back(item);
        index++;
    }

    this->fragmentItems = new QList <ModelItem*>();
    this->lines = new QList <ProductLine*>();

    this->fragmentItems->push_back(item);
    this->lines->push_back(item->getProdLine());
    item->setParentFragment(this);

    this->infoDialog = NULL;
    //this->transformMatrix = NULL;


}

ModelFragment::~ModelFragment()
{
    logFile << "~ModelFragment" << endl;
    //contains pointers to QPointFs owned by ModelItems -> delete just pointers
    delete this->endPoints;
    this->endPoints=NULL;
    delete this->endPointsAngles;
    this->endPointsAngles=NULL;
    //delete this->endPointsGraphics items through scene::removeItem function


    for (int i = 0; i < this->endPointsGraphics->count();i++)
    {
        app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(this->endPointsGraphics->at(i));
        delete this->endPointsGraphics->at(i);
        //this->endPointsGraphics->at(i)=NULL;
    }
    delete this->endPointsGraphics;
    this->endPointsGraphics=NULL;

    //contains pointers to ModelItems from fragmentItems list -> delete just pointers
    delete this->endPointsItems;
    this->endPointsItems=NULL;

    QList<ModelItem*>::Iterator it = this->fragmentItems->begin();
    for (int i = 0; i < this->fragmentItems->count();i++)
    {
        delete this->fragmentItems->at(i);
        *it=NULL;
        it++;
        //this->fragmentItems->at(i)=NULL;
    }
    delete this->fragmentItems;
    this->fragmentItems=NULL;

    //dont delete this - it is owned by the QMainWindow or WorkspaceWidget
    //delete this->infoDialog;

    //contains pointers to ProductLines, which will be probably re-used -> delete just pointers
    delete this->lines;
    this->lines=NULL;
}
/*
ModelFragmentWidget::ModelFragmentWidget(ModelItem* item, ProductLine* line, QPointF * startPt,QList<QPointF *> *endPts,QList<qreal> * endPtAngles)
{
    this->endPoints = endPts;
    this->endPoints->push_front(startPt);
    this->endPointsAngles = endPtAngles;
    this->endPointsAngles->push_front(0);
    this->fragmentItems = new QList <ModelItem*>();
    this->lines = new QList <ProductLine*>();
    ////this->startPoint = startPt;
    ////this->startPointAngle = 0;
    this->infoDialog = NULL;

    //ModelFragment constructor can't allocate new ModelItem (which will be then inserted in the scene) - it has to be done in the doubleclickevent
    //because of left/right turn rotation
    //ModelItem * it = new ModelItem(*item);
    this->transformMatrix = new QTransform();

    this->fragmentItems->push_back(item);
    this->lines->push_back(line);
    item->setParentFragment(this);

}
*/

QList <ModelItem*> * ModelFragment::getFragmentItems() const
{
    return this->fragmentItems;
}

QList<ProductLine*> * ModelFragment::getProductLines() const
{
    return this->lines;
}

/** **
QPointF * ModelFragmentWidget::getStartPoint() const
{
    return this->startPoint;
}
*/
QList<QPointF *> * ModelFragment::getEndPoints() const
{
    return this->endPoints;
}

QList<QGraphicsEllipseItem *> *ModelFragment::getEndPointsGraphics() const
{
    return this->endPointsGraphics;

}

/** **
qreal ModelFragmentWidget::getStartPointAngle()
{
    return this->startPointAngle;
}
*/
QList<qreal> * ModelFragment::getEndPointsAngles() const
{
    return this->endPointsAngles;
}

QList<ModelItem *> *ModelFragment::getEndPointsItems() const
{
    return this->endPointsItems;
}

int ModelFragment::addFragmentItems(QList <ModelItem*> * listToAppend)
{
    if (listToAppend==NULL)
        return 1;
    //this->fragmentItems->append(*listToAppend);
    //while loop - call addFragItem(iter,??which point??);
    return 0;
}
/*
void fixAngles(ModelFragment * fragment, int lane)
{
    qreal currentRotation = fragment->getFragmentItems()->first()->getTurnDegree(2*lane-1);
    //QList<ModelItem*>::Iterator itemIter = fragment->getFragmentItems()->begin();
    //(*itemIter)->setEndPointAngle(2*lane-2,(*itemIter)->getTurnDegree(2*lane-2)-180);
    ModelItem * item = fragment->getFragmentItems()->first();
    ModelItem * previousItem = item;


    //item->setEndPointAngle(2*lane-2,item->getTurnDegree(2*lane-2)-180);

    item = item->getNeighbour(2*lane-1);

    //QPointF pointOfConnection = item->getEndPoint(2*lane-1);

    logFile << "FixAngles of lane " << lane << endl;

    while (item!=fragment->getFragmentItems()->first())
    {
        //find the point of connection
        int i = 0;
        while(item->getEndPoint(i)!=NULL)
        {
            if (item->getNeighbour(item->getEndPoint(i))==previousItem)
            {
                //item->setEndPointAngle(i,currentRotation-item->getTurnDegree(i+1));
                qreal currentRotationBefore = currentRotation;

                if (i%2==0)
                {
                    if (((item->getTurnDegree(i+1)-item->getTurnDegree(i))==180 && item->getType()!=T1))
                        currentRotation+=abs(item->getTurnDegree(i+1));
                    else
                        currentRotation+=(abs(item->getTurnDegree(i+1))-abs(item->getTurnDegree(i)));
                }
                else
                {
                    if (((item->getTurnDegree(i-1)-item->getTurnDegree(i))==180 && item->getType()!=T1))
                        currentRotation+=abs(item->getTurnDegree(i-1));
                    else
                        currentRotation+=(abs(item->getTurnDegree(i-1))-abs(item->getTurnDegree(i)));
                }

                if (currentRotation>180)
                    currentRotation-=180;

                item->setEndPointAngle(i,currentRotationBefore);
                if (i%2==0)
                    item->setEndPointAngle(i+1,currentRotation);
                else
                    item->setEndPointAngle(i-1,currentRotation);
                //debug here, because angles are not set correctly - check currentRotation variable

                previousItem=item;
                if (i%2==0)
                    item=item->getNeighbour(i+1);
                else
                    item=item->getNeighbour(i-1);
                logFile << "    current rotation is " << currentRotation << endl;

                break;
            }
            i++;

        }


    }



    return;
}
*/

int ModelFragment::addFragmentItem(ModelItem* item,QPointF * point, int insertionIndex)
{
    if (item == NULL)
        return 1;
    logFile << "Adding item " << item->getPartNo()->toStdString() << " at point [" << point->x() << "," << point->y() << "] with index " << insertionIndex << " and address " << item << " into fragment with id " << app->getWindow()->getWorkspaceWidget()->getFragmentIndex(this) << endl;

    logFile << "    Item has endpoints:" << endl;
    int xyz = 0;
    while (item->getEndPoint(xyz)!=NULL)
    {
        logFile << "        [" << xyz << "]: [" << item->getEndPoint(xyz)->x() << ", " << item->getEndPoint(xyz)->y() << "] at " << item->getEndPoint(xyz) << " and with rotation of " << item->getTurnAngle(xyz) << "° and Neighbour is at " << item->getNeighbour(xyz) << endl;
        xyz++;
    }

    logFile << "    EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;

    /*
     *How it works:
     *-item is inserted at point's value
     *-item's endPoint[0] is used to connect with the rest of the fragment
     *-item's endPoint[1] is used as a new active point
     *-all remaining endPoints are added as fragments' endpoints
    */


    item->setParentFragment(this);
    this->fragmentItems->push_back(item);

    //find fragment's endPoint which equals to "point" parameter and remove it - it can't be used as endPoint anymore
    qreal currentFragmentRotation = 0;
    ModelItem * endPointItem = NULL;
    QList<QPointF*>::Iterator iter = this->endPoints->begin();
    int i = 0;

    QRectF insertionArea(QPointF(point->x()-TOLERANCE_HALF,point->y()-TOLERANCE_HALF),QSizeF(TOLERANCE_HALF*2,TOLERANCE_HALF*2));


    while(iter!=this->endPoints->end())
    {

        //if (**iter==*point)
        if (insertionArea.contains(**iter))
        {
            //get frag. rotation and then remove it from the list - won't be used
            currentFragmentRotation = this->endPointsAngles->at(i);
            endPointItem = this->endPointsItems->at(i);

            if (180==abs(currentFragmentRotation-(item->getTurnAngle(insertionIndex))))
            {
                cout << "watch out" << endl;
            }

            this->removeEndPoint(*iter);
            break;
        }
        iter++;
        i++;
    }







    //get start-point-rotation of inserted item and subtract it from currentFragmentRotation
    //^^-result==0 -> no rotation is needed
    if ((currentFragmentRotation-item->getTurnAngle(insertionIndex)!=0) && endPointItem!=NULL)
    {
        if (180==abs(currentFragmentRotation-(item->getTurnAngle(insertionIndex))))
        {
            cout << "watch out" << endl;
        }

        //rotate 2D model and modify item's attributes
        item->rotate(currentFragmentRotation-(item->getTurnAngle(insertionIndex)));
        item->updateEndPointsHeightGraphics();
    }

    //cout << "Debug: " << item->getType() << ", rotated by:" <<currentFragmentRotation-(item->getTurnDegree(0)) << endl;


    QPointF pointOfInsertion(*point);



    //by default item's endPoints are in local item's coordinates (center of item=[0,0] => without -(i.EP(0)) "point" parameter would be in the [0,0] of "item" parameter
    *point-=*item->getEndPoint(insertionIndex);
    qreal dx = point->x();
    qreal dy = point->y();

    //this fixes the strange behaviour of T1 2D Model
    if (item->getType()==T1)
    {
        item->rotate(180);
        item->moveBy(-2*item->getEndPoint(insertionIndex)->x(),-2*item->getEndPoint(insertionIndex)->y());

    }

    item->moveBy(dx,dy);


    //inserted item has been moved so it can finally be checked (because its points are now in global coords) whether more frag.endPoints can be removed
    QList<int> doNotAddThese;
    if (item->getSlotTrackInfo()!=NULL)
    {
        iter=this->endPoints->begin();
        QList<ModelItem*>::Iterator ePIIter = this->endPointsItems->begin();
        while (iter!=this->endPoints->end())
        {
            QRectF r(QPointF((*iter)->x()-TOLERANCE_HALF,(*iter)->y()-TOLERANCE_HALF),QSizeF(2*TOLERANCE_HALF,2*TOLERANCE_HALF));
            for (int j = 0; j < 2*item->getSlotTrackInfo()->numberOfLanes; j++)
            {

                //if (**iter==*item->getEndPoint(j))
                if (r.contains(*item->getEndPoint(j)))
                {

                    //item->setNeighbour(*ePIIter,j);
                    this->removeEndPoint(*iter);
                    /**
                      this should be changed - why?
*/
                    iter=this->endPoints->begin();
                    ePIIter=this->endPointsItems->begin();
                    doNotAddThese.push_back(j);
                    iter--;
                    ePIIter--;
                    break;
                }
            }
            iter++;
            ePIIter++;
        }
    }
    else
    {
        iter=this->endPoints->begin();
        while (iter!=this->endPoints->end())
        {
            QRectF r(QPointF((*iter)->x()-TOLERANCE_HALF,(*iter)->y()-TOLERANCE_HALF),QSizeF(2*TOLERANCE_HALF,2*TOLERANCE_HALF));
            for (int j = 0; item->getEndPoint(j)!=NULL; j++)
            {
                //if (**iter==*item->getEndPoint(j))
                if (r.contains(*item->getEndPoint(j)))
                {
                    this->removeEndPoint(*iter);

                    iter=this->endPoints->begin();
                    doNotAddThese.push_back(j);
                    iter--;
                    break;
                }
            }
            iter++;
        }
    }

    doNotAddThese.push_back(insertionIndex);

    qreal newFragmentRotation = insertionIndex%2==0 ? item->getTurnAngle(insertionIndex+1) : item->getTurnAngle(insertionIndex-1);



    if (insertionIndex%2==0)
    {
        if (!doNotAddThese.contains(insertionIndex+1))
            this->endPointsAngles->push_back(newFragmentRotation);
    }
    else
    {
        if (!doNotAddThese.contains(insertionIndex-1))
            this->endPointsAngles->push_back(newFragmentRotation);
    }
    /*
    else
    {
        int index = 2;
        while (item->getEndPoint(index)!=NULL)
        {
            if (!doNotAddThese.contains(index))
            {
                this->endPointsAngles->push_back(item->getTurnDegree(index));
                break;
            }
            index++;
        }
    }
*/

    //set height profile of inserted item to h.p. of endPoint at which the item is being inserted
    int dz = endPointItem==NULL ? 0 : endPointItem->getHeightProfileAt(&pointOfInsertion);
    item->adjustHeightProfile(dz,&pointOfInsertion);
    if (item->getType()==C1 || item->getType()==S1 || item->getType()==C2)
        insertionIndex%2==0 ? item->adjustHeightProfile(dz,item->getEndPoint(insertionIndex+1)) : item->adjustHeightProfile(dz,item->getEndPoint(insertionIndex-1));



    if (item->getSlotTrackInfo()!=NULL)
    {

        //for each remaining lane: modify height - adjust it by the height of fragments height at the same endpoint
        for (int k = 2; k < 2*item->getSlotTrackInfo()->numberOfLanes; k+=2)
        {

            qreal dzLane = endPointItem==NULL ? 0 : endPointItem->getHeightProfileAt(item->getEndPoint(k));
            item->adjustHeightProfile(dzLane,item->getEndPoint(k));
            item->adjustHeightProfile(dzLane,item->getEndPoint(k+1));


        }
        //*/

    }

    //when rebuilding the fragment during the deletion both neighbours of one lane might be notNull and it will cause
    //that the "connected" variable will be true
    bool wasNotConnectedBefore = true;

    int count = 1;
    if (item->getSlotTrackInfo()!=NULL)
        count = item->getSlotTrackInfo()->numberOfLanes;

    for (int i = 1; i < 2*count; i+=2)
    {
        if (item->getNeighbour(i)!=NULL)
            wasNotConnectedBefore=false;
    }


    //set neighbours of both endItem and inserted item

    if (endPointItem!=NULL)
    {
        if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItem->get2DModelNoText()->isSelected()))
        {}
        else
        {
            if (endPointItem!=NULL)
                    endPointItem->setNeighbour(item,&pointOfInsertion);
            if (item->getType()!=T1)
                item->setNeighbour(endPointItem,&pointOfInsertion);
            else
                item->setNeighbour(endPointItem,item->getEndPoint(insertionIndex));
        }
    }







    ///TODO: MOVE IT
    //update graphic representation of fragment
    app->getWindow()->getWorkspaceWidget()->updateFragment(this);


    QPointF * newPt = NULL;

    /**
      TODO
      -change it so that if [1] is not usable, next usable ep will be used as newPt
* /
    ///if doNotAddThese contains(1) do something more than this
    ///

    if (insertionIndex%2 == 0)
    {
        if (doNotAddThese.contains(insertionIndex+1))
        {
            ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(insertionIndex+1));

            item->setNeighbour(endPointItemOdd,item->getEndPoint(insertionIndex+1));
            endPointItemOdd->setNeighbour(item,item->getEndPoint(insertionIndex+1));
        }
    }
    else
    {
        if (doNotAddThese.contains(insertionIndex-1))
        {
            ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(1));

            item->setNeighbour(endPointItemOdd,item->getEndPoint(insertionIndex-1));
            endPointItemOdd->setNeighbour(item,item->getEndPoint(insertionIndex-1));
        }
    }

    ///*/

    if (item->getEndPoint()!=NULL)
    {
        insertionIndex%2==0 ? newPt = new QPointF(item->getEndPoint(insertionIndex+1)->x(),item->getEndPoint(insertionIndex+1)->y()) : newPt = new QPointF(item->getEndPoint(insertionIndex-1)->x(),item->getEndPoint(insertionIndex-1)->y());
    }


//there may be a mistake in following code - slot track with odd number of ePs is wrong





///logFile << "    AAAA EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;

/*
    if (newPt!=NULL)
    {
        //check if fragment endPoints can be connected
        for (int i = 0; i < this->endPoints->count(); i++)
        {
            //trackStress(), pointStress(), involve also endpoint angles
            if (pointStress(this->endPoints->at(i),newPt)<15)
            {
                //qreal aS = angleStress(this->endPointsAngles->at(i)+180,newFragmentRotation);
                qreal aS = angleStress(this->endPointsAngles->at(i),newFragmentRotation);
                if (aS < 5)
                {
                    //close the fragment
                    ///connected = true;
                    ///this->removeEndPoint(this->endPoints->at(i));
                    //fix the appearance of fragment
                    //if (aS > pS) -> rotate else move
                }
            }

        }
    }
*/
    if (newPt==NULL)// || doNotAddThese.contains(insertionIndex+1))
    {/*
        if (newPt!=NULL)
            *newPt=QPointF(0,0);
        else*/
        newPt = new QPointF(0,0);

        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
        this->endPointsAngles->removeOne(newFragmentRotation);
    }
    else
    {
        //-1 is missing
        if (!doNotAddThese.contains(insertionIndex+1))
            app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        else
        {
            if (item->getSlotTrackInfo()!=NULL)
            {
                int n = insertionIndex+3;
                while (item->getEndPoint(n)!=NULL)
                {
                    if (!doNotAddThese.contains(n))
                    {
                        *newPt=QPointF(*item->getEndPoint(n));
                        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
                        break;
                    }
                    n+=2;
                }
                if ((item->getEndPoint(n)==NULL))
                {
                    *newPt = QPointF(0,0);
                    app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
                    app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
                    //this->endPointsAngles->removeOne(newFragmentRotation);
                }
            }
            else
            {
                int n = insertionIndex+2;
                while (item->getEndPoint(n)!=NULL)
                {
                    if (!doNotAddThese.contains(n))
                    {
                        *newPt=QPointF(*item->getEndPoint(n));
                        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
                        break;
                    }
                    n++;
                }
                if ((item->getEndPoint(n)==NULL))
                {
                    *newPt = QPointF(0,0);
                    app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
                    app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
                    //this->endPointsAngles->removeOne(newFragmentRotation);
                }
            }
        }

///logFile << "    BBBB EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;

        if (insertionIndex%2==0)
        {
            if (item->getEndPoint(insertionIndex+1)!=NULL && !doNotAddThese.contains(insertionIndex+1))
            {
                this->addEndPoint(item->getEndPoint(insertionIndex+1));
                this->endPointsItems->push_back(item);
                doNotAddThese.push_back(insertionIndex+1);
            }
        }
        else
        {
            if (item->getEndPoint(insertionIndex-1)!=NULL && !doNotAddThese.contains(insertionIndex-1))
            {
                this->addEndPoint(item->getEndPoint(insertionIndex-1));
                this->endPointsItems->push_back(item);
                doNotAddThese.push_back(insertionIndex-1);
            }
        }

///logFile << "    CCCC EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;


        //this is needed for correct rebuilding of fragment
        /*
          TODO:
          modify the condition
          * /

        qreal dA = (item->getTurnDegree(1)-item->getTurnDegree(0));
        dA = dA /2.0;
        dA = abs(dA);

        if (item->getNeighbour(insertionIndex)==NULL &&
        //        (180!=abs((abs(item->getTurnDegree(insertionIndex)))-abs(item->getTurnDegree(insertionIndex+1))))
            (((item->getTurnDegree(0)+2*dA)-item->getTurnDegree(1))==0)
                )
            item->setEndPointAngle(insertionIndex,180+item->getTurnDegree(insertionIndex));

*/
        /*
////
        qreal dA = (item->getTurnDegree(insertionIndex+1)-item->getTurnDegree(insertionIndex));
        dA = dA /2.0;
        dA = abs(dA);
        if ((abs(item->getTurnDegree(insertionIndex+1))-abs(item->getTurnDegree(insertionIndex)))>=0)//(dA>=0)//(item->getTurnDegree(insertionIndex+1)>item->getTurnDegree(insertionIndex))
        {

            dA = abs(dA);
            qreal result =((item->getTurnDegree(insertionIndex)+2*dA)-item->getTurnDegree(insertionIndex+1));
            if (result==0 && item->getNeighbour(insertionIndex)==NULL)
                item->setEndPointAngle(insertionIndex,180+item->getTurnDegree(insertionIndex));
        }
        ///is this condition always correct?
        else if (dA<90) //dA=90 means it was =180
        {
            //qreal dA = (item->getTurnDegree(insertionIndex+1)-item->getTurnDegree(insertionIndex));
            //dA = dA /2.0;
            dA = abs(dA);
            qreal result =((item->getTurnDegree(insertionIndex+1)+2*dA)-item->getTurnDegree(insertionIndex));
            if (result==0 &&  item->getNeighbour(insertionIndex)==NULL)
                item->setEndPointAngle(insertionIndex,180+item->getTurnDegree(insertionIndex));
        }
////
*/
/*
 *this shouldnt be done, because it sets 180 difference to each inserted item which is wrong
        if (!item->leftRightDifference180(insertionIndex,insertionIndex+1))
            item->setEndPointAngle(insertionIndex,180+item->getTurnDegree(insertionIndex));
*/

        //add all remaining endPoint's and modify slot track neighbours for other lanes
        int index = 0;
        while (item->getEndPoint(index)!=NULL)
        {
            if (!doNotAddThese.empty())
            {
                if (!doNotAddThese.contains(index))
                {
                    /*
                      TODO:
                      modify the condition
                      * /
                    qreal dA = (item->getTurnDegree(index+1)-item->getTurnDegree(index));
                    dA = dA /2.0;
                    dA = abs(dA);

                    if (index%2==0 && item->getNeighbour(index)==NULL &&
                            //(180!=abs((abs(item->getTurnDegree(index)))-abs(item->getTurnDegree(index+1))))
                        (((item->getTurnDegree(index)+2*dA)-item->getTurnDegree(index+1))==0)
                            )
                        item->setEndPointAngle(index,180+item->getTurnDegree(index));*/
////
/*                    qreal dA = (item->getTurnDegree(index+1)-item->getTurnDegree(index));
                    dA = dA /2.0;
                    dA = abs(dA);
                    if ((abs(item->getTurnDegree(index+1))-abs(item->getTurnDegree(index)))>=0)//(dA>=0)//(item->getTurnDegree(index+1)>item->getTurnDegree(index))
                    {

                        dA = abs(dA);
                        qreal result =((item->getTurnDegree(index)+2*dA)-item->getTurnDegree(index+1));
                        if (index%2==0 && item->getNeighbour(index)==NULL && result==0)
                            item->setEndPointAngle(index,180+item->getTurnDegree(index));
                    }
                    ///is this condition always correct?
                    else if (dA<90) //dA=90 means it was =180
                    {
                        //qreal dA = (item->getTurnDegree(index+1)-item->getTurnDegree(index));
                        //dA = dA /2.0;
                        dA = abs(dA);
                        qreal result =((item->getTurnDegree(index+1)+2*dA)-item->getTurnDegree(index));
                        if (index%2==0 && item->getNeighbour(index)==NULL && result==0)
                            item->setEndPointAngle(index,180+item->getTurnDegree(index));
                    }
                    */
                    if (index%2==0 && item->getNeighbour(index)==NULL && !item->leftRightDifference180(index,index+1))
                        item->setEndPointAngle(index,180+item->getTurnAngle(index));

////


                    this->addEndPoint(item->getEndPoint(index));
                    this->endPointsAngles->push_back(item->getTurnAngle(index));
                    this->endPointsItems->push_back(item);
                }
                else if (index!=insertionIndex)
                {

/**
                      TODO
                      -THIS IS WRONG FOR RAIL PARTS - NEIGHBOURS ARE INCORRECT (also odd indices for slot track?)
                      -angles are not set correctly
                      */

                    doNotAddThese.removeOne(index);
                    //for slot track
                    if (index%2==0)
                    {
                        if (endPointItem!=NULL)
                        {
                            if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItem->get2DModelNoText()->isSelected()))
                            {}
                            else
                            {
                                if (item->getNeighbour(item->getEndPoint(index))==NULL)
                                item->setNeighbour(endPointItem,item->getEndPoint(index));

                                //if (endPointItem!=NULL)
                                endPointItem->setNeighbour(item,item->getEndPoint(index));

                                cout << "place breakpoint here" << endl;
                            }

                        }


                    //this fixes angles which have been rotated by 180 in makeNewItem function, but were in doNotUse list
                    ///item->setEndPointAngle(index,item->getTurnDegree(index)-180);

/*                    int x = 0;
                    while (endPointItem->getEndPoint(x)!=NULL)
                    {
                        if (*endPointItem->getEndPoint(x)==pointOfInsertion)
                        {

                            break;
                        }
                        x++;
                    }
                    x = 0+x%2;
                    while(endPointItem->getEndPoint(x)!=NULL)
                    {
                        endPointItem->setEndPointAngle(x,endPointItem->getTurnDegree(x)-180);
                        x++;
                    }*/
                    }
                    else if (item->getNeighbour(index)==NULL)
                    {

                        ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(index));

                        if (endPointItemOdd!=item)
                        {
                            if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItemOdd->get2DModelNoText()->isSelected()))
                            {}
                            else
                            {
                                item->setNeighbour(endPointItemOdd,item->getEndPoint(index));
                                if (endPointItemOdd!=NULL)// && endPointItemOdd!=item)
                                    endPointItemOdd->setNeighbour(item,item->getEndPoint(index));
                            }
                        }
                    }
                    //for rail find neighbour at point
                    //
                }
            }
            else
            {

                this->addEndPoint(item->getEndPoint(index));
                this->endPointsAngles->push_back(item->getTurnAngle(index));
                this->endPointsItems->push_back(item);
            }
            index++;
        }
    }

///logFile << "    DDDD EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;

    bool connected = false;
    int firstConnectedIndex = 0;
    QList<int> processedLanes;
    while (item->getEndPoint(firstConnectedIndex)!=NULL)
    {
        if (item->getNeighbour(firstConnectedIndex)!=NULL && item->getNeighbour(firstConnectedIndex)!=item->getNeighbour(0) && item->getNeighbour(0)!=NULL)
        {
            connected=true;
            ////
            connected = connected && wasNotConnectedBefore;
            //move to while loop above
            if(connected)
            {

                qreal itemRotation = item->getTurnAngle()-item->getTurnAngle(0);
                if (abs(itemRotation)==180 && item->getType()!=T1)
                    itemRotation=0;

                qreal neighConnectionIndex = 0;

                while(item->getNeighbour(0)->getNeighbour(neighConnectionIndex)!=item)
                {
                    neighConnectionIndex++;
                }

                ///item->setEndPointAngle(0,item->getNeighbour(0)->getTurnDegree(neighConnectionIndex));
                ///item->setEndPointAngle(firstConnectedIndex,item->getNeighbour(0)->getTurnDegree(neighConnectionIndex)+180+itemRotation);



                /*
                ModelItem * item2 = item;
                ModelItem * previousItem = item2;

                item2 = item2->getNeighbour(firstConnectedIndex);

                int laneIndex = firstConnectedIndex%2==0 ? firstConnectedIndex/2+1 : (firstConnectedIndex+1)/2;

                while (item2!=this->getFragmentItems()->first())
                {
                    //find the point of connection
                    int j = 0;
                    while(item2->getEndPoint(j)!=NULL)
                    {
                        if (item2->getNeighbour(item2->getEndPoint(j))==previousItem)
                        {
                            laneIndex = j%2==0 ? j/2+1 : (j+1)/2;
                            previousItem=item2;
                            item2=item2->getNeighbour(j+1);

                            break;
                        }
                        j++;

                    }


                }
                int j = 0;
                while (item2->getNeighbour(j)!=previousItem)
                {
                    j++;
                    if (item2->getNeighbour(j)==previousItem && processedLanes.contains(j))
                        j++;
                }

                laneIndex = j%2==0 ? j/2+1 : (j+1)/2;
                processedLanes.push_back(j);
                //fixAngles(this,laneIndex);*/
            }
            ////

            //break;
        }
        firstConnectedIndex++;
    }


    if (this->endPoints->empty())
    {
        newPt = new QPointF(0,0);

        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);

    }



    logFile << "    Item has endpoints:" << endl;
    xyz = 0;
    while (item->getEndPoint(xyz)!=NULL)
    {
        logFile << "        [" << xyz << "]: [" << item->getEndPoint(xyz)->x() << ", " << item->getEndPoint(xyz)->y() << "] at " << item->getEndPoint(xyz) << " and with rotation of " << item->getTurnAngle(xyz) << "° and Neighbour is at " << item->getNeighbour(xyz) << endl;
        xyz++;
    }

logFile << "    EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;


       return 0;
}

/**
  TODO
  -check conditions using dA variable for angles which are larger on the left side of item
*/

void recursivelyAdd(ModelItem * item, ModelFragment * fragment, QPointF *pt)
{
    if (item->getParentFragment()!=fragment)
    {
        QPointF * newPt = new QPointF(*pt);
        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);

        int n = 0;
        QRectF rPt(QPointF(pt->x()-TOLERANCE_HALF,pt->y()-TOLERANCE_HALF),QSizeF(2*TOLERANCE_HALF,2*TOLERANCE_HALF));
        while(item->getEndPoint(n)!=NULL)
        {
            //if (*item->getEndPoint(n)==*pt)
            if (rPt.contains(*item->getEndPoint(n)))
                break;
            n++;
        }





        if (item->getEndPoint(n)==NULL)
        {
            logFile << "  problem in recursion with item at address " << item  << endl;
            return;
        }

        if (n==0)
        {
            //add item to fragment;
            fragment->addFragmentItem(item,newPt);
            cout << "Item No. " << item->getPartNo()->toStdString() << " was added at point " << newPt->x() << "," << newPt->y()  << " in fragment at address " << fragment << endl;
        }
        else
        {
///see paper and debug this part of the function

            ModelItem * neigh = item;
            while (true)
            {
                //if neighbour at point [0] has 180 difference of angles
                    //enter the recursion


                //if neigh has no neighbour add it
                if (neigh->getNeighbour(0)==NULL)
                {
                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getTurnAngle(0)<< " -> " << neigh->getTurnAngle(1)<< " will be added." <<endl;
                    recursivelyAdd(neigh,fragment,neigh->getEndPoint(0));
                    fragment->addEndPoint(neigh->getEndPoint(0),true,neigh->getTurnAngle(0),neigh);
                    break;
                }
                if (neigh->getNeighbour(0)->getNeighbour(0)==neigh && !neigh->leftRightDifference180(0,1))
                {
                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getTurnAngle(0)<< " -> " << neigh->getTurnAngle(1)<< " will be added." <<endl;
                    recursivelyAdd(neigh,fragment,neigh->getEndPoint(0));
                    fragment->addEndPoint(neigh->getEndPoint(0),true,neigh->getTurnAngle(0),neigh);
                    break;
                }


                //if neighbour[0] of neigh has difference of 180 add it

                if (neigh->getNeighbour(0)->getParentFragment()==fragment)
                {
                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getTurnAngle(0)<< " -> " << neigh->getTurnAngle(1)<< " will be added." <<endl;
                    recursivelyAdd(neigh,fragment,neigh->getEndPoint(0));                    fragment->addEndPoint(neigh->getEndPoint(0),true,neigh->getTurnAngle(0),neigh);
                    break;
                }

                if (neigh->getNeighbour(0)->leftRightDifference180(0,1)

                        )
                {


                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getNeighbour(0)->getTurnAngle(0)<< " -> " << neigh->getNeighbour(0)->getTurnAngle(1)<< " will be added." <<endl;
                    recursivelyAdd(neigh->getNeighbour(0),fragment,neigh->getNeighbour(0)->getEndPoint(0));
                    fragment->addEndPoint(neigh->getNeighbour(0)->getEndPoint(0),true,neigh->getNeighbour(0)->getTurnAngle(0),neigh->getNeighbour(0));
                    break;
                }
                //else if neighbour[0] doesn't have the 180 diff, but satisfies condition below, add it.
                //both neigh and neighbour[0] dont have the 180 diff
                /**/

                else
                {
                    neigh = neigh->getNeighbour(0);
                }

/**
                        qreal dA = (neigh->getTurnDegree(0+1)-neigh->getTurnDegree(0));
                            dA = dA /2.0;
                            dA = abs(dA);
                            if ((abs(neigh->getTurnDegree(0+1))-abs(neigh->getTurnDegree(0)))>=0)//(dA>=0)//(neigh->getTurnDegree(0+1)>neigh->getTurnDegree(0))
                            {
                                /*

                                dA = abs(dA);
                                qreal result =((neigh->getTurnDegree(0)+2*dA)-neigh->getTurnDegree(0+1));
                                if (result!=0)
                                {
                                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getNeighbour(0)->getTurnDegree(0)<< " -> " << neigh->getNeighbour(0)->getTurnDegree(1)<< " will be added." <<endl;
                                    recursivelyAdd(neigh->getNeighbour(0),fragment,neigh->getNeighbour(0)->getEndPoint(0));
                                    fragment->addEndPoint(neigh->getNeighbour(0)->getEndPoint(0),true,neigh->getNeighbour(0)->getTurnDegree(0),neigh->getNeighbour(0));
                                    break;
                                }* /
                                if (neigh->leftRightDifference180(0,1))
                                {
                                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getNeighbour(0)->getTurnDegree(0)<< " -> " << neigh->getNeighbour(0)->getTurnDegree(1)<< " will be added." <<endl;
                                    recursivelyAdd(neigh->getNeighbour(0),fragment,neigh->getNeighbour(0)->getEndPoint(0));
                                    fragment->addEndPoint(neigh->getNeighbour(0)->getEndPoint(0),true,neigh->getNeighbour(0)->getTurnDegree(0),neigh->getNeighbour(0));
                                    break;
                                }
                                else
                                {
                                    neigh = neigh->getNeighbour(0);
                                }
                            }
                            ///is this condition always correct?
                            else //dA=90 means it was =180
                            {/*
                                //qreal dA = (->getTurnDegree(0+1)-->getTurnDegree(0));
                                //dA = dA /2.0;
                                dA = abs(dA);
                                qreal result =((neigh->getTurnDegree(0+1)+2*dA)-neigh->getTurnDegree(0));
                                if (result!=0)
                                {
                                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getNeighbour(0)->getTurnDegree(0)<< " -> " << neigh->getNeighbour(0)->getTurnDegree(1)<< " will be added." <<endl;
                                    recursivelyAdd(neigh->getNeighbour(0),fragment,neigh->getNeighbour(0)->getEndPoint(0));
                                    fragment->addEndPoint(neigh->getNeighbour(0)->getEndPoint(0),true,neigh->getNeighbour(0)->getTurnDegree(0),neigh->getNeighbour(0));
                                    break;
                                }
                                else
                                {
                                    neigh = neigh->getNeighbour(0);
                                }* /

                                if (neigh->leftRightDifference180(0,1))
                                {
                                    logFile << "Entering recursion from while loop - item with 0-1 rotations of " << neigh->getNeighbour(0)->getTurnDegree(0)<< " -> " << neigh->getNeighbour(0)->getTurnDegree(1)<< " will be added." <<endl;
                                    recursivelyAdd(neigh->getNeighbour(0),fragment,neigh->getNeighbour(0)->getEndPoint(0));
                                    fragment->addEndPoint(neigh->getNeighbour(0)->getEndPoint(0),true,neigh->getNeighbour(0)->getTurnDegree(0),neigh->getNeighbour(0));
                                    break;
                                }
                                else
                                {
                                    neigh = neigh->getNeighbour(0);
                                }

                            }
////
*/

            }

        }



    }
    else
    {

    }
    int i = 1;
    while (item->getEndPoint(i)!=NULL)
    {
        if (item->getNeighbour(i)!=NULL)
        {
            if (item->getNeighbour(i)->getParentFragment()!=fragment)
            {
/*
modify this condition
                if ( item->getNeighbour(i)->getSlotTrackInfo()!=NULL
                     && (((item->getNeighbour(i)->getRadius()<0 && item->getRadius()>0))
                         || (item->getRadius()<0 && item->getNeighbour(i)->getRadius()>0))
                     )//                     && i%2==1)
                     */

                if (item->getNeighbour(i)->getSlotTrackInfo()!=NULL
                        && ((item->getRadius()>0 && item->getNeighbour(i)->getRadius()<0 && i%2==1)
                            || (item->getRadius()<0 && item->getNeighbour(i)->getRadius()>0) ///how about modulo??
                            || (item->getRadius()>0 && item->getNeighbour(i)->getRadius()>0 && i%2==0)
                            ||  (item->getRadius()<0 && item->getNeighbour(i)->getRadius()<0 && i%2==0)
                            )
                        )
                {

                    ModelItem * firstPointNeighbour = item->getNeighbour(i);

                    //check if the neighbour is connected at more than one point
                    int l = i;
                    while (l<2*item->getSlotTrackInfo()->numberOfLanes)
                    {
                        if (item->getNeighbour(l)!=firstPointNeighbour)
                        {
                            break;
                        }
                        l+=2;
                    }
                    l-=2;
                    if (l<0)
                        l=1;
                    QPointF * newPt = item->getEndPoint(l);
                    logFile << "Entering recursion from recursion at l==" <<l << endl;
                    if (item->getNeighbour(l)->getParentFragment()!=fragment)
                        recursivelyAdd(item->getNeighbour(l),fragment,newPt);
                    logFile << "Leaving recursion" << endl;
                    ///
                }
                else
                    recursivelyAdd(item->getNeighbour(i),fragment,item->getEndPoint(i));
            }
        }
        i++;
    }
    i=0;
    if (item->getNeighbour(i)!=NULL)
    {
        if (item->getNeighbour(i)->getParentFragment()!=fragment)
        {
            if (item->getNeighbour(i)->getSlotTrackInfo()!=NULL
                    && ((item->getRadius()>0 && item->getNeighbour(i)->getRadius()<0 && i%2==1)
                        || (item->getRadius()<0 && item->getNeighbour(i)->getRadius()>0) ///how about modulo??
                        || (item->getRadius()>0 && item->getNeighbour(i)->getRadius()>0 && i%2==0)
                        ||  (item->getRadius()<0 && item->getNeighbour(i)->getRadius()<0 && i%2==0)
                        )
                    )
            {

                ModelItem * firstPointNeighbour = item->getNeighbour(i);

                //check if the neighbour is connected at more than one point
                int l = i;
                while (l<2*item->getSlotTrackInfo()->numberOfLanes)
                {
                    if (item->getNeighbour(l)!=firstPointNeighbour)
                    {
                        break;
                    }
                    l+=2;
                }
                l-=2;
                if (l<0)
                    l=1;
                QPointF * newPt = item->getEndPoint(l);
                logFile << "Entering recursion from recursion at l==" <<l << endl;
                if (item->getNeighbour(l)->getParentFragment()!=fragment)
                    recursivelyAdd(item->getNeighbour(l),fragment,newPt);
                logFile << "Leaving recursion" << endl;
            }
            else
                recursivelyAdd(item->getNeighbour(i),fragment,item->getEndPoint(i));
        }
    }

    return;

}

int ModelFragment::deleteFragmentItem(ModelItem *item)
{
    logFile << "Deleting item " << item->getPartNo()->toStdString() << " at point [" << item->getEndPoint(0)->x() << "," << item->getEndPoint(0)->y() << "] and address " << item << " from fragment with id " << app->getWindow()->getWorkspaceWidget()->getFragmentIndex(this) << endl;

    //application should distinguish these special cases:
    //-there's just one item in the fragment - case A)
    //-there are more items in the fragment but "item" belongs to list of endPointsItems && has just 1 not-NULL neighbour => fragment will stay continous - case B)
    //-there are more items in the fragment and deletion of "item" will make fragment non-continous - case C)

    //case A)
    //if count of fragment items == 1 && item==fragment item
    if (this->fragmentItems->count()==1 && item==this->fragmentItems->first())
    {
        //destruct the fragment in workspace widget
        return -1;

    }

    //case B) - run this code only if item has exactly 1 not-NULL neighbour (caution: slot track)
    //iterate through endPointsItems and try to find "item" if it wasn't successful, go to case C) else:
    //-removed

    //case C)
    //for each neighbour N of item:  !!check if N.parentFragment==this - true -> ok, false -> NOP

    //dont start from neighbours
    //start from fragmentItems[0] - if [0]!=item, create new Fragment and start recursion from [0]
    //iterate through the list


    int i = 0;
    while (item->getEndPoint(i)!=NULL)
    {
        ModelItem * neighbour = item->getNeighbour(i);
        if (neighbour!=NULL)
        {
            //if (neighbour->getParentFragment()==this)
            //{
                //N.setNeighbour(NULL,index/point of connection)
                neighbour->setNeighbour(NULL,item->getEndPoint(i));

                if (neighbour->getSlotTrackInfo()!=NULL)
                {
                    //if neighbour's index is even, add 180 deg.
                    int k = 0;
                    QRectF area(item->getEndPoint(i)->x()-TOLERANCE_HALF,item->getEndPoint(i)->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);
                    while (neighbour->getEndPoint(k)!=NULL)
                    {

                        //check following if's

                        if (area.contains(*neighbour->getEndPoint(k)) && k%2==0)
                        {
                            /*
                            qreal dA = (neighbour->getTurnDegree(k+1)-neighbour->getTurnDegree(k));
                            dA = dA /2.0;
                            dA = abs(dA);
                            if ((abs(neighbour->getTurnDegree(k+1))-abs(neighbour->getTurnDegree(k)))>=0)//(dA>=0)//(neighbour->getTurnDegree(k+1)>neighbour->getTurnDegree(k))
                            {

                                dA = abs(dA);
                                qreal result =((neighbour->getTurnDegree(k)+2*dA)-neighbour->getTurnDegree(k+1));
                                if (result==0)
                                    neighbour->setEndPointAngle(k,180+neighbour->getTurnDegree(k));
                            }
                            ///is this condition always correct?
                            else if (dA<90) //dA=90 means it was =180
                            {
                                //qreal dA = (neighbour->getTurnDegree(k+1)-neighbour->getTurnDegree(k));
                                //dA = dA /2.0;
                                dA = abs(dA);
                                qreal result =((neighbour->getTurnDegree(k+1)+2*dA)-neighbour->getTurnDegree(k));
                                if (result==0)
                                    neighbour->setEndPointAngle(k,180+neighbour->getTurnDegree(k));
                            }
*/

                            if (!neighbour->leftRightDifference180(k,k+1))
                                neighbour->setEndPointAngle(k,180+neighbour->getTurnAngle(k));

                            break;
                        }
                        k++;
                    }
                }
                else
                {
                    /**
                      TODO
                      */
                }


            //}
        }
        i++;
    }
    i=-1;




    i=0;
    while (item->getEndPoint(i)!=NULL)
    {
        item->setNeighbour(NULL,i);
        i++;
    }

    for (int j = 0; j < this->fragmentItems->count() ;j++)
    {
        if (this->fragmentItems->at(j)!=item)
        {
            //if (this->fragmentItems->at(j)->getParentFragment()==this && 180==abs(abs(this->fragmentItems->at(j)->getTurnDegree(1))-abs(this->fragmentItems->at(j)->getTurnDegree(0)))) // && this->fragmentItems->at(j)->getNeighbour(0)==NULL)

            //doesn't work for items with neighbour at 0 - their parFrag=='this', but 'this' will be deleted
            //if (this->fragmentItems->at(j)->getParentFragment()==this && this->fragmentItems->at(j)->getNeighbour(0)==NULL)

            if (this->fragmentItems->at(j)->getParentFragment()==this)
            {

                //this 180 deg. thing isn't good solution - you have to subtract 180 from point [0] of j==0 item

                /**
                  TODO:
                  -change slot track angles for all left side points with null neighbours
*/
/*
///debug following 180 manipulation!

                if ((j==0 && this->fragmentItems->at(j)->getNeighbour(0)==NULL)
                       // && (180<abs((abs(this->fragmentItems->at(j)->getTurnDegree(0))-180)-abs(this->fragmentItems->at(j)->getTurnDegree(1))))
                        )
                    this->fragmentItems->at(j)->setEndPointAngle(0,-180+this->fragmentItems->at(j)->getTurnDegree(0));


                //if (0!=((min(abs(this->fragmentItems->at(j)->getTurnDegree(0)),abs(this->fragmentItems->at(j)->getTurnDegree(1)))+180)-
                //        (max(abs(this->fragmentItems->at(j)->getTurnDegree(0)),abs(this->fragmentItems->at(j)->getTurnDegree(1))))))
                if (180<abs((abs(this->fragmentItems->at(j)->getTurnDegree(0))-180)-abs(this->fragmentItems->at(j)->getTurnDegree(1))))
                    this->fragmentItems->at(j)->setEndPointAngle(0,180+this->fragmentItems->at(j)->getTurnDegree(0));


                if (this->fragmentItems->at(j)->getSlotTrackInfo()!=NULL)
                {
                    int y = 2;
                    while (this->fragmentItems->at(j)->getEndPoint(y)!=NULL)
                    {
                        if (180<abs((abs(this->fragmentItems->at(j)->getTurnDegree(y))-180)-abs(this->fragmentItems->at(j)->getTurnDegree(y+1))))
                            this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnDegree(y));
                        y+=2;
                    }
                }
---*/

                //do something with left side angles??
                if (this->fragmentItems->at(j)->getSlotTrackInfo()!=NULL)
                {
                    int y = 0;
                    while (this->fragmentItems->at(j)->getEndPoint(y)!=NULL)
                    {
                        //original version
                        //if (!this->fragmentItems->at(j)->leftRightDifference180(y,y+1))
                        //modified version because of connectFragments()
                        if (this->fragmentItems->at(j)->getNeighbour(y)==NULL)
                            this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnAngle(y));
                        y+=2;
                    }
                }
                else
                {
                    /**
                      TODO
                      -rail parts left side - caution - turnouts and t1-t10
*/
                }


                ModelFragment * newF = new ModelFragment(this->fragmentItems->at(j));

                logFile << "New fragment has been created from item " << this->fragmentItems->at(j) << " at j==[" << j << "] and address " << newF << endl;

                app->getWindow()->getWorkspaceWidget()->addFragment(newF);
                //QPointF ptZero(0,0);
                //recursivelyAdd(this->fragmentItems->at(j),newF,&ptZero);
                int k = 0;
                while (this->fragmentItems->at(j)->getEndPoint(k)!=NULL)
                {
                    if (this->fragmentItems->at(j)->getNeighbour(k)!=NULL)
                    {
                        /**
                          TODO:
                          -slot track - don't insert at the first point of connection - check connection at all lanes and according to sgn(radius)
                          and number of lanes of connection, choose the outer most lane to connect item at
*/
                        //the recursion should be started from odd number (clockwise system of adding items)
                        //if (this->fragmentItems->at(j)->getSlotTrackInfo()!=NULL && (this->fragmentItems->at(j)->getRadius()<0 || this->fragmentItems->at(j)->getNeighbour(k)->getRadius()<0))// && k%2==1)
                        ////if (this->fragmentItems->at(j)->getSlotTrackInfo()!=NULL && ((this->fragmentItems->at(j)->getRadius()<0 && this->fragmentItems->at(j)->getNeighbour(k)->getRadius()>0) || (this->fragmentItems->at(j)->getRadius()>0 && this->fragmentItems->at(j)->getNeighbour(k)->getRadius()<0)))
                        if (this->fragmentItems->at(j)->getNeighbour(k)->getSlotTrackInfo()!=NULL
                        && ((this->fragmentItems->at(j)->getRadius()>0 && this->fragmentItems->at(j)->getNeighbour(k)->getRadius()<0 && k%2==1)
                            || (this->fragmentItems->at(j)->getRadius()<0 && this->fragmentItems->at(j)->getNeighbour(k)->getRadius()>0) ///how about modulo??
                            || (this->fragmentItems->at(j)->getRadius()>0 && this->fragmentItems->at(j)->getNeighbour(k)->getRadius()>0 && k%2==0)
                            ||  (this->fragmentItems->at(j)->getRadius()<0 && this->fragmentItems->at(j)->getNeighbour(k)->getRadius()<0 && k%2==0)
                            )
                        )
                        {
                            ModelItem * firstPointNeighbour = this->fragmentItems->at(j)->getNeighbour(k);

                            //check if the neighbour is connected at more than one point
                            int l = k;
                            while (l<=2*this->fragmentItems->at(j)->getSlotTrackInfo()->numberOfLanes)
                            {
                                if (this->fragmentItems->at(j)->getNeighbour(l)!=firstPointNeighbour)
                                {
                                    //l-=2;
                                    break;
                                }
                                l+=2;
                            }
                            l-=2;
                            if (l<0)
                                l=1;
                            QPointF * newPt = this->fragmentItems->at(j)->getEndPoint(l);
                            logFile << "Entering recursion with l==" << l << endl;
                            recursivelyAdd(this->fragmentItems->at(j)->getNeighbour(l),newF,newPt);
                            logFile << "Leaving recursion" << endl;


                        }
                        else// if (k%2==1)
                        {
                            QPointF * newPt = this->fragmentItems->at(j)->getEndPoint(k);
                            logFile << "Entering recursion" << endl;
                            recursivelyAdd(this->fragmentItems->at(j)->getNeighbour(k),newF,newPt);
                            logFile << "Leaving recursion" << endl;
                        }


                    }
                    k++;
                }
            }
        }
    }



    this->fragmentItems->clear();
    this->fragmentItems->push_back(item);

    //delete item
    //delete remaining parts of old fragment (=this) !!!caution: endPoints contain pointers to items which now belong to other fragments
    return -1;
}

/**
int ModelFragment::addEndPoints (QList<QPointF *> * listToAppend)
{
    if (listToAppend==NULL)
        return 1;
    this->endPoints->append(*listToAppend);
    return 0;
}
*/

int ModelFragment::removeEndPoint (QPointF * pt)
{
    if (pt == NULL)
        return 1;

    int index = this->endPoints->indexOf(pt);
    QGraphicsItem * item = this->endPointsGraphics->at(index);
    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(item);
    delete item;
    this->endPointsGraphics->removeAt(index);
    this->endPointsAngles->removeAt(index);
    this->endPointsItems->removeAt(index);
    bool success = this->endPoints->removeOne(pt);


    if (success)
        return 0;
    else
        return 2;
}
/*
int ModelFragment::removeEndPoint (int index)
{
    if (index < 0 || index >= this->endPoints->size())
        return 1;
    this->endPoints->removeAt(index);
    return 0;
}
*/
int ModelFragment::setEndPointAngle(QPointF * pt, qreal angle)
{
    if (pt==NULL)
        return 1;
    int index = 0;
    while (index < this->endPoints->count())
    {
        if (*pt==*this->endPoints->at(index))
            break;
        index++;
    }

    if (index>=this->endPoints->count()) //??
        return 2;
    //this->endPointsAngles->at(index)=angle;
    QList<qreal>::Iterator iter = this->endPointsAngles->begin();
    for (int i = 0; i < index; i++)
        iter++;
    *iter=angle;

    return 0;
}

int ModelFragment::setEndPointAngle(int index, qreal angle)
{
    QList<qreal>::Iterator iter = this->endPointsAngles->begin();
    if (index < 0 || index >=this->endPointsAngles->size())
        return 1;
    for (int i = 0; i < index; i++)
        iter++;
    *iter=angle;
    return 0;
}

int ModelFragment::showInfoDialog()
{
    int result = 0;
    if (this->infoDialog==NULL)
        result = this->initInfoDialog();

    if (result)
        return result;

    this->infoDialog->show();


    return result;
}

int ModelFragment::initInfoDialog()
{
    this->infoDialog = new QDialog(app->getWindow());
    QFormLayout * layout = new QFormLayout(this->infoDialog);
    this->infoDialog->setWindowTitle("Model fragment info");

    /*Model fragment info
     *parts count
     *parts listbox or something similar
     *parts are made by manufacturers:
     *list box or something similar
     *track length?? how to compute the distance? sum of all item's lengths
     */
    QString str (QString::number(this->fragmentItems->count()));
    QLabel * label = new QLabel(str, this->infoDialog);
    QListWidget * listWidgetI = new QListWidget(this->infoDialog);
    for (int i = 0; i < this->fragmentItems->count();i++)
    {
        QString str;
        str.append(*this->fragmentItems->at(i)->getPartNo());
        listWidgetI->addItem(str);
    }

    layout->insertRow(0,"Parts count:",label);
    layout->insertRow(1,"Parts used:",listWidgetI);

    QListWidget * listWidgetM = new QListWidget(this->infoDialog);
    for (int i = 0; i < this->fragmentItems->count(); i++)
    {
        listWidgetM->addItem(*this->fragmentItems->at(i)->getProdLine()->getName());
    }
    layout->insertRow(2,"Parts' manufacturer(s)",listWidgetM);

    this->infoDialog->setLayout(layout);
    return 0;
}

void ModelFragment::moveBy(qreal dx, qreal dy)
{
    if (dx==0 && dy==0)
        return;
    logFile << "Moving fragment " << app->getWindow()->getWorkspaceWidget()->getFragmentIndex(this) << " by [" << dx << "," << dy << "]" << endl;

    for (int i = 0; i < this->fragmentItems->count(); i++)
    {

        this->fragmentItems->at(i)->moveBy(dx,dy);

    }

    //move all endpoints' graphics of this fragment
    QList<QGraphicsEllipseItem*>::Iterator it2 = this->endPointsGraphics->begin();
    while(it2!=this->endPointsGraphics->end())
    {
        (*it2)->moveBy(dx,dy);
        it2++;
    }


}

void ModelFragment::rotate(qreal angle, QPointF * center)
{
    if (angle!=0)
        logFile << "Rotating fragment "  << app->getWindow()->getWorkspaceWidget()->getFragmentIndex(this) << " by " << angle << endl;
    QList<ModelItem*>::Iterator itemIter = this->fragmentItems->begin();
    QList<qreal>::Iterator angleIter = this->endPointsAngles->begin();
    for (int i = 0; i < this->fragmentItems->count(); i++)
    {

        (*itemIter)->rotate(angle, center);
        itemIter++;


    }
    QTransform mat;
    mat.rotate(angle);




    QList<QGraphicsEllipseItem*>::Iterator it2 = this->endPointsGraphics->begin();
    while(it2!=this->endPointsGraphics->end())
    {
        QPointF newPointAfterRotation((*it2)->scenePos().x(),(*it2)->scenePos().y());
        rotatePoint(&newPointAfterRotation,angle,center);

        (*angleIter)+=angle;
        (*it2)->setTransform(mat,true);
        (*it2)->setPos(newPointAfterRotation.x(),newPointAfterRotation.y());
        it2++;
        angleIter++;
    }

}

bool ModelFragment::leftSide(ModelItem *item, ModelItem * &firstItemWith180Diff)
{
    /**
      TODO:
      -what happens if "item" has 180 diff?
*/
    ModelItem * m = item;
    ModelItem * mPrev = item;
    QList<ModelItem *> visited;
    QList<ModelItem *> toVisit;

    bool first = true;
    int k = 0;
    while (m->getEndPoint(k)!=NULL)
    {
        if (m->getNeighbour(k)!=NULL)
        {
            toVisit.push_back(m->getNeighbour(k));
        }
        k++;
    }
    visited.push_back(m);

    ///add always all neighbours, but if you find 180 diff item, check whether the point of connection is even
    while (!toVisit.empty())
    {
        m=toVisit.first();
        toVisit.pop_front();
        k = 0;
        while (m->getEndPoint(k)!=NULL)
        {
            //connecting with the right side of item but with the left side of fragment
            if (m->leftRightDifference180(k,k+1) && m->getNeighbour(k)==mPrev)
            {
                firstItemWith180Diff=m;
                return true;
            }
            else if (m->leftRightDifference180(k,k+1))
            {
                if (first)
                {
                    firstItemWith180Diff=m;
                    first=false;
                }

            }
            if (m->getNeighbour(k)!=NULL)
            {


                if (!visited.contains(m->getNeighbour(k)))
                    toVisit.push_back(m->getNeighbour(k));
            }
            k+=2;
        }
        if (!visited.contains(m))
            visited.push_back(m);
        mPrev = m;
    }
    return false;
}

int ModelFragment::addEndPoint(QPointF* pt, bool additionalInfo, qreal rotation, ModelItem * endPointItem)
{
    if (pt==NULL)
        return 1;

    //QGraphicsEllipseItem * qgpi = new QGraphicsEllipseItem(pt->x(),pt->y(),1,1);
    QGraphicsEllipseItem * qgpi = new QGraphicsEllipseItem(0,0,1,1);
    qgpi->setPos(pt->x(),pt->y());

    QPen p = qgpi->pen();
    p.setWidth(4);
    qgpi->setPen(p);

    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(qgpi);

    this->endPointsGraphics->push_back(qgpi);
    this->endPoints->push_back(pt);

    if (additionalInfo)
    {
        this->endPointsAngles->push_back(rotation);
        this->endPointsItems->push_back(endPointItem);
    }

    return 0;
}




SlotTrackInfo::SlotTrackInfo()
{
    this->fstLaneDist=0;
    this->lanesGauge=0;
    this->lanesGaugeEnd=0;
    this->numberOfLanes=0;
}

SlotTrackInfo::SlotTrackInfo(const SlotTrackInfo &s)
{
    this->fstLaneDist=s.fstLaneDist;
    this->lanesGauge=s.lanesGauge;
    this->lanesGaugeEnd=s.lanesGaugeEnd;
    this->numberOfLanes=s.numberOfLanes;
}

