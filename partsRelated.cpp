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

#include <QFormLayout>
#include <QListWidget>
#include <QSpinBox>
#include <QTextEdit>
#include <QTime>
#include "application.h"
#include "partsRelated.h"
#include "heightPathItem.h"
#include "globalVariables.h"
#include "itemTypeEnums.h"
#include "mathFunctions.h"


GenericModelItem::GenericModelItem(QString &partNo, QString &nameEn, QString &nameCs, ProductLine *productLine, QWidget *parentWidget)
{
    this->partNo = new QString(partNo);
    this->nameEn = new QString(nameEn);
    this->nameCs = new QString(nameCs);
    this->availableCount=10;
    this->prodLine=productLine;
    this->parentWidget = parentWidget;
    this->model2D=NULL;
    this->model2DNoText=NULL;
}

GenericModelItem::~GenericModelItem()
{
    delete this->partNo;
    delete this->nameCs;
    delete this->nameEn;
    this->partNo=NULL;
    this->nameCs=NULL;
    this->nameEn=NULL;

    if (this->model2D!=NULL)
    {
        if (this->model2D->scene()!=NULL && app->getWindow()!=NULL)
            this->model2D->scene()->removeItem(this->model2D);
        if (app->getWindow()!=NULL)
            delete this->model2D;

        this->model2D=NULL;
    }

    if (this->model2DNoText!=NULL)
    {
        if (this->model2DNoText->scene()!=NULL && app->getWindow()!=NULL)
            this->model2DNoText->scene()->removeItem(this->model2DNoText);
        if (app->getWindow()!=NULL)
            delete this->model2DNoText;
        this->model2DNoText=NULL;
    }
}

QString *GenericModelItem::getPartNo() const
{return this->partNo;}
QString * GenericModelItem::getNameEn() const
{return this->nameEn;}
QString * GenericModelItem::getNameCs() const
{return this->nameCs;}

GraphicsPathItem *GenericModelItem::get2DModel() const
{return this->model2D;}

int GenericModelItem::set2DModel(GraphicsPathItem *model)
{
    if (model==NULL)
        return 1;
    this->model2D=model;
    return 0;
}

GraphicsPathItem *GenericModelItem::get2DModelNoText() const
{return this->model2DNoText;}

int GenericModelItem::set2DModelNoText(GraphicsPathItem *model)
{
    if (model==NULL)
        return 1;
    this->model2DNoText=model;
    return 0;
}

unsigned int GenericModelItem::getAvailableCount() const
{return this->availableCount;}

void GenericModelItem::setAvailableCount(unsigned int count)
{this->availableCount=count;}

void GenericModelItem::incrAvailableCount()
{++this->availableCount;}

void GenericModelItem::decrAvailableCount()
{
    if (this->availableCount!=0)
    --this->availableCount;
}

QWidget *GenericModelItem::getParentWidget() const
{return this->parentWidget;}

ProductLine *GenericModelItem::getProdLine() const
{return this->prodLine;}


ModelItem::ModelItem(QString &partNumber, QString &partNameEn, QString &partNameCs,
                     QList<QPointF> &endPoints, QList<qreal> angles, qreal turnRadius,
                     qreal width, qreal height, ItemType type, ProductLine *prodLine, QWidget *parentWidg)  : GenericModelItem(partNumber,partNameEn,partNameCs,prodLine,parentWidg)
{
    logFile << "        ModelItem constructor #2" << endl;
    logFile << "            " << partNumber.toStdString() << ", r: " << turnRadius << endl;
    this->endPoints = new QList <QPointF*>();
    this->neighbours = new QList<ModelItem*>();
    this->endPointsHeight = new QList<int>();
    this->endPointsHeightGraphics = new QList<QGraphicsPathItem*>();

    this->slotTrackInfo = NULL;

    QList<QPointF>::Iterator epIter = endPoints.begin();
    if (type==CB && app!=NULL)
    {
        ModelItem * prodLineModel = app->getAppData()->getDatabase()->findModelItemByName(*prodLine->getName(),partNumber);
        int index = 0;
        while(epIter!=endPoints.end())
        {
            this->endPoints->push_back(new QPointF(*epIter));
            epIter++;
            this->neighbours->push_back(NULL);
            this->endPointsHeight->push_back(prodLineModel->getHeightProfileAt(prodLineModel->getEndPoint(index)));
            this->endPointsHeightGraphics->push_back(NULL);
            index++;
        }
    }
    else
    {
        while(epIter!=endPoints.end())
        {
            this->endPoints->push_back(new QPointF(*epIter));
            epIter++;
            this->neighbours->push_back(NULL);
            this->endPointsHeight->push_back(0);
            this->endPointsHeightGraphics->push_back(NULL);
        }
    }


    this->endPointsAngles = new QList <qreal>();
    QList<qreal>::Iterator aIter = angles.begin();
    int i = 0;
    while(aIter!=angles.end())
    {
        this->endPointsAngles->push_back(*aIter);
        aIter++;
        i++;
    }



    this->radius=turnRadius;
    this->radius2=0;
    this->itemWidth=width;
    this->itemHeight=height+=prodLine->getScaleEnum()/2.0;

    this->parentFragment=NULL;
    this->recursionStopper=false;
    this->recursionStopperAdj=false;
    this->deleteFlag=false;
    this->slotTrackInfo = NULL;

    this->t=type;

    //stores the angle which will keep the track both "rideable" and flexible
    this->maxFlex=0;
    /*
     the length of the track does affect the flexibility
    */
    qreal angle = 0;
    if (this->endPoints->count()>1)
    {
        angle = abs((*this->endPointsAngles)[0] - (*this->endPointsAngles)[1]);
    }
    qreal trackLength = angle*((PI*this->radius)/180);
    if (angle==0)
        trackLength=this->radius;

    this->maxFlex=abs(trackLength)/50;
    if (this->slotTrackInfo!=NULL)
        this->maxFlex*=0.85;

    this->maxFlex=min(3.0,this->maxFlex);

    logFile << "        partNumber: " << partNumber.toStdString() << " maxFlex: " << this->maxFlex << endl;
}

ModelItem::~ModelItem()
{
    logFile << "        ~ModelItem at address " << this << ", deleteFlag: " << this->deleteFlag << endl;// << this->partNo->toStdString() << " position of point [0] is:" << this->getEndPoint(0) << endl;

    this->deleteFlag=true;
    int c = this->endPoints->count();
    for (int i = c-1; i >= 0; i--)
    {
        if (this->neighbours->at(i)!=NULL)
        {
            this->neighbours->at(i)->setNeighbour(NULL,this->getEndPoint(i));
        }
        delete this->endPoints->at(i);
        if (this->endPointsHeightGraphics->at(i)!=NULL && this->endPointsHeightGraphics->at(i)->scene()!=NULL)
            app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(this->endPointsHeightGraphics->at(i));
        delete this->endPointsHeightGraphics->at(i);
    }
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

bool ModelItem::getDeleteFlag() const
{
    return this->deleteFlag;
}

void ModelItem::setDeleteFlag()
{
    this->deleteFlag=true;
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

QPointF *ModelItem::getEndPoint(ModelItem *neighbour) const
{
    for (int i = 0; i < this->neighbours->count();i++)
    {
        if (this->neighbours->at(i)==neighbour)
            return this->endPoints->at(i);
    }
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
        if (pointsAreCloseEnough(pt,this->endPoints->at(index),this->getProdLine()->getScaleEnum()/4.0))
            break;
        index++;
    }

    if (index>=this->endPoints->count())
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

GraphicsPathItemModelItem * ModelItem::get2DModel() const
{
    return (GraphicsPathItemModelItem*)GenericModelItem::get2DModel();
}

int ModelItem::set2DModel(GraphicsPathItemModelItem *model)
{
    if (model==NULL)
        return 1;
    GenericModelItem::set2DModel(model);
    return 0;
}

GraphicsPathItemModelItem *ModelItem::get2DModelNoText() const
{
    return (GraphicsPathItemModelItem*)GenericModelItem::get2DModelNoText();
}

int ModelItem::set2DModelNoText(GraphicsPathItemModelItem *model)
{
    if (model==NULL)
        return 1;
    GenericModelItem::set2DModelNoText(model);
    return 0;
}

int ModelItem::generate2DModel(bool text)
{
    QRectF rectOuter;
    QRectF rectInner;

    qreal gauge = 0;
    if (this->slotTrackInfo==NULL)
        gauge = this->getProdLine()->getScaleEnum()/2.0;
    else
        gauge = abs(this->radius-this->radius2)/2.0;

    if (this->radius>0 && this->slotTrackInfo==NULL)
    {

        rectOuter = QRectF(this->radius+gauge/1.0,-this->radius-gauge/1.0,-this->radius*2-2*gauge,this->radius*2+2*gauge);
        rectInner = QRectF(-this->radius+gauge/1.0,-this->radius+gauge/1.0,this->radius*2-2*gauge,this->radius*2-2*gauge);
    }
    else if (this->radius<0 && this->slotTrackInfo==NULL)
    {
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
    QString label(*this->getPartNo());
    label.append(" ");
    QString s("");
    if (this->getTurnAngle()!=0)
        s.setNum(2*this->getTurnAngle(),'f',1);
    else
        s.setNum(2*this->getTurnAngle(),'f',0);

    label.append(s);
    label.append("° ");
    s.setNum(abs(this->radius));
    label.append(s);

    if (text)
    {
        if (this->slotTrackInfo==NULL)
            itemPath.addText(-(12*this->getPartNo()->length())/2,-12-gauge-this->radius,font,*this->getPartNo());
        else
        {
            if (this->t==SC)
                itemPath.addText(-(12*this->getPartNo()->length())/2,-12-this->radius-this->slotTrackInfo->getFstLaneDist(),font,*this->getPartNo());
            else if (this->t==J2)
                itemPath.addText(-(12*this->getPartNo()->length())/2,-12-this->radius-this->slotTrackInfo->getLanesGauge(),font,*this->getPartNo());
            else
                itemPath.addText(-(12*this->getPartNo()->length())/2,-12-this->radius,font,*this->getPartNo());
        }
    }

    if (this->t==C1 || this->t==C2 || this->t==CB)
    {
        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,2*this->getTurnAngle());
        itemPath.arcTo(rectInner,startAngle,2*this->getTurnAngle());

        itemPath.closeSubpath();

        if (this->slotTrackInfo!=NULL)
            itemPath.translate(0,-0*gauge);

    }
    else if (this->t==E1 || this->t==S1 || this->t==HS || this->t==HE || this->t==HC || this->t==SC || ((this->t==X2 || this->t==J5 || this->t==J1 || this->t==J2) && this->slotTrackInfo!=NULL))
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
                qreal d1 = this->slotTrackInfo->getFstLaneDist();
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
            qreal d1 = this->slotTrackInfo->getFstLaneDist()+this->slotTrackInfo->getLanesGauge()/2;
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
            qreal d1 = -this->slotTrackInfo->getFstLaneDist()-this->slotTrackInfo->getLanesGauge()/2;
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
        if (this->getParentWidget()!=NULL)
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
                if (this->getParentWidget()!=NULL)
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

                    rectOuter2 = QRectF(-this->radius2-gauge/1.0,-this->radius2-gauge/1.0,this->radius2*2+2*gauge,this->radius2*2+2*gauge);
                    rectInner2 = QRectF(this->radius2-gauge/1.0,-this->radius2+gauge/1.0,-this->radius2*2+2*gauge,this->radius2*2-2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    startAngle = 90-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1)));

                    itemPath.arcTo(rectInner2,startAngle,(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
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
                itemPath.addPolygon(line);
                itemPath.addPolygon(line2);
                itemPath.translate(0,(this->itemHeight-gauge));
            }
            else
            {
                if (this->getParentWidget()!=NULL)
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
                    itemPath.translate(0,abs(this->radius2-this->radius));

                    rectOuter2 = QRectF(-this->radius2+gauge/1.0,-this->radius2+gauge/1.0,this->radius2*2-2*gauge,this->radius2*2-2*gauge);
                    rectInner2 = QRectF(this->radius2+gauge/1.0,-this->radius2-gauge/1.0,-this->radius2*2-2*gauge,this->radius2*2+2*gauge);

                    startAngle = 90;
                    itemPath.arcMoveTo(rectOuter2,startAngle);
                    itemPath.arcTo(rectOuter2,startAngle,+(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    startAngle = 90-(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1)));

                    itemPath.arcTo(rectInner2,startAngle,+(abs(this->getTurnAngle(0))+abs(this->getTurnAngle(1))));
                    itemPath.translate(0,-abs(this->radius2-this->radius));

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

        if (radius > 0)
        {
            itemPath.translate(-this->getEndPoint(0)->x(),2*radius);
            itemPath.arcMoveTo(rectOuterMinus,startAngle);
            itemPath.arcTo(rectOuterMinus,startAngle,(this->getTurnAngle(2)));
            itemPath.arcMoveTo(rectInnerMinus,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,-(this->getTurnAngle(2)));
            itemPath.translate(this->getEndPoint(0)->x(),-2*radius);

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
            rotatePoint(this->getEndPoint(2),180);
            rotatePoint(this->getEndPoint(3),180);

            itemPath.translate(this->getEndPoint(0)->x(),2*radius);
            itemPath.arcMoveTo(rectOuterMinus,startAngle);
            itemPath.arcTo(rectOuterMinus,startAngle,-(this->getTurnAngle(2)));
            itemPath.arcMoveTo(rectInnerMinus,startAngle);
            itemPath.arcTo(rectInnerMinus,startAngle,(this->getTurnAngle(2)));
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
                this->getEndPoint(2)->setY(-this->getEndPoint(2)->y());
                this->getEndPoint(3)->setY(-this->getEndPoint(3)->y());
            }


            qreal yMove = 0;

            if (this->t==J4)
            {
                //dist(0,2) = 2 * r * sin (alpha/2)
                //dist(0,2)/(2*sin(alpha/2) = r
                qreal rJ4 = dist(this->getEndPoint(0),this->getEndPoint(1))/(2*sin((PI/180)*(abs(this->getTurnAngle(3)-this->getTurnAngle(1))/2)));
                if (this->radius<0)
                    rJ4*=-1;
                yMove=rJ4-this->radius;

                QRectF rectOuter;
                QRectF rectInner;
                rectOuter = QRectF(rJ4+gauge/1.0,-rJ4-gauge/1.0,-rJ4*2-2*gauge,rJ4*2+2*gauge);
                rectInner = QRectF(-rJ4+gauge/1.0,-rJ4+gauge/1.0,rJ4*2-2*gauge,rJ4*2-2*gauge);

                startAngle = 90;
                itemPath.translate(this->getEndPoint(0)->x(),-rJ4+this->radius);
                itemPath.arcMoveTo(rectOuter,startAngle);
                itemPath.arcTo(rectOuter,startAngle,-(this->getTurnAngle(2)));
                itemPath.arcMoveTo(rectInner,startAngle);
                itemPath.arcTo(rectInner,startAngle,(this->getTurnAngle(2)));

                startAngle = 270;
                itemPath.translate(-this->getEndPoint(0)->x(),2*rJ4);
                itemPath.translate(-this->getEndPoint(0)->x(),0);

                itemPath.arcMoveTo(rectOuter,startAngle);
                itemPath.arcTo(rectOuter,startAngle,(this->getTurnAngle(3)));
                itemPath.arcMoveTo(rectInner,startAngle);
                itemPath.arcTo(rectInner,startAngle,-(this->getTurnAngle(3)));
                itemPath.translate(this->getEndPoint(0)->x(),0);
                itemPath.translate(0,-2*this->radius);
            }

            if (this->radius>0)
                itemPath.translate(0,gauge-yMove);
            else
                itemPath.translate(0,-gauge-yMove);
        }
        else
        {
            qreal d1 = this->slotTrackInfo->getFstLaneDist();
            qreal d2 = (this->slotTrackInfo->getNumberOfLanes()-1)*this->slotTrackInfo->getLanesGauge();

            if (this->radius<0)
            {
                d1*=-1;
                d2*=-1;
            }

            QPolygonF part;
            part << QPointF(-this->radius,      -this->radius2);
            part << QPointF(0,                  -this->radius2);
            part << QPointF(0,                  -this->radius2-d1);
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
        if (this->getParentWidget()!=NULL)
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
                dy = -this->itemHeight+2*gauge;
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


            if (this->getParentWidget()==NULL)
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
            line << QPointF(-abs(this->radius2),-abs(this->radius));
            line << QPointF(0,-abs(this->radius));
            line << QPointF(0,abs(this->radius));
            line << QPointF(-abs(this->radius2),abs(this->radius));
            line << QPointF(-abs(this->radius2),-abs(this->radius));

            qreal dx = this->slotTrackInfo->getLanesGaugeEnd();
            if (this->radius<0)
                dx*=1;

            line2 << QPointF(this->slotTrackInfo->getLanesGaugeEnd(),-abs(this->radius));
            line2 << QPointF(abs(this->radius2)+dx,-abs(this->radius));
            line2 << QPointF(abs(this->radius2)+dx,abs(this->radius));
            line2 << QPointF(this->slotTrackInfo->getLanesGaugeEnd(),abs(this->radius));

            qreal y = this->slotTrackInfo->getFstLaneDist()/2.0;

            if (this->radius<0)
            {
                y*=-1;
            }

            for (unsigned int i = 0; i < this->slotTrackInfo->getNumberOfLanes(); i++)
            {
                line2 << QPointF(this->slotTrackInfo->getLanesGaugeEnd(),this->radius-y);
                if (this->radius>0)
                    y += this->slotTrackInfo->getFstLaneDist()/2.0;
                else
                    y -= this->slotTrackInfo->getFstLaneDist()/2.0;
                line2 << QPointF(this->slotTrackInfo->getLanesGaugeEnd()+36,this->radius-y);
                if (this->radius>0)
                    y += this->slotTrackInfo->getFstLaneDist()/2.0;
                else
                    y -= this->slotTrackInfo->getFstLaneDist()/2.0;
                line2 << QPointF(this->slotTrackInfo->getLanesGaugeEnd(),this->radius-y);
                if (this->radius>0)
                    y += this->slotTrackInfo->getLanesGauge()-this->slotTrackInfo->getFstLaneDist();
                else
                    y -= this->slotTrackInfo->getLanesGauge()-this->slotTrackInfo->getFstLaneDist();
            }

            line2 << QPointF(this->slotTrackInfo->getLanesGaugeEnd(),-abs(this->radius));
            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);
        }
    }

    GraphicsPathItemModelItem * gpi = new GraphicsPathItemModelItem(this);
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

        QGraphicsPathItem * lane = new QGraphicsPathItem(gpi);

        QPainterPath lanePath;

        if (this->t==C1 || this->t==CB)
        {
            qreal rectParam = this->slotTrackInfo->getFstLaneDist();
            if (radius <0)
                rectParam *=-1;
            QRectF rectLane;
            for (unsigned int i = 0; i < this->slotTrackInfo->getNumberOfLanes(); i ++)
            {
                rectLane = QRectF(this->radius-rectParam,
                                -this->radius+rectParam,
                                -this->radius*2+2*rectParam,
                                this->radius*2-2*rectParam);

                lanePath.arcMoveTo(rectLane,startAngle);
                lanePath.arcTo(rectLane,startAngle,2*this->getTurnAngle());

                if (radius > 0)
                    rectParam += this->slotTrackInfo->getLanesGauge();
                else
                    rectParam -= this->slotTrackInfo->getLanesGauge();

            }
            lanePath.translate(0,this->radius);
        }
        else if (this->t==S1 || this->t==X1 || this->t==SC || this->t==J5 || this->t==J1 || this->t==J2)
        {
            qreal yCoord = this->slotTrackInfo->getFstLaneDist();
            if (this->radius<0)
                yCoord *=-1;
            for (unsigned int i = 0; i < this->slotTrackInfo->getNumberOfLanes(); i++)
            {
                QPolygonF lane;
                lane << QPointF(-this->radius2,yCoord);
                lane << QPointF(this->radius2,yCoord);

                lanePath.addPolygon(lane);
                if (this->radius>0)
                    yCoord+=this->slotTrackInfo->getLanesGauge();
                else
                    yCoord-=this->slotTrackInfo->getLanesGauge();
            }
            if (this->t==X1)
            {
                qreal d1 = this->slotTrackInfo->getFstLaneDist();
                qreal d2 = (this->slotTrackInfo->getNumberOfLanes()-1)*this->slotTrackInfo->getLanesGauge();

                if (this->radius<0)
                {
                    d1*=-1;
                    d2*=-1;
                }

                qreal xCoord = this->slotTrackInfo->getFstLaneDist();
                if (this->radius<0)
                    xCoord *=-1;
                for (unsigned int i = 0; i < this->slotTrackInfo->getNumberOfLanes(); i++)
                {
                    QPolygonF lane;
                    lane << QPointF(xCoord,-d1);
                    lane << QPointF(xCoord,2*this->radius2-d1);

                    lanePath.addPolygon(lane);

                    if (this->radius>0)
                        xCoord+=this->slotTrackInfo->getLanesGauge();
                    else
                        xCoord-=this->slotTrackInfo->getLanesGauge();
                }
            }
        }
        else if (this->t==C2)
        {
            qreal rectParam = this->slotTrackInfo->getFstLaneDist();
            if (radius <0)
                rectParam *=-1;

            QRectF rectLane;

            int counter = 0;

            for (unsigned int j = 0; j < this->slotTrackInfo->getNumberOfLanes(); j+=2)
            {
                rectLane = QRectF(this->radius-rectParam,
                                -this->radius+rectParam,
                                -this->radius*2+2*rectParam,
                                this->radius*2-2*rectParam);


                lanePath.moveTo(*this->getEndPoint(0+counter*4));
                lanePath.cubicTo(this->getEndPoint(0+counter*4)->x()/2,rectParam,
                                 0,rectParam,
                                 this->getEndPoint(3+counter*4)->x(),this->getEndPoint(3+counter*4)->y());
                lanePath.moveTo(*this->getEndPoint(1+counter*4));
                lanePath.cubicTo(this->getEndPoint(1+counter*4)->x()/2,rectParam,
                                 0,rectParam,
                                 this->getEndPoint(2+counter*4)->x(),this->getEndPoint(2+counter*4)->y());

                if (radius > 0)
                    rectParam += 2*this->slotTrackInfo->getLanesGauge();
                else
                    rectParam -= 2*this->slotTrackInfo->getLanesGauge();
                counter++;
            }


        }

        else if (this->t==HS || this->t==HE || this->t==HC)
        {
            for (unsigned int i = 0; i < this->slotTrackInfo->getNumberOfLanes()*2; i+=2)
            {
                lanePath.moveTo(*this->getEndPoint(i));
                lanePath.cubicTo(this->getEndPoint(i)->x()+abs(this->radius),this->getEndPoint(i)->y(),this->getEndPoint(i+1)->x()-abs(this->radius),this->getEndPoint(i+1)->y(),this->getEndPoint(i+1)->x(),this->getEndPoint(i+1)->y());
            }
        }

        else if (this->t==JM)
        {
            qreal yCoord = this->slotTrackInfo->getFstLaneDist();
            if (this->radius<0)
                yCoord *=-1;
            for (unsigned int i = 0; i < this->slotTrackInfo->getNumberOfLanes(); i++)
            {
                QPolygonF lane;
                if (this->radius2>0)
                    lane << QPointF(-this->radius2,yCoord);
                else
                    lane << QPointF(this->radius2,yCoord);
                lane << QPointF(0,yCoord);

                QPolygonF lane2;
                lane2 << QPointF(this->slotTrackInfo->getLanesGaugeEnd(),yCoord);
                if (this->radius>0)
                    lane2 << QPointF(this->radius2+this->slotTrackInfo->getLanesGaugeEnd(),yCoord);
                else
                    lane2 << QPointF(-this->radius2+this->slotTrackInfo->getLanesGaugeEnd(),yCoord);

                lanePath.addPolygon(lane);
                lanePath.addPolygon(lane2);

                if (this->radius>0)
                    yCoord+=this->slotTrackInfo->getLanesGauge();
                else
                    yCoord-=this->slotTrackInfo->getLanesGauge();
            }
        }
        //don't change this "if" to "else if" - it has to be like that!
        if (this->t==X2 || this->t==J5)
        {
            qreal yCoord = this->slotTrackInfo->getFstLaneDist();
            qreal laneDist = this->slotTrackInfo->getLanesGauge();
            if (this->radius<0)
            {
                yCoord *=-1;
                laneDist*=-1;
            }
            for (unsigned int i = 0; i < this->slotTrackInfo->getNumberOfLanes(); i+=2)
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
                    yCoord+=2*this->slotTrackInfo->getLanesGauge();
                else
                    yCoord-=2*this->slotTrackInfo->getLanesGauge();
            }
        }
        else if (this->t==J1)
        {
            qreal d1 = this->slotTrackInfo->getFstLaneDist()+(this->slotTrackInfo->getNumberOfLanes()-1)*this->slotTrackInfo->getLanesGauge();
            qreal d2 = this->slotTrackInfo->getFstLaneDist()+(this->slotTrackInfo->getNumberOfLanes())*this->slotTrackInfo->getLanesGauge();
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
            qreal d1 =this->slotTrackInfo->getFstLaneDist();
            qreal d2 =-this->slotTrackInfo->getLanesGauge()+this->slotTrackInfo->getFstLaneDist();
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

    qreal num = 0;
    if (this->t!=J5)
        num = (this->itemHeight-this->getProdLine()->getScaleEnum()/2.0);

    if (this->slotTrackInfo!=NULL)
    {
        num = 0;
    }

    if (this->radius>0)
        itemPath.translate(0,this->radius-num);
    else
        itemPath.translate(0,this->radius+num);

    gpi->setPath(itemPath);
    gpi->changeCountPath(10);
    gpi->setToolTip(label);

    if (text)
    {
        if (this->t==J2 && this->slotTrackInfo!=NULL)
            gpi->moveBy(0,this->slotTrackInfo->getLanesGauge());
        this->set2DModel(gpi);
    }
    else
    {
        this->set2DModelNoText(gpi);
    }

    this->updateEndPointsHeightGraphics();

    if (this->slotTrackInfo!=NULL && !text && this->endPointsHeightGraphics->first()!=NULL)
        QObject::connect(gpi,SIGNAL(hpiDialogExec(QGraphicsSceneMouseEvent*)),((HeightPathItem*)this->endPointsHeightGraphics->at(0)),SLOT(mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)));

    return 0;
}

ItemType ModelItem::getType() const
{return this->t;}
ModelFragment *ModelItem::getParentFragment() const
{return this->parentFragment;}

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
        logFile << "        Rotating item " << this->getPartNo()->toStdString() << " by " << angle << endl;

    QString str = QString("rotate item %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(0),QString::number(0));

    str.append(QString::number(angle));

    QTransform rot;
    QTransform rot2;
    rot2 = rot.rotate(angle);
    this->get2DModelNoText()->setTransform(rot,true);
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

    QPointF s(this->itemWidth,this->itemHeight);
    rotatePoint(&s,angle);

    this->itemHeight=s.y();
    this->itemWidth=s.x();

    if (this->slotTrackInfo!=NULL)
    {
        QList<BorderItem*> visitedBorders;
        for (int i = 0; i < this->slotTrackInfo->getBorderEndPointsGraphics()->count();i++)
        {
            rotatePoint(this->slotTrackInfo->getBorderEndPoints()->at(i),angle);
            this->slotTrackInfo->getBorderEndPointsGraphics()->at(i)->setPos(*this->slotTrackInfo->getBorderEndPoints()->at(i));
            if (this->slotTrackInfo->getBorders()->at(i)!=NULL && !visitedBorders.contains(this->slotTrackInfo->getBorders()->at(i)))
            {
                this->slotTrackInfo->getBorders()->at(i)->rotate(angle);
                visitedBorders << this->slotTrackInfo->getBorders()->at(i);
            }
        }
    }

    QString negStr = QString("rotate item %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(0),QString::number(0));
    negStr.append(QString::number(-angle));
    //DONT PRINT COMMAND
    //app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
}

void ModelItem::rotate(qreal angle, QPointF *center, bool printCommand)
{
    if (angle!=0)
        logFile << "        Rotating item " << this->getPartNo()->toStdString() << " by " << angle << " around point [" << center->x() << "," << center->y() << "]"  << endl;

    QString str = QString("rotate item %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(center->x()),QString::number(center->y()));
    str.append(QString::number(angle));

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
        this->getEndPoint(index)->setX(this->getEndPoint(index)->x());
        this->getEndPoint(index)->setY(this->getEndPoint(index)->y());
        rotatePoint(this->getEndPoint(index),angle,center);
        index++;
    }
    QPointF s(this->itemWidth,this->itemHeight);
    rotatePoint(&s,angle);

    this->itemHeight=s.y();
    this->itemWidth=s.x();

    if (this->slotTrackInfo!=NULL)
    {
        QList<BorderItem*> visitedBorders;
        for (int i = 0; i < this->slotTrackInfo->getBorderEndPointsGraphics()->count();i++)
        {
            rotatePoint(this->slotTrackInfo->getBorderEndPoints()->at(i),angle,center);
            this->slotTrackInfo->getBorderEndPointsGraphics()->at(i)->setPos(*this->slotTrackInfo->getBorderEndPoints()->at(i));
            if (this->slotTrackInfo->getBorders()->at(i)!=NULL && !visitedBorders.contains(this->slotTrackInfo->getBorders()->at(i)))
            {
                this->slotTrackInfo->getBorders()->at(i)->rotate(angle,center);
                visitedBorders << this->slotTrackInfo->getBorders()->at(i);
            }
        }
    }

    QString negStr = QString("rotate item %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(center->x()),QString::number(center->y()));
    negStr.append(QString::number(-angle));

    if (printCommand)
    {
        app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
    }
}

void ModelItem::moveBy(qreal dx, qreal dy)
{
    if (dx!=0 && dy!=0)
        logFile << "        Moving item " << this->getPartNo()->toStdString() << " by [" << dx << "," << dy << "]" << endl;
    QTransform mat;
    mat.translate(-dx,-dy);

    this->get2DModelNoText()->moveBy(dx,dy);
    for(int i = 0; i < this->endPoints->count(); i++)
    {
        this->endPoints->at(i)->setX(this->endPoints->at(i)->x()+dx);
        this->endPoints->at(i)->setY(this->endPoints->at(i)->y()+dy);
        if (this->endPointsHeightGraphics->at(i)!=NULL)
            this->endPointsHeightGraphics->at(i)->moveBy(dx,dy);
    }
    if (this->slotTrackInfo!=NULL)
    {
        QList<BorderItem*> visitedBorders;
        for (int i = 0; i < this->slotTrackInfo->getBorderEndPointsGraphics()->count();i++)
        {
            this->slotTrackInfo->getBorderEndPointsGraphics()->at(i)->moveBy(dx,dy);
            this->slotTrackInfo->getBorderEndPoints()->at(i)->setX(this->slotTrackInfo->getBorderEndPoints()->at(i)->x()+dx);
            this->slotTrackInfo->getBorderEndPoints()->at(i)->setY(this->slotTrackInfo->getBorderEndPoints()->at(i)->y()+dy);
            if (this->slotTrackInfo->getBorders()->at(i)!=NULL && !visitedBorders.contains(this->slotTrackInfo->getBorders()->at(i)))
            {
                this->slotTrackInfo->getBorders()->at(i)->moveBy(dx,dy);
                visitedBorders << this->slotTrackInfo->getBorders()->at(i);
            }
        }
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

int ModelItem::adjustHeightProfile(int dz, QPointF *point, bool printCommand, bool adjustOnlyThis)//, bool ignoreRecursionStopper)
{
    if (this->recursionStopperAdj)
        return 0;

    if (printCommand)
    {
        QString str = QString("height point %1 %2 %3 %4 %5").arg(QString::number(dz),QString::number(point->x()),QString::number(point->y()),QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()));
        QString negStr = QString("height point %1 %2 %3 %4 %5").arg(QString::number(-dz),QString::number(point->x()),QString::number(point->y()),QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()));
        app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
    }

    if (point==NULL || dz<=-12300) //dz part - in case when item->getHeightProfileAt(point) is called and point is not found -12345 is returned
        return 1;

    if (dz!=0)
        logFile << "        Raising item " << this->getPartNo()->toStdString() << "by " << dz << " at point (" << point->x() << ", " << point->y() << ")" << endl;

    //find the index of the point
    int index = 0;
    QList<QPointF*>::Iterator ePIter = this->endPoints->begin();
    QList<int>::Iterator heightIter = this->endPointsHeight->begin();
    QList<ModelItem*>::Iterator neighIter = this->neighbours->begin();

    //rectangle approximation of point
    QRectF rect(0,0,0,0);

    while (ePIter!=this->endPoints->end())
    {
        if (pointsAreCloseEnough(point,*ePIter,this->getProdLine()->getScaleEnum()/4.0))
            break;

        index++;
        ePIter++;
        heightIter++;
        neighIter++;
    }
    if (index==this->endPoints->count())
        return 2;

    //adjustHeight of all endPoints and all neighbours

    if (adjustOnlyThis)
        *heightIter+=dz;
    else if (((this->t>=T1 && this->t<=T10) || (this->t>=J1 && this->t<=J5) || this->t==X1) && this->slotTrackInfo==NULL)
    {
        heightIter=this->endPointsHeight->begin();
        ePIter=this->endPoints->begin();
        neighIter=this->neighbours->begin();
        for (int i = 0; i < this->neighbours->count();i++)
        {
            if (!this->recursionStopperAdj)
            {
                if ((*neighIter)!=NULL && !adjustOnlyThis)
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
            if ((*neighIter)!=NULL && !adjustOnlyThis)
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
        if ((*neighIter)!=NULL && !adjustOnlyThis)
        {
            while ((*neighIter)->getEndPoint(nEPIndex)!=NULL)
            {
                //rect = QRectF((*neighIter)->getEndPoint(nEPIndex)->x()-2.5,(*neighIter)->getEndPoint(nEPIndex)->y()-2.5,5,5);
                //if (rect.contains(*point) && !this->recursionStopperAdj )
                if (pointsAreCloseEnough(point,(*neighIter)->getEndPoint(nEPIndex),this->getProdLine()->getScaleEnum()/4.0))
                {
                    this->recursionStopperAdj=true;
                    qreal newDz = dz;//-(*neighIter)->getHeightProfileAt((*neighIter)->getEndPoint(nEPIndex));
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

    qreal gaugeHalf = 8;
    QFont font2;
    font2.setPixelSize(1.5*gaugeHalf);

    QList<QGraphicsPathItem*>::Iterator grIter = this->endPointsHeightGraphics->begin();
    for (int i = 0; i < this->endPointsHeightGraphics->count();i++)
    {
        QPainterPath heightInfoEllipse;
        QPainterPath heightInfoText;
        heightInfoEllipse.addEllipse(*this->getEndPoint(i),gaugeHalf,gaugeHalf);
        QString number;
        QString hText = QString::number(this->endPointsHeight->at(i));
        font2.setPixelSize((1.5*gaugeHalf)/(hText.length()*0.5+0.5));
        if (font2.pixelSize()==1.5*gaugeHalf)
            heightInfoText.addText(this->getEndPoint(i)->x()-gaugeHalf/2.0,this->getEndPoint(i)->y()+gaugeHalf/2,font2,number.setNum(this->endPointsHeight->at(i)));
        else
            heightInfoText.addText(this->getEndPoint(i)->x()-gaugeHalf/1.5,this->getEndPoint(i)->y()+gaugeHalf/2.5,font2,number.setNum(this->endPointsHeight->at(i)));

        QGraphicsPathItem * gpiProfile = new QGraphicsPathItem();
        QGraphicsPathItem * gpiProfileNumber = new QGraphicsPathItem(gpiProfile);

        gpiProfile->setPath(heightInfoEllipse);
        gpiProfileNumber->setPath(heightInfoText);

        QBrush b = gpiProfile->brush();
        if (app!=NULL)
        {
            if (pointsAreCloseEnough(app->getWindow()->getWorkspaceWidget()->getActiveEndPoint(),this->endPoints->at(i),this->getProdLine()->getScaleEnum()/4.0))
            {
                b.setColor(QColor(0,200,255));

            }
            else
                b.setColor(Qt::blue);

            ModelItem * n = this->getNeighbour(i);
            if (n!=NULL)
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
        gpiProfile->setZValue(2000);

        QGraphicsPathItem * toDelete = (*grIter);

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
                if(this->getParentWidget()==app->getWindow()->getWorkspaceWidget())
                {
                    if ((*grIter)==NULL && (grIter)==this->endPointsHeightGraphics->begin())
                        hpi = new HeightPathItem(this);
                    else
                        hpi = new HeightPathItem(*(HeightPathItem*)(this->endPointsHeightGraphics->first()));

                    hpi->setPath(gpiProfile->path());
                    hpi->setBrush(gpiProfile->brush());
                    hpi->setPen(gpiProfile->pen());
                    gpiProfileNumber->setParentItem(hpi);
                    hpi->setVisible(gpiProfile->isVisible());
                    hpi->setFlag(QGraphicsItem::ItemIsSelectable,true);
                    hpi->setZValue(5000);
                    (*this->endPointsHeightGraphics)[i]=(hpi);

                    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(hpi);
                }
            }
        }
        else
        {
            (*this->endPointsHeightGraphics)[i]=gpiProfile;
            if (app!=NULL)
                app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(gpiProfile);
        }

        if (app!=NULL)
        {
            if (((toDelete)!=NULL))
            {
                app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem((toDelete));
                //this should be done, because the childItems would cause memory leaks
                for (int i = 0; i < toDelete->childItems().count(); i++)
                    delete toDelete->childItems().at(i);
                //this cant be done, because caller of this method can be instance of HPI (aditionally - call of this method is not the last line in HPI's method
                //delete toDelete;
            }
        }
        grIter++;
    }
    if (app!=NULL)
    {}

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

        if (pointsAreCloseEnough(*ePIter,point,this->getProdLine()->getScaleEnum()/4.0))
            break;

        index++;
        ePIter++;
        heightIter++;
    }
    if (index==this->endPoints->count())
        return -12345;

    return *heightIter;
}

void ModelItem::setLateralAngle(qreal angle)
{
    if (this->slotTrackInfo==NULL)
        return;
    ((HeightPathItem*)this->endPointsHeightGraphics->at(0))->setAngle(angle);
    ((HeightPathItem*)this->endPointsHeightGraphics->at(0))->adjustHeightOfParentItem();
}

int ModelItem::getEndPointIndex(QPointF *pt)
{
    int index = -1;
    int i = 0;
    while (this->getEndPoint(i)!=NULL && !pointsAreCloseEnough(this->getEndPoint(i),pt,this->getProdLine()->getScaleEnum()/4.0))
        i++;
    if (this->getEndPoint(i)!=NULL)
        index = i;
    return index;
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
        if (pointsAreCloseEnough(pt,this->endPoints->at(index),this->getProdLine()->getScaleEnum()/4.0))
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
    if (this->slotTrackInfo!=NULL)
        delete this->slotTrackInfo;
    this->slotTrackInfo = s;
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
    int ptIndex = 0;//this->endPoints->indexOf(pos,0);

    //correct version
    QList<QPointF*>::Iterator iter = this->endPoints->begin();
    while (iter!=this->endPoints->end())
    {
        if (pointsAreCloseEnough(pos,*iter,this->getProdLine()->getScaleEnum()/4.0))
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
    if (neighbour!=NULL)
    {
        if (neighbour->getDeleteFlag())
            return 1;
    }
    if (this->deleteFlag && neighbour!=NULL)
        return 1;

    if (index >=this->neighbours->count() || index < 0)
        return 2;

    QList<ModelItem*>::Iterator i = this->neighbours->begin();
    while (*i != this->neighbours->at(index))
        i++;

    *i=neighbour;

    if (neighbour!=NULL)
    {
        if (this->slotTrackInfo!=NULL)
        {
            int nIndex = 0;
            for (int i = 0; i < (neighbour)->endPoints->count();i++)
            {
                if (pointsAreCloseEnough((neighbour)->endPoints->at(i),this->getEndPoint(index),this->getProdLine()->getScaleEnum()/4.0))
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

int ModelItem::setNeighbour(ModelItem *neighbour, QPointF *pos)
{
    //logFile << "set neighbour: this=" << this << ", neighbour=" << neighbour << endl;
    if (pos==NULL)
        return 1;

    if (neighbour!=NULL)
    {
        if (neighbour->getDeleteFlag())
            return 1;
    }
    if (this->deleteFlag  && neighbour!=NULL)
        return 1;

    int index = -1;
    QList<ModelItem*>::Iterator iter = this->neighbours->begin();

    for (int i = 0; i < this->endPoints->count();i++)
    {
        if (pointsAreCloseEnough(this->endPoints->at(i),pos,this->getProdLine()->getScaleEnum()/4.0))
        {
            index = i;
            break;
        }
        iter++;
    }

    if (index < 0 || index>=this->endPoints->count())
    {
        return 2;
    }

    *iter=neighbour;

    if (neighbour!=NULL)
    {
        if (this->slotTrackInfo!=NULL)
        {
            int nIndex = 0;
            for (int i = 0; i < (neighbour)->endPoints->count();i++)
            {
                if (pointsAreCloseEnough((neighbour)->endPoints->at(i),pos,this->getProdLine()->getScaleEnum()/4.0))
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
    if (this->t==T1)
        return true;
    if (this->slotTrackInfo!=NULL && (this->t==J1 || this->t==J2))
    {
        if ((unsigned int)index1==2*this->slotTrackInfo->getNumberOfLanes())
        {
            if (this->radius>0)
                return true; //in fact this is incorrect, but it prevents the angle from being modified
            else
            {
                if (index2>=this->endPoints->count())
                    index2-=2;
                if (abs(this->getTurnAngle(index1)-this->getTurnAngle(index2))>165)
                    return true;
                else
                    return false;
            }
        }
    }

    if (index1 >= this->endPoints->count() || index2 >= this->endPoints->count())
        return false;
    if (index1==index2)
        return true;

    if (this->radius<0 && this->t==J1 && this->slotTrackInfo==NULL)
    {
        if (index1==0 && index2==1)
            index2=2;
    }

    qreal dA = ((this->getTurnAngle(index2))-(this->getTurnAngle(index1)));

    if (this->t==J3)
    {
        if (abs(dA>135))
            return true;
    }

    dA = dA;
    dA = abs(dA);

    if (this->radius<0 && this->t==J1)
    {
        if (dA>165)
            return true;
        qreal result = 0;
        result = ((abs(this->getTurnAngle(index2))+dA)-abs(this->getTurnAngle(index1)));
        if (result==0)
            return false;
        else
            return true;
    }
    else if (this->radius>0 && this->t==J2)
    {
        if (dA>165)
            return true;
        qreal result = 0;
        result = ((abs(this->getTurnAngle(index2))+dA)-abs(this->getTurnAngle(index1)));
        if (result==0)
            return false;
        else
            return true;
    }

    //left < right
    if (((this->getTurnAngle(index2))-(this->getTurnAngle(index1)))>=0)//(dA>=0)//(this->getTurnDegree(index2)>this->getTurnDegree(index1))
    {
        dA = abs(dA);
        qreal result = 0;
        if (this->radius<0)
            result = ((abs(this->getTurnAngle(index2))+dA)-abs(this->getTurnAngle(index1)));
        else
            result =(((this->getTurnAngle(index1))+dA)-(this->getTurnAngle(index2)));
        //for straight parts
        if (dA==180)
            return true;
        if (result==0)
            return false;
    }
    else if (dA<180)
    {
        dA = abs(dA);
        qreal result = 0;
        if (this->radius>0)// )
            result = ((abs(this->getTurnAngle(index2))+dA)-abs(this->getTurnAngle(index1)));
        else
            result = abs((-(this->getTurnAngle(index1))+dA)+(this->getTurnAngle(index2)));
        if ((result==0 && this->radius<0) || (result==0 && this->getTurnAngle(index2)<0 && this->getTurnAngle(index1)<0))
            return false;
    }
    return true;
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

qreal ModelItem::getMaxFlex() const
{return this->maxFlex;}

BorderItem::BorderItem(QString &partNo, QString &nameEn, QString &nameCs,
                       qreal dAlpha, qreal radius, QList<QPointF> &endPoints,
                       bool innerBorder, ProductLine *prodLine, QWidget *parentWidg) : GenericModelItem(partNo,nameEn,nameCs,prodLine,parentWidg)
{
    this->deleteFlag=false;
    this->endPoints = new QList <QPointF*>();
    this->neighbours = new QList<ModelItem*>();
    this->dAlpha=dAlpha;
    this->radius=radius;
    this->innerBorder=innerBorder;

    for (int i = 0; i < endPoints.count(); i++)
    {
        this->endPoints->push_back(new QPointF(endPoints[i]));
        this->neighbours->push_back(NULL);
    }
}

BorderItem::~BorderItem()
{
    for (int i = 0; i < this->endPoints->count(); i++)
        delete this->endPoints->at(i);
    delete this->endPoints;
    delete this->neighbours;
}

QPointF * BorderItem::getEndPoint(int index) const
{
    if (index<0 || index >= this->endPoints->count())
        return NULL;

    QPointF * pt = this->endPoints->at(index);
    return pt;

}

int BorderItem::getEndPointsCount() const
{return this->endPoints->count();}
qreal BorderItem::getAngle() const
{return this->dAlpha;}
qreal BorderItem::getRadius() const
{return this->radius;}
bool BorderItem::getInnerBorderFlag() const
{return this->innerBorder;}
bool BorderItem::getDeleteFlag() const
{return this->deleteFlag;}
GraphicsPathItemBorderItem * BorderItem::get2DModel() const
{return (GraphicsPathItemBorderItem *) GenericModelItem::get2DModel();}

int BorderItem::set2DModel(GraphicsPathItemBorderItem * model)
{
    if (model!=NULL)
        GenericModelItem::set2DModel(model);
    else return 1;
    return 0;
}
GraphicsPathItemBorderItem *BorderItem::get2DModelNoText() const
{return (GraphicsPathItemBorderItem *) GenericModelItem::get2DModelNoText();}

int BorderItem::set2DModelNoText(GraphicsPathItemBorderItem * model)
{
    if (model!=NULL)
        GenericModelItem::set2DModelNoText(model);
    else return 1;
    return 0;
}

int BorderItem::generate2DModel(bool text)
{
    QRectF rectOuter;
    QRectF rectInner;
    qreal gauge = 0;
    gauge = 16;

    if (this->innerBorder)
    {
        if (this->radius>0)
        {
            rectInner = QRectF(-this->radius+50,-this->radius+50,this->radius*2-100,this->radius*2-100);
            rectOuter = QRectF(this->radius,-this->radius,-this->radius*2,this->radius*2);
        }
        else if (this->radius<0)
        {
            rectOuter = QRectF(this->radius,-this->radius,-this->radius*2,this->radius*2);
            rectInner = QRectF(-this->radius-50,-this->radius+50,this->radius*2+100,this->radius*2+100);
        }
    }
    else
    {
        if (this->radius>0)
        {
            rectInner = QRectF(-this->radius,-this->radius,this->radius*2,this->radius*2);
            rectOuter = QRectF(this->radius+50,-this->radius-50,-this->radius*2-100,this->radius*2+100);
        }
        else if (this->radius<0)
        {
            rectOuter = QRectF(this->radius,-this->radius,-this->radius*2,this->radius*2);
            rectInner = QRectF(-this->radius-50,-this->radius+50,this->radius*2+100,this->radius*2+100);
        }
    }

    QPainterPath itemPath;
    qreal startAngle = 90-(this->dAlpha/2);
    QFont font;
    font.setPixelSize(20);
    QString label(*this->getPartNo());
    label.append(" ");
    QString s("");
    if (this->getAngle()!=0)
        s.setNum(this->getAngle(),'f',1);
    else
        s.setNum(this->getAngle(),'f',0);
    label.append(s);
    label.append("° ");
    s.setNum(this->radius);
    label.append(s);
    if (text)
    {
        qreal movement = -50;
        if (this->innerBorder)
            movement = 0;
        if (this->dAlpha>2)
            itemPath.addText(-(12*this->getPartNo()->length())/2,movement+24-this->radius-50,font,*this->getPartNo());
        else
            itemPath.addText(-(12*this->getPartNo()->length())/2,0*movement+24-this->radius-50,font,*this->getPartNo());

    }
    if (this->dAlpha>2)
    {
        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,this->dAlpha);
        itemPath.arcTo(rectInner,startAngle,this->dAlpha);
        itemPath.closeSubpath();
        itemPath.translate(0,-0*gauge);
    }
    else
    {
        QPolygonF poly;
        poly << QPointF(-this->radius,0-this->radius);
        poly << QPointF(this->radius,0-this->radius);
        poly << QPointF(this->radius,50-this->radius);
        if (!this->innerBorder)
            poly << QPointF(-this->radius,50-this->radius);
        else
            poly << QPointF(-this->radius,14-this->radius);
        poly << QPointF(-this->radius,0-this->radius);
        itemPath.addPolygon(poly);
    }

    GraphicsPathItemBorderItem * gpi = new GraphicsPathItemBorderItem(this);
    gpi->setFlag(QGraphicsItem::ItemIsMovable,false);
    gpi->setFlag(QGraphicsItem::ItemIsSelectable,true);

    QGraphicsPathItem * qgpi = new QGraphicsPathItem();
    QGraphicsPathItem * qgpi2 = new QGraphicsPathItem();

    qgpi->setParentItem(gpi);
    qgpi2->setParentItem(gpi);

    QPainterPath pathKerbRed;
    QPainterPath pathKerbWhite;
    if (this->dAlpha>2)
    {
        QRectF rect(rectInner.x()-7,rectInner.y()-7,rectInner.size().width()+14,rectInner.size().height()+14);
        if (this->innerBorder)
            rect = QRectF(rectOuter.x()-7,rectOuter.y()+7,rectOuter.size().width()+14,rectOuter.size().height()+14);

            pathKerbRed.arcMoveTo(rect,startAngle);
            pathKerbRed.arcTo(rect,startAngle,this->dAlpha);
            pathKerbRed.translate(0,this->radius);

            pathKerbWhite.arcMoveTo(rect,startAngle);
            pathKerbWhite.arcTo(rect,startAngle,this->dAlpha);
            pathKerbWhite.translate(0,this->radius);
    }
    else
    {
        pathKerbRed.moveTo(-this->radius+0,7);
        pathKerbRed.lineTo(this->radius-0,7);

        pathKerbWhite.moveTo(-this->radius+0,7);
        pathKerbWhite.lineTo(this->radius-0,7);
    }
    qgpi->setPath(pathKerbRed);
    qgpi2->setPath(pathKerbWhite);
    QPen p = qgpi->pen();
    p.setCapStyle(Qt::FlatCap);
    p.setStyle(Qt::SolidLine);
    p.setWidth(14);
    p.setColor(Qt::red);
    qgpi->setPen(p);
    p.setColor(Qt::white);
    p.setStyle(Qt::SolidLine);
    qgpi2->setPen(p);

    if (this->radius>0)
        itemPath.translate(0,this->radius);

    gpi->setPath(itemPath);
    gpi->changeCountPath(10);
    gpi->setToolTip(label);

    qgpi->setZValue(gpi->zValue()+2);
    qgpi2->setZValue(gpi->zValue()+1);

    if (text)
    {
        qreal movement = 25;
        if (this->innerBorder)
            movement = 0;
        if (this->dAlpha>2)
            gpi->moveBy(0,movement-0*24);
        else
            gpi->moveBy(0,0*movement+0*24);

        this->set2DModel(gpi);
    }
    else
    {
        this->set2DModelNoText(gpi);
    }

    return 0;
}

ModelItem * BorderItem::getNeighbour(int index)
{
    if (index<0 || index>=this->neighbours->count())
        return NULL;
    return this->neighbours->at(index);
}
ModelItem * BorderItem::getNeighbour(QPointF * pos)
{
    int ptIndex = 0;
    //correct version
    QList<QPointF*>::Iterator iter = this->endPoints->begin();
    while (iter!=this->endPoints->end())
    {
        if (pointsAreCloseEnough(pos,*iter,this->getProdLine()->getScaleEnum()/4.0))
            break;
        ptIndex++;
        iter++;
    }

    if (ptIndex>=this->endPoints->count())
        return NULL;
    return this->neighbours->at(ptIndex);
}

int BorderItem::setNeighbour(ModelItem * neighbour, int index)
{
    if (neighbour!=NULL)
    {
        if (neighbour->getDeleteFlag())
            return 1;
    }
    if (this->deleteFlag && neighbour!=NULL)
       return 1;

    if (index >=this->neighbours->count() || index < 0)
        return 2;

    QList<ModelItem*>::Iterator i = this->neighbours->begin();
    while (*i != this->neighbours->at(index))
        i++;

    *i=neighbour;
    return 0;
}
int BorderItem::setNeighbour(ModelItem * neighbour, QPointF * pos)
{
    if (pos==NULL)
        return 1;

    if (neighbour!=NULL)
    {
        if (neighbour->getDeleteFlag())
            return 1;
    }
    if (this->deleteFlag  && neighbour!=NULL)
        return 1;

    int index = -1;
    QList<ModelItem*>::Iterator iter = this->neighbours->begin();

    for (int i = 0; i < this->endPoints->count();i++)
    {
        if (pointsAreCloseEnough(this->endPoints->at(i),pos,this->getProdLine()->getScaleEnum()/4.0))
        {
            index = i;
            break;
        }
        iter++;
    }

    if (index < 0 || index>=this->endPoints->count())
        return 2;

    *iter=neighbour;

    return 0;
}

void BorderItem::rotate(qreal angle)
{
    QTransform rot;
    QTransform rot2;
    rot2 = rot.rotate(angle);
    this->get2DModelNoText()->setTransform(rot,true);

    int index = 0;
    while (this->getEndPoint(index)!=NULL)
    {
        rotatePoint(this->getEndPoint(index),angle);
        index++;
    }
}

void BorderItem::rotate(qreal angle, QPointF *center, bool printCommand)
{
    //just to avoid warning
    logFile << "            I want to avoid compiler warnings: " << printCommand << endl;

    QTransform mat;
    QPointF newPointAfterRotation(this->get2DModelNoText()->scenePos().x(),this->get2DModelNoText()->scenePos().y());
    rotatePoint(&newPointAfterRotation,angle,center);

    //rotate 2D
    //set screen position of rotated 2D
    mat.reset();
    mat.rotate(angle);
    this->get2DModelNoText()->setTransform(mat,true);
    this->get2DModelNoText()->setPos(newPointAfterRotation.x(),newPointAfterRotation.y());


    int index = 0;
    while (this->getEndPoint(index)!=NULL)
    {
        rotatePoint(this->getEndPoint(index),angle,center);
        index++;
    }
}

void BorderItem::moveBy(qreal dx, qreal dy)
{
    QTransform mat;
    mat.translate(-dx,-dy);

    this->get2DModelNoText()->moveBy(dx,dy);
    for(int i = 0; i < this->endPoints->count(); i++)
    {
        this->endPoints->at(i)->setX(this->endPoints->at(i)->x()+dx);
        this->endPoints->at(i)->setY(this->endPoints->at(i)->y()+dy);

    }
}


GraphicsPathItem::GraphicsPathItem(GenericModelItem *item, QGraphicsItem *parent) : QObject(), QGraphicsPathItem(parent)
{
    this->setFlag(QGraphicsItem::ItemIsMovable,false);
    this->setFlag(QGraphicsItem::ItemIsSelectable,true);
    this->setAcceptHoverEvents(true);

    this->restrictedCountPath = NULL;
    this->parentItem=item;
    this->dialog=NULL;
    this->contextMenuSBW = NULL;
    this->contextMenuWSW = NULL;
}

GraphicsPathItem::GraphicsPathItem(GenericModelItem *item, const QPainterPath &path, QGraphicsItem *parent) : QGraphicsPathItem(path,parent)
{
    this->setFlag(QGraphicsItem::ItemIsMovable,false);
    this->setFlag(QGraphicsItem::ItemIsSelectable,true);
    this->setAcceptHoverEvents(true);

    this->restrictedCountPath = NULL;
    this->parentItem=item;
    this->dialog=NULL;
    this->contextMenuSBW = NULL;
    this->contextMenuWSW = NULL;
}

GraphicsPathItem::~GraphicsPathItem()
{
    logFile << "~GraphicsPathItem " << this << endl;
    if (this->scene()!=NULL)
        this->scene()->removeItem(this);
    delete this->restrictedCountPath;
}

bool GraphicsPathItem::contains(const QPointF &point) const
{return this->path().boundingRect().contains(point);}
QRectF GraphicsPathItem::boundingRect() const
{return this->path().controlPointRect();}

QPainterPath GraphicsPathItem::shape() const
{
    QPainterPath * p = new QPainterPath();
    p->addRect(this->path().controlPointRect());
    return *p;
}

GenericModelItem *GraphicsPathItem::getParentItem()
{return this->parentItem;}

void GraphicsPathItem::changeCountPath(unsigned int count)
{
    //GraphicsPathItem * newItem = new GraphicsPathItem(this->parentItem,this);
    QGraphicsPathItem * newItem = new QGraphicsPathItem(this);

    QPainterPath pp;
    QFont font;
    font.setPixelSize(20);
    QString text;
    text.setNum(count);
    text.append("x");

    pp.addText(-this->parentItem->getPartNo()->length()*12-48,-12,font,text);

    QBrush b = newItem->brush();
    QPen p = newItem->pen();
    p.setWidth(0);

    if (count==1)
    {
        p.setColor(Qt::yellow);
        b.setColor(Qt::yellow);
    }
    else if (count==0)
    {
        p.setColor(Qt::red);
        b.setColor(Qt::red);
    }
    else
    {
        p.setColor(Qt::green);
        b.setColor(Qt::green);
    }
    b.setStyle(Qt::SolidPattern);
    newItem->setBrush(b);
    newItem->setPath(pp);
    newItem->setPen(p);

    if (this->restrictedCountPath!=NULL)
    {
        this->restrictedCountPath->scene()->removeItem(this->restrictedCountPath);
        delete this->restrictedCountPath;
    }

    this->restrictedCountPath=newItem;

    if (app==NULL)
        this->restrictedCountPath->setVisible(false);
    else
    {
        if (!app->getRestrictedInventoryMode())
            this->restrictedCountPath->setVisible(false);
    }
}

int GraphicsPathItem::initDialog()
{
    this->dialog = new QDialog (app->getWindow());
    this->dialog->setWindowTitle("Change available count");
    QFormLayout * layout = new QFormLayout(this->dialog);

    QLabel * lineTextBox = new QLabel(*(this->parentItem->getProdLine()->getName()),this->dialog);

    lineTextBox->setFixedHeight(30);
    QLabel * partNoTextBox = new QLabel(*(this->parentItem->getPartNo()),this->dialog);
    partNoTextBox->setFixedHeight(30);

    QSpinBox * countSpinBox = new QSpinBox(this->dialog);
    countSpinBox->setRange(0,999);
    countSpinBox->setValue(this->parentItem->getAvailableCount());

    QPushButton * confirm = new QPushButton("Confirm",this->dialog);
    QPushButton * discard = new QPushButton("Discard",this->dialog);

    QWidget::connect(discard,SIGNAL(clicked()),this->dialog,SLOT(close()));
    connect(confirm,SIGNAL(clicked()),this,SLOT(updateItem()));

    if (!app->getUserPreferences()->getLocale()->startsWith("EN"))
    {
        this->dialog->setWindowTitle("Změnit dostupné množství");
        layout->addRow("Název výrobce:", lineTextBox);
        layout->addRow("Číslo dílu:",partNoTextBox);
        layout->addRow("Dostupné množství:",countSpinBox);
        confirm->setText("Potvrdit");
        discard->setText("Storno");
        layout->addRow(confirm,discard);
    }
    else
    {
        layout->addRow("Manufacturer's name:", lineTextBox);
        layout->addRow("Part No:",partNoTextBox);
        layout->addRow("Available count:",countSpinBox);
        layout->addRow(confirm,discard);

    }
    this->dialog->setLayout(layout);
    return 0;
}

int GraphicsPathItem::initMenus()
{
    this->contextMenuSBW = new QMenu(app->getWindow()->getSideBarWidget());
    QAction * action = new QAction ("Change available count",this->contextMenuSBW);
    this->contextMenuSBW->addAction(action);
    this->contextMenuWSW = new QMenu(app->getWindow()->getWorkspaceWidget());
    this->contextMenuWSW->addActions(app->getWindow()->getMainContextMenu()->actions());

    QAction * action2 = new QAction ("Section info",this->contextMenuWSW);
    if (app->getUserPreferences()->getLocale()->startsWith("CS"))
    {
        action->setText("Změnit dostupné množství dílu");
        action2->setText("Informace o úseku trati");
    }
    this->contextMenuWSW->addSeparator();
    this->contextMenuWSW->addAction(action2);
    return 0;
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

    if (actionSelected != NULL && (actionSelected->text().startsWith("Fragment info") || actionSelected->text().startsWith("Change")
                                   || actionSelected->text().startsWith("Informace") || actionSelected->text().startsWith("Změnit")) )
    {
        if (this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget())
        {
            if (this->type()==QGraphicsItem::UserType+2)
            {
                if (((ModelItem*)this->parentItem)->getParentFragment()!=NULL)
                    ((ModelItem*)this->parentItem)->getParentFragment()->showInfoDialog();
            }
        }
        else
            this->dialog->show();
    }
}

void GraphicsPathItem::keyPressEvent(QKeyEvent *event)
{app->sendEvent(app->getWindow()->getWorkspaceWidget(),event);}

void GraphicsPathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QString message;
    message.append(*this->parentItem->getProdLine()->getName());
    message.append(" ");
    message.append(*this->parentItem->getPartNo());
    app->getWindow()->statusBar()->showMessage(message);

    //just to avoid warning
    event->setLastPos(event->lastPos());
}

void GraphicsPathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    app->getWindow()->statusBar()->showMessage("");
    //just to avoid warning
    event->setLastPos(event->lastPos());
}

QGraphicsPathItem *GraphicsPathItem::getRestrictedCountPath() const
{return this->restrictedCountPath;}

void GraphicsPathItem::updateItem()
{
    unsigned int count = ((QSpinBox*)this->dialog->layout()->itemAt(5)->widget())->value();
    this->parentItem->setAvailableCount(count);
    this->changeCountPath(count);
    this->dialog->close();
}

GraphicsPathItemModelItem::GraphicsPathItemModelItem(ModelItem * item, QGraphicsItem * parent) : GraphicsPathItem(item,parent)
{}
GraphicsPathItemModelItem::GraphicsPathItemModelItem(ModelItem * item, const QPainterPath & path, QGraphicsItem * parent) : GraphicsPathItem(item,path,parent)
{}
GraphicsPathItemModelItem::~GraphicsPathItemModelItem()
{
    logFile << "~GraphicsPathItemModelItem " << this << endl;
    if (this->scene()!=NULL)
        this->scene()->removeItem(this);

    if (this->getParentItem()->getParentFragment()!=NULL)
        app->getWindow()->getWorkspaceWidget()->deselectItem((this)->getParentItem());

    if (this->getParentItem()->getSlotTrackInfo()!=NULL)
    {
        //this will delete slot track lanes' qgpi
        delete this->childItems().first();
    }
}

void GraphicsPathItemModelItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=true;
    app->getWindow()->getWorkspaceWidget()->selectItem(this->getParentItem());
    QGraphicsPathItem::mousePressEvent(event);
}
void GraphicsPathItemModelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=false;
    QGraphicsPathItem::mouseReleaseEvent(event);
    if (this->getParentItem()->getParentWidget()==app->getWindow()->getWorkspaceWidget())
        app->getWindow()->getWorkspaceWidget()->connectFragments(this->getParentItem()->getParentFragment());
}

void GraphicsPathItemModelItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((this->mousePressed) && this->getParentItem()->getParentFragment()!=NULL)
    {
        if (this->getParentItem()->getParentWidget()==app->getWindow()->getWorkspaceWidget() && app->getWindow()->getWorkspaceWidget()->getRotationMode())
        {
            //somehow get the angle of rotation
            QPointF center(this->scenePos().x(),this->scenePos().y());
            QPointF * newPoint = new QPointF(*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
            qreal angle = event->scenePos().x()-event->lastScenePos().x();

            rotatePoint(newPoint,angle,&center);
            app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);

            this->getParentItem()->getParentFragment()->rotate(angle,&center);
        }
        else
        {
            QPointF diff = event->lastScenePos()-event->scenePos();
            this->getParentItem()->getParentFragment()->moveBy(-diff.x(),-diff.y());

            if (this->getParentItem()->getParentFragment()==app->getWindow()->getWorkspaceWidget()->getActiveFragment())
            {
                QPointF * newPoint = new QPointF(*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
                newPoint->setX(newPoint->x()-diff.x());
                newPoint->setY(newPoint->y()-diff.y());
                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);
            }
        }
    }

    QGraphicsPathItem::mouseMoveEvent(event);
}

ModelItem *GraphicsPathItemModelItem::getParentItem()
{return (ModelItem*)GraphicsPathItem::getParentItem();}

int GraphicsPathItemModelItem::type() const
{return Type;}

void GraphicsPathItemModelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (this->getParentItem()->getParentWidget()!=app->getWindow()->getWorkspaceWidget() && (app->getRestrictedInventoryMode() && this->getRestrictedCountPath()!=NULL))
    {
        this->getRestrictedCountPath()->setVisible(true);
    }
    else if ((!app->getRestrictedInventoryMode() || this->getParentItem()->getParentWidget()==app->getWindow()->getWorkspaceWidget()) && this->getRestrictedCountPath()!=NULL)
        this->getRestrictedCountPath()->setVisible(false);

    if (this->isSelected())
    {
        if (this->getParentItem()->getSlotTrackInfo()!=NULL)
        {
            QBrush b = this->brush();
            b.setColor(QColor(64,110,64));
            b.setStyle(Qt::SolidPattern);
            this->setBrush(b);
        }
        else
        {
            QPen p = this->pen();
            p.setColor(QColor(0,200,0));
            this->setPen(p);
        }
    }
    else
    {
        if (this->getParentItem()->getSlotTrackInfo()!=NULL)
        {
            QBrush b = this->brush();
            b.setColor(QColor(64,68,64));
            b.setStyle(Qt::SolidPattern);
            this->setBrush(b);
        }
        else
        {
            QPen p = this->pen();
            p.setColor(QColor(0,0,0));
            this->setPen(p);
        }
    }

    if (painter->brush().color()==Qt::transparent)
        this->setBrush(painter->brush());

    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= !QStyle::State_Selected;
    QGraphicsPathItem::paint(painter,&myoption,widget);
}


void GraphicsPathItemModelItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!app->getWindow()->getWorkspaceWidget()->getHeightProfileMode())
    {
        if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()!=NULL)
        {
            for (int i = 0; i < app->getWindow()->getWorkspaceWidget()->getActiveFragment()->getProductLines()->count(); i++)
            {
                if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()->getProductLines()->at(i)->getType()!=this->getParentItem()->getProdLine()->getType())
                {
                    app->getAppData()->setMessageDialogText("You cannot connect rail and slot parts.","Autodráhové a železniční díly nelze spojovat");
                    app->getAppData()->getMessageDialog()->exec();
                    QGraphicsPathItem::mouseDoubleClickEvent(event);
                    return;
                }
            }
        }

        if (this->getParentItem()->getParentFragment()==NULL)
            app->getWindow()->getWorkspaceWidget()->makeItem(this->getParentItem(),NULL,event->scenePos());

    }
    else if (this->getParentItem()->getSlotTrackInfo()!=NULL)
    {
        emit this->hpiDialogExec(event);
    }

    //this fixes the bug with "no background" of SBW scene
    QBrush b = app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->backgroundBrush();
    b.setColor(QColor(255,255,255));
    app->getWindow()->getSideBarWidget()->getCurrentScene()->setBackgroundBrush(b);

    QGraphicsPathItem::mouseDoubleClickEvent(event);
}


void GraphicsPathItemModelItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    ItemType t = this->getParentItem()->getType();

    QString message;
    message.append(*this->getParentItem()->getProdLine()->getName());
    message.append(" ");
    message.append(*this->getParentItem()->getPartNo());

    bool english = true;
    if (app->getUserPreferences()->getLocale()->startsWith("CS"))
        english = false;

    if (t==C1 || t==C2 || t==J1 || t==J2 || t==J3 || t==CB)
    {
        if (english)
            message.append(", Radius: ");
        else
            message.append(", Poloměr: ");
        message.append(QString::number(abs(this->getParentItem()->getRadius())));
    }
    else
    {
        if (english)
            message.append(", Length: ");
        else
            message.append(", Délka: ");
        if (this->getParentItem()->getSlotTrackInfo()!=NULL)
            message.append(QString::number(abs(2*this->getParentItem()->getSecondRadius())));
        else
            message.append(QString::number(abs(2*this->getParentItem()->getRadius())));
    }

    if (this->getParentItem()->getSlotTrackInfo()!=NULL && (t==C1 || t==C2 || t==CB))
    {
        if (english)
            message.append(QString(", Inner radius: %1").arg(QString::number(abs(this->getParentItem()->getSecondRadius()))));
        else
            message.append(QString(", Vnitřní poloměr: %1").arg(QString::number(abs(this->getParentItem()->getSecondRadius()))));
    }

    if (t!=E1)
    {
        qreal angle = this->getParentItem()->getTurnAngle(1)-this->getParentItem()->getTurnAngle(0);
        if (this->getParentItem()->leftRightDifference180(0,1))
            angle+=180;
        if (t==C1 || t==C2 || t==J1 || t==J2 || t==J3 || t==CB)
        {
            if (english)
                message.append(QString(", Angle: %1").arg(QString::number(angle)));
            else
                message.append(QString(", Úhel: %1").arg(QString::number(angle)));
        }
    }

    //message.append(*this->parentItem->getPartNo());
    app->getWindow()->statusBar()->showMessage(message);

    //just to avoid warning
    event->setLastPos(event->lastPos());
}

GraphicsPathItemBorderItem::GraphicsPathItemBorderItem(BorderItem * item, QGraphicsItem * parent) : GraphicsPathItem(item,parent) //: QGraphicsPathItem(parent)
{
    this->setFlag(QGraphicsItem::ItemIsMovable,true);
    this->setFlag(QGraphicsItem::ItemIsSelectable,true);

    this->mousePressed=false;
}

GraphicsPathItemBorderItem::~GraphicsPathItemBorderItem()
{
    if (app->getWindow()!=NULL && this->scene()==app->getWindow()->getWorkspaceWidget()->getGraphicsScene())
        delete this->getParentItem();

    //delete kerb
    for (int i = this->childItems().count()-1; i >=0; i--)
    {
        if (this->childItems().at(i)!=this->getRestrictedCountPath())
            delete this->childItems().at(i);
    }
}

BorderItem * GraphicsPathItemBorderItem::getParentItem()
{return (BorderItem*)GraphicsPathItem::getParentItem();}

int GraphicsPathItemBorderItem::type() const
{return Type;}

void GraphicsPathItemBorderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (this->getParentItem()->getParentWidget()!=app->getWindow()->getWorkspaceWidget() && (app->getRestrictedInventoryMode() && this->getRestrictedCountPath()!=NULL))
    {
        this->getRestrictedCountPath()->setVisible(true);
    }
    else if ((!app->getRestrictedInventoryMode() || this->getParentItem()->getParentWidget()==app->getWindow()->getWorkspaceWidget()) && this->getRestrictedCountPath()!=NULL)
        this->getRestrictedCountPath()->setVisible(false);

    if (this->isSelected())
    {
        QBrush b = this->brush();
        b.setColor(QColor(230,230,64));
        b.setStyle(Qt::SolidPattern);
        this->setBrush(b);
    }
    else
    {
        QBrush b = this->brush();
        b.setColor(QColor(230,150,64));
        b.setStyle(Qt::SolidPattern);
        this->setBrush(b);
    }

    if (painter->brush().color()==Qt::transparent)
        this->setBrush(painter->brush());

    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= !QStyle::State_Selected;

    QGraphicsPathItem::paint(painter,&myoption,widget);
}
void GraphicsPathItemBorderItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!app->getWindow()->getWorkspaceWidget()->getHeightProfileMode())
    {
        if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()!=NULL)
        {
            if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()->getFragmentItems()->first()->getSlotTrackInfo()==NULL)
            {
                app->getAppData()->setMessageDialogText("You cannot connect rail and slot parts.","Autodráhové a železniční díly nelze spojovat");
                app->getAppData()->getMessageDialog()->exec();
                QGraphicsPathItem::mouseDoubleClickEvent(event);
                return;
            }
        }

        if (this->getParentItem()->getParentWidget()!=app->getWindow()->getWorkspaceWidget())
            app->getWindow()->getWorkspaceWidget()->makeBorder(this->getParentItem());
        else
            this->setTransform(this->transform().scale(-1,1));
    }
    //this fixes the bug with "no background" of SBW scene
    QBrush b = app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->backgroundBrush();
    b.setColor(QColor(255,255,255));
    app->getWindow()->getSideBarWidget()->getCurrentScene()->setBackgroundBrush(b);

    QGraphicsPathItem::mouseDoubleClickEvent(event);
}

void GraphicsPathItemBorderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=true;
    QGraphicsPathItem::mousePressEvent(event);
}
void GraphicsPathItemBorderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=false;
    QGraphicsPathItem::mouseReleaseEvent(event);
}
void GraphicsPathItemBorderItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((this->mousePressed) && this->getParentItem()->getParentWidget()==app->getWindow()->getWorkspaceWidget())
    {
        if (app->getWindow()->getWorkspaceWidget()->getRotationMode())
        {
            QPointF center(this->scenePos().x(),this->scenePos().y());
            QPointF * newPoint = new QPointF(*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
            qreal angle = event->scenePos().x()-event->lastScenePos().x();

            rotatePoint(newPoint,angle,&center);
            app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);

            this->getParentItem()->getNeighbour(0)->getParentFragment()->rotate(angle,&center);
        }
        else
        {
            QPointF diff = event->lastScenePos()-event->scenePos();
            this->getParentItem()->getNeighbour(0)->getParentFragment()->moveBy(-diff.x(),-diff.y());

            if (this->getParentItem()->getNeighbour(0)->getParentFragment()==app->getWindow()->getWorkspaceWidget()->getActiveFragment())
            {
                QPointF * newPoint = new QPointF(*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
                newPoint->setX(newPoint->x()-diff.x());
                newPoint->setY(newPoint->y()-diff.y());
                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);
            }
        }
    }
    QGraphicsPathItem::mouseMoveEvent(event);
}

void GraphicsPathItemBorderItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{

    QString message;
    message.append(*this->getParentItem()->getProdLine()->getName());
    message.append(" ");
    message.append(*this->getParentItem()->getPartNo());

    bool english = true;
    if (app->getUserPreferences()->getLocale()->startsWith("CS"))
        english = false;

    if (this->getParentItem()->getAngle()>5)
    {
        if (english)
            message.append(", Radius: ");
        else
            message.append(", Poloměr: ");

        qreal angle = (this->getParentItem()->getAngle());
        message.append(QString::number(abs(this->getParentItem()->getRadius())));
        if (english)
            message.append(QString(", Angle: %1").arg(QString::number(angle)));
        else
            message.append(QString(", Úhel: %1").arg(QString::number(angle)));
    }
    else
    {
        if (english)
            message.append(", Length: ");
        else
            message.append(", Délka: ");

        message.append(QString::number(abs(2*this->getParentItem()->getRadius())));
    }

    app->getWindow()->statusBar()->showMessage(message);

    //just to avoid warning
    event->setLastPos(event->lastPos());
}

GraphicsPathItemVegetationItem::GraphicsPathItemVegetationItem(VegetationItem *item, QGraphicsItem * parent) : GraphicsPathItem(item,parent)
{this->mousePressed=false;}

GraphicsPathItemVegetationItem::~GraphicsPathItemVegetationItem()
{
    for (int i = this->childItems().count()-1; i >=0; i--)
    {
        if (this->childItems().at(i)!=this->getRestrictedCountPath())
        {
            delete this->childItems().at(i);
        }
    }
}
VegetationItem * GraphicsPathItemVegetationItem::getParentItem()
{return (VegetationItem*)GraphicsPathItem::getParentItem();}

int GraphicsPathItemVegetationItem::type() const
{return Type;}
void GraphicsPathItemVegetationItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!app->getWindow()->getWorkspaceWidget()->getHeightProfileMode())
    {

        if (this->getParentItem()->getParentWidget()!=app->getWindow()->getWorkspaceWidget())
            app->getWindow()->getWorkspaceWidget()->makeVegetation(this->getParentItem());
    }

    //this fixes the bug with "no background" of SBW scene
    QBrush b = app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->backgroundBrush();
    b.setColor(QColor(255,255,255));
    app->getWindow()->getSideBarWidget()->getCurrentScene()->setBackgroundBrush(b);

    QGraphicsPathItem::mouseDoubleClickEvent(event);
}
void GraphicsPathItemVegetationItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=true;
    app->getWindow()->getWorkspaceWidget()->selectItem(this->getParentItem());
    QGraphicsPathItem::mousePressEvent(event);
}
void GraphicsPathItemVegetationItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    this->mousePressed=false;
    //QGraphicsPathItem::mouseReleaseEvent(event);

    //just to avoid warning
    event->setLastPos(event->lastPos());
}
void GraphicsPathItemVegetationItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((this->mousePressed) && this->getParentItem()->getParentWidget()==app->getWindow()->getWorkspaceWidget())
    {
        if (app->getWindow()->getWorkspaceWidget()->getRotationMode())
        {
            QPointF center(this->scenePos().x(),this->scenePos().y());
            qreal angle = event->scenePos().x()-event->lastScenePos().x();
            //this->getParentItem()->rotate(angle,&center);
            for (int i = 0; i < app->getWindow()->getWorkspaceWidget()->getSelectionVegetation()->count(); i++)
            {
                app->getWindow()->getWorkspaceWidget()->getSelectionVegetation()->at(i)->rotate(angle,&center);
            }
        }
        else
        {
            QPointF diff = event->lastScenePos()-event->scenePos();
            //this->getParentItem()->moveBy(-diff.x(),-diff.y());
            for (int i = 0; i < app->getWindow()->getWorkspaceWidget()->getSelectionVegetation()->count(); i++)
            {
                app->getWindow()->getWorkspaceWidget()->getSelectionVegetation()->at(i)->moveBy(-diff.x(),-diff.y());
            }
        }
    }
    QGraphicsPathItem::mouseMoveEvent(event);
}

void GraphicsPathItemVegetationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (this->getParentItem()->getParentWidget()!=app->getWindow()->getWorkspaceWidget() && (app->getRestrictedInventoryMode() && this->getRestrictedCountPath()!=NULL))
    {
        this->getRestrictedCountPath()->setVisible(true);
    }
    else if ((!app->getRestrictedInventoryMode() || this->getParentItem()->getParentWidget()==app->getWindow()->getWorkspaceWidget()) && this->getRestrictedCountPath()!=NULL)
        this->getRestrictedCountPath()->setVisible(false);

    QPen p = this->pen();
    p.setWidth(0);

    if (this->isSelected())
    {
        QBrush b = this->brush();
        if (this->getParentItem()->getSeason()->startsWith("Spring") || this->getParentItem()->getSeason()->startsWith("Summer"))
        {
            b.setColor(QColor(0,200,64));
            p.setColor(QColor(0,200,64));
        }
        else if (this->getParentItem()->getSeason()->startsWith("Autumn"))
        {
            b.setColor(QColor(250,220,64));
            p.setColor(QColor(250,220,64));
        }
        else
        {
            b.setColor(QColor(150,150,254));
            p.setColor(QColor(150,150,254));
        }

        b.setStyle(Qt::SolidPattern);
        this->setBrush(b);
    }
    else
    {
        QBrush b = this->brush();

        if (this->getParentItem()->getSeason()->startsWith("Spring") || this->getParentItem()->getSeason()->startsWith("Summer"))
        {
            b.setColor(QColor(0,150,64));
            p.setColor(QColor(0,150,64));
        }
        else if (this->getParentItem()->getSeason()->startsWith("Autumn"))
        {
            b.setColor(QColor(190,150,64));
            p.setColor(QColor(190,150,64));
        }
        else
        {
            b.setColor(QColor(200,200,254));
            p.setColor(QColor(200,200,254));
        }
        b.setStyle(Qt::SolidPattern);
        this->setBrush(b);

    }
    this->setPen(p);

    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= !QStyle::State_Selected;

    QGraphicsPathItem::paint(painter,&myoption,widget);
}

VegetationItem::VegetationItem(QString &partNo, QString &nameEn, QString &nameCs, QString &season,
                               qreal width, qreal height, ProductLine *prodLine, QWidget *parentWidget) : GenericModelItem(partNo,nameEn,nameCs,prodLine,parentWidget)
{
    this->season = new QString(season);
    this->currentRotation=0;
    this->itemHeight=height;
    this->itemWidth=width;

}

VegetationItem::~VegetationItem()
{
    if (this->get2DModelNoText()->scene()!=NULL)
        app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(this->get2DModelNoText());

    delete this->season;

    this->season= NULL;
}
qreal VegetationItem::getItemWidth() const
{return this->itemWidth;}
qreal VegetationItem::getItemHeight() const
{return this->itemHeight;}

qreal VegetationItem::getRotation() const
{return this->currentRotation;}

void VegetationItem::setRotation(qreal alpha)
{this->currentRotation=alpha;}



QString *VegetationItem::getSeason() const
{return this->season;}
GraphicsPathItemVegetationItem * VegetationItem::get2DModel() const

{return (GraphicsPathItemVegetationItem*)GenericModelItem::get2DModel();}
int VegetationItem::set2DModel(GraphicsPathItemVegetationItem *model)
{
    if (model==NULL)
        return -1;
    else
        GenericModelItem::set2DModel(model);
    return 0;
}
GraphicsPathItemVegetationItem *VegetationItem::get2DModelNoText() const
{return (GraphicsPathItemVegetationItem *)GenericModelItem::get2DModelNoText();}
int VegetationItem::set2DModelNoText(GraphicsPathItemVegetationItem * model)
{
    if (model==NULL)
        return -1;
    else
        GenericModelItem::set2DModelNoText(model);
    return 0;
}
int VegetationItem::generate2DModel(bool text)
{
    GraphicsPathItemVegetationItem * gpi = new GraphicsPathItemVegetationItem(this);
    QPainterPath pp;

    QFont font;
    font.setPixelSize(20);

    if (text)
        pp.addText((-(12*this->getPartNo()->length())/2),-12,font,*this->getPartNo());

    pp.addEllipse(-this->itemWidth/2,-this->itemHeight/4,this->itemHeight/2,this->itemHeight/2);
    pp.addEllipse(-this->itemWidth*0.375,-this->itemHeight*0.375,this->itemHeight*0.75,this->itemHeight*0.75);
    pp.addEllipse(0,-this->itemHeight*0.25,this->itemHeight*0.5,this->itemHeight*0.5);
    if (this->itemHeight!=this->itemWidth)
    {
        pp.addEllipse(0,0,this->itemHeight*0.5,this->itemHeight*0.5);
        pp.addEllipse(-this->itemHeight/2,-this->itemHeight/2,this->itemHeight,this->itemHeight);
    }
    pp.addEllipse(-this->itemWidth*0.375,-this->itemHeight/2,this->itemHeight*0.5,this->itemHeight*0.5);
    pp.addEllipse(this->itemWidth/5,-this->itemHeight/4,this->itemHeight/2,this->itemHeight/2);
    pp.addEllipse(this->itemWidth/8,-this->itemHeight/2,this->itemHeight*0.5,this->itemHeight*0.5);

    if (this->season->startsWith("Spring"))
    {
        QGraphicsPathItem * spring = new QGraphicsPathItem(gpi);
        QPainterPath pathSpring;
        pathSpring.addEllipse(-this->itemWidth/4,-this->itemHeight/18,this->itemHeight/20,this->itemHeight/20);
        pathSpring.addEllipse(-this->itemWidth/5,-this->itemHeight*0.15,this->itemHeight/20,this->itemHeight/20);
        pathSpring.addEllipse(0,-this->itemHeight*0.17,this->itemHeight/20,this->itemHeight/20);
        //pathSpring.addEllipse(-this->itemWidth*0.75,-this->itemHeight/20,this->itemHeight/20,this->itemHeight/20);
        pathSpring.addEllipse(this->itemWidth/5,-this->itemHeight/8,this->itemHeight/20,this->itemHeight/20);
        pathSpring.addEllipse(this->itemWidth/8,-this->itemHeight/9,this->itemHeight/20,this->itemHeight/20);
        spring->setPath(pathSpring);
        QBrush b = spring->brush();
        b.setColor(Qt::white);
        b.setStyle(Qt::SolidPattern);
        spring->setBrush(b);
        QPen p = spring->pen();
        p.setColor(Qt::white);
        spring->setPen(p);
    }

    pp.setFillRule(Qt::WindingFill);
    gpi->setPath(pp);

    if (text)
        this->set2DModel(gpi);
    else
        this->set2DModelNoText(gpi);

    gpi->changeCountPath(this->getAvailableCount());
    gpi->setFlag(QGraphicsItem::ItemIsMovable,false);
    gpi->setFlag(QGraphicsItem::ItemIsSelectable,true);

    return 0;
}


void VegetationItem::rotate (qreal angle)
{
    if (angle!=0)
        logFile << "        Rotating vegetation " << this->getPartNo()->toStdString() << " by " << angle << " around point [" << 0 << "," << 0 << "]"  << endl;

    QString str = QString("rotate vegetation %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(0),QString::number(0));

    str.append(QString::number(angle));

    QTransform mat;
    QPointF newPointAfterRotation(this->get2DModelNoText()->scenePos().x(),this->get2DModelNoText()->scenePos().y());

    rotatePoint(&newPointAfterRotation,angle);

    //rotate 2D
    //set screen position of rotated 2D
    mat.reset();
    mat.rotate(angle);
    this->get2DModelNoText()->setTransform(mat,true);
    this->get2DModelNoText()->setPos(newPointAfterRotation.x(),newPointAfterRotation.y());

    QPointF s(this->itemWidth,this->itemHeight);
    rotatePoint(&s,angle);
    this->currentRotation+=angle;

    this->itemHeight=s.y();
    this->itemWidth=s.x();

    QString negStr = QString("rotate vegetation %1 %2 0 0 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()));
    negStr.append(QString::number(-angle));

    app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
}
void VegetationItem::rotate (qreal angle,QPointF * center, bool printCommand)
{
    //just to avoid warning
    logFile << "            I want to avoid compiler warnings: " << printCommand << endl;

    if (angle!=0)
        logFile << "        Rotating vegetation " << this->getPartNo()->toStdString() << " by " << angle << " around point [" << center->x() << "," << center->y() << "]"  << endl;

    QString str = QString("rotate vegetation %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(center->x()),QString::number(center->y()));
    str.append(QString::number(angle));

    QTransform mat;
    QPointF newPointAfterRotation(this->get2DModelNoText()->scenePos().x(),this->get2DModelNoText()->scenePos().y());

    rotatePoint(&newPointAfterRotation,angle,center);

    //rotate 2D
    //set screen position of rotated 2D
    mat.reset();
    mat.rotate(angle);
    this->get2DModelNoText()->setTransform(mat,true);
    this->get2DModelNoText()->setPos(newPointAfterRotation.x(),newPointAfterRotation.y());

    QPointF s(this->itemWidth,this->itemHeight);
    rotatePoint(&s,angle);

    this->currentRotation+=angle;

    this->itemHeight=s.y();
    this->itemWidth=s.x();



    QString negStr = QString("rotate vegetation %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(center->x()),QString::number(center->y()));
    negStr.append(QString::number(-angle));

    app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
}

void VegetationItem::moveBy(qreal dx, qreal dy)
{
    this->get2DModelNoText()->moveBy(dx,dy);
}
void VegetationItem::moveBy(qreal dx, qreal dy, bool printCommand)
{
    QString str = (QString("move vegetation %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(dx),QString::number(dy)));
    this->get2DModelNoText()->moveBy(dx,dy);
    QString negStr = (QString("move vegetation %1 %2 %3 %4 ").arg(QString::number(this->get2DModelNoText()->scenePos().x()),QString::number(this->get2DModelNoText()->scenePos().y()),QString::number(-dx),QString::number(-dy)));
    if (printCommand)
        app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
}

ProductLine::ProductLine(QString &name,QString &scale, ScaleEnum s, QString &gauge, bool type, QList<ModelItem*> &items)
{
    this->name=new QString(name);
    this->scale=new QString(scale);
    this->scaleE=s;
    this->type=type;
    this->gauge=new QString(gauge);
    this->items=new QList<ModelItem*>(items);
    this->borderItems=new QList<BorderItem*>();
    this->vegetationItems=new QList<VegetationItem*>();
    this->maxTrackRadius=0;
    this->minTrackRadius=50000;
    this->maxStraightLength=0;
    this->lastFoundIndex=0;

}
ProductLine::ProductLine(QString &name,QString &scale, ScaleEnum s, QString &gauge, bool type)
{
    this->name=new QString(name);
    this->scale=new QString(scale);
    this->scaleE=s;
    this->type=type;
    this->items = new QList<ModelItem*>();
    this->borderItems=new QList<BorderItem*>();
    this->vegetationItems=new QList<VegetationItem*>();
    this->gauge=new QString(gauge);
    this->maxTrackRadius=0;
    this->minTrackRadius=50000;
    this->maxStraightLength=0;
    this->lastFoundIndex=0;
}

ProductLine::~ProductLine()
{
    for (int i = 0; i < this->items->count(); i++)
        delete this->items->at(i);
    for (int i = 0; i < this->borderItems->count(); i++)
        delete this->borderItems->at(i);
    for (int i = 0; i < this->vegetationItems->count(); i++)
        delete this->vegetationItems->at(i);

    delete this->vegetationItems;
    delete this->borderItems;
    delete this->items;
    delete this->name;
    delete this->scale;
    delete this->gauge;
}


QString * ProductLine::getName() const
{return this->name;}

QString * ProductLine::getScale() const
{return this->scale;}

ScaleEnum ProductLine::getScaleEnum() const
{return this->scaleE;}

bool ProductLine::getType() const
{return this->type;}

QList<ModelItem*> * ProductLine::getItemsList() const
{return this->items;}

QList<BorderItem *> *ProductLine::getBorderItemsList() const
{return this->borderItems;}

QList<VegetationItem *> *ProductLine::getVegetationItemsList() const
{return this->vegetationItems;}

ModelItem *ProductLine::findItemByPartNo(QString *partNo)
{
    ModelItem * ret = NULL;
    if (this->lastFoundIndex>=this->items->count())
        this->lastFoundIndex=0;
    for (int i = this->lastFoundIndex; i < this->items->count();i++)
    {
        if (this->items->at(i)->getPartNo()->startsWith(*partNo))
        {
            ret = this->items->at(i);
            this->lastFoundIndex=i;
            break;
        }
    }
    if (ret==NULL)
    {
        for (int i = 0; i < this->lastFoundIndex;i++)
        {
            if (this->items->at(i)->getPartNo()->startsWith(*partNo))
            {
                ret = this->items->at(i);
                this->lastFoundIndex=i;
                break;
            }
        }
    }
    return ret;

}

BorderItem *ProductLine::findBorderItemByPartNo(QString *partNo)
{
    BorderItem * ret = NULL;
    if (this->lastFoundIndex>=this->borderItems->count())
        this->lastFoundIndex=0;
    for (int i = this->lastFoundIndex; i < this->borderItems->count();i++)
    {
        if (this->borderItems->at(i)->getPartNo()->startsWith(*partNo))
        {
            ret = this->borderItems->at(i);
            this->lastFoundIndex=i;
            break;
        }
    }
    if (ret==NULL)
    {
        for (int i = 0; i < this->lastFoundIndex;i++)
        {
            if (this->borderItems->at(i)->getPartNo()->startsWith(*partNo))
            {
                ret = this->borderItems->at(i);
                this->lastFoundIndex=i;
                break;
            }
        }
    }
    return ret;
}

VegetationItem *ProductLine::findVegetationItemByPartNo(QString *partNo)
{
    VegetationItem * ret = NULL;
    if (this->lastFoundIndex>=this->vegetationItems->count())
        this->lastFoundIndex=0;
    for (int i = lastFoundIndex; i < this->vegetationItems->count();i++)
    {
        if (this->vegetationItems->at(i)->getPartNo()->startsWith(*partNo))
        {
            ret = this->vegetationItems->at(i);
            this->lastFoundIndex=i;
            break;
        }
    }
    if (ret==NULL)
    {
        for (int i = 0; i < lastFoundIndex;i++)
        {
            if (this->vegetationItems->at(i)->getPartNo()->startsWith(*partNo))
            {
                ret = this->vegetationItems->at(i);
                this->lastFoundIndex=i;
                break;
            }
        }
    }
    return ret;
}

int ProductLine::addItem(ModelItem* item)
{
    if (item==NULL)
        return 1;
    {
        for (int i = 0; i < this->items->count(); i++)
        {
            if (*this->items->at(i)->getPartNo()==*item->getPartNo())
                return 2;
        }
    }

    this->items->push_back(item);
    if (item->getRadius()>this->maxTrackRadius)
    {
        if (item->getType()==C1 || item->getType()==C2 || item->getType()==CB
                || item->getType()==J1 || item->getType()==J2 || item->getType()==J3 || item->getType()==HC
                )
        this->maxTrackRadius=item->getRadius();
    }

    if(item->getRadius()<this->minTrackRadius)
    {
        if (item->getType()==C1 || item->getType()==C2 || item->getType()==CB
                || item->getType()==J1 || item->getType()==J2 || item->getType()==J3 || item->getType()==HC
                )
        this->minTrackRadius=item->getRadius();
    }
    if (this->type)
    {
        if (item->getRadius()>this->maxStraightLength && item->getType()==S1)
            this->maxStraightLength=item->getRadius();
    }
    else
    {
        if (item->getSecondRadius()>this->maxStraightLength && (item->getType()==S1 || item->getType()==HE || item->getType()==HS || item->getType()==HC || item->getType()==SC))
            this->maxStraightLength=item->getSecondRadius();
    }
    return 0;
}

int ProductLine::addItem(BorderItem *item)
{
    if (item==NULL || this->type)
        return 1;
    this->borderItems->push_back(item);
    return 0;

}

int ProductLine::addItem(VegetationItem *item)
{
    if (item==NULL)
        return 1;
    this->vegetationItems->push_back(item);
    return 0;
}

qreal ProductLine::getMinRadius() const
{return this->minTrackRadius;}

qreal ProductLine::getMaxRadius() const
{return this->maxTrackRadius;}

qreal ProductLine::getMaxStraight() const
{return this->maxStraightLength;}

ModelFragment::ModelFragment(ModelItem *item)
{
    logFile << "    ModelFragment constructor" << endl;
    this->endPoints = new QList<QPointF*>();

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
    this->fragmentID=-1024;
}

ModelFragment::~ModelFragment()
{
    logFile << "    ~ModelFragment, id " << this->fragmentID << " " << this << endl;
    logFile << "        fragment contains " << this->fragmentItems->count() << " parts" << endl;
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

void ModelFragment::setID(int id)
{
    logFile << "        Set fragment id from " << this->fragmentID << " to " << id << endl;
    this->fragmentID=id;

}

int ModelFragment::getID() const
{return this->fragmentID;}

ModelItem *ModelFragment::findEndPointItem(QPointF *approxPos)
{
    ModelItem * mi = NULL;
    for (int i = 0; i < this->endPoints->count(); i++)
    {
        if (pointsAreCloseEnough(this->endPoints->at(i),approxPos,this->getProductLines()->first()->getScaleEnum()/4.0))
        {
            *approxPos=*this->endPoints->at(i);
            mi=this->endPointsItems->at(i);
            break;
        }
    }
    return mi;
}

QList <ModelItem*> * ModelFragment::getFragmentItems() const
{return this->fragmentItems;}

QList<ProductLine*> * ModelFragment::getProductLines() const
{return this->lines;}

QList<QPointF *> * ModelFragment::getEndPoints() const
{return this->endPoints;}

QList<QGraphicsEllipseItem *> *ModelFragment::getEndPointsGraphics() const
{return this->endPointsGraphics;}

QList<qreal> * ModelFragment::getEndPointsAngles() const
{return this->endPointsAngles;}

QList<ModelItem *> *ModelFragment::getEndPointsItems() const
{return this->endPointsItems;}

int ModelFragment::addFragmentItem(ModelItem* item,QPointF * point, int insertionIndex)
{
    QTime timer;
    timer.start();

    if (item == NULL)
        return 1;
    logFile << "    Adding item " << item->getPartNo()->toStdString() << " at point [" << point->x() << "," << point->y() << "] with index " << insertionIndex << " and address " << item << " into fragment with id " << this->fragmentID << endl;
    logFile << "        EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;

    /*
     *How it works:
     *-item is inserted at point's value
     *-item's endPoint[0] is used to connect with the rest of the fragment
     *-item's endPoint[1] is used as a new active point
     *-all remaining endPoints are added as fragments' endpoints
    */


    item->setParentFragment(this);
    this->fragmentItems->push_back(item);

    if (!this->lines->contains(item->getProdLine()))
        this->lines->push_back(item->getProdLine());

    //find fragment's endPoint which equals to "point" parameter and remove it - it can't be used as endPoint anymore
    qreal currentFragmentRotation = 0;
    ModelItem * endPointItem = NULL;
    QList<QPointF*>::Iterator iter = this->endPoints->begin();
    int i = 0;

    QRectF insertionArea(QPointF(point->x()-TOLERANCE_HALF,point->y()-TOLERANCE_HALF),QSizeF(TOLERANCE_HALF*2,TOLERANCE_HALF*2));

    while(iter!=this->endPoints->end())
    {
        if (insertionArea.contains(**iter))
        {
            //get frag. rotation and then remove it from the list - won't be used
            currentFragmentRotation = this->endPointsAngles->at(i);
            endPointItem = this->endPointsItems->at(i);
            this->removeEndPoint(*iter);
            break;
        }
        iter++;
        i++;
    }

    //get start-point-rotation of inserted item and subtract it from currentFragmentRotation
    //^^-result==0 -> no rotation is needed
    if ((currentFragmentRotation-item->getTurnAngle(insertionIndex)!=0))// && endPointItem!=NULL)
    {
        //rotate 2D model and modify item's attributes
        item->rotate(currentFragmentRotation-(item->getTurnAngle(insertionIndex)));
        item->updateEndPointsHeightGraphics();
    }

    QPointF pointOfInsertion(*point);
    QPointF point2 = *point;

    //by default item's endPoints are in local item's coordinates (center of item=[0,0] => without -(i.EP(0)) "point" parameter would be in the [0,0] of "item" parameter
    point2-=*item->getEndPoint(insertionIndex);
    qreal dx = point2.x();
    qreal dy = point2.y();

    //this fixes the strange behaviour of T1 2D Model
    if (item->getType()==T1)
    {
        item->rotate(180);
        item->moveBy(-2*item->getEndPoint(insertionIndex)->x(),-2*item->getEndPoint(insertionIndex)->y());

    }

    item->moveBy(dx,dy);

    //set height profile of inserted item to h.p. of endPoint at which the item is being inserted
    int dz = endPointItem==NULL ? 0 : endPointItem->getHeightProfileAt(&pointOfInsertion);
    if (dz!=-12345)
    {
        if (item->getSlotTrackInfo()!=NULL && endPointItem!=NULL && item->getSlotTrackInfo()->getNumberOfLanes()>1 && endPointItem->getSlotTrackInfo()->getNumberOfLanes()>1  && item->getType()!=CB)
        {
            qreal h1 = endPointItem->getHeightProfileAt(endPointItem->getEndPoint(1));
            qreal h3 = endPointItem->getHeightProfileAt(endPointItem->getEndPoint(3));

            qreal angle = 180/PI*asin(abs(h1-h3)/endPointItem->getSlotTrackInfo()->getLanesGauge());
            item->setLateralAngle(angle);

            for (unsigned int k = 0; k < 2*item->getSlotTrackInfo()->getNumberOfLanes(); k+=2)
            {
                qreal dzLane = endPointItem->getHeightProfileAt(item->getEndPoint(k))-item->getHeightProfileAt(item->getEndPoint(k));
                item->adjustHeightProfile(dzLane,item->getEndPoint(k));
                item->adjustHeightProfile(dzLane,item->getEndPoint(k+1));
            }

        }
        else if (item->getType()==CB)
        {
            if (endPointItem->getType()==CB)
            {
                dz = endPointItem->getHeightProfileAt(&pointOfInsertion)-item->getHeightProfileAt(&pointOfInsertion);

            }
            if (dz!=-12345)
            {
                for (unsigned int k = 0; k < 2*item->getSlotTrackInfo()->getNumberOfLanes(); k++)
                {
                    item->adjustHeightProfile(dz,item->getEndPoint(k));
                }

                for (unsigned int k = 0; k < 2*item->getSlotTrackInfo()->getNumberOfLanes(); k+=2)
                {
                    endPointItem->adjustHeightProfile(item->getHeightProfileAt(item->getEndPoint(k))-endPointItem->getHeightProfileAt(item->getEndPoint(k)),item->getEndPoint(k));
                }
            }
        }
        else
        {
            item->adjustHeightProfile(dz-item->getHeightProfileAt(&pointOfInsertion),&pointOfInsertion);
            if (item->getType()==C1 || item->getType()==S1 || item->getType()==C2 || item->getType()==CB)
                insertionIndex%2==0 ? item->adjustHeightProfile(dz-item->getHeightProfileAt(item->getEndPoint(insertionIndex+1)),item->getEndPoint(insertionIndex+1))
                                    : item->adjustHeightProfile(dz-item->getHeightProfileAt(item->getEndPoint(insertionIndex-1)),item->getEndPoint(insertionIndex-1));
        }
    }

    //inserted item has been moved so it can finally be checked (because its points are now in global coords) whether more frag.endPoints can be removed
    QList<int> doNotAddThese;
    if (item->getSlotTrackInfo()!=NULL)
    {
        iter=this->endPoints->begin();
        QList<ModelItem*>::Iterator ePIIter = this->endPointsItems->begin();
        while (iter!=this->endPoints->end())
        {
            int j = 0;

            while (item->getEndPoint(j)!=NULL)
            {
                if (j==insertionIndex && item->getType()==HC)
                {
                    j++;
                    if (item->getEndPoint(j)==NULL)
                        break;
                }
                if (pointsAreCloseEnough(item->getEndPoint(j),*iter,item->getProdLine()->getScaleEnum()/4.0) && item->getHeightProfileAt(item->getEndPoint(j))==(*ePIIter)->getHeightProfileAt(item->getEndPoint(j)))
                {
                    this->removeEndPoint(*iter);


                    iter=this->endPoints->begin();
                    ePIIter=this->endPointsItems->begin();
                    doNotAddThese.push_back(j);
                    iter--;
                    ePIIter--;
                    break;

                }
                j++;
            }
            iter++;
            ePIIter++;
        }
    }
    else
    {
        iter=this->endPoints->begin();
        QList<ModelItem*>::Iterator ePIIter = this->endPointsItems->begin();
        QList<qreal>::Iterator ePAIter = this->endPointsAngles->begin();

        while (iter!=this->endPoints->end())
        {
            for (int j = 0; item->getEndPoint(j)!=NULL; j++)
            {
                qreal dAlpha = abs(item->getTurnAngle(j)-*ePAIter);
                while (dAlpha >=360)
                    dAlpha-=360;
                while (dAlpha < 0)
                    dAlpha+=360;

                if (pointsAreCloseEnough(item->getEndPoint(j),*iter,item->getProdLine()->getScaleEnum()/4.0)
                        && item->getHeightProfileAt(item->getEndPoint(j))==(*ePIIter)->getHeightProfileAt(item->getEndPoint(j))
                        && (dAlpha<=10 || (dAlpha>=170 && dAlpha<=190) || dAlpha >=350)
                        )
                {
                    this->removeEndPoint(*iter);

                    iter=this->endPoints->begin();
                    ePIIter=this->endPointsItems->begin();
                    ePAIter = this->endPointsAngles->begin();
                    doNotAddThese.push_back(j);
                    iter--;
                    ePIIter--;
                    ePAIter--;
                    break;
                }
            }
            iter++;
            ePIIter++;
            ePAIter++;
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

    QPointF * newPt = NULL;
    if (item->getEndPoint()!=NULL)
    {
        insertionIndex%2==0 ? newPt = new QPointF(item->getEndPoint(insertionIndex+1)->x(),item->getEndPoint(insertionIndex+1)->y()) : newPt = new QPointF(item->getEndPoint(insertionIndex-1)->x(),item->getEndPoint(insertionIndex-1)->y());
    }

    if (newPt==NULL)
    {
        newPt = new QPointF(0,0);

        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
        this->endPointsAngles->removeOne(newFragmentRotation);
    }
    else
    {
        //-1 is missing
        if (!doNotAddThese.contains(insertionIndex+1))
        {
            app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        }
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
                }
            }
        }

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

        if (item->getNeighbour(insertionIndex)==NULL)
            doNotAddThese.removeOne(insertionIndex);

        //add all remaining endPoint's and modify neighbours
        ItemType t = item->getType();
        if ( t==T1)
        {
            int index = 2;
            while (item->getEndPoint(index)!=NULL)
            {
                if (doNotAddThese.contains(index))
                {
                    ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(index),item);

                    if (endPointItemOdd!=item && endPointItemOdd!=NULL)
                    {
                        if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItemOdd->get2DModelNoText()->isSelected()))
                        {}
                        else if (endPointItemOdd->getNeighbour(item->getEndPoint(index))==NULL && endPointItemOdd->getHeightProfileAt(item->getEndPoint(index))==item->getHeightProfileAt(item->getEndPoint(index)))
                        {
                            item->setNeighbour(endPointItemOdd,item->getEndPoint(index));
                            if (endPointItemOdd!=NULL)// && endPointItemOdd!=item)
                                endPointItemOdd->setNeighbour(item,item->getEndPoint(index));
                        }
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
        else if (((t==J1 || t==J2) && item->getSlotTrackInfo()==NULL) || (t==J3) || t==J4 || (t==X1 && item->getSlotTrackInfo()==NULL))
        {
            int index = 1;
            doNotAddThese.removeOne(index);
            if (item->getNeighbour(index)==NULL)
            {
                ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(index),item);
                if (endPointItemOdd!=item && endPointItemOdd!=NULL)
                {
                    if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItemOdd->get2DModelNoText()->isSelected()))
                    {}
                    else if (endPointItemOdd->getNeighbour(item->getEndPoint(index))==NULL && endPointItemOdd->getHeightProfileAt(item->getEndPoint(index))==item->getHeightProfileAt(item->getEndPoint(index)))
                    {
                        item->setNeighbour(endPointItemOdd,item->getEndPoint(index));
                        if (endPointItemOdd!=NULL)
                            endPointItemOdd->setNeighbour(item,item->getEndPoint(index));
                    }
                }
            }
            index = 2;
            if (!doNotAddThese.contains(index))
            {
                if (t==J1 || t==J2)
                {
                    this->addEndPoint(item->getEndPoint(index));
                    this->endPointsAngles->push_back(item->getTurnAngle(index));
                    this->endPointsItems->push_back(item);
                }
                else if (t==J4 || t==J5 || t==X1)
                {
                    if (item->getNeighbour(index)==NULL && !item->leftRightDifference180(index,index+1))
                        item->setEndPointAngle(index,180+item->getTurnAngle(index));
                    this->addEndPoint(item->getEndPoint(index));
                    this->endPointsAngles->push_back(item->getTurnAngle(index));
                    this->endPointsItems->push_back(item);
                }
                else if (t==J3)
                {
                    this->addEndPoint(item->getEndPoint(index));
                    this->endPointsAngles->push_back(item->getTurnAngle(index));
                    this->endPointsItems->push_back(item);
                }
            }
            else if (index!=insertionIndex)
            {
                index = 2;
                while (index<4)
                {
                    doNotAddThese.removeOne(index);
                    if (item->getNeighbour(index)==NULL)
                    {
                        ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(index),item);
                        if (endPointItemOdd!=item && endPointItemOdd!=NULL)
                        {
                            if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItemOdd->get2DModelNoText()->isSelected()))
                            {}
                            else if (endPointItemOdd->getNeighbour(item->getEndPoint(index))==NULL && endPointItemOdd->getHeightProfileAt(item->getEndPoint(index))==item->getHeightProfileAt(item->getEndPoint(index)))
                            {
                                item->setNeighbour(endPointItemOdd,item->getEndPoint(index));
                                if (endPointItemOdd!=NULL)// && endPointItemOdd!=item)
                                    endPointItemOdd->setNeighbour(item,item->getEndPoint(index));
                            }
                        }
                    }
                    index++;
                }


            }
            index = 3;
            if (!doNotAddThese.contains(index))
            {
                if (t==J3 || t==J4 || t==J5 || t==X1)
                {
                    this->addEndPoint(item->getEndPoint(index));
                    this->endPointsAngles->push_back(item->getTurnAngle(index));
                    this->endPointsItems->push_back(item);
                }
            }
            else
            {
                if (t==J3 || t==J4 || t==J5 || t==X1)
                {
                    while (index<4)
                    {
                        doNotAddThese.removeOne(index);
                        if (item->getNeighbour(index)==NULL)
                        {
                            ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(index),item);
                            if (endPointItemOdd!=item && endPointItemOdd!=NULL)
                            {
                                if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItemOdd->get2DModelNoText()->isSelected()))
                                {}
                                else if (endPointItemOdd->getNeighbour(item->getEndPoint(index))==NULL && endPointItemOdd->getHeightProfileAt(item->getEndPoint(index))==item->getHeightProfileAt(item->getEndPoint(index)))
                                {
                                    item->setNeighbour(endPointItemOdd,item->getEndPoint(index));
                                    if (endPointItemOdd!=NULL)// && endPointItemOdd!=item)
                                        endPointItemOdd->setNeighbour(item,item->getEndPoint(index));
                                }
                            }
                        }
                        index++;
                    }
                }
            }
        }
        else //slot track, rail C1 and S1
        {
            //doesnt work for J1, J2, J3, J4, J5, X1 and T1 items because of changing even angles
            int index = 0;
            while (item->getEndPoint(index)!=NULL)
            {
                if (true)
                {
                    if (!doNotAddThese.contains(index))
                    {
                        if (item->getType()==J2 || item->getType()==J1)
                        {
                            if (index%2==0 && item->getEndPoint(index+1)!=NULL && item->getNeighbour(index)==NULL && !item->leftRightDifference180(index,index+1))
                                item->setEndPointAngle(index,180+item->getTurnAngle(index));
                        }
                        else
                        {
                            if (index%2==0 && item->getNeighbour(index)==NULL && !item->leftRightDifference180(index,index+1))
                                item->setEndPointAngle(index,180+item->getTurnAngle(index));
                        }

                        this->addEndPoint(item->getEndPoint(index));
                        this->endPointsAngles->push_back(item->getTurnAngle(index));
                        this->endPointsItems->push_back(item);
                    }
                    else if (index!=insertionIndex)
                    {
                        doNotAddThese.removeOne(index);
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
                                    endPointItem->setNeighbour(item,item->getEndPoint(index));
                                }
                            }
                            else if (item->getEndPoint(index+1)==NULL)
                            {
                                ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(index),item);

                                if (endPointItemOdd!=item && endPointItemOdd!=NULL)
                                {
                                    if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItemOdd->get2DModelNoText()->isSelected()))
                                    {}
                                    else if (endPointItemOdd->getNeighbour(item->getEndPoint(index))==NULL && endPointItemOdd->getHeightProfileAt(item->getEndPoint(index))==item->getHeightProfileAt(item->getEndPoint(index)))
                                    {
                                        item->setNeighbour(endPointItemOdd,item->getEndPoint(index));
                                        if (endPointItemOdd!=NULL)// && endPointItemOdd!=item)
                                            endPointItemOdd->setNeighbour(item,item->getEndPoint(index));
                                    }
                                }
                            }
                        }
                        else if (item->getNeighbour(index)==NULL)
                        {

                            ModelItem * endPointItemOdd = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(item->getEndPoint(index),item);

                            if (endPointItemOdd!=item && endPointItemOdd!=NULL)
                            {
                                if ((app->getWindow()->getWorkspaceWidget()->getDeletePress() && endPointItemOdd->get2DModelNoText()->isSelected()))
                                {}
                                else if (endPointItemOdd->getNeighbour(item->getEndPoint(index))==NULL && endPointItemOdd->getHeightProfileAt(item->getEndPoint(index))==item->getHeightProfileAt(item->getEndPoint(index)))
                                {
                                    item->setNeighbour(endPointItemOdd,item->getEndPoint(index));
                                    if (endPointItemOdd!=NULL)// && endPointItemOdd!=item)
                                        endPointItemOdd->setNeighbour(item,item->getEndPoint(index));
                                }
                            }
                        }
                    }
                }
                index++;
            }
        }
    }


    //update graphic representation of fragment
    app->getWindow()->getWorkspaceWidget()->updateFragment(this);

    if (this->endPoints->empty())
    {
        newPt = new QPointF(0,0);

        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);

    }

    logFile << "        Item has endpoints:" << endl;
    int xyz = 0;
    while (item->getEndPoint(xyz)!=NULL)
    {
        logFile << "            [" << xyz << "]: [" << item->getEndPoint(xyz)->x() << ", " << item->getEndPoint(xyz)->y() << "] at " << item->getEndPoint(xyz) << " and with rotation of " << item->getTurnAngle(xyz) << "° and Neighbour is at " << item->getNeighbour(xyz) << endl;
        xyz++;
    }

    logFile << "        EP: " << this->endPoints->count() << " EPA: " << this->endPointsAngles->count() << endl;

    return 0;
}

int ModelFragment::deleteFragmentItem(ModelItem *item, QList<int> *idList)
{
    logFile << "    Deleting item " << item->getPartNo()->toStdString() << " at point [" << item->getEndPoint(0)->x() << "," << item->getEndPoint(0)->y() << "] and address " << item << " from fragment with id " << this->fragmentID << endl;

    item->setDeleteFlag();

    //application should distinguish these special cases:
    //-there's just one item in the fragment - case A)
    //-there are more items in the fragment but "item" belongs to list of endPointsItems && has just 1 not-NULL neighbour => fragment will stay continous - case B)
    //-there are more items in the fragment and deletion of "item" will make fragment non-continous - case C)

    //case A)
    //if count of fragment items == 1 && item==fragment item
    if (this->fragmentItems->count()==1 && item==this->fragmentItems->first())
    {
        QPointF * newActive = new QPointF();
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newActive);        
        //destruct the fragment in workspace widget
        return -1;

    }

    //case B) - run this code only if item has exactly 1 not-NULL neighbour (caution: slot track)
    //iterate through endPointsItems and try to find "item" if it wasn't successful, go to case C) else:
    //-removed because of performance issues

    //case C)
    //for each neighbour N of item:  !!check if N.parentFragment==this - true -> ok, false -> NOP

    //dont start from neighbours
    //start from fragmentItems[0] - if [0]!=item, create new Fragment and start recursion from [0]
    //iterate through the list

    QPointF * newActive = new QPointF();
    {
        for (int k = 0; k < this->getEndPoints()->count();k++)
        {
            if (*this->getEndPoints()->at(k)==*app->getWindow()->getWorkspaceWidget()->getActiveEndPoint())
            {
                if (this->endPointsItems->at(k)!=item)
                    *newActive = *this->getEndPoints()->at(k);
                else
                {
                    int m = 0;
                    while (item->getEndPoint(m)!=NULL)
                    {
                        if (item->getNeighbour(m)!=NULL)
                        {
                            *newActive = *item->getEndPoint(m);
                            k=this->getEndPoints()->count()+1;
                            break;
                        }
                        m++;
                    }
                }
            }
        }
    }

    //separate neighbours
    int i = 0;
    while (item->getEndPoint(i)!=NULL)
    {
        ModelItem * neighbour = item->getNeighbour(i);
        if (neighbour!=NULL)
        {
            neighbour->setNeighbour(NULL,item->getEndPoint(i));

            if (neighbour->getSlotTrackInfo()!=NULL)
            {
                //if neighbour's index is even, add 180 deg.
                int k = 0;
                while (neighbour->getEndPoint(k)!=NULL)
                {
                    if (pointsAreCloseEnough(neighbour->getEndPoint(k),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0) && k%2==0)
                    {
                        if (!neighbour->leftRightDifference180(k,k+1))
                            neighbour->setEndPointAngle(k,180+neighbour->getTurnAngle(k));

                        break;
                    }
                    k++;
                }
            }
            else
            {
                int k = 0;
                ItemType nT = neighbour->getType();
                if (nT == C1 || nT == S1 || nT == E1 || (nT >= T2 && nT <= T10))
                {
                    while (neighbour->getEndPoint(k)!=NULL)
                    {
                        if (pointsAreCloseEnough(neighbour->getEndPoint(k),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0) && k%2==0)
                        {
                            if (!neighbour->leftRightDifference180(k,k+1))
                                neighbour->setEndPointAngle(k,180+neighbour->getTurnAngle(k));
                            break;
                        }
                        k++;
                    }
                }
                else if (nT==J1)
                {
                    if (pointsAreCloseEnough(neighbour->getEndPoint(0),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0) && neighbour->getRadius()>0)
                    {
                        if (!neighbour->leftRightDifference180(0,1))
                            neighbour->setEndPointAngle(0,180+neighbour->getTurnAngle(0));
                    }
                    else if (pointsAreCloseEnough(neighbour->getEndPoint(0),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0) && neighbour->getRadius()<0)
                    {
                        if (!neighbour->leftRightDifference180(0,2))
                            neighbour->setEndPointAngle(0,180+neighbour->getTurnAngle(0));
                    }
                    else if (pointsAreCloseEnough(neighbour->getEndPoint(1),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0) && neighbour->getRadius()<0)
                    {
                        if (!neighbour->leftRightDifference180(1,2))
                            neighbour->setEndPointAngle(1,180+neighbour->getTurnAngle(1));
                    }


                }
                else if (nT==J2)
                {
                    if (neighbour->getRadius()>0)
                    {
                        if (pointsAreCloseEnough(neighbour->getEndPoint(0),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                        {
                            if (!neighbour->leftRightDifference180(0,2))
                                neighbour->setEndPointAngle(0,180+neighbour->getTurnAngle(0));
                        }
                        if (pointsAreCloseEnough(neighbour->getEndPoint(1),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                        {
                            if (!neighbour->leftRightDifference180(1,2))
                                neighbour->setEndPointAngle(1,180+neighbour->getTurnAngle(1));
                        }
                    }
                    else
                    {
                        if (pointsAreCloseEnough(neighbour->getEndPoint(0),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                        {
                            if (!neighbour->leftRightDifference180(0,1))
                                neighbour->setEndPointAngle(0,180+neighbour->getTurnAngle(0));
                        }
                    }
                }
                else if (nT==J3)
                {
                    if (neighbour->getRadius()>0)
                    {
                        if (pointsAreCloseEnough(neighbour->getEndPoint(0),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                        {
                            if (!neighbour->leftRightDifference180(0,1))
                                neighbour->setEndPointAngle(0,180+neighbour->getTurnAngle(0));
                        }
                    }
                    else
                    {
                        k=1;
                        while (k<4)
                        {
                            if (pointsAreCloseEnough(neighbour->getEndPoint(k),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                            {
                                if (!neighbour->leftRightDifference180(0,k))
                                    neighbour->setEndPointAngle(k,180+neighbour->getTurnAngle(k));
                                break;
                            }
                            k++;
                        }
                    }
                }
                else if (nT==J4 || nT==X1 || nT==J5)
                {
                    if (pointsAreCloseEnough(neighbour->getEndPoint(0),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                    {
                        if (!neighbour->leftRightDifference180(0,1))
                            neighbour->setEndPointAngle(0,180+neighbour->getTurnAngle(0));
                    }
                    else if (pointsAreCloseEnough(neighbour->getEndPoint(2),item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                    {
                        if (!neighbour->leftRightDifference180(2,3))
                            neighbour->setEndPointAngle(2,180+neighbour->getTurnAngle(2));
                    }
                }
                else if (nT==T1)
                {
                    //do nothing
                }
            }
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
    bool first = true;

    int idIndex = 1;

    for (int j = 0; j < this->fragmentItems->count() ;j++)
    {
        if (this->fragmentItems->at(j)!=item && !this->fragmentItems->at(j)->getDeleteFlag())
        {
            if (this->fragmentItems->at(j)->getParentFragment()==this)
            {
                //do something with left side angles if it is needed
                if (this->fragmentItems->at(j)->getSlotTrackInfo()!=NULL)
                {
                    int y = 0;
                    while (this->fragmentItems->at(j)->getEndPoint(y)!=NULL)
                    {
                        //original version
                        if (!this->fragmentItems->at(j)->leftRightDifference180(y,y+1))
                        //modified version because of connectFragments()
                            this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnAngle(y));
                        y+=2;
                    }
                }
                else
                {
                    int y = 0;
                    ItemType t = this->fragmentItems->at(j)->getType();
                    if (t==E1)
                    {}
                    else if (t==C1 || t==S1 || t==CB)
                    {
                        if (!this->fragmentItems->at(j)->leftRightDifference180(y,y+1))
                            this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnAngle(y));
                    }

                    else if (t==J1)
                    {
                        if (this->fragmentItems->at(j)->getRadius()>0)
                        {
                            if (!this->fragmentItems->at(j)->leftRightDifference180(0,1))
                                this->fragmentItems->at(j)->setEndPointAngle(0,180+this->fragmentItems->at(j)->getTurnAngle(0));

                            if (!this->fragmentItems->at(j)->leftRightDifference180(0,2))
                                this->fragmentItems->at(j)->setEndPointAngle(0,180+this->fragmentItems->at(j)->getTurnAngle(0));

                        }
                        else
                        {
                            if (!this->fragmentItems->at(j)->leftRightDifference180(0,2))
                                this->fragmentItems->at(j)->setEndPointAngle(0,180+this->fragmentItems->at(j)->getTurnAngle(0));

                            if (!this->fragmentItems->at(j)->leftRightDifference180(1,2))
                                this->fragmentItems->at(j)->setEndPointAngle(1,180+this->fragmentItems->at(j)->getTurnAngle(1));
                        }
                    }
                    else if (t==J2)
                    {
                        if (this->fragmentItems->at(j)->getRadius()>0)
                        {
                            if (!this->fragmentItems->at(j)->leftRightDifference180(0,2))
                                this->fragmentItems->at(j)->setEndPointAngle(0,180+this->fragmentItems->at(j)->getTurnAngle(0));

                            if (!this->fragmentItems->at(j)->leftRightDifference180(1,2))
                                this->fragmentItems->at(j)->setEndPointAngle(1,180+this->fragmentItems->at(j)->getTurnAngle(1));
                        }
                        else
                        {
                            if (!this->fragmentItems->at(j)->leftRightDifference180(0,1))
                                this->fragmentItems->at(j)->setEndPointAngle(0,180+this->fragmentItems->at(j)->getTurnAngle(0));
                        }
                    }
                    else if (t==J3)
                    {
                        if (!this->fragmentItems->at(j)->leftRightDifference180(y,y+1))
                            this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnAngle(y));
                    }
                    else if (t==J4 || t==J5 || t==X1)
                    {
                        if (!this->fragmentItems->at(j)->leftRightDifference180(y,y+1))
                            this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnAngle(y));
                        y=2;
                        if (!this->fragmentItems->at(j)->leftRightDifference180(y,y+1))
                            this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnAngle(y));
                    }
                    else if (t>=T2 && t<=T10)
                    {
                        y=0;
                        while (this->fragmentItems->at(j)->getEndPoint(y)!=NULL)
                        {
                            if (!this->fragmentItems->at(j)->leftRightDifference180(y,y+1))
                                this->fragmentItems->at(j)->setEndPointAngle(y,180+this->fragmentItems->at(j)->getTurnAngle(y));
                            y+=2;
                        }
                    }
                }

                ModelFragment * newF = new ModelFragment(this->fragmentItems->at(j));

                logFile << "    New fragment has been created from item " << this->fragmentItems->at(j) << " at j==[" << j << "] and address " << newF << endl;
                if (first)
                {
                    app->getWindow()->getWorkspaceWidget()->addFragment(newF,item->getParentFragment()->getID());
                    first = false;
                }
                else
                {
                    if (idList!=NULL && idList->count()>1)
                    {
                        app->getWindow()->getWorkspaceWidget()->addFragment(newF,(*idList)[idIndex]);
                        idIndex++;
                    }
                    else
                    {
                        app->getWindow()->getWorkspaceWidget()->addFragment(newF);

                    }
                }

                rebuildFragment(this->fragmentItems->at(j),newF);
                if (newF->getFragmentItems()->count()==1)
                {
                    int k = 0;
                    while (newF->getFragmentItems()->first()->getEndPoint(k)!=NULL)
                    {
                        newF->getFragmentItems()->first()->setNeighbour(NULL,newF->getFragmentItems()->first()->getEndPoint(k));
                        k++;
                    }
                }

                if (*newActive==QPointF(0,0))
                {
                    if (newF->getEndPoints()->count()>1)
                        *newActive = *newF->getEndPoints()->at(1);
                    else
                        *newActive = *newF->getEndPoints()->at(0);
                }

                 app->getWindow()->getWorkspaceWidget()->setActiveFragment(newF);
            }
        }
    }

    app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newActive);

    QList<ModelItem*> list;
    list.append(*this->fragmentItems);
    this->fragmentItems->clear();

    for (int i = 0; i < list.count(); i++)
    {
        if (list.at(i)->getDeleteFlag())
            this->fragmentItems->push_back(list.at(i));
    }

    //delete item
    //delete remaining parts of old fragment (=this) !!!caution: endPoints contain pointers to items which now belong to other fragments
    return -1;
}


int ModelFragment::removeEndPoint (QPointF * &pt)
{
    if (pt == NULL)
        return 1;

    int index = this->endPoints->indexOf(pt);

    bool success = false;
    if (index!=-1)
        success = true;

    if (!success)
        return 2;

    QGraphicsItem * item = this->endPointsGraphics->at(index);
    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(item);
    delete item;
    this->endPointsGraphics->removeAt(index);
    this->endPointsAngles->removeAt(index);
    this->endPointsItems->removeAt(index);

    //only remove pointer, because ITEM owns the point, fragment owns just another copy of pointer
    this->endPoints->removeAt(index);

    return 0;


}

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
    else
    {
        this->infoDialog->deleteLater();
        result = this->initInfoDialog();
    }

    if (result)
        return result;

    this->infoDialog->show();
    return result;
}

int ModelFragment::initInfoDialog()
{
    this->infoDialog = new QDialog(app->getWindow());
    QFormLayout * layout = new QFormLayout(this->infoDialog);

    bool english = true;

    if (app->getUserPreferences()->getLocale()->startsWith("CS"))
        english = false;

    if (english)
        this->infoDialog->setWindowTitle("Track section info");
    else
        this->infoDialog->setWindowTitle("Informace o úseku trati");

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
    QListWidget * listWidgetM = new QListWidget(this->infoDialog);

    //int = count, QString = partNo (prodLine)
    QMap<QString,int> itemsMap;
    QMap<QString,int> prodLineMap;

    //fill both maps
    for (int i = 0; i < this->fragmentItems->count();i++)
    {
        QString str;
        str.append(*this->fragmentItems->at(i)->getPartNo());
        str.append(" (");
        str.append(*this->fragmentItems->at(i)->getProdLine()->getName());
        str.append(")");

        itemsMap[str]++;
        prodLineMap[*this->fragmentItems->at(i)->getProdLine()->getName()]++;

    }

    //print both maps
    QMap<QString,int>::Iterator it = itemsMap.begin();
    for (int i = 0; i < itemsMap.count(); i++, it++)
    {
        QString val = QString::number(it.value());
        val.append("x ");
        val.append(it.key());
        listWidgetI->addItem(val);
    }

    it = prodLineMap.begin();
    for (int i = 0; i < prodLineMap.count(); i++, it++)
    {
        QString val = QString::number(it.value());
        val.append("x ");
        val.append(it.key());
        listWidgetM->addItem(val);
    }

    /*get approximate size of the fragment - too approximated
    QPolygonF poly;
    for (int i = 0; i < this->fragmentItems->count(); i++)
    {
        int j = 0;
        while (this->fragmentItems->at(i)->getEndPoint(j)!=NULL)
        {
            poly << *this->fragmentItems->at(i)->getEndPoint(j);
            j++;
        }
    }*/

    //how to get exact dimensions of the fragment
    //create QGraphicsScene and add (copies?) of 2D models
    //sceneSize should equal to the dimensions
    QRectF poly;
    for (int i = 0; i < this->fragmentItems->count(); i++)
    {
        poly = poly.united(this->fragmentItems->at(i)->get2DModelNoText()->sceneBoundingRect());
    }

    QString sizeStr;
    if (this->lines->first()->getType())
        sizeStr.append(QString::number((int)poly.width()+this->lines->first()->getScaleEnum()/4.0));
    else
        sizeStr.append(QString::number((int)poly.width()));
    sizeStr.append("x");
    if (this->lines->first()->getType())
        sizeStr.append(QString::number((int)poly.height()+this->lines->first()->getScaleEnum()/4.0));
    else
        sizeStr.append(QString::number((int)poly.height()));
    sizeStr.append("mm");
    QLabel * label2 = new QLabel(sizeStr);
    QPushButton * close = new QPushButton("Close");
    QObject::connect(close,SIGNAL(clicked()),this->infoDialog,SLOT(reject()));

    if (english)
    {
        layout->insertRow(0,"Parts count:",label);
        layout->insertRow(1,"Parts used:",listWidgetI);
        layout->insertRow(2,"Parts' manufacturer(s)",listWidgetM);
        layout->insertRow(3,"Total size (approx.): ", label2);
        layout->insertRow(4,"",close);
    }
    else
    {
        close->setText("Zavřít");
        layout->insertRow(0,"Počet dílů:",label);
        layout->insertRow(1,"Použité díly:",listWidgetI);
        layout->insertRow(2,"Výrobci dílů",listWidgetM);
        layout->insertRow(3,"Celková velikost (přibližná): ", label2);
        layout->insertRow(4,"",close);
    }

    this->infoDialog->setLayout(layout);
    return 0;
}

void ModelFragment::moveBy(qreal dx, qreal dy)
{
    if (dx==0 && dy==0)
        return;
    logFile << "    Moving fragment " << app->getWindow()->getWorkspaceWidget()->getFragmentIndex(this) << " by [" << dx << "," << dy << "]" << endl;
    QString str = (QString("move fragment %1 %2 %3 ").arg(QString::number(this->fragmentID),QString::number(dx),QString::number(dy)));

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
    QString negStr = (QString("move fragment %1 %2 %3").arg(QString::number(this->fragmentID),QString::number(-dx),QString::number(-dy)));
    app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
}

void ModelFragment::rotate(qreal angle, QPointF * center)
{
    if (angle!=0)
        logFile << "    Rotating fragment "  << app->getWindow()->getWorkspaceWidget()->getFragmentIndex(this) << " by " << angle << endl;

    QString str = (QString("rotate fragment %1 %2 %3 %4 ").arg(QString::number(this->fragmentID),QString::number(center->x()),QString::number(center->y()),QString::number(angle)));

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

    QString negStr = (QString("rotate fragment %1 %2 %3 %4 ").arg(QString::number(this->fragmentID),QString::number(center->x()),QString::number(center->y()),QString::number(-angle)));;
    app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);
}

void ModelFragment::updateEndPointsGraphics()
{
    for (int i = 0; i < this->endPoints->count(); i++)
        this->endPointsGraphics->at(i)->setPos(*this->endPoints->at(i));
}

int ModelFragment::addEndPoint(QPointF* pt, bool additionalInfo, qreal rotation, ModelItem * endPointItem)
{
    if (pt==NULL)
        return 1;

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

SlotTrackInfo::SlotTrackInfo(ModelItem * item,unsigned int numberOfLanes,qreal lanesGauge,qreal lanesGaugeEnd,qreal fstLaneDist)
{
    this->parentItem=item;
    this->fstLaneDist=fstLaneDist;
    this->lanesGauge=lanesGauge;
    this->lanesGaugeEnd=lanesGaugeEnd;
    this->numberOfLanes=numberOfLanes;
    this->borderEndPointsGraphics=new QList<QGraphicsEllipseItem*>();
    this->borderEndPoints=new QList<QPointF*>();
    this->borders=new QList<BorderItem*>();

    qreal dAlpha = (item->getTurnAngle(1))-(item->getTurnAngle(0));
    if (item->leftRightDifference180(0,1))
        dAlpha-=180;
    while (dAlpha<0)
        dAlpha=abs(dAlpha);
    while (dAlpha>=360)
        dAlpha-=360;
    int n = dAlpha/22.5;
    qreal angle = -dAlpha/2+22.5/2;
    for (int i = 0; i < n; i++)
    {
        QPointF * pt = new QPointF(0,-this->parentItem->getRadius());
        rotatePoint(pt,-angle);
        pt->setY(pt->y()+this->parentItem->getRadius());

        (*this->borders) << NULL;
        (*this->borderEndPoints) << pt;

        if (this->parentItem->getParentWidget()!=NULL)
        {
            QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(-5,-5,10,10);
            ellipse->setPos(*pt);
            QBrush b = ellipse->brush();
            b.setColor(Qt::red);
            b.setStyle(Qt::SolidPattern);
            ellipse->setBrush(b);
            ellipse->setZValue(-5000);
            (*this->borderEndPointsGraphics) << ellipse;
            app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(ellipse);
        }
        angle+=22.5;
    }
    angle = -dAlpha/2+22.5/2;
    for (int i = 0; i < n; i++)
    {
        QPointF * pt = new QPointF(0,-this->parentItem->getSecondRadius());
        rotatePoint(pt,-angle);
        pt->setY(pt->y()+this->parentItem->getSecondRadius());

        (*this->borders) << NULL;
        (*this->borderEndPoints) << pt;

        if (this->parentItem->getParentWidget()!=NULL)
        {
            QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(-5,-5,10,10);
            ellipse->setPos(*pt);
            QBrush b = ellipse->brush();
            b.setColor(Qt::red);
            b.setStyle(Qt::SolidPattern);
            ellipse->setBrush(b);
            ellipse->setZValue(-5000);
            (*this->borderEndPointsGraphics) << ellipse;
            app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(ellipse);
        }
        angle+=22.5;
    }

    if (item->getType()==X1)
    {}
    else if (!(item->getType()==C1 || item->getType()==C2 || item->getType()==CB))
    {
        qreal stdStraight = item->getProdLine()->getMaxStraight();
        int n = (item->getSecondRadius())/stdStraight+1;
        qreal xCoord = -abs(((int)n-1)*(stdStraight/2));
        for (int i = 0; i < 2*n; i++)
        {
            if (i==n)
                xCoord = -abs(((int)n-1)*(stdStraight/2));
            if (i==n && this->parentItem->getType()==J1)
                break;
            if (i==0 && this->parentItem->getType()==J2)
                i=n;
            QPointF * pt = NULL;
            if (i>=n)
                pt = new QPointF(xCoord,2*item->getRadius());
            else
                pt = new QPointF(xCoord,0);
            (*this->borders) << NULL;
            (*this->borderEndPoints) << pt;

            if (this->parentItem->getParentWidget()!=NULL)
            {
                QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(-5,-5,10,10);
                ellipse->setPos(*pt);
                QBrush b = ellipse->brush();
                b.setColor(Qt::red);
                b.setStyle(Qt::SolidPattern);
                ellipse->setBrush(b);
                ellipse->setZValue(-5000);
                (*this->borderEndPointsGraphics) << ellipse;
                app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(ellipse);
            }
            xCoord+=stdStraight;
        }
    }
}

SlotTrackInfo::SlotTrackInfo(const SlotTrackInfo &s)
{
    this->parentItem=s.parentItem;
    this->fstLaneDist=s.fstLaneDist;
    this->lanesGauge=s.lanesGauge;
    this->lanesGaugeEnd=s.lanesGaugeEnd;
    this->numberOfLanes=s.numberOfLanes;
    this->borderEndPointsGraphics=new QList<QGraphicsEllipseItem*>();
    this->borderEndPoints=new QList<QPointF*>();
    this->borders=new QList<BorderItem*>();

    ModelItem * item = s.parentItem;
    qreal dAlpha = (item->getTurnAngle(1))-(item->getTurnAngle(0));
    if (item->getType()==J1 && item->getRadius()<0)
    {
        if (abs(item->getTurnAngle(1)-item->getTurnAngle(0)==180))
            dAlpha-=180;
    }
    else if (item->leftRightDifference180(0,1))
        dAlpha-=180;
    while (dAlpha<0)
        dAlpha=abs(dAlpha);
    while (dAlpha>=360)
        dAlpha-=360;
    int n = dAlpha/22.5;
    qreal angle = -dAlpha/2+22.5/2;

    for (int i = 0; i < n; i++)
    {
        QPointF * pt = new QPointF(0,-this->parentItem->getRadius());
        rotatePoint(pt,-angle);
        pt->setY(pt->y()+this->parentItem->getRadius());
        (*this->borders) << NULL;
        (*this->borderEndPoints) << pt;

        {
            QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(-5,-5,10,10);
            ellipse->setPos(*pt);
            QBrush b = ellipse->brush();
            b.setColor(Qt::red);
            b.setStyle(Qt::SolidPattern);
            ellipse->setBrush(b);
            ellipse->setZValue(-5000);
            (*this->borderEndPointsGraphics) << ellipse;
            app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(ellipse);
        }
        angle+=22.5;
    }

    angle = -dAlpha/2+22.5/2;
    qreal dRadius = this->parentItem->getRadius()-this->parentItem->getSecondRadius();
    for (int i = 0; i < n; i++)
    {
        QPointF * pt = new QPointF(0,-this->parentItem->getSecondRadius());
        rotatePoint(pt,-angle);
        pt->setY(pt->y()+this->parentItem->getSecondRadius()+dRadius);

        (*this->borders) << NULL;
        (*this->borderEndPoints) << pt;

        if (this->parentItem->getParentWidget()!=NULL)
        {
            QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(-5,-5,10,10);
            ellipse->setPos(*pt);
            QBrush b = ellipse->brush();
            b.setColor(Qt::red);
            b.setStyle(Qt::SolidPattern);
            ellipse->setBrush(b);
            ellipse->setZValue(-5000);
            (*this->borderEndPointsGraphics) << ellipse;
            app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(ellipse);
        }
        angle+=22.5;
    }

    if (item->getType()==X1)
    {}
    else if (!(item->getType()==C1 || item->getType()==C2 || item->getType()==CB))
    {
        qreal stdStraight = item->getProdLine()->getMaxStraight();
        int n = abs(item->getSecondRadius())/stdStraight+1;
        qreal xCoord = -abs(((int)n-1)*(stdStraight/2));
        int startValue = 0;
        if (this->parentItem->getType()==SC)
            startValue=n;
        for (int i = startValue; i < 2*n; i++)
        {
            if (i==n)
                xCoord = -abs(((int)n-1)*(stdStraight/2));
            if (i==n && this->parentItem->getType()==J1)
                break;
            if (i==0 && this->parentItem->getType()==J2)
                i=n;
            QPointF * pt = NULL;
            if (i>=n)
                pt = new QPointF(xCoord,2*item->getRadius());
            else
                pt = new QPointF(xCoord,0);

            (*this->borders) << NULL;
            (*this->borderEndPoints) << pt;

            if (this->parentItem->getParentWidget()!=NULL)
            {
                QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(-5,-5,10,10);
                ellipse->setPos(*pt);
                QBrush b = ellipse->brush();
                b.setColor(Qt::red);
                b.setStyle(Qt::SolidPattern);
                ellipse->setBrush(b);
                ellipse->setZValue(-5000);
                (*this->borderEndPointsGraphics) << ellipse;
                app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(ellipse);
            }
            xCoord+=stdStraight;
        }
    }
}

SlotTrackInfo::~SlotTrackInfo()
{
    if (this->parentItem->getParentWidget()!=NULL)
    {
        for (int i = 0; i < this->borderEndPointsGraphics->count();i++)
            app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(this->borderEndPointsGraphics->at(i));
    }
    if (this->parentItem->getParentFragment()==NULL)
    {
        for (int i = 0; i < this->borders->count();i++)
        {
            if (this->borders->at(i)!=NULL)
                delete this->borders->at(i);
        }
    }
    for (int i = 0; i < this->borderEndPoints->count();i++)
        delete this->borderEndPoints->at(i);

    delete this->borderEndPoints;
    delete this->borderEndPointsGraphics;
    delete this->borders;
}

ModelItem *SlotTrackInfo::getParentItem() const
{return this->parentItem;}

int SlotTrackInfo::setParentItem(ModelItem *item)
{
    if (item!=NULL)
    {
        this->parentItem=item;
        return 0;
    }
    return 1;
}

unsigned int SlotTrackInfo::getNumberOfLanes() const
{return this->numberOfLanes;}

qreal SlotTrackInfo::getLanesGauge() const
{return this->lanesGauge;}

qreal SlotTrackInfo::getLanesGaugeEnd() const
{return this->lanesGaugeEnd;}

qreal SlotTrackInfo::getFstLaneDist() const
{return this->fstLaneDist;}

QList<BorderItem *> *SlotTrackInfo::getBorders() const
{return this->borders;}

QList<QPointF *> *SlotTrackInfo::getBorderEndPoints() const
{return this->borderEndPoints;}

QList<QGraphicsEllipseItem *> *SlotTrackInfo::getBorderEndPointsGraphics() const
{return this->borderEndPointsGraphics;}

void SlotTrackInfo::addBorder(BorderItem *border)
{
    logFile << "    Adding the border item " << border->getPartNo()->toStdString() << " at address " << border << " to the item at address " << this->parentItem << endl;


    QString str(QString("make border %1 %2 %3 %4 %5").arg(*border->getPartNo(),*border->getProdLine()->getName(),(border->getInnerBorderFlag() ? "I" : "O"),QString::number(app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->x()),QString::number(app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->y())));

    //move and rotate the border
    qreal dx = border->getEndPoint(0)->x()-app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->x();
    qreal dy = border->getEndPoint(0)->y()-app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->y();
    qreal dAlpha = this->parentItem->getTurnAngle(1)-this->parentItem->getTurnAngle(0);
    if (this->parentItem->leftRightDifference180(0,1))
        dAlpha+=180;

    while (dAlpha<0)
        dAlpha=abs(dAlpha);
    while (dAlpha>=360)
        dAlpha-=360;

    border->moveBy(-dx,-dy);

    qreal angle = this->parentItem->getTurnAngle(0);

    if (this->parentItem->leftRightDifference180(0,1))
        angle-=180;

    bool angleAdjusted = false;
    //set the neighbours
    int j = 0;
    while (border->getEndPoint(j)!=NULL)
    {
        for (int i = 0; i < this->borderEndPoints->count(); i++)
        {
            if (pointsAreCloseEnough(border->getEndPoint(j),this->borderEndPoints->at(i),this->parentItem->getProdLine()->getScaleEnum()/4.0))
            {
                logFile << "        Index of connection point: " << i << endl;
                border->setNeighbour(this->parentItem,j);
                (*this->borders)[i]=border;
                if (!angleAdjusted)
                {
                    angle+=(this->borderEndPoints->count()-1-i)*22.5;
                    angle-=(border->getEndPointsCount()-1)*22.5;
                    angle = angle+border->getAngle()/2;
                    if (this->parentItem->getRadius()<0)
                        angle=angle-dAlpha+180;

                    if (!border->getInnerBorderFlag())
                        angle-=dAlpha;

                    if (border->getAngle()==0 && this->parentItem->getRadius()>0)
                    {
                        angle=0;
                        if (i<this->borderEndPoints->count()/2 && !(this->parentItem->getType()==SC || this->parentItem->getType()==J1 || this->parentItem->getType()==J2))
                            angle=180;
                        if (this->parentItem->getType()==J1)
                            angle=180;

                        angle+=this->parentItem->getTurnAngle(0);

                        if (this->parentItem->leftRightDifference180(0,1))
                            angle-=180;
                    }
                    else if (border->getAngle()==0)
                    {
                        angle=180;
                        if (i<this->borderEndPoints->count()/2 && !(this->parentItem->getType()==SC || this->parentItem->getType()==J1 || this->parentItem->getType()==J2))
                            angle=0;
                        if (this->parentItem->getType()==J1)
                            angle=0;

                        angle+=this->parentItem->getTurnAngle(0);

                        if (this->parentItem->leftRightDifference180(0,1))
                            angle-=180;
                    }

                    logFile << "    Border item will be rotated by " << angle << " around point (" << border->getEndPoint(0)->x() << ", " << border->getEndPoint(0)->y() << endl;
                    QPointF pt = *border->getEndPoint(0);
                    border->rotate(angle,&pt);
                    angleAdjusted=true;
                    i=-1;
                }
            }
        }
        j++;
    }

    //border items have scenePos in a strange place, so executeCommand will not find them
    QString negStr(QString("delete border %1 %2 %3").arg(QString::number(this->parentItem->getParentFragment()->getID()),QString::number(border->getEndPoint(0)->x()),QString::number(border->getEndPoint(0)->y())));

    app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);

    logFile << "    Printing border neighbours:" << endl;
    j = 0;
    while (border->getEndPoint(j)!=NULL)
    {
        logFile << "        " << j << ": " << border->getNeighbour(j) << endl;
        j++;
    }
    logFile << "    Printing item border-neighbours:" << endl;
    for (int i = 0; i < this->borderEndPoints->count(); i++)
    {
        logFile << "        " << i << ": " << this->borders->at(i) << endl;
    }
    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(border->get2DModelNoText());
}

void SlotTrackInfo::removeBorder(BorderItem *border)
{
    logFile << "    Removing the border item " << border->getPartNo()->toStdString() << " at address " << border << " from the item at address " << this->parentItem << endl;

    /*
     *how does the deletion work?
     *Scenario A:
     *  -borderItem was selected by the user, then the delete key was pressed   -> workspace->removeItems() has to be modified
     *  -borderItem is separated from the rest of the model, removed from the scene and then it is destructed (removeBorder())
     *
     *Scenario B:
     *  -modelItem is selected and deleted -> item will be deleted together with the borders -> no need to call removeBorder()
     *  -there may appear a problem related with undo/redo -> correct undo sequence is "make item, make border,..."
     *                                                      -> correct redo sequence "noop, noop, ..., remove item" (remove item has to cause the deletion of borders)
     *
     */

    QString str(QString("delete border %1 %2 %3").arg(QString::number(this->parentItem->getParentFragment()->getID()),QString::number(border->getEndPoint(0)->x()),QString::number(border->getEndPoint(0)->y())));
    QString negStr(QString("make border %1 %2 %3 %4 %5").arg(*border->getPartNo(),*border->getProdLine()->getName(),(border->getInnerBorderFlag() ? "I" : "O"),QString::number(border->getEndPoint(0)->x()),QString::number(border->getEndPoint(0)->y())));

    app->getWindow()->getWorkspaceWidget()->pushBackCommand(str,negStr);

    int i = 0;
    while (border->getEndPoint(i)!=NULL)
    {
        border->setNeighbour(NULL,i);
        i++;
    }
    i = 0;
    for (;i <this->borders->count();i++)
    {
        if (this->borders->at(i)==border)
            (*this->borders)[i]=NULL;
    }
    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->removeItem(border->get2DModelNoText());

    delete border;
}

void makeNewItem(QPointF eventPos, GraphicsPathItemModelItem * gpi, ModelItem * parentItem, ModelItem * toMake, bool key)
{
    bool enable = true;
    if (parentItem->getParentWidget()!=app->getWindow()->getWorkspaceWidget() || key)
    {
        if (app->getRestrictedInventoryMode())
        {
            if (parentItem->getAvailableCount()<1)
                enable = false;
            parentItem->decrAvailableCount();
            gpi->changeCountPath(parentItem->getAvailableCount());
        }

        if (enable)
        {
            ModelItem * it = NULL;
            QList<qreal> * endDegrees = new QList<qreal>();
            QList<QPointF> * endPoints = new QList<QPointF>();
            unsigned int index = 0;
            while(toMake->getEndPoint(index)!=NULL)
            {
                if (eventPos.x()<gpi->pos().x()/2)
                {
                    if ((toMake->getType()==HS || toMake->getType()==HE || toMake->getType()==HC))
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
                    if (((toMake->getType()==X1 && index>=toMake->getSlotTrackInfo()->getNumberOfLanes()*2)) || ((toMake->getType()==J1 || toMake->getType()==J2) && index==2*toMake->getSlotTrackInfo()->getNumberOfLanes()))
                        pt = QPointF(-toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y());
                    else
                    {
                        pt = QPointF(toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y());
                        if (toMake->getType()==HS || toMake->getType()==HE  || toMake->getType()==HC)
                        {
                            pt.setX(-pt.x());

                            QPointF pt2(*toMake->getEndPoint(index+1));
                            pt2.setX(-pt2.x());
                            pt2.setY(-pt2.y());
                            endPoints->push_back(pt2);
                            endDegrees->push_back(toMake->getTurnAngle(index+1));

                            if (index==0)
                                (*endDegrees)[index]=toMake->getTurnAngle(index);

                            index++;

                        }
                    }
                    endPoints->push_back(pt);
                }
                else if (toMake->getType()==J5 && toMake->getSlotTrackInfo()==NULL && eventPos.x()<gpi->pos().x()/2)
                {
                    endPoints->push_back(QPointF(toMake->getEndPoint(index)->x(),-toMake->getEndPoint(index)->y()));
                }
                else
                    endPoints->push_back(*toMake->getEndPoint(index));
                index++;
            }

            if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
            {
                if (eventPos.x()<gpi->pos().x()/2)
                {
                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,-toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(-toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                    {
                        toMake->getSlotTrackInfo()->setParentItem(it);
                        it->setSlotTrackInfo(new SlotTrackInfo(*toMake->getSlotTrackInfo()));
                        it->getSlotTrackInfo()->setParentItem(it);
                        toMake->getSlotTrackInfo()->setParentItem(toMake);
                    }
                }
                else
                {
                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                    {
                        toMake->getSlotTrackInfo()->setParentItem(it);
                        it->setSlotTrackInfo(new SlotTrackInfo(*toMake->getSlotTrackInfo()));
                        it->getSlotTrackInfo()->setParentItem(it);
                        toMake->getSlotTrackInfo()->setParentItem(toMake);
                    }
                }
                //no need to generate model with text, because text isn't needed in workspace scene
                it->generate2DModel(false);
                if (eventPos.x()<gpi->pos().x()/2)
                {
                    if ((!(it->getType()>=T1 && it->getType()<=T10) && app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL))
                        it->setEndPointAngle(0,180+it->getTurnAngle(0));
                }
                else
                {
                    if ((!(it->getType()>=T1 && it->getType()<=T10) && app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL))
                        it->setEndPointAngle(0,180+it->getTurnAngle(0));
                }

                if (toMake->getType()==J2 && toMake->getSlotTrackInfo()==NULL)
                {
                    it->setEndPointAngle(1,180-it->getTurnAngle());
                }

                if(toMake->getType()==J1 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()>gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2,180-it->getTurnAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2));

                if(toMake->getType()==J2 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()>gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2,180-it->getTurnAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2));

                if ((toMake->getType()==X1 || toMake->getType()==J4) && toMake->getSlotTrackInfo()==NULL)
                {
                    if (it->getRadius()<0)
                    {
                    }
                    it->setEndPointAngle(2,180-it->getTurnAngle(2));

                }
                else if ((toMake->getType()==X1 || toMake->getType()==J4) && toMake->getSlotTrackInfo()!=NULL)
                {
                    int index = toMake->getSlotTrackInfo()->getNumberOfLanes()*2;
                    while (it->getEndPoint(index)!=NULL)
                    {
                        it->setEndPointAngle(index,180+it->getTurnAngle(index));
                        index+=2;
                    }
                }

                if (toMake->getType()==J5)
                    it->setEndPointAngle(2,180);
                if (it->getType()>=T2 && it->getType()<=T10)
                {
                    int index = 0;
                    while (index<2*(it->getType()-8))
                    {
                        it->setEndPointAngle(index,180+it->getTurnAngle(index));
                        index+=2;
                    }
                }
                it->moveBy(app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->x(),app->getWindow()->getWorkspaceWidget()->getActiveEndPoint()->y());
                ModelFragment * fragment = new ModelFragment(it);

                app->getWindow()->getWorkspaceWidget()->addFragment(fragment);
                app->getWindow()->getWorkspaceWidget()->setActiveFragment(fragment);

                app->getWindow()->getWorkspaceWidget()->setLastInserted(it);

                QPointF * newActive = NULL;

                if (fragment->getEndPoints()->count()>1)
                    newActive = new QPointF(*fragment->getEndPoints()->at(1));
                else
                    newActive = new QPointF(*fragment->getEndPoints()->at(0));

                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newActive);
            }
            else
            {
                it = NULL;
                if (eventPos.x()<gpi->pos().x()/2)
                {
                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,-toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(-toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                    {
                        toMake->getSlotTrackInfo()->setParentItem(it);
                        it->setSlotTrackInfo(new SlotTrackInfo(*toMake->getSlotTrackInfo()));
                        it->getSlotTrackInfo()->setParentItem(it);
                        toMake->getSlotTrackInfo()->setParentItem(toMake);
                    }
                }
                else
                {
                    it = new ModelItem(*toMake->getPartNo(),*toMake->getNameEn(),*toMake->getNameCs(),
                                       *endPoints,*endDegrees,toMake->getRadius(),toMake->getItemWidth(),
                                       toMake->getItemHeight(), toMake->getType(),toMake->getProdLine(),
                                       app->getWindow()->getWorkspaceWidget());
                    it->setSecondRadius(toMake->getSecondRadius());
                    if (toMake->getSlotTrackInfo()!=NULL)
                    {
                        toMake->getSlotTrackInfo()->setParentItem(it);
                        it->setSlotTrackInfo(new SlotTrackInfo(*toMake->getSlotTrackInfo()));
                        it->getSlotTrackInfo()->setParentItem(it);
                        toMake->getSlotTrackInfo()->setParentItem(toMake);
                    }
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

                if(toMake->getType()==J1 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()<gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2,180-it->getTurnAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2));

                if(toMake->getType()==J2 && toMake->getSlotTrackInfo()!=NULL && eventPos.x()<gpi->pos().x()/2)
                    it->setEndPointAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2,180-it->getTurnAngle(it->getSlotTrackInfo()->getNumberOfLanes()*2));

                if (toMake->getType()==X1 || toMake->getType()==J4)
                {
                    it->setEndPointAngle(2,180-it->getTurnAngle(2));
                }

                if (toMake->getType()==J5)
                    it->setEndPointAngle(2,180);

                if (toMake->getType()==T1)
                    it->rotate(180);

                app->getWindow()->getWorkspaceWidget()->getActiveFragment()->addFragmentItem(it,app->getWindow()->getWorkspaceWidget()->getActiveEndPoint());
                app->getWindow()->getWorkspaceWidget()->setLastInserted(it);

            }
            app->getWindow()->getWorkspaceWidget()->setLastUsedPart(parentItem);
            app->getWindow()->getWorkspaceWidget()->setLastEventPos(eventPos);
        }
    }
}
void makeNewBorder(BorderItem * item)
{
    bool enable = true;

    if (app->getRestrictedInventoryMode())
    {
        if (item->getAvailableCount()<1)
            enable = false;
        item->decrAvailableCount();
        item->get2DModel()->changeCountPath(item->getAvailableCount());
    }

    if (enable)
    {
        BorderItem * it = NULL;

        QList<QPointF> * endPoints = new QList<QPointF>();
        int index = 0;

        while(item->getEndPoint(index)!=NULL)
        {
            endPoints->push_back(*item->getEndPoint(index));
            index++;
        }

        it = new BorderItem(*item->getPartNo(),*item->getNameEn(),*item->getNameCs(),
                           item->getAngle(),item->getRadius(),*endPoints,item->getInnerBorderFlag(),item->getProdLine(),
                           app->getWindow()->getWorkspaceWidget());

        //no need to generate model with text, because text isn't needed in workspace scene
        it->generate2DModel(false);

        app->getWindow()->getWorkspaceWidget()->getSelection()->first()->getSlotTrackInfo()->addBorder(it);
    }
}

VegetationItem * makeNewVegetation(VegetationItem *item)
{
    bool enable = true;

    if (app->getRestrictedInventoryMode())
    {
        if (item->getAvailableCount()<1)
            enable = false;
        item->decrAvailableCount();
        item->get2DModel()->changeCountPath(item->getAvailableCount());
    }
    if (enable)
    {
        VegetationItem * it = NULL;

        it = new VegetationItem(*item->getPartNo(),*item->getNameEn(),*item->getNameCs(),*item->getSeason(),
                                item->getItemWidth(),item->getItemHeight(),item->getProdLine(),app->getWindow()->getWorkspaceWidget());

        QPointF * pt = app->getWindow()->getWorkspaceWidget()->getActiveEndPoint();

        //no need to generate model with text, because text isn't needed in workspace scene
        it->generate2DModel(false);

        app->getWindow()->getWorkspaceWidget()->addVegetation(it);
        app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(it->get2DModelNoText());
        it->moveBy(pt->x(),pt->y(),false);

        return it;
    }
    else
        return NULL;
}

void rebuildFragment(ModelItem * startItem, ModelFragment * fragment)
{
    logFile << "    Rebuilding fragment: new fragment has address " << fragment << " and id " << fragment->getID() << ", start item is at " << startItem << endl;

    ModelItem * currentItem = NULL;
    ModelItem * previousItem = startItem;
    QPointF * currentPoint = NULL;
    QList <ModelItem*> itemsToVisit;
    QList <ModelItem*> visitedItems;

    {
        /*what is this block of code good for:
         *-this function is called in the way that startItem = item from which parameter "fragment" was created
         *-it means that startItem can't be added anymore (it would be for the second time) -> add it into visitedItems list
         *-but the list of its neighbours is needed
        */
        itemsToVisit.push_back(startItem);
        int k = 0;
        while (startItem->getEndPoint(k)!=NULL)
        {
            if (startItem->getNeighbour(k)!=NULL && !itemsToVisit.contains(startItem->getNeighbour(k)))
                itemsToVisit.push_back(startItem->getNeighbour(k));
            k++;
        }

        visitedItems.push_back(startItem);
    }
    bool makeFirst = false; //is used when there is no way how to connect currentItem by the point[0] and preserve continuity of fragment
    while (!itemsToVisit.empty())
    {
        if (currentItem!=itemsToVisit.first())
            previousItem = currentItem;

        for (int k = 0; k < itemsToVisit.count();k++)
        {
            if (itemsToVisit.at(k)->getDeleteFlag())
            {
                itemsToVisit.removeAll(itemsToVisit.at(k));
                k=0;
                continue;
            }
        }

        if (!itemsToVisit.empty())
            currentItem = itemsToVisit.first();
        else
            return;

        logFile << "    currentItem = " << currentItem << ", previousItem = " << previousItem << endl;

        currentPoint = new QPointF(*currentItem->getEndPoint(0));
        itemsToVisit.removeFirst();

        //add item in the fragment at appropriate point (see recursion)
        if (!visitedItems.contains(currentItem))
        {
            int i = 0; //for indexing of previousItem
            int j = 0; //for indexing of currentItem
            while (previousItem->getEndPoint(i)!=NULL)
            {
                if (previousItem->getNeighbour(i)==currentItem)
                    break;
                i++;
            }

            if (previousItem->getEndPoint(i)==NULL)
            {
                //there is no connection between current and previous item
                //what to do when it happens?
                //previousItem should contain the pointer to item which is connected to currentItem
                //it can be [L] or [R] neighbour of the current item depending on the direction of the currItem
                //to determine if it is L or R neighbour do this:
                //loop through the endpoints of currentItem's neighbour at [0] and find the correct index of connection
                i = 0;
                while (currentItem->getEndPoint(j)!=NULL)
                {
                    if (currentItem->getNeighbour(j)!=NULL && visitedItems.contains(currentItem->getNeighbour(j)))
                    {
                        while (currentItem->getNeighbour(j)->getEndPoint(i)!=NULL && currentItem->getNeighbour(j)->getNeighbour(i)!=currentItem)
                        {
                            i++;
                        }
                        if (currentItem->getNeighbour(j)->getEndPoint(i)!=NULL)
                            break;
                    }
                    j++;
                }
                previousItem = currentItem->getNeighbour(j);
            }
            else
            {
                while (currentItem->getEndPoint(j)!=NULL)
                {
                    if (currentItem->getNeighbour(j)==previousItem)
                        break;
                    j++;
                }
            }

            if (previousItem==NULL)
            {
                logFile << "    ERROR while rebuildFragment" << endl;
                return;
            }

            *currentPoint=*previousItem->getEndPoint(i);

            qreal dAlpha = abs(currentItem->getTurnAngle(j)-previousItem->getTurnAngle(i));

            while (dAlpha<0)
                dAlpha+=360;
            while(dAlpha>360)
                dAlpha-=360;

            //this fixes incorrect behaviour of deletion after bending
            for (int k = 0; k < fragment->getEndPoints()->count(); k++)
            {
                if (currentItem->getSlotTrackInfo()!=NULL && (currentItem->getType()==J1 || currentItem->getType()==J2) && j==4)
                {
                    if (pointsAreCloseEnough(fragment->getEndPoints()->at(k),currentItem->getEndPoint(j),currentItem->getProdLine()->getScaleEnum()/4.0))
                    {
                        //QPointF * pt = fragment->getEndPoints()->at(k);
                        //fragment->removeEndPoint(pt);
                        if (abs(dAlpha)>=150 && abs(dAlpha)<=290)
                        {
                            fragment->addEndPoint(currentItem->getEndPoint(1),true,currentItem->getTurnAngle(j)-180,currentItem->getNeighbour(1));
                            logFile << "    1.1RebuildFragment: endpoint (" << currentItem->getEndPoint(j%2)->x() << ", " << currentItem->getEndPoint(j%2)->y() << ") with the angle of " << currentItem->getTurnAngle(j) << "-" << dAlpha << " degrees was added in fragment " << fragment->getID() << endl;
                        }
                        else
                        {
                            fragment->addEndPoint(currentItem->getEndPoint(1),true,currentItem->getTurnAngle(j),currentItem->getNeighbour(1));
                            logFile << "    1.1RebuildFragment: endpoint (" << currentItem->getEndPoint(j%2)->x() << ", " << currentItem->getEndPoint(j%2)->y() << ") with the angle of " << currentItem->getTurnAngle(j) << " degrees was added in fragment " << fragment->getID() << endl;
                        }
                        break;
                    }
                }
                else if (pointsAreCloseEnough(fragment->getEndPoints()->at(k),currentItem->getEndPoint(j%2),currentItem->getProdLine()->getScaleEnum()/4.0))
                {
                    QPointF * pt = fragment->getEndPoints()->at(k);
                    fragment->removeEndPoint(pt);
                    if (abs(dAlpha)>=150 && abs(dAlpha)<=290)
                    {

                        fragment->addEndPoint(currentItem->getEndPoint(j%2),true,currentItem->getTurnAngle(j)-180,previousItem);
                        logFile << "    1.2RebuildFragment: endpoint (" << currentItem->getEndPoint(j%2)->x() << ", " << currentItem->getEndPoint(j%2)->y() << ") with the angle of " << currentItem->getTurnAngle(j) << "-" << dAlpha << " degrees was added in fragment " << fragment->getID() << endl;
                    }
                    else
                    {
                        fragment->addEndPoint(currentItem->getEndPoint(j%2),true,currentItem->getTurnAngle(j),previousItem);
                        logFile << "    1.2RebuildFragment: endpoint (" << currentItem->getEndPoint(j%2)->x() << ", " << currentItem->getEndPoint(j%2)->y() << ") with the angle of " << currentItem->getTurnAngle(j) << " degrees was added in fragment " << fragment->getID() << endl;
                    }
                    break;
                }
            }
            //if N is at odd point && N is connected to currentItem by odd point: -P-><-C-
            //or N is at odd point && N is connected to currentItem by even point: -C->-P->
            logFile << "        i=" << i << " j=" << j << endl;
            if ((i%2==0 && j%2==0 && currentItem->getSlotTrackInfo()!=NULL && (currentItem->getType()==J1 || currentItem->getType()==J2)) || (i%2==1 && j%2==1) || (i%2==0 && j%2==1) || makeFirst || (dAlpha > 150))//(i%2==1 && j%2==1)
            {
                int l = i;
                if ((((previousItem->getRadius()>0 && currentItem->getRadius() >0)
                    || (previousItem->getRadius()<0 && currentItem->getRadius() <0))
                        && ((i%2==1 && j%2==1)))
                    ||
                        (((previousItem->getRadius()>0 && currentItem->getRadius() <0)
                        || (previousItem->getRadius()<0 && currentItem->getRadius() >0))
                        && ((i%2==0 && j%2==1)))
                    )
                {
                    bool incremented = false;
                    while (previousItem->getEndPoint(l)!=NULL)
                    {
                        if (previousItem->getNeighbour(l)!=currentItem)
                        {
                            //don't do it in case when there is just one-lane connection
                            if (incremented)
                                l-=2;
                            break;
                        }
                        l+=2;
                        incremented = true;
                    }
                    //if this condition is satisfied it means that previousItem is connected with its last lane too, so it needs to be decreased
                    if (previousItem->getEndPoint(l)==NULL)
                        l-=2;
                    logFile << "        l=" << l << endl;
                }

                ModelItem * currentItemNeighbourAtMod = currentItem->getNeighbour(j%2);
                if (currentItem->getType()==J1 || currentItem->getType()==J2 || currentItem->getType()==J3 || currentItem->getType()==J1)
                    currentItemNeighbourAtMod = currentItem->getNeighbour(j);
                QList<ModelItem*> currentItemOriginalNeighbours;
                QList<QPointF> currentItemOriginalPoints;

                int k = 0;
                while (currentItem->getEndPoint(k)!=NULL)
                {
                    currentItemOriginalNeighbours.push_back(currentItem->getNeighbour(k));
                    currentItemOriginalPoints.push_back(QPointF(*currentItem->getEndPoint(k)));
                    if (currentItem->getNeighbour(k)==NULL)
                    {
                        QPointF * ptr = currentItem->getEndPoint(k);
                        fragment->removeEndPoint(ptr);

                    }
                    else
                    {
                        currentItem->getNeighbour(k)->setNeighbour(NULL,currentItem->getEndPoint(k));
                    }
                    currentItem->setNeighbour(NULL,k);
                    k++;
                }

                //fragment->getFragmentItems()->removeOne(currentItem);
                itemsToVisit.removeAll(currentItem);
                //find the item from which "currentItem" was created
                ModelItem * originalItem = NULL;
                QList<ModelItem*>::Iterator iter = currentItem->getProdLine()->getItemsList()->begin();
                while (iter!=currentItem->getProdLine()->getItemsList()->end())
                {
                    if (*(*iter)->getNameCs()==*currentItem->getNameCs()
                            && *(*iter)->getNameEn()==*currentItem->getNameEn()
                            && *(*iter)->getPartNo()==*currentItem->getPartNo())
                    {
                        originalItem = *iter;
                        break;
                    }
                    iter++;
                }
                QPointF pos;
                //determine if the new one will be left- or right-turn-item
                if (dAlpha > 90)
                {
                    if ((currentItem->getRadius()<0 && j%2==0)
                        || (currentItem->getRadius()>0 && j%2==1)
                        || (currentItem->getRadius()>0 && currentItem->getType()==J2 && j==2 && currentItem->getSlotTrackInfo()==NULL)
                            )
                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                    else
                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);

                    if (currentItem->getSlotTrackInfo()!=NULL && (currentItem->getType()==J1 || currentItem->getType()==J2))
                    {
                        if ((j%2==1 || j==4) && currentItem->getRadius()<0)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                        else if (j%2==1 || j==4)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                    }
                }
                else
                {
                    logFile << "        deciding radius - currentItem radius: " << currentItem->getRadius() << endl;
                    if ((currentItem->getRadius()<0 && j%2==0)
                            || (currentItem->getRadius()>0 && j%2==1))
                            //((currentItem->getRadius()>0))
                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                    else
                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                }
                if ((currentItem->getType()==X1 || currentItem->getType()==J4))
                {
                    //if signbit(currentItem->getRadius())!=signbit(pos.x())
                    if (j==0 || j==1)
                    {
                        if (currentItem->getRadius()>0)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                        else
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                    }
                    else if (j==2)
                    {
                        if (currentItem->getRadius()>0)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                        else
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                    }
                }
                if (currentItem->getType()==HE || currentItem->getType()==HS)
                {
                    if (currentItem->getRadius()>0)
                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                    else
                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                }
                //QPointF * active = new QPointF(*previousItem->getEndPoint(l));
                QPointF * active = new QPointF(*currentItem->getEndPoint(j%2));
                QPointF * forFragment = new QPointF(*currentItem->getEndPoint(j%2));

                if (j==4 && currentItem->getSlotTrackInfo()!=NULL && (currentItem->getType()==J1 || currentItem->getType()==J2))
                {
                    currentItemNeighbourAtMod=currentItemOriginalNeighbours.at(1);
                    *currentPoint = *currentItem->getEndPoint(1);
                    *forFragment = *currentPoint;
                    *active = *forFragment;
                }

                //this bool variable is used in situations when [0] is not reachable in usual way
                bool isThere = false;
                bool wasChecked = false;
                if (currentItem->getSlotTrackInfo() || (currentItem->getType() >= T2 && currentItem->getType()<=T10))
                {
                    bool modPointIsUnreachable = false;
                    {
                        //QRectF r(currentItem->getEndPoint(j%2)->x()-TOLERANCE_HALF,currentItem->getEndPoint(j%2)->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);
                        int index = 0;
                        while(previousItem->getEndPoint(index)!=NULL)
                        {
                            //if (pointsAreCloseEnough(previousItem->getEndPoint(index),currentItem->getEndPoint(j%2),previousItem->getProdLine()->getScaleEnum()/4.0))
                            if (pointsAreCloseEnough(previousItem->getEndPoint(index),forFragment,previousItem->getProdLine()->getScaleEnum()/4.0))
                                break;
                            index++;
                        }
                        if (previousItem->getEndPoint(index)==NULL)
                            modPointIsUnreachable=true;
                    }

                    int prevCount = 0;
                    int currCount = 0;
                    while (previousItem->getEndPoint(prevCount)!=NULL)
                        prevCount++;
                    while (currentItem->getEndPoint(currCount)!=NULL)
                        currCount++;
                    prevCount--;
                    currCount--;

                    if (prevCount!=currCount
                        || modPointIsUnreachable)
                    {
                        //angle needs to be set if [0] is not reachable from previousItem
                        qreal angleOfPoint=previousItem->getTurnAngle(i);

                        if (j%2==0 && currentItem->getEndPoint(j+1)!=NULL)
                        {
                            for (int n = 0; n < currentItemOriginalNeighbours.count(); n+=2)
                            {
                                if (currentItemOriginalNeighbours.at(n)!=NULL && currentItemOriginalNeighbours.at(n)->getParentFragment()==fragment)
                                {
                                    angleOfPoint=currentItemOriginalNeighbours.at(n)->getTurnAngle(currentItem->getEndPoint(n));
                                    break;
                                }
                            }
                        }

                        for (int n = 0; n < fragment->getEndPoints()->count(); n++)
                        {
                            wasChecked=true;
                            if (pointsAreCloseEnough(forFragment,fragment->getEndPoints()->at(n),fragment->getProductLines()->first()->getScaleEnum()/4.0))
                            {
                                isThere = true;
                                break;
                            }


                        }
                        if (!isThere)
                        {

                            fragment->addEndPoint(forFragment,true,angleOfPoint,currentItemNeighbourAtMod);
                            logFile << "    2RebuildFragment: endpoint (" << forFragment->x() << ", " << forFragment->y() << ") with the angle of " << angleOfPoint << " degrees was added in fragment " << fragment->getID() << endl;
                        }
                    }
                }

                else if (currentItem->getType()==J4 || currentItem->getType()==X1)
                {
                    *active = *currentItem->getEndPoint(j);
                    *forFragment = *currentItem->getEndPoint(j);
                    qreal angleOfPoint = 0;
                    for (int n = j%2; n < currentItemOriginalNeighbours.count(); n+=2)
                    {
                        if (currentItemOriginalNeighbours.at(n)!=NULL && currentItemOriginalNeighbours.at(n)->getParentFragment()==fragment)
                        {
                            angleOfPoint=currentItemOriginalNeighbours.at(n)->getTurnAngle(currentItem->getEndPoint(j));

                            break;
                        }
                    }


                    for (int n = 0; n < fragment->getEndPoints()->count(); n++)
                    {
                        wasChecked=true;
                        if (pointsAreCloseEnough(forFragment,fragment->getEndPoints()->at(n),fragment->getProductLines()->first()->getScaleEnum()/4.0))
                        {
                            isThere = true;
                            break;
                        }
                    }
                    if (!isThere)
                    {
                        fragment->addEndPoint(forFragment,true,angleOfPoint,currentItemNeighbourAtMod);
                        logFile << "    3RebuildFragment: endpoint (" << forFragment->x() << ", " << forFragment->y() << ") with the angle of " << angleOfPoint << " degrees was added in fragment " << fragment->getID() << endl;
                    }
                }
                else if (currentItem->getType()==J2)
                {
                    *active = *currentItem->getEndPoint(j);
                    *forFragment = *currentItem->getEndPoint(j);
                    qreal angleOfPoint = 0;

                    for (int n = 0; n < currentItemOriginalNeighbours.count(); n++)
                    {
                        if (currentItemOriginalNeighbours.at(n)!=NULL && currentItemOriginalNeighbours.at(n)->getParentFragment()==fragment)
                        {
                            angleOfPoint=currentItemOriginalNeighbours.at(n)->getTurnAngle(currentItem->getEndPoint(j));
                            break;
                        }
                    }

                    if (j==1 && currentItem->getRadius()>0)
                    {
                        *active = *currentItem->getEndPoint(0);
                        *forFragment = *currentItem->getEndPoint(0);

                        if (!(currentItem->leftRightDifference180(0,2) &&currentItem->leftRightDifference180(1,2)))
                            angleOfPoint -= abs(-180+currentItem->getTurnAngle(0)-currentItem->getTurnAngle(1));
                        else
                            angleOfPoint -= abs(currentItem->getTurnAngle(0)-currentItem->getTurnAngle(1));
                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                    }
                    else if (j==1)
                    {
                        *active = *currentItem->getEndPoint(2);
                        *forFragment = *currentItem->getEndPoint(2);

                        if (!(currentItem->leftRightDifference180(0,2) &&currentItem->leftRightDifference180(1,0)))
                            angleOfPoint -= abs(-180+currentItem->getTurnAngle(2)-currentItem->getTurnAngle(1));
                        else
                            angleOfPoint -= abs(currentItem->getTurnAngle(2)-currentItem->getTurnAngle(1));

                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                    }
                    else if (j==2 && currentItem->getRadius()<0)
                    {
                        *active = *currentItem->getEndPoint(2);
                        *forFragment = *currentItem->getEndPoint(2);

                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                    }

                    for (int n = 0; n < fragment->getEndPoints()->count(); n++)
                    {
                        wasChecked=true;
                        if (pointsAreCloseEnough(forFragment,fragment->getEndPoints()->at(n),fragment->getProductLines()->first()->getScaleEnum()/4.0))
                        {
                            isThere = true;
                            break;
                        }
                    }
                    if (!isThere)
                    {
                        fragment->addEndPoint(forFragment,true,angleOfPoint,currentItemNeighbourAtMod);
                        logFile << "    4RebuildFragment: endpoint (" << forFragment->x() << ", " << forFragment->y() << ") with the angle of " << angleOfPoint << " degrees was added in fragment " << fragment->getID() << endl;
                    }


                }
                else if (currentItem->getType()==J1)
                {
                    qreal angleOfPoint = 0;

                    for (int n = 0; n < currentItemOriginalNeighbours.count(); n++)
                    {
                        if (currentItemOriginalNeighbours.at(n)!=NULL && currentItemOriginalNeighbours.at(n)->getParentFragment()==fragment)
                        {
                            angleOfPoint=currentItemOriginalNeighbours.at(n)->getTurnAngle(currentItem->getEndPoint(j));
                            break;
                        }
                    }

                    if (j==1 && currentItem->getRadius()>0)
                    {
                        *active = *currentItem->getEndPoint(2);
                        *forFragment = *currentItem->getEndPoint(2);

                        if (!(currentItem->leftRightDifference180(0,2) &&currentItem->leftRightDifference180(0,1)))
                            angleOfPoint += abs(180+currentItem->getTurnAngle(1)-currentItem->getTurnAngle(2));
                        else
                            angleOfPoint += abs(currentItem->getTurnAngle(2)-currentItem->getTurnAngle(1));



                    }
                    else if (j==1)
                    {
                        *active = *currentItem->getEndPoint(0);
                        *forFragment = *currentItem->getEndPoint(0);

                        if (!(currentItem->leftRightDifference180(0,2) &&currentItem->leftRightDifference180(1,2)))
                            angleOfPoint += abs(180+currentItem->getTurnAngle(1)-currentItem->getTurnAngle(0));
                        else
                            angleOfPoint += abs(currentItem->getTurnAngle(1)-currentItem->getTurnAngle(0));

                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                    }
                    else if (j==2 && currentItem->getRadius()>0)
                    {
                        *active = *currentItem->getEndPoint(2);
                        *forFragment = *currentItem->getEndPoint(2);

                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                    }
                    else if (j==2)
                    {
                        *active = *currentItem->getEndPoint(2);
                        *forFragment = *currentItem->getEndPoint(2);

                        pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                    }

                    for (int n = 0; n < fragment->getEndPoints()->count(); n++)
                    {
                        wasChecked=true;
                        if (pointsAreCloseEnough(forFragment,fragment->getEndPoints()->at(n),fragment->getProductLines()->first()->getScaleEnum()/4.0))
                        {
                            isThere = true;
                            break;
                        }
                    }
                    if (!isThere)
                    {
                        fragment->addEndPoint(forFragment,true,angleOfPoint,currentItemNeighbourAtMod);
                        logFile << "    5RebuildFragment: endpoint (" << forFragment->x() << ", " << forFragment->y() << ") with the angle of " << angleOfPoint << " degrees was added in fragment " << fragment->getID() << endl;
                    }
                }
                else if (currentItem->getType()==J3)
                {
                    qreal angleOfPoint =0;

                    for (int n = 0; n < currentItemOriginalNeighbours.count(); n++)
                    {
                        if (currentItemOriginalNeighbours.at(n)!=NULL && currentItemOriginalNeighbours.at(n)->getParentFragment()==fragment)
                        {
                            angleOfPoint=currentItemOriginalNeighbours.at(n)->getTurnAngle(currentItem->getEndPoint(j));
                            break;
                        }
                    }

                    if (j==0)
                    {
                        *active = *currentItem->getEndPoint(j);
                        *forFragment = *currentItem->getEndPoint(j);

                        if (currentItem->getRadius()>0)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                        else
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);

                    }
                    else if (j==1)
                    {
                        *active = *currentItem->getEndPoint(j);
                        *forFragment = *currentItem->getEndPoint(j);

                        if (currentItem->getRadius()>0)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                        else
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);

                    }
                    else if ((j==2 || j==3) && currentItem->getRadius()>0)
                    {
                        *active = *currentItem->getEndPoint(1);
                        *forFragment = *currentItem->getEndPoint(1);

                        if (currentItem->getRadius()>0)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);
                        else
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);

                        if (j==2)
                            angleOfPoint += abs(currentItem->getTurnAngle(j)-currentItem->getTurnAngle(1));
                        else
                            angleOfPoint -= abs(currentItem->getTurnAngle(j)-currentItem->getTurnAngle(1));
                    }
                    else if ((j==2 || j==3) && currentItem->getRadius()<0)
                    {
                        *active = *currentItem->getEndPoint(0);
                        *forFragment = *currentItem->getEndPoint(0);

                        if (currentItem->getRadius()>0)
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()+5);
                        else
                            pos.setX(originalItem->get2DModelNoText()->scenePos().x()-5);

                        if (!(currentItem->leftRightDifference180(j,1) && currentItem->leftRightDifference180(0,1))
                           || !(!currentItem->leftRightDifference180(j,1) && !currentItem->leftRightDifference180(0,1)))
                            angleOfPoint -= abs(180+currentItem->getTurnAngle(j)-currentItem->getTurnAngle(1));
                        else
                            angleOfPoint -= abs(currentItem->getTurnAngle(j)-currentItem->getTurnAngle(1));
                    }

                    for (int n = 0; n < fragment->getEndPoints()->count(); n++)
                    {
                        wasChecked=true;
                        if (pointsAreCloseEnough(forFragment,fragment->getEndPoints()->at(n),fragment->getProductLines()->first()->getScaleEnum()/4.0))
                        {
                            isThere = true;
                            break;
                        }
                    }
                    if (!isThere)
                    {
                        fragment->addEndPoint(forFragment,true,angleOfPoint,currentItemNeighbourAtMod);
                        logFile << "    6RebuildFragment: endpoint (" << forFragment->x() << ", " << forFragment->y() << ") with the angle of " << angleOfPoint << " degrees was added in fragment " << fragment->getID() << endl;
                    }
                }

                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(active);
                ModelFragment * previouslyActiveFragment = app->getWindow()->getWorkspaceWidget()->getActiveFragment();
                app->getWindow()->getWorkspaceWidget()->setActiveFragment(fragment);

                currentItem->setDeleteFlag();
                unsigned int countBefore = originalItem->getAvailableCount();
                makeNewItem(pos,originalItem->get2DModelNoText(),originalItem,originalItem,false);

                if (app->getRestrictedInventoryMode())
                {
                    originalItem->setAvailableCount(countBefore);
                    originalItem->get2DModel()->changeCountPath(countBefore);
                }

                app->getWindow()->getWorkspaceWidget()->setActiveFragment(previouslyActiveFragment);

                if (makeFirst)
                    makeFirst=false;

                ModelItem * newN = fragment->getFragmentItems()->last();

                if (currentItem->getSlotTrackInfo()!=NULL)
                {
                    QList<BorderItem*>* currentBorders = currentItem->getSlotTrackInfo()->getBorders();
                    for (int n = 0; n < currentBorders->count(); n++)
                    {
                        if (currentBorders->at(n)!=NULL)
                        {
                            (*newN->getSlotTrackInfo()->getBorders())[n]=currentBorders->at(n);
                            currentBorders->at(n)->setNeighbour(newN,currentItem->getSlotTrackInfo()->getBorderEndPoints()->at(n));
                        }
                    }
                }

                {
                    int n = 0;
                    while (newN->getEndPoint(n)!=NULL)
                    {
                        newN->adjustHeightProfile(currentItem->getHeightProfileAt(newN->getEndPoint(n))-newN->getHeightProfileAt(newN->getEndPoint(n)),newN->getEndPoint(n),false);
                        n++;
                    }
                }

                if (currentItem->get2DModelNoText()->isSelected())
                {
                    app->getWindow()->getWorkspaceWidget()->selectItem(newN);
                    app->getWindow()->getWorkspaceWidget()->deselectItem(currentItem);

                }

                if (!isThere && wasChecked)
                {
                    fragment->addEndPoint(newN->getEndPoint(0),true,newN->getTurnAngle(0),newN);
                }

                //fix the neighbours
                {
                    int k = 0;
                    while (newN->getEndPoint(k)!=NULL)
                    {
                        if (newN->getSlotTrackInfo()!=NULL && k%2==0)
                        {
                            if ((newN->getType()==J1 || newN->getType()==J2) && newN->getEndPoint(k+1)==NULL)
                            {}
                            else
                            {
                                k++;
                                continue;
                            }
                        }

                        int indexOld = -1;
                        for (int m = 0; m < currentItemOriginalPoints.count(); m++)
                        {
                            if (pointsAreCloseEnough(&currentItemOriginalPoints[m],newN->getEndPoint(k),newN->getProdLine()->getScaleEnum()/4.0))
                            {
                                indexOld=m;
                                break;
                            }
                        }
                        if (indexOld!=-1 && newN->getType()!=J3)
                        {
                            if (k%2==0 &&
                                (!newN->leftRightDifference180(k,k+1) && (currentItemOriginalNeighbours.at(indexOld)==NULL
                                                                          || currentItemOriginalNeighbours.at(indexOld)->getParentFragment()!=fragment) )
                                    )
                            {
                                if (k==0)
                                {
                                    newN->setEndPointAngle(k,180+newN->getTurnAngle(k));
                                    fragment->setEndPointAngle(newN->getEndPoint(k),newN->getTurnAngle(k));
                                }
                                else if (!(newN->getType()==J2 || newN->getType()==J1))
                                    newN->setEndPointAngle(k,180+newN->getTurnAngle(k));
                            }
                            if (k==1 && (newN->getType()==J2) && newN->getRadius()<0 && newN->getSlotTrackInfo()==NULL)
                            {
                                if (currentItemOriginalNeighbours.at(indexOld)!=NULL && newN->leftRightDifference180(1,2))
                                    newN->setEndPointAngle(newN->getEndPoint(k),-180+newN->getTurnAngle(k));
                            }
                            if (k==0 && (newN->getType()==J1) && newN->getRadius()<0 && !newN->leftRightDifference180(0,2)  && newN->getSlotTrackInfo()==NULL)
                            {
                                newN->setEndPointAngle(k,180+newN->getTurnAngle(k));
                                fragment->setEndPointAngle(newN->getEndPoint(k),newN->getTurnAngle(k));
                            }

                            newN->setNeighbour(currentItemOriginalNeighbours.at(indexOld),newN->getEndPoint(k));
                            if (currentItemOriginalNeighbours.at(indexOld)!=NULL)
                            {
                                currentItemOriginalNeighbours.at(indexOld)->setNeighbour(newN,newN->getEndPoint(k));
                            }
                        }
                        else if (indexOld!=-1 && newN->getType()==J3)
                        {
                            if (k==0 &&
                                (!newN->leftRightDifference180(k,k+1) && (currentItemOriginalNeighbours.at(indexOld)==NULL
                                                                          || currentItemOriginalNeighbours.at(indexOld)->getParentFragment()!=fragment) )
                                    )
                            {
                                newN->setEndPointAngle(k,180+newN->getTurnAngle(k));
                                fragment->setEndPointAngle(newN->getEndPoint(k),newN->getTurnAngle(k));
                            }
                            if ((k==2 || k==3) && newN->getRadius()<0)
                            {
                                if ((!newN->leftRightDifference180(1,k) && (currentItemOriginalNeighbours.at(indexOld)==NULL
                                                                              || currentItemOriginalNeighbours.at(indexOld)->getParentFragment()!=fragment) ))
                                {
                                    newN->setEndPointAngle(k,180+newN->getTurnAngle(k));
                                    fragment->setEndPointAngle(newN->getEndPoint(k),newN->getTurnAngle(k));
                                }
                            }

                            newN->setNeighbour(currentItemOriginalNeighbours.at(indexOld),newN->getEndPoint(k));
                            if (currentItemOriginalNeighbours.at(indexOld)!=NULL)
                            {
                                currentItemOriginalNeighbours.at(indexOld)->setNeighbour(newN,newN->getEndPoint(k));
                            }
                        }
                        else
                        {
                            /*
                             * Self repairing code
                             * -when this error occurs, it means that the item is flipped by 180 deg.
                             * -in this situation the "newN" and previousItem neighbourhood is set in the wrong way
                             */
                            logFile << "    ERROR 4131: indexOld == -1" << endl;
                            newN->rotate(180,forFragment);
                            fragment->updateEndPointsGraphics();
                            int iN = 0;
                            int iP = 0;
                            while (newN->getEndPoint(iN)!=NULL)
                            {
                                iP=0;
                                while (previousItem->getEndPoint(iP)!=NULL)
                                {
                                    if (pointsAreCloseEnough(previousItem->getEndPoint(iP),newN->getEndPoint(iN),newN->getProdLine()->getScaleEnum()/4))
                                    {
                                        if (previousItem->getNeighbour(iP)==NULL)
                                        {
                                            if (previousItem->getSlotTrackInfo()!=NULL)
                                            {
                                                int index1 = 0;
                                                int index2 = 0;
                                                if (iP%2==0 && !((previousItem->getType()==J1 || previousItem->getType()==J2) && previousItem->getEndPoint(iP+1)==NULL))
                                                {
                                                    index1 = iP;
                                                    index2 = iP+1;
                                                }
                                                else if (!((previousItem->getType()==J1 || previousItem->getType()==J2) && previousItem->getEndPoint(iP+1)==NULL))
                                                {
                                                    index1 = iP-1;
                                                    index2 = iP;
                                                }

                                                if (!previousItem->leftRightDifference180(index1,index2))
                                                    previousItem->setEndPointAngle(iP,previousItem->getTurnAngle(iP)+180);
                                            }
                                            previousItem->setNeighbour(newN,previousItem->getEndPoint(iP));
                                        }
                                        if (newN->getNeighbour(iN)==NULL)
                                        {
                                            newN->setNeighbour(previousItem,newN->getEndPoint(iN));
                                        }
                                        QPointF * pt = NULL;
                                        while (fragment->findEndPointItem(newN->getEndPoint(iN))!=NULL)
                                        {
                                            pt = newN->getEndPoint(iN);
                                            if (fragment->removeEndPoint(pt)==2)
                                            {
                                                pt=previousItem->getEndPoint(iP);
                                                fragment->removeEndPoint(pt);
                                            }
                                        }
                                    }
                                    iP++;
                                }
                                iN++;
                            }
                            k=0;
                            continue;
                        }
                        k++;
                    }
                }

                visitedItems.push_back(newN);
                int x = 0;
                while (newN->getEndPoint(x)!=NULL)
                {
                    // for each neighbour of newN
                    //     for each neighbour of that neighbour
                    //         if n == currentItem (which will be deleted)
                    //             set it to newN

                    if (newN->getNeighbour(x)!=NULL)
                    {
                        int nIndex = 0;
                        ModelItem * n = newN->getNeighbour(x);
                        while (n->getEndPoint(nIndex)!=NULL)
                        {
                            if (n->getNeighbour(nIndex)==currentItem)
                                n->setNeighbour(newN, n->getEndPoint(nIndex));
                            nIndex++;
                        }
                    }
                    if (!visitedItems.contains(newN->getNeighbour(x)) && !itemsToVisit.contains(newN->getNeighbour(x)) && newN->getNeighbour(x)!=NULL)
                        itemsToVisit.push_front(newN->getNeighbour(x));
                    x++;
                }
                itemsToVisit.removeAll(currentItem);
                currentItem = newN;
            }
            //now the "i" variable contains the index of the first point of connection
            else if (currentItem->getSlotTrackInfo()!=NULL && currentItem->getType()!=J1 && currentItem->getType()!=J2
                    && ((currentItem->getRadius()>0 && previousItem->getRadius()<0  && i%2==1)
                        || (currentItem->getRadius()<0 && previousItem->getRadius()>0 && i%2==1)
                        || (currentItem->getRadius()>0 && previousItem->getRadius()>0 && i%2==0)
                        || (currentItem->getRadius()<0 && previousItem->getRadius()<0  && i%2==0)
                        )
                    )
            {
                //get appropriate lane for connection - it is lane at point i if there is just one point of connection, otherwise search for the last lane of connection
                int l = i;
                bool incremented = false;
                while (previousItem->getEndPoint(l)!=NULL)
                {
                    if (previousItem->getNeighbour(l)!=currentItem)
                    {
                        //don't do it in case when there is just one-lane connection
                        if (incremented)
                            l-=2;
                        break;
                    }
                    l+=2;
                    incremented = true;
                }
                //if this condition is satisfied it means that previousItem is connected with its last lane too, so it needs to be decreased
                if (previousItem->getEndPoint(l)==NULL)
                    l-=2;
                *currentPoint = *previousItem->getEndPoint(l);
                //when there is 4-lane straight and double 2 lane curve you get error,
                //because bottom straight part will be connected by its [0] point instead of [4]
                if (pointsAreCloseEnough(currentItem->getEndPoint(0),currentPoint,currentItem->getProdLine()->getScaleEnum()/4.0))
                {
                    itemsToVisit.push_front(currentItem);
                    makeFirst =true;
                    continue;
                }
                fragment->addFragmentItem(currentItem,currentPoint);
            }
            else
            {
                *currentPoint = *previousItem->getEndPoint(i);
                if (currentItem->getType()==J2 && currentItem->getSlotTrackInfo()!=NULL && currentItem->getRadius()<0)
                    *currentPoint = *currentItem->getEndPoint(j%2);

                if (previousItem->getType()==X1 && previousItem->getSlotTrackInfo()!=NULL)
                    *currentPoint=*currentItem->getEndPoint(j%2);

                if (pointsAreCloseEnough(currentItem->getEndPoint(0),currentPoint,currentItem->getProdLine()->getScaleEnum()/4.0))
                {
                    itemsToVisit.push_front(currentItem);
                    makeFirst =true;
                    continue;
                }
                fragment->addFragmentItem(currentItem,currentPoint);
            }
        }
        //check the next item:
            //for each neighbour N:
                //if N is at odd point && N is connected to currentItem by odd point
                    //disconnect N from fragment
                    //add all points of currentItem which were used for connection with N
                    //create new item which will replace N
                    //currentItem = newN
                    //currentPoint = apropriate point of newN (lanes etc.)
                    //note: neighbours of the new item should be set properly in addFragmentItem()
                //else
                    //just queue the neighbour
        int i = 0;
        while (currentItem->getEndPoint(i)!=NULL && !visitedItems.contains(currentItem))
        {
            if (currentItem->getNeighbour(i)!=NULL)
            {
                ModelItem * n = currentItem->getNeighbour(i);
                int j = 0;
                while (n->getEndPoint(j)!=NULL)
                {
                    if (n->getNeighbour(j)==currentItem)
                        break;
                    j++;
                }
                if (!visitedItems.contains(n) && !itemsToVisit.contains(n))
                    itemsToVisit.push_front(n);
            }
            i++;
        }

        if (!visitedItems.contains(currentItem))
            visitedItems.push_back(currentItem);
        previousItem = currentItem;
    }    
    logFile << "    Printing all angles after rebuildFragment():" << endl;
    ModelItem * m = fragment->getFragmentItems()->first();
    for (int j = 0; j < fragment->getFragmentItems()->count();j++)
    {
        m = fragment->getFragmentItems()->at(j);
        logFile << "        Item: " << m << endl;
        int i = 0;
        while (m->getEndPoint(i)!=NULL)
        {
            logFile << "            [" << i << "]: " << m->getTurnAngle(i) << " neighbour is " << m->getNeighbour(i) << endl;
            i++;
        }
    }
}
