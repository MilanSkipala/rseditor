#include "graphicsScene.h"
#include "globalVariables.h"

GraphicsScene::GraphicsScene(QObject *parent) : QGraphicsScene(parent)
{
    this->mouseMode=false;
}

GraphicsScene::GraphicsScene(const QRectF &sceneRect, QObject *parent) : QGraphicsScene(sceneRect,parent)
{
    this->mouseMode=false;

}

GraphicsScene::GraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent) : QGraphicsScene(x,y,width,height,parent)
{
    this->mouseMode=false;
}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (this->mouseMode)
        QGraphicsScene::mousePressEvent(event);
    else
    {
        QPointF * pos = new QPointF(event->scenePos());
        ModelFragment* frag = app->getWindow()->getWorkspaceWidget()->findFragmentByApproxPos(pos);
        if (frag!=NULL)
        {
            app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(pos);
            app->getWindow()->getWorkspaceWidget()->setActiveFragment(frag);
        }
        else
            delete pos;
    }

    QGraphicsScene::mousePressEvent(event);
}

void GraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!this->mouseMode)
        QGraphicsScene::mouseDoubleClickEvent(event);
    else
    {

        QPointF * newPoint = new QPointF(event->scenePos());
        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPoint);
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);

        QGraphicsScene::mouseDoubleClickEvent(event);
    }
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
