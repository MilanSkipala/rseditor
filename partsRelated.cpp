#include "application.h"
#include "partsRelated.h"
#include "globalVariables.h"


ModelFragmentWidget::ModelFragmentWidget()
{
    this->endPoints = new QList<QPoint *>();
    this->endPointsAngles = new QList <qreal>();
    this->fragmentItems = new QList <ModelItem*>();
    this->lines = new QList <ProductLine*>();
    this->startPoint = NULL;
    this->startPointAngle = 0;



}

ModelFragmentWidget::ModelFragmentWidget(ModelItem* item, ProductLine* line, QPoint * startPt,QList<QPoint *> *endPts,QList<qreal> * endPtAngles)
{
    this->endPoints = endPts;
    this->endPointsAngles = endPtAngles;
    this->fragmentItems = new QList <ModelItem*>();
    this->lines = new QList <ProductLine*>();
    this->startPoint = startPt;
    this->startPointAngle = 0;
    //ModelFragment constructor can't allocate new ModelItem (which will be then inserted in the scene) - it has to be done in the doubleclickevent
    //because of basic left/right turn rotation

    //ModelItem * it = new ModelItem(*item);



    this->fragmentItems->push_back(item);
    this->lines->push_back(line);


}


QList <ModelItem*> * ModelFragmentWidget::getFragmentItems() const
{
    return this->fragmentItems;
}

QList<ProductLine*> * ModelFragmentWidget::getProductLines() const
{
    return this->lines;
}

QPoint * ModelFragmentWidget::getStartPoint() const
{
    return this->startPoint;
}

QList<QPoint *> * ModelFragmentWidget::getEndPoints() const
{
    return this->endPoints;
}

qreal ModelFragmentWidget::getStartPointAngle()
{
    return this->startPointAngle;
}

QList<qreal> * ModelFragmentWidget::getEndPointsAngles() const
{
    return this->endPointsAngles;
}

int ModelFragmentWidget::addFragmentItems(QList <ModelItem*> * listToAppend)
{
    if (listToAppend==NULL)
        return 1;
    this->fragmentItems->append(*listToAppend);
    return 0;
}

int ModelFragmentWidget::addFragmentItem(ModelItem* item)
{
    if (item == NULL)
        return 1;
    this->fragmentItems->push_back(item);
    return 0;
}

int ModelFragmentWidget::setStartPoint(QPoint * pt)
{
    if (pt==NULL)
        return 1;
    this->startPoint=pt;
    return 0;
}

int ModelFragmentWidget::addEndPoints (QList<QPoint *> * listToAppend)
{
    if (listToAppend==NULL)
        return 1;
    this->endPoints->append(*listToAppend);
    return 0;
}

int ModelFragmentWidget::removeEndPoint (QPoint * pt)
{
    if (pt == NULL)
        return 1;
    bool success = this->endPoints->removeOne(pt);
    if (success)
        return 0;
    else
        return 2;
}

int ModelFragmentWidget::removeEndPoint (int index)
{
    if (index < 0 || index >= this->endPoints->size())
        return 1;
    this->endPoints->removeAt(index);
    return 0;
}

int ModelFragmentWidget::removeStartPoint()
{
    delete this->startPoint;
    this->startPoint=NULL;
    return 0;
}

int ModelFragmentWidget::setStartPointAngle(qreal angle)
{
    this->startPointAngle = angle;
    return 0;
}

int ModelFragmentWidget::setEndPointAngle(QPoint * pt, qreal angle)
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

int ModelFragmentWidget::setEndPointAngle(int index, qreal angle)
{
    QList<qreal>::Iterator iter = this->endPointsAngles->begin();
    if (index < 0 || index >=this->endPointsAngles->size())
        return 1;
    for (int i = 0; i <= index; i++)
        iter++;
    *iter=angle;
    return 0;
}

//ModelItem::ModelItem(QString * partNumber, QString * partNameEn, QString * partNameCs,
//                     QPoint * start, QPoint * end ,qreal degree, qreal turnRadius,
//                     void * appPointer, QWidget * parentWidg, QGraphicsItem * contourParent)

ModelItem::ModelItem(QString & partNumber, QString & partNameEn, QString & partNameCs,
                     QPoint & start, QPoint & end ,qreal degree,
                     qreal turnRadius, ProductLine * prodLine, QWidget * parentWidg ,QGraphicsItem * contourParent)/// : QWidget (parentWidg)
{
    //this->app_ptr = appPointer;


    this->partNo = new QString(partNumber);
    this->nameEn = new QString(partNameEn);
    this->nameCs = new QString(partNameCs);
    this->startPoint = new QPoint(start);
    this->endPoint = new QPoint(end);
    this->turnDegree=degree;
    this->radius=turnRadius;

    this->availableCount=10;
    this->prodLine=prodLine;
    this->parentFragment=NULL;
    this->parentWidget = parentWidg;


    //this->label = new QLabel(parentWidg);

    /*
    if (app_ptr->getUserPreferences()->getLocale()="Cs")
        this->label->setText(this->nameCs);
    else
        this->label->setText(this->nameEn);
*/

    ///incorrect
    this->glModel = new QGLWidget();



}

ModelItem::ModelItem(ModelItem &orig)
{
    this->availableCount=orig.availableCount;
    this->contourModel= new GraphicsPathItem(&orig,orig.get2DModel()->path());
    this->contourModel->setFlag(QGraphicsItem::ItemIsMovable);
    this->contourModel->setFlag(QGraphicsItem::ItemIsSelectable);
    this->contourModel->setVisible(true);

    this->endPoint=orig.endPoint;
    this->glModel=orig.glModel;
    //this->label=unused
    this->nameCs=orig.nameCs;
    this->nameEn=orig.nameEn;

    //may cause errors
    this->parentWidget=app->getWindow()->getWorkspaceWidget();

    this->partNo=orig.partNo;
    this->prodLine=orig.prodLine;
    this->radius=orig.radius;
    this->startPoint=orig.startPoint;
    this->turnDegree=orig.turnDegree;
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
QPoint * ModelItem::getStartPoint() const
{
    return this->startPoint;
}
QPoint * ModelItem::getEndPoint() const
{
    return this->endPoint;
}

qreal ModelItem::getTurnDegree() const
{
    return this->turnDegree;
}
qreal ModelItem::getRadius() const
{
    return this->radius;
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

int ModelItem::generate2DModel()
{
    QRectF rectOuter(-this->radius+32,-this->radius+32,this->radius*2-64,this->radius*2-64);
    QRectF rectInner(-this->radius+24,-this->radius+24,this->radius*2-48,this->radius*2-48);

    QPainterPath itemPath;

    qreal startAngle = 90-(this->turnDegree/2);

    QFont font;
    font.setPixelSize(10);
    QString label(*this->partNo);
    label.append(" ");
    QString s("");
    if (this->turnDegree!=0)
        s.setNum(this->turnDegree,'f',1);
    else
        s.setNum(this->turnDegree,'f',0);

    label.append(s);
    label.append("° ");
    s.setNum(this->radius);
    label.append(s);



    ///QPainterPath textPath;
    itemPath.addText(-(6*this->partNo->length())/2,48-this->radius,font,*this->partNo);

    if (this->turnDegree!=0)
    {
        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,this->turnDegree);
        itemPath.arcMoveTo(rectInner,startAngle);
        itemPath.arcTo(rectInner,startAngle,this->turnDegree);
    }
    else
    {
        QPolygon line;
        line << QPoint(-this->radius/2,-this->radius/2);
        line << QPoint(this->radius/2,-this->radius/2);
        QPolygon line2;
        line2 << QPoint(-this->radius/2,-this->radius/2+8);
        line2 << QPoint(this->radius/2,-this->radius/2+8);

        itemPath.addPolygon(line);
        itemPath.addPolygon(line2);
    }



    GraphicsPathItem * gpi = new GraphicsPathItem(this);
    gpi->setFlag(QGraphicsItem::ItemIsMovable,true);
    gpi->setFlag(QGraphicsItem::ItemIsSelectable,true);


    gpi->setPath(itemPath);
    gpi->changeCountPath(10,this->radius);
    gpi->setToolTip(label);

    ///GraphicsPathItem *gpiText = new GraphicsPathItem(gpi);
    ///gpiText->setPath(textPath);
    ///QBrush b = gpiText->brush();
    ///b.setColor(Qt::red);
    ///b.setStyle(Qt::SolidPattern);
    ///QPen p = gpiText->pen();
    ///p.setWidth(0);
    ///gpiText->setPen(p);
    ///gpiText->setBrush(b);

    ///gpiText->setVisible(false);



    this->contourModel=gpi;

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

ProductLine *ModelItem::getProdLine() const
{
    return this->prodLine;
}


void ModelItem::moveLabel(QPoint * point)
{
    //this->label->move(*point);
}

QWidget *ModelItem::getParentWidget() const
{
    return this->parentWidget;
}

GraphicsPathItem::GraphicsPathItem(ModelItem * item, QGraphicsItem * parent) : QGraphicsPathItem(parent)
{
    //this->mouseMoveOffset = new QPoint(0,0);
    this->setFlag(QGraphicsItem::ItemIsMovable,true);
    this->setFlag(QGraphicsItem::ItemIsSelectable,true);
    this->restrictedCountPath = NULL;
    this->parentItem=item;

    //if (this->parentItem()==NULL)
    //    this->changeCountPath(10);
}
GraphicsPathItem::GraphicsPathItem(ModelItem * item, const QPainterPath & path, QGraphicsItem * parent) : QGraphicsPathItem(path,parent)
{
    //this->mouseMoveOffset = new QPoint(0,0);
    this->restrictedCountPath = NULL;
    this->parentItem=item;
    //if (this->parentItem()==NULL)
    //    this->changeCountPath(10);
}


/*
void GraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsPathItem::mousePressEvent(event);
    //QGraphicsScene::mousePressEvent(event);

    if (this->contains(event->pos()))
        *(this->mouseMoveOffset) = event->pos();
/*
    QPen p = this->pen();
    p.setWidth(15);
    p.setColor(Qt::black);
    this->setPen(p);

    QBrush b = this->brush();
    b.setColor(Qt::yellow);
    this->setBrush(b);

    if ((event->pos().x()>this->pos().x() ||event->pos().y()>this->pos().y()))
    {
        p.setColor(Qt::yellow);
        this->setPen(p);

    }

  * /
    this->update();
    //QGraphicsItem::mousePressEvent(event);



}
void GraphicsPathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsPathItem::mouseReleaseEvent(event);
    //QGraphicsScene::mouseReleaseEvent(event);
    /*
    QPen p = this->pen();
    p.setWidth(5);
    this->setPen(p);

    QBrush b = this->brush();
    b.setColor(Qt::red);
    this->setBrush(b);
    * /
    QGraphicsPathItem::mouseReleaseEvent(event);
}

void GraphicsPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsPathItem::mouseMoveEvent(event);
    //QGraphicsScene::mouseMoveEvent(event);

    this->moveBy(event->pos().x()-this->mouseMoveOffset->x(),event->pos().y()-this->mouseMoveOffset->y());
    this->update();
}
*/


bool GraphicsPathItem::contains(const QPoint &point) const
{
    return this->path().boundingRect().contains(point);
    //return QRectF(this->pos().x(),this->pos().y(),100,64).contains(point);

}

QRectF GraphicsPathItem::boundingRect() const
{
    //return QRectF(this->pos().x(),this->pos().y(),100,64);
    return this->path().controlPointRect();
}

QPainterPath GraphicsPathItem::shape() const
{
    QPainterPath * p = new QPainterPath();
    p->addRect(this->path().controlPointRect());
    //->addRect(this->pos().x(),this->pos().y(),100,64);
    return *p;
}

void GraphicsPathItem::changeCountPath(unsigned int count, qreal radius)
{
    GraphicsPathItem * newItem = new GraphicsPathItem(this->parentItem,this);

    QPainterPath pp;
    QRectF r(-50+5,5+38-(radius),15,13);
    pp.addRect(r);
    QFont font;
    font.setPixelSize(10);
    QString text;
    text.setNum(count);

    pp.addText(-49+5,5+49-(radius),font,text);
    QBrush b = newItem->brush();
    b.setColor(Qt::green);
    b.setStyle(Qt::SolidPattern);
    newItem->setBrush(b);
    newItem->setPath(pp);

    if (this->restrictedCountPath!=NULL)
        delete this->restrictedCountPath;

    this->restrictedCountPath=newItem;

}

void GraphicsPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (app->getRestrictedInventoryMode() && this->restrictedCountPath!=NULL)
        this->restrictedCountPath->setVisible(true);
    else if (!app->getRestrictedInventoryMode() && this->restrictedCountPath!=NULL)
        this->restrictedCountPath->setVisible(false);
    QGraphicsPathItem::paint(painter,option,widget);
}

void GraphicsPathItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    bool enable = true;
    if (app->getRestrictedInventoryMode())
    {
        if (this->parentItem->getAvailableCount()<1)
            enable = false;
        this->parentItem->decrAvailableCount();
        this->changeCountPath(this->parentItem->getAvailableCount(),this->parentItem->getRadius());
    }
    if (!(this->parentItem->getParentWidget()==app->getWindow()->getWorkspaceWidget()) && enable)
    {

        ///encapsulate all of these lines in if (currentPoint?? == NULL)
        if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
        {
            QList<qreal> * endDegrees = new QList<qreal>();
            if (event->scenePos().x()<this->pos().x()/2)
                endDegrees->push_back(this->parentItem->getTurnDegree());
            else
            {
                endDegrees->push_back(-this->parentItem->getTurnDegree());
            }
            QList<QPoint*> * endPoints = new QList<QPoint*>();
            endPoints->push_back(this->parentItem->getEndPoint());


            ModelItem * it = new ModelItem(*this->parentItem->getPartNo(),*this->parentItem->getNameEn(),*this->parentItem->getNameCs(),*this->parentItem->getStartPoint(),*this->parentItem->getEndPoint(),this->parentItem->getTurnDegree(),this->parentItem->getRadius(),this->parentItem->getProdLine(),app->getWindow()->getWorkspaceWidget());

            it->generate2DModel();
            if (event->scenePos().x()<this->pos().x()/2)
            {
                QTransform rot;
                it->get2DModel()->setTransform(rot.rotate(180));
            }

            ModelFragmentWidget * fragment = new ModelFragmentWidget(it,this->parentItem->getProdLine(),this->parentItem->getStartPoint(), endPoints, endDegrees);
            app->getWindow()->getWorkspaceWidget()->addFragment(fragment);
        }
        ///else activePoint's fragment - addItem


    }

    QGraphicsPathItem::mouseDoubleClickEvent(event);
}



ProductLine::ProductLine(QString &name,QString &scale, QString &gauge, bool type, QList<ModelItem*> &items)
{
    this->name=new QString(name);
    this->scale=new QString(scale);
    this->type=type;
    this->gauge=new QString(gauge);
    this->items=new QList<ModelItem*>(items);
    this->maxTrackRadius=0;
    this->minTrackRadius=50000;

}
ProductLine::ProductLine(QString &name,QString &scale, QString &gauge, bool type)
{
    this->name=new QString(name);
    this->scale=new QString(scale);
    this->type=type;
    this->items = new QList<ModelItem*>();
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

/*
int ProductLine::generate2DModels()
{
    QList<ModelItem*>::Iterator iter = this->items->begin();
    while (iter!=this->items->end())
    {
        /*
         *this cannot be used - doesn't preserve scale of one product line
         *

        qreal normalized = (this->maxTrackRadius-(*iter)->getRadius())/(this->maxTrackRadius-this->minTrackRadius);
        normalized = 1 - 0.2*normalized;
        int sizeOfItem = 32;
        QRectF rectOuter(-sizeOfItem*normalized,-sizeOfItem*normalized,sizeOfItem*normalized*2,sizeOfItem*normalized*2);
        sizeOfItem=24;
        QRectF rectInner(-sizeOfItem*normalized,-sizeOfItem*normalized,sizeOfItem*normalized*2,sizeOfItem*normalized*2);
        * /





        QRectF rectOuter(-(*iter)->getRadius()+32,-(*iter)->getRadius()+32,(*iter)->getRadius()*2-64,(*iter)->getRadius()*2-64);
        QRectF rectInner(-(*iter)->getRadius()+24,-(*iter)->getRadius()+24,(*iter)->getRadius()*2-48,(*iter)->getRadius()*2-48);


*/
        /**
          *TODO
          *-restricted mode - red color for 1 and 0 available parts
          *-restricted mode - add QGraphicsItem attribute and switch its visibility for items in scene
          *-hide label (=load the other model) in scene
          *-implement context menu for GPI - change item info dialog (where is also changeAvailableCountColumn)
            */

/*
        QPainterPath itemPath;

        qreal startAngle = 90-((*iter)->getTurnDegree()/2);

        QFont font;
        font.setPixelSize(10);
        QString label(*(*iter)->getPartNo());
        label.append(" ");
        QString s("");
        if ((*iter)->getTurnDegree()!=0)
            s.setNum((*iter)->getTurnDegree(),'f',1);
        else
            s.setNum((*iter)->getTurnDegree(),'f',0);

        label.append(s);
        label.append("° ");
        s.setNum((*iter)->getRadius());
        label.append(s);



        ///QPainterPath textPath;
        itemPath.addText(-(6*(*iter)->getPartNo()->length())/2,48-(*iter)->getRadius(),font,*(*iter)->getPartNo());

        if ((*iter)->getTurnDegree()!=0)
        {
            itemPath.arcMoveTo(rectOuter,startAngle);
            itemPath.arcTo(rectOuter,startAngle,(*iter)->getTurnDegree());
            itemPath.arcMoveTo(rectInner,startAngle);
            itemPath.arcTo(rectInner,startAngle,(*iter)->getTurnDegree());
        }
        else
        {
            QPolygon line;
            line << QPoint(-(*iter)->getRadius()/2,-(*iter)->getRadius()/2);
            line << QPoint((*iter)->getRadius()/2,-(*iter)->getRadius()/2);
            QPolygon line2;
            line2 << QPoint(-(*iter)->getRadius()/2,-(*iter)->getRadius()/2+8);
            line2 << QPoint((*iter)->getRadius()/2,-(*iter)->getRadius()/2+8);

            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);
        }



        GraphicsPathItem * gpi = new GraphicsPathItem(*iter);
        gpi->setFlag(QGraphicsItem::ItemIsMovable,true);
        gpi->setFlag(QGraphicsItem::ItemIsSelectable,true);


        gpi->setPath(itemPath);
        gpi->changeCountPath(10,(*iter)->getRadius());
        gpi->setToolTip(label);

        ///GraphicsPathItem *gpiText = new GraphicsPathItem(gpi);
        ///gpiText->setPath(textPath);
        ///QBrush b = gpiText->brush();
        ///b.setColor(Qt::red);
        ///b.setStyle(Qt::SolidPattern);
        ///QPen p = gpiText->pen();
        ///p.setWidth(0);
        ///gpiText->setPen(p);
        ///gpiText->setBrush(b);

        ///gpiText->setVisible(false);



        (*iter)->set2DModel(gpi);

        iter++;
    }
    return 0;
}
*/
