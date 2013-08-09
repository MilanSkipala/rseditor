#include "workspaceWidget.h"

WorkspaceWidget::WorkspaceWidget(QMenu * context, QWidget * parent) : QGraphicsView(parent)//QScrollArea(parent)
{
    //this->scale=1;
    this->selection= (new QList<ModelItem*>());
    this->contextMenu = context;
    this->modelFragments = (new QList<ModelFragmentWidget*>());
    //this->frameWidget = new QFrame(this,this->windowFlags());
    this->graphicsScene = new QGraphicsScene(0,0,2000,3000);
    this->setScene(this->graphicsScene);
    this->mousePress=false;
    this->ctrlPress=false;
    this->lastUsedPart=NULL;
    this->activeEndPoint = NULL;
    this->activeFragment = NULL;



    QPalette pal = this->palette();
    //pal.setColor(QPalette::Window,Qt::white);
    this->setPalette(pal);
    this->setAutoFillBackground(true);

    ///this->move(150,0);
    //this->frameWidget->setFixedWidth(5000);
    //this->frameWidget->setFixedHeight(5000);


    ///this->resize(550,420);


    //this->graphicsView = new QGraphicsView(this->graphicsScene,this);
    //this->setWidget(this->graphicsView);


    QPolygon poly;
    poly << QPoint(10,10);
    poly << QPoint(30,10);
    poly << QPoint(30,45);
    poly << QPoint(10,30);


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


    QString s1("123");
    QString s2("Part1");
    QString s3("Dilek1");
    QPoint pt(0,0);
    QPoint pt2(10,10);
    QWidget * parentWidg = this;//parent;//(((Application*)(this->app_ptr))->getWindow()->getSideBarWidget());
    //ModelItem * mi = new ModelItem(&s1,&s2,&s3,&pt,&pt2,90,35,NULL, parentWidg);
    //ModelItem * mi = new ModelItem(&s1,&s2,&s3,&pt,&pt2,90,35,parentWidg);

    ///wrong
    //mi->get2DModel()->moveBy(50,50);

    //GraphicsPathItem * gpi = new GraphicsPathItem(*pp1);

    //gpi->setBrush(b);
    //gpi->setPen(pen);


    //this->graphicsScene->addItem(gpi);
    //this->graphicsScene->addItem(mi->get2DModel());



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
    QGraphicsView::wheelEvent(evt);
    //this->scale+=(evt->delta())/127;
    this->scaleView(evt->delta()/128);
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
    if (num>0.2)
        scale(factor,factor);
}

void WorkspaceWidget::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
    if (event->key()==Qt::Key_Control)
        ctrlPress=true;
}
void WorkspaceWidget::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsView::keyReleaseEvent(event);
    if (event->key()==Qt::Key_Control)
        ctrlPress=false;
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

int WorkspaceWidget::connectFragments(ModelFragmentWidget * a, ModelFragmentWidget * b, QPoint * aP, QPoint * bP)
{
    /**
      TODO
    */
    return 0;
}

int WorkspaceWidget::connectFragments(int index1, int index2, QPoint * aP, QPoint * bP)
{
    /**
      TODO
    */
    return 0;
}

int WorkspaceWidget::disconnectFragments(ModelFragmentWidget * a, ModelFragmentWidget * b)
{
    /**
      TODO
    */
    return 0;
}
int WorkspaceWidget::disconnectFragments(int index1, int index2)
{
    /**
      TODO
    */
    return 0;
}

int WorkspaceWidget::addFragment(ModelFragmentWidget * frag)
{
    if (frag==NULL)
        return 1;
    this->modelFragments->push_back(frag);
    QList<ModelItem*>::Iterator iter = frag->getFragmentItems()->begin();

    while (iter!=frag->getFragmentItems()->end())
    {
        (*iter)->get2DModel()->moveBy(this->graphicsScene->width()/2,this->graphicsScene->height()/2);
        this->graphicsScene->addItem((*iter)->get2DModel());

        iter++;
    }


    QList<QGraphicsItem*> list = this->graphicsScene->items(Qt::DescendingOrder);
    return 0;
}
int WorkspaceWidget::removeFragment(ModelFragmentWidget * frag)
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

ModelFragmentWidget *WorkspaceWidget::getActiveFragment() const
{
    return this->activeFragment;
}

int WorkspaceWidget::setActiveFragment(ModelFragmentWidget *frag)
{
    if (frag==NULL)
        return 1;

    this->activeFragment=frag;
    return 0;
}

QPoint *WorkspaceWidget::getActiveEndPoint() const
{
    return this->activeEndPoint;
}

int WorkspaceWidget::setActiveEndPoint(QPoint *pt)
{
    if (pt==NULL)
        return 1;
    this->activeEndPoint=pt;
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

