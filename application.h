#ifndef APPLICATION_H
#define APPLICATION_H


#include "includeHeaders.h"
#include "database.h"
#include "partsRelated.h"
#include "sideBarWidget.h"
#include "workspaceWidget.h"

class Database;
/*
 * unified system of classes' and attributes' names - myTitle____, where ____ equals to Qclass name without 'Q'
 * all classes have atributes of pointer types (or list of pointers)ž
 */


/*
 *WorkspaceWidget inherits from QScrollArea
 *
 *contains
 *-QGraphicsView
 *-list of QGraphicsScenes - modelFragments; each fragment consisting of QPainterPaths Items
 *
 */

class AppData //IMPLEMENTATION COMPLETE ����-NOT ENOUGH PIXMAPS-����
{
    QPixmap * newFilePixmap;
    QPixmap * openFilePixmap;
    QPixmap * saveFilePixmap;
    QPixmap * undoPixmap;
    QPixmap * redoPixmap;
    // ...


    Database * database;

public:
    AppData();
    QPixmap * getNewFilePixmap() const;
    QPixmap * getOpenFilePixmap() const;
    QPixmap * getSaveFilePixmap() const;
    QPixmap * getUndoPixmap() const;
    QPixmap * getRedoPixmap() const;
    // ...


    Database * getDatabase() const;
};

class Window : public QMainWindow ///IMPLEMENTATION INCOMPLETE
{
    Q_OBJECT
    QMenu * mainContextMenu;
    QMenuBar * mainMenuBar;
    QToolBar * mainToolBar;
    QStatusBar * mainStatusBar;
    WorkspaceWidget * workspace;
    SideBarWidget * sideBar;

public:
    Window();
    QMenu * getMainContextMenu() const;
    QMenuBar * getMainMenuBar() const;
    QStatusBar * getMainStatusBar() const;
    QToolBar * getMainToolBar() const;
    WorkspaceWidget * getWorkspaceWidget() const;
    SideBarWidget * getSideBarWidget() const;

    bool setMainContextMenu(QMenu * menu);
    bool setMainMenuBar(QMenuBar * menu);
    bool setMainStatusBar(QStatusBar * sb);
    bool setMainToolBar(QToolBar * toolBar);
    bool setWorkspaceWidget(WorkspaceWidget * ws);
    bool setSideBarWidget(SideBarWidget * sb);

    void contextMenuEvent(QContextMenuEvent * evt);
    void keyPressEvent(QKeyEvent * evt); ///MISSING
    void keyReleaseEvent(QKeyEvent * evt);

};

class Preferences
{
    QString * locale;
    QSize * lastWindowSize;
    QString ** lastFiveProjects;
    ///...
public:
    Preferences(QString * path);
    QString * getLocale() const;
    QSize * getLastSize() const;
    QString ** getLastProjects() const;
};

//probably should exist as a global variable (then you can remove some db and other pointers)
/* APPLICATION REQUIRES:
 * -Qt5 installed
 * -/etc/RTEditor/userpref.conf
 * or ./data/... on Windows
 */
class Application : public QApplication //IMPLEMENTATION COMPLETE
{
    Q_OBJECT
    Window * window; //dealloc?
    Preferences * userPreferences;//dealloc!
    AppData * appData;//dealloc!
    bool restrictedInventoryMode;
    bool allowMixedProductLines;

public:
    //Application(Window * wind,String * userPref, AppData * appDt, bool invMode);
    /**
      Application constructor - doesn't need any arguments, all paths are known.
      Application starts with empty database
     */
    Application(int argc, char ** argv);
    ~Application();

    Window * getWindow() const;
    Preferences * getUserPreferences() const;
    AppData * getAppData() const;
    bool getRestrictedInventoryMode() const;
    bool getAllowMixedProductLines() const;

    bool setAppData(AppData * d);

    bool setAllowMixedProductLines(bool mode);
    int setupUI();

public slots:
    bool setRestrictedInventoryMode(bool mode);

};

#endif // APPLICATION_H
