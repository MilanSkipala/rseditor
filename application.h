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

#ifndef APPLICATION_H
#define APPLICATION_H


#include "includeHeaders.h"
#include <QErrorMessage>
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
 *contains
 *-QGraphicsView
 *-list of QGraphicsScenes - modelFragments; each fragment consisting of QPainterPaths Items
 */

class AppData /// ����-NOT ENOUGH PIXMAPS-����
{
    QPixmap * newFilePixmap;
    QPixmap * openFilePixmap;
    QPixmap * saveFilePixmap;
    QPixmap * undoPixmap;
    QPixmap * redoPixmap;
    QPixmap * rotateToolPixmap;
    QPixmap * heightProfilePixmap;
    QPixmap * heightProfileUpPixmap;
    QPixmap * heightProfileDownPixmap;
    QPixmap * bendPixmap;
    QPixmap * completePixmap;

    QPixmap * undoPixmapL;
    QPixmap * redoPixmapL;
    QPixmap * rotateToolPixmapL;
    QPixmap * heightProfilePixmapL;
    QPixmap * heightProfileUpPixmapL;
    QPixmap * heightProfileDownPixmapL;
    QPixmap * bendPixmapL;
    QPixmap * completePixmapL;



    /// ...


    Database * database;

    QMessageBox * messageDialog;

    QErrorMessage * bendAndCloseMessage;

    //QString * lastProjects[5];

public:
    AppData(QString &lang);
    QPixmap * getNewFilePixmap() const;
    QPixmap * getOpenFilePixmap() const;
    QPixmap * getSaveFilePixmap() const;
    QPixmap * getUndoPixmap() const;
    QPixmap * getRedoPixmap() const;
    QPixmap * getRotateToolPixmap() const;
    QPixmap * getHeightProfilePixmap() const;
    QPixmap * getHeightProfileUpPixmap() const;
    QPixmap * getHeightProfileDownPixmap() const;
    QPixmap * getBendPixmap() const;
    QPixmap * getCompletePixmap() const;
    QPixmap * getUndoPixmapL() const;
    QPixmap * getRedoPixmapL() const;
    QPixmap * getRotateToolPixmapL() const;
    QPixmap * getHeightProfilePixmapL() const;
    QPixmap * getHeightProfileUpPixmapL() const;
    QPixmap * getHeightProfileDownPixmapL() const;
    QPixmap * getBendPixmapL() const;
    QPixmap * getCompletePixmapL() const;
    /// ...


    Database * getDatabase();

    void setMessageDialogText(QString textEn, QString textCs);
    QMessageBox * getMessageDialog();

    QErrorMessage * getErrorMessage();

    //QString ** getLastProjects();
    //void addProject(QString * projName);

};

class Window : public QMainWindow
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
    //~Window();
    QMenu * getMainContextMenu()  ;
    QMenuBar * getMainMenuBar()  ;
    QStatusBar * getMainStatusBar()  ;
    QToolBar * getMainToolBar()  ;
    WorkspaceWidget * getWorkspaceWidget()  ;
    SideBarWidget * getSideBarWidget()  ;

    bool setMainContextMenu(QMenu * menu);
    bool setMainMenuBar(QMenuBar * menu);
    bool setMainStatusBar(QStatusBar * sb);
    bool setMainToolBar(QToolBar * toolBar);
    bool setWorkspaceWidget(WorkspaceWidget * ws);
    bool setSideBarWidget(SideBarWidget * sb);

protected:
    void contextMenuEvent(QContextMenuEvent * evt);
    void keyPressEvent(QKeyEvent * evt);
    void keyReleaseEvent(QKeyEvent * evt);
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *evt);

};

class Preferences
{
    QString * preferencesPath;
    QString * locale;
    QSize * lastWindowSize;
    QString ** lastProjects;
    QAction ** actions;
    bool displayHelpForBendAndClose;
    bool smallIcons;
    bool saveScenePos;
    ///...
    ///...
public:
    Preferences(QString * path);
    ~Preferences();
    QString * getLocale();
    void setLocale(QString *lang);

    QSize * getLastSize();
    void setLastSize(const QSize * winSize);

    bool getSmallIconsFlag();
    void setSmallIconsFlag(bool flag);

    bool getSaveScenePosFlag();
    void setSaveScenePosFlag(bool flag);

    QString ** getLastProjects();
    void addProject(QString * projName);
    void setActions(QAction * a1,QAction * a2,QAction * a3,QAction * a4,QAction * a5);


    bool getDisplayHelpBendAndClose() const;
    void setDisplayHelpBendAndClose(bool val);
};

//probably should exist as a global variable (then you can remove some db and other pointers)
/* APPLICATION REQUIRES:
 * -Qt5 installed
 * -/etc/RSEditor/userpref.conf
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
    QString projectName;
    bool saveAsFlag;

    QDialog * modelInfoDialog;
    QTreeView * treeView;

    QDialog * preferencesDialog;
    QDialog * aboutDialog;

public:
    //Application(Window * wind,String * userPref, AppData * appDt, bool invMode);
    /*
      Application constructor - doesn't need any arguments, all paths are known.
      Application starts with full database but empty combobox of "active manufacturers"
     */
    Application(int argc, char ** argv);
    ~Application();

    Window * getWindow()  ;
    Preferences * getUserPreferences()  ;
    AppData * getAppData()  ;
    bool getRestrictedInventoryMode()  ;
    bool getAllowMixedProductLines()  ;

    bool setAppData(AppData * d);
    void showModelInfo();

    QTreeView * getTreeView();


    int setupUI();

public slots:
    bool setRestrictedInventoryMode(bool mode);
    bool setAllowMixedProductLines(bool mode);
    void save();
    void saveAs();
    void open();
    void openLast(int index);
    void newFile();
    void saveModelInfo();
    void savePreferences();
    void saveInventory();

};

#endif // APPLICATION_H
