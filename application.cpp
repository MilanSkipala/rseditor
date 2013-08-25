#include "partsRelated.h"
#include "application.h"
#include "database.h"
#include "globalVariables.h"

Application::Application(int argc, char ** argv) : QApplication(argc, argv)
{
    //check whether the app is running on Linux or Win system

    //check if the .conf files in directory /etc/RTEditor/ or .\\data\\ exist - if not, exit app

    QString userPref = ""; // load data from userpref.conf

    this->userPreferences = new Preferences(&userPref); //new QString(userPref);

    //check if the /etc/RTEditor/icons or \\data\\icons\\ exist - if not, exit app

    this->appData = new AppData();
    this->restrictedInventoryMode=false;
    this->window = new Window();
    this->setupUI();

}

Application::~Application()
{
    delete this->appData;
    delete this->window;
    delete this->userPreferences;
}

Window * Application::getWindow() const
{
    return this->window;
}
Preferences * Application::getUserPreferences() const
{
    return this->userPreferences;
}
AppData * Application::getAppData() const
{
    return this->appData;
}
bool Application::getRestrictedInventoryMode() const
{
    return this->restrictedInventoryMode;
}
bool Application::getAllowMixedProductLines() const
{
    return this->allowMixedProductLines;
}


bool Application::setAppData(AppData * d)
{
    if (d==NULL)
        return false;

    this->appData = d;
    return true;

}
bool Application::setRestrictedInventoryMode(bool mode)
{
    this->restrictedInventoryMode=mode;
    //this->window->getSideBarWidget()->update();
    if (this->window->getSideBarWidget()->getCurrentScene()!=NULL)
        this->window->getSideBarWidget()->getCurrentScene()->update(this->window->getSideBarWidget()->getCurrentScene()->sceneRect());
    return true;
}
bool Application::setAllowMixedProductLines(bool mode)
{
    this->allowMixedProductLines=mode;
    return true;
}
int Application::setupUI()
{
    bool result = false;
    result = this->window->setMainMenuBar(new QMenuBar(this->window));
    this->window->setMenuBar(this->window->getMainMenuBar());


    //MENU FILE
    QMenu * fileMenu = new QMenu("File", this->window->getMainMenuBar());
    QMenu * recentFiles = new QMenu ("Recent projects",fileMenu);
    QMenu * editMenu = new QMenu("Edit",this->window->getMainMenuBar());
    QMenu * modelMenu = new QMenu("Model",this->window->getMainMenuBar());
    QMenu * aboutMenu = new QMenu("About", this->window->getMainMenuBar());

    QAction * newFileAction = new QAction(*this->appData->getNewFilePixmap(),"New Project",fileMenu);
    QAction * openFileAction = new QAction(*this->appData->getOpenFilePixmap(),"Open Project",fileMenu);
    QAction * saveFileAction = new QAction(*this->appData->getSaveFilePixmap(),"Save Project",fileMenu);

    QAction * recent1 = new QAction ("Replace this text with filename",recentFiles);
    QAction * recent2 = new QAction ("Replace this text with filename",recentFiles);
    QAction * recent3 = new QAction ("Replace this text with filename",recentFiles);
    QAction * recent4 = new QAction ("Replace this text with filename",recentFiles);
    QAction * recent5 = new QAction ("Replace this text with filename",recentFiles);    

    QAction * closeFileAction = new QAction("Close project 'Filename'",fileMenu);
    QAction * exportAction = new QAction("Export 2D graphic",fileMenu);
    QAction * quitAction = new QAction("Exit", fileMenu);

    QAction * undoAction = new QAction(*this->getAppData()->getUndoPixmap(),"Undo",editMenu);
    QAction * redoAction = new QAction(*this->getAppData()->getRedoPixmap(),"Redo",editMenu);
    ///change pixmap getters!!!!!!!!!!!!!!!!!
    QAction * repeatAction = new QAction(*this->getAppData()->getUndoPixmap(),"Repeat last track",editMenu);
    QAction * copyAction = new QAction(*this->getAppData()->getUndoPixmap(),"Copy",editMenu);
    QAction * pasteAction = new QAction(*this->getAppData()->getUndoPixmap(),"Paste",editMenu);
    QAction * cutAction = new QAction(*this->getAppData()->getUndoPixmap(),"Cut",editMenu);
    QAction * deleteAction = new QAction(*this->getAppData()->getUndoPixmap(),"Delete",editMenu);
    QAction * preferencesAction = new QAction(*this->getAppData()->getUndoPixmap(),"Preferences",editMenu);

    QAction * restrictedAction = new QAction("Restricted mode",modelMenu);
    QAction * manufactAction = new QAction("Mix manufacturers",modelMenu);
    QAction * addManuAction = new QAction(*this->getAppData()->getUndoPixmap(),"Add manufacturer",modelMenu);
    QAction * removeManuAction = new QAction(*this->getAppData()->getUndoPixmap(),"Remove manufacturer",modelMenu);
    QAction * inventoryAction = new QAction(*this->getAppData()->getUndoPixmap(),"Inventory",modelMenu);
    QAction * glViewAction = new QAction(*this->getAppData()->getUndoPixmap(),"3D View",modelMenu);
    restrictedAction->setCheckable(true);
    manufactAction->setCheckable(true);
    restrictedAction->setChecked(false);
    manufactAction->setChecked(false);
    connect(restrictedAction,SIGNAL(toggled(bool)),this,SLOT(setRestrictedInventoryMode(bool)));

    QAction * aboutAction = new QAction(*this->getAppData()->getUndoPixmap(),"About", aboutMenu);
    QAction * helpAction = new QAction(*this->getAppData()->getUndoPixmap(),"Help", aboutMenu);

    QAction * endPointToggleAction = new QAction(*this->getAppData()->getOpenFilePixmap(),"Create end point",NULL);
    endPointToggleAction->setCheckable(true);

    this->window->setWindowTitle("R&T Editor");

//other QActions

    recentFiles->addAction(recent1);
    recentFiles->addAction(recent2);
    recentFiles->addAction(recent3);
    recentFiles->addAction(recent4);
    recentFiles->addAction(recent5);

    fileMenu->addAction(newFileAction);
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(saveFileAction);
    fileMenu->addMenu(recentFiles);
    fileMenu->addSeparator();
    fileMenu->addAction(closeFileAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addAction(repeatAction);
    editMenu->addSeparator();
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);
    editMenu->addAction(cutAction);
    editMenu->addAction(deleteAction);
    editMenu->addSeparator();
    editMenu->addAction(preferencesAction);

    modelMenu->addAction(restrictedAction);
    modelMenu->addAction(manufactAction);
    modelMenu->addSeparator();
    modelMenu->addAction(addManuAction);
    modelMenu->addAction(removeManuAction);
    modelMenu->addSeparator();
    modelMenu->addAction(inventoryAction);
    modelMenu->addSeparator();
    modelMenu->addAction(glViewAction);

    aboutMenu->addAction(aboutAction);
    aboutMenu->addAction(helpAction);



    this->window->getMainMenuBar()->addMenu(fileMenu);
    this->window->getMainMenuBar()->addMenu(editMenu);
    this->window->getMainMenuBar()->addMenu(modelMenu);
    this->window->getMainMenuBar()->addMenu(aboutMenu);

    //MENU FILE
    //new
    //open
    //save
    //recent >
    //--
    //close
    //--
    //export
    //--
    //exit


    //MENU EDIT
    //undo
    //redo
    //repeat last track
    //--
    //cut
    //copy
    //paste
    //delete
    //--
    //Preferences

    //MENU MODEL
    //restricted mode - checkbox
    //manufacturers combination - checkbox
    //--
    //Add manufacturer
    //Remove manufacturer
    //--
    //Inventory
    //--
    //3D View

    //MENU ABOUT
    //help
    //about


    //MAIN CONTEXT MENU
    //copy
    //cut
    //paste
    //delete
    //--
    //restricted mode
    //manufacturers combo

    this->window->setMainContextMenu(new QMenu(this->window));
    QMenu * temp_ptr = this->window->getMainContextMenu();
    temp_ptr->addAction(copyAction);
    temp_ptr->addAction(cutAction);
    temp_ptr->addAction(pasteAction);
    temp_ptr->addAction(deleteAction);
    temp_ptr->addSeparator();
    temp_ptr->addAction(restrictedAction);
    temp_ptr->addAction(manufactAction);
    temp_ptr=NULL;


    this->window->setMainToolBar(new QToolBar(this->window));
    this->window->addToolBar(this->window->getMainToolBar());
    this->window->getMainToolBar()->addAction(undoAction);
    this->window->getMainToolBar()->addAction(redoAction);
    this->window->getMainToolBar()->addAction(endPointToggleAction);


    this->window->setMainStatusBar(new QStatusBar(this->window));
    this->window->setStatusBar(this->window->getMainStatusBar());
    this->window->getMainStatusBar()->showMessage("Hello world!");


    this->window->resize(700,500);

    //this->window->setWorkspaceWidget(new WorkspaceWidget(this->window));


    QFrame * central = new QFrame(this->window);
    this->window->setCentralWidget(central);
    QGridLayout * layout = new QGridLayout(central);

    QMenu * sideBarContextMenu = new QMenu(this->window);
    sideBarContextMenu->addAction(restrictedAction);
    sideBarContextMenu->addAction(manufactAction);

    this->window->setSideBarWidget(new SideBarWidget(this->getAppData()->getDatabase(),sideBarContextMenu,this->window->centralWidget()));
    this->window->setWorkspaceWidget(new WorkspaceWidget(this->window->getMainContextMenu(),this->window->centralWidget()));

    connect(endPointToggleAction,SIGNAL(triggered()),this->window->getWorkspaceWidget()->getGraphicsScene(),SLOT(toggleMode()));

    this->window->getSideBarWidget()->setMinimumWidth(120);
    layout->addWidget(this->window->getSideBarWidget(),0,0,1,1);
    layout->addWidget(this->window->getWorkspaceWidget(),0,1,1,2);
    this->window->setLayout(layout);

    return 0;
}

Window::Window() : QMainWindow()
{
    this->mainContextMenu=NULL;
    this->mainMenuBar=NULL;
    this->mainStatusBar=NULL;
    this->mainToolBar=NULL;
    this->workspace=NULL;
    this->sideBar=NULL;
    this->workspace=NULL;

}

QMenu * Window::getMainContextMenu() const
{
    return this->mainContextMenu;
}
QMenuBar * Window::getMainMenuBar() const
{
    return this->mainMenuBar;
}
QStatusBar * Window::getMainStatusBar() const
{
    return this->mainStatusBar;
}
QToolBar * Window::getMainToolBar() const
{
    return this->mainToolBar;
}
WorkspaceWidget * Window::getWorkspaceWidget() const
{
    return this->workspace;
}
SideBarWidget * Window::getSideBarWidget() const
{
    return this->sideBar;
}

bool Window::setMainContextMenu(QMenu * menu)
{
    if (menu==NULL)
        return false;
    this->mainContextMenu = menu;
    return true;
}
bool Window::setMainMenuBar(QMenuBar * menu)
{
    if(menu==NULL)
        return false;
    this->mainMenuBar=menu;
    return true;
}
bool Window::setMainStatusBar(QStatusBar * sb)
{
    if(sb==NULL)
        return false;
    this->mainStatusBar=sb;
    return true;
}
bool Window::setMainToolBar(QToolBar * toolBar)
{
    if(toolBar==NULL)
        return false;
    this->mainToolBar=toolBar;
    return true;
}
bool Window::setWorkspaceWidget(WorkspaceWidget * ws)
{
    if (ws==NULL)
        return false;
    this->workspace = ws;
    return true;
}
bool Window::setSideBarWidget(SideBarWidget * sb)
{
    if (sb==NULL)
        return false;
    this->sideBar=sb;
    return true;
}
void Window::contextMenuEvent(QContextMenuEvent * evt)
{
    //QMainWindow::contextMenuEvent(evt);
    ///^^-causes problems - why?

    this->getMainContextMenu()->popup(evt->globalPos());
}
void Window::keyPressEvent(QKeyEvent * evt)
{
    //QMainWindow::keyPressEvent(evt);
    /**
      TODO
      */

    app->sendEvent(this->workspace,evt);
}

/*
void Window::keyReleaseEvent(QKeyEvent *evt)
{
    app->postEvent(this->workspace,evt);
}
*/

AppData::AppData()
{
    this->database =new Database();
    //this->newFilePixmap = new QPixmap("/etc/RTEditor/icons/..");
    this->newFilePixmap = new QPixmap("/media/sf_Shared_Virtual/Ikony/NewFile.png");
    this->openFilePixmap = new QPixmap("/media/sf_Shared_Virtual/Ikony/OpenFile.png");
    this->saveFilePixmap = new QPixmap("/media/sf_Shared_Virtual/Ikony/SaveFile.png");

    this->undoPixmap = new QPixmap("/media/sf_Shared_Virtual/Ikony/Undo.png");
    this->redoPixmap = new QPixmap("/media/sf_Shared_Virtual/Ikony/Redo.png");
}

QPixmap * AppData::getNewFilePixmap() const
{
    return this->newFilePixmap;
}
QPixmap * AppData::getOpenFilePixmap() const
{
    return this->openFilePixmap;
}
QPixmap * AppData::getSaveFilePixmap() const
{
    return this->saveFilePixmap;
}
QPixmap * AppData::getUndoPixmap() const
{
    return this->undoPixmap;
}
QPixmap * AppData::getRedoPixmap() const
{
    return this->redoPixmap;
}
Database * AppData::getDatabase() const
{
    return this->database;
}

Preferences::Preferences(QString *path)
{

}



