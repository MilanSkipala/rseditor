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

#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include "includeHeaders.h"


//enum SelectionMode {};

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
    bool mouseMode; //F=select activeEP by doubleClick, T=create aEP by doubleClick
    QMenu * contextMenuItem;
    QMenu * contextMenuScene;

public:
    /*
    GraphicsScene(QObject *parent = 0);
    GraphicsScene(const QRectF &sceneRect, QObject *parent = 0);
    GraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = 0);
    */
    GraphicsScene(QMenu * contextMenuItem, QMenu * contextMenuScene, QObject *parent = 0);
    GraphicsScene(QMenu * contextMenuItem, QMenu * contextMenuScene, const QRectF &sceneRect, QObject *parent = 0);
    GraphicsScene(QMenu * contextMenuItem, QMenu * contextMenuScene, qreal x, qreal y, qreal width, qreal height, QObject *parent = 0);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    //void setSelectionArea(const QPainterPath &path, const QTransform &deviceTransform);

public slots:
    void toggleMode();

};


#endif //GRAPHICSSCENE_H

/*
#ifndef GRAPHICSELLIPSEITEM_H
#define GRAPHICSELLIPSEITEM_H

#include "includeHeaders.h"

class GraphicsEllipseItem : public QGraphicsEllipseItem
{
public:
    GraphicsEllipseItem( QGraphicsItem * parent = 0 ) : QGraphicsEllipseItem (parent = 0 ) {this->setFlag(QGraphicsItem::ItemIsSelectable);}
    GraphicsEllipseItem( const QRectF & rect, QGraphicsItem * parent = 0) : QGraphicsEllipseItem (rect, parent) {this->setFlag(QGraphicsItem::ItemIsSelectable);}
    GraphicsEllipseItem( qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent = 0 ) : QGraphicsEllipseItem (x, y, width, height, parent) {this->setFlag(QGraphicsItem::ItemIsSelectable);}
    QRectF boundingRect();
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

};

#endif // GRAPHICSELLIPSEITEM_H
*/
