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

QList<QPointF>generatePointCopies(int n, QPointF & original)
{
    QList<QPointF> list;
    if (n<0)
        return list;

    for (int i = 0; i < n; i++)
    {
        list.push_back(original);
    }


    return list;
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
            else if (typeStr=="J4")
                t = J4;
            else if (typeStr=="J5")
                t = J5;
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
            else if (typeStr=="HS")
                t = HS;
            else if (typeStr=="HE")
                t = HE;

            QString angle1;
            qreal ang1 = 0;

            if (t!=J5 && !(t>=T1 && t<=T10))
            {
                angle1 = in.readLine();
                angle1.remove(0,7);
                ang1 = angle1.toDouble();
            }


            qreal ang2 = 0;

            if (t==J1 || t==J2 || t==J3)
            {
                QString angle2 = in.readLine();
                angle2.remove(0,7);
                ang2 = angle2.toDouble();
            }


            QString radius = in.readLine();
            radius.remove(0,7);
            qreal rad = radius.toDouble();
            qreal rad2 = 0;

            if (((t==J1 || t==J2) && ang2!=0) || t==J5)
            {
                QString radius2 = in.readLine();
                radius2.remove(0,8);
                rad2 = radius2.toDouble();
            }

            QList<QPointF> endPoints;
            QList<QPointF>::Iterator endPointsIterator;
            QList<qreal> angles;


            //more endpoints have to be generated with generateQPointFList(n);
            QPointF pt1;
            QPointF pt2;
            QPointF pt3;
            QPointF pt4;
            ModelItem * mi = NULL;

            qreal xLen = 0;
            qreal yHeight = 0;

            qreal xCoord = 0;
            qreal xLen2 = 0;
            qreal trackGaugeHalf = (*this->productLines->find(*this->currentProductLine))->getScaleEnum()/2.0;


            if ((*this->productLines->find(*this->currentProductLine))->getType())
            {

                switch(t)
                {
                case C1:

                    //rad+=(*this->productLines->find(*this->currentProductLine))->getScaleEnum()/4.0;

                    xLen = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                    pt1 = QPointF(-xLen/2,0);
                    pt2 = QPointF(xLen/2,0);

                    endPoints.push_back(pt1);
                    endPoints.push_back(pt2);

                    //rad-=(*this->productLines->find(*this->currentProductLine))->getScaleEnum()/4.0;

                    yHeight = (rad-rad*(sin((90-(ang1/2.0))*PI/180)));

                    yHeight -= trackGaugeHalf;

                    angles.push_back(-ang1/2.0);
                    angles.push_back(ang1/2.0);

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
                case J2:
                    xLen = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                    pt1 = QPointF(-xLen/2,0);
                    pt2 = QPointF(xLen/2,0);
                    pt3 = QPointF(xLen/2,0);

                    //if ang2==0 => classic turnout
                    //else - two-radius turnout
                    if (t==J1)
                    {
                        if (ang2==0)
                            rotatePoint(&pt2,-ang1);
                        else
                        {
                            //compute xLen for larger radius
                            //+=
                            xCoord = 0;
                            xLen2 = 2*rad2*(cos((90-(ang2/2.0))*PI/180));
                            xCoord = xLen2/2.0 + (xLen2-xLen)/2.0;
                            xCoord += xLen/2.0;
                            pt2 = QPointF(xCoord,-cos((90-ang2/2.0)*PI/180)*xLen2);
                            rotatePoint(&pt2,ang1/2.0);
                            pt2.setX(xCoord - xLen/2.0);
                            pt2.setY(-pt2.y());
                            cout << "hello";
                        }
                    }
                    else
                    {
                        if (ang2==0)
                            rotatePoint(&pt2,180+ang1);
                        else
                        {
                        }
                    }


                    endPoints.push_back(pt1);
                    endPoints.push_back(pt2);
                    endPoints.push_back(pt3);

                    yHeight = (rad-rad*(sin((90-(ang1/2.0))*PI/180)));


                    angles.push_back(-ang1/2.0);


                    if (ang2==0)
                        angles.push_back(-ang1/2.0);
                    else
                        angles.push_back(ang2-ang1/2.0);

                    angles.push_back(ang1/2.0);


                    //mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, yHeight-8, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, yHeight-trackGaugeHalf, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                    mi->setSecondRadius(rad2);

                    break;
                case J3:
                    xLen = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                    pt1 = QPointF(-xLen/2,0);
                    pt2 = QPointF(xLen/2,0);
                    pt3 = QPointF(xLen/2,0);
                    pt4 = QPointF(xLen/2,0);

                    rotatePoint(&pt3,-ang1);
                    rotatePoint(&pt4,ang2);


                    endPoints.push_back(pt1);
                    endPoints.push_back(pt2);
                    endPoints.push_back(pt3);
                    endPoints.push_back(pt4);

                    yHeight = (rad-rad*(sin((90-(ang1/2.0))*PI/180)));


                    angles.push_back(0);
                    angles.push_back(0);
                    angles.push_back(-ang1);
                    angles.push_back(ang2);



                    //! -8 is correct? Check all scales
                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, yHeight-8, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                    break;

                //case J5:
                //    break;
                case J4:
                case X1:

                    if (t==J4)
                        xLen = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                    else
                        xLen = 2*rad;

                    pt1 = QPointF(-xLen/2.0,0);
                    pt2 = QPointF(xLen/2.0,0);
                    pt3 = QPointF(-xLen/2.0,0);
                    pt4 = QPointF(xLen/2.0,0);
                    rotatePoint(&pt3,-ang1);
                    rotatePoint(&pt4,-ang1);

                    endPoints.push_back(pt1);
                    endPoints.push_back(pt2);
                    endPoints.push_back(pt3);
                    endPoints.push_back(pt4);

                    angles.push_back(0);
                    angles.push_back(0);
                    angles.push_back(ang1);
                    angles.push_back(-ang1);

                    /**
        *fix yHeight
    */
                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, 0, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                    break;

                case J5:
                    pt1 = QPointF(-rad,rad2);
                    pt2 = QPointF(rad,rad2);
                    pt3 = QPointF(-rad,-rad2);
                    pt4 = QPointF(rad,-rad2);

                    endPoints.push_back(pt1);
                    endPoints.push_back(pt2);
                    endPoints.push_back(pt3);
                    endPoints.push_back(pt4);

                    angles.push_back(0);
                    angles.push_back(0);
                    angles.push_back(0);
                    angles.push_back(0);


                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, 2*rad, rad2, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);

                    mi->setSecondRadius(rad2);

                    break;
                case C2:
                    break;
                case CH:
                    break;
                case T1:
                    //!caution: xLen variable is used for storing the perimeter of circle
                    //!caution: xLen2 variable is used for storing angle of current rotation
                    //get perimeter
                    xLen = 2*PI*rad;
                    xLen2 = 0;
                    //get the first point in circle
                    pt1 = (QPointF(rad,0));
                    //get the maximal number of rail exits which can be placed around the circle
                    endPoints.append(generatePointCopies((xLen/(2*trackGaugeHalf+4)),pt1));

                    //!caution: xLen is now used for storing the step of rotation
                    xLen = 360/(xLen/(2*trackGaugeHalf+4));

                    endPointsIterator = endPoints.begin();
                    while (endPointsIterator!=endPoints.end())
                    {

                        rotatePoint(&(*endPointsIterator),xLen2);
                        angles.push_back(xLen2);
                        xLen2+=xLen;
                        endPointsIterator++;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, 2*rad, 2*rad, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);


                    break;
                case T2:
                case T3:
                case T4:
                case T5:
                case T6:
                case T7:
                case T8:
                case T9:
                case T10:

                    pt1 = QPointF(-rad,0);
                    pt2 = QPointF(rad,0);
                    xLen = 2*rad;
                    yHeight = (t-8)*(2*trackGaugeHalf+4);
                    endPoints.append(generatePointCopies((t-8),pt1));
                    endPoints.append(generatePointCopies((t-8),pt2));
                    //!caution: xLen2 is used as "index"
                    xLen2 = 0;

                    endPointsIterator=endPoints.begin();
                    while(xLen2!=(t-8))
                    {
                        movePoint(&*endPointsIterator,0,xLen2*(2*trackGaugeHalf+4));
                        angles.push_back(0);
                        endPointsIterator++;
                        xLen2+=1;
                    }
                    xLen2=0;
                    while (endPointsIterator!=endPoints.end())
                    {
                        movePoint(&*endPointsIterator,0,xLen2*(2*trackGaugeHalf+4));
                        angles.push_back(0);
                        endPointsIterator++;
                        xLen2+=1;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, 2*rad, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);


                    break;
                default:
                    //show error dialog
                    break;



                }

            }
            else
            {
                /*

                !pokud autodráha, uprav i proměnnou rad kvůli počítání bodů
    */
                trackGaugeHalf = abs(rad-rad2)/2.0;

                SlotTrackInfo * sti = new SlotTrackInfo();//memory leak in assignment? (later - not at this line)
                QString radius2Str = in.readLine();
                radius2Str.remove(0,8);
                QString fstLaneStr = in.readLine();
                fstLaneStr.remove(0,10);
                QString laneDistStr = in.readLine();
                laneDistStr.remove(0,9);
                QString laneDistEndStr=laneDistStr;
                if (t==HS || t==HE)
                {
                    laneDistEndStr = in.readLine();
                    laneDistEndStr.remove(0,12);
                }

                QString numOfLanesStr = in.readLine();
                numOfLanesStr.remove(0,9);

                qreal rad2sl = radius2Str.toDouble();
                qreal fstLane = fstLaneStr.toDouble();
                qreal laneDist = laneDistStr.toDouble();
                qreal laneDistEnd = laneDistEndStr.toDouble();
                qreal numOfLanes = numOfLanesStr.toDouble();


                sti->fstLaneDist=fstLane;
                sti->lanesGauge=laneDist;
                sti->lanesGaugeEnd=laneDistEnd;
                sti->numberOfLanes=numOfLanes;


                if (t==C1 || t==C2)
                {
                    //compute dimensions of item
                    qreal itemRadius = rad;
                    xLen = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                    yHeight = (rad-rad*(sin((90-(ang1/2.0))*PI/180)));
                    yHeight -= trackGaugeHalf;

                    rad -=sti->fstLaneDist;
                    for (int i = 0; i < sti->numberOfLanes; i++)
                    {
                        qreal ptX;// = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                        qreal ptY;
                        qreal phi = ang1/2.0;

                        toCartesian(rad,phi,ptX,ptY);

                        //ptY = i*sti->lanesGauge+sti->fstLaneDist;

                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(-ptX,ptY));
                        endPoints.push_back(QPointF(ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(ptX,ptY));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=sti->lanesGauge;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                }
                else if (t==S1)
                {
                    ///
                    qreal itemRadius = rad;
                    xLen = 2*rad2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    rad -=sti->fstLaneDist;
                    for (int i = 0; i < sti->numberOfLanes; i++)
                    {
                        qreal ptX = rad2sl;
                        qreal ptY = rad;


                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(-ptX,ptY));
                        endPoints.push_back(QPointF(ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(ptX,ptY));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=sti->lanesGauge;
                    }
                    ///

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                }
                else if (t==CH)
                {}
                else if (t==J1)
                {}
                else if (t==J2)
                {}
                //else if (t==C2)
                //{}
                else if (t==X1)
                {
                    ///
                    qreal itemRadius = rad;
                    xLen = 2*rad2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    rad -=sti->fstLaneDist;
                    for (int i = 0; i < sti->numberOfLanes; i++)
                    {
                        qreal ptX = rad2sl;
                        qreal ptY = rad;


                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        endPoints.push_back(QPointF(ptX,-ptY+itemRadius));
                        angles.push_back(0);
                        angles.push_back(0);
                        rad-=sti->lanesGauge;
                    }
                    rad = sti->fstLaneDist;
                    for (int i = 0; i < sti->numberOfLanes; i++)
                    {
                        qreal ptX = rad;
                        qreal ptY = rad2sl;
                        endPoints.push_back(QPointF(ptX,-sti->fstLaneDist));
                        endPoints.push_back(QPointF(ptX,2*itemRadius-sti->fstLaneDist));
                        angles.push_back(-90);
                        angles.push_back(90);
                        rad+=sti->lanesGauge;
                    }


                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                }
                else if (t==X2)
                {}
                else if (t==X3)
                {}
                else if (t==HS)
                {
                    qreal itemRadius = rad;

                    xLen = 2*rad2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    rad -=sti->fstLaneDist;
                    qreal ptY2 = rad-(sti->lanesGauge-sti->lanesGaugeEnd);
                    for (int i = 0; i < sti->numberOfLanes; i++)
                    {
                        qreal ptX = rad2sl;
                        qreal ptY = rad;


                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(-ptX,ptY));
                        endPoints.push_back(QPointF(ptX,-ptY2+itemRadius));
                        //endPoints.push_back(QPointF(ptX,ptY));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=sti->lanesGauge;
                        ptY2-=sti->lanesGaugeEnd;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                }
                else if (t==HE)
                {
                    qreal itemRadius = rad;

                    xLen = 2*rad2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    rad -=sti->fstLaneDist;
                    qreal ptY2 = rad-(sti->lanesGauge-sti->lanesGaugeEnd);
                    for (int i = 0; i < sti->numberOfLanes; i++)
                    {
                        qreal ptX = rad2sl;
                        qreal ptY = rad;


                        endPoints.push_back(QPointF(-ptX,-ptY2+itemRadius));
                        //endPoints.push_back(QPointF(-ptX,ptY));
                        endPoints.push_back(QPointF(ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(ptX,ptY));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=sti->lanesGauge;
                        ptY2-=sti->lanesGaugeEnd;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    (*this->productLines->find(*this->currentProductLine))->addItem(mi);
                }
                else if (t==H1)
                {}
                else if (t==SC)
                {}
                else if (t==JM)
                {}
                else
                {/*show error dialog*/}


                mi->setSecondRadius(rad2sl);
                mi->setSlotTrackInfo(sti);
            }

            /*
            if (ang1!=0)
            {

            }
            else
            {

            }*/

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

            (*itemIter)->get2DModel()->moveBy(0,i*(48*(*(*this->productLines->find(*this->currentProductLine))).getScaleEnum())); //WARNING - 64 is the variable sizeOfItem
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
