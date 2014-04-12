#include "graphicsScene.h"
#include "globalVariables.h"
#include "mathFunctions.h"

GraphicsScene::GraphicsScene(QMenu * contextMenuItem, QMenu * contextMenuScene,  QObject *parent) : QGraphicsScene(parent)
//GraphicsScene::GraphicsScene(QObject *parent) : QGraphicsScene(parent)
{
    this->mouseMode=false;
    this->contextMenuItem=contextMenuItem;
    this->contextMenuScene=contextMenuScene;

}

GraphicsScene::GraphicsScene(QMenu * contextMenuItem, QMenu * contextMenuScene, const QRectF &sceneRect, QObject *parent) : QGraphicsScene(sceneRect,parent)
//GraphicsScene::GraphicsScene(const QRectF &sceneRect, QObject *parent) : QGraphicsScene(sceneRect,parent)
{
    this->mouseMode=false;
    this->contextMenuItem=contextMenuItem;
    this->contextMenuScene=contextMenuScene;
}

GraphicsScene::GraphicsScene(QMenu * contextMenuItem, QMenu * contextMenuScene, qreal x, qreal y, qreal width, qreal height, QObject *parent) : QGraphicsScene(x,y,width,height,parent)
//GraphicsScene::GraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent) : QGraphicsScene(x,y,width,height,parent)
{
    this->mouseMode=false;
    this->contextMenuItem=contextMenuItem;
    this->contextMenuScene=contextMenuScene;
}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (this->mouseMode)
        QGraphicsScene::mousePressEvent(event);
    else
    {
        if (app->getWindow()->getWorkspaceWidget()->getHeightProfileMode())
        {
            QPointF * pos = new QPointF(event->scenePos());
            ModelItem* item = app->getWindow()->getWorkspaceWidget()->findItemByApproxPos(pos,NULL);
            if (item!=NULL)
            {
                app->getWindow()->getWorkspaceWidget()->setActiveItem(item);
                app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
                app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pos);
                app->getWindow()->getWorkspaceWidget()->selectItem(item);
                app->getWindow()->getWorkspaceWidget()->setActiveItem(item);
            }
            else
                delete pos;
        }
        else
        {
            if (!app->getWindow()->getWorkspaceWidget()->getSelectTwoPointsBend() && !app->getWindow()->getWorkspaceWidget()->getSelectTwoPointsComplete())
            {
                QPointF * pos = new QPointF(event->scenePos());
                ModelFragment* frag = app->getWindow()->getWorkspaceWidget()->findFragmentByApproxPos(pos);
                if (frag!=NULL)
                {
                    app->getWindow()->getWorkspaceWidget()->setActiveFragment(frag);
                    app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pos);
                    app->getWindow()->getWorkspaceWidget()->setActiveItem(NULL);
                    app->getWindow()->getWorkspaceWidget()->selectItem(frag->findEndPointItem(pos));
                }
                else if (this->itemAt(event->scenePos(),QTransform())!=NULL)
                {
                    QRectF r(event->scenePos().x()-16,event->scenePos().y()-16,32,32);
                    QList<QGraphicsItem*> items = this->items(r);
                    for (int i = 0; i < items.count();i++)
                    {
                        if (items.at(i)->type()==(QGraphicsItem::UserType+2))
                        {
                            ModelItem * mi = ((GraphicsPathItemModelItem*)items.at(i))->getParentItem();
                            if (mi->getSlotTrackInfo()!=NULL)
                            {
                                ///int j = 0;
                                ///while (mi->getSlotTrackInfo()->getBorderEndPoint(j)!=NULL)
                                for (int j = 0; j < mi->getSlotTrackInfo()->getBorderEndPoints()->count();j++)
                                {
                                    QPointF ePos = event->scenePos();
                                    if (pointsAreCloseEnough(&ePos,mi->getSlotTrackInfo()->getBorderEndPoints()->at(j)))
                                    {
                                        if (mi->getSlotTrackInfo()->getBorders()->at(j)!=NULL)
                                            break;

                                        *pos=*mi->getSlotTrackInfo()->getBorderEndPoints()->at(j);
                                        app->getWindow()->getWorkspaceWidget()->setActiveFragment(mi->getParentFragment());
                                        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pos);
                                        //app->getWindow()->getWorkspaceWidget()->setActiveItem(mi);
                                        app->getWindow()->getWorkspaceWidget()->selectItem(mi);
                                        i=items.count();
                                        break;
                                    }
                                }
                            }
                        }
                        else if (items.at(i)->type()==(QGraphicsItem::UserType+3))
                        {
                            //app->getWindow()->getWorkspaceWidget()->setActiveFragment(((GraphicsPathItemBorderItem*)items.at(i))->getParentItem()->getNeighbour(0)->getParentFragment());
                            app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
                            app->getWindow()->getWorkspaceWidget()->getSelection()->clear();
                            app->getWindow()->getWorkspaceWidget()->selectBorder(((GraphicsPathItemBorderItem*)items.at(i))->getParentItem());
                        }
                    }

                }
                else
                {
                    app->getWindow()->getWorkspaceWidget()->getSelection()->clear();
                    delete pos;
                }
            }
            else if (app->getWindow()->getWorkspaceWidget()->getSelectTwoPointsBend())
            {
                QPointF * pos = new QPointF(event->scenePos());
                if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                {
                    ModelFragment* frag = app->getWindow()->getWorkspaceWidget()->findFragmentByApproxPos(pos);
                    if (frag!=NULL)
                    {
                        app->getWindow()->getWorkspaceWidget()->setActiveFragment(frag);
                        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pos);
                        app->getWindow()->getWorkspaceWidget()->setActiveItem(NULL);
                        app->getWindow()->getWorkspaceWidget()->selectItem(frag->findEndPointItem(pos));
                    }
                    else
                        delete pos;
                }
                else
                {
                    ModelFragment* frag = app->getWindow()->getWorkspaceWidget()->findFragmentByApproxPos(pos);
                    if (frag!=NULL && frag==app->getWindow()->getWorkspaceWidget()->getActiveFragment())
                        app->getWindow()->getWorkspaceWidget()->bendAndClose(frag,app->getWindow()->getWorkspaceWidget()->getActiveEndPoint(),pos);
                    else if (frag!=NULL)
                        app->getWindow()->getWorkspaceWidget()->bendAndClose(app->getWindow()->getWorkspaceWidget()->getActiveFragment(),frag,app->getWindow()->getWorkspaceWidget()->getActiveEndPoint(),pos);
                    ///delete pos?
                }
            }
            else if (app->getWindow()->getWorkspaceWidget()->getSelectTwoPointsComplete())
            {
                QPointF * pos = new QPointF(event->scenePos());
                if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                {
                    ModelFragment* frag = app->getWindow()->getWorkspaceWidget()->findFragmentByApproxPos(pos);
                    if (frag!=NULL)
                    {
                        app->getWindow()->getWorkspaceWidget()->setActiveFragment(frag);
                        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pos);
                        app->getWindow()->getWorkspaceWidget()->setActiveItem(NULL);
                        app->getWindow()->getWorkspaceWidget()->selectItem(frag->findEndPointItem(pos));
                    }
                    else
                        delete pos;
                }
                else
                {
                    ModelFragment* frag = app->getWindow()->getWorkspaceWidget()->findFragmentByApproxPos(pos);
                    if (frag!=NULL && frag==app->getWindow()->getWorkspaceWidget()->getActiveFragment())
                        app->getWindow()->getWorkspaceWidget()->completeFragment(frag,app->getWindow()->getWorkspaceWidget()->getActiveEndPoint(),pos);
                    else if (frag!=NULL)
                        app->getWindow()->getWorkspaceWidget()->completeFragment(frag,app->getWindow()->getWorkspaceWidget()->getActiveEndPoint(),pos);
                    ///delete pos?
                }
            }


            //app->getWindow()->getWorkspaceWidget()->commandExecution(QString("select item %1 %2").arg(pos->x()).arg(pos->y()));
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void GraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    /*if (!this->mouseMode)
        QGraphicsScene::mouseDoubleClickEvent(event);
    else
    {*/
/*
    QString command ("make point ");
    command.append(QString::number(event->scenePos().x()));
    command.append(" ");
    command.append(QString::number(event->scenePos().y()));
    app->getWindow()->getWorkspaceWidget()->commandExecution(command);
*/

    QPointF * newPoint = new QPointF(event->scenePos());
    app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
    app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);

    QGraphicsScene::mouseDoubleClickEvent(event);
    //}
}

void GraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    //if (!this->items(event->scenePos()).empty())
    if (this->itemAt(event->scenePos(),QTransform())!=NULL)
    {
        if (this->itemAt(event->scenePos(),QTransform())->type()==QGraphicsItem::UserType+2)
            QGraphicsScene::contextMenuEvent(event);
        else
            this->contextMenuScene->exec(event->screenPos());
        /*
        if (this->itemAt(event->scenePos(),QTransform())->type()==QGraphicsItem::UserType+2)
            ((GraphicsPathItem*)this->itemAt(event->scenePos(),QTransform()))->contextMenuEvent(event);
            //this->contextMenuItem->exec(event->screenPos());*/
    }
    else
        this->contextMenuScene->exec(event->screenPos());
        //QGraphicsScene::contextMenuEvent()
}



void GraphicsScene::toggleMode()
{
    if (this->mouseMode)
        this->mouseMode=false;
    else
        this->mouseMode=true;
}


/*
QRectF GraphicsEllipseItem::boundingRect()
{
    return QRectF(0,0,100,100);
}

void GraphicsEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mousePressEvent(event);

    if (this->isSelected())
        qApp->quit();

}
*/
