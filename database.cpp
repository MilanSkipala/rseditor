#include <QtGlobal>
#include "globalVariables.h"
#include "database.h"

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

            skip(line,in);

            //get gauge
            //line = in.readLine();
            QString gauge = line.remove(0,6);

            skip(line,in);



            /**
              TODO
              -what causes rubbish in the db file?
            */

            ProductLine * productLine = new ProductLine(name,scale, gauge, type);
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

            qreal x1, y1, x2, y2;

            for (int i = 0; i < 4; i++)
            {
                QString line = in.readLine();
                line.remove(0,3);
                if (i==0)
                    x1 = line.toDouble();
                else if (i==1)
                    y1 = line.toDouble();
                else if (i==2)
                    x2 = line.toDouble();
                else if (i==3)
                    y2 = line.toDouble();
            }
            QPoint start = QPoint(x1,y1);
            QPoint end = QPoint(x2,y2);

            QString degree = in.readLine();
            degree.remove(0,7);
            qreal deg = degree.toDouble();

            QString radius = in.readLine();
            radius.remove(0,7);
            qreal rad = radius.toDouble();

            ModelItem * mi = new ModelItem(partNo,nameEn,nameCs,start,end,deg,rad, *this->productLines->find(*this->currentProductLine));//parentWidget??
            (*this->productLines->find(*this->currentProductLine))->addItem(mi);

        }


    }

    //generate 2D and 3D models
    QMap<QString,ProductLine*>::Iterator iter1 = this->productLines->begin();
    while (iter1!=this->productLines->end())
    {
        QList<ModelItem*>::Iterator itemIter = (*iter1)->getItemsList()->begin();
        while(itemIter!=(*iter1)->getItemsList()->end())
        {
            (*itemIter)->generate2DModel();
            (*itemIter)->get2DModel()->setFlag(QGraphicsItem::ItemIsMovable,false);
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

/* load path items from modelItems instead of this
        QBrush b = scene->backgroundBrush();
        b.setColor(Qt::red);
        b.setStyle(Qt::SolidPattern);
        scene->setBackgroundBrush(b);

        QPainterPath * pPath = new QPainterPath();

        //pPath->moveTo(25,25);

        pPath->arcMoveTo(QRectF(-30,-30,60,60),0);
        pPath->arcTo(QRectF(-30,-30,60,60),0,245);
        pPath->arcMoveTo(QRectF(-20,-20,40,40),0);
        pPath->arcTo(QRectF(-20,-20,40,40),0,245);
        GraphicsPathItem * gPItem = new GraphicsPathItem(*pPath);
        b.setColor(Qt::yellow);
        gPItem->setBrush(b);
        scene->addItem(gPItem);

/**/



        QList<ModelItem*>::Iterator itemIter = (*iter)->getItemsList()->begin();
        int i = 0;
        while(itemIter!=(*iter)->getItemsList()->end())
        {

            (*itemIter)->get2DModel()->moveBy(0,i*(64)+(*itemIter)->getRadius()); //WARNING - 64 is the variable sizeOfItem
            scene->addItem((*itemIter)->get2DModel());
            itemIter++;
            i++;


        }
        this->scenes->insert(text,scene);
        iter++;
    }

    QString x("ahoj");
    x.append(" svete!");


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
