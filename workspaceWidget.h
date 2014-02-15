#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

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
    QList<ModelItem*> * selection;
    bool mousePress;
    bool ctrlPress;
    bool deletePress;
    bool shiftPress;

    bool rotationMode;
    bool heightProfileMode;
    int indexUndoRedo;

//public: ///this has to be private
    QList <QString> actionListRedo;
    QList <QString> actionListUndo;

    bool undoRedoCalled;
    bool eraseFollowing; //this "resets" redo feature, when i.e. another item is inserted after undo
    bool doNotPrintSetEndPoint;

    int nextIDToUse;

public:
    WorkspaceWidget(QMenu* context, QWidget * parent = 0);

    int commandExecution(QString command);
    int pushBackCommand(QString command, QString negCommand);

    int makeLastItem();
    int makeItem(ModelItem * item,QPointF * pt, bool left);
    int makeItem(ModelItem * item,QPointF * pt, QPointF eventPos);

    int selectItem(ModelItem* item);
    int deselectItem(ModelItem* item);

    int connectFragments(ModelFragment * a);
    int connectFragments(QPointF *posA, QPointF *posB, ModelFragment * a, ModelFragment * b, ModelItem * aItem, ModelItem * bItem);
    int disconnectFragments(ModelFragment * c, QPointF * disconnectAt, int idA, int idB);

    int addFragment(ModelFragment * frag, int fragID = -1);   ///INCOMPLETE
    int removeFragment(ModelFragment * frag);///INCOMPLETE - HANDLE ACTIVE POINTS
    int removeFragment(int index);///INCOMPLETE - HANDLE ACTIVE POINTS
    int removeItem(ModelItem * item);

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

    int exportCurrentState(ofstream & file);
    int setCurrentState(ifstream & file);
    void resetWorkspace();

public slots:
    void toggleRotationMode();
    void toggleHeightProfileMode();
    void adjustHeightOfActive();
    //both the action and its negation are on the same index in actionLists
    void undo();//do the "negative action" and decrease the counter
    void redo();//increase the counter and do the action

protected:
    void contextMenuEvent(QContextMenuEvent * evt);///MISSING
    void mousePressEvent(QMouseEvent * evt) ;///MISSING
    void mouseReleaseEvent(QMouseEvent *event);///MISSING
    void mouseMoveEvent(QMouseEvent * evt);///MISSING
    void wheelEvent(QWheelEvent *evt);///MISSING
    void keyPressEvent(QKeyEvent *event);///MISSING
    void keyReleaseEvent(QKeyEvent *event);///MISSING
    void scaleView(qreal factor);


};



#endif // WORKSPACEWIDGET_H
