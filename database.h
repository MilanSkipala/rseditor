/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

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

#ifndef DATABASE_H
#define DATABASE_H

#include <QFile>
#include <QTextStream>
#include "includeHeaders.h"
#include "partsRelated.h"

class Database
{
    QMap<QString,ProductLine*> * productLines;
    QMap<QString,QGraphicsScene*> * scenes;
    QString *currentProductLine;
    QString *currentItem;
public:
    Database(QString &lang);

    ProductLine * findProductLineByName(QString &name) const;

    int setCurrentProductLine(QString &name);
    QString * getCurrentProductLinesName();
    ProductLine * getCurrentProductLine();
    QMap<QString,ProductLine*>::Iterator getDatabaseIterator();
    QMap<QString,ProductLine*>::Iterator getDatabaseEndIterator();

    ModelItem * findModelItemByName(QString &manufactName, QString &partName) const;

    ModelItem * getNextModelItem(QString &manufactName) const;

    BorderItem * findBorderItemByName(QString &manufactName, QString &partName) const;
    VegetationItem * findVegetationItemByName(QString &manufactName, QString &partName) const;

    QGraphicsScene * findSceneByString(QString &scaleAndname) const;

    int addProductLine(ProductLine * manufact);
    int size();
    //int addModelItem(QString manufactName, ModelItem * itemToAdd);//ProductLine's name
    //int addModelItem(ModelItem *itemToAdd);


};

#endif // DATABASE_H
