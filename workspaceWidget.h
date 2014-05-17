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

#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include <QTreeView>
#include "includeHeaders.h"
#include "partsRelated.h"



class WorkspaceWidget : public QGraphicsView
{
    Q_OBJECT
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
    ~WorkspaceWidget();

    /**
     * @brief commandExecution method parses QString command, "finds" its arguments (model items etc.) and calls appropriate method
     * @param command
     * @return
     */
    int commandExecution(QString command);
    /**
     * @brief pushBackCommand inserts command and its negation in undo/redo command lists
     * @param command
     * @param negCommand
     * @return
     */
    int pushBackCommand(QString command, QString negCommand);

    /**
     * @brief makeItem inserts item at point pt. Item is inserted in track section at pt.
     * @param item
     * @param pt
     * @param left
     * @return
     */
    int makeItem(ModelItem * item,QPointF * pt, bool left);
    int makeItem(ModelItem * item,QPointF * pt, QPointF eventPos);

    /**
     * @brief makeBorder inserts border item. Slot track item has to be selected AND its border endpoint has to be selected
     * @param border
     * @return
     */
    int makeBorder(BorderItem * border);

    /**
     * @brief makeVegetation inserts vegetation item at position of activeEndPoint
     * @param item
     * @return
     */
    int makeVegetation(VegetationItem * item);

    /**
     * @brief selectItem adds item in selection list. If CTRL key is not pressed it clears current selection
     * @param item
     * @return
     */
    int selectItem(ModelItem* item);
    /**
     * @brief deselectItem removes item from selection list.
     * @param item
     * @return
     */
    int deselectItem(ModelItem* item);
    QList<ModelItem*> * getSelection();

    int selectItem(VegetationItem * item);
    int deselectItem(VegetationItem * item);
    QList<VegetationItem *> *getSelectionVegetation();

    void selectBorder(BorderItem * border);

    /**
     * @brief connectFragments finds fragment b which can be connected to a and connects them
     * @param a
     * @param startItem is used only when the method is called from Undo/Redo
     * @return
     */
    int connectFragments(ModelFragment * a, ModelItem * startItem = NULL);

    /**
     * @brief connectFragments - this method is called by commandExecution where all parameters are already known
     * @param posA
     * @param posB
     * @param a
     * @param b
     * @param aItem
     * @param bItem
     * @param fragCID
     * @return
     */
    int connectFragments(QPointF *posA, QPointF *posB, ModelFragment * a, ModelFragment * b, ModelItem * aItem, ModelItem * bItem, int fragCID = -1);

    /**
     * @brief disconnectFragment splits track section c into two sections with IDs set to idA and idB
     * @param c
     * @param disconnectAt
     * @param idA
     * @param idB
     * @return
     */
    int disconnectFragment(ModelFragment * c, QPointF * disconnectAt, int idA, int idB);

    /**
     * @brief addVegetation inserts item into list of vegetation items in the workspace
     * @param item
     * @return
     */
    int addVegetation(VegetationItem * item);

    /**
     * @brief addFragment inserts track section "frag" into list of track sections in the workspace
     * @param frag
     * @param fragID if == -1 then ID is set to nextIDToUse, otherwise ID is set to fragID
     * @return
     */
    int addFragment(ModelFragment * frag, int fragID = -1);

    int removeFragment(ModelFragment * frag);

    /**
     * @brief removeItem removes item from its parent track section
     * @param item
     * @param idList is used when the method is called by commandExecution() - fragments created by deletion have their IDs set to idList values
     * @return
     */
    int removeItem(ModelItem * item, QList<int>*idList = NULL);
    /**
     * @brief removeVegetation removes vegetation item from the list of items and frees item's memory
     * @param item
     * @return
     */
    int removeVegetation(VegetationItem * item);

    /**
     * @brief bendAndClose - if pt1 and pt2 are not too far and angles at these points are not too different the method
     *                  closes the track section by bending its circular section
     * @param frag
     * @param pt1
     * @param pt2
     * @return
     */
    int bendAndClose(ModelFragment * frag, QPointF * pt1, QPointF * pt2);

    /**
     * @brief bendAndClose uses at most 5 items of A and 5 items of B to bend the track
     * @param fragA
     * @param fragB
     * @param pt1
     * @param pt2
     * @return
     */
    int bendAndClose(ModelFragment * fragA, ModelFragment * fragB, QPointF * pt1, QPointF * pt2);

    /**
     * @brief completeFragment automatically builds the track between pt1 and pt2
     * @param frag
     * @param pt1
     * @param pt2
     * @return
     */
    int completeFragment(ModelFragment * frag, QPointF * pt1, QPointF * pt2);
    int completeFragment(ModelFragment *fragA, ModelFragment * fragB, QPointF *ptA, QPointF *ptB);

    /**
     * @brief updateFragment will add new items of "frag" to graphicsScene
     * @param frag
     * @return
     */
    int updateFragment(ModelFragment * frag);

    /**
     * @brief findFragmentByApproxPos - tries to find track fragment by position
     * @param point - if fragment is found then value is set to exact value of endpoint, otherwise value remains the same
     * @return
     */
    ModelFragment * findFragmentByApproxPos(QPointF * point);

    /**
     * @brief findItemByApproxPos - tries to find track item by position
     * @param point - if item is found then value is set to exact value of endpoint, otherwise value remains the same
     * @param dontWantThisItem - used especially when looking for the neighbour of item (-> then item is not the part which should be returned)
     * @return
     */
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

    /**
     * @brief canInsert
     * @param item
     * @return true if scales of activeFragment and item are the same
     */
    bool canInsert(ModelItem * item) const;
    /**
     * @brief canInsert
     * @param item
     * @return true if selection contains just one slot track item and its radius matches border item's radius
     */
    bool canInsert(BorderItem * item) const;

    /**
     * @brief exportCurrentState prints string commands, execution of which brings workspace to current state
     * @param file
     * @return
     */
    int exportCurrentState(QTextStream& file);
    /**
     * @brief setCurrentState sets scene parameters and then calls commandExecution() to build model from file
     * @param file
     * @return
     */
    int setCurrentState(QTextStream & file);
    /**
     * @brief resetWorkspace clears selection lists, undo/redo lists, deletes all items in the scene
     */
    void resetWorkspace();

    //flags
    bool getSelectTwoPointsBend() const;
    bool getSelectTwoPointsComplete() const;
    bool unsavedChangesFlag() const;

public slots:
    void toggleRotationMode();
    void toggleHeightProfileMode();
    void adjustHeightOfActive();


    //both the action and its negation are on the same index in actionLists
    /**
     * @brief undo (string) action and its negation are at the same index in actionLists
     *              undo/redo calls commandExecution() and modifies indexUndoRedo
     *                  it may call itself recursively to make undo/redo more user friendly
     */
    void undo();//do the "negative action" and decrease the counter

    /**
     * @brief redo (string) action and its negation are at the same index in actionLists
     *              undo/redo calls commandExecution() and modifies indexUndoRedo
     *                  it may call itself recursively to make undo/redo more user friendly
     */
    void redo();//increase the counter and do the action


    void copy();
    void paste();
    void cut();

    void selectFragmentToClose();
    void selectFragmentToComplete();

    /**
     * @brief makeLastItem inserts again last used item. Is called when spacebar is pressed
     * @return
     */
    int makeLastItem();
    /**
     * @brief removeItems iterate through all items in the workspace and remove selected ones
     */
    void removeItems();

    /**
     * @brief modelInfo fills Application member of type QTreeView with information about track sections and their items
     */
    void modelInfo();

    /**
     * @brief exportBitmap lets user specify output PNG file and prints the scene into the file
     *              track items are not filled (slot track) to save some cartridge
     */
    void exportBitmap();

protected:
    void mousePressEvent(QMouseEvent * evt);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *evt);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void scaleView(qreal factor);
    void paintEvent(QPaintEvent *event);

};



#endif // WORKSPACEWIDGET_H
