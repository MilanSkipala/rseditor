/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

    This file is a part of Rail & Slot Editor.
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

#include <QDesktopServices>
#include <QFileDialog>
#include <QFormLayout>
#include <QSignalMapper>
#include <QTreeView>
#include <QLineEdit>
#include "partsRelated.h"
#include "application.h"
#include "database.h"
#include "globalVariables.h"

Application::Application(int argc, char ** argv) : QApplication(argc, argv)
{
    logFile << "Application constructor" << endl;
    QString * userPref = new QString(qApp->applicationDirPath()); // load data from user.pref

#ifdef Q_OS_LINUX
    userPref->append("/RSEditor/user.pref");
#endif
#ifdef Q_OS_WIN
    userPref->append("\\RSEditor\\user.pref");
#endif

    this->userPreferences = new Preferences(userPref);
    this->preferencesDialog = NULL;

    this->appData = new AppData(*this->userPreferences->getLocale());
    this->restrictedInventoryMode=false;
    this->allowMixedProductLines=false;
    this->saveAsFlag=false;
    this->window = new Window();
    this->projectName=QString("Untitled");    
    this->setupUI();
    this->getWindow()->getSideBarWidget()->resetSideBar();
    delete userPref;

}

Application::~Application()
{
    logFile << "~Application";



    this->window->getWorkspaceWidget()->resetWorkspace();
    delete this->window;
    this->window=NULL;
    delete this->userPreferences;
    this->userPreferences=NULL;
    delete this->appData;
    app=NULL;
    logFile.close();
}

Window * Application::getWindow()
{return this->window;}
Preferences * Application::getUserPreferences()
{return this->userPreferences;}
AppData * Application::getAppData()
{return this->appData;}
bool Application::getRestrictedInventoryMode()
{return this->restrictedInventoryMode;}
bool Application::getAllowMixedProductLines()
{return this->allowMixedProductLines;}

bool Application::setAppData(AppData * d)
{
    if (d==NULL)
        return false;

    this->appData = d;
    return true;
}

void Application::showModelInfo()
{this->modelInfoDialog->show();}
QTreeView *Application::getTreeView()
{return this->treeView;}

bool Application::setRestrictedInventoryMode(bool mode)
{
    this->restrictedInventoryMode=mode;
    if (this->window->getSideBarWidget()->getCurrentScene()!=NULL)
        this->window->getSideBarWidget()->getCurrentScene()->update(this->window->getSideBarWidget()->getCurrentScene()->sceneRect());
    return true;
}
bool Application::setAllowMixedProductLines(bool mode)
{
    this->allowMixedProductLines=mode;
    return true;
}

void Application::save()
{


    QString qpath = this->projectName;

    if (this->userPreferences->getLocale()->contains("EN"))
    {
        if (this->projectName.endsWith("Untitled") || this->projectName.endsWith("Untitled*"))
            qpath = QFileDialog::getSaveFileName(this->window, "Save file", "", "*.rse");
        else if (this->saveAsFlag==true)
            qpath = QFileDialog::getSaveFileName(this->window, "Save as", "", "*.rse");
        if (!qpath.endsWith(".rse"))
            qpath.append(".rse");
    }
    else
    {
        if (this->projectName.endsWith("Untitled") || this->projectName.endsWith("Untitled*"))
            qpath = QFileDialog::getSaveFileName(this->window, "Uložit soubor", "", "*.rse");
        else if (this->saveAsFlag==true)
            qpath = QFileDialog::getSaveFileName(this->window, "Uložit jako", "", "*.rse");
        if (!qpath.endsWith(".rse"))
            qpath.append(".rse");
    }

    if (qpath=="" || qpath==".rse")
        return;

    QFile output(qpath);
    output.open(QFile::WriteOnly);
    QTextStream outputFile(&output);

    if (!output.isOpen())
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be saved").arg(qpath),QString("Nepodařilo se uložit soubor \"%1\".").arg(qpath));
        this->appData->getMessageDialog()->exec();
        return;
    }

    this->window->setWindowTitle(qpath.mid(qpath.lastIndexOf("/")+1,qpath.length()-qpath.lastIndexOf("/")).remove(".rse").prepend("Rail & Slot Editor - "));


    outputFile << "PROJECT DATA" << endl;
    outputFile << "Project name: " << qpath << endl;
    outputFile << "Loaded manufacturers:" << endl;
    for (int i = 0; i < this->window->getSideBarWidget()->getProductLines()->count(); i++)
        outputFile << this->window->getSideBarWidget()->getProductLines()->itemText(i) << endl;

    outputFile << "Active manufacturer:" << endl;
    outputFile << this->window->getSideBarWidget()->getProductLines()->currentText() << endl;

    outputFile << "WORKSPACE DATA" << endl;
    this->window->getWorkspaceWidget()->exportCurrentState(outputFile);

    this->projectName=qpath;
    this->userPreferences->addProject(&qpath);


    app->getWindow()->getSideBarWidget()->printInventoryState(outputFile);

    output.close();

    if (this->getUserPreferences()->getLocale()->startsWith("EN"))
        this->window->statusBar()->showMessage("File was saved succesfully.");
    else
        this->window->statusBar()->showMessage("Soubor byl úspěšně uložen.");


}

void Application::saveAs()
{
    this->saveAsFlag=true;
    this->save();
    this->saveAsFlag=false;
}

void Application::open()
{

    QString qpath;
    if (this->getUserPreferences()->getLocale()->startsWith("EN"))
        qpath = QFileDialog::getOpenFileName(this->window, tr("Open File"),
                                 "",
                                 tr("Rail & Slot Editor files (*.rse)"));
                                 //"*.rse");
    else
        qpath = QFileDialog::getOpenFileName(this->window, tr("Otevřít"),
                                 "",
                                 tr("Rail & Slot Editor files (*.rse)"));
                                 //"*.rse");

    if (qpath=="")
        return;

    QFile input(qpath);

    input.open(QFile::ReadOnly);

    QTextStream inputFile(&input);

    if (!input.isOpen())
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be opened").arg(qpath),QString("Nepodařilo se otevřít soubor \"%1\".").arg(qpath));
        this->appData->getMessageDialog()->exec();
        return;
    }

    if (this->window->getWorkspaceWidget()->unsavedChangesFlag())
    {
        QMessageBox * unsaved = new QMessageBox(this->window);
         if (this->userPreferences->getLocale()->contains("EN"))
            unsaved->setText(QString("There are some unsaved changes in \"%1\".\n Would you like to save the file?").arg(this->projectName));
        else
            unsaved->setText(QString("Soubor \"%1\" obsahuje neuložené změny.\n Přejete si soubor uložit?").arg(this->projectName));
        QPushButton * saveB = unsaved->addButton(QMessageBox::Yes);
        QPushButton * dontSaveB = unsaved->addButton(QMessageBox::No);
        QPushButton * cancelB = unsaved->addButton(QMessageBox::Cancel);

        unsaved->exec();
        QPushButton * clicked = (QPushButton*)unsaved->clickedButton();
        if (clicked==saveB)
            this->save();
        else if (clicked==dontSaveB)
        {}
        else if (clicked==cancelB)
            return;



    }


    QString activeProdLine;
    QString qstr;
    qstr = inputFile.readLine();
    if (!qstr.startsWith("PROJECT DATA"))
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be read").arg(qpath),QString("Nepodařilo se přečíst soubor \"%1\".").arg(qpath));
        this->appData->getMessageDialog()->exec();
        return;
    }



    bool readProdLines = false;
    while(!qstr.startsWith("WORKSPACE DATA"))
    {

        if (qstr.startsWith("Loaded manufacturers:"))
        {
            readProdLines = true;
            qstr = inputFile.readLine();
        }
        else if (qstr.startsWith("Active manufacturer"))
        {
            readProdLines = false;
            qstr = inputFile.readLine();
            activeProdLine=qstr;
        }
        if (readProdLines)
        {
            bool insert = true;
            for (int i = 0; i < this->window->getSideBarWidget()->getProductLines()->count();i++)
            {
                if (!this->window->getSideBarWidget()->getProductLines()->itemText(i).contains(qstr))
                    insert = false;
            }
            if (insert)
                this->window->getSideBarWidget()->getProductLines()->addItem(qstr);
        }

        qstr = inputFile.readLine();
    }
    this->window->getSideBarWidget()->getProductLines()->setCurrentText(activeProdLine);

    this->userPreferences->addProject(&qpath);
    int retVal = this->window->getWorkspaceWidget()->setCurrentState(inputFile);
    if (retVal<0)
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be loaded. Some items are missing in your database.").arg(qpath),QString("Nepodařilo se načíst soubor \"%1\". Některé díly chybí ve vaší databázi.").arg(qpath));
        this->appData->getMessageDialog()->exec();

        if (this->getUserPreferences()->getLocale()->startsWith("EN"))
            this->window->statusBar()->showMessage("File couldn't be loaded. Some items are not available in your database.");
        else
            this->window->statusBar()->showMessage("Nepodařilo se načíst soubor. V databázi chybí některé díly použité v souboru.");
        logFile << "ERROR: Missing items in database" << endl;
        this->window->getWorkspaceWidget()->resetWorkspace();

        return;
    }
    else if (retVal > 0)
    {
        this->appData->setMessageDialogText(QString("Error occured during loading of the file \"%1\".").arg(qpath),QString("Při načítání souboru \"%1\" došlo k chybě.").arg(qpath));
        this->appData->getMessageDialog()->exec();

        if (this->getUserPreferences()->getLocale()->startsWith("EN"))
            this->window->statusBar()->showMessage("File was not loaded in the correct way.");
        else
            this->window->statusBar()->showMessage("Nepodařilo se správně načíst soubor.");
        logFile << "ERROR: retVal=" << retVal << endl;
    }
    this->window->getSideBarWidget()->setInventoryState(inputFile);

    this->projectName=qpath;
    this->window->setWindowTitle(qpath.mid(qpath.lastIndexOf("/")+1).prepend("Rail & Slot Editor - "));

    input.close();

    if (this->getUserPreferences()->getLocale()->startsWith("EN"))
        this->window->statusBar()->showMessage("File was opened succesfully.");
    else
        this->window->statusBar()->showMessage("Soubor byl úspěšně otevřen.");
}

void Application::openLast(int index)
{
    QString qpath = *this->userPreferences->getLastProjects()[index];
    if (qpath=="")
        return;

    QFile input(qpath);

    input.open(QFile::ReadOnly);

    QTextStream inputFile(&input);


    if (!input.isOpen())
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be opened").arg(qpath),QString("Nepodařilo se otevřít soubor \"%1\".").arg(qpath));
        this->appData->getMessageDialog()->exec();
        return;
    }

    if (this->window->getWorkspaceWidget()->unsavedChangesFlag())
    {
        QMessageBox * unsaved = new QMessageBox(this->window);
        if (this->userPreferences->getLocale()->contains("EN"))
            unsaved->setText(QString("There are some unsaved changes in \"%1\".\n Would you like to save the file?").arg(this->projectName));
        else
            unsaved->setText(QString("Soubor \"%1\" obsahuje neuložené změny.\n Přejete si soubor uložit?").arg(this->projectName));
        QPushButton * saveB = unsaved->addButton(QMessageBox::Yes);
        QPushButton * dontSaveB = unsaved->addButton(QMessageBox::No);
        QPushButton * cancelB = unsaved->addButton(QMessageBox::Cancel);

        unsaved->exec();
        QPushButton * clicked = (QPushButton*)unsaved->clickedButton();
        if (clicked==saveB)
            this->save();
        else if (clicked==dontSaveB)
        {}
        else if (clicked==cancelB)
            return;
    }

    QString activeProdLine;
    QString qstr;
    qstr = inputFile.readLine();
    if (!qstr.startsWith("PROJECT DATA"))
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be read").arg(qpath),QString("Nepodařilo se přečíst soubor \"%1\".").arg(qpath));
        this->appData->getMessageDialog()->exec();
        return;
    }



    bool readProdLines = false;
    while(!qstr.startsWith("WORKSPACE DATA"))
    {

        if (qstr.startsWith("Loaded manufacturers:"))
        {
            readProdLines = true;
            qstr = inputFile.readLine();
        }
        else if (qstr.startsWith("Active manufacturer"))
        {
            readProdLines = false;
            qstr = inputFile.readLine();
            activeProdLine=qstr;
        }
        if (readProdLines)
        {
            bool insert = true;
            for (int i = 0; i < this->window->getSideBarWidget()->getProductLines()->count();i++)
            {
                if (!this->window->getSideBarWidget()->getProductLines()->itemText(i).contains(qstr))
                    insert = false;
            }
            if (insert)
                this->window->getSideBarWidget()->getProductLines()->addItem(qstr);
        }

        qstr = inputFile.readLine();
    }
    this->window->getSideBarWidget()->getProductLines()->setCurrentText(activeProdLine);
    this->userPreferences->addProject(&qpath);
    int retVal = this->window->getWorkspaceWidget()->setCurrentState(inputFile);
    if (retVal<0)
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be loaded. Some items are missing in your database.").arg(qpath),QString("Nepodařilo se načíst soubor \"%1\". Některé díly chybí ve vaší databázi.").arg(qpath));
        this->appData->getMessageDialog()->exec();

        if (this->getUserPreferences()->getLocale()->startsWith("EN"))
            this->window->statusBar()->showMessage("File couldn't be loaded. Some items are not available in your database.");
        else
            this->window->statusBar()->showMessage("Nepodařilo se načíst soubor. V databázi chybí některé díly použité v souboru.");
        logFile << "ERROR: Missing items in database" << endl;
        this->window->getWorkspaceWidget()->resetWorkspace();

        return;
    }
    else if (retVal > 0)
    {
        this->appData->setMessageDialogText(QString("Error occured during loading of the file \"%1\".").arg(qpath),QString("Při načítání souboru \"%1\" došlo k chybě.").arg(qpath));
        this->appData->getMessageDialog()->exec();

        if (this->getUserPreferences()->getLocale()->startsWith("EN"))
            this->window->statusBar()->showMessage("File was not loaded in the correct way.");
        else
            this->window->statusBar()->showMessage("Nepodařilo se správně načíst soubor.");
        logFile << "ERROR: retVal=" << retVal << endl;
    }
    this->projectName=qpath;
    this->window->getSideBarWidget()->setInventoryState(inputFile);

    this->window->setWindowTitle(qpath.mid(qpath.lastIndexOf("/")+1).prepend("Rail & Slot Editor - "));

    input.close();
}

void Application::newFile()
{
    if (this->window->getWorkspaceWidget()->unsavedChangesFlag())
    {
        QMessageBox * unsaved = new QMessageBox(this->window);
        if (this->userPreferences->getLocale()->contains("EN"))
            unsaved->setText(QString("There are some unsaved changes in \"%1\".\n Would you like to save the file?").arg(this->projectName));
        else
            unsaved->setText(QString("Soubor \"%1\" obsahuje neuložené změny.\n Přejete si soubor uložit?").arg(this->projectName));
        QPushButton * saveB = unsaved->addButton(QMessageBox::Yes);
        QPushButton * dontSaveB = unsaved->addButton(QMessageBox::No);
        QPushButton * cancelB = unsaved->addButton(QMessageBox::Cancel);

        unsaved->exec();
        QPushButton * clicked = (QPushButton*)unsaved->clickedButton();
        if (clicked==saveB)
            this->save();
        else if (clicked==dontSaveB)
        {}
        else if (clicked==cancelB)
            return;
    }
    logFile << "newFile will be created" << endl;
    this->getWindow()->getSideBarWidget()->resetSideBar();
    logFile << "sideBar has been succesfully reset" << endl;
    this->getWindow()->getWorkspaceWidget()->resetWorkspace();
    logFile << "workspace has been succesfully reset" << endl;
    this->projectName=QString("Untitled");

    QDialog * sceneSizeDialog = new QDialog(this->window);
    sceneSizeDialog->setWindowTitle("Set workspace dimensions");
    QFormLayout * sceneSizeLayout = new QFormLayout(sceneSizeDialog);
    QPushButton * confirm = new QPushButton("Confirm",sceneSizeDialog);
    connect(confirm,SIGNAL(released()),sceneSizeDialog,SLOT(accept()));

    QLineEdit * wTextField = new QLineEdit("4000");
    QLineEdit * hTextField = new QLineEdit("3000");
    wTextField->setInputMask("D00000");
    hTextField->setInputMask("D00000");



    qreal w = 4000;
    qreal h = 3000;

    if (this->getUserPreferences()->getLocale()->startsWith("EN"))
    {
        sceneSizeLayout->addRow("Workspace width [mm]:",wTextField);
        sceneSizeLayout->addRow("Workspace height [mm]:",hTextField);
        sceneSizeLayout->addRow("",confirm);

    }
    else
    {
        sceneSizeLayout->addRow("Šířka pracovní plochy [mm]:",wTextField);
        sceneSizeLayout->addRow("Výška pracovní plochy [mm]:",hTextField);
        confirm->setText("OK");
        sceneSizeDialog->setWindowTitle("Nastavit rozměry pracovní plochy");
        sceneSizeLayout->addRow("",confirm);
    }

    int ret = sceneSizeDialog->exec();
    if (ret==QDialog::Accepted)
    {
        w =(wTextField)->text().toDouble();
        h = (hTextField)->text().toDouble();
    }


    this->getWindow()->getWorkspaceWidget()->getGraphicsScene()->setSceneRect(-w/2,-h/2,w,h);
    this->getWindow()->setWindowTitle("Rail & Slot Editor - Untitled");


}

void Application::saveModelInfo()
{
    QString qpath;
    qpath = QFileDialog::getSaveFileName(this->window, "Export", "", "*.htm");

    if (!qpath.endsWith(".htm"))
        qpath.append(".htm");

    if (qpath=="")
        return;

    QFile output(qpath);
    output.open(QFile::WriteOnly);

    QTextStream outputFile(&output);

    if (!output.isOpen())//.is_open())
    {
        this->appData->setMessageDialogText(QString("The file \"%1\" could not be saved").arg(qpath),QString("Nepodařilo se uložit soubor \"%1\".").arg(qpath));
        this->appData->getMessageDialog()->exec();
        return;
    }


    if (this->getUserPreferences()->getLocale()->startsWith("EN"))
    {
        outputFile << QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n<html>\n<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n<meta name=\"generator\" content=\"Rail & Slot Editor\">\n");
        outputFile << QString("<title>Rail & Slot Editor - Parts list</title>\n</head>\n<body>\n<table border=\"1\">\n");
    }
    else
    {
        outputFile << QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n<html>\n<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=windows-1250\">\n<meta name=\"generator\" content=\"Rail & Slot Editor\">\n");
        outputFile << QString("<title>Rail & Slot Editor - Seznam dílů</title>\n</head>\n<body>\n<table border=\"1\">\n");
    }


    QAbstractItemModel * model = this->treeView->model();

    for (int i = 0; i < model->rowCount();i++)
    {
        outputFile << "<tr>" << endl;
        outputFile << "<td>" << endl;
        outputFile << model->data(model->index(i,0)).toString() << endl;
        outputFile << "</td>" << endl;
        outputFile << "<td colspan=\"3\">&nbsp" << endl;
        outputFile << "</td>" << endl;
        outputFile << "</tr>" << endl;


        int j = 0;
        while (model->index(i,0).child(j,0).isValid())
        {
            outputFile << "<tr>" << endl;
            outputFile << "<td>&nbsp" << endl;
            outputFile << "</td>" << endl;
            outputFile << "<td>" << endl;
            outputFile << model->data(model->index(i,0).child(j,0)).toString().toUtf8() << endl;
            outputFile << "</td>" << endl;
            outputFile << "<td>&nbsp" << endl;
            outputFile << "</td>" << endl;
            outputFile << "<td>" << endl;
            outputFile << model->data(model->index(i,0).child(j,0).child(0,0)).toString().toUtf8() << endl;
            outputFile << "</td>" << endl;
            outputFile << "</tr>" << endl;
            j++;
        }


    }

    outputFile << QString("</table>\n</body>\n</html>\n");


}

void Application::savePreferences()
{


    QList<void*> ptrs;
    for (int i = 0; i < 8;i++)
    {
        ptrs.push_back(((QFormLayout*)this->preferencesDialog->layout())->itemAt(i)->widget());
        //cout << ((QFormLayout*)this->preferencesDialog->layout())->itemAt(i)->widget()->metaObject()->className() << endl;
    }
    QString langStr = ((QComboBox*)this->preferencesDialog->layout()->itemAt(1)->widget())->currentText();

    bool savePos = ((QCheckBox*)this->preferencesDialog->layout()->itemAt(3)->widget())->isChecked();
    bool largeIcons = ((QCheckBox*)this->preferencesDialog->layout()->itemAt(5)->widget())->isChecked();


    if (langStr.startsWith("EN"))
        langStr="EN";
    else
        langStr="CS";

    this->userPreferences->setLocale(&langStr);

    if(savePos)
        this->userPreferences->setSaveScenePosFlag(true);
    else
        this->userPreferences->setSaveScenePosFlag(false);

    if (largeIcons)
        this->userPreferences->setSmallIconsFlag(true);
    else
        this->userPreferences->setSmallIconsFlag(false);

    this->preferencesDialog->close();

}

void Application::saveInventory()
{
#ifdef Q_OS_LINUX
    QString path(folderPathLinux);
#endif


#ifdef Q_OS_WIN
    QString path(folderPathWin);
#endif

    path.append("Inventory.rsi");
    QFile f(path);

    f.open(QFile::WriteOnly);

    QTextStream outputFile(&f);

    if (f.isOpen())
        this->window->getSideBarWidget()->printInventoryState(outputFile);
    else
    {
        logFile << "ERROR: Couldn't save inventory" << endl;
    }

}

void Application::displayHelp()
{
    if (this->userPreferences->getLocale()->startsWith("EN"))
        QDesktopServices::openUrl(QUrl::fromLocalFile(this->applicationDirPath().append("/Rail & Slot Editor - Quick start.pdf")));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(this->applicationDirPath().append("/Rail & Slot Editor - Začínáme.pdf")));
    logFile << "displayHelp()" << endl;
}

int Application::setupUI()
{

    this->window->setMainMenuBar(new QMenuBar(this->window));
    this->window->setMenuBar(this->window->getMainMenuBar());
    this->window->setWindowTitle(QString("Rail & Slot Editor - %1").arg(this->projectName));


    //MENU FILE
    QMenu * fileMenu = new QMenu("File", this->window->getMainMenuBar());
    QMenu * recentFiles = new QMenu ("Recent projects",fileMenu);
    QMenu * editMenu = new QMenu("Edit",this->window->getMainMenuBar());
    QMenu * modelMenu = new QMenu("Model",this->window->getMainMenuBar());
    QMenu * aboutMenu = new QMenu("About", this->window->getMainMenuBar());

    QAction * newFileAction = new QAction(*this->appData->getNewFilePixmap(),"New Project",fileMenu);
    newFileAction->setShortcut(QKeySequence("Ctrl+N"));
    QAction * openFileAction = new QAction(*this->appData->getOpenFilePixmap(),"Open Project",fileMenu);
    openFileAction->setShortcut(QKeySequence("Ctrl+O"));

    QAction * saveFileAction = new QAction(*this->appData->getSaveFilePixmap(),"Save Project",fileMenu);
    saveFileAction->setShortcut(QKeySequence("Ctrl+S"));
    QAction * saveAsAction = new QAction(*this->appData->getSaveFilePixmap(),"Save Project As",fileMenu);
    saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));

    QAction * recent1 = new QAction ((*this->userPreferences->getLastProjects()[0]),recentFiles);
    QAction * recent2 = new QAction ((*this->userPreferences->getLastProjects()[1]),recentFiles);
    QAction * recent3 = new QAction ((*this->userPreferences->getLastProjects()[2]),recentFiles);
    QAction * recent4 = new QAction ((*this->userPreferences->getLastProjects()[3]),recentFiles);
    QAction * recent5 = new QAction ((*this->userPreferences->getLastProjects()[4]),recentFiles);

    this->userPreferences->setActions(recent1,recent2,recent3,recent4,recent5);

    QSignalMapper * signalMapper = new QSignalMapper(this);
    connect(recent1,SIGNAL(triggered()),signalMapper,SLOT(map()));
    connect(recent2,SIGNAL(triggered()),signalMapper,SLOT(map()));
    connect(recent3,SIGNAL(triggered()),signalMapper,SLOT(map()));
    connect(recent4,SIGNAL(triggered()),signalMapper,SLOT(map()));
    connect(recent5,SIGNAL(triggered()),signalMapper,SLOT(map()));
    signalMapper->setMapping(recent1,0);
    signalMapper->setMapping(recent2,1);
    signalMapper->setMapping(recent3,2);
    signalMapper->setMapping(recent4,3);
    signalMapper->setMapping(recent5,4);
    connect(signalMapper,SIGNAL(mapped(int)),this,SLOT(openLast(int)));

    QAction * exportAction = new QAction("Export 2D graphic",fileMenu);
    QAction * saveInventoryAction = new QAction("Save inventory",fileMenu);

    QAction * quitAction = new QAction("Exit", fileMenu);

    QAction * undoAction = new QAction(*this->getAppData()->getUndoPixmap(),"Undo",editMenu);
    undoAction->setShortcut(QKeySequence("Ctrl+Z"));
    QAction * redoAction = new QAction(*this->getAppData()->getRedoPixmap(),"Redo",editMenu);
    QList<QKeySequence> l;
    l.push_back(QKeySequence("Ctrl+Y"));
    l.push_back(QKeySequence("Ctrl+Shift+Z"));


    redoAction->setShortcuts(l);

    QAction * repeatAction = new QAction(*this->getAppData()->getRepeatPixmap(),"Insert last item again",editMenu);
    repeatAction->setShortcut(QKeySequence("Space"));

    QAction * copyAction = new QAction(*this->getAppData()->getCopyPixmap(),"Copy",editMenu);
    copyAction->setShortcut(QKeySequence("Ctrl+C"));
    QAction * pasteAction = new QAction(*this->getAppData()->getPastePixmap(),"Paste",editMenu);
    pasteAction->setShortcut(QKeySequence("Ctrl+V"));
    QAction * cutAction = new QAction(*this->getAppData()->getCutPixmap(),"Cut",editMenu);
    cutAction->setShortcut(QKeySequence("Ctrl+X"));
    QAction * deleteAction = new QAction(*this->getAppData()->getDeletePixmap(),"Delete",editMenu);
    deleteAction->setShortcut(QKeySequence("Delete"));
    QAction * preferencesAction = new QAction(*this->getAppData()->getPreferencesPixmap(),"Preferences",editMenu);


    QAction * restrictedAction = new QAction("Restricted mode",modelMenu);
    QAction * manufactAction = new QAction("Mix manufacturers",modelMenu);
    QAction * modelOverviewAction = new QAction(*this->getAppData()->getModelOverviewPixmap(),"Model overview",modelMenu);
    //QAction * glViewAction = new QAction(*this->getAppData()->getUndoPixmap(),"3D View",modelMenu);
    restrictedAction->setCheckable(true);
    manufactAction->setCheckable(true);
    restrictedAction->setChecked(false);
    manufactAction->setChecked(false);
    connect(restrictedAction,SIGNAL(toggled(bool)),this,SLOT(setRestrictedInventoryMode(bool)));
    connect(manufactAction,SIGNAL(toggled(bool)),this,SLOT(setAllowMixedProductLines(bool)));



    QAction * aboutAction = new QAction(*this->getAppData()->getAboutPixmap(),"About", aboutMenu);
    QAction * helpAction = new QAction(*this->getAppData()->getHelpPixmap(),"Help", aboutMenu);

    QAction * rotationToggleAction = new QAction(*this->getAppData()->getRotateToolPixmap(),"Rotation mode",NULL);
    rotationToggleAction->setCheckable(true);
    rotationToggleAction->setShortcut(QKeySequence("R"));

    QAction * heightProfileToggleAction = new QAction(*this->getAppData()->getHeightProfilePixmap(),"Height profile mode",NULL);
    heightProfileToggleAction->setCheckable(true);

    QAction * heightProfileUpAction = new QAction(*this->getAppData()->getHeightProfileUpPixmap(),"Increase height",NULL);
    QAction * heightProfileDownAction = new QAction(*this->getAppData()->getHeightProfileDownPixmap(),"Decrease height",NULL);

    QAction * bendAction = new QAction(*this->getAppData()->getBendPixmap(),"Bend and close the track",NULL);
    bendAction->setCheckable(true);

    QAction * completeAction = new QAction(*this->getAppData()->getCompletePixmap(),"Complete track",NULL);

    completeAction->setCheckable(true);



//other QActions

    recentFiles->addAction(recent1);
    recentFiles->addAction(recent2);
    recentFiles->addAction(recent3);
    recentFiles->addAction(recent4);
    recentFiles->addAction(recent5);

    fileMenu->addAction(newFileAction);
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(saveFileAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addMenu(recentFiles);
    fileMenu->addSeparator();
    fileMenu->addSeparator();
    fileMenu->addAction(exportAction);
    fileMenu->addAction(saveInventoryAction);
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
    modelMenu->addSeparator();
    modelMenu->addAction(modelOverviewAction);
    modelMenu->addSeparator();

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
    //Model overview
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

    rotationToggleAction->setParent(this->window->getMainToolBar());
    heightProfileToggleAction->setParent(this->window->getMainToolBar());
    heightProfileUpAction->setParent(this->window->getMainToolBar());
    heightProfileDownAction->setParent(this->window->getMainToolBar());
    bendAction->setParent(this->window->getMainToolBar());
    completeAction->setParent(this->window->getMainToolBar());

    this->window->getMainToolBar()->addAction(undoAction);
    this->window->getMainToolBar()->addAction(redoAction);
    this->window->getMainToolBar()->addSeparator();
    this->window->getMainToolBar()->addAction(rotationToggleAction);
    this->window->getMainToolBar()->addSeparator();
    this->window->getMainToolBar()->addAction(heightProfileToggleAction);
    this->window->getMainToolBar()->addAction(heightProfileUpAction);
    this->window->getMainToolBar()->addAction(heightProfileDownAction);
    this->window->getMainToolBar()->addAction(bendAction);
    this->window->getMainToolBar()->addAction(completeAction);


    this->window->setMainStatusBar(new QStatusBar(this->window));
    this->window->setStatusBar(this->window->getMainStatusBar());
    this->window->getMainStatusBar()->showMessage("Welcome! Start by pressing \"Add\" button to import parts library.");

    if (!this->userPreferences->getSmallIconsFlag())
        this->window->getMainToolBar()->setIconSize(QSize(64,64));

    this->window->resize(700,500);

    QSplitter * central = new QSplitter(Qt::Horizontal,this->window);

    this->window->setCentralWidget(central);


    QMenu * sideBarContextMenu = new QMenu(this->window);
    sideBarContextMenu->addAction(restrictedAction);
    sideBarContextMenu->addAction(manufactAction);

    QMenu * wswContextMenu = new QMenu(this->window);
    wswContextMenu->addAction(pasteAction);
    wswContextMenu->addAction(restrictedAction);
    wswContextMenu->addAction(manufactAction);
    wswContextMenu->addSeparator();
    wswContextMenu->addAction(rotationToggleAction);
    wswContextMenu->addAction(heightProfileToggleAction);
    wswContextMenu->addAction(heightProfileUpAction);
    wswContextMenu->addAction(heightProfileDownAction);
    wswContextMenu->addSeparator();
    wswContextMenu->addAction(bendAction);
    wswContextMenu->addAction(completeAction);

    this->window->setSideBarWidget(new SideBarWidget(this->getUserPreferences()->getLocale(), this->getAppData()->getDatabase(),sideBarContextMenu,this->window->centralWidget()));
    this->window->setWorkspaceWidget(new WorkspaceWidget(wswContextMenu,this->window->centralWidget()));

    //connect(endPointToggleAction,SIGNAL(triggered()),this->window->getWorkspaceWidget()->getGraphicsScene(),SLOT(toggleMode()));
    connect(rotationToggleAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(toggleRotationMode()));
    connect(heightProfileToggleAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(toggleHeightProfileMode()));
    connect(heightProfileUpAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(adjustHeightOfActive()));
    connect(heightProfileDownAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(adjustHeightOfActive()));
    connect(bendAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(selectFragmentToClose()));
    connect(completeAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(selectFragmentToComplete()));
    connect(repeatAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(makeLastItem()));
    connect(deleteAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(removeItems()));

    connect(copyAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(copy()));
    connect(pasteAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(paste()));
    connect(cutAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(cut()));

    connect(undoAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(undo()));
    connect(redoAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(redo()));
    connect(saveFileAction,SIGNAL(triggered()),this,SLOT(save()));
    connect(saveAsAction,SIGNAL(triggered()),this,SLOT(saveAs()));
    connect(openFileAction,SIGNAL(triggered()),this,SLOT(open()));
    connect(newFileAction,SIGNAL(triggered()),this,SLOT(newFile()));



    connect(helpAction,SIGNAL(triggered()),this,SLOT(displayHelp()));

    QDialog * overviewDialog = new QDialog(this->window);
    QGridLayout * gridLayout = new QGridLayout(overviewDialog);
    QTreeView * treeView = new QTreeView(this->window);
    this->treeView=treeView;
    overviewDialog->setWindowTitle("Model overview");

    if (*this->userPreferences->getLocale()=="CS")
    {
        fileMenu->setTitle("Soubor");
        recentFiles->setTitle("Naposledy otevřené");
        editMenu->setTitle("Úpravy");
        aboutMenu->setTitle("Nápověda");
        newFileAction->setText("Nový");
        openFileAction->setText("Otevřít");
        saveFileAction->setText("Uložit");
        saveAsAction->setText("Uložit jako...");
        exportAction->setText("Export obrazu");
        saveInventoryAction->setText("Uložit stav inventáře");
        quitAction->setText("Ukončit program");
        undoAction->setText("Zpět");
        redoAction->setText("Znovu");
        repeatAction->setText("Vložit ");
        copyAction->setText("Kopírovat");
        cutAction->setText("Vyjmout");
        pasteAction->setText("Vložit");
        deleteAction->setText("Smazat");
        preferencesAction->setText("Možnosti");
        restrictedAction->setText("Omezený režim");
        manufactAction->setText("Povolit míchání výrobců");
        //addManuAction->setText("Přidat výrobce");
        //removeManuAction->setText("Smazat výrobce");
        modelOverviewAction->setText("Informace o modelu");
        aboutAction->setText("O aplikaci");
        helpAction->setText("Nápověda");
        rotationToggleAction->setText("Režim otáčení");
        heightProfileToggleAction->setText("Režim výšky trati");
        heightProfileUpAction->setText("Zvětšit výšku");
        heightProfileDownAction->setText("Zmenšit výšku");
        bendAction->setText("Ohnout a spojit trať");
        completeAction->setText("Dokončit trať");
        this->window->getMainStatusBar()->showMessage("Vítejte! Začněte kliknutím na tlačítko \"Přidat\" pro přidání výrobců modelových dílů");
        overviewDialog->setWindowTitle("Informace o modelu");

        newFileAction->setStatusTip("Vytvořit nový projekt");
        openFileAction->setStatusTip("Otevřít existující projekt");
        saveFileAction->setStatusTip("Uložit změny");
        saveAsAction->setStatusTip("Uložit projekt jako...");
        exportAction->setStatusTip("Exportovat obsah pracovní plochy do obrázku");
        saveInventoryAction->setStatusTip("Uložit stav inventáře. Bude přepsán aktuálně uložený stav.");
        quitAction->setStatusTip("Ukončit aplikaci");
        undoAction->setStatusTip("Zpět");
        redoAction->setStatusTip("Znovu");
        repeatAction->setStatusTip("Vložit znovu naposledy použitý tratový díl");
        copyAction->setStatusTip("Kopírovat vybrané dílky");
        cutAction->setStatusTip("Vyjmout vybrané dílky");
        pasteAction->setStatusTip("Vložit vybrané dílky");
        deleteAction->setStatusTip("Smazat vybrané dílky");
        preferencesAction->setStatusTip("Uživatelská nastavení");
        restrictedAction->setStatusTip("Zapnout/Vypnout režim s omezeným množstvím dílků");
        manufactAction->setStatusTip("Zapnout/Vypnout režim s povoleným kombinováním různých výrobců v jednom úseku trati");
        //addManuAction->setStatusTip();
        //removeManuAction->setStatusTip();
        modelOverviewAction->setStatusTip("Zobrazit přehled použitých dílů v modelu");
        aboutAction->setStatusTip("O aplikaci");
        helpAction->setStatusTip("Nápověda");
        rotationToggleAction->setStatusTip("Zapnout/Vypnout režim otáčení");
        heightProfileToggleAction->setStatusTip("Zapnout/Vypnout režim pro úpravu výškového profilu trati");
        heightProfileUpAction->setStatusTip("Zvýšit vybraný díl ve vybraném bodě");
        heightProfileDownAction->setStatusTip("Snížit vybraný díl ve vybraném bodě");
        bendAction->setStatusTip("Pokusit se pomocí ohybu uzavřít model");
        completeAction->setStatusTip("Automaticky dopočítat podobu trati mezi vybranými body");

    }
    else
    {
        newFileAction->setStatusTip("Create new project");
        openFileAction->setStatusTip("Open existing project");
        saveFileAction->setStatusTip("Save changes");
        saveAsAction->setStatusTip("Save project as...");
        exportAction->setStatusTip("Export workspace content to image");
        saveInventoryAction->setStatusTip("Save state of the inventory. Current saved state will be overwritten.");
        quitAction->setStatusTip("Quit application");
        undoAction->setStatusTip("Undo action");
        redoAction->setStatusTip("Redo action");
        repeatAction->setStatusTip("Insert again last used track part");
        copyAction->setStatusTip("Copy selected parts");
        cutAction->setStatusTip("Cut selected parts");
        pasteAction->setStatusTip("Paste selected parts");
        deleteAction->setStatusTip("Delete selected parts");
        preferencesAction->setStatusTip("User preferences");
        restrictedAction->setStatusTip("Turn on/off restricted inventory mode");
        manufactAction->setStatusTip("Allow mixing of manufacturers in one track section");
        //addManuAction->setStatusTip();
        //removeManuAction->setStatusTip();
        modelOverviewAction->setStatusTip("Show overview of parts used in the model");
        aboutAction->setStatusTip("Information about application");
        helpAction->setStatusTip("Show help");
        rotationToggleAction->setStatusTip("Turn on/off rotation mode");
        heightProfileToggleAction->setStatusTip("Turn on/off height profile mode");
        heightProfileUpAction->setStatusTip("Increase the height of the item at selected point");
        heightProfileDownAction->setStatusTip("Decrease the height of the item at selected point");
        bendAction->setStatusTip("Try to close the track section by bending the track");
        completeAction->setStatusTip("Try to complete the track automatically");
    }


/*
    QSignalMapper * signalMapper = new QSignalMapper(this->window->getWorkspaceWidget());

    connect(modelOverviewAction,SIGNAL(triggered()),signalMapper,SLOT(map()));
    signalMapper->setMapping(modelOverviewAction,treeView);
    connect(signalMapper,SIGNAL(mapped(QWidget*)),this->window->getWorkspaceWidget(),SLOT(modelInfo(QTreeView*)));
*/
    connect(modelOverviewAction,SIGNAL(triggered()),this->window->getWorkspaceWidget(),SLOT(modelInfo()));


    //formLayout->addRow("Workspace size [mm]",new QLabel("00"));
    //formLayout->addRow("Model size [mm]",new QLabel("01"));
    treeView->setExpandsOnDoubleClick(true);
    //treeView->setMaximumHeight(2048);


    QPushButton * buttonOverview = new QPushButton("Export to file",overviewDialog);
    QPushButton * closeOverview = new QPushButton("Close",overviewDialog);

    if (*this->userPreferences->getLocale()=="CS")
    {
        buttonOverview->setText("Exportovat do souboru");
        closeOverview->setText("Zavřít");
    }

    gridLayout->addWidget(treeView,0,0,1,2);
    gridLayout->addWidget(buttonOverview,1,1,1,1);
    gridLayout->addWidget(closeOverview,1,0,1,1);

    connect(buttonOverview,SIGNAL(clicked()),this,SLOT(saveModelInfo()));
    connect(closeOverview,SIGNAL(clicked()),overviewDialog,SLOT(close()));
    connect(saveInventoryAction,SIGNAL(triggered()),this,SLOT(saveInventory()));
    connect(exportAction,SIGNAL(triggered()),this->getWindow()->getWorkspaceWidget(),SLOT(exportBitmap()));




    this->modelInfoDialog = overviewDialog;
    //this->modelInfoDialog->resize(this->modelInfoDialog->width()*2,this->window->height());



    this->window->getSideBarWidget()->setMinimumWidth(150);

    /**
    layout->addWidget(this->window->getSideBarWidget(),0,0,1,1);
    layout->addWidget(this->window->getWorkspaceWidget(),0,1,1,2);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    central->setLayout(layout);*/
    central->addWidget(this->window->getSideBarWidget());
    central->addWidget(this->window->getWorkspaceWidget());



    {
        //PREFERENCES DIALOG
        QDialog * prefDialog = new QDialog (this->window);
        QFormLayout * prefLayout = new QFormLayout(prefDialog);
        QPushButton * discard = new QPushButton("Discard",prefDialog);
        connect(discard,SIGNAL(clicked()),prefDialog,SLOT(close()));
        QPushButton * saveChanges = new QPushButton("Save changes",prefDialog);
        prefDialog->setWindowTitle("Preferences");

        if (this->userPreferences->getLocale()->contains("CS"))
        {
            prefDialog->setWindowTitle("Možnosti");
            discard->setText("Storno");
            saveChanges->setText("Uložit změny");

        }

        QComboBox * langCombo = new QComboBox(prefDialog);
        langCombo->addItem("EN - English");
        langCombo->addItem("CS - Czech");
        if (this->userPreferences->getLocale()->contains("EN"))
        {
            prefLayout->addRow("Select language", langCombo);
        }
        else
        {
            prefLayout->addRow("Vyberte jazyk", langCombo);

        }

        QCheckBox * chbSave = new QCheckBox(prefDialog);
        chbSave->setChecked(this->userPreferences->getSaveScenePosFlag());

        if (this->userPreferences->getLocale()->contains("EN"))
            prefLayout->addRow("Save view position\nin the project file",chbSave);
        else
            prefLayout->addRow("Do uloženého souboru zapiš i pozici pohledu",chbSave);

        QCheckBox * chbIcons = new QCheckBox(prefDialog);
        chbIcons->setChecked(this->userPreferences->getSmallIconsFlag());

        if (this->userPreferences->getLocale()->contains("EN"))
            prefLayout->addRow("Display small icons in menu",chbIcons);
        else
            prefLayout->addRow("Zobraz menší ikony v menu",chbIcons);

        prefLayout->addRow(new QLabel("UPOZORNĚNÍ: Aby se projevily změny jazykového nastavení, je nutné restartovat aplikaci"));
        prefLayout->addRow(new QLabel("NOTE: You have to restart the application to apply language settings"));

        prefLayout->addRow(discard,saveChanges);

        prefDialog->setLayout(prefLayout);

        this->preferencesDialog=prefDialog;

        connect(preferencesAction,SIGNAL(triggered()),this->preferencesDialog,SLOT(exec()));
        connect(saveChanges,SIGNAL(clicked()),this,SLOT(savePreferences()));




    }



#ifdef Q_OS_LINUX
    this->window->setGeometry(0,0,this->userPreferences->getLastSize()->width(),this->userPreferences->getLastSize()->height());
#endif
#ifdef Q_OS_WIN
    this->window->setGeometry(0,30,this->userPreferences->getLastSize()->width(),this->userPreferences->getLastSize()->height());
#endif


    this->aboutDialog = new QDialog (this->window);

    QLabel * lbl = new QLabel("Author: Milan Skipala.");
    QLabel * lbl2 = new QLabel("License: GNU GPL");
    QPushButton * b = new QPushButton("Close");
    QFormLayout * aboutL = new QFormLayout(this->aboutDialog);
    if (this->userPreferences->getLocale()->startsWith("CS"))
    {
        lbl->setText("Autor: Milan Skipala.");
        lbl2->setText("Licence: GNU GPL");
        b->setText("Zavřít");
    }
    aboutL->addRow(lbl);
    aboutL->addRow(lbl2);
    aboutL->addRow(b);
    this->aboutDialog->setLayout(aboutL);

    connect(aboutAction,SIGNAL(triggered()),this->aboutDialog,SLOT(exec()));
    connect(b,SIGNAL(clicked()),this->aboutDialog,SLOT(close()));

    return 0;
}

Window::Window() : QMainWindow()
{
    logFile << "window constructor" << endl;
    this->mainContextMenu=NULL;
    this->mainMenuBar=NULL;
    this->mainStatusBar=NULL;
    this->mainToolBar=NULL;
    this->workspace=NULL;
    this->sideBar=NULL;
    this->workspace=NULL;

}


QMenu * Window::getMainContextMenu()
{
    return this->mainContextMenu;
}
QMenuBar * Window::getMainMenuBar()
{
    return this->mainMenuBar;
}
QStatusBar * Window::getMainStatusBar()
{
    return this->mainStatusBar;
}
QToolBar * Window::getMainToolBar()
{
    return this->mainToolBar;
}
WorkspaceWidget * Window::getWorkspaceWidget()
{
    return this->workspace;
}
SideBarWidget * Window::getSideBarWidget()
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
    this->getMainContextMenu()->popup(evt->globalPos());
}
void Window::keyPressEvent(QKeyEvent * evt)
{
    app->sendEvent(this->workspace,evt);
}

void Window::keyReleaseEvent(QKeyEvent *evt)
{
    app->sendEvent(this->workspace,evt);
}

void Window::closeEvent(QCloseEvent * event)
{
    if (this->getWorkspaceWidget()->unsavedChangesFlag())
    {
        QMessageBox * unsaved = new QMessageBox(this);
        if (app->getUserPreferences()->getLocale()->contains("EN"))
            unsaved->setText(QString("There are some unsaved changes.\n Would you like to save the file?"));
        else
            unsaved->setText(QString("Soubor obsahuje neuložené změny.\n Přejete si soubor uložit?"));
        QPushButton * saveB = unsaved->addButton(QMessageBox::Yes);
        QPushButton * dontSaveB = unsaved->addButton(QMessageBox::No);
        QPushButton * cancelB = unsaved->addButton(QMessageBox::Cancel);

        unsaved->exec();
        QPushButton * clicked = (QPushButton*)unsaved->clickedButton();
        if (clicked==saveB)
            app->save();
        else if (clicked==dontSaveB)
        {}
        else if (clicked==cancelB)
        {
            event->ignore();
            return;
        }
    }
    event->accept();
    return;
}

void Window::resizeEvent(QResizeEvent * evt)
{

    if (app!=NULL)
        app->getUserPreferences()->setLastSize(&evt->size());
}

AppData::AppData(QString &lang)
{
    logFile << "AppData constructor" << endl;
    this->database =new Database(lang);

#ifdef Q_OS_LINUX
    QString path(folderPathLinux);
    path.append("Icons/");
#endif


#ifdef Q_OS_WIN
    QString path(folderPathWin);
    path.append("Icons\\");
#endif

    path.prepend(qApp->applicationDirPath());


    this->newFilePixmap = new QPixmap(QString("%1NewFile.png").arg(path));
    this->openFilePixmap = new QPixmap(QString("%1OpenFile.png").arg(path));
    this->saveFilePixmap = new QPixmap(QString("%1SaveFile.png").arg(path));

    this->undoPixmap = new QPixmap(QString("%1Undo.png").arg(path));
    this->redoPixmap = new QPixmap(QString("%1Redo.png").arg(path));

    this->rotateToolPixmap = new QPixmap(QString("%1RotateTool.png").arg(path));
    this->heightProfilePixmap = new QPixmap(QString("%1HeightProfile.png").arg(path));
    this->heightProfileUpPixmap = new QPixmap(QString("%1HeightProfileUp.png").arg(path));
    this->heightProfileDownPixmap = new QPixmap(QString("%1HeightProfileDown.png").arg(path));

    this->bendPixmap = new QPixmap(QString("%1Bend.png").arg(path));
    this->completePixmap =  new QPixmap(QString("%1Complete.png").arg(path));

    this->repeatPixmap =  new QPixmap(QString("%1Repeat.png").arg(path));

    this->copyPixmap =  new QPixmap(QString("%1Copy.png").arg(path));
    this->pastePixmap =  new QPixmap(QString("%1Paste.png").arg(path));
    this->cutPixmap =  new QPixmap(QString("%1Cut.png").arg(path));
    this->deletePixmap =  new QPixmap(QString("%1Delete.png").arg(path));

    this->preferencesPixmap =  new QPixmap(QString("%1Preferences.png").arg(path));
    this->modelOverviewPixmap =  new QPixmap(QString("%1ModelInfo.png").arg(path));

    this->helpPixmap =  new QPixmap(QString("%1Help.png").arg(path));
    this->aboutPixmap =  new QPixmap(QString("%1About.png").arg(path));

    logFile << "icons were loaded" << endl;


    this->messageDialog = new QMessageBox();
    this->messageDialog->setText("");
    this->messageDialog->setIcon(QMessageBox::Warning);
    this->messageDialog->setButtonText(0,"OK");

    this->bendAndCloseMessage = new QErrorMessage();
}

AppData::~AppData()
{
    logFile << "~AppData " << this << endl;
    delete this->database;
}

QPixmap * AppData::getNewFilePixmap() const
{return this->newFilePixmap;}
QPixmap * AppData::getOpenFilePixmap() const
{return this->openFilePixmap;}
QPixmap * AppData::getSaveFilePixmap() const
{return this->saveFilePixmap;}
QPixmap * AppData::getUndoPixmap() const
{return this->undoPixmap;}
QPixmap * AppData::getRedoPixmap() const
{return this->redoPixmap;}
QPixmap *AppData::getRotateToolPixmap() const
{return this->rotateToolPixmap;}
QPixmap *AppData::getHeightProfilePixmap() const
{return this->heightProfilePixmap;}
QPixmap *AppData::getHeightProfileUpPixmap() const
{return this->heightProfileUpPixmap;}
QPixmap *AppData::getHeightProfileDownPixmap() const
{return this->heightProfileDownPixmap;}
QPixmap *AppData::getBendPixmap() const
{return this->bendPixmap;}
QPixmap *AppData::getCompletePixmap() const
{return this->completePixmap;}
QPixmap * AppData::getRepeatPixmap() const
{return this->repeatPixmap;}
QPixmap * AppData::getCopyPixmap() const
{return this->copyPixmap;}
QPixmap * AppData::getPastePixmap() const
{return this->pastePixmap;}
QPixmap * AppData::getCutPixmap() const
{return this->cutPixmap;}
QPixmap * AppData::getDeletePixmap() const
{return this->deletePixmap;}
QPixmap * AppData::getPreferencesPixmap() const
{return this->preferencesPixmap;}
QPixmap * AppData::getModelOverviewPixmap() const
{return this->modelOverviewPixmap;}
QPixmap * AppData::getHelpPixmap() const
{return this->helpPixmap;}
QPixmap * AppData::getAboutPixmap() const
{return this->aboutPixmap;}
Database * AppData::getDatabase()
{return this->database;}

void AppData::setMessageDialogText(QString textEn, QString textCs)
{
    if (*app->getUserPreferences()->getLocale()==QString("EN"))
        this->messageDialog->setText(textEn);
    else
        this->messageDialog->setText(textCs);
}
QMessageBox *AppData::getMessageDialog()
{return this->messageDialog;}
QErrorMessage *AppData::getErrorMessage()
{return this->bendAndCloseMessage;}

Preferences::Preferences(QString *path)
{
    logFile << "Preferences constructor" << endl;
    this->actions=new QAction*[5];

    QFile file(*path);
    file.open(QFile::ReadOnly);
    this->preferencesPath=new QString(*path);
    QString example("Lang=EN\nLastWindowSize=500x400\nDisplayHelpBendClose=1\nSmallIcons=1\nSaveScenePos=1\n");
    example.append("LastFile1=\nLastFile2=\nLastFile3=\nLastFile4=\nLastFile5=\n");



    /*example preferences file
     *
    Lang=EN\n
    LastWindowSize=500x400\n
    DisplayHelpBendClose=1\n
    SmallIcons=1\n
    SaveScenePos=1\n
    LastFile1=\n
    LastFile2=\n
    LastFile3=\n
    LastFile4=\n
    LastFile5=\n
    */

    QTextStream fileStream(&file);
    if (!file.isOpen())
    {
        logFile << "userPreferences couldn't be opened" << endl;

        this->locale = new QString("EN");
        this->lastWindowSize = new QSize(500,300);
        this->displayHelpForBendAndClose=true;
        this->smallIcons=true;
        this->saveScenePos=true;
        this->lastProjects=new QString*[5];
        this->lastProjects[0]=new QString();
        this->lastProjects[1]=new QString();
        this->lastProjects[2]=new QString();
        this->lastProjects[3]=new QString();
        this->lastProjects[4]=new QString();


    }

    QString str;
    int lastFilePos = 0;
    while (!fileStream.atEnd())
    {
        str = fileStream.readLine();
        if (str.startsWith("Lang="))
        {
            str.remove(0,5);
            this->locale = new QString(str);
        }
        else if (str.startsWith("LastWindowSize="))
        {
            str.remove(0,15);
            int w = str.left(str.indexOf("x")).toInt();
            str = str.remove(0,str.indexOf("x")+1);
            int h = str.toInt();

            this->lastWindowSize = new QSize(w,h);
        }
        else if (str.startsWith("DisplayHelpBendClose="))
        {
            str.remove(0,21);
            if (str.startsWith("1"))
                this->displayHelpForBendAndClose=true;
            else
                this->displayHelpForBendAndClose=false;
        }
        else if (str.startsWith("SmallIcons="))
        {
            str.remove(0,11);
            if (str.startsWith("1"))
                this->smallIcons=true;
            else
                this->smallIcons=false;
        }
        else if (str.startsWith("SaveScenePos="))
        {
            str.remove(0,13);
            if (str.startsWith("1"))
                this->saveScenePos=true;
            else
                this->saveScenePos=false;
        }
        else if (str.startsWith("LastFile"))
        {
            str.remove(0,10);
            if (lastFilePos==0)
                this->lastProjects=new QString*[5];
            this->lastProjects[lastFilePos]=new QString(str);
            lastFilePos++;
        }


    }

    file.close();

}

Preferences::~Preferences()
{

    QFile file(*this->preferencesPath);
    file.open(QFile::WriteOnly);

    QTextStream fileStream(&file);

    fileStream << "Lang=" << *this->locale << endl;
    fileStream << "LastWindowSize="<< this->lastWindowSize->width() << "x" << this->lastWindowSize->height() << endl;
    fileStream << "DisplayHelpBendClose="<< (this->displayHelpForBendAndClose ? "1" : "0") << endl;
    fileStream << "SmallIcons="<< (this->smallIcons ? "1" : "0") << endl;
    fileStream << "SaveScenePos="<< (this->saveScenePos ? "1" : "0") << endl;
    fileStream << "LastFile1="<< *this->lastProjects[0] << endl;
    fileStream << "LastFile2="<< *this->lastProjects[1] << endl;
    fileStream << "LastFile3="<< *this->lastProjects[2] << endl;
    fileStream << "LastFile4="<< *this->lastProjects[3] << endl;
    fileStream << "LastFile5="<< *this->lastProjects[4] << endl;
    file.close();

    //delete this->lastProjects[0];
    delete this->lastProjects[4];
    delete this->lastProjects[3];
    delete this->lastProjects[2];
    delete this->lastProjects[1];
    delete this->lastProjects[0];
    delete this->locale;
    delete this->lastWindowSize;
    delete this->preferencesPath;
    delete[] this->lastProjects;
    delete[] this->actions;

}

QString *Preferences::getLocale()
{return this->locale;}
void Preferences::setLocale(QString *lang)
{*this->locale=*lang;}
QSize *Preferences::getLastSize()
{return this->lastWindowSize;}
void Preferences::setLastSize(const QSize *winSize)
{*this->lastWindowSize=*winSize;}
bool Preferences::getSmallIconsFlag()
{return this->smallIcons;}
void Preferences::setSmallIconsFlag(bool flag)
{this->smallIcons=flag;}
bool Preferences::getSaveScenePosFlag()
{return this->saveScenePos;}
void Preferences::setSaveScenePosFlag(bool flag)
{this->saveScenePos=flag;}
bool Preferences::getDisplayHelpBendAndClose() const
{return this->displayHelpForBendAndClose;}
void Preferences::setDisplayHelpBendAndClose(bool val)
{this->displayHelpForBendAndClose=val;}


QString **Preferences::getLastProjects()
{return this->lastProjects;}

void Preferences::addProject(QString *projName)
{
    int i = 0;
    for (; i < 5; i++)
    {
        if (*projName==*this->lastProjects[i])
            break;
    }

    if (i==5)
    {
        QString * newFst = this->lastProjects[4];
        this->lastProjects[4] = this->lastProjects[3];
        this->lastProjects[3] = this->lastProjects[2];
        this->lastProjects[2] = this->lastProjects[1];
        this->lastProjects[1] = this->lastProjects[0];
        this->lastProjects[0] = newFst;
        *this->lastProjects[0] = *projName;

    }
    else
    {
        QString * newFst =  this->lastProjects[i];
        for (int j = i; j >= 1; j--)
        {
            this->lastProjects[j]=this->lastProjects[j-1];
        }
        this->lastProjects[0]=newFst;
    }

    this->actions[0]->setText(*this->lastProjects[0]);
    this->actions[1]->setText(*this->lastProjects[1]);
    this->actions[2]->setText(*this->lastProjects[2]);
    this->actions[3]->setText(*this->lastProjects[3]);
    this->actions[4]->setText(*this->lastProjects[4]);

}

void Preferences::setActions(QAction *a1, QAction *a2, QAction *a3, QAction *a4, QAction *a5)
{
    this->actions[0]=a1;
    this->actions[1]=a2;
    this->actions[2]=a3;
    this->actions[3]=a4;
    this->actions[4]=a5;

}
