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
    QGraphicsScene * graphicsScene;

    QPoint * activeEndPoint; //end point which belongs to fragment on line below
    ModelFragmentWidget * activeFragment;
    ModelItem * lastUsedPart;

    QMenu * contextMenu;
    QList<ModelFragmentWidget*> * modelFragments;
    QList<ModelItem*> * selection;
    bool mousePress;
    bool ctrlPress;

public:
    WorkspaceWidget(QMenu* context, QWidget * parent = 0);

    int selectItem(ModelItem* item);
    int deselectItem(ModelItem* item);

    int connectFragments(ModelFragmentWidget * a, ModelFragmentWidget * b, QPoint * aP, QPoint * bP);///MISSING
    int connectFragments(int index1, int index2, QPoint * aP, QPoint * bP);///MISSING
    int disconnectFragments(ModelFragmentWidget * a, ModelFragmentWidget * b);///MISSING
    int disconnectFragments(int index1, int index2);///MISSING
    int addFragment(ModelFragmentWidget * frag);///INCOMPLETE
    int removeFragment(ModelFragmentWidget * frag);///INCOMPLETE - HANDLE ACTIVE POINTS
    int removeFragment(int index);///INCOMPLETE - HANDLE ACTIVE POINTS
    int updateFragment(ModelFragmentWidget * frag); ///INCOMPLETE - MISSING ERROR HANDLING

    ModelFragmentWidget * getActiveFragment() const;
    int setActiveFragment(ModelFragmentWidget * frag);

    QPoint * getActiveEndPoint() const;
    int setActiveEndPoint(QPoint * pt);

    ModelItem * getLastUsedPart() const;
    int setLastUsedPart (ModelItem * part);
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
