
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
    this->itemWidth=width;
    this->itemHeight=height;//+=prodLine->getScaleEnum();


    this->availableCount=10;
    this->prodLine=prodLine;
    this->parentFragment=NULL;
    this->parentWidget = parentWidg;

    this->contourModel=NULL;
    this->contourModelNoText=NULL;
    this->glModel=NULL;

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

    qreal gauge = this->prodLine->getScaleEnum()/2.0;
    if (this->radius>0)
    {

        rectOuter = QRectF(this->radius+gauge/1.0,-this->radius-gauge/1.0,-this->radius*2-2*gauge,this->radius*2+2*gauge);
        //rectOuter = QRectF(-this->radius-gauge/2.0,-this->radius-gauge/2.0,this->radius*2+gauge,this->radius*2+gauge);
        rectInner = QRectF(-this->radius+gauge/1.0,-this->radius+gauge/1.0,this->radius*2-2*gauge,this->radius*2-2*gauge);
    }
    else
    {
        rectOuter = QRectF(this->radius-gauge/1.0,-this->radius+gauge/1.0,-this->radius*2+2*gauge,this->radius*2-2*gauge);
        //rectOuter = QRectF(-this->radius+gauge/2.0,-this->radius+gauge/2.0,this->radius*2-gauge,this->radius*2-gauge);
        rectInner = QRectF(-this->radius-gauge/1.0,-this->radius-gauge/1.0,this->radius*2+2*gauge,this->radius*2+2*gauge);
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

    if (this->t==C1)
    {
        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,2*this->getTurnDegree());



        //itemPath.arcMoveTo(rectInner,startAngle);
        itemPath.arcTo(rectInner,startAngle,2*this->getTurnDegree());


        itemPath.closeSubpath();

    }
    else if (this->t==E1 || this->t==S1)
    {
        if (this->radius>0)
        {
            QPolygonF line;
            line << QPointF(-this->radius,-this->radius-gauge/1.0);
            line << QPointF(this->radius,-this->radius-gauge/1.0);
            line << QPointF(this->radius,-this->radius+gauge/1.0);
            QPolygonF line2;
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
            QPolygonF line2;
            line2 << QPointF(-this->radius,-this->radius-gauge/1.0);
            line2 << QPointF(this->radius,-this->radius-gauge/1.0);
            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);
            itemPath.translate(0,-gauge);
        }

    }
    else if (this->t==J1)
    {
        /* radius > 0
                     __====1
             _____---_____
            0_____---_____ 2

            radius < 0

            1===___
              _____---_____
             0_____---_____ 2


             angles > 0 => right turnout thus:
                0--1 is straight
                2--1 is straight
             else left turnout
        */


        itemPath.arcMoveTo(rectOuter,startAngle);
        itemPath.arcTo(rectOuter,startAngle,2*this->getTurnDegree());
        itemPath.arcTo(rectInner,startAngle,2*this->getTurnDegree());


        itemPath.closeSubpath();
        if (this->parentWidget!=NULL)
            itemPath.translate(0,-this->itemHeight+gauge);
        else
            itemPath.translate(0,-this->itemHeight);
        //if angle[2] == 0



        if (this->radius>0 )
        {
            QPolygonF line;
            line << QPointF(this->endPoints->at(0)->x(),-radius+this->endPoints->at(0)->y()-gauge);
            line << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()-gauge);
            line << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()+gauge);

            QPolygonF line2;
            line2 << QPointF(this->endPoints->at(0)->x(),-radius+this->endPoints->at(0)->y()+gauge);
            line2 << QPointF(this->endPoints->at(1)->x(),-radius+this->endPoints->at(1)->y()+gauge);

            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);

            itemPath.translate(0,(this->itemHeight-gauge));

        }
        else
        {


            (this->endPoints->at(1))->setX((-1)*(this->endPoints->at(1)->x()));
            (this->endPoints->at(1))->setY((-1)*(this->endPoints->at(1)->y()));

            (*(++this->endPointsAngles->begin()))-=180;
            (*(++this->endPointsAngles->begin()))*=-1;
            //(*(++(++this->endPointsAngles->begin())))-=180;



            QPolygonF line;
            line << QPointF(this->endPoints->at(2)->x(),-this->radius+this->endPoints->at(2)->y()-this->itemHeight-(this->itemHeight-gauge));
            line << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge));
            line << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);

            QPolygonF line2;
            line2 << QPointF(this->endPoints->at(2)->x(),-this->radius+this->endPoints->at(2)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);
            line2 << QPointF(this->endPoints->at(1)->x(),-this->radius+this->endPoints->at(1)->y()-this->itemHeight-(this->itemHeight-gauge)+2*gauge);

            //itemPath.translate(0,-(this->itemHeight-gauge));
            itemPath.addPolygon(line);
            itemPath.addPolygon(line2);
            //itemPath.translate(0,(this->itemHeight-gauge));
            itemPath.translate(0,(this->itemHeight-gauge));

        }


    }
    if (this->t==J2)
    {

    }


    GraphicsPathItem * gpi = new GraphicsPathItem(this);
    gpi->setFlag(QGraphicsItem::ItemIsMovable,false);
    gpi->setFlag(QGraphicsItem::ItemIsSelectable,true);

    //qreal num = 0*(this->itemHeight-this->prodLine->getScaleEnum()/2);
    qreal num = (this->itemHeight-this->prodLine->getScaleEnum()/2.0);
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
    /**
      *TODO
      *-following works just for parts with 2 endPoints
      *-add rotation of smaller rectangles (with i.e. junction points) to get correct new positions
      */
    QTransform rot;
    QTransform rot2;
    rot2 = rot.rotate(angle);
    this->contourModelNoText->setTransform(rot);
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

void ModelItem::moveBy(qreal dx, qreal dy)
{
    this->get2DModelNoText()->moveBy(dx,dy);
    for(int i = 0; i < this->endPoints->count(); i++)
    {
        this->endPoints->at(i)->setX(this->endPoints->at(i)->x()+dx);
        this->endPoints->at(i)->setY(this->endPoints->at(i)->y()+dy);
    }
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
                    endDegrees->push_back(-toMake->getTurnDegree(index));
                else
                    endDegrees->push_back(toMake->getTurnDegree(index));
                endPoints->push_back(*toMake->getEndPoint(index));
                index++;
            }


            if (eventPos.x()<gpi->pos().x()/2)
            {
                if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                    //endDegrees->push_front(toMake->getTurnDegree(0));
                    //endDegrees->push_front(180-toMake->getTurnDegree(0));
                    *(endDegrees->begin())=180-toMake->getTurnDegree(0);

                /*else
                    endDegrees->push_front(-toMake->getTurnDegree(0));

                endDegrees->push_back(-toMake->getTurnDegree());
                */
            }
            else
            {
                if (app->getWindow()->getWorkspaceWidget()->getActiveFragment()==NULL)
                    //endDegrees->push_front(toMake->getTurnDegree(0));
                    //endDegrees->push_front(180+toMake->getTurnDegree(0));
                    *(endDegrees->begin())=180+toMake->getTurnDegree(0);
                /*
                else
                    endDegrees->push_front(toMake->getTurnDegree(0));
                endDegrees->push_back(toMake->getTurnDegree());*/
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
                }
                //no need to generate model with text, because text isn't needed in workspace scene
                it->generate2DModel(false);

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

                QPointF * newActive = new QPointF(*fragment->getEndPoints()->at(1));
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
                }

                //no need to generate model with text, because text isn't needed in workspace scene
                it->generate2DModel(false);

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
    QList<QPointF*>::Iterator iter = this->endPoints->begin();
    int i = 0;
    while(iter!=this->endPoints->end())
    {
        if (**iter==*point)
        {

            //this->endPoints->removeAt(i);
            break;
        }
        iter++;
        i++;
    }

    //get frag. rotation and then remove it from the list - won't be used
    qreal currentFragmentRotation = this->endPointsAngles->at(i);
    //this->endPointsAngles->removeAt(i);
    !!this->removeEndPoint(*iter);


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

    item->moveBy(dx,dy);

    qreal newFragmentRotation = item->getTurnDegree();

    this->endPointsAngles->push_back(newFragmentRotation);


    app->getWindow()->getWorkspaceWidget()->updateFragment(this);



    QPointF * newPt = new QPointF(item->getEndPoint()->x(),item->getEndPoint()->y());

    bool connected = false;

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



    if (connected)
    {
        *newPt=QPointF(0,0);
        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        app->getWindow()->getWorkspaceWidget()->setActiveFragment(NULL);
        this->endPointsAngles->removeOne(newFragmentRotation);
    }
    else
    {
        app->getWindow()->getWorkspaceWidget()->setActiveEndPoint(newPt);
        //!mistake is here, because you cant insert pointer which isnt part of any modelItem - you have to find the point of same value;
        //!this->addEndPoint(newPt);

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

int ModelFragment::addEndPoint(QPointF *pt)
{
    if (pt==NULL)
        return 1;

    QGraphicsEllipseItem * qgpi = new QGraphicsEllipseItem(pt->x(),pt->y(),1,1);

    QPen p = qgpi->pen();
    p.setWidth(4);
    qgpi->setPen(p);

    app->getWindow()->getWorkspaceWidget()->getGraphicsScene()->addItem(qgpi);

    this->endPointsGraphics->push_back(qgpi);
    this->endPoints->push_back(pt);

    return 0;
}
