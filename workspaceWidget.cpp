#include "workspaceWidget.h"
#include "globalVariables.h"

WorkspaceWidget::WorkspaceWidget(QMenu * context, QWidget * parent) : QGraphicsView(parent)//QScrollArea(parent)
{
    this->rotationMode=false;
    //this->scale=1;
    this->selection= (new QList<ModelItem*>());
    this->contextMenu = context;
    this->modelFragments = (new QList<ModelFragment*>());
    this->graphicsScene = new GraphicsScene(-2000,-1500,4000,3000);
    //this->setRenderHint(QPainter::Antialiasing);
    this->setAlignment(Qt::AlignTop);
    this->setAlignment(Qt::AlignLeft);

    this->setScene(this->graphicsScene);

    this->mousePress=false;
    this->ctrlPress=false;
    this->lastUsedPart=NULL;
    this->activeEndPoint = new QPointF(0,0);
    this->activeFragment = NULL;

    this->activeEndPointGraphic=NULL;
    this->setActiveEndPoint(this->activeEndPoint);


    QPalette pal = this->palette();
    //pal.setColor(QPalette::Window,Qt::white);
    this->setPalette(pal);
    this->setAutoFillBackground(true);

    QPolygonF poly;
    poly << QPointF(10,10);
    poly << QPointF(30,10);
    poly << QPointF(30,45);
    poly << QPointF(10,30);


    QPainterPath * pp1 = new QPainterPath();
    pp1->addPolygon(poly);
    pp1->setFillRule(Qt::OddEvenFill);


    QBrush b;
    b.setColor(Qt::white);

    b.setStyle(Qt::SolidPattern);
    this->graphicsScene->setBackgroundBrush(b);

    b.setColor(Qt::red);

    QPen pen(Qt::green);
    pen.setWidth(5);


}

void WorkspaceWidget::contextMenuEvent(QContextMenuEvent *evt)
{
    QGraphicsView::contextMenuEvent(evt);
    /**
    TODO
    */
}
void WorkspaceWidget::mousePressEvent(QMouseEvent *evt)
{
    QGraphicsView::mousePressEvent(evt);
    /**
    TODO
    */

    //if (evt->button()==Qt::RightButton)
        //this->scaleView(2);
    this->mousePress=true;

}
void WorkspaceWidget::mouseReleaseEvent(QMouseEvent *evt)
{
    QGraphicsView::mouseReleaseEvent(evt);
    /**
    TODO
    */

    /*
     *for each fragment check if any start/end point is close enough to any start/endpoint of "this"
     *
    */

    this->mousePress=false;

}
void WorkspaceWidget::wheelEvent(QWheelEvent *evt)
{

    if (evt->delta()>0 && this->ctrlPress)
        this->scaleView(1.05);
    else if (evt->delta()<0 && this->ctrlPress)
        this->scaleView(0.95);
    else
        QGraphicsView::wheelEvent(evt);
    //this->scale+=(evt->delta())/127;
    //this->scaleView(evt->delta()/63.2);
}


void WorkspaceWidget::mouseMoveEvent(QMouseEvent *evt)
{
    QGraphicsView::mouseMoveEvent(evt);
    /**
    TODO
    */
    if (this->mousePress==true && ctrlPress==true)
    {

    QBrush b;
    b.setColor(Qt::green);

    b.setStyle(Qt::SolidPattern);
    this->graphicsScene->setBackgroundBrush(b);
    }
}

void WorkspaceWidget::scaleView(qreal factor)
{
    qreal num = matrix().scale(factor,factor).mapRect(QRectF(0,0,1,1)).width();
    if (num>0.05)
        scale(factor,factor);
}

void WorkspaceWidget::keyPressEvent(QKeyEvent *event)
{
    //QGraphicsView::keyPressEvent(event);
    if (event->key()==Qt::Key_Control)
        ctrlPress=true;

    if(event->key()==Qt::Key_Space && this->lastUsedPart!=NULL)
    {
        GraphicsPathItem * gpi = this->lastUsedPart->get2DModelNoText();
        QPointF pt;
        if (this->lastUsedPart->getRadius()<0)
            pt = QPointF(gpi->scenePos().x()+1,gpi->scenePos().y()+1);
        else
            pt = QPointF(gpi->scenePos().x()+gpi->boundingRect().width()-2,gpi->scenePos().y()+gpi->boundingRect().height()-2);

        makeNewItem(*this->lastEventPos,gpi,this->lastUsedPart,this->lastUsedPart, true);
    }

    if (event->key()==Qt::Key_Delete)
    {
        for (int i = 0; i < this->modelFragments->count(); i++)
        {
            for (int j = 0; j < this->modelFragments->at(i)->getFragmentItems()->count(); j++)
            {
                if (this->modelFragments->at(i)->getFragmentItems()->at(j)->get2DModelNoText()->isSelected())
                    this->modelFragments->at(i)->deleteFragmentItem(this->modelFragments->at(i)->getFragmentItems()->at(j));
            }
        }
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






    if (event->key()==Qt::Key_Plus)
        scaleView(1.05);
    if (event->key()==Qt::Key_Minus)
        scaleView(0.95);
}
void WorkspaceWidget::keyReleaseEvent(QKeyEvent *event)
{

    if (event->key()==Qt::Key_Control)
        ctrlPress=false;
    //QGraphicsView::keyReleaseEvent(event);
}

int WorkspaceWidget::selectItem(ModelItem* item)
{
    if (item==NULL)
        return 1;
    this->selection->push_back(item);
    return 0;
}
int WorkspaceWidget::deselectItem(ModelItem* item)
{
    if (item==NULL)
        return 1;
    bool success = this->selection->removeOne(item);
    if (success)
        return 0;
    else
        return 2;
}

int WorkspaceWidget::connectFragments(ModelFragment * a, ModelFragment * b, QPointF * aP, QPointF * bP)
{
    /**
      TODO
    */
    return 0;
}

int WorkspaceWidget::connectFragments(int index1, int index2, QPointF * aP, QPointF * bP)
{
    /**
      TODO
    */
    return 0;
}
/*
int WorkspaceWidget::disconnectFragments(ModelFragment * a, ModelFragment * b)
{
    / **
      TODO
    * /
    return 0;
}
int WorkspaceWidget::disconnectFragments(int index1, int index2)
{
    / **
      TODO
    * /
    return 0;
}
*/
int WorkspaceWidget::addFragment(ModelFragment * frag)
{
    if (frag==NULL)
        return 1;
    this->modelFragments->push_back(frag);
    QList<ModelItem*>::Iterator iter = frag->getFragmentItems()->begin();

    while (iter!=frag->getFragmentItems()->end())
    {
        //(*iter)->get2DModelNoText()->moveBy(this->graphicsScene->width()/2,this->graphicsScene->height()/2);
        this->graphicsScene->addItem((*iter)->get2DModelNoText());

        iter++;
    }


    QList<QGraphicsItem*> list = this->graphicsScene->items(Qt::DescendingOrder);
    return 0;
}
int WorkspaceWidget::removeFragment(ModelFragment * frag)
{
    if (frag==NULL)
        return 1;
    bool success = this->modelFragments->removeOne(frag);
    if (success)
        return 0;
    else
        return 2;
}

int WorkspaceWidget::removeFragment(int index)
{
    if (index < 0 || index >= this->modelFragments->size())
        return 1;
    this->modelFragments->removeAt(index);
    return 0;
}

int WorkspaceWidget::updateFragment(ModelFragment *frag)
{
    for (int i = 0; i < frag->getFragmentItems()->count();i++)
    {
        //removes all items (just added items won't have effect on this action)
        this->graphicsScene->removeItem(frag->getFragmentItems()->at(i)->get2DModelNoText());
        //add all items including those just added
        this->graphicsScene->addItem(frag->getFragmentItems()->at(i)->get2DModelNoText());
    }
    return 0;
}

ModelFragment *WorkspaceWidget::findFragmentByApproxPos(QPointF *point)
{
    ModelFragment * pointer = NULL;
    QRectF rect(point->x()-5,point->y()-5,10,10);
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

ModelFragment *WorkspaceWidget::getActiveFragment() const
{
    return this->activeFragment;
}

void WorkspaceWidget::setActiveFragment(ModelFragment *frag)
{
    this->activeFragment=frag;
}

QPointF *WorkspaceWidget::getActiveEndPoint() const
{
    return this->activeEndPoint;
}

int WorkspaceWidget::setActiveEndPoint(QPointF *pt)
{
    if (pt==NULL)
        return 1;
    this->activeEndPoint=pt;

    if (this->activeEndPointGraphic!=NULL)
    {
        this->graphicsScene->removeItem(this->activeEndPointGraphic);
    }
    QPainterPath * pp = new QPainterPath();
    pp->addEllipse(0,0,1,1);

    QGraphicsPathItem * qgpi = new QGraphicsPathItem(*pp);
    QPen p = qgpi->pen();
    p.setWidth(8);

    qgpi->setPen(p);
    qgpi->moveBy(pt->x(),pt->y());
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

void WorkspaceWidget::setLastEventPos(QPointF point)
{
    delete this->lastEventPos;
    this->lastEventPos = new QPointF(point);
}

GraphicsScene *WorkspaceWidget::getGraphicsScene() const
{
    return this->graphicsScene;
}

bool WorkspaceWidget::getRotationMode()
{
    return this->rotationMode;
}

void WorkspaceWidget::toggleRotationMode()
{
    if (this->rotationMode==true)
        this->rotationMode=false;
    else
        this->rotationMode=true;
}



/*
int WorkspaceWidget::setAppPointer(void * app_ptr)
{
    if (app_ptr==NULL)
        return 1;
    this->app_ptr=app_ptr;
    return 0;
}
void * WorkspaceWidget::getAppPointer() const
{
    return this->app_ptr;
}
*/

