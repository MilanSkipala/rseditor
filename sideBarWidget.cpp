#include <QScrollBar>
#include "sideBarWidget.h"

SideBarWidget::SideBarWidget(Database * db, QMenu * context, QWidget * parent) : QFrame(parent, parent->windowFlags())//QWidget(parent, parent->windowFlags())
{
    this->database=db;

    /**
      TODO
      -custom context menu
      -remove "context" param from constructor
    */
    this->contextMenu=context;
    this->layout = new QGridLayout(this);

    this->prodLineCBox=new QComboBox(this);
    this->graphicsView = new QGraphicsView(this);
    this->graphicsView->setRenderHint(QPainter::Antialiasing);
    this->graphicsView->setAlignment(Qt::AlignTop);

    this->currentScene=NULL;

    this->addButton = new QPushButton("Add",this);
    this->delButton = new QPushButton("Del",this);

    this->addButton->setMinimumWidth(25);
    this->delButton->setMinimumWidth(25);
    this->addButton->setMaximumWidth(50);
    this->delButton->setMaximumWidth(50);

    /*
      -following should result in better looking SBW
    */
    this->prodLineCBox->setMinimumWidth(80);
    this->prodLineCBox->setMaximumWidth(190);
    this->layout->setSizeConstraint(QLayout::SetMaximumSize);

    this->graphicsView->setMinimumWidth(130);



    QPalette pal = this->palette();
    pal.setColor(QPalette::Window,Qt::white);
    this->setPalette(pal);
    this->setAutoFillBackground(true);

    this->layout->addWidget(this->prodLineCBox,0,0,1,2);//,0,0,3,3);
    this->layout->addWidget(this->addButton,1,0);//,0,1,2,1);
    this->layout->addWidget(this->delButton,1,1);//,0,2,1,1);
    this->layout->addWidget(this->graphicsView,2,0,3,2);

    this->setLayout(this->layout);

    this->setGeometry(0,0,215,420);

    connect(this->addButton, SIGNAL(clicked()),this,SLOT(showAddDialog()));
    connect( this->delButton, SIGNAL(clicked()),this,SLOT(showDelDialog()));

    //initialize add and del dialog
    this->addDialog = this->initAddDialog();
    this->delDialog = this->initDelDialog();


}

void SideBarWidget::contextMenuEvent(QContextMenuEvent *evt)
{
    this->contextMenu->popup(evt->globalPos());
}
void SideBarWidget::mousePressEvent(QMouseEvent *evt)
{
    QFrame::mousePressEvent(evt);
    //this->graphicsView->event(evt);

    /**
    TODO
    */
}
void SideBarWidget::mouseMoveEvent(QMouseEvent *evt)
{
    QFrame::mouseMoveEvent(evt);
    /**
    TODO
    */
}
void SideBarWidget::paintEvent(QPaintEvent *evt)
{
    QFrame::paintEvent(evt);

    //ProductLine * pl = this->database->findProductLineByName(this->prodLineCBox->currentText());
    QGraphicsScene * sceneBefore = this->currentScene;
    QString text = this->prodLineCBox->currentText();
    if (this->prodLineCBox->currentText()!="")
        this->currentScene = this->database->findSceneByString(text);




    this->graphicsView->setScene(this->currentScene);

    if (this->currentScene!=NULL)
    {
        QTransform tr;//(this->graphicsView->transform());
        qreal sceneScale = 1;
        sceneScale = 110/this->currentScene->width();
        tr.scale(sceneScale,sceneScale);
        //if (tr.m11()>=sceneScale || this->graphicsView->scene()!=sceneBefore)
        //if (tr.m11()>=sceneScale)
        this->graphicsView->setTransform(tr);
    }

    if (sceneBefore!=this->currentScene)
    {
        this->graphicsView->verticalScrollBar()->setValue(this->graphicsView->verticalScrollBar()->minimum());
    }



//*/


}

void SideBarWidget::showAddDialog()
{
    this->addDialog->exec();
}

void SideBarWidget::showDelDialog()
{
    this->delDialog->exec();
}

void SideBarWidget::closeAddDialog()
{
    this->addDialog->close();
}

void SideBarWidget::closeDelDialog()
{
    this->delDialog->close();
}

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
    connect(pb2,SIGNAL(clicked()),this,SLOT(closeDelDialog()));
    connect(pb,SIGNAL(clicked()),this,SLOT(deleteSelection()));

    l->addWidget(pb);
    l->addWidget(pb2);
    d->setLayout(l);
    return d;
    return NULL;
}

QGraphicsScene *SideBarWidget::getCurrentScene() const
{
    return this->currentScene;
}

QComboBox *SideBarWidget::getProductLines()
{
    return this->prodLineCBox;
}
