#include <cmath>
#include "workspaceWidget.h"
#include "globalVariables.h"

WorkspaceWidget::WorkspaceWidget(QMenu * context, QWidget * parent) : QGraphicsView(parent)//QScrollArea(parent)
{
    this->deletePress=false;
    this->rotationMode=false;
    this->heightProfileMode=false;

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
    this->lastEventPos=NULL;
    this->activeEndPoint = NULL;//new QPointF(0,0);
    this->activeFragment = NULL;
    this->activeItem = NULL;

    this->activeEndPointGraphic=NULL;
    this->setActiveEndPoint(new QPointF(0,0));


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

    if(event->key()==Qt::Key_Space && this->lastUsedPart!=NULL && !this->heightProfileMode)
    {
        for (int i = 0; i < this->activeFragment->getProductLines()->count(); i++)
        {
            if (this->activeFragment->getProductLines()->at(i)->getType()!=this->lastUsedPart->getProdLine()->getType())
            {
                app->getAppData()->setMessageDialogText("You cannot connect rail and slot parts.","Autodráhové a železniční díly nelze spojovat");
                app->getAppData()->getMessageDialog()->exec();
                return;
            }

        }
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
        this->deletePress = true;
        for (int i = 0; i < this->modelFragments->count(); i++)
        {
            for (int j = 0; j < this->modelFragments->at(i)->getFragmentItems()->count(); j++)
            {
                if (this->modelFragments->at(i)->getFragmentItems()->at(j)->get2DModelNoText()->isSelected())
                {

                    int ret = this->modelFragments->at(i)->deleteFragmentItem(this->modelFragments->at(i)->getFragmentItems()->at(j));
                    if (ret==-1)
                    {

                        delete this->modelFragments->at(i);
                        this->modelFragments->removeOne(this->modelFragments->at(i));
                        i=0;
                        j=0;
                        break;
                    }
                }
            }
        }
        this->deletePress=false;
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
int WorkspaceWidget::connectFragments(ModelFragment *a)
{
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
                if (area.contains(**bPointIter) && a!=(*fragIter) && !firstFound && *aEPItemIter!=*bEPItemIter)
                {
                    //modify neighbour of "a" endItem at this point
                    (*aEPItemIter)->setNeighbour(*bEPItemIter,*bPointIter);


                    //modify Neighbour of (*fragIter) endItem at this point
                    (*bEPItemIter)->setNeighbour(*aEPItemIter,*bPointIter);
                }

                else if (area.contains(**bPointIter) && a!=(*fragIter) && firstFound)
                {

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
                    //get info about l-r side of A
                    //aLeft = a->leftSide(*aEPItemIter, firstItemWith180Diff);
                    if (*aEPItemIter==firstItemWith180Diff && aI%2==0)
                        aLeft=true;


                    //get info about l-r side of B
                    //bLeft = b->leftSide(*bEPItemIter, uselessPointer);
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



                    logFile << "    Connecting fragments using the rebuildFragment(..)" << endl;

                    ModelFragment * c = new ModelFragment(b->getFragmentItems()->first());
                    rebuildFragment(b->getFragmentItems()->first(),c);


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

                    this->modelFragments->removeOne(b);
                    delete (b);
                    b=c;
                    this->modelFragments->push_back(b);


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
                } while (*bPointIter==NULL);


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

/*
int WorkspaceWidget::connectFragments(ModelFragment *a)
{
    /**
      NOTE: commented parts of code are in last backup

      KNOWN BUGS:
        -not working properly for rail parts
        -sequences like connect-delete-connect-... may lead into incorrect behaviour (mostly (maybe not only) 100 and 000 cases)

* /
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


    /**
      TODO:
      -check correctness of T1-T10 rail parts
    * /
    bool aEPILeft = false;//info just about EPItem
    bool bEPILeft = false;
    bool aEPI180diff = false;
    bool bEPI180diff = false;
    bool aLeft = false;//info about whole fragment
    bool bLeft = false;


    //for each point of "a":
    while(true)//(aPointIter!=a->getEndPoints()->end())
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
        while(fragIter!=this->modelFragments->end())
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
                if (area.contains(**bPointIter) && a!=(*fragIter) && !firstFound && *aEPItemIter!=*bEPItemIter)
                {
                    //modify neighbour of "a" endItem at this point
                    (*aEPItemIter)->setNeighbour(*bEPItemIter,*bPointIter);


                    //modify Neighbour of (*fragIter) endItem at this point
                    (*bEPItemIter)->setNeighbour(*aEPItemIter,*bPointIter);
                }

                else if (area.contains(**bPointIter) && a!=(*fragIter) && firstFound)
                {

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
                    //get info about l-r side of A
                    aLeft = a->leftSide(*aEPItemIter, firstItemWith180Diff);
                    if (*aEPItemIter==firstItemWith180Diff && aI%2==0)
                        aLeft=true;


                    //get info about l-r side of B
                    bLeft = b->leftSide(*bEPItemIter, uselessPointer);
                    if (*bEPItemIter==uselessPointer && bI%2==0)
                        bLeft=true;

                    qreal dAlpha =(*bEPAngleIter)-(*aEPAngleIter);

                    logFile << "    Fragments will be connected" << endl;
                    logFile << "        A: " << a << "\n    B: " << b << endl;
                    logFile << "        A: " << aEPI180diff << aLeft << aEPILeft << endl;
                    logFile << "        B: " << bEPI180diff << bLeft << bEPILeft << endl;



                    if (!aEPI180diff)
                    {
                        if (!aLeft)
                        {
                            logFile << "        !aLeft" << endl;
                            //modify first item with 180 diff: add 180 to whole right part
                            //then add 180 to the right part of the fragment
                            //update dAlpha with new values of aAngleIterator

                            ModelItem * m = firstItemWith180Diff;
                            QList<ModelItem *> visited;
                            QList<ModelItem *> toVisit;

                            int k = 1;
                            while (m->getEndPoint(k)!=NULL)
                            {
                                if (m->getNeighbour(k)==NULL)
                                {
                                }
                                else
                                {
                                    if (m!=*aEPItemIter)
                                        m->setEndPointAngle(k,180+m->getTurnAngle(k));
                                    if (!toVisit.contains(m->getNeighbour(k)) && m->getNeighbour(k)!=NULL)
                                        toVisit.push_back(m->getNeighbour(k));
                                }

                                k+=2;
                            }
                            visited.push_back(m);


                            while (!toVisit.empty())
                            {
                                m=toVisit.first();
                                toVisit.pop_front();
                                k = 0;
                                while (m->getEndPoint(k)!=NULL && m!=*aEPItemIter)
                                {

                                    if (m->getNeighbour(k)!=NULL)
                                    {
                                        m->setEndPointAngle(k,180+m->getTurnAngle(k));
                                        if(a->getEndPoints()->contains(m->getEndPoint(k)))
                                        {
                                            QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                                            for (int y = 0; y < a->getEndPoints()->indexOf(m->getEndPoint(k)); y++){it++;}
                                            *it+=180;
                                        }

                                        if (!visited.contains(m->getNeighbour(k)) && m->getNeighbour(k)!=NULL
                                            && !toVisit.contains(m->getNeighbour(k)))
                                            toVisit.push_back(m->getNeighbour(k));

                                    }

                                    k++;
                                }
                                if (!visited.contains(m))
                                    visited.push_back(m);

                            }

                            k = 1;
                            while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                            {
                                (*aEPItemIter)->setEndPointAngle(k,+180+(*aEPItemIter)->getTurnAngle(k));
                                if ((*aEPItemIter)->getNeighbour(k)==NULL)
                                        a->setEndPointAngle((*aEPItemIter)->getEndPoint(k),(*aEPItemIter)->getTurnAngle(k));

                                k+=2;
                            }

                             dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                        }

                        if (aLeft)
                        {
                            logFile << "        aLeft" << endl;
                            //find first item with 180 diff, then subtract 180 from whole left part (!=NULL neighbours) of fragment
                            ModelItem * m = firstItemWith180Diff;
                            QList<ModelItem *> visited;
                            QList<ModelItem *> toVisit;

                            int k = 0;
                            while (m->getEndPoint(k)!=NULL)
                            {
                                if (m->getNeighbour(k)!=NULL)
                                {
                                    if (m!=*aEPItemIter)
                                        m->setEndPointAngle(k,-180+m->getTurnAngle(k));
                                    if (!toVisit.contains(m->getNeighbour(k)))
                                        toVisit.push_back(m->getNeighbour(k));
                                }
                                k+=2;
                            }
                            visited.push_back(m);

                            while (!toVisit.empty())
                            {
                                m=toVisit.first();
                                toVisit.pop_front();
                                k = 0;
                                while (m->getEndPoint(k)!=NULL)
                                {
                                    if ((!aEPI180diff &&  aLeft &&  !aEPILeft))
                                    {
                                        if (m->getNeighbour(k)!=NULL)
                                        {
                                            m->setEndPointAngle(k,-180+m->getTurnAngle(k));
                                            if(a->getEndPoints()->contains(m->getEndPoint(k)))
                                            {
                                                QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                                                for (int y = 0; y < a->getEndPoints()->indexOf(m->getEndPoint(k)); y++){it++;}
                                                *it-=180;
                                            }

                                            if (!visited.contains(m->getNeighbour(k)) && m->getNeighbour(k)!=NULL
                                                && !toVisit.contains(m->getNeighbour(k)))
                                                toVisit.push_back(m->getNeighbour(k));
                                        }
                                        else if (m->getNeighbour(k)==NULL && k%2==0)
                                        {
                                            m->setEndPointAngle(k,-360+m->getTurnAngle(k));
                                            if(a->getEndPoints()->contains(m->getEndPoint(k)))
                                            {
                                                QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                                                for (int y = 0; y < a->getEndPoints()->indexOf(m->getEndPoint(k)); y++){it++;}
                                                *it-=360;
                                            }
                                        }

                                    }
                                    else
                                    {
                                        m->setEndPointAngle(k,-180+m->getTurnAngle(k));
                                        if(a->getEndPoints()->contains(m->getEndPoint(k)))
                                        {
                                            QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                                            for (int y = 0; y < a->getEndPoints()->indexOf(m->getEndPoint(k)); y++){it++;}
                                            *it-=180;
                                        }

                                        if (!visited.contains(m->getNeighbour(k)) && m->getNeighbour(k)!=NULL
                                            && !toVisit.contains(m->getNeighbour(k)))
                                            toVisit.push_back(m->getNeighbour(k));
                                    }

                                    k++;
                                }
                                if (!visited.contains(m))
                                    visited.push_back(m);

                            }

                            if ((!aEPI180diff &&  aLeft &&  !aEPILeft))
                            {
                                int k = 0;
                                while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                                {
                                    if ((*aEPItemIter)->getNeighbour(k)==NULL)
                                    {
                                        (*aEPItemIter)->setEndPointAngle(k,-180+(*aEPItemIter)->getTurnAngle(k));
                                        a->setEndPointAngle((*aEPItemIter)->getEndPoint(k),(*aEPItemIter)->getTurnAngle(k));
                                    }


                                    k++;
                                }
                            }
                             dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                        }
                        if ((!aEPI180diff &&  aLeft &&  aEPILeft))
                        {
                            for (int i = 0; i < a->getFragmentItems()->count();i++)
                            {
                                int k = 0;
                                while (a->getFragmentItems()->at(i)->getEndPoint(k)!=NULL)
                                {
                                    a->getFragmentItems()->at(i)->setEndPointAngle(k,-180+a->getFragmentItems()->at(i)->getTurnAngle(k));
                                    k++;
                                }

                            }
                            QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                            for (int y = 0; y < a->getEndPoints()->count(); y++){
                                *it-=180;
                                it++;
                            }
                            dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                        }

                    }
                    else //if (aEPI180diff==true)
                    {
                        if (( aEPI180diff &&  aLeft && !aEPILeft))
                        {
                            for (int i = 0; i < a->getFragmentItems()->count();i++)
                            {
                                int k = 0;
                                while (a->getFragmentItems()->at(i)->getEndPoint(k)!=NULL)
                                {
                                    a->getFragmentItems()->at(i)->setEndPointAngle(k,-180+a->getFragmentItems()->at(i)->getTurnAngle(k));
                                    k++;
                                }

                            }
                            QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                            for (int y = 0; y < a->getEndPoints()->count(); y++){
                                *it-=180;
                                it++;
                            }

                            dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                        }

                        else if (( aEPI180diff &&  aLeft &&  aEPILeft))
                        {
                            int k = 0;
                            while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                            {
                                if ((*aEPItemIter)->getNeighbour(k)==NULL)
                                {
                                    QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                                    for (int y = 0; y < a->getEndPoints()->indexOf((*aEPItemIter)->getEndPoint(k)); y++){it++;}
                                    *it-=180;
                                    (*aEPItemIter)->setEndPointAngle(k,-180+(*aEPItemIter)->getTurnAngle(k));
                                }


                                k+=2;
                            }
                            dAlpha =(*bEPAngleIter)-(*aEPAngleIter);

                        }

                        else if (aLeft && bLeft)
                        {
                            /*Testing hasnt proved it, but there is still a little chance of a bug here:
                             *this might set only angles in one lane sometimes, but testing hasnt showed such behaviour
                             * /
                            logFile << "    Warning: entering block of code which might cause an error" << endl;
                            (*aEPAngleIter)-=180;

                            int k = 0;
                            while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                            {
                                (*aEPItemIter)->setEndPointAngle(k,-180+(*aEPItemIter)->getTurnAngle(k));
                                k+=2;
                            }

                            ModelItem * neigh = (*aEPItemIter)->getNeighbour(aI);
                            ModelItem * neighPrev = (*aEPItemIter);
                            int nIPrev=0;
                            while (neigh!=(*aEPItemIter) && neigh!=NULL)
                            {
                                //find endPoint, the neighbour of which is neighPrev
                                int nI = 0;
                                while(neigh->getEndPoint(nI)!=NULL)
                                {
                                    if (neigh->getNeighbour(nI)==neighPrev)
                                        break;
                                    nI++;
                                }

                                neighPrev=neigh;
                                nIPrev=nI;

                                //modify angles of points
                                neigh->setEndPointAngle(nI,-180+neigh->getTurnAngle(nI));
                                if (nI%2==0)
                                {
                                    neigh->setEndPointAngle(nI+1,-180+neigh->getTurnAngle(nI+1));
                                    neigh=neigh->getNeighbour(nI+1);
                                }
                                else
                                {
                                    neigh->setEndPointAngle(nI-1,-180+neigh->getTurnAngle(nI-1));
                                    neigh=neigh->getNeighbour(nI-1);
                                }
                            }

                            //if neigh==null, then the last neighPrev value is the endPointItem -> adjust also fragment angle
                            if (neigh==NULL && neighPrev!=*aEPItemIter)
                            {
                                int k = 0;
                                while (neighPrev->getEndPoint(k)!=NULL)
                                {
                                    if (neighPrev->getNeighbour(k)==NULL && k%2==nIPrev%2)
                                    {
                                        a->setEndPointAngle(neighPrev->getEndPoint(k),-180+a->getEndPointsAngles()->at(a->getEndPoints()->indexOf(neighPrev->getEndPoint(k))));

                                    }
                                    k++;
                                }
                            }
                            dAlpha =(*bEPAngleIter)-(*aEPAngleIter);

                        }
                        //repaired - dont do anything with this:
                        else if (!aLeft)
                        {
                            logFile << "    !aLeft && aEPI180Diff" << endl;

                            //modify first item with 180 diff: add 180 to whole right part
                            //then add 180 to the right part of the fragment
                            //update dAlpha with new values of aAngleIterator

                            ModelItem * m = firstItemWith180Diff;
                            QList<ModelItem *> visited;
                            QList<ModelItem *> toVisit;

                            int k = 1;
                            while (m->getEndPoint(k)!=NULL)
                            {
                                if (m->getNeighbour(k)==NULL)
                                {
                                }
                                else
                                {
                                    if (m!=*aEPItemIter)
                                        m->setEndPointAngle(k,180+m->getTurnAngle(k));
                                    if (!toVisit.contains(m->getNeighbour(k)) && m->getNeighbour(k)!=NULL)
                                        toVisit.push_back(m->getNeighbour(k));
                                }

                                k+=2;
                            }
                            visited.push_back(m);


                            while (!toVisit.empty())
                            {
                                m=toVisit.first();
                                toVisit.pop_front();
                                k = 0;
                                while (m->getEndPoint(k)!=NULL && m!=*aEPItemIter)
                                {

                                    if (m->getNeighbour(k)!=NULL)
                                    {
                                        m->setEndPointAngle(k,180+m->getTurnAngle(k));
                                        if(a->getEndPoints()->contains(m->getEndPoint(k)))
                                        {
                                            QList<qreal>::Iterator it = a->getEndPointsAngles()->begin();
                                            for (int y = 0; y < a->getEndPoints()->indexOf(m->getEndPoint(k)); y++){it++;}
                                            *it+=180;
                                        }

                                        if (!visited.contains(m->getNeighbour(k)) && m->getNeighbour(k)!=NULL
                                            && !toVisit.contains(m->getNeighbour(k)))
                                            toVisit.push_back(m->getNeighbour(k));

                                    }

                                    k++;
                                }
                                if (!visited.contains(m))
                                    visited.push_back(m);

                            }

                            k = 1;
                            while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                            {
                                        (*aEPItemIter)->setEndPointAngle(k,+180+(*aEPItemIter)->getTurnAngle(k));
                                if ((*aEPItemIter)->getNeighbour(k)==NULL)
                                        a->setEndPointAngle((*aEPItemIter)->getEndPoint(k),(*aEPItemIter)->getTurnAngle(k));

                                k+=2;
                            }

                            dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                        }

                    }


                    logFile << "    A: fragment will be rotated by the angle of " << dAlpha << endl;
                    a->rotate(dAlpha,*bPointIter);

                    //moveBy(dX,dY) between points
                    qreal dX = (*bPointIter)->x()-(*aPointIter)->x();
                    qreal dY = (*bPointIter)->y()-(*aPointIter)->y();;
                    a->moveBy(dX,dY);


                    //modify neighbour of "a" endItem at this point
                    (*aEPItemIter)->setNeighbour(*bEPItemIter,*bPointIter);

                    //modify Neighbour of (*fragIter) endItem at this point
                    (*bEPItemIter)->setNeighbour(*aEPItemIter,*bPointIter);


                    logFile << "    Connecting fragments using the recursion" << endl;
                    recursivelyAdd(*aEPItemIter,b,*aPointIter);

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


                    if ((!aEPI180diff  && aLeft && !aEPILeft))
                    {
                        int k = 1;
                        while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                        {
                            if ((*aEPItemIter)->getNeighbour(k)==NULL)
                            {
                                (*aEPItemIter)->setEndPointAngle(k,+180+(*aEPItemIter)->getTurnAngle(k));
                                b->setEndPointAngle((*aEPItemIter)->getEndPoint(k),(*aEPItemIter)->getTurnAngle(k));
                            }
                            k+=2;
                        }
                    }
                    else if ((!aEPI180diff && !aLeft && !aEPILeft) )
                    {
                        int k = 0;
                        while (firstItemWith180Diff->getEndPoint(k)!=NULL)
                        {

                            if (firstItemWith180Diff->getNeighbour(k)==NULL && firstItemWith180Diff->getNeighbour(k+1)!=NULL)
                            {
                                firstItemWith180Diff->setEndPointAngle(k,-180+firstItemWith180Diff->getTurnAngle(k));
                                QList<qreal>::Iterator it = b->getEndPointsAngles()->begin();
                                for (int y = 0; y < b->getEndPoints()->indexOf(firstItemWith180Diff->getEndPoint(k)); y++){it++;}
                                *it-=180;
                            }
                            k+=2;
                        }

                    }
                    else if (( aEPI180diff &&  aLeft && !aEPILeft &&  bEPI180diff &&  bLeft &&  bEPILeft))
                    {
                        int k = 0;
                        while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                        {
                            if (k%2==0)
                            {
                                if ((*aEPItemIter)->getNeighbour(k)==NULL)
                                {
                                    (*aEPItemIter)->setEndPointAngle(k,-360+(*aEPItemIter)->getTurnAngle(k));
                                    QList<qreal>::Iterator it = b->getEndPointsAngles()->begin();
                                    for (int y = 0; y < b->getEndPoints()->indexOf((*aEPItemIter)->getEndPoint(k)); y++){it++;}
                                    *it-=360;
                                }
                            }
                            k++;
                        }
                    }
                    else if ((!aEPI180diff &&  aLeft &&  aEPILeft)
                          || ( aEPI180diff &&  aLeft && !aEPILeft))
                    {
                        /** WARNING: 180-angle-bug may appear:
                         * there is RR sequence connected in the strange way - second item is connected only in one point
                         * connect RR seq. with its right side to one straight part
                         * connect this fragment to anything
                         * "backwards" neighbour of one turn has wrong angle, because it's got reverse orientation than the one
                         * which is expected by this part of algorithm
                         *
                         * QUESTION: may this situation appear in any different way?

                        * /
                        for (int i = 0; i < a->getFragmentItems()->count();i++)
                        {
                            int k = 0;
                            while (a->getFragmentItems()->at(i)->getEndPoint(k)!=NULL)
                            {
                                if (k%2==0)
                                {
                                    if (a->getFragmentItems()->at(i)->getNeighbour(k)==NULL)
                                    {
                                       a->getFragmentItems()->at(i)->setEndPointAngle(k,-180+a->getFragmentItems()->at(i)->getTurnAngle(k));
                                       b->setEndPointAngle(a->getFragmentItems()->at(i)->getEndPoint(k),a->getFragmentItems()->at(i)->getTurnAngle(k));
                                    }
                                }
                                else
                                {
                                    if (a->getFragmentItems()->at(i)->getNeighbour(k)==NULL)
                                    {
                                       a->getFragmentItems()->at(i)->setEndPointAngle(k,+180+a->getFragmentItems()->at(i)->getTurnAngle(k));
                                       b->setEndPointAngle(a->getFragmentItems()->at(i)->getEndPoint(k),a->getFragmentItems()->at(i)->getTurnAngle(k));
                                    }
                                }

                                k++;
                            }

                        }

                    }
                    else if ( aEPI180diff &&  aLeft &&  aEPILeft)
                    {}
                    else if (( aEPI180diff && !aLeft && !aEPILeft))
                    {
                        int k = 0;
                        while ((*aEPItemIter)->getEndPoint(k)!=NULL)
                        {
                                if ((*aEPItemIter)->getNeighbour(k)==NULL)
                                {
                                    (*aEPItemIter)->setEndPointAngle(k,-180+(*aEPItemIter)->getTurnAngle(k));
                                    QList<qreal>::Iterator it = b->getEndPointsAngles()->begin();
                                    for (int y = 0; y < b->getEndPoints()->indexOf((*aEPItemIter)->getEndPoint(k)); y++){it++;}
                                    *it-=180;
                                }
                            k++;
                        }
                    }


                    //addFragmentItem() changes left angles of firstItemWith180Diff if there is no neighbour -> they need to be set back
                    //because R-R connection will cause the mirror flip of the fragment A
                    //if (!aLeft && !bLeft && !aEPI180diff && !bEPI180diff)
                    else if ((!aLeft && !aEPI180diff)
                             )
                    {
                        int k = 0;
                        while (firstItemWith180Diff->getEndPoint(k)!=NULL)
                        {
                            if (firstItemWith180Diff->getNeighbour(k)==NULL &&
                                firstItemWith180Diff->getNeighbour(k+1)!=NULL)
                            {
                                /**
                                  NOTE:
                                  -why is this done: addFragmentItem modifies left side angles with no neighbours
                                                     but all parts with clockwise direction won't have 180 diff.
                                * /

                                firstItemWith180Diff->setEndPointAngle(k,-180+firstItemWith180Diff->getTurnAngle(k));
                                QList<qreal>::Iterator it = b->getEndPointsAngles()->begin();
                                for (int y = 0; y < b->getEndPoints()->indexOf(firstItemWith180Diff->getEndPoint(k)); y++){it++;}
                                *it-=180;
                            }
                            k+=2;
                        }
                    }


                    firstFound=false;
                    bFirstFound=b;
                    bFirstFoundCountOfItems=bCountBefore;

                }

                do
                {
                    bEPItemIter++;
                    bPointIter++;
                    bEPAngleIter++;
                    bIndex++;
                } while (*bPointIter==NULL);


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

    if (!firstFound)
    {

        a->getFragmentItems()->clear();
        this->modelFragments->removeOne(a);
        delete (a);
        a = NULL;

        logFile << "    Printing all angles after fragments connection:" << endl;
        ModelItem * m = bFirstFound->getEndPointsItems()->first();
        for (int j = 0; j < bFirstFound->getFragmentItems()->count();j++)
        {
            m = bFirstFound->getFragmentItems()->at(j);
            logFile << "        Item: " << m << endl;
            int i = 0;
            while (m->getEndPoint(i)!=NULL)
            {
                logFile << "            [" << i << "]: " << m->getTurnAngle(i) << " neighbour is " << m->getNeighbour(i) << endl;
                i++;
            }


        }


        this->connectFragments(bFirstFound);
        return 0;
    }

    return 0;
}
*/
/**
int WorkspaceWidget::connectFragments(ModelFragment * a, ModelFragment * b, QPointF * aP, QPointF * bP)
{
    / **
      TODO
    * /
    return 0;
}

int WorkspaceWidget::connectFragments(int index1, int index2, QPointF * aP, QPointF * bP)
{
    / **
      TODO
    * /
    return 0;
}*/

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
        if (frag->getFragmentItems()->at(i)->get2DModelNoText()->scene()!=NULL)
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

ModelItem *WorkspaceWidget::findItemByApproxPos(QPointF *point, ModelItem * dontWantThisItem)
{
    ModelItem * pointer = NULL;
    QRectF rect(point->x()-SEARCH_TOLERANCE_HALF,point->y()-SEARCH_TOLERANCE_HALF,2*SEARCH_TOLERANCE_HALF,2*SEARCH_TOLERANCE_HALF);
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
        item->updateEndPointsHeightGraphics();
}

QPointF *WorkspaceWidget::getActiveEndPoint() const
{
    return this->activeEndPoint;
}

int WorkspaceWidget::setActiveEndPoint(QPointF *pt)
{
    if (pt==NULL)
        return 1;
    if (this->activeEndPoint!=NULL)
        delete this->activeEndPoint;
    this->activeEndPoint=pt;

    if (this->activeEndPointGraphic!=NULL)
    {
        this->graphicsScene->removeItem(this->activeEndPointGraphic);
        this->activeEndPointGraphic=NULL;
    }
    QPainterPath * pp = new QPainterPath();
    pp->addEllipse(0,0,1,1);

    QGraphicsPathItem * qgpi = new QGraphicsPathItem(*pp);
    QPen p = qgpi->pen();
    p.setWidth(8);

    qgpi->setPen(p);
    qgpi->moveBy(pt->x(),pt->y());
    qgpi->setZValue(-10);
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

bool WorkspaceWidget::getHeightProfileMode()
{
    return this->heightProfileMode;
}

bool WorkspaceWidget::getDeletePress() const
{
    return this->deletePress;
}

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
        /*if (this->activeEndPoint!=NULL)
        {
            this->graphicsScene->removeItem(this->activeEndPointGraphic);
            delete this->activeEndPoint;
        }
        this->activeEndPointGraphic=NULL;
        this->activeEndPoint=NULL;*/
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
                if (list.at(i)->toolTip()=="Decrease height")
                {
                    if (this->activeItem->getSlotTrackInfo()==NULL)
                        this->activeItem->adjustHeightProfile(-1,this->activeEndPoint);
                    else
                    {
                        for (int j = 0; this->activeItem->getEndPoint(j)!=NULL; j++)
                            this->activeItem->adjustHeightProfile(-1,this->activeItem->getEndPoint(j));
                    }
                }
                else
                {
                    if (this->activeItem->getSlotTrackInfo()==NULL)
                        this->activeItem->adjustHeightProfile(1,this->activeEndPoint);
                    else
                    {
                        for (int j = 0; this->activeItem->getEndPoint(j)!=NULL; j++)
                            this->activeItem->adjustHeightProfile(1,this->activeItem->getEndPoint(j));
                    }

                }
            }
        }
        //if (list.at(list.indexOf(this->sender()))->toolTip()=="Decrease height")

        //else

    }
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

