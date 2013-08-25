#include <QtGlobal>
#include <cmath>
#include "mathFunctions.h"
#include "globalVariables.h"
#include "database.h"
#include "itemTypeEnums.h"
#include "scales.h"

int skip(QString &line, QTextStream &in)
{
    line = in.readLine();
    while ((line=="" || line=="\n" || line=="\r\n") && !line.isNull())
        line = in.readLine();
    if (line.isNull())
        return -1;
    return 0;

}

Database::Database()
{

    this->currentItem=new QString("");
    this->currentProductLine= new QString("");
    this->productLines = new QMap<QString,ProductLine*>();

#ifdef Q_OS_LINUX
    QString path(folderPathLinux);
#endif


#ifdef Q_OS_WIN
    QString path(folderPathWin);
#endif


    int loadMode = 0; //0=loading track parts 1=loading track accesories 2=loading other models
    path.append("partsSet.set");


    QFile * dbFile = new QFile(path);

    if (!dbFile->open(QIODevice::ReadOnly | QIODevice::Text))
        exit(1);

    QTextStream in(dbFile);
    QString line("");//in.readLine();
    while (!line.isNull())
    {
        /*
         *load parts
        */
        skip(line,in);

        if (line.startsWith("[[["))
        {
            loadMode=0;
            //get name of manufacturer
            QString name = line.replace("[","");
            name = line.replace("]","");

            skip(line,in);

            //get type of product line
            QString typeStr = line;//in.readLine();
            bool type;
            if (typeStr.contains("1"))
                type=true;
            else
                type=false;

            //line = in.readLine();
            skip(line,in);

            //get scale
            //line = in.readLine();
            QString scale = line.remove(0,6);
            ScaleEnum s;
            if (type)
            {

                if (scale=="G")
                    s=SCALE_G;
                else if (scale=="L")
                    s=SCALE_L;
                else if (scale=="O" || scale=="0")
                    s=SCALE_O;
                else if (scale=="S")
                    s=SCALE_S;
                else if (scale=="OO" || scale=="00")
                    s=SCALE_OO;
                else if (scale=="HO" || scale=="H0")
                    s=SCALE_HO;
                else if (scale=="TT")
                    s=SCALE_TT;
                else if (scale=="N")
                    s=SCALE_N;
                else if (scale=="Z")
                    s=SCALE_Z;
                else if (scale=="ZZ")
                    s=SCALE_ZZ;
                else if (scale=="T")
                    s=SCALE_T;
                else
                    s=SCALE_undef;
            }
            else
            {
                if (scale=="1:24")
                    s=SCALE_T;
                else if (scale=="1:32")
                    s=SCALE_T;
                else if (scale=="1:43")
                    s=SCALE_T;
                else if (scale=="1:64" || scale=="1:87" || scale=="HO" || scale=="H0")
                    s=SCALE_HOSLOT;
                else
                    s=SCALE_undef;

            }

            skip(line,in);

            //get gauge
            //line = in.readLine();
            QString gauge = line.remove(0,6);

            skip(line,in);



            /**
              TODO
              -what causes rubbish in the db file?
            */

            ProductLine * productLine = new ProductLine(name,scale,s, gauge, type);
            this->addProductLine(productLine);
            this->setCurrentProductLine(name);


            /*
            do
                line = in.readLine();
            while (line == "\n" || line=="");
*/

        }
        if (line.startsWith("[["))
        {
            if (line.startsWith("[[Track"))
            {
                //loadMode=0;
                //skip(line,in);
                continue;
            }
            else if (line.startsWith("[[Accesories"))
            {
                loadMode=1;
                //skip(line,in);
                continue;
            }
            else //other models
            {
                loadMode=2;
                //skip(line,in);
                continue;
            }
        }

        if ((!line.isNull() || line.startsWith("[")) && loadMode==0)
        {


            QString partNo = line; //in.readLine();
            partNo = partNo.replace("[","");
            partNo = partNo.replace("]","");

            QString nameEn = in.readLine();
            QString nameCs = in.readLine();


            QString typeStr = in.readLine();
            typeStr.remove(0,5);
            ItemType t;

            if (typeStr=="C1")
                t = C1;
            else if (typeStr=="E1")
                t = E1;
            else if (typeStr=="J1")
                t = J1;
            else if (typeStr=="J2")
                t = J2;
            else if (typeStr=="J3")
                t = J3;
            else if (typeStr=="S1")
                t = S1;
            else if (typeStr=="X1")
                t = X1;
            else if (typeStr=="T1")
                t = T1;
            else if (typeStr=="T2")
                t = T2;
            else if (typeStr=="T3")
                t = T3;
            else if (typeStr=="T4")
                t = T4;
            else if (typeStr=="T5")
                t = T5;
            else if (typeStr=="T6")
                t = T6;
            else if (typeStr=="T7")
                t = T7;
            else if (typeStr=="T8")
                t = T8;
            else if (typeStr=="T9")
                t = T9;
            else if (typeStr=="T10")
                t = T10;
            else if (typeStr=="C2")
                t = C2;
            else if (typeStr=="CH")
                t = CH;
            else if (typeStr=="X1")
                t = X1;

            QString degree = in.readLine();
            degree.remove(0,7);
            qreal deg = degree.toDouble();

            QString radius = in.readLine();
            radius.remove(0,7);
            qreal rad = radius.toDouble();

            QList<QPointF> endPoints;
            QList<qreal> angles;

            QPointF pt1;
            QPointF pt2;
            QPointF pt3;
            QPointF pt4;
            ModelItem * mi = NULL;

            qreal xLen = 0;
            qreal yHeight = 0;

            //more endpoints have to be generated with generateQPointFList(n);
            switch(t)
            {
            case C1:

                //rad+=(*this->productLines->find(*this->currentProductLine))->getScaleEnum()/4.0;

                xLen = 2*rad*(cos((90-(deg/2.0))*PI/180));
                pt1 = QPointF(-xLen/2,-0*(*this->productLines->find(*this->currentProductLine))->getScaleEnum()/4.0);
                pt2 = QPointF(xLen/2,-0*(*this->productLines->find(*this->currentProductLine))->getScaleEnum()/4.0);

                endPoints.push_back(pt1);
                endPoints.push_back(pt2);

                //rad-=(*this->productLines->find(*this->currentProductLine))->getScaleEnum()/4.0;

                yHeight = (rad-rad*(sin((90-(deg/2.0))*PI/180)));

                yHeight -= (*this->productLines->find(*this->currentProductLine))->getScaleEnum()/2.0;

                angles.push_back(-deg/2.0);
                angles.push_back(deg/2.0);

                mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                break;
            case S1:
            case E1:

                pt1 = QPointF(-rad,0);
                pt2 = QPointF(rad,0);

                endPoints.push_back(pt1);
                angles.push_back(0);

                if (t==S1)
                {
                    endPoints.push_back(pt2);
                    angles.push_back(0);
                }

                //ModelItem * mi = new ModelItem(partNo,nameEn,nameCs,start,end,deg,rad, rad, 0, *this->productLines->find(*this->currentProductLine));//parentWidget??
                mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, 0, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                (*this->productLines->find(*this->currentProductLine))->addItem(mi);

                break;
            case J1:
                xLen = 2*rad*(cos((90-(deg/2.0))*PI/180));
                pt1 = QPointF(-xLen/2,0);
                pt2 = QPointF(xLen/2+xLen/10,0);
                pt3 = QPointF(xLen/2,0);

                rotatePoint(&pt2,-deg);
                pt2.setY(-8-2);

                endPoints.push_back(pt1);
                endPoints.push_back(pt2);
                endPoints.push_back(pt3);

                yHeight = (rad-rad*(sin((90-(deg/2.0))*PI/180)));


                angles.push_back(-deg/2.0);
                angles.push_back(-deg/2.0);
                angles.push_back(deg/2.0);

                mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, 0, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                (*this->productLines->find(*this->currentProductLine))->addItem(mi);

                break;
            case J2:
                break;
            case J3:
                break;
            case X1:
                break;
            case C2:
                break;
            case CH:
                break;
            case T1:
                break;
                ///TO DO: T2-T10

            }

            if (deg!=0)
            {

            }
            else
            {

            }

        }


    }

    //generate 2D and 3D models
    QMap<QString,ProductLine*>::Iterator iter1 = this->productLines->begin();
    while (iter1!=this->productLines->end())
    {
        QList<ModelItem*>::Iterator itemIter = (*iter1)->getItemsList()->begin();
        while(itemIter!=(*iter1)->getItemsList()->end())
        {
            (*itemIter)->generate2DModel(true);
            (*itemIter)->get2DModel()->setFlag(QGraphicsItem::ItemIsMovable,false);


            (*itemIter)->generate2DModel(false);

            itemIter++;
        }
        //(*iter1)->generate2DModels();
        iter1++;
    }

    //initialize graphics scene for each product line - this scene will be loaded in sideBarWidget
    this->scenes = new QMap<QString,QGraphicsScene*>();
    QMap<QString,ProductLine*>::Iterator iter = this->productLines->begin();
    while (iter!=this->productLines->end())
    {
        QGraphicsScene * scene = new QGraphicsScene();

        QString text = *(*iter)->getScale();
        text.append(" ");
        text.append(*(*iter)->getName());

        QList<ModelItem*>::Iterator itemIter = (*iter)->getItemsList()->begin();
        int i = 0;
        while(itemIter!=(*iter)->getItemsList()->end())
        {

            (*itemIter)->get2DModel()->moveBy(0,i*64+(*itemIter)->getItemHeight()); //WARNING - 64 is the variable sizeOfItem
            scene->addItem((*itemIter)->get2DModel());
            itemIter++;
            i++;


        }
        this->scenes->insert(text,scene);
        iter++;
    }

}



ProductLine * Database::findProductLineByName(QString &name) const
{
    if (name == "")
        return NULL;
    return *this->productLines->find(name);
}

int Database::setCurrentProductLine(QString &name)
{
    if (name == "")
        return 1;

    ProductLine * pl=*this->productLines->find(name);
    if (pl==NULL)
    {
        *this->currentProductLine="";
        return 2;
    }
    this->currentProductLine=pl->getName();

    return 0;

}

QString * Database::getCurrentProductLinesName()
{
    return this->currentProductLine;
}

ProductLine * Database::getCurrentProductLine()
{
    if (*this->currentProductLine=="")
        return NULL;
    return *this->productLines->find(*this->currentProductLine);
}

QMap<QString,ProductLine*>::Iterator Database::getDatabaseIterator()
{
    return this->productLines->begin();
}

/*
ModelItem * Database::findModelItemByName(QString &manufactName, QString &partName) const
{
    if (manufactName=="" || partName=="")
        return NULL;
    ProductLine * pl = *this->productLines->find(manufactName);
    pl->getItemsList();
}

ModelItem * Database::getNextModelItem(QString &manufactName) const
{}
*/

int Database::addProductLine(ProductLine * manufact)
{
    if (manufact==NULL)
        return 1;
    this->productLines->insert(*manufact->getName(),manufact);
    return 0;
}

int Database::size()
{
    return this->productLines->size();
}

/*
int Database::addModelItem(ModelItem *itemToAdd)
{}
*/


QGraphicsScene *Database::findSceneByString(QString &scaleAndName) const
{
    if (scaleAndName=="")
        return NULL;
    return *this->scenes->find(scaleAndName);
}
