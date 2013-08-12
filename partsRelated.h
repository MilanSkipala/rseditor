#ifndef PARTSRELATED_H
#define PARTSRELATED_H

#include "includeHeaders.h"
#include "itemTypeEnums.h"

class ProductLine;
class ModelItem;
class GraphicsPathItem : public QObject, public QGraphicsPathItem
{
    /*
     *boundingRect(), shape(), and contains()
    */
    Q_OBJECT
    GraphicsPathItem * restrictedCountPath;
    ModelItem * parentItem;
    QMenu * contextMenuSBW;
    QMenu * contextMenuWSW;

    QDialog * dialog;
    QDialog * infoDialog;

    bool mousePressed;
    QPointF * offset;

public:
    GraphicsPathItem(ModelItem * item, QGraphicsItem * parent = 0);
    GraphicsPathItem(ModelItem * item, const QPainterPath & path, QGraphicsItem * parent = 0);
    bool contains( const QPoint & point ) const;
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void changeCountPath(unsigned int count, qreal radius);
    int initDialog();
    int initInfoDialog();
    int initMenus();

    //protected events
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private slots:
    void updateItem();

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);


};

class ModelFragmentWidget;

class ModelItem
{
    QString * partNo;
    QString * nameEn;
    QString * nameCs;
    ////QPoint * startPoint;
    QList <QPoint*> * endPoints;
    unsigned int availableCount;

    GraphicsPathItem * contourModel; //2D model displayed in sideBarWidget - text can't be child of model with no text, because then the label is selectable on its own
    GraphicsPathItem * contourModelNoText;//and makes the clickable area much smaller

    QGLWidget * glModel;///?????????????????

    QWidget * parentWidget;//workspaceWidget or NULL - value is used for determining whether 2D Model is movable or not
    ModelFragmentWidget * parentFragment; //parent fragment is used when items are clicked and dragged -> whole fragment "moves" (=calls move() of all items)
    ProductLine * prodLine;//parent product line - each item should know prod. line which it belongs to - the "mixed manufacturers or not" mode is based on this pointer

    RailItemTypes t;

    ////qreal turnDegree;
    QList <qreal> * endPointsAngles;

    qreal radius;

public:
    //use this constructor only if QWidget isn't inherited
    //ModelItem(QString * partNumber, QString * partNameEn, QString * partNameCs,
    //        QPoint * start, QPoint * end ,qreal degree, qreal turnRadius,
    //        void * appPointer, QWidget * parentWidg = 0 ,QGraphicsItem * contourParent = 0);


    ////modify constructor - no start & end points, but ref to QList
    ModelItem(QString & partNumber, QString & partNameEn, QString & partNameCs,
              QPoint & start, QPoint & end ,qreal degree,
              qreal turnRadius, ProductLine * prodLine, QWidget * parentWidg = 0);

    ModelItem(ModelItem &orig);
    QString * getPartNo() const;
    QString * getNameEn() const;
    QString * getNameCs() const;
    ////QPoint * getStartPoint() const;
    QPoint * getEndPoint(int index = 1) const;
    qreal getTurnDegree(int index = 1) const;
    qreal getRadius() const;
    GraphicsPathItem * get2DModel() const;
    int set2DModel(GraphicsPathItem * model);
    GraphicsPathItem * get2DModelNoText() const;
    int set2DModelNoText(GraphicsPathItem * model);

    int generate2DModel(bool text);

    unsigned int getAvailableCount() const;
    void setAvailableCount(unsigned int count);
    void incrAvailableCount();
    void decrAvailableCount();

    QWidget * getParentWidget() const;
    ProductLine * getProdLine() const;
    ModelFragmentWidget * getParentFragment() const;
    int setParentFragment(ModelFragmentWidget * frag);


    //check if it is necessary
    void moveLabel(QPoint * point);



protected:

    //void mousePressEvent(QMouseEvent *);
};

class ProductLine
{
    QList<ModelItem*> * items;
    QString * name;
    QString * scale;
    QString * gauge;
    bool type; //true=rail false=slotcar track
    qreal maxTrackRadius;
    qreal minTrackRadius;

public:
    //ProductLine(QString * pLName, QString * pLScale, bool type, QString * filePath);
    ProductLine(QString &name,QString &scale, QString &gauge, bool type, QList<ModelItem*> &items);
    ProductLine(QString &name,QString &scale, QString &gauge, bool type);
    QString * getName() const;
    QString * getScale() const;
    bool getType() const;
    QList<ModelItem*> * getItemsList() const;

    int setItemsList(QList<ModelItem*>* list);
    int addItem(ModelItem* item);
    //int generate2DModels();
    //int generate3DModels();

};

class ModelFragmentWidget// : public QWidget //inherits from QWidget, because QWidget is used as a parent of QGrItem-s
                                           //thus when QGrItem is clicked and dragged this QWidg. is being moved
                        //doesnt inherit anything - has "move" method which can be called by any part of fragment on its click
        //: public QGraphicsView///...Scene?  /// QWidget ///QPainter??
{
    QList <ModelItem*> * fragmentItems;
    QList<ProductLine*> * lines;
    QTransform * transformMatrix;
    ////QPoint * startPoint;
    QList<QPoint *> *endPoints;
    ////int startPointAngle;
    QList<qreal> *endPointsAngles;

    QDialog * infoDialog;

public:
    ////add new constructor without startPt
    ModelFragmentWidget();
    ModelFragmentWidget(ModelItem* item, ProductLine* line, QPoint * startPt,QList<QPoint *> *endPts,QList<qreal> * endPtAngles);
    //ModelFragmentWidget(ModelFragmentWidget * mfw);


    QList <ModelItem*> * getFragmentItems() const;
    QList<ProductLine*> * getProductLines() const;
    QPoint * getStartPoint() const;
    QList<QPoint *> * getEndPoints() const;
    qreal getStartPointAngle();
    QList<qreal> * getEndPointsAngles() const;

    int addFragmentItems(QList <ModelItem*> * listToAppend);
    int addFragmentItem(ModelItem* item, QPoint * point);
    int setStartPoint(QPoint * pt);
    int addEndPoints (QList<QPoint *> * listToAppend);
    int removeEndPoint (QPoint * pt);
    int removeEndPoint (int index);
    int removeStartPoint();
    int setStartPointAngle(qreal angle);
    int setEndPointAngle(QPoint * pt, qreal angle);
    int setEndPointAngle(int index, qreal angle);

    int showInfoDialog();
    int initInfoDialog();

    void moveBy(qreal dx, qreal dy);

};



#endif // PARTSRELATED_H
