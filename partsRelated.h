#ifndef PARTSRELATED_H
#define PARTSRELATED_H

#include "includeHeaders.h"
#include "graphicsScene.h"
#include "itemTypeEnums.h"
#include "scales.h"

class WorkspaceWidget;
class ProductLine;
class ModelItem;

/**
  GPI class contains graphic representation of the item and some additional information
*/

class GraphicsPathItem : public QObject, public QGraphicsPathItem
{
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
    ~GraphicsPathItem();
    bool contains( const QPointF & point ) const;
    QRectF boundingRect() const;
    QPainterPath shape() const;
    /**
      method changeCountPath deletes old QGraphicsPathItem representing count of available parts
      and creates new QGPI.
      Parameter radius is used for computation of QGPI's position
    */
    void changeCountPath(unsigned int count, qreal radius);

    /**
      initialize dialog using of which the count of available items can be changed
    */
    int initDialog();
    //int initInfoDialog();

    /**
      initialize context menus - 1 is used in SideBarWidget, 1 in WorkspaceWidget
    */
    int initMenus();

    //protected events
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    /**
      mouseDoubleClickEvent() calls creating of the new ModelItem instance which is based on the data of parentItem attribute
    */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateItem();

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    /**
      mouseMoveEvent() provides moving and rotation functionality
    */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);


};

class ModelFragment;

class SlotTrackInfo
{
    /*
     *slot track ModelItem logic:
     *-ModelItem.radius stores outer radius of model piece (including "road")
     *-ModelItem.radius2 stores inner radius of model piece (including "road")
     *-ModelItem.t stores type of part - curve/straight/curved crossing/straight crossing/straight chicane/some other special types (see ITEMTYPEENUMS_H)
     *-SlotTrackInfo stores numberOfLanes (1/2/4/6/8/whatever number should work with large enough radius)
     *-SlotTrackInfo.lanesGauge stores the distance between every two lanes, doesn't affect anything if numberOfLanes==1
     *-SlotTrackInfo.lanesGaugeEnd stores:
     *      -if ModelItem.t==HE || HS then gauge of lanes at the end of the track piece
     *      -if ModelItem.t==JM then gap distance
     *      -otherwise - stores the same number as lanesGauge attribute
     *-SlotTrackInfo.fstLaneDist stores the distance of the first "outer-most" lane
*/
public:
    SlotTrackInfo();
    SlotTrackInfo(const SlotTrackInfo & s);
    unsigned int numberOfLanes;
    qreal lanesGauge;
    qreal lanesGaugeEnd; //is used only for SB part - straight bottleneck
    qreal fstLaneDist;
};

class ModelItem
{

    QString * partNo;
    QString * nameEn;
    QString * nameCs;
    ////QPointF * startPoint;

    unsigned int availableCount;

    GraphicsPathItem * contourModel; //2D model displayed in sideBarWidget - text can't be child of model with no text, because then the label is selectable on its own
    GraphicsPathItem * contourModelNoText;//and makes the clickable area much smaller

    QGLWidget * glModel;///?????????????????

    QWidget * parentWidget;//workspaceWidget or NULL - value is used for determining whether 2D Model is movable or not
    ModelFragment * parentFragment; //parent fragment is used when items are clicked and dragged -> whole fragment "moves" (=calls move() of all items)
    ProductLine * prodLine;//parent product line - each item should know prod. line which it belongs to - the "mixed manufacturers or not" mode is based on this pointer

    ItemType t;

    ////qreal turnDegree;
    QList <QPointF*> * endPoints;
    QList <qreal> * endPointsAngles;
    QList <ModelItem*> * neighbours;
    QList <int> * endPointsHeight;
    QList <QGraphicsPathItem*> * endPointsHeightGraphics;

    qreal radius;
    qreal radius2; //is used only for curved turnouts (t==J1 || J2) and for slot track
    qreal itemWidth;
    qreal itemHeight;

    SlotTrackInfo * slotTrackInfo;

    bool recursionStopper;
    bool recursionStopperAdj;
    bool deleteFlag;

public:
    //use this constructor only if QWidget isn't inherited
    //ModelItem(QString * partNumber, QString * partNameEn, QString * partNameCs,
    //        QPointF * start, QPointF * end ,qreal degree, qreal turnRadius,
    //        void * appPointer, QWidget * parentWidg = 0 ,QGraphicsItem * contourParent = 0);


    ////modify constructor - no start & end points, but ref to QList
    ModelItem(QString & partNumber, QString & partNameEn, QString & partNameCs,
              QPointF & start, QPointF & end ,qreal degree,
              qreal turnRadius, qreal width, qreal height, ProductLine * prodLine, QWidget * parentWidg = 0);

    ModelItem(QString & partNumber, QString & partNameEn, QString & partNameCs,
              QList<QPointF> & endPoints ,QList<qreal> angles, qreal turnRadius,
              qreal width, qreal height, ItemType type, ProductLine * prodLine, QWidget * parentWidg = 0);

    ~ModelItem();

    bool getDeleteFlag() const;
    void setDeleteFlag();

    QString * getPartNo() const;
    QString * getNameEn() const;
    QString * getNameCs() const;
    ////QPointF * getStartPoint() const;
    QPointF * getEndPoint(int index = 1) const;
    qreal getTurnAngle(int index = 1) const;
    qreal getTurnAngle(QPointF * pt) const;
    qreal getRadius() const;
    qreal getItemWidth() const;
    qreal getItemHeight() const;

    GraphicsPathItem * get2DModel() const;
    int set2DModel(GraphicsPathItem * model);
    GraphicsPathItem * get2DModelNoText() const;
    int set2DModelNoText(GraphicsPathItem * model);

    int generate2DModel(bool text);

    unsigned int getAvailableCount() const;
    void setAvailableCount(unsigned int count);
    void incrAvailableCount();
    void decrAvailableCount();

    ItemType getType() const;

    QWidget * getParentWidget() const;
    ProductLine * getProdLine() const;
    ModelFragment * getParentFragment() const;
    int setParentFragment(ModelFragment * frag);

    void rotate (qreal angle);
    void rotate (qreal angle,QPointF * center);


    void moveBy(qreal dx, qreal dy);
    qreal getSecondRadius() const;
    /**
      change name to rad2
*/
    void setSecondRadius (qreal radi2);

    int adjustHeightProfile(int dz, QPointF * point);//, bool ignoreRecursionStopper = false);
    void updateEndPointsHeightGraphics(bool forceUpdate = false);
    int getHeightProfileAt(QPointF * point);


    //check if it is necessary
    //void moveLabel(QPointF * point);



    void setEndPointAngle(int index, qreal angle);
    void setEndPointAngle(QPointF * pt, qreal angle);

    SlotTrackInfo * getSlotTrackInfo();
    int setSlotTrackInfo(SlotTrackInfo * s);

    ModelItem * getNeighbour(int index);
    ModelItem * getNeighbour(QPointF * pos);

    int setNeighbour(ModelItem * neighbour, int index);
    int setNeighbour(ModelItem * neighbour, QPointF * pos);

    /**
     leftRightDifference180 returns if there is 180 deg. difference between two points
     @param index1 index of the first point
     @param index2 index of the second point
     */
    bool leftRightDifference180 (int index1, int index2) const;
    /**
     leftRightDifference180 returns if there is 180 deg. difference between two points
     @param pt1 the first point
     @param pt2 the second point
     */
    bool leftRightDifference180 (QPointF * pt1, QPointF * pt2) const;

protected:

    //void mousePressEvent(QMouseEvent *);
};

class ProductLine
{
    QList<ModelItem*> * items;
    QString * name;
    QString * scale;
    ScaleEnum scaleE;
    QString * gauge;
    bool type; //true=rail false=slot track
    qreal maxTrackRadius;
    qreal minTrackRadius;

public:
    //ProductLine(QString * pLName, QString * pLScale, bool type, QString * filePath);
    ProductLine(QString &name,QString &scale, ScaleEnum s, QString &gauge, bool type, QList<ModelItem*> &items);
    ProductLine(QString &name,QString &scale, ScaleEnum s, QString &gauge, bool type);
    QString * getName() const;
    QString * getScale() const;
    ScaleEnum getScaleEnum() const;
    bool getType() const;
    QList<ModelItem*> * getItemsList() const;

    int setItemsList(QList<ModelItem*>* list);
    int addItem(ModelItem* item);
    //int generate2DModels();
    //int generate3DModels();

};

class ModelFragment// : public QWidget //inherits from QWidget, because QWidget is used as a parent of QGrItem-s
                                           //thus when QGrItem is clicked and dragged this QWidg. is being moved
                        //doesnt inherit anything - has "move" method which can be called by any part of fragment on its click
        //: public QGraphicsView///...Scene?  /// QWidget ///QPainter??
{
    QList <ModelItem*> * fragmentItems;
    QList<ProductLine*> * lines;
    //QTransform * transformMatrix;
    ////QPointF * startPoint;
    QList<QPointF *> *endPoints;
    QList<qreal> *endPointsAngles;
    QList<QGraphicsEllipseItem*> *endPointsGraphics;
    QList<ModelItem*> *endPointsItems;

    QDialog * infoDialog;

    //friend WorkspaceWidget;

public:
    ////add new constructor without startPt
    ModelFragment();
    //ModelFragmentWidget(ModelItem* item, ProductLine* line, QPointF * startPt,QList<QPointF *> *endPts,QList<qreal> * endPtAngles);
    ModelFragment(ModelItem* item);
    //ModelFragmentWidget(ModelFragmentWidget * mfw);

    ~ModelFragment();

    ModelItem * findEndPointItem(QPointF * approxPos);

    QList <ModelItem*> * getFragmentItems() const;
    QList<ProductLine*> * getProductLines() const;
    QList<QPointF *> * getEndPoints() const;
    QList<QGraphicsEllipseItem*> * getEndPointsGraphics() const;
    QList<qreal> * getEndPointsAngles() const;
    QList<ModelItem*> * getEndPointsItems() const;

    int addFragmentItems(QList <ModelItem*> * listToAppend);
    int addFragmentItem(ModelItem* item, QPointF * point, int insertionIndex = 0);

    //return-value -1 means that item is the only item of fragment and whole fragment should be deleted by workspacewidget
    int deleteFragmentItem(ModelItem * item);
    /* pseudocode:
     * ModelItem * toDelete = item;
     * List * pointsOfDeleted = item.endPoints
     * for i < this.fragmentItems.count do
     *      for j < this.fragmentItems.at[i].endPoints.count() do
     *          for k < pointsOfDeleted.count() do
     *              if (*pointsOfDeleted[k] circa equals to *this.fragmentItems.at[i].endPoints[j])
     *                  continuity isn't preserved by deleting => split fragment into pointsOfDeleted.count() fragments
    */

    int setStartPoint(QPointF * pt);
    //int addEndPoints (QList<QPointF *> * listToAppend);
    int addEndPoint (QPointF* pt, bool additionalInfo = false, qreal rotation = 0, ModelItem * endPointItem = NULL);
    int removeEndPoint (QPointF *&pt);
    //int removeEndPoint (int index);
    int setEndPointAngle(QPointF * pt, qreal angle);
    int setEndPointAngle(int index, qreal angle);

    int showInfoDialog();
    int initInfoDialog();

    void moveBy(qreal dx, qreal dy);
    void rotate(qreal angle, QPointF * center);

    bool leftSide(ModelItem *item, ModelItem *&firstItemWith180Diff);

};
/**
 * @brief makeNewItem creates new instance of ModelItem based on the parameters and either creates new fragment or inserts the new instance
 * into the current active fragment
 * @param eventPos position of mouseEvent - decides whether the left or right turn/item will be created
 * @param gpi graphic representation of the original item
 * @param parentItem
 * @param toMake
 * @param key
 */
void makeNewItem(QPointF eventPos, GraphicsPathItem * gpi, ModelItem * parentItem, ModelItem * toMake, bool key);
void recursivelyAdd(ModelItem * item, ModelFragment * fragment, QPointF *pt);
void rebuildFragment(ModelItem * startItem, ModelFragment * fragment);


#endif // PARTSRELATED_H
