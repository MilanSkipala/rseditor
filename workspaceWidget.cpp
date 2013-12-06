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

    /**
     *TODO
     *known bug - only one neighbour is set
    */
    ModelFragment * b = NULL;

    QList<ModelFragment*>::Iterator fragIter = this->modelFragments->begin();
    QList<QPointF*>::Iterator aPointIter = a->getEndPoints()->begin();
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

    QList<int> bDoNotAddIndices;
    QList<int> aDoNotAddIndices;

    /**
      TODO:
      -check correctness of T1-T10 rail parts
    */
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

        qreal modifyDAlpha = 0;


        //for each fragment "fragIter" in workspace:
        while(fragIter!=this->modelFragments->end())
        {


            QList<QPointF*>::Iterator bPointIter = (*fragIter)->getEndPoints()->begin();
            QList<ModelItem*>::Iterator bEPItemIter = (*fragIter)->getEndPointsItems()->begin();
            QList<qreal>::Iterator bEPAngleIter = (*fragIter)->getEndPointsAngles()->begin();
            int bIndex = 0;
            b = (*fragIter);
            int bCountBefore = b->getEndPoints()->count();

            //continue if iterator equals to method argument - a cannot be connected with itself
            if (a==b)
            {
                fragIter++;
                continue;
            }



            //for each point of "fragIter":

            //check only points which have been in b fragment before merging with a
            while (bIndex<bCountBefore)//(*bPointIter!=bPointIterLast)
            {

                //find points at which two fragments can be connected:
                if (false && area.contains(**bPointIter) && a!=(*fragIter))///TODO: && tolerance of angle?????? Probably not, because it should be rotated/moved accurately and then the tolerances
                    ///in whole new fragment should be checked
                {
                    logFile << "Fragment A: " << a << " and fragment B: " << b << " will be connected" << endl;

                    //previous (not working) versions are in backup from 27-11-2013

                    /* NOTE
                    //fragments A and B can be connected in four possible ways:
                    //1)aEPI's left side && bEPI's left side
                    //2)aEPI's left side && bEPI's right side
                    //3)aEPI's right side && bEPI's left side
                    //4)aEPI's right side && bEPI's right side

                    //additionally - if aEPI has 180 difference the angle of aEPAngleIter has to be modified by -+180 and
                    //                  angle of that endPoint (item.setEPA()) and angles of all 'neighbourhood' have to be modified too

                    //1) and 2) -> a's left side -=180
                    //3) and 4) -> a's right side +=180
*/

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

                    logFile << "        A will be connected with its left side: " << (aLeft ? "true" : "false") << " aEPILeft: " << (aEPILeft ? "true" : "false") << endl;
                    logFile << "        A has 180 deg. difference: " << aEPI180diff << endl;
                    logFile << "        B will be connected with its left side: " << (bEPILeft ? "true" : "false") << endl;
                    logFile << "        B has 180 deg. difference: " << aEPI180diff << endl;


                    ///THIS IS THE RIGHT PLACE FOR:
                    //////DO THE FOLLOWING CODE SOMEWHERE ABOVE SO THAT IT IS EXECUTED AND CODED JUST ONCE
                    //find some "neighbour" (it doesn't have to be connected directly to this item) which satisfies:
                    //if (n.n(0)="this") && indexOfConnection%2==0)
                    // || if (n.180diff==true)
                    //{}
                    ///


                    //modify angles by +-180 if needed
                    if (aEPILeft && aEPI180diff)//(aEPILeft && aEPI180diff)
                    {
                        logFile << "    A: all left side angles in one lane are being changed by -180" << endl;
                        (*aEPAngleIter)-=180;
                        (*aEPItemIter)->setEndPointAngle(aI,-180+(*aEPItemIter)->getTurnAngle(aI));
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
                        /**
                          TODO:
                          -adjust aEPAngleIter
                        */
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

                    }
                    else if (!aEPILeft && aEPI180diff)
                    {

                       logFile << "    A: all right side angles in one lane are being changed by +180" << endl;

                        (*aEPAngleIter)+=180;
                        (*aEPItemIter)->setEndPointAngle(aI,+180+(*aEPItemIter)->getTurnAngle(aI));
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
                            neigh->setEndPointAngle(nI,+180+neigh->getTurnAngle(nI));
                            if (nI%2==0)
                            {
                                neigh->setEndPointAngle(nI+1,+180+neigh->getTurnAngle(nI+1));
                                neigh=neigh->getNeighbour(nI+1);
                            }
                            else
                            {
                                neigh->setEndPointAngle(nI-1,+180+neigh->getTurnAngle(nI-1));
                                neigh=neigh->getNeighbour(nI-1);
                            }

                        }
                        //if neigh==null, then the last neighPrev value is the endPointItem -> adjust also fragment angle
                        /**
                          TODO:
                          -adjust aEPAngleIter
                        */
                        if (neigh==NULL && neighPrev!=*aEPItemIter)
                        {
                            int k = 0;
                            while (neighPrev->getEndPoint(k)!=NULL)
                            {
                                if (neighPrev->getNeighbour(k)==NULL && k%2==nIPrev%2)
                                    a->setEndPointAngle(neighPrev->getEndPoint(k),+180+a->getEndPointsAngles()->at(a->getEndPoints()->indexOf(neighPrev->getEndPoint(k))));
                                k++;
                            }
                        }

                        //logFile << "        A: all right side angles are being unchanged" << endl;
                    }

                    /**
                     * TODO:
                     * -pokud "A" pripojuji pravou stranou dilku (bez rozdilu 180), ale tento dilek je pripojeny (i pres sousedy) k leve strane
                     *  dilku s rozdilem 180, otoc fragment o +180
                     *
                     * -pokud "A" pripojuji levou stranou dilku (bez rozdilu 180), jenz je pripojeny k leve strane
                     *  dilku s rozdilem 180, otoc fragment o -180
                     *
                     *
                     *!!!! pred implem. overeni spravnosti prikladem na papir !!!!!
                     *
                     */

                    /*part 1:
                     if (!aEPILeft && !aEPI180diff)
                     {
                     try to find some neighbour which satisfies:
                     if (n.leftRight... && indexOfConnection%2==0)
                         a->rotate(+180,*aPointIter);
                     }
*/
                    /*part 2:
                     if (aEPILeft && !aEPI180diff)
                     try to find some neighbour which satisfies:
                     if (n.leftRight... && indexOfConnection%2==0)
                         a->rotate(-180,*aPointIter);
*/

                    if (aEPILeft && !aEPI180diff)
                    {
                        ///DO THE FOLLOWING CODE SOMEWHERE ABOVE SO THAT IT IS EXECUTED AND CODED JUST ONCE
                        //find some "neighbour" (it doesn't have to be connected directly to this item) which satisfies:
                        //if (n.n(0)="this") && indexOfConnection%2==0)
                        //{}
                        ///
                        //if some item was found, rotate a by 180
                        ///a->rotate(-180,*aPointIter);
                        cout << "Im back" << endl;
                    }
                    else if (!aEPILeft && !aEPI180diff && firstFound)
                    {

                        ///DO THE FOLLOWING CODE SOMEWHERE ABOVE SO THAT IT IS EXECUTED AND CODED JUST ONCE
                        //find some "neighbour" (it doesn't have to be connected directly to this item) which satisfies:
                        //if (n.leftRightDifference180(a,b) && indexOfConnection%2==0)
                        //{n.n(0)="this"}
                        ///
                        //if some item was found, rotate a by 180

                        modifyDAlpha = 180;

                        QList<ModelItem*> visited;
                        QList<ModelItem*> queueToVisit;
                        ModelItem * mi = *aEPItemIter;
                        ModelItem * miPrev = NULL;

                        int k = 0;
                        while (mi->getEndPoint(k)!=NULL)
                        {
                            if (mi->getNeighbour(k)!=NULL)
                            {
                                queueToVisit.push_back(mi->getNeighbour(k));
                            }
                            k+=2;
                        }
                        miPrev = mi;
                        visited.push_back(mi);

                        while (!queueToVisit.empty())
                        {
                            mi = queueToVisit.first();
                            queueToVisit.pop_front();
                            k = 0;
                            while (mi->getEndPoint(k)!=NULL)
                            {
                                //connecting with the right side of item but with the left side of fragment
                                if ((mi->leftRightDifference180(k,k+1) && mi->getNeighbour(k)==miPrev))
                                {
                        ///TODO:
                        ///-BUG?: mi.gn(k)==miPrev probably doesnt return true for all posible true states
                                    queueToVisit.clear();
                                    modifyDAlpha = 180;
                                    //for all visited parts (=parts between A's left end and first part with 180 diff.
                                    //modify all angles by modifyDAlpha
                                    for (int j = 0; j < visited.count();j++)
                                    {
                                        int p = 0;
                                        while (visited.at(j)->getEndPoint(p)!=NULL)
                                        {
                                            visited.at(j)->setEndPointAngle(p,visited.at(j)->getTurnAngle(p)-modifyDAlpha);
                                            p++;
                                        }
                                    }
                                    //+modify by 180 all left-side neighbours at points of connection
                                    //with the last visited item
                                    int p = 0;
                                    while(visited.last()->getEndPoint(p)!=NULL)
                                    {
                                        visited.last()->getNeighbour(p)->setEndPointAngle(visited.last()->getEndPoint(p)
                                                                                          ,visited.last()->getNeighbour(p)->getTurnAngle(visited.last()->getEndPoint(p))-modifyDAlpha);
                                        p+=2;
                                    }
                                    break;
                                }
                                else if (mi->leftRightDifference180(k,k+1))
                                {
///chyba: sem se dostane bez ohledu na to, jestli pripojuji levou stranu fragmentu, nebo pravou!!
///reseni:
///jak poznam, ze jsem narazil na levou stranu fragmentu?
///     narazil jsem na dilek s rozdilem 180deg. && predchozi dilek je na leve strane aktualniho
///
///chyba: sem se dostane ve vsech ostatnich pripadech, kdy !(pripojuji levou stranu && 180)
                                    cout << "Im here" << endl;

                                }

                                if (mi->getNeighbour(k)!=NULL)
                                {


                                    if (!visited.contains(mi->getNeighbour(k)))
                                        queueToVisit.push_back(mi->getNeighbour(k));
                                }

                                k+=2;
                                //k++;
                            }
                            if (!visited.contains(mi))
                                visited.push_back(mi);
                            miPrev = mi;
                        }




                    }
/**
                    watch out - part can be connected by its right side and is connected on the leftside of fragment -> it may cause problems

                    ??does it mean, that rightside && leftside of fragment/some other item => rightside=!rightside??
*/

                    if (firstFound)
                    {


                        //PROBLEM: B<A -> angle is less than zero which is ok
                        //BUT there can be a situation, where you get following angles after rotation
                        //A: 157
                        //B: -157





                        qreal dAlpha =(*bEPAngleIter)-(*aEPAngleIter)+modifyDAlpha;

                        logFile << "    A: fragment will be rotated by the angle of " << dAlpha << endl;

                        a->rotate(dAlpha,*bPointIter);

                        /*
                        for (int i = 0; i < a->getFragmentItems()->count(); i++)
                        {
                            int xyz = 0;
                            while (a->getFragmentItems()->at(i)->getEndPoint(xyz)!=NULL)
                            {
                                a->getFragmentItems()->at(i)->setEndPointAngle(xyz,a->getFragmentItems()->at(i)->getTurnDegree(xyz)-modifyDAlpha);
                                xyz++;
                            }
                        }
                        for (int i = 0; i < a->getEndPointsAngles()->count();i++)
                        {
                            a->setEndPointAngle(i,a->getEndPointsAngles()->at(i)-modifyDAlpha);
                        }*/




                        //moveBy(dX,dY) between points
                        qreal dX = (*bPointIter)->x()-(*aPointIter)->x();
                        qreal dY = (*bPointIter)->y()-(*aPointIter)->y();;
                        a->moveBy(dX,dY);


                        //set parentFragment of all b's items
                        QList<ModelItem*>::Iterator parFragIter = a->getFragmentItems()->begin();
                        while(parFragIter!=a->getFragmentItems()->end())
                        {
                            (*parFragIter)->setParentFragment(b);
                            parFragIter++;
                        }

                        firstFound=false;
                        bFirstFound=b;

///TODO: modify this part of code
                        //if (aIndex>1)
                        if (aI>1)
                        {
                            fragIter = this->modelFragments->begin();
                            aPointIter = a->getEndPoints()->begin();
                            aEPAngleIter = a->getEndPointsAngles()->begin();
                            aEPItemIter = a->getEndPointsItems()->begin();
                            aEPGraphIter = a->getEndPointsGraphics()->begin();
                            aPointIter--;
                            aEPAngleIter--;
                            aEPItemIter--;
                            aEPGraphIter--;

                            aIndex = -1;
                            break;
                        }


                    }

///
                    //modify neighbour of "a" endItem at this point
                    (*aEPItemIter)->setNeighbour(*bEPItemIter,*bPointIter);

                    //modify Neighbour of (*fragIter) endItem at this point
                    (*bEPItemIter)->setNeighbour(*aEPItemIter,*bPointIter);

                    aDoNotAddIndices.push_back(aIndex);
                    bDoNotAddIndices.push_back(bIndex);



/*
                    for (int i = 0; i < a->getEndPoints()->count(); i++)
                    {
                        //V3
                        //check whether the aPoint may be added or not - if it is almost equal to bPointIter do NOT add it and also remove bPointIter later in this method
                        //if (!area.contains(**bPointIter))
                        QRectF bArea((*bPointIter)->x()-TOLERANCE_HALF,(*bPointIter)->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);
                        if (!bArea.contains(*a->getEndPoints()->at(i)))
                        {
                            b->getEndPoints()->append(a->getEndPoints()->at(i));
                            //do something with this value and also with angle of item
                            //do what? compute dA between aEPI angles - if aEPI has L-R difference of 180 modify its angles
                            qreal dA = 0;
                            b->getEndPointsAngles()->append(a->getEndPointsAngles()->at(i));
                            b->getEndPointsGraphics()->append(a->getEndPointsGraphics()->at(i));
                            b->getEndPointsItems()->append(a->getEndPointsItems()->at(i));
                        }


                    }*/



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

                    /**
                      TODO:
                      check correctness of behaviour in cases when aEPILeft=true
*/

                    ModelItem * firstItemWith180Diff = NULL;
                    qreal dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                    if (!aEPI180diff)
                    {
                        logFile << "    EndPointItem of A doesn't have 180deg. difference" << endl;

                        ModelItem * uselessPointer = NULL;
                        //get info about l-r side of A
                        aLeft = a->leftSide(*aEPItemIter, firstItemWith180Diff);
                        //get info about l-r side of B
                        bLeft = b->leftSide(*bEPItemIter, uselessPointer);


                        if (!aLeft)// && !bLeft)
                        {
                            logFile << "        !aLeft" << endl;
                            //modify first item with 180 diff: add 180 to whole right part (!=NULL neighbours) of fragment
                            //                                 add 360 to points with NULL neighbours
                            //then add 180 to whole fragment
                            //update dAlpha with new values of aAngleIterator

                            ModelItem * m = firstItemWith180Diff;
                            QList<ModelItem *> visited;
                            QList<ModelItem *> toVisit;


                            int k = 1;
                            while (m->getEndPoint(k)!=NULL)
                            {
                                if (k%2==1)
                                {
                                    if (m->getNeighbour(k)==NULL)
                                    {
                                        m->setEndPointAngle(k,360+m->getTurnAngle(k));
                                    }
                                    else
                                    {
                                        m->setEndPointAngle(k,180+m->getTurnAngle(k));
                                        if (!toVisit.contains(m->getNeighbour(k)))
                                            toVisit.push_back(m->getNeighbour(k));
                                    }
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
                                    //if (m->getNeighbour(k)!=NULL)
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
                             dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                        }

                        if (aLeft)//&& !bLeft)
                        {
                            logFile << "        aLeft" << endl;
                            //find first item with 180 diff, then subtract 180 from whole left part (!=NULL neighbours) of fragment
                            ModelItem * m = firstItemWith180Diff;
                            QList<ModelItem *> visited;
                            QList<ModelItem *> toVisit;


                            int k = 0;
                            while (m->getEndPoint(k)!=NULL)
                            {/*
                                    if (m->getNeighbour(k)==NULL)
                                    {
                                        m->setEndPointAngle(k,360+m->getTurnAngle(k));
                                    }
                                    else*/ if (m->getNeighbour(k)!=NULL)
                                    {
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
                                    //if (m->getNeighbour(k)!=NULL)
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
                             dAlpha =(*bEPAngleIter)-(*aEPAngleIter);
                        }
                       /* if (!aLeft && bLeft)
                        {
                            logFile << "        !aLeft && bLeft" << endl;

                        }
                        if (aLeft && bLeft)
                        {
                            dAlpha += 180;
                        }*/
                    }
                    else
                    {
                        if (aLeft)//(aEPILeft && aEPI180diff)
                        {
                            logFile << "    A: all left side angles in one lane are being changed by -180" << endl;
                            (*aEPAngleIter)-=180;
                            (*aEPItemIter)->setEndPointAngle(aI,-180+(*aEPItemIter)->getTurnAngle(aI));
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

                        }
                        else if (!aLeft)
                        {

                           logFile << "    A: all right side angles in one lane are being changed by +180" << endl;

                            (*aEPAngleIter)+=180;
                            (*aEPItemIter)->setEndPointAngle(aI,+180+(*aEPItemIter)->getTurnAngle(aI));
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
                                neigh->setEndPointAngle(nI,+180+neigh->getTurnAngle(nI));
                                if (nI%2==0)
                                {
                                    neigh->setEndPointAngle(nI+1,+180+neigh->getTurnAngle(nI+1));
                                    neigh=neigh->getNeighbour(nI+1);
                                }
                                else
                                {
                                    neigh->setEndPointAngle(nI-1,+180+neigh->getTurnAngle(nI-1));
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
                                        a->setEndPointAngle(neighPrev->getEndPoint(k),+180+a->getEndPointsAngles()->at(a->getEndPoints()->indexOf(neighPrev->getEndPoint(k))));
                                    k++;
                                }
                            }
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

                    logFile << "connecting via recursion" << endl;
                    recursivelyAdd(*aEPItemIter,b,*aPointIter);

                    //addFragmentItem() changes left angles of firstItemWith180Diff if there is no neighbour -> they need to be set back
                    //because R-R connection will cause the mirror flip of the fragment A
                    if (!aLeft && !bLeft && !aEPI180diff && !bEPI180diff)
                    {
/**
  TODO
  -known bug:
        -[0->3] null neighbours are ok but if there are [4->7] null neighbours, the subtraction is not needed
  -solution:
        -debug addFragmentItem() step by step to see why the angles are being changed
*/
                        int k = 2;
                        while (firstItemWith180Diff->getEndPoint(k)!=NULL)
                        {
                            if (firstItemWith180Diff->getNeighbour(k)==NULL)
                            {
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

                }

                /*
                if (!bArea.contains(**aPointIter) && a!=(*fragIter))
                {
                    b->getEndPoints()->append(*aPointIter);
                    //do something with this value and also with angle of item
                    //do what? compute dA between aEPI angles - if aEPI has L-R difference of 180 modify its angles
                    qreal dA = 0;
                    b->getEndPointsAngles()->append(*aEPAngleIter);
                    b->getEndPointsGraphics()->append(*aEPGraphIter);
                    b->getEndPointsItems()->append(*aEPItemIter);
                }*/

                if (*bPointIter!=NULL)
                {
                    bEPItemIter++;
                    bPointIter++;
                    bEPAngleIter++;
                    bIndex++;
                }
                else
                {
                    //bPointIter is null because it has been deleted in recursivelyAdd()
                    //if it happens, start the loop again
                    bPointIter = (*fragIter)->getEndPoints()->begin();
                    bEPItemIter = (*fragIter)->getEndPointsItems()->begin();
                    bEPAngleIter = (*fragIter)->getEndPointsAngles()->begin();
                    bIndex = 0;

                }

            }
/*V1.5
            if (!firstFound)
            {
                a->getFragmentItems()->clear();
                a->getEndPointsGraphics()->clear();
                this->modelFragments->removeOne(a);
                delete (a);
                firstFound=true;
            }*/


            if (aIndex==-1)
                break;

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

/*
        logFile << "    Printing pointers after incrementing aIndex=="<<aIndex<< endl;

        logFile << "        A.EP " << a->getEndPoints() << " B.EP " << b->getEndPoints() << endl;
        logFile << "        A.EPA " << a->getEndPointsAngles() << " B.EPA " << b->getEndPointsAngles() << endl;
        logFile << "        A.EPG " << a->getEndPointsGraphics() << " B.EPG " << b->getEndPointsGraphics() << endl;
        logFile << "        A.EPI " << a->getEndPointsItems() << " B.EPI " << b->getEndPointsItems() << endl;
        logFile << "        A.FI " << a->getFragmentItems() << " B.FI " << b->getFragmentItems() << endl;
*/

    }


    ///V2
    if (!firstFound)
    {
  /*
        qSort(bDoNotAddIndices);
        //for (int i  = 0; i < bDoNotAddIndices.count();i++)
        for (int i = bDoNotAddIndices.count()-1; i >= 0; i--)
        {

            bFirstFound->getEndPoints()->removeAt(bDoNotAddIndices.at(i));
            bFirstFound->getEndPointsAngles()->removeAt(bDoNotAddIndices.at(i));
            this->graphicsScene->removeItem(bFirstFound->getEndPointsGraphics()->at(bDoNotAddIndices.at(i)));
            bFirstFound->getEndPointsGraphics()->removeAt(bDoNotAddIndices.at(i));
            bFirstFound->getEndPointsItems()->removeAt(bDoNotAddIndices.at(i));

        }


        for (int i = 0; i < a->getEndPoints()->count(); i++)
        {
            if (!aDoNotAddIndices.contains(i))
            {
                bFirstFound->getEndPoints()->append(a->getEndPoints()->at(i));

                //do something with this value and also with angle of item
                //do what? compute dA between aEPI angles - if aEPI has L-R difference of 180 modify its angles
                qreal dA = 0;
                bFirstFound->getEndPointsAngles()->append(a->getEndPointsAngles()->at(i));
                bFirstFound->getEndPointsGraphics()->append(a->getEndPointsGraphics()->at(i));
                bFirstFound->getEndPointsItems()->append(a->getEndPointsItems()->at(i));
            }
            else
                this->graphicsScene->removeItem(a->getEndPointsGraphics()->at(i));
        }


        bFirstFound->getFragmentItems()->append(*a->getFragmentItems());
        bFirstFound->getProductLines()->append(*a->getProductLines());

*/
        a->getFragmentItems()->clear();
        this->modelFragments->removeOne(a);
        delete (a);
        a = NULL;

        logFile << "Printing all angles after fragments connection:" << endl;
        ModelItem * m = bFirstFound->getEndPointsItems()->first();
        for (int j = 0; j < bFirstFound->getFragmentItems()->count();j++)
        {
            m = bFirstFound->getFragmentItems()->at(j);
            logFile << "    Item: " << m << endl;
            int i = 0;
            while (m->getEndPoint(i)!=NULL)
            {
                logFile << "        [" << i << "]: " << m->getTurnAngle(i) << " neighbour is " << m->getNeighbour(i) << endl;
                i++;
            }


        }


        this->connectFragments(bFirstFound);
        return 0;
    }

    return 0;
}
/**
int WorkspaceWidget::connectFragments(ModelFragment * a, ModelFragment * b, QPointF * aP, QPointF * bP)
{
    /**
      TODO
    * /
    return 0;
}

int WorkspaceWidget::connectFragments(int index1, int index2, QPointF * aP, QPointF * bP)
{
    /**
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

ModelItem *WorkspaceWidget::findItemByApproxPos(QPointF *point)
{
    ModelItem * pointer = NULL;
    QRectF rect(point->x()-5,point->y()-5,10,10);
    QList<ModelFragment*>::Iterator fragIter = this->modelFragments->begin();
    while (fragIter!=this->modelFragments->end() && pointer==NULL)
    {
        QList<ModelItem*>::Iterator itemIter = (*fragIter)->getFragmentItems()->begin();
        while (itemIter!=(*fragIter)->getFragmentItems()->end()  && pointer==NULL)
        {
            int index = 0;
            while ((*itemIter)->getEndPoint(index)!=NULL && pointer==NULL)
            {
                if (rect.contains(*(*itemIter)->getEndPoint(index)))
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

