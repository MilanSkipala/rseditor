#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include <QTreeView>
#include "includeHeaders.h"
#include "partsRelated.h"



class WorkspaceWidget : public QGraphicsView ///public QScrollArea ///IMPLEMENTATION INCOMPLETE
{
    Q_OBJECT
    //void * app_ptr;
    //QFrame * frameWidget;
    QGraphicsView * graphicsView;
    GraphicsScene * graphicsScene;
    QGraphicsItem * activeEndPointGraphic;

    QPointF * activeEndPoint; //end point which is same as point in activeFragment (same value, not pointer)
    QPointF * activeEndPointPrev; //this is needed to make the undo/redo feature working

    QPointF * lastEventPos;
    ModelFragment * activeFragment;
    ModelFragment * activeFragmentPrev;

    ModelItem * activeItem;
    ModelItem * activeItemPrev;
    ModelItem * lastUsedPart; //this is the pointer to item in SideBarWidget
    ModelItem * lastInserted; //this is the pointer to item in WorkspaceWidget

    QMenu * contextMenu;
    QList<ModelFragment*> * modelFragments;
    QList<VegetationItem*> * vegetationItems;

    QList<ModelItem*> * selection;
    QList<VegetationItem*> * selectionVegetation;
    QList<QString> * copiedItems;

    bool mousePress;
    bool ctrlPress;
    bool deletePress;
    bool shiftPress;

    bool selectTwoPointsBend;
    bool selectTwoPointsComplete;

    bool rotationMode;
    bool heightProfileMode;
    int indexUndoRedo;


    QList <QString> actionListRedo;
    QList <QString> actionListUndo;

    bool undoRedoCalled;
    bool eraseFollowing; //this "resets" redo feature, when i.e. another item is inserted after undo
    bool doNotPrintSetEndPoint;
    bool unsavedChanges;

    int nextIDToUse;

public:
    WorkspaceWidget(QMenu* context, QWidget * parent = 0);

    int commandExecution(QString command);
    int pushBackCommand(QString command, QString negCommand);

    //int makeLastItem(); this needs to be a slot, because of the keyboard shortcut
    int makeItem(ModelItem * item,QPointF * pt, bool left);
    int makeItem(ModelItem * item,QPointF * pt, QPointF eventPos);

    int makeBorder(BorderItem * border);

    int makeVegetation(VegetationItem * item);

    int selectItem(ModelItem* item);
    int deselectItem(ModelItem* item);
    QList<ModelItem*> * getSelection();

    int selectItem(VegetationItem * item);
    int deselectItem(VegetationItem * item);
    QList<VegetationItem *> *getSelectionVegetation();

    void selectBorder(BorderItem * border);

    //startItem parameter is used only when the method is called from Undo/Redo
    int connectFragments(ModelFragment * a, ModelItem * startItem = NULL);
    int connectFragments(QPointF *posA, QPointF *posB, ModelFragment * a, ModelFragment * b, ModelItem * aItem, ModelItem * bItem, int fragCID = -1);

    int disconnectFragment(ModelFragment * c, QPointF * disconnectAt, int idA, int idB);

    int addVegetation(VegetationItem * item);

    int addFragment(ModelFragment * frag, int fragID = -1);   ///INCOMPLETE
    int removeFragment(ModelFragment * frag);///INCOMPLETE - HANDLE ACTIVE POINTS
    int removeFragment(int index);///INCOMPLETE - HANDLE ACTIVE POINTS
    int removeItem(ModelItem * item, QList<int>*idList = NULL);
    int removeVegetation(VegetationItem * item);


    int bendAndCloseV1(ModelFragment * frag, QPointF * pt1, QPointF * pt2);
    int bendAndClose(ModelFragment * frag, QPointF * pt1, QPointF * pt2);

    int bendAndClose(ModelFragment * fragA, ModelFragment * fragB, QPointF * pt1, QPointF * pt2);

    ///is this version needed too?
    ///if it is, it should be done like this:
    ///     -get FlexNeeded
    ///     -find some point(s) of rotation/movement
    ///     -bend fragA, connectFragments(fragA,fragB)
    ///int bendAndClose(ModelFragment * fragA, ModelFragment * fragB, QPointF * pt1, QPointF * pt2);

    int completeFragment(ModelFragment * frag, QPointF * pt1, QPointF * pt2);
    int completeFragment(ModelFragment *fragA, ModelFragment * fragB, QPointF *ptA, QPointF *ptB);

    int updateFragment(ModelFragment * frag);

    //value of point will be modified to value of endpoint which was found.
    //if it is too far, function returns NULL and *point remains the same.
    ModelFragment * findFragmentByApproxPos(QPointF * point);
    ModelItem * findItemByApproxPos(QPointF * point, ModelItem * dontWantThisItem);
    ModelFragment * findFragmentByID(int id);

    int getFragmentIndex(ModelFragment* frag);

    ModelFragment * getActiveFragment() const;
    void setActiveFragment(ModelFragment * frag);

    ModelItem * getActiveItem() const;
    void setActiveItem(ModelItem * item);

    QPointF * getActiveEndPoint() const;
    int setActiveEndPoint(QPointF * pt);

    ModelItem * getLastUsedPart() const;
    int setLastUsedPart (ModelItem * part);

    int setLastInserted (ModelItem * item);

    void setLastEventPos (QPointF point);

    GraphicsScene * getGraphicsScene() const;

    bool getRotationMode();
    bool getHeightProfileMode();

    bool getDeletePress() const;

    bool canInsert(ModelItem * item) const;
    bool canInsert(BorderItem * item) const;

    int exportCurrentState(QTextStream& file);
    int setCurrentState(QTextStream & file);
    void resetWorkspace();

    bool getSelectTwoPointsBend();
    bool getSelectTwoPointsComplete();

    bool unsavedChangesFlag() const;

public slots:
    void toggleRotationMode();
    void toggleHeightProfileMode();
    void adjustHeightOfActive();
    //both the action and its negation are on the same index in actionLists
    void undo();//do the "negative action" and decrease the counter
    void redo();//increase the counter and do the action

    void copy();
    void paste();
    void cut();

    void selectFragmentToClose();
    void selectFragmentToComplete();

    int makeLastItem();
    void removeItems();

    //void modelInfo(QTreeView * tree);
    void modelInfo();

    void exportBitmap();

protected:
    void contextMenuEvent(QContextMenuEvent * evt);///MISSING
    void mousePressEvent(QMouseEvent * evt) ;///MISSING
    void mouseReleaseEvent(QMouseEvent *event);///MISSING
    void mouseMoveEvent(QMouseEvent * evt);///MISSING
    void wheelEvent(QWheelEvent *evt);///MISSING
    void keyPressEvent(QKeyEvent *event);///MISSING
    void keyReleaseEvent(QKeyEvent *event);///MISSING
    void scaleView(qreal factor);
    void paintEvent(QPaintEvent *event);


};



#endif // WORKSPACEWIDGET_H
