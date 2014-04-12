#include "includeHeaders.h"
#include "application.h"
#include "partsRelated.h"
#include "database.h"

//QString folderPathLinux  = "/media/sf_Shared_Virtual/RSEditor/";
QString folderPathLinux  = "./RSEditor/";
//QString folderPathWin  = "C:\\";
QString folderPathWin  = ".\\RSEditor\\";
Application * app;

fstream logFile;

int adjustCallCount = 0;
int updateCallCount = 0;


int main(int argc, char ** argv)
{
    //fstream commands;
    //commands.open("IOfile.txt");
    logFile.open("log.txt",ios_base::out);
    logFile << "application is starting" << endl;
    //QApplication app(argc,argv);
    Application application(argc,argv);
    app = &application;
    application.getWindow()->show();

    /*while(!commands.eof())
    {
        string str;
        getline(commands,str);
        app->getWindow()->getWorkspaceWidget()->actionListRedo.push_back(QString(str.c_str()));
    }*/



    int result = application.exec();
    int a = adjustCallCount;
    int u = updateCallCount;
/**
    how should inventory saving work:
        -first load basic info about scene etc.
        -load all items and place them in the workspace
        -add "Inventory" line in file
        -ProductLine=prodLineName line
        -partNo=count;
    if item is not found stop reading the file, write error in logFile, call newFile();


    when application is started and after newFile action:
        -load "Inventory.txt"
        -prodLineName line
        -partNo=count;
        if item is not found, just write it in logFile and keep reading

IMPLEMENTED WAY:
    -inventory state is saved/loaded when the project file is opened/saved
        -project files contain info only about product lines used in the model (=> used=it is contained in the comboBox before/during saving)
    -application also uses file Inventory.rsi placed in a folder with partSet.rsd (database)
        -it's content is used everytime when new project is created (=on startup and when newFileAction is used)
        -it contains info about all product lines, which were available in the database at the time of last "save inventory" action
        -"Save inventory" action causes overwriting of Inventory file and since the inventory saving is based on the database it means that
            any changes done in the database affect the inventory (i.e. user deletes ProductLine -> inventory data for deleted prod. line are lost)


*/

/**

    todo all borders:
        -graphics - R2 or R3 should contain circa 22 red or white "squares"*/

/*    for (int i = 0; i < app->getWindow()->getWorkspaceWidget()->actionListRedo.count(); i++)
        commands << app->getWindow()->getWorkspaceWidget()->actionListRedo.at(i).toStdString() << endl;
*/
    //commands.close();


    return result;
}

/*
 *?generateModel? - text positioning should be dependent on ScaleEnum
*/

/*
 * "how to make height profile":
 *# each item has |# of endPoints| pointers to qreal variables
 *# when user adjusts height of one point of one item, adjustHeight method
 *#  of neighbour is called too. Neighbour item is found by QPointF of which
 *#  height is being adjusted
 *
 *"how to make it working"
 *-click on toggleMode -> display blue circles
 *#click in the scene -> findItem
 *#click on heightIncrease -> adjustHeight of activeItem and activeEndPoint
 *
 *who owns the heightGraphicsItem?
 *-modelItem
*/
