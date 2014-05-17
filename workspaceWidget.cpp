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

#include <cmath>
#include <QErrorMessage>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTime>
#include "workspaceWidget.h"
#include "globalVariables.h"
#include "mathFunctions.h"
#include "bezier.h"

WorkspaceWidget::WorkspaceWidget(QMenu * context, QWidget * parent) : QGraphicsView(parent)//QScrollArea(parent)
{
    this->deletePress=false;
    this->rotationMode=false;
    this->heightProfileMode=false;
    this->selectTwoPointsBend=false;
    this->selectTwoPointsComplete=false;

    this->selection= (new QList<ModelItem*>());
    this->selectionVegetation= new QList <VegetationItem*>();
    this->copiedItems = new QList<QString>();
    this->contextMenu = context;

    this->modelFragments = (new QList<ModelFragment*>());
    this->vegetationItems = new QList<VegetationItem*>();
    this->graphicsScene = new GraphicsScene(this->contextMenu,this->contextMenu,-2000,-1500,4000,3000);
    this->setRenderHint(QPainter::Antialiasing);
    this->setAlignment(Qt::AlignTop);
    this->setAlignment(Qt::AlignLeft);

    this->setScene(this->graphicsScene);

    this->mousePress=false;
    this->ctrlPress=false;
    this->shiftPress=false;
    this->lastUsedPart=NULL;
    this->lastInserted=NULL;
    this->lastEventPos=NULL;
    this->activeEndPoint = NULL;
    this->activeFragment = NULL;
    this->activeItem = NULL;
    this->indexUndoRedo=-1;
    this->doNotPrintSetEndPoint=true;
    this->unsavedChanges=false;

    this->activeEndPointGraphic=NULL;
    QPointF pt;
    this->setActiveEndPoint(&pt);

    this->doNotPrintSetEndPoint=false;

    QPalette pal = this->palette();
    this->setPalette(pal);
    this->setAutoFillBackground(true);

    QBrush b;
    b.setColor(Qt::white);

    b.setStyle(Qt::SolidPattern);
    this->graphicsScene->setBackgroundBrush(b);

    b.setColor(Qt::red);

    QPen pen(Qt::green);
    pen.setWidth(5);

    this->undoRedoCalled=false;
    this->eraseFollowing=true;
    this->nextIDToUse=0;

    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

WorkspaceWidget::~WorkspaceWidget()
{
    delete this->activeEndPoint;
    delete this->activeEndPointPrev;
    delete this->lastEventPos;
    delete this->modelFragments;
    delete this->vegetationItems;
    delete this->selection;
    delete this->selectionVegetation;
    delete this->copiedItems;
}

int WorkspaceWidget::commandExecution(QString command)
{

    logFile << "executing command: " << command.toStdString() << endl;

    //insert endpoint [x,y] with angle alpha into active fragment:
        //null neighbour alpha x.x y.y
        //null neighbour2 alpha x.x y.y
    //set active endpoint
        //make point x.x y.y
    //insert L/R item partNo at position [x,y]
        //make item partNo prodLineName L x.x y.y
    //insert vegetation item at position
        //make vegetation partNo prodLineName x.x y.y
    //set active endPoint
        //select point x y
    //select item scenePosX scenePosY
    //select vegetation x.x y.y
    //move fragment ID dx dy
    //move vegetation x.x y.y
    //rotate fragment ID cx.cx cy.cy alpha
    //rotate vegetation x.x y.y cx.x cy.y alpha
    //adjust height of point [x,y] by dz. this call does affect one item and one point
        //height point dz x.x y.y scenePos.x scenePos.y
    //delete item ID x.x y.y
    //delete vegetation x.x y.y
    //connect fragments A and B at point [xA,yA] and [xB,yB]
        //connect xA yA xB yB IDA IDB
    //disconnect IDC pointAt IDA IDB
    //bend close ID pt1X pt1Y pt2X pt2Y
    //bend open ID pt1X pt1Y pt2X pt2Y

    if (this->eraseFollowing && this->indexUndoRedo!=(this->actionListRedo.count()-1))
    {
        QList<QString>::Iterator it = this->actionListRedo.begin();
        QList<QString>::Iterator it2 = this->actionListUndo.begin();

        for (int i = 0; i < this->indexUndoRedo; i++)
        {it++;it2++;}
        this->actionListRedo.erase(it,this->actionListRedo.end());
        this->actionListUndo.erase(it2,this->actionListUndo.end());
    }
    if (command.startsWith("null neighbour"))
    {
        bool item = false;
        if (command.startsWith("null neighbour2"))
            item = true;
        //null neighbour angle x y
        if (item)
            command.replace("null neighbour2","");
        else
            command.replace("null neighbour","");
        command = command.trimmed();
        qreal angle = 0;
        qreal x = 0;
        qreal y = 0;

        angle = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        x = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y = command.toDouble();

        if (this->activeFragment==NULL)
        {
            logFile << "ERROR: null active fragment" << endl;
            return -1;
        }

        if (item)
        {
            this->activeFragment->addEndPoint(new QPointF(x,y),true,angle,this->activeFragment->getFragmentItems()->last());
            this->activeFragment->getFragmentItems()->last()->setEndPointAngle(0,angle);
        }
        else
            this->activeFragment->addEndPoint(new QPointF(x,y),true,angle);
    }
    else if (command.startsWith("make"))
    {
        command.remove(0,4);
        //command.remove(" ");
        command = command.trimmed();

        //make last
        if (command.startsWith("last"))
        {
            //this->makeLastItem(); //replaced by make item ...
        }
        //make point x.x y.y
        else if (command.startsWith("point"))
        {
            qreal x = 0;
            qreal y = 0;
            command.remove(0,5);
            command = command.trimmed();

            x = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            y = command.toDouble();

            QPointF * newPoint = new QPointF(x,y);
            app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
            app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);
            delete newPoint;

        }
        //make item partNo prodLineName L x.x y.y [id] where (x,y) is the point at which point[0] will be connected
        //if item creates new fragment the command has one additional argument - id of fragment which is created
        else if (command.startsWith("item"))
        {
            command.remove(0,4);
            command = command.trimmed();
            QString partNo = command.left(command.indexOf(" "));
            command.remove(0,partNo.length());
            command = command.trimmed();

            //prodLineName may contain whitespaces -> find lastIndexOf L and R characters -> if (char at (found index+2)==number) then you can use the index-1 as end character of prodLineName
            int lastL = command.lastIndexOf("L");
            int lastR = command.lastIndexOf("R");
            int endIndex = 0;
            if (lastL!=-1 && lastL+2<command.length() && (command.at(lastL+2).isDigit() || command.at(lastL+2)==QChar('-')))
                endIndex=lastL-1;
            if (lastR!=-1 && lastR+2<command.length() && (command.at(lastR+2).isDigit() || command.at(lastR+2)==QChar('-')))
                endIndex=lastR-1;

            QString prodLineName = command.left(endIndex);
            command.remove(0,prodLineName.length());
            command = command.trimmed();
            bool left = false;
            if (command.startsWith("L"))
                left = true;

            command.remove(0,1);
            command = command.trimmed();

            qreal x = 0;
            qreal y = 0;

            x = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();


            int id = -54321;
            if (command.contains(" "))
            {
                y = (command.left(command.indexOf(" "))).toDouble();
                command = command.remove(0,command.indexOf(" ")).trimmed();
                id = command.toDouble();
            }
            else
                y = command.toDouble();

            QPointF * newPoint = new QPointF(x,y);

            ModelItem * item = app->getAppData()->getDatabase()->findModelItemByName(prodLineName,partNo);

            if (item==NULL)
            {
                delete newPoint;
                return -1;
            }

            this->makeItem(item,newPoint,left);

            if (id!=-54321 && !command.contains(" "))
                this->activeFragment->setID(id);

        }
        //make border partNo prodLineName I x.x y.y
        else if (command.startsWith("border"))
        {
            command.remove(0,6);
            command = command.trimmed();
            QString partNo = command.left(command.indexOf(" "));
            command.remove(0,partNo.length());
            command = command.trimmed();

            qreal x = 0;
            qreal y = 0;

            y = (command.right(command.length()-command.lastIndexOf(" "))).toDouble();
            command = command.remove(command.lastIndexOf(" "),15).trimmed();
            x = (command.right(command.length()-command.lastIndexOf(" "))).toDouble();
            command = command.remove(command.lastIndexOf(" "),15).trimmed();
            command.remove(command.length()-1,1);
            command = command.trimmed();

            QString prodLineName = command;
            command.remove(0,prodLineName.length());
            command = command.trimmed();

            QPointF * newPoint = new QPointF(x,y);

            BorderItem * item = app->getAppData()->getDatabase()->findBorderItemByName(prodLineName,partNo);

            ModelItem * modelItem = NULL;

            GraphicsPathItemModelItem * gpi = NULL;
            QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(x-TOLERANCE_HALF,y-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF),Qt::IntersectsItemShape,Qt::AscendingOrder);
            for (int i = 0; i < gList.count();i++)
            {
                QGraphicsItem * qgpi = gList.at(i);

                if (qgpi!=NULL)
                {
                    if (qgpi->type()!=QGraphicsItem::UserType+2)// || qgpi->type()!=QGraphicsItem::UserType+3)
                        continue;
                }
                else
                    continue;

                if (qgpi->type()==QGraphicsItem::UserType+2)
                {
                    gpi = (GraphicsPathItemModelItem*)qgpi;
                    modelItem = gpi->getParentItem();
                    SlotTrackInfo * sti = modelItem->getSlotTrackInfo();
                    for (int i = 0; i < sti->getBorderEndPoints()->count(); i++)
                    {
                        if (pointsAreCloseEnough(sti->getBorderEndPoints()->at(i),newPoint,modelItem->getProdLine()->getScaleEnum()/4.0))
                        {
                            break;
                        }
                    }
                }
                else
                    continue;
            }

            if (item==NULL || modelItem==NULL)
                return -1;

            this->selectItem(modelItem);
            this->setActiveFragment(modelItem->getParentFragment());
            this->setActiveEndPoint(newPoint);
            this->makeBorder(item);
        }
        //make vegetation partNo prodLineName x.x y.y
        else if (command.startsWith("vegetation"))
        {
            command.remove(0,10);
            command = command.trimmed();
            QString partNo = command.left(command.indexOf(" "));
            command.remove(0,partNo.length());
            command = command.trimmed();

            qreal x = 0;
            qreal y = 0;

            y = (command.right(command.length()-command.lastIndexOf(" "))).toDouble();
            command = command.remove(command.lastIndexOf(" "),15).trimmed();
            x = (command.right(command.length()-command.lastIndexOf(" "))).toDouble();

            QString prodLineName = command.remove(command.lastIndexOf(" "),15).trimmed();
            command.remove(0,prodLineName.length());
            command = command.trimmed();

            QPointF * newPoint = new QPointF(x,y);

            VegetationItem * item = app->getAppData()->getDatabase()->findVegetationItemByName(prodLineName,partNo);
            if (item==NULL)
                return -1;
            this->setActiveEndPoint(newPoint);
            this->makeVegetation(item);
        }
        else
        {
            logFile << "ERROR: Invalid command: " << command.toStdString() << endl;
        }
    }
    else if (command.startsWith("select"))
    {
        command.remove(0,6);
        command = command.trimmed();

        //select point x y
        if (command.startsWith("point"))
        {
            qreal x = 0;
            qreal y = 0;
            command.remove(0,5);
            command = command.trimmed();

            x = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            y = command.toDouble();

            QPointF * pos = new QPointF(x,y);
            ModelFragment* frag = app->getWindow()->getWorkspaceWidget()->findFragmentByApproxPos(pos);

            if (frag!=NULL)
            {
                app->getWindow()->getWorkspaceWidget()->setActiveFragment(frag);
                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pos);
                this->selectItem(frag->findEndPointItem(pos));
                app->getWindow()->getWorkspaceWidget()->setActiveItem(NULL);
            }
            else
            {
                logFile << "null frag variable 1" << endl;
                delete pos;
            }
        }
        //select item scenePosX scenePosY
        else if (command.startsWith("item"))
        {
            command.remove(0,4);
            command = command.trimmed();

            qreal x = 0;
            qreal y = 0;
            command = command.trimmed();

            x = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            y = command.toDouble();

            GraphicsPathItemModelItem * gpi = NULL;
            QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(x-TOLERANCE_HALF,y-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
            for (int i = 0; i < gList.count();i++)
            {
                //GraphicsPathItem * gpi = NULL;
                QPointF sPos(gList.at(i)->scenePos());
                QPointF pos2(x,y);
                if (pointsAreCloseEnough(&sPos,&pos2,TOLERANCE_HALF))
                {
                    QGraphicsItem * qgpi = gList.at(i);

                    if (qgpi!=NULL)
                    {
                        if (qgpi->type()!=QGraphicsItem::UserType+2)
                        {
                            continue;
                        }
                    }
                    else
                        continue;

                    gpi = (GraphicsPathItemModelItem*)qgpi;

                }
            }

            if (gpi==NULL)
            {
                logFile << "ERROR: item was not found" << endl;
                return 1;
            }
            this->selectItem(gpi->getParentItem());
        }
        else if (command.startsWith("fragment"))
        {
            command.remove(0,8);
            command = command.trimmed();

            int id = -1;

            id = command.toInt();
            ModelFragment * f = this->findFragmentByID(id);
            if (f==NULL)
            {
                logFile << "ERROR: fragment was not found" << endl;
            }
            this->setActiveFragment(f);

        }
        else if (command.startsWith("vegetation"))
        {
            command.remove(0,10);
            command = command.trimmed();

            qreal x = 0;
            qreal y = 0;
            command = command.trimmed();

            x = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            y = command.toDouble();

            GraphicsPathItemVegetationItem * gpi = NULL;
            QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(x-TOLERANCE_HALF,y-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
            for (int i = 0; i < gList.count();i++)
            {
                QPointF sPos(gList.at(i)->scenePos());
                QPointF pos2(x,y);
                if (pointsAreCloseEnough(&sPos,&pos2,TOLERANCE_HALF))
                {
                    QGraphicsItem * qgpi = gList.at(i);

                    if (qgpi!=NULL)
                    {
                        if (qgpi->type()!=QGraphicsItem::UserType+4)
                        {
                            continue;
                        }
                    }
                    else
                        continue;
                    gpi = (GraphicsPathItemVegetationItem*)qgpi;
                }
            }
            if (gpi==NULL)
            {
                logFile << "ERROR: item was not found" << endl;
                return 1;
            }
            this->selectItem(gpi->getParentItem());
        }
    }
    else if (command.startsWith("move"))
    {
        //move fragment ID dx dy
        command.remove(0,4);
        command = command.trimmed();
        if (!command.startsWith("fragment") && !command.startsWith("vegetation"))
        {
            logFile << "ERROR: Invalid command: " << command.toStdString() << endl;
        }
        if (command.startsWith("fragment"))
        {
            command.remove(0,8);

            qreal id = 0;
            command = command.trimmed();
            id = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();

            qreal dx = 0;
            qreal dy = 0;
            command = command.trimmed();

            dx = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            dy = command.toDouble();

            ModelFragment * f = this->findFragmentByID(id);
            if (f!=NULL)
            {
                f->moveBy(dx,dy);
            }
            else
            {
                logFile << "ERROR: fragment not found by the id: " << id << endl;
                return 1;
            }
        }
        else
        {
            command.remove(0,10);

            qreal x = 0;
            qreal y = 0;
            qreal dx = 0;
            qreal dy = 0;
            command = command.trimmed();

            x = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            y = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            dx = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            dy = command.toDouble();

            GraphicsPathItemVegetationItem * gpi = NULL;
            QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(x-TOLERANCE_HALF,y-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
            for (int i = 0; i < gList.count();i++)
            {
                QPointF sPos(gList.at(i)->scenePos());
                QPointF pos2(x,y);
                if (pointsAreCloseEnough(&sPos,&pos2,TOLERANCE_HALF))
                {
                    QGraphicsItem * qgpi = gList.at(i);
                    if (qgpi!=NULL)
                    {
                        if (qgpi->type()!=QGraphicsItem::UserType+4)
                        {
                            continue;
                        }
                    }
                    else
                        continue;
                    gpi = (GraphicsPathItemVegetationItem*)qgpi;
                }
            }
            if (gpi==NULL)
            {
                logFile << "ERROR: item was not found" << endl;
                return 1;
            }
            gpi->getParentItem()->moveBy(dx,dy);
        }
    }
    else if (command.startsWith("rotate"))
    {
        //rotate item posX posY cx cy alpha
        //rotate fragment ID cx.cx cy.cy alpha
        command.remove(0,6);
        command = command.trimmed();
        if (!(command.startsWith("fragment") || command.startsWith("item") || command.startsWith("vegetation")))
        {
            logFile << "Invalid command: " << command.toStdString() << endl;
        }
        bool fragMode = true;
        if (command.startsWith("item") || command.startsWith("vegetation"))

            fragMode = false;

        //fragment id
        qreal id = 0;
        //item position
        qreal pX = 0;
        qreal pY = 0;

        bool startsWithItem=false;
        if (fragMode)
        {
            command.remove(0,8);
            command = command.trimmed();

            id = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
        }
        else
        {
            if (command.startsWith("item"))
            {
                startsWithItem=true;
                command.remove(0,4);
            }
            else
                command.remove(0,10);
            command = command.trimmed();

            pX = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            pY = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
        }

        qreal cx = 0;
        qreal cy = 0;

        cx = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        cy = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        qreal alpha = 0;
        command = command.trimmed();
        alpha = command.toDouble();

        if (fragMode)
        {
            ModelFragment* frag = this->findFragmentByID(id);
            if (frag==NULL)
            {
                logFile<<"ERROR: fragment was not found by id: " << id << endl;
                return 1;
            }

            QPointF pos;
            pos.setX(cx);
            pos.setY(cy);
            frag->rotate(alpha,&pos);
        }
        else if (startsWithItem)
        {
            QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(pX-TOLERANCE_HALF,pY-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
            for (int i = 0; i < gList.count();i++)
            {
                GraphicsPathItemModelItem * gpi = NULL;
                QPointF sPos(gList.at(i)->scenePos());
                QPointF pos2(pX,pY);
                if (pointsAreCloseEnough(&sPos,&pos2,TOLERANCE_HALF))
                {
                    QGraphicsItem * qgpi = gList.at(i);

                    if (qgpi!=NULL)
                    {
                        if (qgpi->type()!=QGraphicsItem::UserType+2)
                        {
                            continue;
                        }
                    }
                    else
                        continue;

                    QPointF pos;
                    pos.setX(cx);
                    pos.setY(cy);
                    gpi = (GraphicsPathItemModelItem*)qgpi;

                    logFile << "    found item with scenePos " << sPos.x() << ", " << sPos.y() << endl;

                    gpi->getParentItem()->rotate(alpha,&pos);
                    ModelItem * it = gpi->getParentItem();

                    int xyz = 0;
                    while (it->getEndPoint(xyz)!=NULL)
                    {
                        if (it->getNeighbour(xyz)==NULL)
                            it->getParentFragment()->setEndPointAngle(it->getEndPoint(xyz),it->getTurnAngle(xyz));
                        xyz++;
                    }
                }
            }
        }
        else
        {
            GraphicsPathItemVegetationItem * gpi = NULL;
            QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(pX-TOLERANCE_HALF,pY-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
            for (int i = 0; i < gList.count();i++)
            {
                QPointF sPos(gList.at(i)->scenePos());
                QPointF pos2(pX,pY);
                if (pointsAreCloseEnough(&sPos,&pos2,TOLERANCE_HALF))
                {
                    QGraphicsItem * qgpi = gList.at(i);

                    if (qgpi!=NULL)
                    {
                        if (qgpi->type()!=QGraphicsItem::UserType+4)
                        {
                            continue;
                        }
                    }
                    else
                        continue;

                    gpi = (GraphicsPathItemVegetationItem*)qgpi;

                }
            }

            if (gpi==NULL)
            {
                logFile << "ERROR: item was not found" << endl;
                return 1;
            }
            if ((int)cx==(int)cy && (int)cy==(int)alpha)
            {
                cx=pX;
                cy=pY;
            }
            QPointF center(cx,cy);            
            gpi->getParentItem()->rotate(alpha,&center);
        }
    }
    else if (command.startsWith("height"))
    {
        //point/item/latAngle/longAngle dz x.x y.y scenePos.x scenePos.y
        //x.x y.y = point at which the height will be adjusted
        //x.2 y.2 = scenePos() of item which owns the point
        command.remove(0,6);
        command = command.trimmed();

        int type =-1; //0=point,2=latAngle,3=longAngle - 2 and 3 are not used

        if (command.startsWith("point"))
        {
            command.remove(0,5);
            command = command.trimmed();
            type = 0;
        }

        qreal dz = 0;
        qreal x = 0;
        qreal y = 0;
        qreal x2 = 0;
        qreal y2 = 0;

        dz = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        x = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        x2 = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y2 = command.toDouble();

        QPointF * pos = new QPointF(x,y);

        ModelItem * mi = NULL;

        QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(x2-TOLERANCE_HALF,y2-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
        for (int i = 0; i < gList.count();i++)
        {
            QPointF sPos(gList.at(i)->scenePos());
            QPointF pos2(x2,y2);
            if (pointsAreCloseEnough(&sPos,&pos2,TOLERANCE_HALF))
            {
                QGraphicsItem * qgpi = gList.at(i);

                if (qgpi!=NULL)
                {
                    if (qgpi->type()!=QGraphicsItem::UserType+2)
                    {
                        continue;
                    }
                }
                else
                    continue;
                mi = ((GraphicsPathItemModelItem*)qgpi)->getParentItem();
            }
        }

        if (mi==NULL)
        {
            delete pos;
            logFile << "height null gpi" << endl;
            return 1;
        }

        if (mi!=NULL)
        {
            if (type==0)
                mi->adjustHeightProfile(dz,pos,true,true);
            else
            {
                logFile << "Invalid command: " << command.toStdString() << endl;
                return 1;
            }
            delete pos;
        }
        else
            delete pos;
    }
    else if (command.startsWith("delete"))
    {
        //delete item id x.x y.y [idList] where id is parentFragment id and [idList] is a sequence of ids separated by whitespaces
        //delete fragment id
        //delete border id x.x y.y where id is parentFragment id and x.x y.y is scene position of the border
        command.remove(0,6);
        command = command.trimmed();

        int type = -1; //0=item, 1=fragment, 2=border, 3=vegetation
        if (command.startsWith("item"))
        {
            command.remove(0,4);
            command = command.trimmed();
            type = 0;
        }
        else if (command.startsWith("fragment"))
        {
            command.remove(0,8);
            command = command.trimmed();
            type = 1;
        }
        else if (command.startsWith("border"))
        {
            command.remove(0,6);
            command = command.trimmed();
            type = 2;
        }
        else if (command.startsWith("vegetation"))
        {
            command.remove(0,10);
            command = command.trimmed();
            type = 3;
        }

        qreal id = 0;
        if (type!=3)
        {
            id = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();

            ModelFragment * f = this->findFragmentByID(id);
            if (f==NULL)
            {
                logFile<<"ERROR: fragment was not found by id: " << id << endl;
                return 1;
            }
            if (type==1)
            {
                this->removeFragment(f);
                return 0;
            }
        }

        qreal x = 0;
        qreal y = 0;

        x = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        QList<int> idList;
        if (type!=3)
        {
            int firstId = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
            idList.push_back(firstId);
            if (firstId!=(int)y) //if it is equal, there is no id as a parameter
            {
                int infinity = 0;
                while (infinity < 64)
                {
                    int temp = (command.left(command.indexOf(" "))).toDouble();
                    command = command.remove(0,command.indexOf(" ")).trimmed();
                    if (temp==idList.last())
                        break;
                    idList.push_back(temp);
                    infinity++;
                }
            }
        }

        GraphicsPathItemVegetationItem * gpiVI = NULL;
        GraphicsPathItemBorderItem * gpiBI = NULL;
        GraphicsPathItemModelItem * gpi = NULL;

        QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(x-TOLERANCE_HALF,y-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF),Qt::IntersectsItemShape,Qt::AscendingOrder);
        if (type==0)
        {
            for (int i = 0; i < gList.count();i++)
            {
                //GraphicsPathItem * gpi = NULL;
                QPointF sPos(gList.at(i)->scenePos());
                QPointF pos2(x,y);
                QGraphicsItem * qgpi = gList.at(i);

                if (pointsAreCloseEnough(&sPos,&pos2,TOLERANCE_HALF))
                {
                    if (qgpi!=NULL)
                    {
                        if (qgpi->type()!=QGraphicsItem::UserType+2)
                            continue;
                    }
                    else
                        continue;

                    if (qgpi->type()==QGraphicsItem::UserType+2)
                        gpi = (GraphicsPathItemModelItem*)qgpi;
                    else
                        continue;
                }
            }
        }
        else if (type==2)
        {
            for (int i = 0; i < gList.count();i++)
            {
                QGraphicsItem * qgpi = gList.at(i);

                if (qgpi!=NULL)
                {
                    if (qgpi->type()!=QGraphicsItem::UserType+3)
                        continue;
                }
                else
                    continue;

                if (qgpi->type()==QGraphicsItem::UserType+3)
                    gpiBI = (GraphicsPathItemBorderItem*)qgpi;
                else
                    continue;
            }
        }
        else
        {
            for (int i = 0; i < gList.count();i++)
            {
                QGraphicsItem * qgpi = gList.at(i);

                if (qgpi!=NULL)
                {
                    if (qgpi->type()!=QGraphicsItem::UserType+4)
                        continue;
                }
                else
                    continue;

                if (qgpi->type()==QGraphicsItem::UserType+4)
                    gpiVI = (GraphicsPathItemVegetationItem*)qgpi;
                else
                    continue;
            }
        }

        if (gpi==NULL && type==0)
        {
            logFile << "    ERROR: delete item command: null gpi" << endl;
            return 1;
        }
        if (gpiBI==NULL && type==2)
        {
            logFile << "    ERROR: delete border command: null gpi" << endl;
            return 1;
        }
        if (gpiVI==NULL && type==3)
        {
            logFile << "    ERROR: delete vegetation command: null gpi" << endl;
            return 1;
        }
        {
            if (type==0)
            {
                ModelItem * mi = gpi->getParentItem();
                if (idList.count()!=0)
                    this->removeItem(mi,&idList);
                else
                    this->removeItem(mi);

            }
            else if (type==2)
            {
                BorderItem * bi = gpiBI->getParentItem();
                bi->getNeighbour(0)->getSlotTrackInfo()->removeBorder(bi);
            }
            else
            {
                VegetationItem * vi = gpiVI->getParentItem();
                this->removeVegetation(vi);
            }
        }
    }
    else if (command.startsWith("connect"))
    {

        //connect xA yA xB yB idA idB
        command.remove(0,7);
        command = command.trimmed();

        //points
        qreal xA = 0;
        qreal yA = 0;
        qreal xB = 0;
        qreal yB = 0;
        int idA = 0;
        int idB = 0;
        int idC = 0;

        xA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        xB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        idA = (command.left(command.indexOf(" "))).toInt();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        idB = (command.left(command.indexOf(" "))).toInt();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        idC = (command.left(command.indexOf(" "))).toInt();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        QPointF * posA = new QPointF(xA,yA);
        QPointF * posB = new QPointF(xB,yB);

        ModelFragment * fA = this->findFragmentByID(idA);
        ModelFragment * fB = this->findFragmentByID(idB);

        if (fA==NULL || fB==NULL)
        {
            logFile << "ERROR: fragment not found - idA " << idA << ", idB " << idB << endl;
            return 1;
        }

        ModelItem * itemA = fA->findEndPointItem(posA);
        ModelItem * itemB = fB->findEndPointItem(posB);

        if (itemA==NULL || itemB==NULL)
        {
            logFile << "ERROR: item was not found - idA " << idA << ", idB " << idB << endl;
            return 1;
        }

        this->connectFragments(posA,posB,fA,fB,itemA,itemB,idC);
    }
    else if (command.startsWith("disconnect"))
    {
        //disconnect idC pointAt idA idB
        command.remove(0,10);
        command = command.trimmed();

        //points
        qreal idC = 0;
        qreal x = 0;
        qreal y = 0;
        int idA = 0;
        int idB = 0;

        idC = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        x = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        idA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        idB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        ModelFragment * c = this->findFragmentByID(idC);
        if (c==NULL)
        {
            logFile << "ERROR: fragment was not found - idC " << idC << endl;
            return 1;
        }
        QPointF pos(x,y);
        this->disconnectFragment(c,&pos,idA, idB);
    }
    else if (command.startsWith("bend"))
    {
        //bend close ID pt1X pt1Y pt2X pt2Y
        //bend open ID pt1X pt1Y pt2X pt2Y
        //bend2 close IDa IDb pt1X pt1Y pt2X pt2Y
        //bend2 open IDa IDb pt1X pt1Y pt2X pt2Y

        bool twoFragments = false;
        if (command.startsWith("bend2"))
        {
            twoFragments=true;
            command.remove(0,5);
            command = command.trimmed();
        }
        else
        {
            command.remove(0,4);
            command = command.trimmed();
        }

        bool close = true;
        if (command.startsWith("close"))
            command = command.remove(0,5).trimmed();
        else
        {
            close = false;
            command = command.remove(0,4).trimmed();
        }

        int id = 0;
        command = command.trimmed();
        id = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        int id2 = -15;
        if (twoFragments)
        {
            command = command.trimmed();
            id2 = (command.left(command.indexOf(" "))).toDouble();
            command = command.remove(0,command.indexOf(" ")).trimmed();
        }

        qreal x1 = 0;
        qreal y1 = 0;
        qreal x2 = 0;
        qreal y2 = 0;


        x1 = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y1 = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        x2 = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y2 = command.toDouble();

        QPointF pt1 (x1,y1);
        QPointF pt2 (x2,y2);

        ModelFragment * frag = this->findFragmentByID(id);
        ModelFragment * fragB = this->findFragmentByID(id2);
        if (frag!=NULL)
        {
            if (close)
            {
                if (twoFragments)
                {
                    if (fragB==NULL)
                    {
                        logFile << "ERROR: fragB is null, id2==" << id2 << endl;
                        return 1;
                    }
                    this->bendAndClose(frag,fragB,&pt1,&pt2);
                }
                else
                    this->bendAndClose(frag,&pt1,&pt2);
            }
            else
            {
                ModelItem * it1 = NULL;
                ModelItem * it2 = NULL;
                QPointF * ptPtr1 = NULL;
                QPointF * ptPtr2 = NULL;

                for (int i = 0; i < frag->getFragmentItems()->count(); i++)
                {
                    int j = 0;
                    while (frag->getFragmentItems()->at(i)->getEndPoint(j)!=NULL)
                    {
                        if (pointsAreCloseEnough(&pt1,frag->getFragmentItems()->at(i)->getEndPoint(j),frag->getProductLines()->first()->getScaleEnum()/4.0))
                        {
                            ptPtr1 = frag->getFragmentItems()->at(i)->getEndPoint(j);
                            it1 = frag->getFragmentItems()->at(i);
                        }
                        if (pointsAreCloseEnough(&pt2,frag->getFragmentItems()->at(i)->getEndPoint(j),frag->getProductLines()->first()->getScaleEnum()/4.0))
                        {
                            ptPtr2 = frag->getFragmentItems()->at(i)->getEndPoint(j);
                            it2 = frag->getFragmentItems()->at(i);
                        }
                        j++;
                    }
                }

                if (!twoFragments)
                {
                    if (it1==NULL || it2==NULL)
                    {
                        logFile << "ERROR: it1=" << it1 << ", it2=" << it2 << ", ptPtr1=(" << ptPtr1->x() << ", ";
                        logFile << ptPtr1->y() << "), ptPtr2=(" << ptPtr2->x() << ", " << ptPtr2->y() << ")" << endl;
                        return 1;
                    }
                    frag->addEndPoint(ptPtr1,true,it1->getTurnAngle(ptPtr1),it1);
                    frag->addEndPoint(ptPtr2,true,it2->getTurnAngle(ptPtr2),it2);
                    it1->setNeighbour(NULL,ptPtr1);
                    it2->setNeighbour(NULL,ptPtr2);
                }
                else
                {
                    if (fragB==NULL)
                    {
                        logFile << "ERROR: fragB is null, id2==" << id2 << endl;
                        return 1;
                    }
                    frag->updateEndPointsGraphics();
                    fragB->updateEndPointsGraphics();
                }
            }
        }
    }

    return 0;
}

int WorkspaceWidget::pushBackCommand(QString command,QString negCommand)
{
    if (!this->unsavedChanges)
        this->unsavedChanges=true;

    if (!app->getWindow()->windowTitle().endsWith("*"))
        app->getWindow()->setWindowTitle(app->getWindow()->windowTitle().append("*"));

    if (this->eraseFollowing && this->indexUndoRedo!=(this->actionListRedo.count()-1))
    {
        QList<QString>::Iterator it = this->actionListRedo.begin();
        QList<QString>::Iterator it2 = this->actionListUndo.begin();

        for (int i = 0; i <= this->indexUndoRedo; i++)
        {it++;it2++;}
        this->actionListRedo.erase(it,this->actionListRedo.end());
        this->actionListUndo.erase(it2,this->actionListUndo.end());
    }

    else if (!this->undoRedoCalled)
    {
        //if command starts with # it means it is not complete
        //if command starts with @ it means it replaces previous incomplete command of the same type
        ///NOTE: # and @ is using just (dis)connect so command type doesn't need to be checked
        if (command.startsWith("@"))
        {
            int i = this->actionListRedo.count()-1;
            for (; i >= 0; i--)
            {
                if (this->actionListRedo.at(i).startsWith("#"))
                    break;
            }
            command.remove("@");
            negCommand.remove("@");
            QString str = this->actionListRedo.at(i);
            QString str2 = this->actionListUndo.at(i);

            this->actionListRedo[i].replace(str,command);
            this->actionListUndo[i].replace(str2,negCommand);
        }

        this->actionListRedo.push_back(command);
        this->actionListUndo.push_back(negCommand);
        this->indexUndoRedo++;
    }
    return 0;
}

int WorkspaceWidget::makeLastItem()
{
    if (this->lastUsedPart==NULL)
        return -1;

    {
        if (this->activeFragment!=NULL)
        {
            for (int i = 0; i < this->activeFragment->getProductLines()->count(); i++)
            {
                if (this->activeFragment->getProductLines()->at(i)->getType()!=this->lastUsedPart->getProdLine()->getType())
                {
                    app->getAppData()->setMessageDialogText("You cannot connect rail and slot parts.","Autodráhové a železniční díly nelze spojovat");
                    app->getAppData()->getMessageDialog()->exec();
                    return 0;
                }

            }
            //if the border-point is current activeEndPoint
            if (this->lastUsedPart->getSlotTrackInfo()!=NULL)
            {
                if (this->activeFragment->findEndPointItem(this->activeEndPoint)==NULL)
                {
                    app->getAppData()->setMessageDialogText("Model part can't be inserted at this point.","Díl nemůže být vložen v tomto bodě.");
                    app->getAppData()->getMessageDialog()->exec();
                    return 0;
                }

            }
        }

        GraphicsPathItemModelItem * gpi = this->lastUsedPart->get2DModel();
        QPointF pt;
        if (this->lastUsedPart->getRadius()<0)
            pt = QPointF(gpi->scenePos().x()+1,gpi->scenePos().y()+1);
        else
            pt = QPointF(gpi->scenePos().x()+gpi->boundingRect().width()-2,gpi->scenePos().y()+gpi->boundingRect().height()-2);

        int fragCountBefore = this->modelFragments->count();

        if (this->canInsert(this->lastUsedPart))
            makeNewItem(*this->lastEventPos,gpi,this->lastUsedPart,this->lastUsedPart, true);

        ModelItem * lastInserted = NULL;
        if (this->activeFragment!=NULL)
            lastInserted = this->activeFragment->getFragmentItems()->last();
        else
            lastInserted = this->activeFragmentPrev->getFragmentItems()->last();

        int idOfNew = -5;
        if (fragCountBefore!=this->modelFragments->count())
            idOfNew = this->activeFragment->getID();

        //print command for item inserting/deleting
        if (lastInserted->getRadius()<0)
        {
            QString str;
            if (idOfNew!=-5)
                str = QString("make item %1 %2 L %3 %4 %5").arg(*lastInserted->getPartNo(),*lastInserted->getProdLine()->getName(),QString::number(lastInserted->getEndPoint(0)->x()),QString::number(lastInserted->getEndPoint(0)->y()),QString::number(idOfNew));
            else
                str = QString("make item %1 %2 L %3 %4 ").arg(*lastInserted->getPartNo(),*lastInserted->getProdLine()->getName(),QString::number(lastInserted->getEndPoint(0)->x()),QString::number(lastInserted->getEndPoint(0)->y()));
            QString negStr = QString("delete item %1 %2 %3").arg(QString::number(lastInserted->getParentFragment()->getID()),QString::number(lastInserted->get2DModelNoText()->scenePos().x()),QString::number(lastInserted->get2DModelNoText()->scenePos().y()));

            this->pushBackCommand(str,negStr);
        }
        else
        {
            QString str;
            if (idOfNew!=-5)
                str = QString("make item %1 %2 R %3 %4 %5 ").arg(*lastInserted->getPartNo(),*lastInserted->getProdLine()->getName(),QString::number(lastInserted->getEndPoint(0)->x()),QString::number(lastInserted->getEndPoint(0)->y()),QString::number(idOfNew));
            else
                str = QString("make item %1 %2 R %3 %4 ").arg(*lastInserted->getPartNo(),*lastInserted->getProdLine()->getName(),QString::number(lastInserted->getEndPoint(0)->x()),QString::number(lastInserted->getEndPoint(0)->y()));
            QString negStr = QString("delete item %1 %2 %3").arg(QString::number(lastInserted->getParentFragment()->getID()),QString::number(lastInserted->get2DModelNoText()->scenePos().x()),QString::number(lastInserted->get2DModelNoText()->scenePos().y()));


            this->pushBackCommand(str,negStr);
        }
    }
    return 0;
}

void WorkspaceWidget::removeItems()
{
    if (this->heightProfileMode)
        return;

    this->deletePress = true;
    for (int i = 0; i < this->modelFragments->count(); i++)
    {
        for (int j = 0; j < this->modelFragments->at(i)->getFragmentItems()->count(); j++)
        {
            if (this->modelFragments->at(i)->getFragmentItems()->at(j)->get2DModelNoText()->isSelected() || this->selection->contains(this->modelFragments->at(i)->getFragmentItems()->at(j)))
            {
                if (this->modelFragments->at(i)->getFragmentItems()->at(j)->getSlotTrackInfo()!=NULL)
                {
                    SlotTrackInfo * sti = this->modelFragments->at(i)->getFragmentItems()->at(j)->getSlotTrackInfo();
                    for (int k = 0; k < sti->getBorders()->count(); k++)
                    {
                        if (sti->getBorders()->at(k)!=NULL)
                            sti->removeBorder(sti->getBorders()->at(k));
                    }
                }

                this->removeItem(this->modelFragments->at(i)->getFragmentItems()->at(j));
                i=-1;
                j=0;
                break;
            }
            else if (this->modelFragments->at(i)->getFragmentItems()->at(j)->getSlotTrackInfo()!=NULL)
            {
                SlotTrackInfo * sti = this->modelFragments->at(i)->getFragmentItems()->at(j)->getSlotTrackInfo();
                for (int k = 0; k < sti->getBorders()->count(); k++)
                {
                    if (sti->getBorders()->at(k)!=NULL && sti->getBorders()->at(k)->get2DModelNoText()->isSelected())
                        sti->removeBorder(sti->getBorders()->at(k));
                }
            }
        }
    }
    for (int i = 0; i < this->vegetationItems->count();i++)
    {
        if (this->selectionVegetation->contains(this->vegetationItems->at(i)))
        {
            this->removeVegetation(this->vegetationItems->at(i));
            i=0;
        }
    }

    this->deletePress=false;
}

void WorkspaceWidget::modelInfo()
{
    QTreeView * tree = app->getTreeView();
    QStandardItemModel * model = new QStandardItemModel();
    QStandardItem * parentItem = model->invisibleRootItem();
    for (int i = 0; i < this->modelFragments->count(); i++)
    {
        QMap<QString,int> itemsMap;
        QList<QString> details;
        QStandardItem *fragment = NULL;
        if (app->getUserPreferences()->getLocale()->startsWith("EN"))
            fragment = new QStandardItem(QString("Track section %0").arg(i+1));
        else
            fragment = new QStandardItem(QString("Traťový úsek %0").arg(i+1));
        for (int j = 0; j < this->modelFragments->at(i)->getFragmentItems()->count(); j++)
        {
            QString str;
            str.append(*this->modelFragments->at(i)->getFragmentItems()->at(j)->getPartNo());
            str.append(" (");
            str.append(*this->modelFragments->at(i)->getFragmentItems()->at(j)->getProdLine()->getName());
            str.append(")");

            itemsMap[str]++;

            QString strDetails;
            strDetails.append(*this->modelFragments->at(i)->getFragmentItems()->at(j)->getPartNo());
            strDetails.append(" ");

            if (app->getUserPreferences()->getLocale()->contains("EN"))
            {
                strDetails.append(this->modelFragments->at(i)->getFragmentItems()->at(j)->getNameEn()->mid(7,-1));
                {
                    ItemType t = this->modelFragments->at(i)->getFragmentItems()->at(j)->getType();
                    if (t==C1 || t==C2 || t==J1 || t==J2 || t==J3 || t==CB)
                    {
                        if (this->modelFragments->at(i)->getFragmentItems()->at(j)->leftRightDifference180(0,1))
                        {
                            strDetails.append(QString(", radius: %0, angle: %1 ").arg(
                                          QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getRadius())),
                                          QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(0)-180-this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(1)))));
                        }
                        else
                        {
                            strDetails.append(QString(", radius: %0, angle: %1 ").arg(
                                          QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getRadius())),
                                          QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(0)-this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(1)))));
                        }
                    }
                    else
                    {
                        if (this->modelFragments->at(i)->getFragmentItems()->at(j)->getSlotTrackInfo()==NULL)
                        {
                            strDetails.append(QString(", length: %0 ").arg(
                                      QString::number(2*this->modelFragments->at(i)->getFragmentItems()->at(j)->getRadius())));
                        }
                        else
                        {
                            strDetails.append(QString(", length: %0 ").arg(
                                      QString::number(2*this->modelFragments->at(i)->getFragmentItems()->at(j)->getSecondRadius()) ));
                        }
                    }
                }
            }
            else
            {
                strDetails.append(this->modelFragments->at(i)->getFragmentItems()->at(j)->getNameCs()->mid(7,-1));
                {
                    ItemType t = this->modelFragments->at(i)->getFragmentItems()->at(j)->getType();
                    if (t==C1 || t==C2 || t==J1 || t==J2 || t==J3 || t==CB)
                    {
                        if (this->modelFragments->at(i)->getFragmentItems()->at(j)->leftRightDifference180(0,1))
                        {
                            strDetails.append(QString(", poloměr: %0, úhel: %1 ").arg(
                                      QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getRadius())),
                                      QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(0)-180-this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(1)))));
                        }
                        else
                        {
                            strDetails.append(QString(", poloměr: %0, úhel: %1 ").arg(
                                      QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getRadius())),
                                      QString::number(abs(this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(0)-this->modelFragments->at(i)->getFragmentItems()->at(j)->getTurnAngle(1)))));
                        }
                    }
                    else
                    {
                        if (this->modelFragments->at(i)->getFragmentItems()->at(j)->getSlotTrackInfo()==NULL)
                        {
                            strDetails.append(QString(", délka: %0 ").arg(
                                      QString::number(2*this->modelFragments->at(i)->getFragmentItems()->at(j)->getRadius())));
                        }
                        else
                        {
                            strDetails.append(QString(", délka: %0 ").arg(
                                      QString::number(2*this->modelFragments->at(i)->getFragmentItems()->at(j)->getSecondRadius()) ));
                        }
                    }
                }
            }
            if (!details.contains(strDetails))
                details.append(strDetails);
        }
        //print both maps
        QMap<QString,int>::Iterator it = itemsMap.begin();
        for (int i = 0; i < itemsMap.count(); i++, it++)
        {
            QString val = QString::number(it.value());
            val.append("x ");
            val.append(it.key());
            //listWidgetI->addItem(val);
            QStandardItem *itemOverview = new QStandardItem(val);
            int j = 0;
            for (;j<details.count();j++)
            {
                if (details[j].startsWith(it.key().left(it.key().indexOf(" "))))
                    break;
            }
            QStandardItem *itemDetails = new QStandardItem(details[j]);
            itemOverview->appendRow(itemDetails);
            fragment->appendRow(itemOverview);
        }
        parentItem->appendRow(fragment);
    }

    tree->setModel(model);
    app->showModelInfo();
}

void WorkspaceWidget::exportBitmap()
{

    QString qpath;

    if (app->getUserPreferences()->getLocale()->contains("EN"))
    {
            qpath = QFileDialog::getSaveFileName(app->getWindow(), "Save file", "", "*.png");
    }
    else
    {
            qpath = QFileDialog::getSaveFileName(app->getWindow(), "Uložit soubor", "", "*.png");
    }
    if (!qpath.endsWith(".png"))
        qpath.append(".png");

    QPointF ptCenter = this->mapToScene(this->viewport()->rect()).boundingRect().center();

    // Selections would also render to the file
    QList<QGraphicsItem*> selectionBefore = this->graphicsScene->selectedItems();
    this->graphicsScene->clearSelection();
    // Re-shrink the scene to it's bounding contents
    QRectF r(this->graphicsScene->sceneRect());
    this->graphicsScene->setSceneRect(this->graphicsScene->itemsBoundingRect());
    // Create the image with the exact size of the shrunk scene
    QImage image((this->graphicsScene->sceneRect().size()).toSize(), QImage::Format_ARGB32);
    // Start all pixels transparent
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHints(this->renderHints());
    QBrush b = painter.brush();
    b.setStyle(Qt::NoBrush);
    b.setColor(Qt::transparent);
    painter.setBrush(b);

    this->graphicsScene->render(&painter);
    image.save(qpath);

    this->graphicsScene->setSceneRect(r);
    for (int i = 0; i < selectionBefore.count();i++)
        selectionBefore[i]->setSelected(true);

    this->centerOn(ptCenter);

    /*b.setColor(Qt::blue);
    b.setStyle(Qt::SolidPattern);
    painter.setBrush(b);
    this->graphicsScene->render(&painter);*/
}

int WorkspaceWidget::makeItem(ModelItem *item, QPointF *pt, bool left)
{
    //if the border-point is current activeEndPoint
    if (item->getSlotTrackInfo()!=NULL && this->activeFragment!=NULL)
    {
        if (this->activeFragment->findEndPointItem(this->activeEndPoint)==NULL)
        {
            app->getAppData()->setMessageDialogText("Model part can't be inserted at this point.","Díl nemůže být vložen v tomto bodě.");
            app->getAppData()->getMessageDialog()->exec();
            return 0;
        }
    }

    QPointF pos;
    if (left)
        pos.setX(item->get2DModelNoText()->scenePos().x()-5);
    else
        pos.setX(item->get2DModelNoText()->scenePos().x()+5);

    if (pt!=NULL)
        this->setActiveEndPoint(pt);
    else
        this->setActiveEndPoint(this->activeEndPoint);

    QPointF ptOld=*this->getActiveEndPoint();
    ModelFragment * previouslyActiveFragment = app->getWindow()->getWorkspaceWidget()->getActiveFragment();
    int fragCountBefore = this->modelFragments->count();

    //insert item without printing commands from setActiveEndPoint()
    this->setActiveFragment(this->findFragmentByApproxPos(pt));
    this->doNotPrintSetEndPoint=true;
    if (this->canInsert(item))
    {
        if (item->get2DModel()!=NULL)
            makeNewItem(pos,item->get2DModel(),item,item,false);
        else
            makeNewItem(pos,item->get2DModelNoText(),item,item,false);
    }

    this->doNotPrintSetEndPoint=false;

    ModelItem * lastInserted = NULL;
    if (this->activeFragment!=NULL)
        lastInserted = this->activeFragment->getFragmentItems()->last();
    else
        lastInserted = this->activeFragmentPrev->getFragmentItems()->last();

    int idOfNew = -5;
    if (fragCountBefore!=this->modelFragments->count())
        idOfNew = this->activeFragment->getID();

    //print command for item inserting/deleting
    if (left)
    {
        QString str;
        if (idOfNew!=-5)
            str = QString("make item %1 %2 L %3 %4 %5").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(pt->x()),QString::number(pt->y()),QString::number(idOfNew));
        else
            str = QString("make item %1 %2 L %3 %4 ").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(pt->x()),QString::number(pt->y()));
        QString negStr = QString("delete item %1 %2 %3").arg(QString::number(lastInserted->getParentFragment()->getID()),QString::number(lastInserted->get2DModelNoText()->scenePos().x()),QString::number(lastInserted->get2DModelNoText()->scenePos().y()));
        this->pushBackCommand(str,negStr);
    }
    else
    {
        QString str;
        if (idOfNew!=-5)
            str = QString("make item %1 %2 R %3 %4 %5 ").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(pt->x()),QString::number(pt->y()),QString::number(idOfNew));
        else
            str = QString("make item %1 %2 R %3 %4 ").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(pt->x()),QString::number(pt->y()));
        QString negStr = QString("delete item %1 %2 %3").arg(QString::number(lastInserted->getParentFragment()->getID()),QString::number(lastInserted->get2DModelNoText()->scenePos().x()),QString::number(lastInserted->get2DModelNoText()->scenePos().y()));
        this->pushBackCommand(str,negStr);
    }
    bool nullIt = false;
    if (pt==NULL)
    {
        pt = new QPointF(*this->activeEndPoint);
        nullIt = true;
    }

    //print command for setting of the endPoint which has to be active after item insertion
    QString str = (QString("make point %1 %2 ").arg(QString::number(ptOld.x()),QString::number(ptOld.y())));
    QString negStr;
    if (this->activeEndPoint!=NULL)
        negStr = (QString("make point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
    else
        negStr = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

    QString str2 = (QString("select point %1 %2 ").arg(QString::number(ptOld.x()),QString::number(ptOld.y())));
    QString negStr2;
    if (this->activeEndPoint!=NULL)
        negStr2 = (QString("select point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
    else
        negStr = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

    this->pushBackCommand(str,negStr2);
    this->pushBackCommand(str2,negStr);

    this->pushBackCommand(str,negStr2);
    this->pushBackCommand(str2,negStr);

    if (nullIt)
    {delete pt; pt=NULL;}

    if (previouslyActiveFragment!=NULL)
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(previouslyActiveFragment);

    return 0;
}

int WorkspaceWidget::makeItem(ModelItem *item, QPointF *pt, QPointF eventPos)
{
    //if the border-point is current activeEndPoint
    if (item->getSlotTrackInfo()!=NULL && this->activeFragment!=NULL)
    {
        if (this->activeFragment->findEndPointItem(this->activeEndPoint)==NULL)
        {
            app->getAppData()->setMessageDialogText("Model part can't be inserted at this point.","Díl nemůže být vložen v tomto bodě.");
            app->getAppData()->getMessageDialog()->exec();
            return 0;
        }
    }

    if (pt!=NULL)
        this->setActiveEndPoint(pt);
    else
        this->setActiveEndPoint(this->activeEndPoint);

    QPointF ptOld=*this->getActiveEndPoint();

    ModelFragment * previouslyActiveFragment = app->getWindow()->getWorkspaceWidget()->getActiveFragment();

    int fragCountBefore = this->modelFragments->count();

    this->doNotPrintSetEndPoint=true;

    if (this->canInsert(item))
        makeNewItem(eventPos,item->get2DModel(),item,item,false);

    this->doNotPrintSetEndPoint=false;

    bool nullIt = false;
    if (pt==NULL)
    {
        pt = new QPointF(*this->activeEndPoint);
        nullIt = true;
    }

    ModelItem * lastInserted = this->lastInserted;

    int idOfNew = -5;
    if (fragCountBefore!=this->modelFragments->count())
        idOfNew = this->activeFragment->getID();

    if (eventPos.x()<item->get2DModelNoText()->pos().x()/2)
    {
        QString str;
        if (idOfNew!=-5)
            str = QString("make item %1 %2 L %3 %4 %5").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(ptOld.x()),QString::number(ptOld.y()),QString::number(idOfNew));
        else
            str = QString("make item %1 %2 L %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(ptOld.x()),QString::number(ptOld.y()));
        QString negStr = QString("delete item %1 %2 %3").arg(QString::number(lastInserted->getParentFragment()->getID()), QString::number(lastInserted->get2DModelNoText()->scenePos().x()),QString::number(lastInserted->get2DModelNoText()->scenePos().y()));
        this->pushBackCommand(str,negStr);
    }
    else
    {
        QString str;
        if (idOfNew!=-5)
            str = QString("make item %1 %2 R %3 %4 %5").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(ptOld.x()),QString::number(ptOld.y()),QString::number(idOfNew));
        else
            str = QString("make item %1 %2 R %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(ptOld.x()),QString::number(ptOld.y()));
        QString negStr = QString("delete item %1 %2 %3").arg(QString::number(lastInserted->getParentFragment()->getID()),QString::number(lastInserted->get2DModelNoText()->scenePos().x()),QString::number(lastInserted->get2DModelNoText()->scenePos().y()));
        this->pushBackCommand(str,negStr);
    }

    QString str = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
    QString negStr;
    if (this->activeEndPoint!=NULL)
        negStr = (QString("make point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
    else
        negStr = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

    QString str2 = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
    QString negStr2;
    if (this->activeEndPoint!=NULL)
        negStr2 = (QString("select point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
    else
        negStr = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

    this->pushBackCommand(str,negStr2);
    this->pushBackCommand(str2,negStr);

    if (nullIt)
    {delete pt; pt = NULL;}

    if (previouslyActiveFragment!=NULL)
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(previouslyActiveFragment);


    return 0;
}

int WorkspaceWidget::makeBorder(BorderItem *border)
{
    ModelFragment * previouslyActiveFragment = this->activeFragment;

    this->doNotPrintSetEndPoint=true;
    if (this->canInsert(border))
        makeNewBorder(border);
    else
        return 1;
    this->doNotPrintSetEndPoint=false;

    QPointF * pt = this->activeEndPoint;

    {
        QString str = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
        QString negStr;
        if (this->activeEndPoint!=NULL)
            negStr = (QString("make point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
        else
            negStr = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

        QString str2 = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
        QString negStr2;
        if (this->activeEndPoint!=NULL)
            negStr2 = (QString("select point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
        else
            negStr = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

        this->pushBackCommand(str,negStr2);
        this->pushBackCommand(str2,negStr);
    }
    if (previouslyActiveFragment!=NULL)
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(previouslyActiveFragment);

    return 0;
}

int WorkspaceWidget::makeVegetation(VegetationItem *item)
{

    VegetationItem * newItem = makeNewVegetation(item);

    QString str(QString("make vegetation %1 %2 %3 %4 ").arg(*newItem->getPartNo(),*newItem->getProdLine()->getName(),QString::number(newItem->get2DModelNoText()->scenePos().x()),QString::number(newItem->get2DModelNoText()->scenePos().y())));
    QString negStr(QString("delete vegetation %1 %2 ").arg(QString::number(newItem->get2DModelNoText()->scenePos().x()),QString::number(newItem->get2DModelNoText()->scenePos().y())));
    this->pushBackCommand(str,negStr);

    QPointF * pt = this->activeEndPoint;
    {
        QString str = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
        QString negStr;
        if (this->activeEndPoint!=NULL)
            negStr = (QString("make point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
        else
            negStr = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

        QString str2 = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
        QString negStr2;
        if (this->activeEndPoint!=NULL)
            negStr2 = (QString("select point %1 %2 ").arg(QString::number(this->activeEndPointPrev->x()),QString::number(this->activeEndPointPrev->y())));
        else
            negStr = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

        this->pushBackCommand(str,negStr2);
        this->pushBackCommand(str2,negStr);
    }

    return 0;
}

void WorkspaceWidget::mousePressEvent(QMouseEvent *evt)
{
    QGraphicsView::mousePressEvent(evt);
    this->mousePress=true;
}

void WorkspaceWidget::mouseReleaseEvent(QMouseEvent *evt)
{
    QGraphicsView::mouseReleaseEvent(evt);
    this->mousePress=false;
}

void WorkspaceWidget::wheelEvent(QWheelEvent *evt)
{
    if (evt->delta()>0 && this->ctrlPress)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        this->scaleView(1.05);
    }
    else if (evt->delta()<0 && this->ctrlPress)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        this->scaleView(0.95);
    }
    else if (evt->delta()<0 && this->shiftPress)
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()-15);
    else if (evt->delta()>0 && this->shiftPress)
    {
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()+15);
    }
    else
        QGraphicsView::wheelEvent(evt);
}

void WorkspaceWidget::scaleView(qreal factor)
{
    qreal num = matrix().scale(factor,factor).mapRect(QRectF(0,0,1,1)).width();
    if (num>0.05)
        scale(factor,factor);
}

void WorkspaceWidget::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
    for (int i = 0; i < this->selection->count(); i++)
        this->selection->at(i)->get2DModelNoText()->setSelected(true);
}

void WorkspaceWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Control)
        this->ctrlPress=true;
    else if (event->key()==Qt::Key_Shift)
        this->shiftPress=true;
    else if(event->key()==Qt::Key_Space && this->lastUsedPart!=NULL && !this->heightProfileMode)
    {
        makeLastItem();
    }

    if (event->key()==Qt::Key_R)
    {
        QList<QAction*>list = app->getWindow()->getMainToolBar()->actions();
        QList<QAction*>::Iterator iter = list.begin();
        while (iter!=list.end())
        {
            if ((*iter)->toolTip()=="Rotate tool")
            {
                if((*iter)->isChecked())
                    (*iter)->setChecked(false);
                else
                    (*iter)->setChecked(true);
            }
            iter++;
        }
        this->toggleRotationMode();
    }
    else if (event->key()==Qt::Key_Plus)
        scaleView(1.05);
    else if (event->key()==Qt::Key_Minus)
        scaleView(0.95);
}

void WorkspaceWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Control)
        this->ctrlPress=false;
    else if (event->key()==Qt::Key_Shift)
        this->shiftPress=false;
}

int WorkspaceWidget::selectItem(ModelItem* item)
{
    if (item==NULL)
        return 1;

    QString str = QString("select item %1 %2").arg(QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
    QString nStr = QString("deselect item %1 %2").arg(QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
    this->pushBackCommand(str,nStr);

    GraphicsPathItemModelItem * gpi = NULL;
    if (item->getParentFragment()==NULL)
        gpi= item->get2DModel();
    else
    {
        if (!this->selection->contains(item))
            this->selection->push_back(item);
        gpi = item->get2DModelNoText();
    }

    if (!this->ctrlPress)
    {
        QList<QGraphicsItem*> list = gpi->scene()->selectedItems();
        QList<QGraphicsItem*>::Iterator iter = list.begin();
        while (iter !=list.end())
        {
            (*iter)->setSelected(false);
            this->selection->removeOne(((GraphicsPathItemModelItem*)(*iter))->getParentItem());
            iter++;
        }
    }

    if (item->getParentFragment()!=NULL && !this->selection->contains(item))
            this->selection->push_back(item);
    gpi->setSelected(true);

    return 0;
}

int WorkspaceWidget::deselectItem(ModelItem* item)
{
    if (item==NULL)
        return 1;

    if (item->getParentFragment()==NULL)
        item->get2DModel()->setSelected(false);
    else
    {
        item->get2DModelNoText()->setSelected(false);
        this->selection->removeOne(item);
    }
    return 0;
}

QList<ModelItem *> *WorkspaceWidget::getSelection()
{return this->selection;}

int WorkspaceWidget::selectItem(VegetationItem *item)
{
    if (item==NULL)
        return 1;

    QString str = QString("select vegetation %1 %2").arg(QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
    QString nStr = QString("deselect vegetation %1 %2").arg(QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
    this->pushBackCommand(str,nStr);

    GraphicsPathItemVegetationItem * gpi = NULL;
    if (item->getParentWidget()!=this)
        item->get2DModel()->setSelected(true);
    else
    {
        gpi = item->get2DModelNoText();
        if (!this->ctrlPress)
        {
            QList<QGraphicsItem*> list = gpi->scene()->selectedItems();
            QList<QGraphicsItem*>::Iterator iter = list.begin();
            while (iter !=list.end())
            {
                (*iter)->setSelected(false);
                this->selectionVegetation->removeOne(((GraphicsPathItemVegetationItem*)(*iter))->getParentItem());
                iter++;
            }
        }

        if (!this->selectionVegetation->contains(item))
            this->selectionVegetation->push_back(item);

        gpi->setSelected(true);
    }
    return 0;
}

int WorkspaceWidget::deselectItem(VegetationItem *item)
{
    if (item==NULL)
        return 1;

    if (item->getParentWidget()!=this)
        item->get2DModel()->setSelected(false);
    else
    {
        item->get2DModelNoText()->setSelected(false);
        this->selectionVegetation->removeOne(item);
    }
    return 0;
}

QList<VegetationItem *> *WorkspaceWidget::getSelectionVegetation()
{return this->selectionVegetation;}

void WorkspaceWidget::selectBorder(BorderItem *border)
{
    QList<QGraphicsItem*> sceneSelection = this->graphicsScene->selectedItems();
    for (int i = 0; i < sceneSelection.count(); i++)
    {
        if (sceneSelection[i]->type()==QGraphicsItem::UserType + 3)
            sceneSelection[i]->setSelected(false);
    }
    border->get2DModelNoText()->setSelected(true);
}

int WorkspaceWidget::connectFragments(ModelFragment *a, ModelItem * startItem)
{
    logFile << "    connectFragments(" << a << ", " << startItem << ")" << endl;
    if (a==NULL)
    {
        logFile << "ERROR: connectFragments(" << a << ", " << startItem << ")" << endl;
        return -1;
    }
    if (a->getEndPoints()->empty())
        return 0;

    ModelFragment * b = NULL;

    QList<QPointF*> aListOfPoints;
    QList<QPointF*> bListOfPoints;
    aListOfPoints.append(*a->getEndPoints());

    QList<ModelFragment*>::Iterator fragIter = this->modelFragments->begin();

    QList<QPointF*>::Iterator aPointIter = aListOfPoints.begin();
    QList<qreal>::Iterator aEPAngleIter = a->getEndPointsAngles()->begin();
    QList<ModelItem*>::Iterator aEPItemIter = a->getEndPointsItems()->begin();
    QList<QGraphicsEllipseItem*>::Iterator aEPGraphIter = a->getEndPointsGraphics()->begin();
    int aIndex = 0;

    QPointF* aPointIterLast = a->getEndPoints()->last();

    //this is used for:
    //      -when the first point of connection is found the lists and parentFragment attributes are modified, then it is set to false
    //      -after checking all points of one fragment check if it is false -> delete old fragment, otherwise noop with fragment
    bool firstFound = true;

    ModelFragment * bFirstFound = NULL;
    int bFirstFoundCountOfItems = 0;

    bool aEPILeft = false;//info just about EPItem
    bool bEPILeft = false;
    bool aEPI180diff = false;
    bool bEPI180diff = false;
    bool aLeft = false;//info about whole fragment
    bool bLeft = false;

    bool loop = true;
    //for each point of "a":
    while(loop)//(aPointIter!=a->getEndPoints()->end())
    {
        QRectF area((*aPointIter)->x()-TOLERANCE_HALF,(*aPointIter)->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);

        //get info about l-r side of aEPI
        //effectiveness trick: obtain aEPILeft everytime, but aLeft only if some points will be connected
        int aI = 0;
        while ((*aEPItemIter)->getEndPoint(aI)!=NULL)
        {
            if ((*aEPItemIter)->getEndPoint(aI)==(*aPointIter))
                break;
            aI++;
        }
        aEPILeft = !(bool)(aI%2);
        if (aI%2==0)
            aEPI180diff = (*aEPItemIter)->leftRightDifference180(aI,aI+1);
        else
            aEPI180diff = (*aEPItemIter)->leftRightDifference180(aI-1,aI);

        //for each fragment "fragIter" in workspace:
        while(fragIter!=this->modelFragments->end() && loop)
        {
            QList<QPointF*>::Iterator bPointIter = (*fragIter)->getEndPoints()->begin();
            QList<ModelItem*>::Iterator bEPItemIter = (*fragIter)->getEndPointsItems()->begin();
            QList<qreal>::Iterator bEPAngleIter = (*fragIter)->getEndPointsAngles()->begin();
            int bIndex = 0;
            b = (*fragIter);
            bListOfPoints.append(*b->getEndPoints());
            int bCountBefore = b->getEndPoints()->count();

            //this is needed for cases when the function iterates through all b-items and increments
            //aIterators -> at that moment b contains also a-items thus neighbours would be set to some wrong values
            //(usually items neighbours are set to item itself)
            if (b==bFirstFound)
                bCountBefore = bFirstFoundCountOfItems;

            //continue if iterator equals to method argument - a cannot be connected with itself
            if (a==b)
            {
                bListOfPoints.clear();
                fragIter++;
                continue;
            }

            //for each point of "fragIter":

            //check only points which have been in b fragment before merging with a
            while (bIndex<bCountBefore)//(*bPointIter!=bPointIterLast)
            {
                this->doNotPrintSetEndPoint=true;
                if (area.contains(**bPointIter) && a!=(*fragIter) && firstFound)
                {
                    ModelFragment * currentActive = this->activeFragment;
                    this->activeFragment=b;
                    if (!this->canInsert(*aEPItemIter))
                    {
                        this->activeFragment=currentActive;
                        return -1;
                    }
                    this->activeFragment=currentActive;

                    //get info about l-r side of bEPI
                    int bI = 0;
                    while ((*bEPItemIter)->getEndPoint(bI)!=NULL)
                    {
                        if ((*bEPItemIter)->getEndPoint(bI)==(*bPointIter))
                            break;
                        bI++;
                    }
                    bEPILeft = !(bool)(bI%2);
                    if (bI%2==0)
                        bEPI180diff = (*bEPItemIter)->leftRightDifference180(bI,bI+1);
                    else
                        bEPI180diff = (*bEPItemIter)->leftRightDifference180(bI-1,bI);

                    ModelItem * firstItemWith180Diff = NULL;
                    ModelItem * uselessPointer = NULL;

                    if (*aEPItemIter==firstItemWith180Diff && aI%2==0)
                        aLeft=true;
                    if (*bEPItemIter==uselessPointer && bI%2==0)
                        bLeft=true;

                    qreal dAlpha =(*bEPAngleIter)-(*aEPAngleIter);

                    logFile << "    Fragments will be connected" << endl;
                    logFile << "        A: " << a << "\n    B: " << b << endl;
                    logFile << "        A: " << aEPI180diff << aLeft << aEPILeft << endl;
                    logFile << "        B: " << bEPI180diff << bLeft << bEPILeft << endl;

                    dAlpha+=180;

                    logFile << "    A: fragment will be rotated by the angle of " << dAlpha << endl;
                    a->rotate(dAlpha,*bPointIter);

                    //moveBy(dX,dY) between points
                    qreal dX = (*bPointIter)->x()-(*aPointIter)->x();
                    qreal dY = (*bPointIter)->y()-(*aPointIter)->y();;
                    a->moveBy(dX,dY);

                    //connect xA yA xB yB IDA IDB
                    //disconnect IDC pointAt.x pointAt.y IDA IDB

                    this->doNotPrintSetEndPoint=false;
                    this->setActiveEndPoint(this->activeEndPoint);
                    this->doNotPrintSetEndPoint=true;

                    this->pushBackCommand(QString("#connect"),QString("#disconnect"));

                    QString str = (QString("@connect %1 %2 %3 %4 %5 %6 ").arg(QString::number((*aPointIter)->x()),QString::number((*aPointIter)->y()),
                                                                             QString::number((*bPointIter)->x()),QString::number((*bPointIter)->y()),
                                                                             QString::number(a->getID()),QString::number(b->getID())));

                    logFile << "aEPItemIter scenePos: " << (*aEPItemIter)->get2DModelNoText()->scenePos().x() << ", " << (*aEPItemIter)->get2DModelNoText()->scenePos().y() << endl;
                    logFile << "bEPItemIter scenePos: " << (*bEPItemIter)->get2DModelNoText()->scenePos().x() << ", " << (*bEPItemIter)->get2DModelNoText()->scenePos().y() << endl;

                    //modify neighbour of "a" endItem at this point
                    (*aEPItemIter)->setNeighbour(*bEPItemIter,*bPointIter);

                    //modify Neighbour of (*fragIter) endItem at this point
                    (*bEPItemIter)->setNeighbour(*aEPItemIter,*bPointIter);

                    {
                        int k = 0;
                        while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                        {
                            int j = 0;
                            QRectF r2((*aEPItemIter)->getEndPoint(k)->x()-TOLERANCE_HALF,(*aEPItemIter)->getEndPoint(k)->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);
                            while ((*bEPItemIter)->getEndPoint(j)!=NULL)
                            {
                                if (r2.contains(*(*bEPItemIter)->getEndPoint(j)))
                                {
                                    (*aEPItemIter)->setNeighbour(*bEPItemIter,(*aEPItemIter)->getEndPoint(k));
                                    (*bEPItemIter)->setNeighbour(*aEPItemIter,(*bEPItemIter)->getEndPoint(j));
                                }
                                j++;
                            }
                            k++;
                        }
                    }

                    QPointF * newActive = new QPointF();
                    {
                        bool wasSet = false;
                        for (int k = 0; k < b->getEndPoints()->count();k++)
                        {
                            if (*b->getEndPoints()->at(k)==*this->activeEndPoint)
                            {
                                *newActive = *b->getEndPoints()->at(k);
                                wasSet=true;
                            }
                        }
                        if (!wasSet)
                        {
                            for (int k = 0; k < a->getEndPoints()->count();k++)
                            {
                                if (*a->getEndPoints()->at(k)==*this->activeEndPoint)
                                    *newActive = *a->getEndPoints()->at(k);
                            }
                        }
                    }

                    logFile << "    Connecting fragments using the rebuildFragment(..)" << endl;

                    ModelFragment * c = NULL;
                    if (startItem == NULL)
                    {
                        c = new ModelFragment(b->getFragmentItems()->first());
                        this->addFragment(c);
                        str.append(QString::number(c->getID()));
                        rebuildFragment(b->getFragmentItems()->first(),c);
                    }
                    else
                    {
                        c = new ModelFragment(startItem);
                        this->addFragment(c);
                        str.append(QString::number(c->getID()));
                        rebuildFragment(startItem,c);
                    }

                    loop = false;
                    firstFound=false;
                    bFirstFound=c;
                    bFirstFoundCountOfItems=bCountBefore;

                    QList<ModelItem*> list;
                    list.append(*b->getFragmentItems());
                    b->getFragmentItems()->clear();

                    for (int i = 0; i < list.count(); i++)
                    {
                        if (list.at(i)->getDeleteFlag())
                            b->getFragmentItems()->push_back(list.at(i));
                    }

                    {
                        int k = 0;
                        for (;k < c->getEndPoints()->count(); k++)
                        {
                            if (pointsAreCloseEnough(newActive,c->getEndPoints()->at(k),c->getProductLines()->first()->getScaleEnum()/4.0))
                                break;
                        }
                        if (k==c->getEndPoints()->count())
                            *newActive=QPointF(0,0);
                    }


                    if (*newActive==QPointF(0,0))
                        *newActive = *c->getEndPoints()->first();

                    this->setActiveEndPoint(newActive);

                    if (this->activeFragment==b || this->activeFragment==a)
                        this->setActiveFragment(c);

                    this->modelFragments->removeOne(b);

                    QString negStr = (QString("@disconnect %1 %2 %3 %4 %5 ").arg(QString::number(c->getID()),
                                                                               QString::number((*aPointIter)->x()),QString::number((*aPointIter)->y()),
                                                                               QString::number(a->getID()),QString::number(b->getID())));
                    this->pushBackCommand(str,negStr);

                    delete (b);
                    b=c;

                    break;
                    //now check which points have been deleted:
                    //loop NxN bListOf... and b.endPoints
                    //if there is some point in bList.. which is missing in b.EPs, set it to NULL (no delete *!!!)
                    //...then check null pointers when incrementing the iterator at the end of the loop
                    QList<QPointF*>::Iterator iter = bListOfPoints.begin();
                    for (int i = 0; i < bListOfPoints.count(); i++,iter++)
                    {
                        if (!b->getEndPoints()->contains(bListOfPoints.at(i)))
                            *iter=NULL;
                    }
                }
                do
                {
                    bEPItemIter++;
                    bPointIter++;
                    bEPAngleIter++;
                    bIndex++;
                } while (*bPointIter==NULL && bPointIter!=(*fragIter)->getEndPoints()->end());
            }

            if (aIndex==-1)
                break;

            bListOfPoints.clear();
            fragIter++;
        }

        if (*aPointIter==aPointIterLast)
            break;

        fragIter=this->modelFragments->begin();
        aPointIter++;
        aEPAngleIter++;
        aEPItemIter++;
        aEPGraphIter++;
        aIndex++;
    }
    this->doNotPrintSetEndPoint=false;

    if (!firstFound)
    {
        if (a!=bFirstFound)
        {
            QList<ModelItem*> list;
            list.append(*a->getFragmentItems());
            a->getFragmentItems()->clear();

            for (int i = 0; i < list.count(); i++)
            {
                if (list.at(i)->getDeleteFlag())
                    a->getFragmentItems()->push_back(list.at(i));
            }

            this->modelFragments->removeOne(a);
            delete (a);
            a = NULL;
        }
        this->connectFragments(bFirstFound);
        return 0;
    }
    return 0;
}

int WorkspaceWidget::connectFragments(QPointF *posA, QPointF *posB, ModelFragment * a, ModelFragment * b, ModelItem * aItem, ModelItem * bItem, int fragCID)
{
    //connect xA yA xB yB IDA IDB IDC
    //disconnect IDC pointAt IDA IDB
    QString str = (QString("@connect %1 %2 %3 %4 %5 %6 %7").arg(QString::number(posA->x()),QString::number(posA->y()),
                                                                  QString::number(posB->x()),QString::number(posB->y()),
                                                                  QString::number(a->getID()),QString::number(b->getID()), QString::number(fragCID)
                                                                  ));

    this->doNotPrintSetEndPoint=true;

    if (a->getEndPoints()->empty() || b->getEndPoints()->empty())
        return 0;

    //this is used for:
    //      -when the first point of connection is found the lists and parentFragment attributes are modified, then it is set to false
    //      -after checking all points of one fragment check if it is false -> delete old fragment, otherwise noop with fragment
    bool firstFound = true;

    ModelFragment * bFirstFound = NULL;
    //int bFirstFoundCountOfItems = 0;

    ModelFragment * currentActive = this->activeFragment;
    this->activeFragment=b;
    if (!this->canInsert(aItem))
    {
        this->activeFragment=currentActive;
        return -1;
    }
    this->activeFragment=currentActive;

    //get info about l-r side of bEPI
    int bI = 0;
    while ((bItem)->getEndPoint(bI)!=NULL)
    {
        if ((bItem)->getEndPoint(bI)==(posB))
            break;
        bI++;
    }

    int x = 0;
    int y = 0;
    while (x < b->getEndPoints()->count() && !pointsAreCloseEnough(b->getEndPoints()->at(x),posB,b->getProductLines()->first()->getScaleEnum()/4.0))
        x++;
    while (y < a->getEndPoints()->count() && !pointsAreCloseEnough(a->getEndPoints()->at(y),posA,a->getProductLines()->first()->getScaleEnum()/4.0))
        y++;

    qreal dAlpha =b->getEndPointsAngles()->at(x)-a->getEndPointsAngles()->at(y);

    logFile << "    Fragments will be connected" << endl;
    logFile << "        A: " << a << "\n        B: " << b << endl;


    dAlpha+=180;

    logFile << "    A: fragment will be rotated by the angle of " << dAlpha << endl;
    a->rotate(dAlpha,posB);

    //moveBy(dX,dY) between points
    qreal dX = (posB)->x()-(posA)->x();
    qreal dY = (posB)->y()-(posA)->y();;
    a->moveBy(dX,dY);

    this->doNotPrintSetEndPoint=false;
    this->setActiveEndPoint(this->activeEndPoint);
    this->doNotPrintSetEndPoint=true;

    this->pushBackCommand(QString("#connect"),QString("#disconnect"));

    //modify neighbour of "a" endItem at this point
    (aItem)->setNeighbour(bItem,posB);

    //modify Neighbour of (*fragIter) endItem at this point
    (bItem)->setNeighbour(aItem,posB);

    {
        int k = 0;

        while ((aItem)->getEndPoint(k)!=NULL)
        {
            int j = 0;
            while ((bItem)->getEndPoint(j)!=NULL)
            {
                if (pointsAreCloseEnough(aItem->getEndPoint(k),bItem->getEndPoint(j),aItem->getProdLine()->getScaleEnum()/4.0))//if (r2.contains(*(bItem)->getEndPoint(j)))
                {
                    (aItem)->setNeighbour(bItem,(aItem)->getEndPoint(k));
                    (bItem)->setNeighbour(aItem,(bItem)->getEndPoint(j));
                }
                j++;
            }
            k++;
        }
    }

    QPointF * newActive = new QPointF();
    {
        bool wasSet = false;
        for (int k = 0; k < b->getEndPoints()->count();k++)
        {
            if (*b->getEndPoints()->at(k)==*this->activeEndPoint)
            {
                *newActive = *b->getEndPoints()->at(k);
                wasSet=true;
            }
        }
        if (!wasSet)
        {
            for (int k = 0; k < a->getEndPoints()->count();k++)
            {
                if (*a->getEndPoints()->at(k)==*this->activeEndPoint)
                    *newActive = *a->getEndPoints()->at(k);
            }
        }
    }

    logFile << "    Connecting fragments using the rebuildFragment(..)" << endl;

    ModelFragment * c = new ModelFragment(b->getFragmentItems()->first());
    rebuildFragment(b->getFragmentItems()->first(),c);

    //loop = false;
    firstFound=false;
    bFirstFound=c;

    QList<ModelItem*> list;
    list.append(*b->getFragmentItems());
    b->getFragmentItems()->clear();

    for (int i = 0; i < list.count(); i++)
    {
        if (list.at(i)->getDeleteFlag())
            b->getFragmentItems()->push_back(list.at(i));
    }

    {
        int k = 0;
        for (;k < c->getEndPoints()->count(); k++)
        {
            if (pointsAreCloseEnough(newActive,c->getEndPoints()->at(k),c->getProductLines()->first()->getScaleEnum()/4.0))
                break;
        }
        if (k==c->getEndPoints()->count())
            *newActive=QPointF(0,0);
    }

    if (*newActive==QPointF(0,0))
        *newActive = *c->getEndPoints()->first();

    this->setActiveEndPoint(newActive);

    if (this->activeFragment==b || this->activeFragment==a)
        this->setActiveFragment(c);

    this->modelFragments->removeOne(b);

    this->addFragment(c,fragCID);

    QString negStr = (QString("@disconnect %1 %2 %3 %4 %5 ").arg(QString::number(c->getID()),
                                                               QString::number((posB)->x()),QString::number((posB)->y()),
                                                               QString::number(a->getID()),QString::number(b->getID())));
    delete (b);
    b=c;

    this->pushBackCommand(str,negStr);
    this->doNotPrintSetEndPoint=false;

    if (!firstFound)
    {
        QList<ModelItem*> list;
        list.append(*a->getFragmentItems());
        a->getFragmentItems()->clear();

        for (int i = 0; i < list.count(); i++)
        {
            if (list.at(i)->getDeleteFlag())
                a->getFragmentItems()->push_back(list.at(i));
        }

        this->modelFragments->removeOne(a);
        delete (a);
        a = NULL;
        this->connectFragments(bFirstFound);
        return 0;
    }
    return 0;
}

int WorkspaceWidget::disconnectFragment(ModelFragment *c, QPointF *disconnectAt, int idA, int idB)
{
    //now find the item at which fragment should be disconnected
    GraphicsPathItemModelItem * gpi = NULL;

    QList<QGraphicsItem*> gList = this->graphicsScene->items(QRectF(disconnectAt->x()-TOLERANCE_HALF,disconnectAt->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
    for (int i = 0; i < gList.count();i++)
    {
        QGraphicsItem * qgpi = gList.at(i);

        if (qgpi!=NULL)
        {
            if (qgpi->type()!=QGraphicsItem::UserType+2)
            {
                continue;
            }
            else
            {
                ModelItem * item = ((GraphicsPathItemModelItem*)qgpi)->getParentItem();
                if (item->getEndPointIndex(disconnectAt)!=-1)
                {
                    gpi = (GraphicsPathItemModelItem*)qgpi;
                    break;
                }
            }
        }
        else
            continue;
    }

    if (gpi==NULL)
    {
        logFile << "ERROR: disconnectFragment hasn't found the item at \"disconnectAt\" position" << endl;
        return 1;
    }

    ModelItem * mi = gpi->getParentItem();
    int i = 0;
    while (mi->getEndPoint(i)!=NULL)
    {
        if (pointsAreCloseEnough(mi->getEndPoint(i),disconnectAt,mi->getProdLine()->getScaleEnum()/4.0))
            break;
        i++;
    }
    if (mi->getEndPoint(i)==NULL)
        i=0;

    if (mi->getNeighbour(i)==NULL)
        return 0;

    this->modelFragments->removeOne(c);

    ModelItem * mi2 = mi->getNeighbour(i);
    mi->getNeighbour(mi->getEndPoint(i))->setNeighbour(NULL,mi->getEndPoint(i));
    mi->setNeighbour(NULL,mi->getEndPoint(i));
    if (mi->getSlotTrackInfo()!=NULL || (mi->getType()>=T2 && mi->getType()<=T10))
    {
        int j = i%2;
        while (mi->getEndPoint(j)!=NULL)
        {
            if (mi->getNeighbour(j)==mi2)
            {
                mi->getNeighbour(mi->getEndPoint(j))->setNeighbour(NULL,mi->getEndPoint(j));
                mi->setNeighbour(NULL,mi->getEndPoint(j));
            }
            j++;
        }
    }

    //angle modification for mi and mi2
    if (mi2->getSlotTrackInfo()!=NULL)
    {
        int y = 0;
        while (mi2->getEndPoint(y)!=NULL)
        {
            if (!mi2->leftRightDifference180(y,y+1))
                mi2->setEndPointAngle(y,180+mi2->getTurnAngle(y));
            y+=2;
        }
    }
    else
    {
        int y = 0;
        ItemType t = mi2->getType();
        if (t==C1 || t==S1 || t==E1 || t==CB)
        {
            if (!mi2->leftRightDifference180(y,y+1))
                mi2->setEndPointAngle(y,180+mi2->getTurnAngle(y));
        }
        else if (t==J1)
        {
            if (mi2->getRadius()>0)
            {
                if (!mi2->leftRightDifference180(0,1))
                    mi2->setEndPointAngle(0,180+mi2->getTurnAngle(0));

                if (!mi2->leftRightDifference180(0,2))
                    mi2->setEndPointAngle(0,180+mi2->getTurnAngle(0));
            }
            else
            {
                if (!mi2->leftRightDifference180(0,2))
                    mi2->setEndPointAngle(0,180+mi2->getTurnAngle(0));

                if (!mi2->leftRightDifference180(1,2))
                    mi2->setEndPointAngle(1,180+mi2->getTurnAngle(1));
            }
        }
        else if (t==J2)
        {
            if (mi2->getRadius()>0)
            {
                if (!mi2->leftRightDifference180(0,2))
                    mi2->setEndPointAngle(0,180+mi2->getTurnAngle(0));

                if (!mi2->leftRightDifference180(1,2))
                    mi2->setEndPointAngle(1,180+mi2->getTurnAngle(1));
            }
            else
            {
                if (!mi2->leftRightDifference180(0,1))
                    mi2->setEndPointAngle(0,180+mi2->getTurnAngle(0));
            }

        }
        else if (t==J3)
        {
            if (!mi2->leftRightDifference180(y,y+1))
                mi2->setEndPointAngle(y,180+mi2->getTurnAngle(y));
        }
        else if (t==J4 || t==J5 || t==X1)
        {
            if (!mi2->leftRightDifference180(y,y+1))
                mi2->setEndPointAngle(y,180+mi2->getTurnAngle(y));
            y=2;
            if (!mi2->leftRightDifference180(y,y+1))
                mi2->setEndPointAngle(y,180+mi2->getTurnAngle(y));
        }
        else if (t>=T2 && t<=T10)
        {
            y=0;
            while (mi2->getEndPoint(y)!=NULL)
            {
                if (!mi2->leftRightDifference180(y,y+1))
                    mi2->setEndPointAngle(y,180+mi2->getTurnAngle(y));
                y+=2;
            }
        }
    }
    if (mi->getSlotTrackInfo()!=NULL)
    {
        int y = 0;
        while (mi->getEndPoint(y)!=NULL)
        {
            if (!mi->leftRightDifference180(y,y+1))
                mi->setEndPointAngle(y,180+mi->getTurnAngle(y));
            y+=2;
        }
    }
    else
    {
        int y = 0;
        ItemType t = mi->getType();
        if (t==C1 || t==S1 || t==E1 || t==CB)
        {
            if (!mi->leftRightDifference180(y,y+1))
                mi->setEndPointAngle(y,180+mi->getTurnAngle(y));
        }
        else if (t==J1)
        {
            if (mi->getRadius()>0)
            {
                if (!mi->leftRightDifference180(0,1))
                    mi->setEndPointAngle(0,180+mi->getTurnAngle(0));

                if (!mi->leftRightDifference180(0,2))
                    mi->setEndPointAngle(0,180+mi->getTurnAngle(0));
            }
            else
            {
                if (!mi->leftRightDifference180(0,2))
                    mi->setEndPointAngle(0,180+mi->getTurnAngle(0));

                if (!mi->leftRightDifference180(1,2))
                    mi->setEndPointAngle(1,180+mi->getTurnAngle(1));
            }
        }
        else if (t==J2)
        {
            if (mi->getRadius()>0)
            {
                if (!mi->leftRightDifference180(0,2))
                    mi->setEndPointAngle(0,180+mi->getTurnAngle(0));

                if (!mi->leftRightDifference180(1,2))
                    mi->setEndPointAngle(1,180+mi->getTurnAngle(1));
            }
            else
            {
                if (!mi->leftRightDifference180(0,1))
                    mi->setEndPointAngle(0,180+mi->getTurnAngle(0));
            }
        }
        else if (t==J3)
        {
            if (!mi->leftRightDifference180(y,y+1))
                mi->setEndPointAngle(y,180+mi->getTurnAngle(y));
        }
        else if (t==J4 || t==J5 || t==X1)
        {
            if (!mi->leftRightDifference180(y,y+1))
                mi->setEndPointAngle(y,180+mi->getTurnAngle(y));
            y=2;
            if (!mi->leftRightDifference180(y,y+1))
                mi->setEndPointAngle(y,180+mi->getTurnAngle(y));
        }
        else if (t>=T2 && t<=T10)
        {
            y=0;
            while (mi->getEndPoint(y)!=NULL)
            {
                if (!mi->leftRightDifference180(y,y+1))
                    mi->setEndPointAngle(y,180+mi->getTurnAngle(y));
                y+=2;
            }
        }
    }

    //fragment name mismatch b should be called "A", because idA is constructed from mi (not mi2)
    ModelFragment * b = new ModelFragment(mi);
    this->addFragment(b, idA);
    rebuildFragment(mi,b);
    if (mi2->getParentFragment()==b)
    {
        return 0;
    }
    ModelFragment * a = new ModelFragment(mi2);
    this->addFragment(a, idB);
    rebuildFragment(mi2,a);
    QList<ModelItem*> theList;
    for (int i = 0; i < c->getFragmentItems()->count();i++)
    {
        if (c->getFragmentItems()->at(i)->getDeleteFlag())
            theList.push_back(c->getFragmentItems()->at(i));
    }

    c->getFragmentItems()->clear();
    c->getFragmentItems()->append(theList);

    if (this->activeFragment==c)
        this->activeFragment=b;

    delete c;

    return 0;
}

int WorkspaceWidget::addVegetation(VegetationItem *item)
{
    if (item==NULL)
        return 1;
    (*this->vegetationItems) << item;
    return 0;
}

int WorkspaceWidget::addFragment(ModelFragment * frag, int fragID)
{
    if (frag==NULL)
        return 1;
    this->modelFragments->push_back(frag);
    QList<ModelItem*>::Iterator iter = frag->getFragmentItems()->begin();

    while (iter!=frag->getFragmentItems()->end())
    {
        if ((**iter).get2DModelNoText()->scene()!=this->graphicsScene)
            this->graphicsScene->addItem((*iter)->get2DModelNoText());

        iter++;
    }

    if (fragID!=-1)
        frag->setID(fragID);
    else
        frag->setID(this->nextIDToUse);

    this->nextIDToUse++;

    return 0;
}

int WorkspaceWidget::removeFragment(ModelFragment * frag)
{
    if (frag==NULL)
        return 1;

    bool success = this->modelFragments->removeOne(frag);
    if (success)
    {
        if (this->findFragmentByApproxPos(this->activeEndPoint)==frag)
            *this->activeEndPoint=QPointF(0,0);

        if (this->activeFragment==frag)
            this->setActiveFragment(NULL);

        delete frag;

        return 0;
    }
    else
        return 2;
}

int WorkspaceWidget::removeItem(ModelItem *item, QList<int> *idList)
{
    QPointF pos (item->get2DModelNoText()->scenePos());
    QString str;
    str = QString("delete item %1 %2 %3 ").arg(QString::number(item->getParentFragment()->getID()),QString::number(pos.x()),QString::number(pos.y()));

    QString negStr;
    if (item->getParentFragment()->getFragmentItems()->count()==1)
    {
        if (item->getRadius()>0)
            negStr = QString("make item %1 %2 R %3 %4 %5").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()),QString::number(item->getParentFragment()->getID()));
        else
            negStr = QString("make item %1 %2 L %3 %4 %5").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()),QString::number(item->getParentFragment()->getID()));
    }
    else
    {
        if (item==item->getParentFragment()->getFragmentItems()->first())
        {
            if (item->getRadius()>0)
                negStr = QString("make item %1 %2 R %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
            else
                negStr = QString("make item %1 %2 L %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
        }
        else
        {
            if (item->getRadius()>0)
                negStr = QString("make item %1 %2 R %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y()));
            else
                negStr = QString("make item %1 %2 L %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y()));
        }
    }

    this->pushBackCommand(str,negStr);

    if (this->lastInserted==item)
    {
        int i = 0;
        while (item->getEndPoint(i)!=NULL)
        {
            if (item->getNeighbour(i)!=NULL && !item->getNeighbour(i)->getDeleteFlag())
            {
                this->lastInserted=item->getNeighbour(i);
                break;
            }
            i++;
        }
    }

    QPointF zeroItemScenePos = item->getParentFragment()->getFragmentItems()->first()->get2DModelNoText()->scenePos();

    int originalId = -12345;

    QList <ModelFragment*> oldFragmentsList = *this->modelFragments;

    ModelItem * originalItem = item->getProdLine()->findItemByPartNo(item->getPartNo());

    ModelFragment * original = item->getParentFragment();
    int originalItemsCount = original->getFragmentItems()->count();
    int ret = item->getParentFragment()->deleteFragmentItem(item,idList);
    this->selection->removeOne(item);

    if (app->getRestrictedInventoryMode())
    {
        originalItem->incrAvailableCount();
        originalItem->get2DModel()->changeCountPath(originalItem->getAvailableCount());
    }

    {
        GraphicsPathItemModelItem * gpi = NULL;
        //QGraphicsPathItem * qgpi = NULL;
        QList<QGraphicsItem*> itemsList = this->scene()->items(QRectF(zeroItemScenePos.x()-TOLERANCE_HALF,zeroItemScenePos.y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
        for (int i = 0; i < itemsList.count();i++)
        {
            if (itemsList[i]->type()==QGraphicsItem::UserType+2)
            {
                gpi = (GraphicsPathItemModelItem*)itemsList[i];
                QPointF pt = gpi->scenePos();
                if (!pointsAreCloseEnough(&pt,&zeroItemScenePos,TOLERANCE_HALF))
                    continue;
                else
                {
                    originalId=gpi->getParentItem()->getParentFragment()->getID();
                    break;
                }
            }
        }
    }

    if (idList==NULL && this->undoRedoCalled==false)
    {
        QString str2 = str;
        for (int i = 0; i < this->modelFragments->count();i++)
        {
            if (!oldFragmentsList.contains(this->modelFragments->at(i)))
                str2.push_back(QString(" %1").arg(QString::number(this->modelFragments->at(i)->getID())));
        }
        this->actionListRedo[this->actionListRedo.lastIndexOf(str)]=str2;
        QString negStr2 = negStr;
        if (originalItemsCount!=1)
        {
            negStr2.append(QString(" %1 %2 %3").arg(QString::number(zeroItemScenePos.x()),QString::number(zeroItemScenePos.y()),QString::number(originalId)));
            this->actionListUndo[this->actionListUndo.lastIndexOf(negStr)]=negStr2;
        }
    }
    if (ret==-1)
    {

        delete original;
        this->modelFragments->removeOne(original);
        //break;
    }
    return 0;
}

int WorkspaceWidget::removeVegetation(VegetationItem *item)
{
    if (item==NULL)
        return -1;

    QString negStr(QString("make vegetation %1 %2 %3 %4 ").arg(*item->getProdLine()->getName(),*item->getPartNo(),QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y())));
    QString str(QString("delete vegetation %1 %2 ").arg(QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y())));
    this->pushBackCommand(str,negStr);

    this->vegetationItems->removeOne(item);
    delete item;

    return 0;
}

int WorkspaceWidget::bendAndClose(ModelFragment *frag, QPointF * pt1, QPointF * pt2)
{
    QList<QAction*> actions = app->getWindow()->getMainToolBar()->actions();
    for (int i = 0; i < actions.count(); i++)
    {
        if (actions[i]->text().startsWith("Bend") || actions[i]->text().startsWith("Ohnout"))
            actions[i]->setChecked(false);
    }
    /*
    -get the dx and dy (not rotated) and dAlpha between points which will be connected
    -var len = get the length of the vector (dx,dy)

    -var diffCoef = abs(dAlpha-len)
    -var maxDiffCoef = 0;
    -loop through the items
        -maxDiffCoef += items[i].getMaxFlex();
    -if (max<diff)
        -return ERROR MESSAGE
    */

    this->selectTwoPointsBend=false;

    if (pt1==NULL || pt2==NULL || frag==NULL)
        return 1;

    QString str = QString("bend close %1 %2 %3 %4 %5").arg(QString::number(frag->getID()),QString::number(pt1->x()),QString::number(pt1->y()),QString::number(pt2->x()),QString::number(pt2->y()));
    QString negStr = QString("bend open %1 %2 %3 %4 %5").arg(QString::number(frag->getID()),QString::number(pt1->x()),QString::number(pt1->y()),QString::number(pt2->x()),QString::number(pt2->y()));

    this->pushBackCommand(str,negStr);

    qreal angle1 = 0;
    qreal angle2 = 0;
    for (int i = 0; i < frag->getEndPoints()->count();i++)
    {
        if (pointsAreCloseEnough(pt1,frag->getEndPoints()->at(i),TOLERANCE_HALF))
        {
            angle1 = frag->getEndPointsAngles()->at(i);
        }
        else if (pointsAreCloseEnough(pt2,frag->getEndPoints()->at(i),TOLERANCE_HALF))
        {
            angle2 = frag->getEndPointsAngles()->at(i);
        }
    }

    ModelItem * item1 = frag->findEndPointItem(pt1);
    ModelItem * item2 = frag->findEndPointItem(pt2);

    int index1 = 0;
    int index2 = 0;
    while (item1->getEndPoint(index1)!=NULL)
    {
        if (pointsAreCloseEnough(pt1,item1->getEndPoint(index1),item1->getProdLine()->getScaleEnum()/4.0))
            break;
        index1++;
    }
    while (item2->getEndPoint(index2)!=NULL)
    {
        if (pointsAreCloseEnough(pt2,item2->getEndPoint(index2),item2->getProdLine()->getScaleEnum()/4.0))
            break;
        index2++;
    }

    bool partsAreCovering = false;
    bool close = true;
    bool yMajor = false;

    QPointF point1 = *pt1;
    QPointF point2 = *pt2;

    QPointF zero = *pt1;
    if (abs(angle1)>abs(angle2))
    {
        rotatePoint(&point1,-angle1,&zero);
        rotatePoint(&point2,-angle1,&zero);
    }
    else
    {
        rotatePoint(&point1,-angle2,&zero);
        rotatePoint(&point2,-angle2,&zero);
    }

    qreal dx = abs (point1.x()-point2.x());
    qreal dy = abs (point1.y()-point2.y());

    if (1.1*dx<dy)
        yMajor=true;

    while (angle1<0)
        angle1+=360;
    while (angle1>360)
        angle1-=360;
    while (angle2<0)
        angle2+=360;
    while (angle2>360)
        angle2-=360;

    qreal dAlpha = abs((angle1)-(angle2));
    //qreal dAlpha = abs(angle1)+abs(angle2);

    if (dAlpha>=168 && dAlpha<=195)
        dAlpha -=180;
    else if (dAlpha<=10)
        dAlpha +=180;

    dAlpha = abs(dAlpha);

    qreal len = sqrt(dx*dx+dy*dy);

    qreal flexNeeded = abs(dAlpha-len)/1.0;

    logFile << "Bending fragment with ID " << frag->getID() << " at point ";
    logFile << pt1->x() << " " << pt1->y() << " and " << pt2->x() << " " << pt2->y() << endl;

    qreal maxFlex = 0;
    for (int i = 0; i < frag->getFragmentItems()->count();i++)
    {
        maxFlex += frag->getFragmentItems()->at(i)->getMaxFlex();
    }
    qreal lenConstraint = item1->getProdLine()->getScaleEnum();
    if (item1->getSlotTrackInfo()==NULL)
        lenConstraint *=3;
    if (maxFlex<flexNeeded || len>=lenConstraint || (dAlpha>=10 && dAlpha <=170) || (dAlpha >= 190 && dAlpha <= 350))
    {
        logFile << "    Flex needed: " << flexNeeded << ", maximal flex: " << maxFlex << endl;
        if (*pt1==*pt2)
            app->getAppData()->setMessageDialogText("Two different points are needed to bend the track. Track section will not be closed.","Pro uzavření úseku trati jsou třeba dva různé body. Trat nebude uzavřena.");
        else
            app->getAppData()->setMessageDialogText("The points you have selected are too far. Track section will not be closed.","Vámi vybrané body jsou příliš daleko. Fragment nebude uzavřen.");
        app->getAppData()->getMessageDialog()->exec();
        return 2;
    }

    logFile << "    Flex needed: " << flexNeeded << ", maximal flex: " << maxFlex << endl;

    logFile << "Point1: " << point1.x() << " " << point1.y() << endl;
    logFile << "Point2: " << point2.x() << " " << point2.y() << endl;


    QPointF pointOfRotation90;

    ModelItem * it90 = NULL;

    int i90 = 0;

    qreal currentAngle = 0;

    QList<ModelItem*>visited;
    QList<ModelItem*>toVisit;
    QList<ModelItem*>turnoutsList;
    QList<int>turnoutsIndices;

    QPointF ptTemp;
    QPointF ptTemp2;

    if (point1.y()<point2.y())
    {
        ptTemp = *pt2;
        ptTemp2 = *pt1;
        toVisit.push_back(frag->findEndPointItem(pt2));
    }
    else
    {
        ptTemp = *pt1;
        ptTemp2 = *pt2;
        toVisit.push_back(frag->findEndPointItem(pt1));
    }

    /*
    ##find the path
    loop through the fragment
        -current = toVisit.first; toVisit.pop_front();
        -if (current is a turnout)
            -turnoutsList.push_front(current)
        -loop (while) through the all endpoints of current, index
            -toVisit.push_front(first "not-yet-visited-neighbour")
            -if (current is a turnout)
                -turnoutIndices.first() = index;
            -if (pointsAreCloseEnough(index,pt2)
                break both loops
            -if some neighbour was added, end loop (index)

        -if (current is a turnout) && (no neighbour was added in code above) && current.ep[index]==NULL
            -remove current(turnout) from turnoutsList (it should be the first item - check it)
        -if (current is ANYTHING) && (no neigh. was added) && current.ep[index]==NULL
            -toVisit.push-front(turnoutsList.first()) //next iteration will continue from nearest turnout
        -visited.push_back(current); //if it is not there yet
    end of the loop

    -turnoutsList now contains all turnouts through which you get from pt1 to pt2
    -turnoutsIndices contains indices of appropriate point for each turnout through which you get closer to the pt2
    */

    ModelItem * current = NULL;
    bool bothLoopsEnabled = true;
    while (!toVisit.empty() && bothLoopsEnabled)
    {
        current = toVisit.first();
        toVisit.pop_front();

        ItemType typeOfCurrent = current->getType();
        bool currentIsATurnout=false;

        if (typeOfCurrent==J1 || typeOfCurrent==J2 || typeOfCurrent==J3 || typeOfCurrent==J4 || typeOfCurrent==X1 || typeOfCurrent==J5)
        {
            turnoutsList.push_front(current);
            turnoutsIndices.push_front(0);
            currentIsATurnout=true;
        }

        bool somethingWasAdded = false;
        int i = 0;
        while (current->getEndPoint(i)!=NULL && bothLoopsEnabled)
        {
            if (current->getNeighbour(i)!=NULL && !visited.contains(current->getNeighbour(i)))
            {
                toVisit.push_front(current->getNeighbour(i));
                somethingWasAdded=true;
                if (currentIsATurnout)
                    turnoutsIndices[0]=i;

                if (somethingWasAdded)
                    break;
            }
            if (ptTemp==*pt1)
            {
                if (pointsAreCloseEnough(pt2, current->getEndPoint(i),current->getProdLine()->getScaleEnum()/4.0))
                {
                    bothLoopsEnabled=false;
                    continue;
                }
            }
            else
            {
                if (pointsAreCloseEnough(pt1, current->getEndPoint(i),current->getProdLine()->getScaleEnum()/4.0))
                {
                    if (currentIsATurnout)
                    {
                        turnoutsList.pop_front();
                        turnoutsIndices.pop_front();
                    }
                    bothLoopsEnabled=false;
                    continue;
                }
            }

            i++;
        }
        if (!bothLoopsEnabled)
            break;

        if (currentIsATurnout && !somethingWasAdded && current->getEndPoint(i)==NULL)
        {
            int c = turnoutsList.count();
            for (int j = 0; j < c;j++)
            {
                int iList = turnoutsList.indexOf(current);
                if (iList==-1)
                    break;
                turnoutsList.removeAt(iList);
                turnoutsIndices.removeAt(iList);
            }
        }
        if (!somethingWasAdded && current->getEndPoint(i)==NULL)
        {
            if (!turnoutsList.empty())
                toVisit.push_front(turnoutsList.first());
        }

        if (!visited.contains(current))
            visited.push_back(current);
        toVisit.removeAll(current);
    }

    /*
    ##get points of future rotation -> "bending-points"
    toVisit.clear(); visited.clear();

    angle = 0;
    loop through the fragment
        -current = toVisit.first();
        -if (current is not turnout)
            loop through all neighbours of current
                first not-visited will be pushed-front in toVisit
                break;

        -if (current is turnout)
            no loop, toVisit.push_front( current.neigh[turnoutIndices[turnoutsList.indexOf(current)]] )
        -angle += current.dAlpha
        -if angle is at least 90, 180 or 270, set appropriate points, items and indices (index stores the direction, where to continue with rotation

    ##rotate*/

    toVisit.clear();
    visited.clear();

    if (point1.y()<point2.y())
        toVisit.push_back(frag->findEndPointItem(pt2));
    else
        toVisit.push_back(frag->findEndPointItem(pt1));

    int countOfItemsToRotate = 0;

    logFile << "dAlpha=" << dAlpha << endl;

    //toVisit.push_back(frag->findEndPointItem(&ptTemp));
    while (!toVisit.empty())
    {
        current = toVisit.first();
        if (current==NULL)
        {
            logFile << "ERROR: bendAndClose, path has been found, current==NULL during the search for rotation points" << endl;
            return -1;
        }
        toVisit.pop_front();

        ItemType typeOfCurrent = current->getType();

        int i = 0;
        if (typeOfCurrent==J1 || typeOfCurrent==J2 || typeOfCurrent==J3 || typeOfCurrent==J4 || typeOfCurrent==X1 || typeOfCurrent==J5)
        {

            while (current->getEndPoint(i)!=NULL)
            {
                if (pointsAreCloseEnough(pt1, current->getEndPoint(i),current->getProdLine()->getScaleEnum()/4.0))
                    break;
                i++;
            }
            if (current->getEndPoint(i)!=NULL)
                break;
            ModelItem * itemToVisit = current->getNeighbour(turnoutsIndices[turnoutsList.indexOf(current)]);
            if (itemToVisit!=NULL)
                toVisit.push_front(itemToVisit);
            i = turnoutsIndices[turnoutsList.indexOf(current)];
        }
        else
        {

            while (current->getEndPoint(i)!=NULL)
            {
                if (current->getNeighbour(i)!=NULL && !visited.contains(current->getNeighbour(i)))
                {
                    toVisit.push_front(current->getNeighbour(i));
                    break;
                }
                i++;
            }
        }

        {
            qreal currentdAlpha = abs(current->getTurnAngle(0)-current->getTurnAngle(1));
            if (current->leftRightDifference180(0,1))
                currentdAlpha-=180;
            currentdAlpha = abs(currentdAlpha);
            if (currentdAlpha==0)
            {
                if (current->getSlotTrackInfo()==NULL && (typeOfCurrent==J1 || typeOfCurrent==J2 || typeOfCurrent==J3))
                {
                    currentdAlpha = abs(current->getTurnAngle(2)-current->getTurnAngle(0));
                    if (current->leftRightDifference180(0,2))
                        currentdAlpha-=180;
                }
            }

            //CW rotation => +=
            //CCW rotation => -=
            //CW rotation <=> current.r>0 && item "is visited in its normal direction" (which means previousItem is connected at even point)
            //                || current.r<0 && item "is visited backwards"
            bool clockWise = true;
            {
                bool leftVisited = false;
                int j = 0;
                while (current->getEndPoint(j)!=NULL)
                {
                    if (visited.contains(current->getNeighbour(j)))
                    {
                        leftVisited = true;
                        break;
                    }
                    j+=2;
                }
                if (leftVisited && current->getRadius()>0)
                    clockWise=true;
                else if (leftVisited && current->getRadius()<0)
                    clockWise=false;
                else if (!leftVisited && current->getRadius()>0)
                    clockWise=false;
                else if (!leftVisited && current->getRadius()<0)
                    clockWise=true;
            }

            if (!clockWise)
                currentdAlpha*=-1;

            currentAngle += (currentdAlpha);

            if (abs(currentAngle) >=76 && it90==NULL)
            {
                i90=i;//(i+1)%2;
                it90=current;
                pointOfRotation90=*current->getEndPoint(i);
                countOfItemsToRotate++;
            }
        }
        visited.push_back(current);
        if (countOfItemsToRotate!=0)
            countOfItemsToRotate++;

    }

    qreal rotationFix = 0;

    /*arc length:
      s = (PI*r*angle)/180
      =>
      180s = PI*r*angle
      180s/rPI = angle
      alpha = 180s/rPI, where s = dist(center,ptTemp)
     */
    qreal r = 0;
    qreal dist12 = 0;
    qreal alpha1 = 0;

    /*
        -left turns are closed by -rotation
        -right turns are opened by -rotation
    */
    logFile << "    PartsAreCovering: " << partsAreCovering << ", close: " << close << ", yMajor: " << yMajor << endl;

    ModelItem * item = NULL;
    QPointF center;
    int index;

    toVisit.clear();


    item=it90;
    index = i90;
    center = pointOfRotation90;
    toVisit.push_back(item->getNeighbour((index)));


    int removeFrom = 0;
    while (visited.at(removeFrom)!=item)
        removeFrom++;
    while (visited.count()-removeFrom-1>0)
        visited.removeLast();

    QList <int> toVisitIndices;
    toVisitIndices.push_back(item->getNeighbour(index)->getEndPointIndex(item->getEndPoint(index)));

    for (int j = 0; j < countOfItemsToRotate && !toVisit.empty() && !pointsAreCloseEnough(&ptTemp,&ptTemp2,frag->getProductLines()->first()->getScaleEnum()/4.0); j++)
    {
        item = toVisit.first();
        index = toVisitIndices.first();
        if (item->getEndPoint(index)!=NULL)
            center = *item->getEndPoint(index);
        else
            break;
        toVisit.pop_front();
        toVisitIndices.pop_front();

        qreal dist12Old = dist12;
        dist12 = dist(&ptTemp,&ptTemp2);
        if (dist12Old<dist12 && dist12Old!=0)
            dist12=0; //=> no changes will be done

        qreal circleDX = abs(ptTemp.x())-abs(center.x());
        qreal circleDY = abs(ptTemp.y())-abs(center.y());

        r=sqrt(circleDX*circleDX+circleDY*circleDY);
        r=max(r,abs(item->getRadius()));


        alpha1 = (180*(dist12))/(r*PI);
        alpha1 = min(alpha1,item->getMaxFlex());

        if (currentAngle>50)
            alpha1 = min(alpha1,item->getMaxFlex()/2);

        rotationFix = alpha1;

        QPointF ptPositive = ptTemp;
        QPointF ptNegative = ptTemp;
        QPointF ptPositiveHalf = ptTemp;
        QPointF ptNegativeHalf = ptTemp;

        rotatePoint (&ptPositive,rotationFix, &center);
        rotatePoint (&ptNegative,-rotationFix, &center);
        rotatePoint (&ptPositiveHalf,rotationFix/2, &center);
        rotatePoint (&ptNegativeHalf,-rotationFix/2, &center);

        qreal distPos = dist(&ptPositive,&ptTemp2);
        qreal distNeg = dist(&ptNegative,&ptTemp2);
        qreal distPosHalf = dist(&ptPositiveHalf,&ptTemp2);
        qreal distNegHalf = dist(&ptNegativeHalf,&ptTemp2);

        qreal array[4] = {distPos,distNeg,distPosHalf,distNegHalf};
        qreal min=array[0];
        int minIndex = 0;
        for (int k = 0; k < 4; k++)
        {
            if (min>array[k])
            {
                min=array[k];
                minIndex=k;
            }
        }

        if (minIndex%2==1)
            rotationFix*=-1;
        if (minIndex>=2)
            rotationFix/=2;

        rotatePoint(&ptTemp,rotationFix, &center);

        rotationFix*=-1;

        QList<ModelItem*> visitedRot;
        QList<ModelItem*> toVisitRot;

        visitedRot.append(visited);

        visitedRot.push_back(item);
        toVisitRot.push_back(item);

        while (!toVisitRot.empty())
        {
            current = toVisitRot.first();
            toVisitRot.pop_front();
            int k = 0;
            while (current->getEndPoint(k)!=NULL)
            {
                if (current->getNeighbour(k)!=NULL && !visitedRot.contains(current->getNeighbour(k)) && !toVisitRot.contains(current->getNeighbour(k)))
                {
                    toVisitRot.push_back(current->getNeighbour(k));
                }
                k++;
            }
            current->rotate(rotationFix,&center,true);
            visitedRot.push_back(current);
        }

        ItemType typeOfCurrent = item->getType();

        int i = 0;

        if (typeOfCurrent==J1 || typeOfCurrent==J2 || typeOfCurrent==J3 || typeOfCurrent==J4 || typeOfCurrent==X1 || typeOfCurrent==J5)
        {
            while (item->getEndPoint(i)!=NULL)
            {
                if (pointsAreCloseEnough(pt1, item->getEndPoint(i),item->getProdLine()->getScaleEnum()/4.0))
                    break;
                i++;
            }
            if (item->getEndPoint(i)!=NULL)
                break;

            ModelItem * itemToVisit = item->getNeighbour(turnoutsIndices[turnoutsList.indexOf(item)]);
            if (itemToVisit!=NULL)
            {
                toVisit.push_front(itemToVisit);
                i = turnoutsIndices[turnoutsList.indexOf(item)];
                toVisitIndices.push_front(i);
            }
        }
        else
        {
            while (item->getEndPoint(i)!=NULL)
            {
                if (item->getNeighbour(i)!=NULL && !visited.contains(item->getNeighbour(i)))
                {
                    toVisit.push_front(item->getNeighbour(i));
                    //toVisitIndices.push_front((i+1)%2);
                    toVisitIndices.push_front(item->getNeighbour(i)->getEndPointIndex(item->getEndPoint(i)));
                    break;
                }
                i++;
            }
        }
        visited.push_back(item);
    }

    //now rotate the last item by the angle which will make the track "visually continous" and removes any "steps" in angles
    if (point1.y()<point2.y())
        *pt2=ptTemp;
    else
        *pt1=ptTemp;

    {
        /*
        code above may cause some visible "steps" on the track
        this block of code fixes this problem
        how is it done:
        -get the "width" of the endPointItem which is not at the ptTemp point ("width" = distance between two endpoints)
        -measure the distance between pt1 and pt2
        -have the circular sector with radius equal to "width". The length of the sector is equal to dist(pt1,pt2).
        -angle of the sector is the angle of the rotation which fixes (not 100% perfect) the problem
        */

        /*arc length:
          s = (PI*r*angle)/180
          =>
          180s = PI*r*angle
          180s/rPI = angle
          alpha = 180s/rPI, where s = dist(pt1,pt2)
         */


        qreal r = 0;
        qreal dist12 = 0;
        qreal alpha = 0;


        *pt1 = *item1->getEndPoint(index1);
        *pt2 = *item2->getEndPoint(index2);

        index1 = 0;
        index2 = 0;
        while (item1->getEndPoint(index1)!=NULL)
        {
            if (pointsAreCloseEnough(pt1,item1->getEndPoint(index1),item1->getProdLine()->getScaleEnum()/4.0))
                break;
            index1++;
        }
        while (item2->getEndPoint(index2)!=NULL)
        {
            if (pointsAreCloseEnough(pt2,item2->getEndPoint(index2),item2->getProdLine()->getScaleEnum()/4.0))
                break;
            index2++;
        }

        angle1=item1->getTurnAngle(index1);
        angle2=item2->getTurnAngle(index2);

        if ((angle1<0 && angle2>0) || (item1->leftRightDifference180(index1,index1+1) && index1%2==0))
            angle1+=180;
        if ((angle1>0 && angle2<0) || (item2->leftRightDifference180(index2,index2+1) && index2%2==0))
            angle2+=180;

        QPointF point1 = *pt1;
        QPointF point2 = *pt2;

        QPointF zero = *pt1;
        rotatePoint(&point1,-angle1,&zero);
        rotatePoint(&point2,-angle1,&zero);

        ModelItem * toRotate = NULL;
        QPointF * fragPoint = NULL;
        int index = 0;
        if (point1.y()<point2.y())
        {
            toRotate = item2;
            index = index2;
            fragPoint = pt2;
        }
        else
        {
            toRotate = item1;
            index = index1;
            fragPoint = pt1;
        }

        if (index%2==0)
        {
            qreal dxItem = toRotate->getEndPoint(index)->x()-toRotate->getEndPoint(index+1)->x();
            qreal dyItem = toRotate->getEndPoint(index)->y()-toRotate->getEndPoint(index+1)->y();
            r = sqrt(dxItem*dxItem+dyItem*dyItem);
            dist12 = abs(point1.y()-point2.y());
        }
        else
        {
            qreal dxItem = toRotate->getEndPoint(index)->x()-toRotate->getEndPoint(index-1)->x();
            qreal dyItem = toRotate->getEndPoint(index)->y()-toRotate->getEndPoint(index-1)->y();
            r = sqrt(dxItem*dxItem+dyItem*dyItem);
            dist12 = abs(point1.y()-point2.y());
        }

        alpha = (180*dist12)/(r*PI);

        //now rotate "test point" - if it is closer to "the other end point", rotate,
        //otherwise rotate in opposite direction
        QPointF testPoint(*fragPoint);
        if (index%2==0)
            rotatePoint(&testPoint,alpha,toRotate->getEndPoint(index+1));
        else
            rotatePoint(&testPoint,alpha,toRotate->getEndPoint(index-1));
        qreal testDX = 0;
        qreal testDY = 0;
        //pointers are equal -> compare to the other point
        if (fragPoint==pt1)
        {
            testDX = abs(testPoint.x()-pt2->x());
            testDY = abs(testPoint.y()-pt2->y());
        }
        else
        {
            testDX = abs(testPoint.x()-pt1->x());
            testDY = abs(testPoint.y()-pt1->y());
        }
        qreal testLen = sqrt(testDX*testDX+testDY*testDY);

        //update length of dx,dy vector
        dx = abs (point1.x()-point2.x());
        dy = abs (point1.y()-point2.y());
        len = sqrt(dx*dx+dy*dy);

        if (testLen>len)
            alpha*=-1;

        //now do the final rotation
        if (index%2==0)
        {
            toRotate->rotate(alpha,toRotate->getEndPoint(index+1),true);
        }
        else
        {
            toRotate->rotate(alpha,toRotate->getEndPoint(index-1),true);
        }
    }

    *pt1 = *item1->getEndPoint(index1);
    *pt2 = *item2->getEndPoint(index2);

    item1->setNeighbour(item2,pt1);
    item2->setNeighbour(item1,pt2);

    int fragIndex1 = -1;
    int fragIndex2 = -1;
    int i = 0;
    while (i < frag->getEndPoints()->count() && (fragIndex1==-1 || fragIndex2==-1))
    {
        if (fragIndex1==-1 && pointsAreCloseEnough(pt1,frag->getEndPoints()->at(i),frag->getProductLines()->first()->getScaleEnum()/4.0))
        {
            fragIndex1=i;
        }
        if (fragIndex2==-1 && pointsAreCloseEnough(pt2,frag->getEndPoints()->at(i),frag->getProductLines()->first()->getScaleEnum()/4.0))
        {
            if (fragIndex1!=i)
                fragIndex2=i;
        }
        i++;
    }
    frag->getEndPointsGraphics()->at(fragIndex1)->setPos(pt1->x(),pt1->y());
    frag->getEndPointsGraphics()->at(fragIndex2)->setPos(pt2->x(),pt2->y());

    frag->updateEndPointsGraphics();

    //second index cant be used - it is not valid!
    frag->removeEndPoint((*frag->getEndPoints())[fragIndex1]);
    if (fragIndex1<fragIndex2)
        fragIndex2--;

    frag->removeEndPoint((*frag->getEndPoints())[fragIndex2]);
    return 0;
}

int WorkspaceWidget::bendAndClose(ModelFragment *fragA, ModelFragment *fragB, QPointF *pt1, QPointF *pt2)
{
    QList<QAction*> actions = app->getWindow()->getMainToolBar()->actions();
    for (int i = 0; i < actions.count(); i++)
    {
        if (actions[i]->text().startsWith("Bend") || actions[i]->text().startsWith("Ohnout"))
            actions[i]->setChecked(false);
    }

    this->selectTwoPointsBend=false;

    if (pt1==NULL || pt2==NULL || fragA==NULL || fragB==NULL)
        return 1;

    QString str = QString("bend2 close %1 %2 %3 %4 %5 %6").arg(QString::number(fragA->getID()),QString::number(fragB->getID()),QString::number(pt1->x()),QString::number(pt1->y()),QString::number(pt2->x()),QString::number(pt2->y()));
    QString negStr = QString("bend2 open %1 %2 %3 %4 %5 %6").arg(QString::number(fragA->getID()),QString::number(fragB->getID()),QString::number(pt1->x()),QString::number(pt1->y()),QString::number(pt2->x()),QString::number(pt2->y()));

    this->pushBackCommand(str,negStr);

    //get the distance between points
    qreal dist12 = dist(pt1,pt2);
    qreal dAlpha = 0;
    qreal maxFlex = 0;
    qreal neededFlex = 0;
    int n = 5;
    n = min(n,fragA->getFragmentItems()->count());
    n = min(n,fragB->getFragmentItems()->count());

    //get dAlpha between points
    qreal angle1 = 0;
    qreal angle2 = 0;
    for (int j = 0; j < 2; j++)
    {
        ModelFragment * frag;
        if (j==0)
            frag=fragA;
        else
            frag=fragB;

        for (int i = 0; i < frag->getEndPoints()->count();i++)
        {
            if (pointsAreCloseEnough(pt1,frag->getEndPoints()->at(i),frag->getProductLines()->first()->getScaleEnum()/4.0))
            {
                angle1 = frag->getEndPointsAngles()->at(i);
            }
            else if (pointsAreCloseEnough(pt2,frag->getEndPoints()->at(i),frag->getProductLines()->first()->getScaleEnum()/4.0))
            {
                angle2 = frag->getEndPointsAngles()->at(i);
            }
        }
    }

    QPointF point1 = *pt1;
    QPointF point2 = *pt2;
    QPointF zero = *pt1;
    if (abs(angle1)>abs(angle2))
    {
        rotatePoint(&point1,-angle1,&zero);
        rotatePoint(&point2,-angle1,&zero);
    }
    else
    {
        rotatePoint(&point1,-angle2,&zero);
        rotatePoint(&point2,-angle2,&zero);
    }

    qreal dx = abs (point1.x()-point2.x());
    qreal dy = abs (point1.y()-point2.y());

    while (angle1<0)
        angle1+=360;
    while (angle1>360)
        angle1-=360;
    while (angle2<0)
        angle2+=360;
    while (angle2>=360)
        angle2-=360;

    dAlpha = abs((angle1)-(angle2));
    if (dAlpha>=160 && dAlpha<=205)
        dAlpha -=180;
    else if (dAlpha<=20)
        dAlpha +=180;
    dAlpha = abs(dAlpha);

    qreal len = sqrt(dx*dx+dy*dy);

    //compute the needed flex
    neededFlex = abs(dAlpha-len);

    ModelItem * firstA = NULL;
    ModelItem * firstB = NULL;
    ModelItem * stopperA = NULL;
    ModelItem * stopperB = NULL;

    //use the last "n" items of both fragments to get maxFlex
    for (int i = 0; i < 2; i++)
    {
        ModelFragment * f;
        QPointF * pt;
        if (i==0)
        {
            f=fragA;
            pt=pt1;
        }
        else
        {
            f=fragB;
            pt=pt2;
        }

        QList<ModelItem*> visited;
        QList<ModelItem*> toVisit;

        ModelItem * current = f->findEndPointItem(pt);
        toVisit.push_back(current);
        while (visited.count()!=n && !toVisit.empty())
        {
            current = toVisit.first();
            toVisit.pop_front();
            maxFlex+=current->getMaxFlex();

            int i = 0;
            while (current->getEndPoint(i)!=NULL)
            {
                if (current->getNeighbour(i)!=NULL && !visited.contains(current->getNeighbour(i)))
                {
                    toVisit.push_front(current->getNeighbour(i));
                    break;
                }
                i++;
            }
            visited.push_back(current);
        }
        if (i==0)
        {
            stopperA = current;
            visited.pop_back();

            //if is needed in cases when one of fragments contains just one item
            if (!visited.empty())
                firstA = visited.last();
        }
        else
        {
            stopperB = current;
            visited.pop_back();
            //if is needed in cases when one of fragments contains just one item
            if (!visited.empty())
                firstB = visited.last();
        }
    }

    logFile << "    Flex needed: " << neededFlex << ", maximal flex: " << maxFlex << endl;
    //compare maxFlex and neededFlex
    qreal lenConstraint = firstA->getProdLine()->getScaleEnum();
    if (firstB->getSlotTrackInfo()==NULL)
        lenConstraint *=3;
    if (maxFlex<neededFlex || len>=lenConstraint || (dAlpha>=10 && dAlpha <=170) || (dAlpha >= 190 && dAlpha <= 350) || (firstA->getProdLine()->getScaleEnum()!=firstB->getProdLine()->getScaleEnum()))
    {        
        app->getAppData()->setMessageDialogText("The points you have selected are too far. Fragments will not be connected.","Vámi vybrané body jsou příliš vzdálené. Tratové segmenty nebudou spojeny.");
        app->getAppData()->getMessageDialog()->exec();
        return 2;
    }

    //apply changes to both fragments
        //rotate n, n-1, n-2,..,1 items. Rotation should be alternating between fragments

    QList<ModelItem*> visited;
    QList<ModelItem*> toVisitA;
    QList<ModelItem*> toVisitB;

    toVisitA.push_back(firstA);
    toVisitB.push_back(firstB);

    visited.push_back(stopperA);
    visited.push_back(stopperB);

    QPointF ptTemp = *pt1;
    QPointF ptTemp2 = *pt2;

    ModelItem * currentA = NULL;
    ModelItem * currentB = NULL;

    //toVisitA and B contain always the same number of items
    while (!toVisitA.empty())
    {
        currentA = toVisitA.first();
        toVisitA.pop_front();

        currentB = toVisitB.first();
        toVisitB.pop_front();

        QPointF rotationCenterA;
        QPointF rotationCenterB;
        int i = 0;
        while (currentA->getEndPoint(i)!=NULL)
        {
            if (visited.contains(currentA->getNeighbour(i)))
            {
                rotationCenterA=*currentA->getEndPoint(i);
                break;
            }
            i++;
        }
        i=0;
        while (currentB->getEndPoint(i)!=NULL)
        {
            if (visited.contains(currentB->getNeighbour(i)))
            {
                rotationCenterB=*currentB->getEndPoint(i);
                break;
            }
            i++;
        }

        qreal dX = ptTemp.x()-ptTemp2.x();
        qreal dY = ptTemp.y()-ptTemp2.y();

        qreal rotationFix = neededFlex/n;// min(neededFlex,maxFlex)/(2*n);
        qreal dxFix = min(1+0*currentA->getProdLine()->getScaleEnum()/8.0,abs(dY/(2*n)));
        qreal dyFix = min(1+0*currentA->getProdLine()->getScaleEnum()/8.0,abs(dX/(2*n)));
        if (dX<0)
            dxFix*=-1;
        if (dY<0)
            dyFix*=-1;


        /*lists of all possible endpoint transformations:
         *  move +
         *  move -
         *  rotate +
         *  rotate -
         *  move + rotate +
         *  move - rotate -
         *  move - rotate +
         *  move + rotate -
         *  rotate + move +
         *  rotate - move -
         *  rotate + move -
         *  rotate - move +
         *  no transformation*/


        QList<QPointF> ptsA;
        QList<QPointF> ptsB;
        for (int i = 0; i < 13; i++)
        {
            ptsA << ptTemp;
            ptsB << ptTemp2;
        }

        //now transform all the points of A
        movePoint(&ptsA[0],dxFix,dyFix);
        movePoint(&ptsA[1],-dxFix,-dyFix);
        rotatePoint(&ptsA[2],rotationFix,&rotationCenterA);
        rotatePoint(&ptsA[3],-rotationFix,&rotationCenterA);

        movePoint(&ptsA[4],dxFix,dyFix);
        rotatePoint(&ptsA[4],rotationFix,&rotationCenterA);
        movePoint(&ptsA[5],-dxFix,-dyFix);
        rotatePoint(&ptsA[5],-rotationFix,&rotationCenterA);
        movePoint(&ptsA[6],-dxFix,-dyFix);
        rotatePoint(&ptsA[6],rotationFix,&rotationCenterA);
        movePoint(&ptsA[7],dxFix,dyFix);
        rotatePoint(&ptsA[7],-rotationFix,&rotationCenterA);

        rotatePoint(&ptsA[8],rotationFix,&rotationCenterA);
        movePoint(&ptsA[8],dxFix,dyFix);
        rotatePoint(&ptsA[9],-rotationFix,&rotationCenterA);
        movePoint(&ptsA[9],-dxFix,-dyFix);
        rotatePoint(&ptsA[10],rotationFix,&rotationCenterA);
        movePoint(&ptsA[10],-dxFix,-dyFix);
        rotatePoint(&ptsA[11],-rotationFix,&rotationCenterA);
        movePoint(&ptsA[11],dxFix,dyFix);

        //flip the direction of the movement
        dxFix*=-1;
        dyFix*=-1;

        //transform all the points of B
        movePoint(&ptsB[0],dxFix,dyFix);
        movePoint(&ptsB[1],-dxFix,-dyFix);
        rotatePoint(&ptsB[2],rotationFix,&rotationCenterB);
        rotatePoint(&ptsB[3],-rotationFix,&rotationCenterB);

        movePoint(&ptsB[4],dxFix,dyFix);
        rotatePoint(&ptsB[4],rotationFix,&rotationCenterB);
        movePoint(&ptsB[5],-dxFix,-dyFix);
        rotatePoint(&ptsB[5],-rotationFix,&rotationCenterB);
        movePoint(&ptsB[6],-dxFix,-dyFix);
        rotatePoint(&ptsB[6],rotationFix,&rotationCenterB);
        movePoint(&ptsB[7],dxFix,dyFix);
        rotatePoint(&ptsB[7],-rotationFix,&rotationCenterB);

        rotatePoint(&ptsB[8],rotationFix,&rotationCenterB);
        movePoint(&ptsB[8],dxFix,dyFix);
        rotatePoint(&ptsB[9],-rotationFix,&rotationCenterB);
        movePoint(&ptsB[9],-dxFix,-dyFix);
        rotatePoint(&ptsB[10],rotationFix,&rotationCenterB);
        movePoint(&ptsB[10],-dxFix,-dyFix);
        rotatePoint(&ptsB[11],-rotationFix,&rotationCenterB);
        movePoint(&ptsB[11],dxFix,dyFix);

        //flip back the direction of the movement
        dxFix*=-1;
        dyFix*=-1;

        qreal minDist = dist12;
        int minIndexA = 12;
        int minIndexB = 12;
        for (int i = 0; i < 13; i++)
        {
            qreal d = 0;
            for (int j = 0; j < 13; j++)
            {
                d = dist(&ptsA[i],&ptsB[j]);
                if (minDist>d)
                {
                    minDist=d;
                    minIndexA=i;
                    minIndexB=j;
                }
            }
        }

        //minIndexA and B store the indices to the transformations which give the best result
        bool rotatePositive = true;
        bool movePositive = true;
        int rotMoveBoth = 0; //0=rotate,1=move,2=rotate and move, 3=move and rotate, 4=no transformation

        QList<ModelItem*> visitedTransform;
        QList<ModelItem*> toVisitTransform;

        visitedTransform.append(visited);
        for (int i = 0; i < 2; i++)
        {
            QPointF center;
            if (i%2==0)
            {
                if (minIndexA<2)
                    rotMoveBoth=1;
                else if (minIndexA<4)
                    rotMoveBoth=0;
                else if (minIndexA<8)
                    rotMoveBoth=3;
                else if (minIndexA<12)
                    rotMoveBoth=2;
                else
                    rotMoveBoth=4;

                if (rotMoveBoth<2)
                {
                    rotatePositive = minIndexA%2==0 ? true : false;
                    movePositive = minIndexA%2==0 ? true : false;
                }
                else if (rotMoveBoth==2)
                {
                    /*
                     RM
                    8++;9--;10+-;11-+;*/
                    rotatePositive = minIndexA%2==0 ? true : false;
                    if (minIndexA==9 || minIndexA==11)
                        movePositive=true;
                    else
                        movePositive=false;
                }
                else if (rotMoveBoth==3)
                {
                    /*
                     MR
                    4++;5--;6-+;7+-;*/
                    rotatePositive = minIndexA%2==0 ? true : false;
                    if (minIndexA==4 || minIndexA==7)
                        movePositive=true;
                    else
                        movePositive=false;
                }
                else
                {
                    rotationFix=0;
                    dxFix=0;
                    dyFix=0;
                }

                if (!movePositive)
                {
                    dxFix*=-1;
                    dyFix*=-1;
                }
                if (!rotatePositive)
                    rotationFix*=-1;
                else
                    rotationFix=abs(rotationFix);


                visitedTransform.push_back(currentA);
                toVisitTransform.push_back(currentA);
                center = rotationCenterA;

                if (rotMoveBoth==0)
                    rotatePoint(&ptTemp,rotationFix,&center);
                else if (rotMoveBoth==1)
                    movePoint(&ptTemp,dxFix,dyFix);
                else if (rotMoveBoth==2)
                {
                    rotatePoint(&ptTemp,rotationFix,&center);
                    movePoint(&ptTemp,dxFix,dyFix);
                }
                else
                {
                    movePoint(&ptTemp,dxFix,dyFix);
                    rotatePoint(&ptTemp,rotationFix,&center);
                }
            }
            else
            {
                if (minIndexB<2)
                    rotMoveBoth=1;
                else if (minIndexB<4)
                    rotMoveBoth=0;
                else if (minIndexB<8)
                    rotMoveBoth=3;
                else if (minIndexB<12)
                    rotMoveBoth=2;
                else
                    rotMoveBoth=4;

                if (rotMoveBoth<2)
                {
                    rotatePositive = minIndexB%2==0 ? true : false;
                    movePositive = minIndexB%2==0 ? true : false;
                }
                else if (rotMoveBoth==2)
                {
                    /*
                     RM
                    8++;9--;10+-;11-+;*/
                    rotatePositive = minIndexB%2==0 ? true : false;
                    if (minIndexB==9 || minIndexB==11)
                        movePositive=true;
                    else
                        movePositive=false;

                }
                else if (rotMoveBoth==3)
                {
                    /*
                     MR
                    4++;5--;6-+;7+-;*/
                    rotatePositive = minIndexB%2==0 ? true : false;
                    if (minIndexB==4 || minIndexB==7)
                        movePositive=true;
                    else
                        movePositive=false;
                }
                else
                {
                    rotationFix=0;
                    dxFix=0;
                    dyFix=0;
                }

                if (!movePositive)
                {
                    dxFix*=-1;
                    dyFix*=-1;
                }
                else
                {
                    dxFix=abs(dxFix);
                    dyFix=abs(dyFix);
                }
                if (!rotatePositive)
                    rotationFix*=-1;
                else
                    rotationFix=abs(rotationFix);


                visitedTransform.push_back(currentB);
                toVisitTransform.push_back(currentB);
                center = rotationCenterB;

                //flip the direction of the movement
                dxFix*=-1;
                dyFix*=-1;

                if (rotMoveBoth==0)
                    rotatePoint(&ptTemp2,rotationFix,&center);
                else if (rotMoveBoth==1)
                    movePoint(&ptTemp2,dxFix,dyFix);
                else if (rotMoveBoth==2)
                {
                    rotatePoint(&ptTemp2,rotationFix,&center);
                    movePoint(&ptTemp2,dxFix,dyFix);
                }
                else
                {
                    movePoint(&ptTemp2,dxFix,dyFix);
                    rotatePoint(&ptTemp2,rotationFix,&center);
                }

            }


            ModelItem * current = NULL;
            while (!toVisitTransform.empty())
            {
                current = toVisitTransform.first();
                toVisitTransform.pop_front();


                int k = 0;
                while (current->getEndPoint(k)!=NULL)
                {
                    if (current->getNeighbour(k)!=NULL && !visitedTransform.contains(current->getNeighbour(k)) && !toVisitTransform.contains(current->getNeighbour(k)))
                    {
                        toVisitTransform.push_back(current->getNeighbour(k));
                    }
                    k++;
                }

                if (rotMoveBoth==0)
                    current->rotate(rotationFix,&center,true);
                else if (rotMoveBoth==1)
                    current->moveBy(dxFix,dyFix);
                else if (rotMoveBoth==2)
                {
                    current->rotate(rotationFix,&center,true);
                    current->moveBy(dxFix,dyFix);
                }
                else if (rotMoveBoth==3)
                {
                    current->moveBy(dxFix,dyFix);
                    current->rotate(rotationFix,&center,true);
                }

                visitedTransform.push_front(current);
            }


        }

        visited.push_back(currentA);
        visited.push_back(currentB);
        i =0;
        while (currentA->getEndPoint(i)!=NULL)
        {
            if (currentA->getNeighbour(i)!=NULL && !visited.contains(currentA->getNeighbour(i)))
            {
                toVisitA.push_front(currentA->getNeighbour(i));
                break;
            }
            i++;
        }
        i=0;
        while (currentB->getEndPoint(i)!=NULL)
        {
            if (currentB->getNeighbour(i)!=NULL && !visited.contains(currentB->getNeighbour(i)))
            {
                toVisitB.push_front(currentB->getNeighbour(i));
                break;
            }
            i++;
        }

        dist12=minDist;
    }

    //modify pt1 and pt2 values
    *pt1=ptTemp;
    *pt2=ptTemp2;

    ModelItem * item1 = fragA->findEndPointItem(pt1);
    ModelItem * item2 = fragB->findEndPointItem(pt2);

    {
        /*
        code above may cause some visible "steps" on the track
        this block of code fixes this problem
        how is it done:
        -get the "width" of the endPointItem which is not at the ptTemp point ("width" = distance between two endpoints)
        -measure the distance between pt1 and pt2
        -have the circular sector with radius equal to "width". The length of the sector is equal to dist(pt1,pt2).
        -angle of the sector is the angle of the rotation which fixes (not 100% perfect) the problem
        */

        /*arc length:
          s = (PI*r*angle)/180
          =>
          180s = PI*r*angle
          180s/rPI = angle
          alpha = 180s/rPI, where s = dist(pt1,pt2)
         */

        qreal r = 0;
        qreal dist12 = 0;
        qreal alpha = 0;
        qreal angle1 = 0;
        qreal angle2 = 0;

        int index1 = item1->getEndPointIndex(pt1);
        int index2 = item2->getEndPointIndex(pt2);

        angle1=item1->getTurnAngle(pt1);
        angle2=item2->getTurnAngle(pt2);

        if ((angle1<0 && angle2>0) || (item1->leftRightDifference180(index1,index1+1) && index1%2==0))
            angle1+=180;
        if ((angle1>0 && angle2<0) || (item2->leftRightDifference180(index2,index2+1) && index2%2==0))
            angle2+=180;

        QPointF point1 = *pt1;
        QPointF point2 = *pt2;

        QPointF center = *pt1;
        rotatePoint(&point1,-angle1,&center);
        rotatePoint(&point2,-angle1,&center);


        ModelItem * toRotate = NULL;
        QPointF * fragPoint = NULL;
        int index = 0;
        if (point1.y()<point2.y())
        {
            toRotate = item1;
            index = index1;
            fragPoint = pt1;
        }
        else
        {
            toRotate = item2;
            index = index2;
            fragPoint = pt2;
        }

        if (index%2==0)
        {
            qreal dxItem = toRotate->getEndPoint(index)->x()-toRotate->getEndPoint(index+1)->x();
            qreal dyItem = toRotate->getEndPoint(index)->y()-toRotate->getEndPoint(index+1)->y();
            r = sqrt(dxItem*dxItem+dyItem*dyItem);
            dist12 = abs(point1.y()-point2.y());

        }
        else
        {
            qreal dxItem = toRotate->getEndPoint(index)->x()-toRotate->getEndPoint(index-1)->x();
            qreal dyItem = toRotate->getEndPoint(index)->y()-toRotate->getEndPoint(index-1)->y();
            r = sqrt(dxItem*dxItem+dyItem*dyItem);
            dist12 = abs(point1.y()-point2.y());
        }

        alpha = (180*dist12)/(r*PI);

        //now rotate "test point" - if it is closer to "the other end point", rotate,
        //otherwise rotate in opposite direction
        QPointF testPoint(*fragPoint);
        QPointF testPointNeg(*fragPoint);
        if (index%2==0)
        {
            rotatePoint(&testPoint,alpha,toRotate->getEndPoint(index+1));
            rotatePoint(&testPointNeg,-alpha,toRotate->getEndPoint(index+1));
        }
        else
        {
            rotatePoint(&testPoint,alpha,toRotate->getEndPoint(index-1));
            rotatePoint(&testPointNeg,-alpha,toRotate->getEndPoint(index-1));
        }

        qreal testLen = 0;
        qreal testLenNeg = 0;

        if (fragPoint==pt1)
        {
            testLen = dist(&testPoint,pt2);
            testLenNeg = dist (&testPointNeg,pt2);
        }
        else
        {
            testLen = dist(&testPoint,pt1);
            testLenNeg = dist (&testPointNeg,pt1);
        }

        //update length of dx,dy vector
        dx = abs (point1.x()-point2.x());
        dy = abs (point1.y()-point2.y());
        len = sqrt(dx*dx+dy*dy);

        if (testLen>len || testLenNeg<testLen)
            alpha*=-1;

        //now do the final rotation
        if (index%2==0)
        {
            toRotate->rotate(alpha,toRotate->getEndPoint(index+1),true);
        }
        else
        {
            toRotate->rotate(alpha,toRotate->getEndPoint(index-1),true);
        }

        if (toRotate==item1)
            ptTemp=*item1->getEndPoint(index);
        else
            ptTemp2=*item2->getEndPoint(index);
    }

    //modify pt1 and pt2 values again
    *pt1=ptTemp;
    *pt2=ptTemp2;

    fragA->updateEndPointsGraphics();
    fragB->updateEndPointsGraphics();

    //call connect fragments
    this->connectFragments(pt1,pt2,fragA,fragB,item1,item2);

    return 0;
}

int WorkspaceWidget::completeFragment(ModelFragment *frag, QPointF *pt1, QPointF *pt2)
{

    QList<QAction*> actions = app->getWindow()->getMainToolBar()->actions();
    for (int i = 0; i < actions.count(); i++)
    {
        if (actions[i]->text().startsWith("Complete") || actions[i]->text().startsWith("Dokončit"))
            actions[i]->setChecked(false);
    }

    /*

//BASIC VERSION OF ALGORITHM
      //STEP 1: get control points of Bezier curve

      get angles at points pt1 and pt2, length of the vector between them
      make pt12 and pt22 - their values equal to pt1/pt2. set x value to x + length/2, then rotate point by the angle at point
      now you probably have the control points of a Bezier curve

      //Bezier curve:




        //STEP 2:
        now you have the Bezier curve and using pointOnBezierCubic() function you can get any point on the curve

        //STEP 3:
        get "ideal item" radius (ideal item fits perfectly the radius of curve's first half) or ideal item length (if it is straight part)
            PointA: u=0, PointB: u=0.5, PointC: u=0.25
            approximation: points ABC are on the circle => you can get the radius of the ideal item
            if points ABC are in line, set bool flag

            //version 2:
                A: u=0, B: u=0.5, angle at A, angle at B (=A+1/2 dAlphaP1P2)
                place points on the circle, get radius of ideal item

        //STEP 4:
        if points from step 3 are in line:
            try to find the right item to insert
                (itemLength)*(almost natural number (+-0.1) )=ideal item length
        else
            try to find the item with almost ideal radius and angle

        //STEP 5:
        insert item (or alternative item). Keep in mind, that slot track still needs lane-switching

        //STEP 6:
        measure the distance between new active endPoint and Bezier curve
            Bezier: u=0.5 (or 0.25, see step 3) will be used as a start point and try to minimize the distance
                by incrementing (and then also decrementing) the value of u
            Distance should be less than some tolerance constant (not the TOLERANCE_HALF)
                if the distance is larger set flag for new Bezier

*/

    ModelFragment * fragA = this->findFragmentByApproxPos(pt1);
    ModelFragment * fragB = this->findFragmentByApproxPos(pt2);


    if (fragA==fragB)
        logFile << "    Complete fragment with id " << frag->getID() << ". Selected points: (" << pt1->x() << ", " << pt1->y() << ")" << " and (" << pt2->x() << ", " << pt2->y() << ")" << endl;
    else
        logFile << "    Complete two fragments: " << endl;

    /*
    //STEP 0
    QPointF controlPoint1,2,3,4,activePt = pt1;*/

    QPointF cp1,cp2,cp3,cp4,activePt;

    activePt=*pt1;

    bool mayExceedSceneSize=false;
    bool sceneExceededForTheFirstTime = true;

    QRectF rect(pt2->x()-150,pt2->y()-150,300,300);
    QRectF smallRect(pt2->x()-75,pt2->y()-75,150,150);

    bool makeNewBezier = true;
    qreal u = 0;

    bool checkLargeRect = false;
    int infiniteCounter=0;
    qreal lastDistance = 0;
    Bezier curve;
    qreal bestMatchDistAccumulated = 0;

    while(!(smallRect.contains(activePt) && !infiniteCounter>3) && !frag->getEndPoints()->empty())
    {
        if (rect.contains(activePt) && infiniteCounter>3)
            checkLargeRect=true;

        /*//STEP 1
        get angles at points activePt and pt2, length of the vector between them
        make pt12 and pt22 - their values are equal to pt1/pt2. set x value to x + length/2, then rotate point by the angle at point
        now you probably have the control points of a Bezier curve*/

        if (fragA->findEndPointItem(&activePt)==NULL)
            break;
        if (fragB->findEndPointItem(pt2)==NULL)
            break;
        qreal angle1 = fragA->findEndPointItem(&activePt)->getTurnAngle(&activePt);
        qreal angle2 = fragB->findEndPointItem(pt2)->getTurnAngle(pt2);
        qreal dx = activePt.x()-pt2->x();
        qreal dy = activePt.y()-pt2->y();
        qreal distance12 = sqrt(dx*dx+dy*dy);
        qreal dAlpha12 = abs(angle1-angle2);

        if (checkLargeRect)
        {
            if (lastDistance<distance12)
            {
                logFile << "Increasing distance, complete fragment is ending" << endl;
                infiniteCounter=257;
            }
        }

        lastDistance=distance12;

        cp1 = activePt;
        cp2 = activePt;
        cp3 = *pt2;
        cp4 = *pt2;

        bool oppositeAngles = false;
        {

            qreal angle1C=angle1;
            qreal angle2C=angle2;
            while (angle1C<0)
                angle1C+=360;
            while (angle2C<0)
                angle2C+=360;
            while (angle1C>360)
                angle1C-=360;
            while (angle2C>360)
                angle2C-=360;
            //dAlpha12=abs(angle1-angle2)+180;
            if (fragA!=fragB)
                dAlpha12=abs((angle1-angle2));
            else
                dAlpha12=abs((angle1-angle2)-180);


            QPointF line1pt1 = activePt;
            QPointF line1pt2 = activePt;
            QPointF line2pt1 = *pt2;
            QPointF line2pt2 = *pt2;
            line1pt2.setX(line1pt2.x()+50);
            line2pt2.setX(line2pt2.x()+50);
            rotatePoint(&line1pt2,angle1C,&line1pt1);
            rotatePoint(&line2pt2,angle2C,&line2pt1);

            qreal dx1 = line1pt1.x()-line1pt2.x();
            qreal dx2 = line2pt1.x()-line2pt2.x();
            qreal dy1 = line1pt1.y()-line1pt2.y();
            qreal dy2 = line2pt1.y()-line2pt2.y();

            /**
             *SOURCE deCasteljau:
             http://www.cs.rutgers.edu/~venugopa/parallel_summer2012/ge.html
            */

            /**
             Ax+By=C
             Dx+Ey=F
             ==>
             x=(CE-BF)/(AE-BD)
             y=(AF-CD)/(AE-BD)
            */
            qreal A,B,C,D,E,F;
            A=dy1;
            B=dx1;
            //C=-dy1*(-line1pt1.x())+(dx1)*(-line1pt1.y());
            C=-dy1*(-line1pt1.x())+(dx1)*(-line1pt1.y());
            D=dy2;
            E=dx2;
            //F=-dy2*(-line2pt1.x())+(dx2)*(-line2pt1.y());
            F=-dy2*(-line2pt1.x())+(dx2)*(-line2pt1.y());
            qreal solutionX,solutionY;
            solutionX=(C*E-B*F)/(A*E-B*D);
            solutionY=-(A*F-C*D)/(A*E-B*D);

            QPointF solution(solutionX,solutionY);

            //now measure the distance from line1pt1 to solution
            qreal solutionDist = dist(&solution,&line1pt1);
            //make new point ptTemp = line1pt1, x+=distance
            QPointF ptTemp(line1pt1.x()+solutionDist,line1pt1.y());

            //rotate ptTemp by angle1
            rotatePoint(&ptTemp,(angle1),&line1pt1);
            //ptTemp "==" solution -> opposite=false

            if (!pointsAreCloseEnough(&ptTemp,&solution,TOLERANCE_HALF))
                oppositeAngles=true;

            logFile << "    equations solution: " << solution.x() << ", " << solution.y() << endl;
            logFile << "    dAlpha12: " << dAlpha12 << endl;

            //if (dAlpha12<10)
            //    oppositeAngles=false;

        }

        qreal multFactor=1;
        if (!fragA->getProductLines()->first()->getType())
            multFactor=0.75;
        else if (fragA != fragB)
        {
            if (dAlpha12<70)
                multFactor=1;
            else if (dAlpha12<120)
                multFactor=2;
            else if (dAlpha12<210 && oppositeAngles)
                multFactor=0.05;
        }

        if (dAlpha12<15 && oppositeAngles)
        {
            cp2.setX(cp2.x()+distance12/(multFactor*1.5));
            cp3.setX(cp3.x()+distance12/(multFactor*1.5));
        }
        else if (oppositeAngles)
        {
            cp2.setX(cp2.x()+distance12/(multFactor*1));
            cp3.setX(cp3.x()+distance12/(multFactor*1));
        }
        else if (distance12<(fragA->getProductLines()->first()->getMinRadius()+fragA->getProductLines()->first()->getMaxRadius())/2)
        {
            cp2.setX(cp2.x()+distance12/(multFactor*2.75));
            cp3.setX(cp3.x()+distance12/(multFactor*2.75));
        }
        else
        {
            cp2.setX(cp2.x()+distance12/(multFactor*2));
            cp3.setX(cp3.x()+distance12/(multFactor*2));
        }

        if (abs(abs(angle1-angle2)-180)<5)
        {
            //in this case the Bezier curve is a straight line -> it would cause infinite loop (straight item fits perfectly -> insert it again, again...
            rotatePoint(&cp2,3,&activePt);
            rotatePoint(&cp3,3,pt2);
        }
        rotatePoint(&cp2,angle1,&activePt);
        rotatePoint(&cp3,angle2,pt2);

        if (dAlpha12>300 || (this->lastInserted->getSlotTrackInfo()!=NULL && dAlpha12 > 150 && dAlpha12 < 210 && fragA==fragB))// || dAlpha<60)
        {
            if (angle2>0)
                rotatePoint(&cp3,-45,pt2);
            else
                rotatePoint(&cp3,45,pt2);
        }

        //STEP 2
        //Bezier curve;
        if (makeNewBezier)
        {
            curve = Bezier(cp1,cp2,cp3,cp4);
            u=0;
            bestMatchDistAccumulated=0;
            makeNewBezier=false;
        }

        /* //STEP 3
        get "ideal item" radius (ideal item fits perfectly the radius of curve's first half) or ideal item length (if it is straight part)
            PointA: u=0, PointB: u=0.5, PointC: u=0.25
            approximation: points ABC are on the circle => you can get the radius of the ideal item
            if points ABC are in line, set bool flag

            //version 2:
                A: u=0, B: u=0.5, angle at A, angle at B (=A+1/2 dAlphaP1P2?)
                place points on the circle, get radius of ideal item

        pointsAreInLine=false;
        rotate ABC so that they are in "neutral" angle -> if they are in line, set the flag to true

        */

        qreal r = 0;
        qreal r2 = 0;
        QPointF ptA,ptB,ptC,ptD,ptE;

        ptA = curve.getPointAtU(u+0);
        ptB = curve.getPointAtU(u+0.166);
        ptC = curve.getPointAtU(u+0.33);

        ptD = curve.getPointAtU(u+0.25);
        ptE = curve.getPointAtU(u+0.5);

        qreal BC,AC,AB,AD,AE,DE;
        BC=dist(&ptC,&ptB);
        AC=dist(&ptA,&ptC);
        AB=dist(&ptA,&ptB);
        DE=dist(&ptD,&ptE);
        AD=dist(&ptA,&ptD);
        AE=dist(&ptA,&ptE);


        //-use vectors to compute angle sinAlphaABC(ADE)
        //r=a/sin (alpha) where a = BC or DE
        qreal dot = (ptB.x()-ptA.x())*(ptC.x()-ptA.x())+(ptB.y()-ptA.y())*(ptC.y()-ptA.y());
        qreal sinAlphaABC = sin(acos(dot/(AB*AC)));
        dot = (ptD.x()-ptA.x())*(ptE.x()-ptA.x())+(ptD.y()-ptA.y())*(ptE.y()-ptA.y());
        qreal sinAlphaADE = sin(acos(dot/(AD*AE)));

        r= BC/(2*sinAlphaABC);
        r2=DE/(2*sinAlphaADE);

        bool straightItem=false;

        /* //STEP 4
        find the ideal item */

        ModelItem * bestMatch = NULL;
        ModelItem * alternativeItem = NULL;
        qreal idealRadius = abs(r);
        qreal idealAngle = (180*(AB+BC))/(idealRadius*PI);;
        qreal idealLength = AC;

        qreal idealRadius2 = abs(r2);
        qreal idealAngle2 = (180*(AD+DE))/(idealRadius2*PI);;
        qreal idealLength2 = AE;

        if (idealAngle<2.5 || idealAngle2<2.5)
            straightItem=true;

        bestMatch = fragA->getProductLines()->first()->getItemsList()->first();
        alternativeItem = fragA->getProductLines()->first()->getItemsList()->first();
        /*
        foreach(item of frag->getProductLines()->first())
            if ((abs(item.radius-idealRadius)<abs(bestMatch.radius-idealRadius) //if radius is closer to ideal
                || (angle is closer to idealAngle && radius is not worse than bestMatch))
                itemIsBetter=true;
            if (radii of bestMatch and item are almost the same && item.angle/idealAngle = natural number +-0.1)
                itemIsBetter=true;
            if (idealLength "==" 2.5*item.length)
                itemIsBetter=true;

            if (itemIsBetter)
                alternative = best
                best = item
        endloop
        */


        /*
         *How to choose the "bestMatch" item:
         IdealIsStraight
         -itemWidth is as close as possible to ideal distance*: +=2
         -
         Else
         -radius is as close as possible: +=2
         -dAlpha*n is as close as possible* && dRadius(this,current best) ==0: +=3 				// *fract(idealDAlpha/dAlpha)<0.1
         -dAlpha*n is as close as possible* && dRadius(this,current best) is less than 5*track gauge: +=2
         -dAlpha*n is as close as possible* && dRadius(this,current best) is larger than 5*track gauge: +=1


         if score is the same && n < than n(bestMatch): +=1

        if (scoreBestMatch is worse)
         change bestMatch
        if (scoreBestMatch is better but scoreAlternative is worse)
         change alternative
        */

        int scoreBest = 0;
        int scoreAlternative = 0;
        int scoreCurrent = 0;
        int scoreCurrent2 = 0;

        QList<ModelItem*>* itemList = fragA->getProductLines()->first()->getItemsList();
        for (int i = 0; i < itemList->count();i++)
        {
            ModelItem * item = itemList->at(i);
            scoreCurrent=0;
            scoreCurrent2=0;

            bool itemInListIsStraight = true;
            if (item->getType()==C1 || item->getType()==C2 || item->getType()==CB
                    || item->getType()==J1 || item->getType()==J2 || item->getType()==J3 || item->getType()==HC
                    )
                itemInListIsStraight=false;

            if (itemList->at(i)->getType()==E1
                    || (itemList->at(i)->getType()>=T1 && itemList->at(i)->getType()<=T10)
                    || (itemInListIsStraight && !straightItem && fragA==fragB)
                    || (!itemInListIsStraight && straightItem && fragA==fragB)
                    || item->getType()==CB
                    )
                continue;

            qreal itemDAlpha = abs(2*item->getTurnAngle(0));

            double intpart = 0;

            if (itemInListIsStraight)
            {
                qreal modFResult = modf(idealLength/item->getItemWidth(),&intpart);
                qreal modFResult2 = modf(idealLength2/item->getItemWidth(),&intpart);

                if (modFResult<0.1 || modFResult >0.9)
                    scoreCurrent+=3;
                else if (modFResult<0.2 || modFResult >0.8)
                    scoreCurrent+=2;
                else if (modFResult<0.3 || modFResult >0.7)
                    scoreCurrent+=1;

                if (modFResult2<0.1 || modFResult2 >0.9)
                    scoreCurrent2+=3;
                else if (modFResult2<0.2 || modFResult2>0.8)
                    scoreCurrent2+=2;
                else if (modFResult2<0.3 || modFResult2>0.7)
                    scoreCurrent2+=1;

                if ((bestMatch->getType()==C1 || bestMatch->getType()==C2 || bestMatch->getType()==CB
                        || bestMatch->getType()==J1 || bestMatch->getType()==J2 || bestMatch->getType()==J3 || bestMatch->getType()==HC
                        ))
                    scoreCurrent+=2;
                if ((alternativeItem->getType()==C1 || alternativeItem->getType()==C2 || alternativeItem->getType()==CB
                        || alternativeItem->getType()==J1 || alternativeItem->getType()==J2 || alternativeItem->getType()==J3 || alternativeItem->getType()==HC
                        ))
                    scoreCurrent2+=2;

                if (!straightItem)
                    scoreCurrent/=2;
            }
            else
            {
                if (abs(item->getRadius()-idealRadius)<=3)
                    scoreCurrent+=6;
                else if (abs(item->getRadius()-idealRadius)<=item->getProdLine()->getScaleEnum())
                    scoreCurrent+=5;
                else if (abs(item->getRadius()-idealRadius)<=2*item->getProdLine()->getScaleEnum())
                    scoreCurrent+=4;
                else if (abs(item->getRadius()-idealRadius)<=3*item->getProdLine()->getScaleEnum())
                    scoreCurrent+=3;
                if (abs(item->getRadius()-idealRadius)<=abs(bestMatch->getRadius()-idealRadius))
                    scoreCurrent+=2;

                if (abs(item->getRadius()-idealRadius2)<=item->getProdLine()->getScaleEnum())
                    scoreCurrent2+=5;
                else if (abs(item->getRadius()-idealRadius2)<=2*item->getProdLine()->getScaleEnum())
                    scoreCurrent2+=4;
                else if (abs(item->getRadius()-idealRadius2)<=3*item->getProdLine()->getScaleEnum())
                    scoreCurrent2+=3;
                if (abs(item->getRadius()-idealRadius2)<=abs(alternativeItem->getRadius()-idealRadius2))
                    scoreCurrent2+=2;

                qreal modFResult = modf(idealAngle/itemDAlpha,&intpart);
                qreal modFResult2 = modf(idealAngle2/itemDAlpha,&intpart);
                qreal modFResult3 = modf(itemDAlpha/idealAngle,&intpart);
                qreal modFResult4 = modf(itemDAlpha/idealAngle2,&intpart);

                if (itemDAlpha<idealAngle)
                {
                    if ((modFResult<0.1 || modFResult>0.9) && (abs(item->getRadius()-idealRadius)<=3))
                        scoreCurrent+=4;
                    else if ((modFResult<0.3 || modFResult>0.7) && (abs(item->getRadius()-idealRadius)<=3))
                        scoreCurrent+=3;
                    else if ((modFResult<0.3 || modFResult>0.7) && (abs(item->getRadius()-idealRadius)<=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent+=2;
                    else if ((modFResult<0.3 || modFResult>0.7) && (abs(item->getRadius()-idealRadius)>=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent+=1;
                }
                else
                {
                    if ((modFResult3<0.1 || modFResult3>0.9) && (abs(item->getRadius()-idealRadius)<=3))
                        scoreCurrent+=3;
                    else if ((modFResult3<0.3 || modFResult3>0.7) && (abs(item->getRadius()-idealRadius)<=3))
                        scoreCurrent+=3;
                    else if ((modFResult3<0.3 || modFResult3>0.7) && (abs(item->getRadius()-idealRadius)<=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent+=2;
                    else if ((modFResult3<0.3 || modFResult3>0.7) && (abs(item->getRadius()-idealRadius)>=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent+=1;
                }
                if (itemDAlpha<idealAngle2)
                {
                    if ((modFResult2<0.1 || modFResult2>0.9) && (abs(item->getRadius()-idealRadius2)<=3))
                        scoreCurrent2+=4;
                    else if ((modFResult2<0.3 || modFResult2>0.7) && (abs(item->getRadius()-idealRadius2)<=3))
                        scoreCurrent2+=3;
                    else if ((modFResult2<0.3 || modFResult2>0.7) && (abs(item->getRadius()-idealRadius2)<=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent2+=2;
                    else if ((modFResult2<0.3 || modFResult2>0.7) && (abs(item->getRadius()-idealRadius2)>=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent2+=1;
                }
                else
                {
                    if ((modFResult4<0.1 || modFResult4>0.9) && (abs(item->getRadius()-idealRadius2)<=3))
                        scoreCurrent2+=4;
                    else if ((modFResult4<0.3 || modFResult4>0.7) && (abs(item->getRadius()-idealRadius2)<=3))
                        scoreCurrent2+=3;
                    else if ((modFResult4<0.3 || modFResult4>0.7) && (abs(item->getRadius()-idealRadius2)<=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent2+=2;
                    else if ((modFResult4<0.3 || modFResult4>0.7) && (abs(item->getRadius()-idealRadius2)>=5*item->getProdLine()->getScaleEnum()))
                        scoreCurrent2+=1;
                }
            }
            if (item->getAvailableCount()<1)
            {
                scoreCurrent=0;
                scoreCurrent2=0;
            }
            if (item->getType()==J1 || item->getType()==J2 || item->getType()==J3 || item->getType()==J4 || item->getType()==J5 || item->getType()==X1)
            {
                if (fragA!=fragB && dAlpha12>=150 && dAlpha12<=210 && (item->getType()==J1 || item->getType()==J2 || item->getType()==J3))
                {
                    scoreCurrent+=3;
                    scoreCurrent2+=3;
                }
                else
                {
                    scoreCurrent-=2;
                    scoreCurrent2-=2;
                }
                if (!(bestMatch->getType()==J1 || bestMatch->getType()==J2 || bestMatch->getType()==J3 || bestMatch->getType()==J4 || bestMatch->getType()==J5 || bestMatch->getType()==X1))
                {
                    while (scoreBest <= scoreCurrent)
                        scoreCurrent--;
                }
                if (!(alternativeItem->getType()==J1 || alternativeItem->getType()==J2 || alternativeItem->getType()==J3 || alternativeItem->getType()==J4 || alternativeItem->getType()==J5 || alternativeItem->getType()==X1))
                {
                    while (scoreAlternative <= scoreCurrent2)
                        scoreCurrent2--;
                }
            }
            if (item->getItemWidth()*0.75>distance12)
            {
                scoreCurrent-=2;
                scoreCurrent2-=2;
            }
            if (itemDAlpha<1)
                itemDAlpha++;
            if (idealAngle<1)
                idealAngle++;
            if (idealAngle2<1)
                idealAngle2++;

            if (alternativeItem->getTurnAngle(0)>=1 && bestMatch->getTurnAngle(0)>=1)
            {
                if (idealAngle>itemDAlpha)
                {
                    if (scoreCurrent==scoreBest && ((int)idealAngle/(int)itemDAlpha < (int)idealAngle/(int)abs(2*bestMatch->getTurnAngle(0))))
                        scoreCurrent++;
                }
                else
                {
                    if (scoreCurrent==scoreBest && ((int)itemDAlpha/(int)idealAngle < (int)abs(2*bestMatch->getTurnAngle(0))/(int)idealAngle))
                        scoreCurrent++;
                }
                if (idealAngle2>itemDAlpha)
                {
                    if (scoreCurrent2==scoreAlternative && ((int)idealAngle2/(int)itemDAlpha < (int)idealAngle2/(int)abs(2*alternativeItem->getTurnAngle(0))))
                        scoreCurrent2++;
                }
                else
                {
                    if (scoreCurrent2==scoreAlternative && ((int)itemDAlpha/(int)idealAngle2 < (int)abs(2*alternativeItem->getTurnAngle(0)/(int)idealAngle2)))
                        scoreCurrent2++;
                }
            }

            if (item->getSlotTrackInfo()!=NULL)
            {
                if (item->getType()==HE || item->getType()==HS || item->getType()==HC || item->getSlotTrackInfo()->getLanesGauge()!=this->lastInserted->getSlotTrackInfo()->getLanesGauge())
                {
                    //bad score was intended
                    scoreCurrent=-1;
                    scoreCurrent2=-1;
                }
            }


            bool oneAssignment = false;



            if (scoreBest < scoreCurrent)
            {
                bestMatch=item;
                scoreBest=scoreCurrent;
                oneAssignment=true;
            }
            else if (scoreBest < scoreCurrent2)
            {
                bestMatch=item;
                scoreBest=scoreCurrent2;
                oneAssignment=true;
            }

            if (scoreAlternative < scoreCurrent && !oneAssignment)
            {
                alternativeItem=item;
                scoreAlternative=scoreCurrent;
            }
            else if (scoreAlternative < scoreCurrent2 && !oneAssignment)
            {
                alternativeItem=item;
                scoreAlternative=scoreCurrent2;
            }




            logFile << "        currentItem: " << item->getPartNo()->toStdString() << ", scoreCurrent: " << scoreCurrent << ", scoreCurrent2: " << scoreCurrent2 << endl;
        }


        if (scoreBest==scoreAlternative && alternativeItem->getTurnAngle(0)>=1 && bestMatch->getTurnAngle(0)>=1)
        {

            if (idealAngle>abs(2*bestMatch->getTurnAngle(0)))
            {
                if ((int)idealAngle/(int)abs(2*alternativeItem->getTurnAngle(0)) < (int)idealAngle/(int)abs(2*bestMatch->getTurnAngle(0)))
                    scoreAlternative++;
                else
                    scoreBest++;

            }
            else
            {
                if ((int)abs(2*alternativeItem->getTurnAngle(0))/(int)idealAngle < (int)abs(2*bestMatch->getTurnAngle(0))/(int)idealAngle)
                    scoreAlternative++;
                else
                    scoreBest++;
            }
            if (idealAngle2>abs(2*alternativeItem->getTurnAngle(0)))
            {
                if ((int)idealAngle2/(int)abs(2*bestMatch->getTurnAngle(0)) < (int)idealAngle2/(int)abs(2*alternativeItem->getTurnAngle(0)))
                    scoreBest++;
                else
                    scoreAlternative++;

            }
            else
            {
                if ((int)abs(2*bestMatch->getTurnAngle(0))/(int)idealAngle2 < (int)abs(2*alternativeItem->getTurnAngle(0))/(int)idealAngle2)
                    scoreBest++;
                else
                    scoreAlternative++;
            }

        }

        u+=1/5;
        if (scoreBest < scoreAlternative)
        {
            swap(bestMatch,alternativeItem);
            swap(scoreBest,scoreAlternative);
        }

        //now decide whether left or right turn will be inserted:
        //-copy bestMatch's points and angles [0] and [1]
        //-move and rotate them so that they simulate right turn item (angles aren't changed)
        //-measure distance to curve at u
        //-reset points, move and rotate them to simulate left turn item (-> "swap" angles)
        //-measure distance
        //if left dist<right dist -> r*=-1;
        QPointF bM0(0*bestMatch->getEndPoint(0)->x()+activePt.x(),0*bestMatch->getEndPoint(0)->y()+activePt.y());
        QPointF bM1(2*bestMatch->getEndPoint(1)->x()+activePt.x(),0*bestMatch->getEndPoint(1)->y()+activePt.y());
        qreal bMA0 = bestMatch->getTurnAngle(0);
        qreal bMA1 = bestMatch->getTurnAngle(1);

        bestMatchDistAccumulated+=bestMatch->getItemWidth();
        QPointF zeroU = curve.getPointAtU(0);
        QPointF thirdU = curve.getPointAtU(0.33);
        qreal newU = 0.3333*bestMatchDistAccumulated/dist(&zeroU,&thirdU);
        QPointF ptAtU = curve.getPointAtU(newU);

        rotatePoint(&bM1,angle1+bMA1,&bM0);
        qreal distR = dist(&bM1,&ptAtU);

        bM1 = QPointF(2*bestMatch->getEndPoint(1)->x()+activePt.x(),0*bestMatch->getEndPoint(1)->y()+activePt.y());
        rotatePoint(&bM1,angle1+bMA0,&bM0);
        qreal distL = dist(&bM1,&ptAtU);
        if (distL < distR)
            r*=-1;

        logFile << QString("    InfiniteCounter: %1, idealRadius: %2, r: %3, idealAngle: %4").arg(QString::number(infiniteCounter),QString::number(idealRadius),QString::number(r),QString::number(idealAngle)).toStdString() << endl;
        logFile << QString("                        idealRadius2: %1, r2: %2, idealAngle2: %3").arg(QString::number(idealRadius2),QString::number(r2),QString::number(idealAngle2)).toStdString() << endl;
        logFile << QString("    BestMatch: %1, radius: %2, angle at 0: %3, score: ").arg(*bestMatch->getPartNo(),QString::number(bestMatch->getRadius()),QString::number(bestMatch->getTurnAngle(0))).toStdString() << scoreBest << endl;
        logFile << QString("    AlternativeMatch: %1, radius: %2, angle at 0: %3, score: ").arg(*alternativeItem->getPartNo(),QString::number(alternativeItem->getRadius()),QString::number(alternativeItem->getTurnAngle(0))).toStdString() << scoreAlternative << endl;

        //STEP 5:
        //insert item (or alternative item). Keep in mind, that slot track still needs lane-switching
        if (bestMatch->getSlotTrackInfo()!=NULL)
        {
            ModelItem * it = this->lastInserted;
            if (pointsAreCloseEnough(pt1,&activePt,bestMatch->getProdLine()->getScaleEnum()/4.0))
                it = this->findItemByApproxPos(&activePt,NULL);

            if ((r<0 && it->getRadius()>0  && it->getEndPointIndex(&activePt)%2==1)
                    || (r>0 && it->getRadius()<0 && it->getEndPointIndex(&activePt)%2==1)
                    || (r>0 && it->getRadius()>0 && it->getEndPointIndex(&activePt)%2==0)
                    || (r<0 && it->getRadius()<0 && it->getEndPointIndex(&activePt)%2==0))
            {
                int index = it->getEndPointIndex(&activePt);
                if (index!=-1)
                {
                    while(it->getEndPoint(index)!=NULL)
                    {
                        index+=2;
                    }
                    if (index>2)
                        index-=2;
                    activePt = *it->getEndPoint(index);
                }
            }
        }

        /*
            if distance < 600
            {
                check these aditional cases:
                    aL,bL
                    aR,bL
                    aL,bR
                    aR,bR where L/R stands for left/right item and a=alternativeItem and b=bestMatch
                check how the angle AND distance are changed
                    what is better: "y" difference of ("item"EP,pt2) is decreased && dAlpha is at least the same or better
                then insert the best combo (bL,bR or some of the four new posibilities)
            }
            else
                standard insertion - see below
        */

        if (distance12 < 600 && dAlpha12<100) //600 = 2*size of large rect
        {
            //at first, test aditional cases which may help to choose the best combo of items
            //aLbL,aRbL,aLbR,aRbR,bLaL,bRaL,bLaR,bRaR
            QList<QPointF> listOfPoints;
            QList <qreal> listOfAngles;

            for (int i = 0; i < 8; i++)
            {
                listOfPoints << QPointF(activePt.x(),activePt.y());
                listOfAngles << angle1;
            }
            for (int i = 0; i < 4; i++)
                listOfPoints[i].setX(listOfPoints[i].x()+alternativeItem->getItemWidth());
            for (int i = 4; i < 8; i++)
                listOfPoints[i].setX(listOfPoints[i].x()+bestMatch->getItemWidth());

            //rotate points so that they are pointing in the same direction as lastInsertedItem
            for (int i = 0; i < 8; i++)
                rotatePoint(&listOfPoints[i],angle1,&activePt);

            //simulate the direction of alternative or best item
            for (int i = 0; i < 4; i++)
            {
                rotatePoint(&listOfPoints[i],2*alternativeItem->getTurnAngle(i%2),&activePt);
                listOfAngles[i]+=2*alternativeItem->getTurnAngle(i%2);
            }
            for (int i = 4; i < 8; i++)
            {
                rotatePoint(&listOfPoints[i],2*bestMatch->getTurnAngle(i%2),&activePt);
                listOfAngles[i]+=2*bestMatch->getTurnAngle(i%2);
            }

            //modify coordinates
            for (int i = 0; i < 4; i++)
                listOfPoints[i].setX(listOfPoints[i].x()+bestMatch->getItemWidth());
            for (int i = 4; i < 8; i++)
                listOfPoints[i].setX(listOfPoints[i].x()+alternativeItem->getItemWidth());

            //simulate the direction after the first "inserted" item
            for (int i = 0; i < 8; i++)
                rotatePoint(&listOfPoints[i],angle1,&activePt);
            for (int i = 0; i < 4; i++)
                rotatePoint(&listOfPoints[i],2*alternativeItem->getTurnAngle(i%2),&activePt);
            for (int i = 4; i < 8; i++)
                rotatePoint(&listOfPoints[i],2*bestMatch->getTurnAngle(i%2),&activePt);

            //simulate the direction of the second alternative or best item
            for (int i = 0; i < 2; i++)
            {
                rotatePoint(&listOfPoints[i],2*bestMatch->getTurnAngle(0),&activePt);
                listOfAngles[i]+=2*bestMatch->getTurnAngle(0);
            }
            for (int i = 2; i < 4; i++)
            {
                rotatePoint(&listOfPoints[i],2*bestMatch->getTurnAngle(1),&activePt);
                listOfAngles[i]+=2*bestMatch->getTurnAngle(1);
            }
            for (int i = 4; i < 6; i++)
            {
                rotatePoint(&listOfPoints[i],2*alternativeItem->getTurnAngle(0),&activePt);
                listOfAngles[i]+=2*alternativeItem->getTurnAngle(0);
            }
            for (int i = 6; i < 8; i++)
            {
                rotatePoint(&listOfPoints[i],2*alternativeItem->getTurnAngle(1),&activePt);
                listOfAngles[i]+=2*alternativeItem->getTurnAngle(1);
            }

            QList <qreal> yDistanceList;
            QList <qreal> dAlphaList;


            //rotate all points in the neutral position
            QPointF activePtCopy = activePt;
            rotatePoint(&activePtCopy,-angle1,&activePt);


            bM1 = QPointF(2*bestMatch->getEndPoint(1)->x()+activePt.x(),0*bestMatch->getEndPoint(1)->y()+activePt.y());
            if (r<0)
                rotatePoint(&bM1,angle1+bMA0,&bM0);
            else
                rotatePoint(&bM1,angle1+bMA1,&bM0);

            rotatePoint(&bM1,-angle2,pt2);

            //start with the distance equal to the state after insertion of just one item
            //(because you need to know what is better - combo or just best item
            qreal minDistY=abs(bM1.y()-pt2->y());
            int minDistIndex=-1;//-1=>bestMatchItem is the best choice

            for (int i = 0; i < 8; i++)
            {
                rotatePoint(&listOfPoints[i],-angle2,pt2);
                yDistanceList << abs(activePt.y()-listOfPoints[i].y());
                dAlphaList << abs(listOfAngles[i]-angle2-180);
                if (yDistanceList[i]<minDistY && dAlphaList[i]<=dAlpha12)
                {
                    minDistY=yDistanceList[i];
                    minDistIndex=i;
                }
            }

            //if distYBestMatch > minDistY && dAlpha is at least the same
            switch (minDistIndex)
            {
            //aLbL,aRbL,aLbR,aRbR,bLaL,bRaL,bLaR,bRaR
            case -1:
                if (r<0)
                    this->makeItem(bestMatch,&activePt,true);
                else
                    this->makeItem(bestMatch,&activePt,false);
                break;
            case 0:
                this->makeItem(alternativeItem,&activePt,true);
                this->makeItem(bestMatch,this->activeEndPoint,true);
                break;
            case 1:
                this->makeItem(alternativeItem,&activePt,false);
                this->makeItem(bestMatch,this->activeEndPoint,true);
                break;
            case 2:
                this->makeItem(alternativeItem,&activePt,true);
                this->makeItem(bestMatch,this->activeEndPoint,false);
                break;
            case 3:
                this->makeItem(alternativeItem,&activePt,false);
                this->makeItem(bestMatch,this->activeEndPoint,false);
                break;
            case 4:
                this->makeItem(bestMatch,&activePt,true);
                this->makeItem(alternativeItem,this->activeEndPoint,true);
                break;
            case 5:
                this->makeItem(bestMatch,&activePt,false);
                this->makeItem(alternativeItem,this->activeEndPoint,true);
                break;
            case 6:
                this->makeItem(bestMatch,&activePt,true);
                this->makeItem(alternativeItem,this->activeEndPoint,false);
                break;
            case 7:
                this->makeItem(bestMatch,&activePt,false);
                this->makeItem(alternativeItem,this->activeEndPoint,false);
                break;
            }
        }
        else
        {
            if (r<0)
                this->makeItem(bestMatch,&activePt,true);
            else
                this->makeItem(bestMatch,&activePt,false);
        }

        /*
        //STEP 6:
        measure the distance between new active endPoint and Bezier curve
        if the slot track is being build, the distance is not computed from new active endPoint,
            but from the "central point" of all lanes -> "average" of endPoints
            Bezier: u=0.5 (or 0.25, see step 3) will be used as a start point and try to minimize the distance
                by incrementing (and then also decrementing) the value of u
            Distance should be less than some tolerance constant (not the TOLERANCE_HALF)
                if the distance is larger set flag for new Bezier

        activePt=active endPoint if the item is not a turnout
                 the point with the shortest distance to pt2 if the item is a turnout
        */

        ModelItem * lastInsertedItem = this->lastInserted;
        QPointF distanceCheckPoint = *this->activeEndPoint;
        QPointF ptAtCurve = curve.getPointAtU(u);
        qreal distanceToCurveOrPt2 = dist(&distanceCheckPoint,&ptAtCurve);
        if (lastInsertedItem->getType()==J1 || lastInsertedItem->getType()==J2 || lastInsertedItem->getType()==J3)
            distanceToCurveOrPt2 = dist(&distanceCheckPoint,pt2);
        if (bestMatch->getSlotTrackInfo()==NULL)
        {
            int i = 0;
            while (lastInsertedItem->getEndPoint(i)!=NULL)
            {
                qreal temp = 0;
                if (lastInsertedItem->getType()==J1 || lastInsertedItem->getType()==J2 || lastInsertedItem->getType()==J3)
                    temp = dist(lastInsertedItem->getEndPoint(i),pt2);
                else
                    temp = dist(lastInsertedItem->getEndPoint(i),&ptAtCurve);
                if (temp < distanceToCurveOrPt2 && lastInsertedItem->getNeighbour(i)==NULL)
                {
                    distanceToCurveOrPt2=temp;
                    distanceCheckPoint=*lastInsertedItem->getEndPoint(i);
                }
                i++;
            }
        }
        activePt=distanceCheckPoint;

        if (infiniteCounter%5==0 || (r<0 && distL>distance12/20) || (r>0 && distR>distance12/20) )
            makeNewBezier=true;
        else
            makeNewBezier=false;

        infiniteCounter++;


        if (infiniteCounter>255)
            break;

        if (!this->graphicsScene->sceneRect().contains(*this->activeEndPoint))
        {
            if (sceneExceededForTheFirstTime)
            {
                sceneExceededForTheFirstTime=false;
                QMessageBox * mb = new QMessageBox(this->window());
                mb->setIcon(QMessageBox::Information);

                if (app->getUserPreferences()->getLocale()->startsWith("EN"))
                {
                    mb->setText("The model has exceeded the workspace bounds. May the model building continue?");
                    mb->addButton("Continue",QMessageBox::AcceptRole);
                    mb->addButton("End computation",QMessageBox::RejectRole);
                }
                else
                {
                    mb->setText("Model překročil hranice pracovní plochy. Jak má program reagovat?");
                    mb->addButton("Pokračovat",QMessageBox::AcceptRole);
                    mb->addButton("Ukončit stavbu",QMessageBox::RejectRole);
                }

                int r = mb->exec();
                if (r==0)
                    mayExceedSceneSize=true;
                else
                    mayExceedSceneSize=false;
            }
            if (!mayExceedSceneSize)
                break;
        }


    }//endloop

    //the model exceeded scene bounds
    if (mayExceedSceneSize && !sceneExceededForTheFirstTime)
    {
        QRectF rScene = this->graphicsScene->sceneRect();
        QRectF rItems = this->graphicsScene->itemsBoundingRect();
        QRectF rNew = rScene.united(rItems);
        this->graphicsScene->setSceneRect(rNew);
        this->setSceneRect(rNew);
        this->centerOn(this->mapToScene(this->viewport()->rect()).boundingRect().center());
    }
    this->selectTwoPointsComplete=false;
    return 0;
}

int WorkspaceWidget::updateFragment(ModelFragment *frag)
{
    for (int i = 0; i < frag->getFragmentItems()->count();i++)
    {
        if (frag->getFragmentItems()->at(i)->get2DModelNoText()->scene()!=this->graphicsScene)
            this->graphicsScene->addItem(frag->getFragmentItems()->at(i)->get2DModelNoText());
    }
    return 0;
}

ModelFragment *WorkspaceWidget::findFragmentByApproxPos(QPointF *point)
{
    ModelFragment * pointer = NULL;
    QRectF rect(point->x()-TOLERANCE_HALF,point->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);
    QList<ModelFragment*>::Iterator it = this->modelFragments->begin();
    while(it!=this->modelFragments->end())
    {
        QList<QPointF*>* ptList = (*it)->getEndPoints();
        QList<QPointF*>::Iterator itPt = ptList->begin();
        while (itPt!=ptList->end())
        {
            if (rect.contains(**itPt))
            {
                *point=**itPt;
                pointer = *it;
                break;
            }
            itPt++;

        }
        it++;
    }

    return pointer;

}

ModelItem *WorkspaceWidget::findItemByApproxPos(QPointF *point, ModelItem * dontWantThisItem)
{
    if (point==NULL)
        return NULL;
    ModelItem * pointer = NULL;
    QRectF rect(point->x()-TOLERANCE_HALF,point->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);
    QList<ModelFragment*>::Iterator fragIter = this->modelFragments->begin();
    while (fragIter!=this->modelFragments->end() && pointer==NULL)
    {
        QList<ModelItem*>::Iterator itemIter = (*fragIter)->getFragmentItems()->begin();
        while (itemIter!=(*fragIter)->getFragmentItems()->end()  && pointer==NULL)
        {
            int index = 0;
            while ((*itemIter)->getEndPoint(index)!=NULL && pointer==NULL)
            {
                if (rect.contains(*(*itemIter)->getEndPoint(index)) && *itemIter!=dontWantThisItem)
                {
                    *point=*(*itemIter)->getEndPoint(index);
                    pointer = (*itemIter);
                }
                index++;
            }

            itemIter++;
        }
        fragIter++;
    }
    return pointer;

}

ModelFragment *WorkspaceWidget::findFragmentByID(int id)
{
    QList<ModelFragment*>::Iterator it = this->modelFragments->begin();
    while (it!=this->modelFragments->end())
    {
        if ((*it)->getID()==id)
            return (*it);
        it++;
    }
    return NULL;
}

int WorkspaceWidget::getFragmentIndex(ModelFragment *frag)
{
    return this->modelFragments->indexOf(frag);
}

ModelFragment *WorkspaceWidget::getActiveFragment() const
{
    return this->activeFragment;
}

void WorkspaceWidget::setActiveFragment(ModelFragment *frag)
{
    QString str;
    QString negStr;
    if (this->activeFragment!=NULL)
        negStr = (QString("select fragment %1 ").arg(QString::number(this->activeFragment->getID())));
        //negStr = (QString("select fragment %1 %2 ").arg(QString::number(this->activeFragment->getFragmentItems()->at(0)->getEndPoint(0)->x()),QString::number(this->activeFragment->getFragmentItems()->at(0)->getEndPoint(0)->y())));
    else
        negStr = (QString("select fragment NULL"));

    if (frag!=NULL)
        str = (QString("select fragment %1 ").arg(QString::number(frag->getID())));
    else
        str = (QString("select fragment NULL"));

    this->pushBackCommand(str,negStr);
    if (this->activeFragment!=NULL)
        this->activeFragmentPrev = this->activeFragment;
    this->activeFragment=frag;
}

ModelItem *WorkspaceWidget::getActiveItem() const
{
    return this->activeItem;
}

void WorkspaceWidget::setActiveItem(ModelItem *item)
{
    this->activeItem=item;
    if (item!=NULL)
        item->updateEndPointsHeightGraphics(true);
}

QPointF *WorkspaceWidget::getActiveEndPoint() const
{
    return this->activeEndPoint;
}

int WorkspaceWidget::setActiveEndPoint(QPointF *pt)
{
    if (this->doNotPrintSetEndPoint==false)
    {
        QString str = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
        QString negStr;
        if (this->activeEndPoint!=NULL)
            negStr = (QString("make point %1 %2 ").arg(QString::number(this->activeEndPoint->x()),QString::number(this->activeEndPoint->y())));
        else
            negStr = (QString("make point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

        QString str2 = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));
        QString negStr2;
        if (this->activeEndPoint!=NULL)
            negStr2 = (QString("select point %1 %2 ").arg(QString::number(this->activeEndPoint->x()),QString::number(this->activeEndPoint->y())));
        else
            negStr = (QString("select point %1 %2 ").arg(QString::number(pt->x()),QString::number(pt->y())));

        this->pushBackCommand(str,negStr2);
        this->pushBackCommand(str2,negStr);
    }
    if (pt==NULL)
        return 1;

    if (this->activeEndPoint==NULL)
    {
        this->activeEndPoint = new QPointF(0,0);
        this->activeEndPointPrev = new QPointF(0,0);
    }

    *this->activeEndPointPrev=*this->activeEndPoint;

    *this->activeEndPoint=*pt;

    if (this->activeEndPointGraphic!=NULL)
    {
        this->graphicsScene->removeItem(this->activeEndPointGraphic);
        this->activeEndPointGraphic=NULL;
    }
    QPainterPath pp = QPainterPath();
    pp.addEllipse(-7.5,-7.5,15,15);

    QGraphicsPathItem * qgpi = new QGraphicsPathItem(pp);
    QPen p = qgpi->pen();
    p.setWidth(0);
    QBrush b = qgpi->brush();
    b.setStyle(Qt::SolidPattern);

    if (this->activeFragment==NULL)
        b.setColor(QColor(255,127,0));
    else
        b.setColor(QColor(0,218,0));


    qgpi->setPen(p);
    qgpi->setBrush(b);
    qgpi->moveBy(pt->x(),pt->y());
    qgpi->setZValue(1000);

    this->graphicsScene->addItem(qgpi);
    this->activeEndPointGraphic=qgpi;

    return 0;
}

ModelItem *WorkspaceWidget::getLastUsedPart() const
{
    return this->lastUsedPart;
}

int WorkspaceWidget::setLastUsedPart(ModelItem *part)
{
    if (part==NULL)
        return 1;
    this->lastUsedPart=part;
    return 0;
}

int WorkspaceWidget::setLastInserted(ModelItem *item)
{
    if (item==NULL)
        return 1;
    this->lastInserted=item;
    return 0;
}

void WorkspaceWidget::setLastEventPos(QPointF point)
{
    delete this->lastEventPos;
    this->lastEventPos = new QPointF(point);
}

GraphicsScene *WorkspaceWidget::getGraphicsScene() const
{return this->graphicsScene;}
bool WorkspaceWidget::getRotationMode()
{return this->rotationMode;}
bool WorkspaceWidget::getHeightProfileMode()
{return this->heightProfileMode;}
bool WorkspaceWidget::getDeletePress() const
{return this->deletePress;}

bool WorkspaceWidget::canInsert(ModelItem *item) const
{
    bool enabled = false;
    ProductLine * pLFirst = NULL;


    if (app->getAllowMixedProductLines())
    {
        if (this->activeFragment!=NULL)
        {
            pLFirst = this->activeFragment->getProductLines()->first();
            if (pLFirst->getScaleEnum()==item->getProdLine()->getScaleEnum())
                enabled =  true;
            else
            {
                app->getAppData()->setMessageDialogText("You cannot mix scales in one fragment of model.","Nelze míchat měřítka v jedné souvislé části modelu");
                app->getAppData()->getMessageDialog()->exec();
            }
        }
        else
        {
            enabled = true;
        }

    }
    else
    {
        if (this->activeFragment!=NULL)
        {
            pLFirst = this->activeFragment->getProductLines()->first();
            if (this->activeFragment->getProductLines()->count()>1
                && pLFirst->getScaleEnum()==item->getProdLine()->getScaleEnum()/4.0)
                enabled = true;
            else
            {
                //compares pointers, which is ok
                if (pLFirst==item->getProdLine())
                    enabled = true;
                else
                {
                    app->getAppData()->setMessageDialogText("It is not allowed to mix parts from various manufacturers.\nYou can enable it in menu \"Model\".","Nelze spojovat dílky různých výrobců. Tuto volbu můžete povolit v menu \"Model\".");
                    app->getAppData()->getMessageDialog()->exec();
                }
            }
        }
        else
            enabled = true;

    }
    return enabled;
}

bool WorkspaceWidget::canInsert(BorderItem *item) const
{

    bool enabled = false;
    if (this->selection->count()!=1 || this->activeFragment==NULL || this->selection->first()->getSlotTrackInfo()==NULL)
        return enabled;

    ModelItem * fstSelected = this->selection->first();

    qreal useless;

    bool outerTrackPoint = true;
    QList<QPointF*>* trackPoints = fstSelected->getSlotTrackInfo()->getBorderEndPoints();
    for (int i = 0; i < trackPoints->count();i++)
    {
        if (pointsAreCloseEnough(trackPoints->at(i),this->activeEndPoint,TOLERANCE_HALF))
        {
            if (i >= trackPoints->count()/2)
                outerTrackPoint=false;
        }
    }

    if (item->getAngle()!=0)
    {
        if (item->getInnerBorderFlag())
        {
            if (abs(fstSelected->getSecondRadius())==item->getRadius() && !outerTrackPoint)
                enabled = true;
        }
        else
        {
            if (abs(fstSelected->getRadius())==item->getRadius()  && outerTrackPoint)
                enabled = true;
        }
    }
    else
    {
            if (modf(abs(fstSelected->getSecondRadius()/item->getRadius()),&useless)<=0.01)
                enabled = true;
    }

    if (fstSelected->getRadius()>0)
    {
        if (fstSelected->leftRightDifference180(0,1))
        {
            //cout << fstSelected->getTurnAngle(1)-fstSelected->getTurnAngle(0) << endl;
            if (abs(modf((fstSelected->getTurnAngle(1)-fstSelected->getTurnAngle(0)+180)/item->getAngle(),&useless))>=0.01)
                enabled=false;
        }
        else
        {
            if (abs(modf((fstSelected->getTurnAngle(1)-fstSelected->getTurnAngle(0))/item->getAngle(),&useless))>=0.01)
                enabled=false;
        }
    }
    else
    {
        if (fstSelected->leftRightDifference180(0,1))
        {
            if (abs(modf((fstSelected->getTurnAngle(1)-fstSelected->getTurnAngle(0)+180)/item->getAngle(),&useless))>=0.01)
                enabled=false;
        }
        else
        {

            if (abs(modf((fstSelected->getTurnAngle(1)-fstSelected->getTurnAngle(0))/item->getAngle(),&useless))>=0.01)
                enabled=false;
        }
    }

    //now search for the point of connection in the selected item
    int connectionIndex = 0;
    //while (fstSelected->getSlotTrackInfo()->getEndPoint(connectionIndex)!=NULL)
    for (; connectionIndex < fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count(); connectionIndex++)
    {
        if (pointsAreCloseEnough(fstSelected->getSlotTrackInfo()->getBorderEndPoints()->at(connectionIndex),this->activeEndPoint,fstSelected->getProdLine()->getScaleEnum()/4.0))
            break;
    }
    //from the previous step you know the index of connection
    //if index-(count of "border"'s endpoints)<0
        //cant insert
    int pointsCount = 0;
    if (item->getInnerBorderFlag() || item->getAngle()<=2)
        pointsCount = fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count();
    else
        pointsCount = fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count()/2;

    if (item->getAngle()>2 && pointsCount<=connectionIndex+item->getEndPointsCount()-1)
        enabled = false;
    else if (item->getAngle()<=2)
    {
        if (fstSelected->getRadius()>0)
        {
            if (connectionIndex>=fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count()/2)
            {
                if (connectionIndex+item->getEndPointsCount()-1>=fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count())
                    enabled = false;
            }
            else
            {
                if (connectionIndex-(item->getEndPointsCount()-1)<0)
                    enabled = false;
            }

        }
        else
        {
            if (connectionIndex>=fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count()/2)
            {
                if (connectionIndex-(item->getEndPointsCount()-1)<fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count()/2)
                    enabled = false;
            }
            else
            {
                if (connectionIndex+item->getEndPointsCount()-1>=fstSelected->getSlotTrackInfo()->getBorderEndPoints()->count()/2)
                    enabled = false;
            }
        }
    }

    //if neighbour at index-(count of "border"'s endpoints) is not null
        //cant insert
    //else if (this->selection->first()->getSlotTrackInfo()->getBorders()->at(connectionIndex+item->getEndPointsCount())!=NULL)
   if (fstSelected->getSlotTrackInfo()->getBorders()->at(connectionIndex+item->getEndPointsCount()-1)!=NULL)
    // else if (fstSelected->getSlotTrackInfo()->getBorders()->at(connectionIndex+item->getEndPointsCount()-1)!=NULL)
        enabled = false;

    return enabled;
}

int WorkspaceWidget::exportCurrentState(QTextStream &file)
{
    int currentIdInFile = 0;

    QPointF ptCenter = this->mapToScene(this->viewport()->rect()).boundingRect().center();
    qreal x = ptCenter.x();
    qreal y = ptCenter.y();

    if (app->getUserPreferences()->getSaveScenePosFlag())
        file << x << " " << y << endl;
    else
        file << "0 0" << endl;
    file << this->scene()->sceneRect().x() << " " << this->scene()->sceneRect().y() << " " << this->scene()->sceneRect().width() << " " << this->scene()->sceneRect().height() << endl;

    if (app->getUserPreferences()->getSaveScenePosFlag())
        file << this->transform().m11() << endl;
    else
        file << "1" << endl;


    for (int i = 0; i < this->modelFragments->count(); i++)
    {
        ModelFragment * f = this->modelFragments->at(i);
        ModelItem * it0 = f->getFragmentItems()->at(0);

        //print first item of each fragment

        file << "make item " << *it0->getPartNo()<< " " << *it0->getProdLine()->getName() << (it0->getRadius()<0 ? " L " : " R ") << it0->get2DModelNoText()->scenePos().x() << " " << it0->get2DModelNoText()->scenePos().y() << endl;
        ItemType t = it0->getType();
        if (t==E1)
            file << "rotate fragment " << currentIdInFile << " " << it0->getEndPoint(0)->x() << " " << it0->getEndPoint(0)->y()  << " " << -180+(it0->getTurnAngle(0)) << endl;
        else if (t==J1 && it0->getSlotTrackInfo()==NULL)
        {
            file << "rotate fragment " << currentIdInFile << " " << it0->get2DModelNoText()->scenePos().x() << " " << it0->get2DModelNoText()->scenePos().y()  << " " << -180+(it0->getTurnAngle(0))+(180+(it0->getTurnAngle(2)-it0->getTurnAngle(0)))/2  << endl;
        }
        else if (t==J2 && it0->getSlotTrackInfo()==NULL)
        {
            file << "rotate fragment " << currentIdInFile << " " << it0->get2DModelNoText()->scenePos().x() << " " << it0->get2DModelNoText()->scenePos().y()  << " " << -180+(it0->getTurnAngle(0))+(180+(it0->getTurnAngle(2)-it0->getTurnAngle(0)))/2  << endl;
        }
        else
            file << "rotate fragment " << currentIdInFile << " " << it0->get2DModelNoText()->scenePos().x() << " " << it0->get2DModelNoText()->scenePos().y()  << " " << -180+(it0->getTurnAngle(0))+(180+(it0->getTurnAngle(1)-it0->getTurnAngle(0)))/2  << endl;


        //print border items of the first item of each fragment

        if (it0->getSlotTrackInfo()!=NULL)
        {
            //this code will print n-times all the borders with n-points
            //but it is not the problem, because executeCommand will make it just once
            for (int index0 = 0; index0 < it0->getSlotTrackInfo()->getBorderEndPoints()->count(); index0++)
            {
                if (it0->getSlotTrackInfo()->getBorders()->at(index0)!=NULL)
                    file << "make border " << *it0->getSlotTrackInfo()->getBorders()->at(index0)->getPartNo()<< " " << *it0->getSlotTrackInfo()->getBorders()->at(index0)->getProdLine()->getName() << (it0->getSlotTrackInfo()->getBorders()->at(index0)->getInnerBorderFlag() ? " I " : " O ") << it0->getSlotTrackInfo()->getBorders()->at(index0)->getEndPoint(0)->x() << " " << it0->getSlotTrackInfo()->getBorders()->at(index0)->getEndPoint(0)->y() << endl;
            }
        }

        for (int j = 1; j < f->getFragmentItems()->count(); j++)
        {
            ModelItem * it = f->getFragmentItems()->at(j);
            if (it->getNeighbour(0)==NULL)
                file << "null neighbour " << -180+it->getTurnAngle(0) << " " << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y() << endl;
            //file << "make item " << it->getPartNo()->toStdString() << " " << it->getProdLine()->getName()->toStdString() << (it->getRadius()<0 ? " L " : " R ") << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y() << endl;

            file << "make item " << *it->getPartNo() << " " << *it->getProdLine()->getName() << (it->getRadius()<0 ? " L " : " R ") << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y() << endl;

            qreal dAlpha = 0;
            {
                ModelItem * neighbour = it->getNeighbour(0);
                int k = 0;
                while (neighbour->getEndPoint(k)!=NULL)
                {
                    if (neighbour->getNeighbour(k)==it)
                        break;
                    k++;
                }
                //dAlpha = it->getTurnAngle(0)-neighbour->getTurnAngle(k);
                dAlpha = neighbour->getTurnAngle(k)-it->getTurnAngle(0);
                while (dAlpha<0)
                    dAlpha+=360;
                while (dAlpha>360)
                    dAlpha-=360;
                if (dAlpha>15)
                    dAlpha=0;
            }


            //file << "rotate item " << it->get2DModelNoText()->scenePos().x() << " " << it->get2DModelNoText()->scenePos().y() << " " << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y() << " " << -dAlpha << endl;
            QPointF scenePosBeforeRotation(it->get2DModelNoText()->scenePos());
            rotatePoint(&scenePosBeforeRotation,dAlpha,it->getEndPoint(0));
            //file << "rotate item " << it->get2DModelNoText()->scenePos().x() << " " << it->get2DModelNoText()->scenePos().y() << " " << it->getNeighbour(0)->getEndPoint(it)->x() << " " << it->getNeighbour(0)->getEndPoint(it)->y() << " " << -dAlpha << endl;

            if (it->getNeighbour(0)==NULL)
                file << "null neighbour2 " << it->getTurnAngle(0) << " " << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y()  << endl;

            //file << "rotate item " << scenePosBeforeRotation.x() << " " << scenePosBeforeRotation.y() << " " << it->getNeighbour(0)->getEndPoint(it)->x() << " " << it->getNeighbour(0)->getEndPoint(it)->y() << " " << -dAlpha << endl;
            file << "rotate item " << scenePosBeforeRotation.x() << " " << scenePosBeforeRotation.y() << " " << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y() << " " << -dAlpha << endl;


            if (it->getSlotTrackInfo()!=NULL)
            {
                QList<BorderItem*> visited;
                for (int index = 0; index < it->getSlotTrackInfo()->getBorderEndPoints()->count(); index++)
                {

                    if (it->getSlotTrackInfo()->getBorders()->at(index)!=NULL && !visited.contains(it->getSlotTrackInfo()->getBorders()->at(index)))
                    {
                        file << "make border " << *it->getSlotTrackInfo()->getBorders()->at(index)->getPartNo()<< " " << *it->getSlotTrackInfo()->getBorders()->at(index)->getProdLine()->getName() << (it->getSlotTrackInfo()->getBorders()->at(index)->getInnerBorderFlag() ? " I " : " O ") << it->getSlotTrackInfo()->getBorders()->at(index)->getEndPoint(0)->x() << " " << it->getSlotTrackInfo()->getBorders()->at(index)->getEndPoint(0)->y() << endl;
                        visited << it->getSlotTrackInfo()->getBorders()->at(index);
                    }
                }
            }
        }
        for (int j = 1; j < f->getFragmentItems()->count(); j++)
        {
            ModelItem * it = f->getFragmentItems()->at(j);
            {
                int k = 0;
                while(it->getEndPoint(k)!=NULL)
                {
                    //height point dz x.x y.y scenePos.x scenePos.y

                    file << "height point " << it->getHeightProfileAt(it->getEndPoint(k)) << " " << it->getEndPoint(k)->x() << " " << it->getEndPoint(k)->y() << " " << it->get2DModelNoText()->scenePos().x() << " " << it->get2DModelNoText()->scenePos().y() << endl;
                    k++;
                }
            }
        }

        currentIdInFile++;
    }

    //how about additional models? Trees, houses etc.
    for (int i = 0; i < this->vegetationItems->count(); i++)
    {
        VegetationItem * vi = this->vegetationItems->at(i);
        file << "make vegetation " << *vi->getPartNo() << " " << *vi->getProdLine()->getName() << " " << vi->get2DModelNoText()->scenePos().x() << " " << vi->get2DModelNoText()->scenePos().y() << endl;
        file << "rotate vegetation " << vi->get2DModelNoText()->scenePos().x() << " " << vi->get2DModelNoText()->scenePos().y() << " " << vi->getRotation() << endl;
    }

    this->unsavedChanges=false;
    return 0;
}

int WorkspaceWidget::setCurrentState(QTextStream &file)
{
    this->resetWorkspace();

    QString str;

    str = file.readLine();

    //centerPoint of view
    qreal xC = 0;
    qreal yC = 0;
    QString s;
    s = str;
    xC = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    yC = s.toDouble();

    qreal x = 0;
    qreal y = 0;
    qreal w = 0;
    qreal h = 0;
    //scene size
    str = file.readLine();
    s = str;
    x = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    y = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    w = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    h = s.toDouble();

    this->graphicsScene->setSceneRect(x,y,w,h);


    x = 0;
    str = file.readLine();
    s = str;
    x = s.toDouble();

    this->scale(x,x);

    this->centerOn(xC,yC);


    int retValue = 0;
    while(!file.atEnd())
    {
        str = file.readLine();
        if (str.startsWith("INVENTORY"))
            break;
        //if (this->commandExecution((str))!=0)
        int val = this->commandExecution(str);
        if (val<0)
            return -1;
        else if (val>0)
            retValue = val;

    }
    this->actionListRedo.clear();
    this->actionListUndo.clear();
    this->indexUndoRedo=-1;
    return retValue;
}

void WorkspaceWidget::resetWorkspace()
{
    if (!this->selection->empty())
    {
        for (int i = this->selection->count()-1; i >=0; i--)
        {
            this->deselectItem(this->selection->at(i));
        }
    }

    this->actionListRedo.clear();
    this->actionListUndo.clear();
    this->indexUndoRedo=-1;
    QPointF pt;

    this->setActiveFragment(NULL);
    this->setActiveEndPoint(&pt);
    this->setActiveItem(NULL);

    this->lastUsedPart=NULL;
    this->lastInserted=NULL;
    this->copiedItems->clear();

    if (!this->modelFragments->empty())
    {
        for (int i = this->modelFragments->count()-1; i >= 0; i--)
        {
            delete this->modelFragments->at(i);
        }
        this->modelFragments->clear();
    }

    logFile << "model fragments deleted succesfully" << endl;

    if (!this->vegetationItems->empty())
    {
        for (int i = this->vegetationItems->count()-1; i >= 0; i--)
        {
            delete this->vegetationItems->at(i);
        }
        this->vegetationItems->clear();
    }
    //border items may still remain in the scene -> delete them too
    int i = 0;
    while (!this->graphicsScene->items().empty())
    {
        this->graphicsScene->removeItem(this->graphicsScene->items().at(i));

    }

    this->selectTwoPointsBend=false;
    this->selectTwoPointsComplete=false;
    this->rotationMode=false;
    this->heightProfileMode=false;
    this->unsavedChanges=false;
    this->undoRedoCalled=false;
    this->eraseFollowing=true;
    this->nextIDToUse=0;
}

bool WorkspaceWidget::getSelectTwoPointsBend() const
{return this->selectTwoPointsBend;}

bool WorkspaceWidget::getSelectTwoPointsComplete() const
{return this->selectTwoPointsComplete;}

bool WorkspaceWidget::unsavedChangesFlag() const
{return this->unsavedChanges;}

void WorkspaceWidget::toggleRotationMode()
{
    if (this->rotationMode==true)
        this->rotationMode=false;
    else
        this->rotationMode=true;
}

void WorkspaceWidget::toggleHeightProfileMode()
{
    if (this->heightProfileMode==true)
    {
        this->heightProfileMode=false;
        this->setActiveEndPoint(new QPointF(0,0));
        this->setActiveItem(NULL);
    }
    else
    {
        this->heightProfileMode=true;
        this->setActiveEndPoint(new QPointF(0,0));
        this->setActiveFragment(NULL);

    }
    QList<ModelFragment*>::Iterator fragIter = this->modelFragments->begin();
    while(fragIter!=this->modelFragments->end())
    {
        QList<ModelItem*>::Iterator itemIter = (*fragIter)->getFragmentItems()->begin();
        while(itemIter!=(*fragIter)->getFragmentItems()->end())
        {
            (*itemIter)->updateEndPointsHeightGraphics(true);
            itemIter++;
        }
        fragIter++;
    }


}

void WorkspaceWidget::adjustHeightOfActive()
{
    //is called by menubar actions press - for all slot track items and !(C1 || S1) rail items increases the height of all endpoints

    if(this->activeItem!=NULL && this->activeEndPoint!=NULL && this->heightProfileMode)
    {
        QList<QAction *> list =(app->getWindow()->getMainToolBar()->actions());
        for (int i = 0; i < list.count();i++)
        {
            if (list.at(i)==this->sender())
            {
                if (list.at(i)->toolTip()=="Decrease height" || list.at(i)->statusTip().startsWith("Snížit"))
                {
                        this->activeItem->adjustHeightProfile(-1,this->activeEndPoint);

                }
                else
                {
                        this->activeItem->adjustHeightProfile(1,this->activeEndPoint);
                }
            }
        }

    }
}

void WorkspaceWidget::undo()
{
//    when it should continue:
//        -select fragment & make point & select point - no combos
//        -make point & select point
//        -make point & select point & rotate fragment - + combos
//        -rotate & rotate

    bool nextStep = false;

    if (this->indexUndoRedo<=0)
        return;

    if (this->heightProfileMode)
    {
        if (!(this->actionListUndo[this->indexUndoRedo].startsWith("height")
                || this->actionListUndo[this->indexUndoRedo].startsWith("make point")
                || this->actionListUndo[this->indexUndoRedo].startsWith("select")
                || this->actionListUndo[this->indexUndoRedo].startsWith("move fragment")
                || this->actionListUndo[this->indexUndoRedo].startsWith("rotate fragment")
                || this->actionListUndo[this->indexUndoRedo].startsWith("deselect")
              ))
            return;
    }

    if (((this->indexUndoRedo < this->actionListRedo.count()-2) && (this->indexUndoRedo > 2)
         && ((this->actionListUndo.at(this->indexUndoRedo).startsWith("make point")
                && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("select point")
                && this->actionListUndo.at(this->indexUndoRedo-2).startsWith("move fragment"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("select point")
                && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("move fragment")
                && this->actionListUndo.at(this->indexUndoRedo-2).startsWith("make point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("move fragment")
                && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("make point")
                && this->actionListUndo.at(this->indexUndoRedo-2).startsWith("select point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("move fragment")
                && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("move fragment") )
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("make point")
                && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("select point")
                && this->actionListUndo.at(this->indexUndoRedo-2).startsWith("rotate fragment"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("select point")
                && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("rotate fragment")
                && this->actionListUndo.at(this->indexUndoRedo-2).startsWith("make point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("rotate fragment")
                && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("make point")
                && this->actionListUndo.at(this->indexUndoRedo-2).startsWith("select point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("rotate fragment")
               && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("rotate fragment") )
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("make point")
               && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("select point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo+1).startsWith("make point")
               && this->actionListUndo.at(this->indexUndoRedo).startsWith("select point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("select fragment")
               && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("make point")
               && this->actionListUndo.at(this->indexUndoRedo-2).startsWith("select point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo+1).startsWith("select fragment")
               && this->actionListUndo.at(this->indexUndoRedo).startsWith("make point")
               && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("select point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo+2).startsWith("select fragment")
               && this->actionListUndo.at(this->indexUndoRedo+1).startsWith("make point")
               && this->actionListUndo.at(this->indexUndoRedo).startsWith("select point"))
             ||
             (this->actionListUndo.at(this->indexUndoRedo).startsWith("move vegetation")
                 && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("move vegetation"))
             ||
             (this->actionListUndo.at(this->indexUndoRedo).startsWith("rotate vegetation")
                 && this->actionListUndo.at(this->indexUndoRedo-1).startsWith("rotate vegetation") )

            ))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("deselect item") && !this->heightProfileMode)
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("height point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("select item") && !this->heightProfileMode)
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("rotate item"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("select fragment")))

        nextStep = true;


    logFile << "UNDO ";

    this->eraseFollowing=false;
    if (this->indexUndoRedo>0)
    {
        this->undoRedoCalled = true;
        if (0!=this->commandExecution(this->actionListUndo.at(this->indexUndoRedo)))
            nextStep=true;

        //this is needed in case the new item is created
        //  -it means redo action is delete - deletion may cause creating of the new fragment
        //  -thus undo causes that the new item is created -> both fragments need to be connected back
        //   and have their id set to the original one
        if (this->actionListUndo.at(this->indexUndoRedo).startsWith("make item"))
        {
            int originalId = -98765;

            ModelItem * zeroItem = NULL;//this->lastInserted->getParentFragment()->getFragmentItems()->at(0);
            ModelFragment * zeroFragment = NULL;

            originalId = -5;//this->lastInserted->getParentFragment()->getID();
            int cBefore = this->modelFragments->count();

            {
                QString command = this->actionListUndo.at(this->indexUndoRedo);
                QString temp = command.mid(command.lastIndexOf(" "));
                int idZero = temp.toInt();
                command = command.remove(command.lastIndexOf(" "),temp.length());
                temp = command.mid(command.lastIndexOf(" "));
                qreal y = temp.toDouble();
                command = command.remove(command.lastIndexOf(" "),temp.length());
                temp = command.mid(command.lastIndexOf(" "));
                qreal x = temp.toDouble();

                QPointF zeroItemScenePos(x,y);


                GraphicsPathItemModelItem * gpi = NULL;
                QList<QGraphicsItem*> itemsList = this->scene()->items(QRectF(zeroItemScenePos.x()-TOLERANCE_HALF,zeroItemScenePos.y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF));
                for (int i = 0; i < itemsList.count();i++)
                {
                    if (itemsList[i]->type()==QGraphicsItem::UserType+2)
                    {
                        gpi = (GraphicsPathItemModelItem*)itemsList[i];
                        QPointF pt = gpi->scenePos();
                        if (!pointsAreCloseEnough(&pt,&zeroItemScenePos,TOLERANCE_HALF))
                            continue;
                        else
                        {
                            zeroItem=gpi->getParentItem();
                            zeroFragment=this->findFragmentByID(idZero);
                            originalId=idZero;
                            break;
                        }
                    }
                }
            }

            if (zeroFragment==NULL || zeroItem==NULL)
                return;


            int i = 0;
            while (zeroItem->getEndPoint(i)!=NULL)
            {
                if (!zeroItem->leftRightDifference180(i,i+1))
                {
                    zeroItem->setEndPointAngle(zeroItem->getEndPoint(i),zeroItem->getTurnAngle(i)-180);
                    zeroFragment->setEndPointAngle(zeroItem->getEndPoint(i),zeroItem->getTurnAngle(i));
                }
                i+=2;
            }
            this->connectFragments(zeroFragment,zeroItem);
            if (this->modelFragments->count()!=cBefore)
            {
                this->modelFragments->last()->setID(originalId);

            }
        }

        this->indexUndoRedo--;
        this->undoRedoCalled = false;
    }
    this->eraseFollowing=true;

    if (nextStep && this->indexUndoRedo!=0)
        undo();

}

void WorkspaceWidget::redo()
{


//        - make point & select point & select fragment - no combos
//        -make point & select point
//        -make point & select point & rotate fragment - + combos


    bool nextStep = false;
    if ((this->indexUndoRedo < this->actionListRedo.count()-2) && (this->indexUndoRedo > 2)
        &&  ((this->actionListRedo.at(this->indexUndoRedo).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("select point")
                && this->actionListRedo.at(this->indexUndoRedo+2).startsWith("move fragment"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("select point")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("move fragment")
                && this->actionListRedo.at(this->indexUndoRedo+2).startsWith("make point"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("move fragment")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo+2).startsWith("select point"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("select point")
                && this->actionListRedo.at(this->indexUndoRedo+2).startsWith("rotate fragment"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("select point")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("rotate fragment")
                && this->actionListRedo.at(this->indexUndoRedo+2).startsWith("make point"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("rotate fragment")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo+2).startsWith("select point"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("rotate fragment")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("rotate fragment"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("select point"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("select point")
                && this->actionListRedo.at(this->indexUndoRedo+2).startsWith("select fragment"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo-1).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo).startsWith("select point")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("select fragment"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo-2).startsWith("make point")
                && this->actionListRedo.at(this->indexUndoRedo-1).startsWith("select point")
                && this->actionListRedo.at(this->indexUndoRedo).startsWith("select fragment"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("deselect item"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("height point"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("select item"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("select fragment"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("rotate vegetation")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("rotate vegetation"))
            ||
            (this->actionListRedo.at(this->indexUndoRedo).startsWith("move vegetation")
                && this->actionListRedo.at(this->indexUndoRedo+1).startsWith("move vegetation"))
             || this->actionListRedo.at(this->indexUndoRedo).startsWith("bend close")
            ))

        nextStep = true;


    logFile << "REDO ";
    this->eraseFollowing=false;



    while (this->actionListRedo.at(this->indexUndoRedo).startsWith("rotate item") && this->indexUndoRedo<this->actionListRedo.count())
        this->indexUndoRedo++;

    if (this->indexUndoRedo<(this->actionListRedo.count()-1))
    {

            this->undoRedoCalled = true;
            this->indexUndoRedo++;
            if (0!=this->commandExecution(this->actionListRedo.at(this->indexUndoRedo)))
                nextStep = true;
            this->undoRedoCalled = false;
    }

    this->eraseFollowing=true;

    if (nextStep)
        redo();
}

void WorkspaceWidget::copy()
{
    //copy the text commands which create items from selection
    this->copiedItems->clear();
    QString str;
    for (int i = 0; i < this->selection->count();i++)
    {
        ModelItem * item = (*this->selection)[i];
        if (item->getRadius()<0)
            str = QString("make item %1 %2 L %3 %4 ").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y()));
        else
            str = QString("make item %1 %2 R %3 %4 ").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y()));
        this->copiedItems->push_back(str);
    }

    for (int i = 0; i < this->selectionVegetation->count(); i++)
    {
        VegetationItem * item = (*this->selectionVegetation)[i];
        QString str(QString("make vegetation %1 %2 %3 %4 ").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y())));
        this->copiedItems->push_back(str);
    }

}

void WorkspaceWidget::paste()
{
    if (this->copiedItems->empty())
        return;


    if (this->activeFragment==NULL)
    {
        /*QPointF pt;
        this->setActiveEndPoint(&pt);*/
    }
    while (!this->selection->empty())
        this->deselectItem(this->selection->first());

    QPointF active = *this->getActiveEndPoint();

    qreal dx = 0;
    qreal dy = 0;
    for (int i = 0; i < this->copiedItems->count(); i++)
    {
        QString command = this->copiedItems->at(i);

        //dx and dy will be subtracted from all insertAt points

        //this was copied from commandExecution() - cE() cant be used, because incorrect points are stored in the commands
        bool makeItem = true;
        if (command.startsWith("make item"))
            command.remove(0,9);
        else
        {
            command.remove(0,15);
            makeItem=false;
        }
        command = command.trimmed();
        QString partNo = command.left(command.indexOf(" "));
        command.remove(0,partNo.length());
        command = command.trimmed();

        //prodLineName may contain whitespaces -> find lastIndexOf L and R characters -> if (char at (found index+2)==number) then you can use the index-1 as end character of prodLineName
        int lastL = command.lastIndexOf("L");
        int lastR = command.lastIndexOf("R");
        int endIndex = 0;
        if (lastL!=-1 && lastL+2<command.length() && (command.at(lastL+2).isDigit() || command.at(lastL+2)==QChar('-')))
            endIndex=lastL-1;
        if (lastR!=-1 && lastR+2<command.length() && (command.at(lastR+2).isDigit() || command.at(lastR+2)==QChar('-')))
            endIndex=lastR-1;

        QString prodLineName = command.left(endIndex);
        command.remove(0,prodLineName.length());
        command = command.trimmed();

        bool left = false;
        if (makeItem)
        {
            if (command.startsWith("L"))
                left = true;
        }

        qreal x = 0;
        qreal y = 0;

        if (makeItem)
        {
            command.remove(0,1);
            command = command.trimmed();
        }

        x = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y = command.toDouble();

        QPointF itemPoint(x,y);

        if (i==0 && makeItem)
        {
            dx = itemPoint.x();//-active.x();
            dy = itemPoint.y();//-active.y();
            itemPoint=active;
        }
        else if (i==0 && !makeItem)
        {
            dx = -itemPoint.x()+active.x();
            dy = -itemPoint.y()+active.y();

        }
        else if (makeItem)
        {
            itemPoint=*this->getActiveEndPoint();
            //itemPoint.setX(itemPoint.x()+dx);
            //itemPoint.setY(itemPoint.y()+dy);
        }
        ModelItem * item = app->getAppData()->getDatabase()->findModelItemByName(prodLineName,partNo);
        VegetationItem * itemV = app->getAppData()->getDatabase()->findVegetationItemByName(prodLineName,partNo);

        if (makeItem)
        {
            this->makeItem(item,&itemPoint,left);
            this->selectItem(item);
        }
        else
        {
            itemPoint.setX(dx+itemPoint.x());
            itemPoint.setY(dy+itemPoint.y());
            this->setActiveEndPoint(&itemPoint);
            this->makeVegetation(itemV);
            this->selectItem(itemV);
        }




    }
}

void WorkspaceWidget::cut()
{
    copy();
    //delete selected items
    int c = this->selection->count()-1;
    for (int i = c; i >= 0; i--)
        this->removeItem(this->selection->at(i));
}

void WorkspaceWidget::selectFragmentToClose()
{
    if (this->selectTwoPointsBend)
    {
        this->selectTwoPointsBend=false;

        return;
    }
    if (this->selectTwoPointsComplete)
    {
        this->selectTwoPointsComplete=false;
        QList<QAction*> actions = app->getWindow()->getMainToolBar()->actions();
        for (int i = 0; i < actions.count(); i++)
        {
            if (actions[i]->text().startsWith("Complete") || actions[i]->text().startsWith("Dokončit"))
                actions[i]->setChecked(false);
        }
    }

    if (app->getUserPreferences()->getLocale()->startsWith("EN"))
        app->getWindow()->statusBar()->showMessage("Select two points (one after another).\nIf it is possible the fragment will be closed.");
    else
        app->getWindow()->statusBar()->showMessage("Vyberte postupně dva body. Pokud je to možné, trat bude uzavřena.");

    if (app->getUserPreferences()->getDisplayHelpBendAndClose())
    {
        if (app->getUserPreferences()->getLocale()->startsWith("EN"))
            app->getAppData()->getErrorMessage()->showMessage("Select two points (one after another) in one fragment.\nIf it is possible the fragment will be closed.");
        else
            app->getAppData()->getErrorMessage()->showMessage("Vyberte postupně dva body. Pokud je to možné, trat bude uzavřena.");
    }

    this->activeFragment=NULL;
    *this->activeEndPoint=QPointF(0,0);
    this->selectTwoPointsBend=true;
}

void WorkspaceWidget::selectFragmentToComplete()
{
    if (this->selectTwoPointsComplete)
    {
        this->selectTwoPointsComplete=false;
        return;
    }
    if (this->selectTwoPointsBend)
    {
        this->selectTwoPointsBend=false;
        QList<QAction*> actions = app->getWindow()->getMainToolBar()->actions();
        for (int i = 0; i < actions.count(); i++)
        {
            if (actions[i]->text().startsWith("Bend") || actions[i]->text().startsWith("Ohnout"))
                actions[i]->setChecked(false);
        }
    }

    if (app->getUserPreferences()->getLocale()->startsWith("EN"))
        app->getWindow()->statusBar()->showMessage("Select two points (one after another).\n Track will be completed then.");
    else
        app->getWindow()->statusBar()->showMessage("Vyberte postupně dva body. Následně se trat dokončí.");

    if (app->getUserPreferences()->getDisplayHelpBendAndClose())
    {
        if (app->getUserPreferences()->getLocale()->startsWith("EN"))
            app->getAppData()->getErrorMessage()->showMessage("Select two points (one after another).\n Track will be completed then.");
        else
            app->getAppData()->getErrorMessage()->showMessage("Vyberte postupně dva body. Následně se trat dokončí.");
    }

    this->activeFragment=NULL;
    *this->activeEndPoint=QPointF(0,0);
    this->selectTwoPointsComplete=true;
}
