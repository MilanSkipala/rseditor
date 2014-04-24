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

#include <QScrollBar>
#include "sideBarWidget.h"
#include "globalVariables.h"

SideBarWidget::SideBarWidget(QString * lang, Database * db, QMenu * context, QWidget * parent): QFrame(parent, parent->windowFlags())//QWidget(parent, parent->windowFlags())
{
    this->database=db;

    this->contextMenu=context;
    this->layout = new QGridLayout(this);
    this->layout->setSizeConstraint(QLayout::SetNoConstraint);

    this->prodLineCBox=new QComboBox(this);
    this->graphicsView = new QGraphicsView(this);
    this->graphicsView->setRenderHint(QPainter::Antialiasing);
    this->graphicsView->setAlignment(Qt::AlignHCenter);

    this->currentScene=NULL;


    this->addButton = new QPushButton("Add",this);
    this->delButton = new QPushButton("Del",this);

    if (lang->startsWith("EN"))
    {
        this->addButton->setStatusTip("Add items from selected manufacturers");
        this->delButton->setStatusTip("Remove manufacturer from the list");

    }
    else
    {
        this->addButton->setStatusTip("Přidat díly od vybraných výrobců");
        this->delButton->setStatusTip("Odstranit výrobce ze seznamu");
        this->addButton->setText("Přidat");
        this->delButton->setText("Odebrat");
    }

    /*
    this->addButton->setMinimumWidth(25);
    this->delButton->setMinimumWidth(25);
    this->addButton->setMaximumWidth(50);
    this->delButton->setMaximumWidth(50);

    / *
      -following should result in better looking SBW
    */

    /**
    this->prodLineCBox->setMinimumWidth(80);
    this->prodLineCBox->setMaximumWidth(390);
    this->layout->setSizeConstraint(QLayout::SetMaximumSize);

    this->graphicsView->setMinimumWidth(130);
*/


    QPalette pal = this->palette();
    pal.setColor(QPalette::Window,Qt::white);
    this->setPalette(pal);
    this->setAutoFillBackground(true);

    this->layout->addWidget(this->prodLineCBox,0,0,1,2);//,0,0,3,3);
    this->layout->addWidget(this->addButton,1,0);//,0,1,2,1);
    this->layout->addWidget(this->delButton,1,1);//,0,2,1,1);
    this->layout->addWidget(this->graphicsView,2,0,3,2);

    this->setLayout(this->layout);


    //this->setGeometry(0,0,215,420);

    connect(this->addButton, SIGNAL(clicked()),this,SLOT(showAddDialog()));
    connect( this->delButton, SIGNAL(clicked()),this,SLOT(showDelDialog()));

    //initialize add and del dialog
    this->addDialog = this->initAddDialog();
    this->delDialog = this->initDelDialog();

    if (lang->startsWith("CS"))
    {
        this->addDialog->setWindowTitle("Přidat výrobce");
        this->delDialog->setWindowTitle("Odebrat výrobce");
    }





}

void SideBarWidget::contextMenuEvent(QContextMenuEvent *evt)
{
    this->contextMenu->popup(evt->globalPos());
}
void SideBarWidget::mousePressEvent(QMouseEvent *evt)
{
    QFrame::mousePressEvent(evt);
    //this->graphicsView->event(evt);
}
void SideBarWidget::mouseMoveEvent(QMouseEvent *evt)
{
    QFrame::mouseMoveEvent(evt);
}
void SideBarWidget::paintEvent(QPaintEvent *evt)
{
    QFrame::paintEvent(evt);

    //ProductLine * pl = this->database->findProductLineByName(this->prodLineCBox->currentText());
    QGraphicsScene * sceneBefore = this->currentScene;
    QString text = this->prodLineCBox->currentText();
    if (this->prodLineCBox->currentText()!="")
    {
        this->currentScene = this->database->findSceneByString(text);
        if (this->currentScene==NULL)
        {
            logFile << "ERROR: currentScene==NULL" << endl;
            return;
        }
    }

    this->graphicsView->setScene(this->currentScene);

    if (this->currentScene!=NULL)
    {
        QTransform tr;//(this->graphicsView->transform());
        qreal sceneScale = 1;
        //subtract the width of the scrollbar
        sceneScale = (this->graphicsView->width()-16)/this->currentScene->width();
        tr.scale(sceneScale,sceneScale);
        //if (tr.m11()>=sceneScale || this->graphicsView->scene()!=sceneBefore)
        //if (tr.m11()>=sceneScale)
        this->graphicsView->setTransform(tr);
        //cout << sceneScale << endl;
    }

    if (sceneBefore!=this->currentScene)
    {
        if (sceneBefore!=NULL)
            sceneBefore->clearSelection();
        this->graphicsView->verticalScrollBar()->setValue(this->graphicsView->verticalScrollBar()->minimum());
    }

}

void SideBarWidget::showAddDialog()
{this->addDialog->exec();}

void SideBarWidget::showDelDialog()
{this->delDialog->exec();}

void SideBarWidget::closeAddDialog()
{this->addDialog->close();}

void SideBarWidget::closeDelDialog()
{this->delDialog->close();}

void SideBarWidget::importSelection()
{
    QList<QListWidgetItem*> list = this->listWidget->selectedItems();

    while(!list.empty())
    {
        QListWidgetItem * ptr = list.front();
        QString text = ptr->text();
        bool duplicate = false;
        for (int i = 0; i < this->prodLineCBox->count(); i++)
        {
            if (this->prodLineCBox->itemText(i)==text)
                duplicate=true;
        }
        if (!duplicate)
            this->prodLineCBox->addItem(text);
        list.pop_front();
    }

    closeAddDialog();
    if (app->getUserPreferences()->getLocale()->startsWith("EN"))
        app->getWindow()->statusBar()->showMessage("Double-click the item to place it in the workspace");
    else
        app->getWindow()->statusBar()->showMessage("Dvojitým kliknutím na dílek ho vložíte na pracovní plochu");
}

void SideBarWidget::deleteSelection()
{
    QList<QListWidgetItem*> list = this->listWidgetD->selectedItems();

    while(!list.empty())
    {
        QListWidgetItem * ptr = list.front();
        QString text = ptr->text();

        for (int i = 0; i < this->prodLineCBox->count(); i++)
        {
            if (this->prodLineCBox->itemText(i)==text)
            {
                this->prodLineCBox->removeItem(i);
                if (this->database->findSceneByString(text)==this->currentScene)
                {
                    QString str = this->prodLineCBox->itemText(i+1);
                    this->currentScene=this->database->findSceneByString(str);
                }
            }
        }
        list.pop_front();
    }
    closeDelDialog();
}


//initialize add and del dialog
QDialog * SideBarWidget::initAddDialog()
{
    QDialog * d = new QDialog(this);
    d->setWindowTitle("Add manufacturer");
    QVBoxLayout * l = new QVBoxLayout(d);

    this->listWidget = new QListWidget(d);

    listWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    QMap<QString,ProductLine*>::Iterator iter = this->database->getDatabaseIterator();
    for (int i = 0; i < this->database->size(); i++)
    {
        QString text = *(*iter)->getScale();
        text.append(" ");
        text.append(*(*iter)->getName());
        QListWidgetItem * item = new QListWidgetItem(text,this->listWidget);
        this->listWidget->addItem(item);
        iter++;
    }
    l->addWidget(this->listWidget);

    QPushButton * pb = new QPushButton("Add selected");
    QPushButton * pb2 = new QPushButton("Close");
    //if (!app->getUserPreferences()->getLocale()->startsWith("EN"))
    if (this->addButton->statusTip().startsWith("Přidat"))
    {
        pb->setText("Přidat vybrané");
        pb2->setText("Storno");
    }
    connect(pb2,SIGNAL(clicked()),this,SLOT(closeAddDialog()));
    connect(pb,SIGNAL(clicked()),this,SLOT(importSelection()));

    l->addWidget(pb);
    l->addWidget(pb2);
    d->setLayout(l);
    return d;
}



QDialog * SideBarWidget::initDelDialog()
{
    QDialog * d = new QDialog(this);
    d->setWindowTitle("Remove manufacturer");
    QVBoxLayout * l = new QVBoxLayout(d);

    this->listWidgetD = new QListWidget(d);

    listWidgetD->setSelectionMode(QAbstractItemView::MultiSelection);

    QMap<QString,ProductLine*>::Iterator iter = this->database->getDatabaseIterator();
    for (int i = 0; i < this->database->size(); i++)
    {
        QString text = *(*iter)->getScale();
        text.append(" ");
        text.append(*(*iter)->getName());
        QListWidgetItem * item = new QListWidgetItem(text,this->listWidgetD);
        this->listWidgetD->addItem(item);
        iter++;
    }
    l->addWidget(this->listWidgetD);

    QPushButton * pb = new QPushButton("Remove selected");
    QPushButton * pb2 = new QPushButton("Close");
    if (this->addButton->statusTip().startsWith("Přidat"))
    {
        pb->setText("Odstranit vybrané");
        pb2->setText("Storno");
    }
    connect(pb2,SIGNAL(clicked()),this,SLOT(closeDelDialog()));
    connect(pb,SIGNAL(clicked()),this,SLOT(deleteSelection()));

    l->addWidget(pb);
    l->addWidget(pb2);
    d->setLayout(l);
    return d;
    return NULL;
}

QGraphicsScene *SideBarWidget::getCurrentScene() const
{return this->currentScene;}

QComboBox *SideBarWidget::getProductLines()
{return this->prodLineCBox;}

void SideBarWidget::resetSideBar()
{
    this->listWidget->clearSelection();
    this->listWidgetD->clearSelection();
    if (this->currentScene!=NULL)
        this->currentScene->clearSelection();
    this->currentScene=NULL;
    this->prodLineCBox->clear();


#ifdef Q_OS_LINUX
    QString path(folderPathLinux);
#endif


#ifdef Q_OS_WIN
    QString path(folderPathWin);
#endif

    path.append("Inventory.rsi");

    QFile input(path);

    input.open(QFile::ReadOnly);

    QTextStream inputFile(&input);
    if (!input.isOpen())
    {
        logFile << "Inventory file couldn't be opened" << endl;
    }
    else
        this->setInventoryState(inputFile);
}

int SideBarWidget::setInventoryState(QTextStream &input)
{
    logFile << "Setting state of inventory" << endl;

    QString str;
    ProductLine * productLine = NULL;
    int mode = 0; //0=modelItems, 1=borderItems, 2=vegetationItems
    while (!input.atEnd())
    {
        str = input.readLine();
        if (str.startsWith("ProductLine="))
        {
            productLine = this->database->findProductLineByName(str.remove(0,12));
            if (productLine==NULL)
            {
                logFile << "    Product line was not found: " << str.toStdString() << endl;
                return 1;
            }
            mode=0;

        }
        else if (str.startsWith("---"))
            mode++;
        else if (str.startsWith("INVENTORY"))
        {}
        else
        {
            if (mode==0)
            {
                QString str2 = str.left(str.indexOf("="));
                ModelItem * it = productLine->findItemByPartNo(&str2);
                if (it==NULL)
                {
                    logFile << "    Item was not found: " << productLine->getName()->toStdString() << " " << str.toStdString() << endl;
                    return 1;
                }
                str.remove(0,str2.count()+1);
                it->setAvailableCount(str.toUInt());
                it->get2DModel()->changeCountPath(it->getAvailableCount());
            }
            else if (mode==1)
            {
                QString str2 = str.left(str.indexOf("="));
                BorderItem * it = productLine->findBorderItemByPartNo(&str2);
                if (it==NULL)
                {
                    logFile << "    Item was not found: " << productLine->getName()->toStdString() << " " << str.toStdString() << endl;
                    return 1;
                }
                str.remove(0,str2.count()+1);
                it->setAvailableCount(str.toUInt());
                it->get2DModel()->changeCountPath(it->getAvailableCount());
            }
            else
            {
                QString str2 = str.left(str.indexOf("="));
                VegetationItem * it = productLine->findVegetationItemByPartNo(&str2);
                if (it==NULL)
                {
                    logFile << "    Item was not found: " << productLine->getName()->toStdString() << " " << str.toStdString() << endl;
                    return 1;
                }
                str.remove(0,str2.count()+1);
                it->setAvailableCount(str.toUInt());
                it->get2DModel()->changeCountPath(it->getAvailableCount());
            }

        }
    }

    return 0;
}

void SideBarWidget::printInventoryState(QTextStream &output)
{
    output << "INVENTORY" << endl;

    QMap<QString,ProductLine*>::Iterator dbIter = this->database->getDatabaseIterator();
    QMap<QString,ProductLine*>::Iterator dbEndIter = this->database->getDatabaseEndIterator();


    //for (int i = 0; i < this->prodLineCBox->count(); i++)
    while (dbIter!=dbEndIter)
    {
        //QString name = this->prodLineCBox->itemText(i);
        //name.remove(0,name.indexOf(" ")+1);
        ProductLine * pl = dbIter.value();//this->database->findProductLineByName(name);
        output << "ProductLine=" << *pl->getName() << endl;
        for (int j = 0; j < pl->getItemsList()->count(); j++)
        {
            output << *pl->getItemsList()->at(j)->getPartNo() << "=" << pl->getItemsList()->at(j)->getAvailableCount() << endl;
        }
        output << "---" << endl;
        for (int j = 0; j < pl->getBorderItemsList()->count(); j++)
        {
            output << *pl->getBorderItemsList()->at(j)->getPartNo() << "=" << pl->getBorderItemsList()->at(j)->getAvailableCount() << endl;
        }
        output << "---" << endl;
        for (int j = 0; j < pl->getVegetationItemsList()->count(); j++)
        {
            output << *pl->getVegetationItemsList()->at(j)->getPartNo() << "=" << pl->getVegetationItemsList()->at(j)->getAvailableCount() << endl;
        }

        dbIter++;
    }


}
