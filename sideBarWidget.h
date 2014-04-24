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

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include "includeHeaders.h"
#include <QDialog>
#include <QListWidget>

#include "database.h"

class Database;

class SideBarWidget : public QFrame//QWidget
{
    Q_OBJECT

    QGridLayout * layout;
    QPushButton * addButton;
    QPushButton * delButton;
    QComboBox * prodLineCBox;
    Database * database;
    QMenu * contextMenu;
    QGraphicsScene * currentScene;
    QGraphicsView * graphicsView;

    QDialog * addDialog;
    QDialog * delDialog;
    QListWidget * listWidget;
    QListWidget * listWidgetD;

public:

    SideBarWidget(QString * lang, Database * db, QMenu * context, QWidget * parent = 0);
    Database * getDatabase() const;

    QDialog * initAddDialog();
    QDialog * initDelDialog();


    QGraphicsScene * getCurrentScene() const;

    QComboBox *getProductLines();
    void resetSideBar();

    int setInventoryState(QTextStream & input);
    void printInventoryState(QTextStream & output);

protected:
    void paintEvent(QPaintEvent * evt);
    void contextMenuEvent(QContextMenuEvent * evt);
    void mousePressEvent(QMouseEvent * evt) ;
    void mouseMoveEvent(QMouseEvent * evt);

public slots:
    void showAddDialog();
    void showDelDialog();
    void closeAddDialog();
    void closeDelDialog();
    void importSelection();
    void deleteSelection();

};

#endif // SIDEBARWIDGET_H
