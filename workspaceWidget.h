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

    QPointF * activeEndPoint; //end point which belongs to fragment on line below
    QPointF * lastEventPos;
    ModelFragment * activeFragment;
    ModelItem * lastUsedPart;

    QMenu * contextMenu;
    QList<ModelFragment*> * modelFragments;
    QList<ModelItem*> * selection;
    bool mousePress;
    bool ctrlPress;

    bool rotationMode;

public:
    WorkspaceWidget(QMenu* context, QWidget * parent = 0);

    int selectItem(ModelItem* item);
    int deselectItem(ModelItem* item);

    int connectFragments(ModelFragment * a, ModelFragment * b, QPointF * aP, QPointF * bP);///MISSING
    int connectFragments(int index1, int index2, QPointF * aP, QPointF * bP);///MISSING
    //int disconnectFragments(ModelFragment * a, ModelFragment * b);///MISSING
    //int disconnectFragments(int index1, int index2);///MISSING
    int addFragment(ModelFragment * frag);///INCOMPLETE
    int removeFragment(ModelFragment * frag);///INCOMPLETE - HANDLE ACTIVE POINTS
    int removeFragment(int index);///INCOMPLETE - HANDLE ACTIVE POINTS
    int updateFragment(ModelFragment * frag); ///INCOMPLETE - MISSING ERROR HANDLING
    //value of point will be modified to value of endpoint which was found.
    //if it is too far, function returns NULL and *point remains the same.
    ModelFragment * findFragmentByApproxPos(QPointF * point);

    ModelFragment * getActiveFragment() const;
    void setActiveFragment(ModelFragment * frag);

    QPointF * getActiveEndPoint() const;
    int setActiveEndPoint(QPointF * pt);

    ModelItem * getLastUsedPart() const;
    int setLastUsedPart (ModelItem * part);

    void setLastEventPos (QPointF point);

    GraphicsScene * getGraphicsScene() const;

    bool getRotationMode();

public slots:
    void toggleRotationMode();

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
