/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

    This file is a part of Rail & Slot Editor.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QtGlobal>
#include <cmath>
#include "mathFunctions.h"
#include "globalVariables.h"
#include "database.h"
#include "itemTypeEnums.h"
#include "scales.h"


void showDBError(const QString & messageEn,const QString & messageCs, const QString & lang)
{
    QMessageBox * messageDialog = new QMessageBox();
    if (lang.startsWith("EN"))
        messageDialog->setText(messageEn);
    else
        messageDialog->setText(messageCs);
    messageDialog->setIcon(QMessageBox::Critical);
    if (lang.startsWith("EN"))
        messageDialog->setButtonText(0,"Accept");
    else
        messageDialog->setButtonText(0,"OK");

    messageDialog->exec();
}

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

Database::Database(QString &lang)
{

    this->currentItem=new QString("");
    this->currentProductLine= NULL;
    this->productLines = new QMap<QString,ProductLine*>();

#ifdef Q_OS_LINUX
    QString path(folderPathLinux);
#endif


#ifdef Q_OS_WIN
    QString path(folderPathWin);
#endif


    path.prepend(qApp->applicationDirPath());

    int loadMode = 0; //0=loading track parts 1=loading track accesories 2=loading other models
    path.append("partsSet.rsd");


    QFile * dbFile = new QFile(path);



    if (!dbFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        logFile << "Couldnt open the database file" << endl;
        logFile << "Application was executed from path: " << qApp->applicationDirPath().toStdString() << endl;

        QMessageBox * messageDialog = new QMessageBox();
        if (lang.startsWith("EN"))
            messageDialog->setText(QString("Database file couldn't be opened. Please check if it is placed in \"RSEditor\" folder. Application will exit."));
        else
            messageDialog->setText(QString("Nepodařilo se otevřít databázi dílu. Prosím ověřte, zda je soubor umístěn ve složce \"RSEditor\". Aplikace se ukončí."));
        messageDialog->setIcon(QMessageBox::Critical);
        messageDialog->setButtonText(0,"OK");
        messageDialog->exec();

        exit(1);
    }

    QTextStream in(dbFile);
    QString encoding = in.readLine();

    if (encoding.startsWith("ISO 8859-2"))
        in.setCodec("ISO 8859-2");
    else
        in.setCodec("UTF-8");
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
                    s=SCALE_24;
                else if (scale=="1:32")
                    s=SCALE_32;
                else if (scale=="1:43")
                    s=SCALE_43;
                else if (scale=="1:64" || scale=="1:87" || scale=="HO" || scale=="H0")
                    s=SCALE_HOSLOT;
                else
                    s=SCALE_undef;

            }

            skip(line,in);

            //get gauge
            QString gauge = line.remove(0,6);

            skip(line,in);

            while (this->productLines->value(name,NULL)!=NULL)
            {
                name.append(" ");
            }

            if (s==SCALE_undef)
            {
                showDBError(QString("Product line \"%1\" contains unsupported scale. It was added in the database anyway.").arg(name),QString("Produktová řada \"%1\" obsahuje nepodporované měřítko. I přesto byla přidána do databáze.").arg(name),lang);

            }

            ProductLine * productLine = new ProductLine(name,scale,s, gauge, type);

            this->addProductLine(productLine);
            this->setCurrentProductLine(name);


        }
        if (line.startsWith("[["))
        {
            if (line.startsWith("[[Track"))
            {
                loadMode=0;
                continue;
            }
            else if (line.startsWith("[[Accesories"))
            {
                loadMode=1;
                continue;
            }
            else //vegetation
            {
                loadMode=2;
                continue;
            }
        }

        if ((!line.isNull() || line.startsWith("[")) && loadMode==0)
        {


            QString partNo = line;
            partNo = partNo.replace("[","");
            partNo = partNo.replace("]","");

            QString nameEn = in.readLine();
            QString nameCs = in.readLine();


            QString typeStr = in.readLine();
            typeStr.remove(0,5);
            ItemType t = UNKNOWN;

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
            else if (typeStr=="SC")
                t = SC;
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
            else if (typeStr=="CB")
                t = CB;
            else if (typeStr=="HC")
                t = HC;
            else if (typeStr=="X2")
                t = X2;
            else if (typeStr=="HS")
                t = HS;
            else if (typeStr=="HE")
                t = HE;
            else if (typeStr=="JM")
                t = JM;

            QString angle1;
            qreal ang1 = 0;

            if (!(t>=T1 && t<=T10) && t!=J5 && t!=X2)
            {
                angle1 = in.readLine();
                angle1.remove(0,7);
                ang1 = angle1.toDouble();
            }


            qreal ang2 = 0;

            if ((t==J1 || t==J2 || t==J3) && (*this->productLines->find(*this->currentProductLine))->getType())
            {
                QString angle2 = in.readLine();
                angle2.remove(0,7);
                ang2 = angle2.toDouble();
            }


            QString radius = in.readLine();
            radius.remove(0,7);
            qreal rad = radius.toDouble();
            qreal radi2 = 0;

            if (((t==J1 || t==J2) && (*this->productLines->find(*this->currentProductLine))->getType() && ang2!=0) || (t==J5 && (*this->productLines->find(*this->currentProductLine))->getType()))
            {
                QString radius2 = in.readLine();
                radius2.remove(0,8);
                radi2 = radius2.toDouble();
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

            if (rad==0)
            {
                showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                continue;
            }


            if ((*this->productLines->find(*this->currentProductLine))->getType())
            {

                switch(t)
                {
                case C1:

                    xLen = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                    pt1 = QPointF(-xLen/2,0);
                    pt2 = QPointF(xLen/2,0);

                    endPoints.push_back(pt1);
                    endPoints.push_back(pt2);

                    yHeight = (rad-rad*(sin((90-(ang1/2.0))*PI/180)));

                    yHeight -= trackGaugeHalf;

                    angles.push_back(-ang1/2.0);
                    angles.push_back(ang1/2.0);

                    if (ang1==0)
                    {
                        showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                        continue;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??


                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                    break;
                case S1:
                //case SC:
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



                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, 0, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                    break;
                case J1:
                case J2:

                    if (ang1==0 || (ang2!=0 && radi2==0) || (ang2==0 && radi2!=0))
                    {
                        showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                        continue;
                    }


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
                            xLen2 = 2*radi2*(cos((90-(ang2/2.0))*PI/180));
                            xCoord = xLen2/2.0 + (xLen2-xLen)/2.0;
                            xCoord += xLen/2.0;
                            pt2 = QPointF(xCoord,-cos((90-ang2/2.0)*PI/180)*xLen2);
                            rotatePoint(&pt2,ang1/2.0);
                            pt2.setX(xCoord - xLen/2.0);
                            pt2.setY(-pt2.y());

                        }
                    }
                    else
                    {
                        if (ang2==0)
                            rotatePoint(&pt2,180+ang1);
                        else
                        {
                            xCoord = 0;
                            xLen2 = 2*radi2*(cos((90-(ang2/2.0))*PI/180));
                            xCoord = xLen2/2.0 + (xLen2-xLen)/2.0;
                            xCoord += xLen/2.0;
                            pt2 = QPointF(xCoord,-cos((90-ang2/2.0)*PI/180)*xLen2);
                            rotatePoint(&pt2,180+ang1/2.0);
                            pt2.setX(-xCoord + xLen/2.0);
                            pt2.setY(pt2.y());
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
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                    mi->setSecondRadius(radi2);

                    break;
                case J3:

                    if (ang1==0 || ang2==0)
                    {
                        showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                        continue;
                    }


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



                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, yHeight-8, t,*this->productLines->find(*this->currentProductLine));
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                    break;

                case J4:
                case X1:


                    if (ang1==0)
                    {
                        showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                        continue;
                    }

                    if (t==J4)
                        xLen = 2*rad;//*(cos((90-(ang1/2.0))*PI/180));
                    else
                        xLen = 2*rad;

                    pt1 = QPointF(-xLen/2.0,0);
                    pt2 = QPointF(xLen/2.0,0);
                    pt3 = QPointF(-xLen/2.0,0);
                    pt4 = QPointF(xLen/2.0,0);
                    /*pt1 = QPointF(-rad,0);
                    pt2 = QPointF(rad,0);
                    pt3 = QPointF(-rad,0);
                    pt4 = QPointF(rad,0);*/
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

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, rad, 0, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                    break;

                case J5:

                    if (radi2==0)
                    {
                        showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                        continue;
                    }

                    pt1 = QPointF(-rad,-radi2);
                    pt2 = QPointF(rad,-radi2);
                    pt3 = QPointF(-rad,radi2);
                    pt4 = QPointF(rad,radi2);

                    endPoints.push_back(pt1);
                    endPoints.push_back(pt2);
                    endPoints.push_back(pt3);
                    endPoints.push_back(pt4);

                    angles.push_back(0);
                    angles.push_back(0);
                    angles.push_back(0);
                    angles.push_back(0);


                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, 2*rad, radi2, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }

                    mi->setSecondRadius(radi2);

                    break;
                case C2:
                case HC:
                case HE:
                case HS:
                case CB:
                case SC:
                case JM:
                case X2:
                    showDBError(QString("Type of part %1 is not supported. Part will not be added in the database").arg(partNo),QString("Typ dílu %1 není podporován. Díl nebude přidán do databáze.").arg(partNo),lang);
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
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }


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

                    /*
                     *this generates points in this way
                     0  1
                     2  3
                     4  5
                     .. ..
                    */
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
                    xLen2=0;
                    for (int i = 0; i < (t-8);i++)
                    {
                        endPoints.push_back(endPoints.at(i));
                        endPoints.push_back(endPoints.at(i+t-8));
                    }
                    for (int i = 0; i < 2*(t-8);i++)
                        endPoints.removeAt(0);

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,rad, 2*rad, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }

                    break;
                default:

                    showDBError(QString("Item type %1 is not supported.").arg(typeStr),QString("Typ dílu %1 není podporován").arg(typeStr), lang);
                    //app->getAppData()->getMessageDialog()->exec();
                    break;



                }

            }
            else
            {

                trackGaugeHalf = abs(rad-radi2)/2.0;


                QString radius2Str = in.readLine();
                radius2Str.remove(0,8);
                QString fstLaneStr = in.readLine();
                fstLaneStr.remove(0,10);
                QString laneDistStr = in.readLine();
                laneDistStr.remove(0,9);
                QString laneDistEndStr=laneDistStr;
                if (t==HS || t==HE || t==HC)
                {
                    laneDistEndStr = in.readLine();
                    laneDistEndStr.remove(0,12);
                }
                else if (t==JM)
                {
                    laneDistEndStr = in.readLine();
                    laneDistEndStr.remove(0,9);
                }

                QString numOfLanesStr = in.readLine();
                numOfLanesStr.remove(0,9);

                QString latAngleStr("0");
                if (t==CB)
                {
                    latAngleStr = in.readLine();
                    latAngleStr.remove(0,13);
                }

                qreal radi2sl = radius2Str.toDouble();
                qreal fstLane = fstLaneStr.toDouble();
                qreal laneDist = laneDistStr.toDouble();
                qreal laneDistEnd = laneDistEndStr.toDouble();
                qreal numOfLanes = numOfLanesStr.toDouble();

                qreal lateralAngle = latAngleStr.toDouble();


                bool error = false;
                if (t==C1 || t==C2 || t==CB)
                {
                    qreal itemWidth = abs(rad-radi2sl);
                    if (itemWidth<((numOfLanes-1)*laneDist+fstLane))
                        error = true;
                }
                else
                {
                    qreal itemWidth = abs(2*rad);
                    if (itemWidth<((numOfLanes-1)*laneDist+fstLane))
                        error = true;
                }
                if (error)
                {
                    logFile << "Wrong data in database: itemWidth is smaller than n*laneDist" << endl;
                    QMessageBox * messageDialog = new QMessageBox();

                    if (lang.startsWith("EN"))
                        messageDialog->setText(QString("Database contains wrong data: the width of item %1 is smaller than the width needed for the lanes. Item will not be loaded.").arg(partNo));
                    else
                        messageDialog->setText(QString("Databáze obsahuje chybná data: šířka dílu %1 je menší než šířka potřebná pro všechny dráhy. Díl nebude načten.").arg(partNo));
                    messageDialog->setIcon(QMessageBox::Warning);
                    messageDialog->setButtonText(0,"OK");
                    messageDialog->exec();
                    continue;
                }

                if (rad==0 || radi2sl==0 || laneDist==0 || numOfLanes==0)
                {
                    showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                    continue;
                }


                if (t==C1 || t==C2 || t==CB)
                {

                    if (ang1==0)
                    {
                        showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                        continue;
                    }

                    //compute dimensions of item
                    qreal itemRadius = rad;
                    xLen = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                    yHeight = (rad-rad*(sin((90-(ang1/2.0))*PI/180)));
                    yHeight -= trackGaugeHalf;

                    rad -=fstLane;
                    for (unsigned int i = 0; i < numOfLanes; i++)
                    {
                        qreal ptX;// = 2*rad*(cos((90-(ang1/2.0))*PI/180));
                        qreal ptY;
                        qreal phi = ang1/2.0;

                        toCartesian(rad,phi,ptX,ptY);

                        //ptY = i*laneDist+fstLane;

                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(-ptX,ptY));
                        endPoints.push_back(QPointF(ptX,-ptY+itemRadius));
                        //endPoints.push_back(QPointF(ptX,ptY));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=laneDist;
                    }


                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }

                    if (t==CB)
                    {
                        //updateEndP... works with slotTrackInfo -> this itemType needs to have STI set sooner than other items

                        SlotTrackInfo * sti = new SlotTrackInfo(mi,numOfLanes,laneDist,laneDistEnd,fstLane);//memory leak in assignment? (later - not at this line)
                        mi->setSlotTrackInfo(sti);

                        mi->updateEndPointsHeightGraphics();
                        //global variable "app" is null at this moment and updateEnd...
                        //uses it, so following call cant be used:
                        //mi->setLateralAngle(lateralAngle);


                        QPointF point2(abs(mi->getRadius())-fstLane,0);
                        QPointF point(abs(radi2sl),0);

                        rotatePoint(&point,lateralAngle);
                        int innerRadiusClimb = (int)point.y();


                        point=point2;

                        for (unsigned int i = 0; i < 2*numOfLanes; i+=2)
                        {
                            rotatePoint(&point,lateralAngle);

                            mi->adjustHeightProfile((int)point.y()-innerRadiusClimb,mi->getEndPoint(i),false);
                            mi->adjustHeightProfile((int)point.y()-innerRadiusClimb,mi->getEndPoint(i+1),false);

                            point2.setX(point2.x()-laneDist);
                            point=point2;
                        }

                    }

                }
                else if (t==S1 || t==SC || t==X2 || t==JM || t==J5 || t==J1 || t==J2)
                {
                    qreal itemRadius = rad;
                    xLen = 2*radi2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    qreal ptX = radi2sl;

                    rad -=fstLane;
                    for (unsigned int i = 0; i < numOfLanes; i++)
                    {
                        qreal ptY = rad;
                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        if (t==JM)
                            endPoints.push_back(QPointF(ptX+laneDistEnd,-ptY+itemRadius));
                        else
                            endPoints.push_back(QPointF(ptX,-ptY+itemRadius));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=laneDist;
                    }
                    if (t==J1)
                    {
                       endPoints.push_back(QPointF(ptX,endPoints.at(2*numOfLanes-1).y()+laneDist));
                       angles.push_back(ang1/2.0);
                    }
                    else if (t==J2)
                    {
                        endPoints.push_back(QPointF(ptX,endPoints.at(0).y()-laneDist));
                        angles.push_back(ang1/2.0);
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    mi->setSecondRadius(radi2sl);
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                }
                else if (t==X1)
                {
                    ///
                    qreal itemRadius = rad;
                    xLen = 2*radi2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    rad -=fstLane;
                    for (unsigned int i = 0; i < numOfLanes; i++)
                    {
                        qreal ptX = radi2sl;
                        qreal ptY = rad;


                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        endPoints.push_back(QPointF(ptX,-ptY+itemRadius));
                        angles.push_back(0);
                        angles.push_back(0);
                        rad-=laneDist;
                    }
                    rad = fstLane;
                    for (unsigned int i = 0; i < numOfLanes; i++)
                    {
                        qreal ptX = rad;
                        endPoints.push_back(QPointF(ptX,-fstLane));
                        endPoints.push_back(QPointF(ptX,2*itemRadius-fstLane));
                        angles.push_back(-90);
                        angles.push_back(90);
                        rad+=laneDist;
                    }


                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                }
                else if (t==HS)
                {
                    qreal itemRadius = rad;

                    xLen = 2*radi2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    rad -=fstLane;
                    qreal ptY2 = rad-(laneDist-laneDistEnd);
                    for (unsigned int i = 0; i < numOfLanes; i++)
                    {
                        qreal ptX = radi2sl;
                        qreal ptY = rad;


                        endPoints.push_back(QPointF(-ptX,-ptY+itemRadius));
                        endPoints.push_back(QPointF(ptX,-ptY2+itemRadius));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=laneDist;
                        ptY2-=laneDistEnd;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    mi->setSecondRadius(radi2sl);
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                }
                else if (t==HE)
                {
                    qreal itemRadius = rad;

                    xLen = 2*radi2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;

                    rad -=fstLane;
                    qreal ptY2 = rad-(laneDist-laneDistEnd);
                    for (unsigned int i = 0; i < numOfLanes; i++)
                    {
                        qreal ptX = radi2sl;
                        qreal ptY = rad;


                        endPoints.push_back(QPointF(-ptX,-ptY2+itemRadius));
                        endPoints.push_back(QPointF(ptX,-ptY+itemRadius));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);
                        rad-=laneDist;
                        ptY2-=laneDistEnd;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    mi->setSecondRadius(radi2sl);
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                }
                else if (t==HC)
                {
                    qreal itemRadius = rad;

                    xLen = 2*radi2sl;
                    yHeight = rad;
                    yHeight -= trackGaugeHalf;


                    qreal ptX = radi2sl;
                    qreal ptY = fstLane;
                    qreal ptYOther = rad-(laneDistEnd*(numOfLanes/2-0.5));

                    for (unsigned int i = 0; i < numOfLanes; i++)
                    {
                        endPoints.push_back(QPointF(-ptX,ptY));
                        endPoints.push_back(QPointF(ptX,ptYOther));

                        angles.push_back(-ang1/2.0);
                        angles.push_back(ang1/2.0);

                        ptY += laneDist;
                        ptYOther += laneDistEnd;
                    }

                    mi = new ModelItem(partNo,nameEn,nameCs,endPoints,angles,itemRadius, xLen, yHeight, t,*this->productLines->find(*this->currentProductLine));//parentWidget??
                    mi->setSecondRadius(radi2sl);
                    if ((*this->productLines->find(*this->currentProductLine))->addItem(mi)!=0)
                    {
                        showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                        delete mi;
                        continue;
                    }
                }
                else
                {
                    app->getAppData()->setMessageDialogText(QString("Item type %1 is not supported.").arg(typeStr),QString("Typ dílu %1 není podporován").arg(typeStr));
                    app->getAppData()->getMessageDialog()->exec();
                }
                SlotTrackInfo * sti = new SlotTrackInfo(mi,numOfLanes,laneDist,laneDistEnd,fstLane);//memory leak in assignment? (later - not at this line)

                mi->setSecondRadius(radi2sl);
                mi->setSlotTrackInfo(sti);
            }
        }

        else if ((!line.isNull() || line.startsWith("[")) && loadMode==1)
        {
            QString partNo = line;
            partNo = partNo.replace("[","");
            partNo = partNo.replace("]","");

            QString nameEn = in.readLine();
            QString nameCs = in.readLine();
            nameEn=nameEn.remove(0,7);
            nameCs=nameCs.remove(0,7);

            QString angle1 = in.readLine();
            angle1.remove(0,6);
            qreal ang1 = angle1.toDouble();


            QString radius = in.readLine();
            radius.remove(0,7);
            qreal rad = radius.toDouble();

            bool innerBorder = false;
            bool endingBorderFlag = false;

            QString borderType = in.readLine();
            if (ang1>2)
            {
                innerBorder = borderType.contains("1");
            }
            else
            {
                endingBorderFlag = borderType.contains("1");
            }

            QList<QPointF> endPoints;

            BorderItem * bi = NULL;

            if (rad==0)
            {
                showDBError(QString("PartNo %1 is specified in the wrong way. Part will not be added in the database").arg(partNo),QString("Díl %1 je specifikován špatným způsobem. Díl nebude přidán do databáze.").arg(partNo),lang);
                continue;
            }



            if (ang1>2)
            {

                int n = ang1/22.5;
                qreal angle = -ang1/2+22.5/2;

                for (int i = 0; i < n; i++)
                {
                    QPointF pt(0,-rad);
                    rotatePoint(&pt,-angle);
                    pt.setY(pt.y()+rad);
                    endPoints << pt;
                    angle+=22.5;
                }

                bi = new BorderItem(partNo,nameEn,nameCs,ang1,rad, endPoints, innerBorder,*this->productLines->find(*this->currentProductLine));//parentWidget??
                if ((*this->productLines->find(*this->currentProductLine))->addItem(bi)!=0)
                {
                    showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                    delete bi;
                    continue;
                }
            }
            else
            {

                qreal stdStraight = (*this->productLines->find(*this->currentProductLine))->getMaxStraight();

                ///qreal n = (rad)/stdStraight+1;
                int n = (rad)/stdStraight+1;
                qreal xCoord = -abs(((int)n-1)*(stdStraight/2));
                for (int i = 0; i < n; i++)
                {
                    endPoints.push_back(QPointF(xCoord,0));
                    xCoord+=stdStraight;
                }

                if (endPoints.empty())
                {
                    logFile << "empty endPoints list for borderItem " << partNo.toStdString() << endl;
                    QMessageBox * messageDialog = new QMessageBox();

                    if (lang.startsWith("EN"))
                        messageDialog->setText(QString("No end-points were generated for track border item No: %1. Please move its specification in the parts database. It has to placed after the specification of the longest straight track item.").arg(partNo));
                    else
                        messageDialog->setText(QString("Pro díl %1 nebyly vygenerovány žádné koncové body. Prosím upravte databázi dílů. Specifikace rovného mantinelu musí být umístěna až za specifikací nejdelšího rovného dílu trati.").arg(partNo));
                    messageDialog->setIcon(QMessageBox::Warning);
                    messageDialog->setButtonText(0,"OK");
                    messageDialog->exec();
                    continue;
                }

                bi = new BorderItem(partNo,nameEn,nameCs,0,rad,endPoints,endingBorderFlag,*this->productLines->find(*this->currentProductLine));//parentWidget??
                if ((*this->productLines->find(*this->currentProductLine))->addItem(bi)!=0)
                {
                    showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                    delete bi;
                    continue;
                }
            }

        }

        else if ((!line.isNull() || line.startsWith("[")) && loadMode==2)
        {
            QString partNo = line; //in.readLine();
            partNo = partNo.replace("[","");
            partNo = partNo.replace("]","");

            QString nameEn = in.readLine();
            QString nameCs = in.readLine();
            nameEn=nameEn.remove(0,7);
            nameCs=nameCs.remove(0,7);

            QString season = in.readLine();
            season.remove(0,7);

            QString widthStr = in.readLine();
            widthStr.remove(0,6);
            qreal width = widthStr.toDouble();

            QString heightStr = in.readLine();
            heightStr.remove(0,7);
            qreal height = heightStr.toDouble();

            VegetationItem * item = new VegetationItem(partNo,nameEn,nameCs,season,width,height,*this->productLines->find(*this->currentProductLine));
            if ((*this->productLines->find(*this->currentProductLine))->addItem(item)!=0)
            {
                showDBError(QString("The database already contains part %1. It won't be added for the second time.").arg(partNo),QString("Databáze již obsahuje díl %1. Díl nebude znovu přidán.").arg(partNo),lang);
                delete item;
                continue;
            }

        }
    }

    logFile << "whole file was read"  << endl;

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
        QList<VegetationItem*>::Iterator itemIterV = (*iter1)->getVegetationItemsList()->begin();
        while(itemIterV!=(*iter1)->getVegetationItemsList()->end())
        {
            (*itemIterV)->generate2DModel(true);
            (*itemIterV)->get2DModel()->setFlag(QGraphicsItem::ItemIsMovable,false);
            (*itemIterV)->generate2DModel(false);

            itemIterV++;
        }

        if (!(*iter1)->getType())
        {
            QList<BorderItem*>::Iterator itemIter = (*iter1)->getBorderItemsList()->begin();
            while(itemIter!=(*iter1)->getBorderItemsList()->end())
            {
                (*itemIter)->generate2DModel(true);
                (*itemIter)->get2DModel()->setFlag(QGraphicsItem::ItemIsMovable,false);


                (*itemIter)->generate2DModel(false);

                itemIter++;
            }
        }

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
        text = text.toUtf8();

        QList<ModelItem*>::Iterator itemIter = (*iter)->getItemsList()->begin();
        int currentMovement = 0;
        int i = 0;
        while(itemIter!=(*iter)->getItemsList()->end())
        {
            (*itemIter)->get2DModel()->moveBy(0,currentMovement);            
            scene->addItem((*itemIter)->get2DModel());
            currentMovement+=(*itemIter)->get2DModel()->boundingRect().size().height()+48;

            itemIter++;
            i++;
        }
        QList<VegetationItem*>::Iterator itemIterV = (*iter)->getVegetationItemsList()->begin();
        while(itemIterV!=(*iter)->getVegetationItemsList()->end())
        {
            (*itemIterV)->get2DModel()->moveBy(0,currentMovement);            
            scene->addItem((*itemIterV)->get2DModel());            
            currentMovement+=(*itemIterV)->get2DModel()->boundingRect().size().height()+48;

            itemIterV++;
            i++;
        }

        currentMovement+=50;

        if (!(*iter)->getType())
        {
            QList<BorderItem*>::Iterator itemIter = (*iter)->getBorderItemsList()->begin();
            while(itemIter!=(*iter)->getBorderItemsList()->end())
            {
                (*itemIter)->get2DModel()->moveBy(0,currentMovement);
                scene->addItem((*itemIter)->get2DModel());
                currentMovement+=(*itemIter)->get2DModel()->boundingRect().size().height()+48;

                itemIter++;
                i++;
            }
        }
        this->scenes->insert(text,scene);
        iter++;
    }

    dbFile->close();

    delete dbFile;

}

Database::~Database()
{
    QList<QString> pLKeys = this->productLines->keys();
    QList<QString> scKeys = this->scenes->keys();
    for (int i = 0; i < this->productLines->count(); i++)
    {
        delete this->productLines->value(pLKeys.at(i));

        QGraphicsScene * sc = this->scenes->value(scKeys.at(i));

        QList<QGraphicsItem*> itList = sc->items();
        for (int j = 0; j < itList.count(); j++)
        {

            if (itList[j]->type()>=QGraphicsItem::UserType)
                sc->removeItem(itList[j]);
        }

        delete sc;
    }
    delete this->productLines;
    delete this->scenes;
    delete this->currentItem;
    //this cant be deleted - it is a pointer to memory freed by ~ProductLine()
    //delete this->currentProductLine;

}



ProductLine * Database::findProductLineByName(QString &name) const
{
    QMap<QString,ProductLine*>::Iterator it = this->productLines->find(name);
    if (it!=this->productLines->end())
        return *it;
    return NULL;
}

int Database::setCurrentProductLine(QString &name)
{
    if (name == "")
        return 1;

    ProductLine * pl=*this->productLines->find(name);
    if (pl==NULL)
    {
        //*this->currentProductLine="";
        this->currentProductLine=NULL;
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

QMap<QString,ProductLine*>::Iterator Database::getDatabaseEndIterator()
{
    return this->productLines->end();
}

ModelItem *Database::findModelItemByName(QString &manufactName, QString &partName) const
{
    ProductLine * pl = this->findProductLineByName(manufactName);
    if (pl==NULL)
        return NULL;

    for (int i = 0; i < pl->getItemsList()->count(); i++)
    {
        if (*pl->getItemsList()->at(i)->getPartNo()==partName)
            return pl->getItemsList()->at(i);
    }
    return NULL;
}

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


QGraphicsScene *Database::findSceneByString(QString &scaleAndName) const
{
    if (scaleAndName=="")
        return NULL;
    QMap<QString,QGraphicsScene*>::Iterator it = this->scenes->find(scaleAndName.toUtf8());
    if (it==this->scenes->end())
        return NULL;
    else
        return *it;

}


BorderItem *Database::findBorderItemByName(QString &manufactName, QString &partName) const
{
    ProductLine * pl = this->findProductLineByName(manufactName);
    if (pl==NULL)
        return NULL;

    for (int i = 0; i < pl->getBorderItemsList()->count(); i++)
    {
        if (*pl->getBorderItemsList()->at(i)->getPartNo()==partName)
            return pl->getBorderItemsList()->at(i);
    }
    return NULL;
}

VegetationItem *Database::findVegetationItemByName(QString &manufactName, QString &partName) const
{
    ProductLine * pl = this->findProductLineByName(manufactName);
    if (pl==NULL)
        return NULL;

    for (int i = 0; i < pl->getVegetationItemsList()->count(); i++)
    {
        if (*pl->getVegetationItemsList()->at(i)->getPartNo()==partName)
            return pl->getVegetationItemsList()->at(i);
    }
    return NULL;
}
