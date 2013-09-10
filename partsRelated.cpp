
#include "QFormLayout"
#include "QListWidget"
#include "QSpinBox"
#include "QTextEdit"
#include "application.h"
#include "partsRelated.h"
#include "globalVariables.h"
#include "itemTypeEnums.h"
#include "mathFunctions.h"


//ModelItem::ModelItem(QString * partNumber, QString * partNameEn, QString * partNameCs,
//                     QPointF * start, QPointF * end ,qreal degree, qreal turnRadius,
//                     void * appPointer, QWidget * parentWidg, QGraphicsItem * contourParent)
ModelItem::ModelItem(QString & partNumber, QString & partNameEn, QString & partNameCs,
                     QPointF &start, QPointF &end ,qreal degree,
                     qreal turnRadius, qreal width, qreal height, ProductLine * prodLine, QWidget * parentWidg)/// : QWidget (parentWidg)
{

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
    this->partNo = new QString(partNumber);
    this->nameEn = new QString(partNameEn);
    this->nameCs = new QString(partNameCs);
    this->endPoints = new QList <QPointF*>();
    this->slotTrackInfo = NULL;

    QList<QPointF>::Iterator epIter = endPoints.begin();
    while(epIter!=endPoints.end())
    {
        this->endPoints->push_back(new QPointF(*epIter));
        epIter++;
    }

    this->endPointsAngles = new QList <qreal>();
    QList<qreal>::Iterator aIter = angles.begin();
    while(aIter!=angles.end())
    {
        this->endPointsAngles->push_back(*aIter);
        aIter++;
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
    this->t=type;

    if (!prodLine->getType())
            this->slotTrackInfo = new SlotTrackInfo();

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

qreal ModelItem::getTurnDegree(int index) const
{
    if (index < this->endPointsAngles->count())
        return this->endPointsAngles->at(index);
    else
        return -9.87654321;
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

    qreal startAngle = 90-(this->getTurnDegree());


    QFont font;
    font.setPixelSize(20);
    QString label(*this->partNo);
    label.append(" ");
    QString s("");
    if (this->getTurnDegree()!=0)
        s.setNum(2*this->getTurnDegree(),'f',1);
    else
        s.setNum(2*this->getTurnDegree(),'f',0);

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
        itemPath.arcTo(rectOuter,startAngle,2*this->getTurnDegree());



        //itemPath.arcMoveTo(rectInner,startAngle);
        itemPath.arcTo(rectInner,startAngle,2*this->getTurnDegree());


        itemPath.closeSubpath();

        if (this->slotTrackInfo!=NULL)
            itemPath.translate(0,-0*gauge);

    }
    else if (this->t==E1 || this->t==S1 || this->t==HS || this->t==HE)
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

        startAngle=90-(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(2)))/2.0;

        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,abs(this->getTurnDegree(0))+abs(this->getTurnDegree(2)));
        itemPath.arcTo(rectInner,startAngle,abs(this->getTurnDegree(0))+abs(this->getTurnDegree(2)));


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
                    mat1.rotate(abs(this->getTurnDegree(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(this->radius2+gauge/1.0,-this->radius2-gauge/1.0,-this->radius2*2-2*gauge,this->radius2*2+2*gauge);
                    rectInner2 = QRectF(-this->radius2+gauge/1.0,-this->radius2+gauge/1.0,this->radius2*2-2*gauge,this->radius2*2-2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    startAngle = 90-(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1)));

                    itemPath.arcTo(rectInner2,startAngle,(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(-abs(this->getTurnDegree(0)));
                    itemPath = mat2.map(itemPath);
                }
                else
                {
                    QMatrix mat1;
                    mat1.rotate(-abs(this->getTurnDegree(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(-this->radius2+gauge/1.0,-this->radius2-gauge/1.0,this->radius2*2-2*gauge,this->radius2*2+2*gauge);
                    rectInner2 = QRectF(this->radius2+gauge/1.0,-this->radius2+gauge/1.0,-this->radius2*2-2*gauge,this->radius2*2-2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    startAngle = 90-(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1)));

                    itemPath.arcTo(rectInner2,startAngle,(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(abs(this->getTurnDegree(0)));
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
                    mat1.rotate(abs(this->getTurnDegree(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(this->radius2+gauge/1.0,this->radius2+gauge/1.0,-this->radius2*2-2*gauge,-this->radius2*2-2*gauge);
                    rectInner2 = QRectF(-this->radius2+gauge/1.0,this->radius2-gauge/1.0,this->radius2*2-2*gauge,-this->radius2*2+2*gauge);

                    startAngle = 270;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,-(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    startAngle = 270+(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1)));

                    itemPath.arcTo(rectInner2,startAngle,-(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(-abs(this->getTurnDegree(0)));
                    itemPath = mat2.map(itemPath);
                }
                else
                {
                    QMatrix mat1;
                    mat1.rotate(-abs(this->getTurnDegree(0)));
                    itemPath = mat1.map(itemPath);


                    itemPath.translate(0,-abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(-this->radius2+gauge/1.0,-this->radius2-gauge/1.0,this->radius2*2-2*gauge,this->radius2*2+2*gauge);
                    rectInner2 = QRectF(this->radius2+gauge/1.0,-this->radius2+gauge/1.0,-this->radius2*2-2*gauge,this->radius2*2-2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    startAngle = 90-(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1)));

                    itemPath.arcTo(rectInner2,startAngle,(abs(this->getTurnDegree(0))+abs(this->getTurnDegree(1))));
                    //itemPath.closeSubpath();

                    itemPath.translate(0,abs(this->radius2-this->radius));

                    QMatrix mat2;
                    mat2.rotate(abs(this->getTurnDegree(0)));
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
            itemPath.arcTo(rectOuterMinus,startAngle,(this->getTurnDegree(2)));
            itemPath.arcMoveTo(rectInnerMinus,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,-(this->getTurnDegree(2)));
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
            itemPath.arcTo(rectOuterMinus,startAngle,(this->getTurnDegree(3)));
            itemPath.arcMoveTo(rectInner,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,-(this->getTurnDegree(3)));

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
            itemPath.arcTo(rectOuterMinus,startAngle,-(this->getTurnDegree(2)));
            itemPath.arcMoveTo(rectInnerMinus,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,(this->getTurnDegree(2)));
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
            itemPath.arcTo(rectOuterMinus,startAngle,-(this->getTurnDegree(3)));
            itemPath.arcMoveTo(rectInner,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,(this->getTurnDegree(3)));

            itemPath.translate(-this->getEndPoint(0)->x(),0);

            itemPath.translate(0,-gauge+1);

            this->setEndPointAngle(2,180-this->getTurnDegree(2));
            this->setEndPointAngle(3,180-this->getTurnDegree(3));
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
                itemPath.arcTo(rectOuter,startAngle,-(this->getTurnDegree(2)));
                itemPath.arcMoveTo(rectInner,startAngle);
                itemPath.arcTo(rectInner,startAngle,(this->getTurnDegree(2)));

                startAngle = 270;
                itemPath.translate(-this->getEndPoint(0)->x(),2*radius);
                itemPath.translate(-this->getEndPoint(0)->x(),0);

                itemPath.arcMoveTo(rectOuter,startAngle);
                itemPath.arcTo(rectOuter,startAngle,(this->getTurnDegree(3)));
                itemPath.arcMoveTo(rectInner,startAngle);
                itemPath.arcTo(rectInner,startAngle,-(this->getTurnDegree(3)));
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

        itemPath.translate(0,this->radius+gauge);



    }


    else if (t>=T2 && t<=T10)
    {
        qreal dy = 0;
        if (this->parentWidget!=NULL)
            dy = -gauge+this->itemHeight-gauge;
        else
        {
            dy = this->itemHeight-2*gauge;
            itemPath.translate(0,this->itemHeight+2*gauge);
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
                lanePath.arcTo(rectLane,startAngle,2*this->getTurnDegree());

                if (radius > 0)
                    rectParam += this->slotTrackInfo->lanesGauge;
                else
                    rectParam -= this->slotTrackInfo->lanesGauge;

            }
            lanePath.translate(0,this->radius);
        }
        else if (this->t==S1 || this->t==X1)
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
        else if (this->t==J1)
        {}
        else if (this->t==J2)
        {}
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


        else if (this->t==X2)
        {}
        else if (this->t==X3)
        {}
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
        else if (this->t==HE)
        {}
        else if (this->t==H1)

        col.setRed(255);
        col.setBlue(0);
        col.setGreen(255);
        col = QColor(255,255,0);

        QPen p = lane->pen();
        p.setColor(col);
        p.setWidth(3);
        lane->setPen(p);

        lane->setPath(lanePath);
//*/


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
        this->contourModel=gpi;
    else
    {
        this->contourModelNoText=gpi;
    }


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
    QTransform mat;
    mat.translate(-dx,-dy);

    this->get2DModelNoText()->moveBy(dx,dy);
    //this->get2DModelNoText()->setTransform(mat,true);
    for(int i = 0; i < this->endPoints->count(); i++)
    {
        this->endPoints->at(i)->setX(this->endPoints->at(i)->x()+dx);
        this->endPoints->at(i)->setY(this->endPoints->at(i)->y()+dy);
    }
}

qreal ModelItem::getSecondRadius() const
{
    return this->radius2;
}

void ModelItem::setSecondRadius(qreal rad2)
{
    this->radius2=rad2;
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

SlotTrackInfo *ModelItem::getSlotTrackInfo()
{
    return this->slotTrackInfo;
}

int ModelItem::setSlotTrackInfo(SlotTrackInfo *s)
{
    if (s==NULL)
        return 1;
    this->slotTrackInfo=s;
    return 0;
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



void GraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=true;
    QGraphicsPathItem::mousePressEvent(event);
}
void GraphicsPathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=false;
    QGraphicsPathItem::mouseReleaseEvent(event);
}
void GraphicsPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((this->mousePressed) && this->parentItem->getParentFragment()!=NULL)
    {
        if (this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget() && app->getWindow()->getWorkspaceWidget()->getRotationMode())
        {
            //somehow get the angle of rotation
            QPointF center(this->scenePos().x(),this->scenePos().y());
            QPointF * newPoint = app->getWindow()->getWorkspaceWidget()->getActiveEndPoint();
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
                QPointF * newPoint = app->getWindow()->getWorkspaceWidget()->getActiveEndPoint();
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
                    if (toMake->getType()==HS)
                        endDegrees->push_back(180-toMake->getTurnDegree(index));
                    else
                        endDegrees->push_back(-toMake->getTurnDegree(index));
                }
                else
                    endDegrees->push_back(toMake->getTurnDegree(index));

                if (toMake->getSlotTrackInfo()!=NULL && eventPos.x()<gpi->pos().x()/2)
                {
                    QPointF pt;
                    if ((toMake->getType()==X1 && index>=toMake->getSlotTrackInfo()->numberOfLanes*2)) //|| (toMake->getType())==SB
                        pt = QPointF(-toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y());
                    else if ((toMake->getType())==HS)
                        pt = QPointF(-toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y());
                    else
                        pt = QPointF(toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y());
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
                    if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                        it->setEndPointAngle(0,180+it->getTurnDegree(0));
                }
                else
                {
                    if (!(it->getType()>=T1 && it->getType()<=T10) && app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                        it->setEndPointAngle(0,180+it->getTurnDegree(0));
                }




                //!eventPos???
                if (toMake->getType()==J2)
                {
                    it->setEndPointAngle(1,180-it->getTurnDegree());
                    /*
                    if (it->getRadius()>0)

                    else
                        it->setEndPointAngle(1,it->getTurnDegree());
                        */
                }
                if (toMake->getType()==X1 || toMake->getType()==J4)
                {
                    if (it->getRadius()<0)
                    {
                    }
                    it->setEndPointAngle(2,180-it->getTurnDegree(2));

                }
                if (toMake->getType()==J5)
                    it->setEndPointAngle(2,180);
                if (it->getType()>=T2 && it->getType()<=T10)
                {
                    int index = 0;
                    while (index!=(it->getType()-8))
                    {
                        it->setEndPointAngle(index,180+it->getTurnDegree(index));
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

                if (toMake->getType()==J2)
                {
                    if (toMake->getRadius()>0)
                        it->setEndPointAngle(1,180-it->getTurnDegree());
                    else
                        it->setEndPointAngle(1,it->getTurnDegree());
                }

                if (toMake->getType()==X1 || toMake->getType()==J4)
                {
                    if (it->getRadius()<0)
                    {
                    }
                    it->setEndPointAngle(2,180-it->getTurnDegree(2));

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
                        it->setEndPointAngle(index,180+it->getTurnDegree(index));
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

    makeNewItem(event->scenePos(),this,this->parentItem,this->parentItem, false);
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
    this->endPoints = new QList<QPointF*>();


    /**
      TODO
      -switch type
     */

    this->endPointsGraphics=new QList<QGraphicsEllipseItem*>();
    this->endPointsAngles = new QList<qreal>();

    int index = 0;
    while (item->getEndPoint(index)!=NULL)
    {
        this->addEndPoint(item->getEndPoint(index));
        this->endPointsAngles->push_back(item->getTurnDegree(index));
        index++;
    }

    this->fragmentItems = new QList <ModelItem*>();
    this->lines = new QList <ProductLine*>();

    this->fragmentItems->push_back(item);
    this->lines->push_back(item->getProdLine());
    item->setParentFragment(this);

    this->infoDialog = NULL;
    this->transformMatrix = NULL;


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

int ModelFragment::addFragmentItems(QList <ModelItem*> * listToAppend)
{
    if (listToAppend==NULL)
        return 1;
    this->fragmentItems->append(*listToAppend);
    return 0;
}

int ModelFragment::addFragmentItem(ModelItem* item,QPointF * point)
{
    if (item == NULL)
        return 1;

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
    QList<QPointF*>::Iterator iter = this->endPoints->begin();
    int i = 0;

    if (item->getType()==T1)
        cout << "T0";


    while(iter!=this->endPoints->end())
    {
        if (**iter==*point)
        {
            //get frag. rotation and then remove it from the list - won't be used
            currentFragmentRotation = this->endPointsAngles->at(i);
            !!this->removeEndPoint(*iter);

            //this->endPoints->removeAt(i);
            break;
        }
        iter++;
        i++;
    }




    //get start-point-rotation of inserted item and subtract it from currentFragmentRotation
    //^^-result==0 -> no rotation is needed
    if (currentFragmentRotation-item->getTurnDegree(0)!=0)
    {

        //rotate 2D model and modify item's attributes
        item->rotate(currentFragmentRotation-(item->getTurnDegree(0)));
    }





    *point-=*item->getEndPoint(0);
    qreal dx = point->x();
    qreal dy = point->y();

    if (item->getType()==T1)
    {

        item->rotate(180);
        item->moveBy(-2*item->getEndPoint(0)->x(),-2*item->getEndPoint(0)->y());

    }

    item->moveBy(dx,dy);



    qreal newFragmentRotation = item->getTurnDegree();

    this->endPointsAngles->push_back(newFragmentRotation);


    app->getWindow()->getWorkspaceWidget()->updateFragment(this);


    QPointF * newPt = NULL;
    if (item->getEndPoint()!=NULL)
         newPt = new QPointF(item->getEndPoint()->x(),item->getEndPoint()->y());

    bool connected = false;

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
                    connected = true;
                    this->removeEndPoint(this->endPoints->at(i));
                    //fix the appearance of fragment
                    //if (aS > pS) -> rotate else move
                }
            }

        }
    }


    if (connected || newPt==NULL)
    {
        if (newPt!=NULL)
            *newPt=QPointF(0,0);
        else
            newPt = new QPointF(0,0);
        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
        this->endPointsAngles->removeOne(newFragmentRotation);
    }
    else
    {
        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);



        if (item->getEndPoint()!=NULL)
            this->addEndPoint(item->getEndPoint());

        //add all remaining endPoint's
        int index = 2;
        while (item->getEndPoint(index)!=NULL)
        {
            this->addEndPoint(item->getEndPoint(index));
            this->endPointsAngles->push_back(item->getTurnDegree(index));
            index++;
        }
    }



    return 0;
}

int ModelFragment::deleteFragmentItem(ModelItem *item)
{
    int i = 0;
    QPointF * endPoint = item->getEndPoint(i);
    while(endPoint!=NULL)
    {
        endPoint=item->getEndPoint(i);
        this->addEndPoint(endPoint);
        if (i==0)
            this->endPointsAngles->push_back(item->getTurnDegree(i));
        else
            this->endPointsAngles->push_back(item->getTurnDegree(i)-180);
        i++;
    }

    this->fragmentItems->removeOne(item);
    app->getWindow()->getWorkspaceWidget()->updateFragment(this);
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
    this->endPointsGraphics->removeAt(index);
    this->endPointsAngles->removeAt(index);
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
    int index = this->endPoints->indexOf(pt);
    if (index==-1) //??
        return 2;
    //this->endPointsAngles->at(index)=angle;
    QList<qreal>::Iterator iter = this->endPointsAngles->begin();
    for (int i = 0; i <= index; i++)
        iter++;
    *iter=angle;

    return 0;
}

int ModelFragment::setEndPointAngle(int index, qreal angle)
{
    QList<qreal>::Iterator iter = this->endPointsAngles->begin();
    if (index < 0 || index >=this->endPointsAngles->size())
        return 1;
    for (int i = 0; i <= index; i++)
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

int ModelFragment::addEndPoint(QPointF *pt)
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

    return 0;
}



