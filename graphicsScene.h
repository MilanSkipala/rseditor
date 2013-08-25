#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include "includeHeaders.h"


//enum SelectionMode {};

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
    bool mouseMode; //F=select activeEP by doubleClick, T=create aEP by doubleClick
public:
    GraphicsScene(QObject *parent = 0);
    GraphicsScene(const QRectF &sceneRect, QObject *parent = 0);
    GraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = 0);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);


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
