#include <cmath>
#include "workspaceWidget.h"
#include "globalVariables.h"
#include "mathFunctions.h"

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
    this->shiftPress=false;
    this->lastUsedPart=NULL;
    this->lastInserted=NULL;
    this->lastEventPos=NULL;
    this->activeEndPoint = NULL;//new QPointF(0,0);
    this->activeFragment = NULL;
    this->activeItem = NULL;
    this->indexUndoRedo=-1;
    this->doNotPrintSetEndPoint=false;

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

    this->undoRedoCalled=false;
    this->eraseFollowing=true;
    this->nextIDToUse=0;



}

int WorkspaceWidget::commandExecution(QString command)
{
    logFile << "executing command: " << command.toStdString() << endl;

    //make point x.x y.y
    //make last
    //make item partNo prodLineName L x.x y.y
    //select point x y
    //select item scenePosX scenePosY
    //move fragment ID dx dy
    //rotate fragment ID cx.cx cy.cy alpha
    //height point dz x.x y.y scenePos.x scenePos.y
    //delete item ID x.x y.y
    //connect xA yA xB yB IDA IDB
    //disconnect IDC pointAt IDA IDB
    //bend - not implemented yet

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
            this->makeLastItem();
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
            QString prodLineName = command.left(command.indexOf(" "));
            command.remove(0,prodLineName.length());
            command = command.trimmed();
            bool left = false;
            if (command.startsWith("L"))
                left = true;

            qreal x = 0;
            qreal y = 0;

            command.remove(0,1);
            command = command.trimmed();

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

            this->makeItem(item,newPoint,left);

            if (id!=-54321)
                this->activeFragment->setID(id);




        }
        else
        {
            logFile << "Invalid command: " << command.toStdString() << endl;
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
                delete pos;

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

            QPointF * pos = new QPointF(x,y);
/*
            ModelFragment* frag = this->findFragmentByApproxPos(pos);
            if (frag!=NULL)
                this->selectItem(frag->findEndPointItem(pos));
            else
                delete pos;*/

            GraphicsPathItem *gpi = (GraphicsPathItem*)this->graphicsScene->itemAt(*pos,QTransform());
            QGraphicsItem * qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(*pos,QTransform());

            if (qgpi->type()!=QGraphicsItem::UserType+2)
                gpi=NULL;


            if (gpi==NULL)
            {
                cout << "select item null gpi" << endl;
                return 1;
            }
            else
            {
                this->selectItem(gpi->getParentItem());
            }



        }
        else if (command.startsWith("fragment"))
        {
            cout << "select fragment" << endl;

        }


    }
    else if (command.startsWith("move"))
    {
        //move fragment ID dx dy

        command.remove(0,4);
        command = command.trimmed();
        if (!command.startsWith("fragment"))
        {
            logFile << "Invalid command: " << command.toStdString() << endl;
        }
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

        /*
        QPointF pos (id,y);


        GraphicsPathItem *gpi = (GraphicsPathItem*)this->graphicsScene->itemAt(pos,QTransform());
        QGraphicsItem * qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(pos,QTransform());

        if (qgpi!=NULL)
        {
            if (qgpi->type()!=QGraphicsItem::UserType+2)
                gpi=NULL;
        }

        if (gpi==NULL)
        {
            cout << "select item null gpi" << endl;
            return 1;
        }
        else
        {
            gpi->getParentItem()->getParentFragment()->moveBy(dx,dy);
        }*/

        /*if (frag!=NULL)
        {
            frag->moveBy(dx,dy);
        }
        else
            return 1;*/

    }
    else if (command.startsWith("rotate"))
    {

        //rotate fragment ID cx.cx cy.cy alpha
        command.remove(0,6);
        command = command.trimmed();
        if (!command.startsWith("fragment"))
        {
            logFile << "Invalid command: " << command.toStdString() << endl;
        }
        command.remove(0,8);


        qreal id = 0;
        command = command.trimmed();

        id = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        qreal cx = 0;
        qreal cy = 0;


        cx = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        cy = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        qreal alpha = 0;
        command = command.trimmed();
        alpha = command.toDouble();

        //QPointF pos (x,y);



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
    else if (command.startsWith("height"))
    {
        /**
          NOTE:
          -if there are two items at exactly same position in the scene, there is undefined behaviour - it can adjust the height of any item
*/
        //point/item/latAngle/longAngle dz x.x y.y scenePos.x scenePos.y
        //x.x y.y = point at which the height will be adjusted
        //x.2 y.2 = scenePos() of item which owns the point
        command.remove(0,6);
        command = command.trimmed();

        int type =-1; //0=point,2=latAngle,3=longAngle

        if (command.startsWith("point"))
        {
            command.remove(0,5);
            command = command.trimmed();
            type = 0;
        }
        /*else if (command.startsWith("latAngle"))
        {
            command.remove(0,8);
            command = command.trimmed();
            type = 2;
        }
        else if (command.startsWith("longAngle"))
        {
            command.remove(0,8);
            command = command.trimmed();
            type = 3;
        }*/

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
        QPointF scenePos(x2,y2);

        QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(x2,y2,8,8);

        QBrush b = ellipse->brush();
        b.setColor(Qt::red);
        b.setStyle(Qt::SolidPattern);
        ellipse->setBrush(b);
        //this->graphicsScene->addItem(ellipse);


        ModelItem * mi = NULL;
        GraphicsPathItem * gpi = (GraphicsPathItem *)this->graphicsScene->itemAt(scenePos,QTransform());
        QGraphicsItem * qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(scenePos,QTransform());

        if (qgpi!=NULL)
        {
            if (qgpi->type()!=QGraphicsItem::UserType+2)
                gpi=NULL;
        }

        if (gpi==NULL)
        {
            cout << "height null gpi" << endl;
            return 1;

        }
        if (gpi!=NULL)
        {
            mi = gpi->getParentItem();

            //gpi=mi->get2DModelNoText();
        }

        if (mi!=NULL)
        {
            if (type==0)
                mi->adjustHeightProfile(dz,pos);
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
        //delete item id x.x y.y where id is parentFragment id
        //delete fragment id
        command.remove(0,6);
        command = command.trimmed();

        int type = -1; //0=item, 1=fragment
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

        qreal id = 0;
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
            return 0; //rather return the value of removeFragment()

        }

        qreal x = 0;
        qreal y = 0;


        x = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        y = command.toDouble();

        QPointF * pos = new QPointF(x,y);



        GraphicsPathItem * gpi = (GraphicsPathItem *)this->graphicsScene->itemAt(*pos,QTransform());
        QGraphicsItem * qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(*pos,QTransform());

        if (qgpi->type()!=QGraphicsItem::UserType+2)
            gpi=NULL;

        if (gpi==NULL)
        {
            cout << "delete null gpi" << endl;
            return 1;
        }
        if (gpi!=NULL)
        {
            if (type==0)
            {
                ModelItem * mi = gpi->getParentItem();
                this->removeItem(mi);

            }
            /*if (type==1)
            {
                ModelFragment * frag = gpi->getParentItem()->getParentFragment();
                this->removeFragment(frag);
            }*/

        }
        delete pos;
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
        qreal idA = 0;
        qreal idB = 0;

        xA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        xB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        idA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        idB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();

        QPointF * posA = new QPointF(xA,yA);
        QPointF * posB = new QPointF(xB,yB);

        /*
        //items
        qreal xIA = 0;
        qreal yIA = 0;
        qreal xIB = 0;
        qreal yIB = 0;

        xIA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yIA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        xIB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yIB = command.toDouble();

        QPointF * posIA = new QPointF(xIA,yIA);
        QPointF * posIB = new QPointF(xIB,yIB);

        QRectF rectIA(posIA->x()-30,posIA->y()-30,60,60);
        QRectF rectIB(posIB->x()-30,posIB->y()-30,60,60);*/

/*        QList <QGraphicsItem*> list = this->graphicsScene->items(rectIA);
        GraphicsPathItem * gpiA = (GraphicsPathItem *)list.first();
        cout << "(" << gpiA->scenePos().x() << ", "<< gpiA->scenePos().y() << ")" << endl;

        QList <QGraphicsItem*>list2 = this->graphicsScene->items(rectIB);
        GraphicsPathItem * gpiB = (GraphicsPathItem *)list2.first();
        cout << "(" << gpiB->scenePos().x() << ", "<< gpiB->scenePos().y() << ")" << endl;

        ModelItem * mi = this->findItemByApproxPos(posA,NULL);
        ModelItem * mi2 = this->findItemByApproxPos(posB,mi);
        GraphicsPathItem * gpiA = mi->get2DModelNoText();
        GraphicsPathItem * gpiB = mi2->get2DModelNoText();

        this->connectFragments(posA,posB,gpiA->getParentItem()->getParentFragment(),gpiB->getParentItem()->getParentFragment(),gpiA->getParentItem(),gpiB->getParentItem());
* /


        GraphicsPathItem * gpiA = (GraphicsPathItem *)this->graphicsScene->itemAt(*posIA,QTransform());
        QGraphicsItem * qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(*posIA,QTransform());

        if (qgpi->type()!=QGraphicsItem::UserType+2)
            gpiA=NULL;


        GraphicsPathItem * gpiB = (GraphicsPathItem *)this->graphicsScene->itemAt(*posIB,QTransform());
        QGraphicsItem * qgpi2 = (QGraphicsItem *)this->graphicsScene->itemAt(*posIB,QTransform());

        if (qgpi2->type()!=QGraphicsItem::UserType+2)
            gpiB=NULL;


        if (gpiA==NULL)
        {
            /*ModelItem * mi = this->findItemByApproxPos(posA,NULL);
            if (mi!=NULL)
                gpiA=mi->get2DModelNoText();* /
            return 1;
        }
        if (gpiB==NULL)
        {
            /*ModelItem * mi = this->findItemByApproxPos(posB,NULL);
            if (mi!=NULL)
                gpiB=mi->get2DModelNoText();* /
            return 1;
        }

        if (gpiA==NULL || gpiB==NULL)
            return 1;*/

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


        this->connectFragments(posA,posB,fA,fB,itemA,itemB);

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
            logFile << "ERROR: item was not found - idC " << idC << endl;
            return 1;
        }
        QPointF pos(x,y);

        /*
        QPointF * posA = new QPointF(xA,yA);
        QPointF * posB = new QPointF(xB,yB);

        //items
        qreal xIA = 0;
        qreal yIA = 0;
        qreal xIB = 0;
        qreal yIB = 0;

        xIA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yIA = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        xIB = (command.left(command.indexOf(" "))).toDouble();
        command = command.remove(0,command.indexOf(" ")).trimmed();
        yIB = command.toDouble();

        QPointF * posIA = new QPointF(xIA,yIA);
        QPointF * posIB = new QPointF(xIB,yIB);


        GraphicsPathItem * gpiA = (GraphicsPathItem *)this->graphicsScene->itemAt(*posIA,QTransform());
        GraphicsPathItem * gpiB = (GraphicsPathItem *)this->graphicsScene->itemAt(*posIB,QTransform());
        QGraphicsItem * qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(*posIA,QTransform());

        if (qgpi->type()!=QGraphicsItem::UserType+2)
            gpiA=NULL;

        qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(*posIB,QTransform());

        if (qgpi->type()!=QGraphicsItem::UserType+2)
            gpiB=NULL;
*/

        /*if (gpiA==NULL)
        {
            ModelItem * mi = this->findItemByApproxPos(posA,NULL);
            if (mi!=NULL)
                gpiA=mi->get2DModelNoText();
        }
        if (gpiB==NULL)
        {
            ModelItem * mi = this->findItemByApproxPos(posB,NULL);
            if (mi!=NULL)
                gpiB=mi->get2DModelNoText();
        }*/
/*
        if (gpiA==NULL || gpiB==NULL)
            return 1;

        if (gpiA!=gpiB)
            this->disconnectFragments(gpiA->getParentItem()->getParentFragment(),posB,idA, idB);
        else
        {
            QList <QGraphicsItem*>list = this->graphicsScene->items(*posB);
            for (int i = 0; i < list.count(); i++)
            {
                if (gpiA!=list.at(i))
                    gpiB=(GraphicsPathItem*)list.at(i);
            }
            ///this->disconnectFragments(posA,posB,gpiA->getParentItem()->getParentFragment(),gpiB->getParentItem()->getParentFragment(),gpiA->getParentItem(),gpiB->getParentItem());

        }*/

        this->disconnectFragments(c,&pos,idA, idB);
    }
    else if (command.startsWith("bend"))
    {}

    return 0;
}

int WorkspaceWidget::pushBackCommand(QString command,QString negCommand)
{
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
    //this->actionListRedo.append(QString("make last"));

    if (this->lastUsedPart==NULL)
        return -1;



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
    }
    GraphicsPathItem * gpi = this->lastUsedPart->get2DModelNoText();
    QPointF pt;
    if (this->lastUsedPart->getRadius()<0)
        pt = QPointF(gpi->scenePos().x()+1,gpi->scenePos().y()+1);
    else
        pt = QPointF(gpi->scenePos().x()+gpi->boundingRect().width()-2,gpi->scenePos().y()+gpi->boundingRect().height()-2);

    if (this->canInsert(this->lastUsedPart))
        makeNewItem(*this->lastEventPos,gpi,this->lastUsedPart,this->lastUsedPart, true);



    ModelItem * lastInserted = NULL;
    if (this->activeFragment!=NULL)
        lastInserted = this->activeFragment->getFragmentItems()->last();
    else
        lastInserted = this->activeFragmentPrev->getFragmentItems()->last();
    QString negStr = QString("delete item %1 %2 %3").arg(QString::number(lastInserted->getParentFragment()->getID()),QString::number(lastInserted->get2DModelNoText()->scenePos().x()),QString::number(lastInserted->get2DModelNoText()->scenePos().y()));
    this->pushBackCommand(QString("make last"),negStr);

    return 0;
}



int WorkspaceWidget::makeItem(ModelItem *item, QPointF *pt, bool left)
{
    QPointF pos;
    if (left)
        pos.setX(item->get2DModelNoText()->scenePos().x()-5);
    else
        pos.setX(item->get2DModelNoText()->scenePos().x()+5);


    if (pt!=NULL)
        this->setActiveEndPoint(pt);
    else ///version with delete causes SEGFAULT
        this->setActiveEndPoint(this->activeEndPoint);

    QPointF ptOld=*this->getActiveEndPoint();

    ModelFragment * previouslyActiveFragment = app->getWindow()->getWorkspaceWidget()->getActiveFragment();
    int fragCountBefore = this->modelFragments->count();

    //insert item without printing commands from setActiveEndPoint()

    this->setActiveFragment(this->findFragmentByApproxPos(pt));

    this->doNotPrintSetEndPoint=true;
    if (this->canInsert(item))
        makeNewItem(pos,item->get2DModelNoText(),item,item,false);

    this->doNotPrintSetEndPoint=false;

    //this->setActiveEndPoint(pt);


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

    if (pt!=NULL)
        this->setActiveEndPoint(pt);
    else ///version with delete causes SEGFAULT
        this->setActiveEndPoint(this->activeEndPoint);

    QPointF ptOld=*this->getActiveEndPoint();

    ModelFragment * previouslyActiveFragment = app->getWindow()->getWorkspaceWidget()->getActiveFragment();

    int fragCountBefore = this->modelFragments->count();

    this->doNotPrintSetEndPoint=true;
    if (this->canInsert(item))
        makeNewItem(eventPos,item->get2DModelNoText(),item,item,false);
    this->doNotPrintSetEndPoint=false;

    bool nullIt = false;
    if (pt==NULL)
    {
        pt = new QPointF(*this->activeEndPoint);
        nullIt = true;
    }


    ModelItem * lastInserted = this->activeFragment->getFragmentItems()->last();

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

    //this->connectFragments(lastInserted->getParentFragment());

    return 0;
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

    this->mousePress=false;

}
void WorkspaceWidget::wheelEvent(QWheelEvent *evt)
{

    if (evt->delta()>0 && this->ctrlPress)
    {
        this->scaleView(1.05);
        this->centerOn(evt->x(),evt->y());
        //this->ensureVisible(evt->pos().x(),evt->pos().y(),1,1);
    }
    else if (evt->delta()<0 && this->ctrlPress)
    {
        this->scaleView(0.95);
        //this->ensureVisible(evt->pos().x(),evt->pos().y(),1,1);
        this->centerOn(evt->x(),evt->y());
    }
    else if (evt->delta()<0 && this->shiftPress)
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()-15);
    else if (evt->delta()>0 && this->shiftPress)
    {
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()+15);
    }
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
        this->ctrlPress=true;
    else if (event->key()==Qt::Key_Shift)
        this->shiftPress=true;
    else if(event->key()==Qt::Key_Space && this->lastUsedPart!=NULL && !this->heightProfileMode)
    {
        makeLastItem();
        //commandExecution(QString("make last"));
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

                    this->removeItem(this->modelFragments->at(i)->getFragmentItems()->at(j));
                    i=0;
                    j=0;
                    break;
                }
            }
        }
        this->deletePress=false;
    }
    else if (event->key()==Qt::Key_R)
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
    else if (event->key()==Qt::Key_Z && this->ctrlPress)
        undo();

}
void WorkspaceWidget::keyReleaseEvent(QKeyEvent *event)
{

    if (event->key()==Qt::Key_Control)
        this->ctrlPress=false;
    else if (event->key()==Qt::Key_Shift)
        this->shiftPress=false;
    //QGraphicsView::keyReleaseEvent(event);
}

int WorkspaceWidget::selectItem(ModelItem* item)
{

    if (item==NULL)
        return 1;

    //this->actionListRedo.append(QString("select item %1 %2").arg(QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y())));

    QString str = QString("select item %1 %2").arg(QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
    QString nStr = QString("deselect item %1 %2").arg(QString::number(item->get2DModelNoText()->scenePos().x()),QString::number(item->get2DModelNoText()->scenePos().y()));
    this->pushBackCommand(str,nStr);


    GraphicsPathItem * gpi = NULL;
    if (item->getParentFragment()==NULL)
        gpi= item->get2DModel();
    else
        gpi = item->get2DModelNoText();



    QList<QGraphicsItem*> list = gpi->scene()->selectedItems();
    QList<QGraphicsItem*>::Iterator iter = list.begin();
    while (iter !=list.end())
    {
        (*iter)->setSelected(false);
        iter++;
    }


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
        item->get2DModelNoText()->setSelected(false);


    return 0;
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

                this->doNotPrintSetEndPoint=true;


                if (area.contains(**bPointIter) && a!=(*fragIter) && !firstFound && *aEPItemIter!=*bEPItemIter)
                {

/*
                    //modify neighbour of "a" endItem at this point
                    (*aEPItemIter)->setNeighbour(*bEPItemIter,*bPointIter);


                    //modify Neighbour of (*fragIter) endItem at this point
                    (*bEPItemIter)->setNeighbour(*aEPItemIter,*bPointIter);*/
                }

                else if (area.contains(**bPointIter) && a!=(*fragIter) && firstFound)
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

                    //connect xA yA xB yB IDA IDB
                    //disconnect IDC pointAt.x pointAt.y IDA IDB


                    this->doNotPrintSetEndPoint=false;
                    this->setActiveEndPoint(this->activeEndPoint);
                    this->doNotPrintSetEndPoint=true;

                    this->pushBackCommand(QString("#connect"),QString("#disconnect"));


                    QString str = (QString("@connect %1 %2 %3 %4 %5 %6").arg(QString::number((*aPointIter)->x()),QString::number((*aPointIter)->y()),
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

                    {
                        int k = 0;
                        for (;k < c->getEndPoints()->count(); k++)
                        {
                            if (pointsAreCloseEnough(newActive,c->getEndPoints()->at(k)))
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

                    this->addFragment(c);

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

int WorkspaceWidget::connectFragments(QPointF *posA, QPointF *posB, ModelFragment * a, ModelFragment * b, ModelItem * aItem, ModelItem * bItem)
{
    //connect xA yA xB yB IDA IDB
    //disconnect IDC pointAt IDA IDB
    QString str = (QString("@connect %1 %2 %3 %4 %5 %6 %7 %8").arg(QString::number(posA->x()),QString::number(posA->y()),
                                                                  QString::number(posB->x()),QString::number(posB->y()),
                                                                  QString::number(a->getID()),QString::number(b->getID())
                                                                  ));



    this->doNotPrintSetEndPoint=true;

    if (a->getEndPoints()->empty() || b->getEndPoints()->empty())
        return 0;

    //this is used for:
    //      -when the first point of connection is found the lists and parentFragment attributes are modified, then it is set to false
    //      -after checking all points of one fragment check if it is false -> delete old fragment, otherwise noop with fragment
    bool firstFound = true;

    ModelFragment * bFirstFound = NULL;
    int bFirstFoundCountOfItems = 0;

    /*
    bool aEPILeft = false;//info just about EPItem
    bool bEPILeft = false;
    bool aEPI180diff = false;
    bool bEPI180diff = false;
    bool aLeft = false;//info about whole fragment
    bool bLeft = false;*/


    bool loop = true;


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
    /*bEPILeft = !(bool)(bI%2);
    if (bI%2==0)
        bEPI180diff = (bItem)->leftRightDifference180(bI,bI+1);
    else
        bEPI180diff = (bItem)->leftRightDifference180(bI-1,bI);
*/

    ModelItem * firstItemWith180Diff = NULL;
    ModelItem * uselessPointer = NULL;
    //get info about l-r side of A
    //aLeft = a->leftSide(aItem, firstItemWith180Diff);
    /*if (aItem==firstItemWith180Diff && aI%2==0)
        aLeft=true;
*/

    //get info about l-r side of B
    //bLeft = b->leftSide(bItem, uselessPointer);
    /*if (bItem==uselessPointer && bI%2==0)
        bLeft=true;
*/
    int x = 0;
    int y = 0;
    while (x < b->getEndPoints()->count() && !pointsAreCloseEnough(b->getEndPoints()->at(x),posB))
        x++;
    while (y < a->getEndPoints()->count() && !pointsAreCloseEnough(a->getEndPoints()->at(y),posA))
        y++;

    qreal dAlpha =b->getEndPointsAngles()->at(x)-a->getEndPointsAngles()->at(y);

    logFile << "    Fragments will be connected" << endl;
    logFile << "        A: " << a << "\n    B: " << b << endl;
    /*logFile << "        A: " << aEPI180diff << aLeft << aEPILeft << endl;
    logFile << "        B: " << bEPI180diff << bLeft << bEPILeft << endl;*/


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
            QRectF r2((aItem)->getEndPoint(k)->x()-TOLERANCE_HALF,(aItem)->getEndPoint(k)->y()-TOLERANCE_HALF,2*TOLERANCE_HALF,2*TOLERANCE_HALF);
            while ((bItem)->getEndPoint(j)!=NULL)
            {
                if (r2.contains(*(bItem)->getEndPoint(j)))
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


    loop = false;
    firstFound=false;
    bFirstFound=c;
    //bFirstFoundCountOfItems=bCountBefore;

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
            if (pointsAreCloseEnough(newActive,c->getEndPoints()->at(k)))
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
    delete (b);
    b=c;
    this->addFragment(c);

    QString negStr = (QString("@disconnect %1 %2 %3 %4 %5 ").arg(QString::number(c->getID()),
                                                               QString::number((posA)->x()),QString::number((posB)->y()),
                                                               QString::number(a->getID()),QString::number(b->getID())));


    this->pushBackCommand(str,negStr);


    //now check which points have been deleted:
    //loop NxN bListOf... and b.endPoints
    //if there is some point in bList.. which is missing in b.EPs, set it to NULL (no delete *!!!)
    //...then check null pointers when incrementing the iterator at the end of the loop
    /*QList<QPointF*>::Iterator iter = bListOfPoints.begin();
    for (int i = 0; i < bListOfPoints.count(); i++,iter++)
    {
        if (!b->getEndPoints()->contains(bListOfPoints.at(i)))
            *iter=NULL;

    }*/



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

int WorkspaceWidget::disconnectFragments(ModelFragment *c, QPointF *disconnectAt, int idA, int idB)
{

    this->modelFragments->removeOne(c);

    //now find the item at which fragment should be divided

    GraphicsPathItem * gpi = (GraphicsPathItem*)this->graphicsScene->itemAt(*disconnectAt,QTransform());
    QGraphicsItem * qgpi = (QGraphicsItem *)this->graphicsScene->itemAt(*disconnectAt,QTransform());

    if (qgpi->type()!=QGraphicsItem::UserType+2)
        gpi=NULL;

    if (gpi==NULL)
        return 1;

    ModelItem * mi = gpi->getParentItem();
    int i = 0;
    while (mi->getEndPoint(i)!=NULL)
    {
        if (pointsAreCloseEnough(mi->getEndPoint(i),disconnectAt))
            break;
        i++;
    }
    if (mi->getEndPoint(i)==NULL)
        i=0;

    if (mi->getNeighbour(i)==NULL)
        return 0;

    ModelItem * mi2 = mi->getNeighbour(i);
    mi->getNeighbour(mi->getEndPoint(i))->setNeighbour(NULL,mi->getEndPoint(i));
    mi->setNeighbour(NULL,mi->getEndPoint(i));
    if (mi->getSlotTrackInfo()!=NULL || (mi->getType()>=T2 && mi->getType()<=T10))
    {
        int j = i%2;
        while (mi->getEndPoint(i)!=NULL)
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
        if (t==C1 || t==S1 || t==E1)
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
        if (t==C1 || t==S1 || t==E1)
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


    ModelFragment * b = new ModelFragment(mi);
    rebuildFragment(mi,b);
    if (mi2->getParentFragment()==b)
    {
        return 0;
    }
    ModelFragment * a = new ModelFragment(mi2);
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

    this->addFragment(b, idA);
    this->addFragment(a, idB);


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
        //(*iter)->get2DModelNoText()->moveBy(this->graphicsScene->width()/2,this->graphicsScene->height()/2);


        this->graphicsScene->addItem((*iter)->get2DModelNoText());

        iter++;
    }

    if (fragID!=-1)
        frag->setID(fragID);
    else
        frag->setID(this->nextIDToUse);

    this->nextIDToUse++;



    //QList<QGraphicsItem*> list = this->graphicsScene->items(Qt::DescendingOrder);
    return 0;
}
int WorkspaceWidget::removeFragment(ModelFragment * frag)
{
    if (frag==NULL)
        return 1;
    bool success = this->modelFragments->removeOne(frag);
    if (success)
    {
        if (this->activeFragment==frag)
            this->setActiveFragment(NULL);

        delete frag;

        return 0;
    }
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

int WorkspaceWidget::removeItem(ModelItem *item)
{

    /**
      TODO
      something is missing here (fragment id)
      -is it needed to write ID if two fragments are created due to removal of item?
*/

    QPointF pos (item->get2DModelNoText()->scenePos());
    //QString negStr = str;
    QString str;
    str = QString("delete item %1 %2 %3").arg(QString::number(item->getParentFragment()->getID()),QString::number(pos.x()),QString::number(pos.y()));

    //QString negStr = QString("make item %1 %2 R %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y()));

    QString negStr;
    if (item->getParentFragment()->getFragmentItems()->count()==1)
        negStr = QString("make item %1 %2 R %3 %4 %5").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y()),QString::number(item->getParentFragment()->getID()));
    else
        negStr = QString("make item %1 %2 R %3 %4").arg(*item->getPartNo(),*item->getProdLine()->getName(),QString::number(item->getEndPoint(0)->x()),QString::number(item->getEndPoint(0)->y()));

    this->pushBackCommand(str,negStr);
    //this->actionListRedo.append();

    ModelFragment * original = item->getParentFragment();
    int ret = item->getParentFragment()->deleteFragmentItem(item);
    //int ret = this->modelFragments->at(i)->deleteFragmentItem(this->modelFragments->at(i)->getFragmentItems()->at(j));
    if (ret==-1)
    {

        delete original;
        this->modelFragments->removeOne(original);
        //break;
    }
    return 0;
}

int WorkspaceWidget::updateFragment(ModelFragment *frag)
{
    for (int i = 0; i < frag->getFragmentItems()->count();i++)
    {
        //removes all items (just added items won't have effect on this action)
        if (frag->getFragmentItems()->at(i)->get2DModelNoText()->scene()!=NULL)
            this->graphicsScene->removeItem(frag->getFragmentItems()->at(i)->get2DModelNoText());

        qreal x = frag->getFragmentItems()->at(i)->get2DModelNoText()->scenePos().x()-2.5;
        qreal y = frag->getFragmentItems()->at(i)->get2DModelNoText()->scenePos().y()-2.5;
        QGraphicsEllipseItem * ellipse = new QGraphicsEllipseItem(x,y,5,5);



        //add all items including those just added
        this->graphicsScene->addItem(frag->getFragmentItems()->at(i)->get2DModelNoText());
        //this->graphicsScene->addItem(ellipse);


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
        item->updateEndPointsHeightGraphics();
}

QPointF *WorkspaceWidget::getActiveEndPoint() const
{
    return this->activeEndPoint;
}

int WorkspaceWidget::setActiveEndPoint(QPointF *pt)
{
    /*
    /**
      TODO:
      -negative action
    * /
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
    pp->addEllipse(-7.5,-7.5,15,15);

    QGraphicsPathItem * qgpi = new QGraphicsPathItem(*pp);
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
    qgpi->setZValue(-10);
    this->graphicsScene->addItem(qgpi);
    this->activeEndPointGraphic=qgpi;

*/

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
    /*if (this->activeEndPoint!=NULL)
        delete this->activeEndPoint;*/

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
    QPainterPath * pp = new QPainterPath();
    pp->addEllipse(-7.5,-7.5,15,15);

    QGraphicsPathItem * qgpi = new QGraphicsPathItem(*pp);
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

int WorkspaceWidget::setLastInserted(ModelItem *item)
{
    if (item==NULL)
        return 1;
    this->lastInserted=item;
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
                && pLFirst->getScaleEnum()==item->getProdLine()->getScaleEnum())
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

int WorkspaceWidget::exportCurrentState(ofstream &file)
{
    QPointF p1 = this->viewport()->pos();
    QSize p2 = this->viewport()->size();
    qreal x = p1.x()+p2.width()/2;
    qreal y = p1.y()+p2.height()/2;

    file << x << " " << y << endl;
    file << this->scene()->sceneRect().x() << " " << this->scene()->sceneRect().y() << " " << this->scene()->sceneRect().width() << " " << this->scene()->sceneRect().height() << endl;
    file << this->transform().m11() << endl;


    for (int i = 0; i < this->modelFragments->count(); i++)
    {
        ModelFragment * f = this->modelFragments->at(i);
        ModelItem * it0 = f->getFragmentItems()->at(0);
        file << "make item " << it0->getPartNo()->toStdString() << " " << it0->getProdLine()->getName()->toStdString() << (it0->getRadius()<0 ? " L " : " R ") << it0->get2DModelNoText()->scenePos().x() << " " << it0->get2DModelNoText()->scenePos().y() << endl;
        for (int j = 1; j < f->getFragmentItems()->count(); j++)
        {
            ModelItem * it = f->getFragmentItems()->at(j);
            if (it->getNeighbour(0)==NULL)
                file << "null neighbour " << -180+it->getTurnAngle(0) << " " << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y() << endl;
            file << "make item " << it->getPartNo()->toStdString() << " " << it->getProdLine()->getName()->toStdString() << (it->getRadius()<0 ? " L " : " R ") << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y() << endl;

            if (it->getNeighbour(0)==NULL)
                file << "null neighbour2 " << it->getTurnAngle(0) << " " << it->getEndPoint(0)->x() << " " << it->getEndPoint(0)->y()  << endl;
        }
    }
    //how about additional models? Trees, houses etc.

    return 0;
}

int WorkspaceWidget::setCurrentState(ifstream &file)
{
    this->resetWorkspace();
    ///sceneFocusPosition?
    ///sceneZoom
    ///activeEndPoint?

    //centerPoint of view
    string strCoord;
    getline(file,strCoord);

    qreal xC = 0;
    qreal yC = 0;
    QString s;
    s = s.fromStdString(strCoord);
    xC = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    yC = s.toDouble();



    qreal x = 0;
    qreal y = 0;
    qreal w = 0;
    qreal h = 0;
    //scene size
    getline(file,strCoord);
    s = s.fromStdString(strCoord);
    x = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    y = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    w = (s.left(s.indexOf(" "))).toDouble();
    s = s.remove(0,s.indexOf(" ")).trimmed();
    h = s.toDouble();

    this->graphicsScene->setSceneRect(x,y,w,h);


    x = 0;
    getline(file,strCoord);
    s = s.fromStdString(strCoord);
    x = s.toDouble();

    this->scale(x,x);

    this->centerOn(xC,yC);


    while(!file.eof())
    {
        string str;
        getline(file,str);
        this->commandExecution(QString().fromStdString(str));
    }
}

void WorkspaceWidget::resetWorkspace()
{
    for (int i = this->indexUndoRedo; i >=0; i--)
    {
        this->undo();
    }
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

void WorkspaceWidget::undo()
{
//    when it should continue:
//        -select fragment & make point & select point - no combos
//        -make point & select point
//        -make point & select point & rotate fragment - + combos
//        -rotate & rotate


    bool nextStep = false;

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

            ))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("deselect item"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("height point"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("select item"))
            ||
            (this->actionListUndo.at(this->indexUndoRedo).startsWith("select fragment")))

        nextStep = true;


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
            /*ModelFragment * f = NULL;
            if (this->activeFragment!=NULL)
            {
                originalId = this->activeFragment->getID();
                f = this->activeFragment;
            }
            else
            {
                originalId = this->activeFragmentPrev->getID();
                f = this->activeFragmentPrev;
            }
            int cBefore = this->modelFragments->count();
            this->connectFragments(f);

            if (this->modelFragments->count()!=cBefore)
            {
                ///WHICH LINE??
                this->activeFragment;
                this->modelFragments->last()->setID(originalId);
            }*/
            originalId = this->lastInserted->getParentFragment()->getID();
            int cBefore = this->modelFragments->count();
            this->connectFragments(this->lastInserted->getParentFragment());
            if (this->modelFragments->count()!=cBefore)
            {
                ///WHICH LINE??
                this->activeFragment;
                this->modelFragments->last()->setID(originalId);
            }


        }



        this->indexUndoRedo--;
        this->undoRedoCalled = false;
    }
    this->eraseFollowing=true;





    if (nextStep)
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
            ))

        nextStep = true;


    this->eraseFollowing=false;
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

