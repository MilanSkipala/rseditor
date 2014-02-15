#include "includeHeaders.h"
#include "application.h"
#include "partsRelated.h"
#include "database.h"

//QString folderPathLinux  = "/media/sf_Shared_Virtual/RTEditor/";
QString folderPathLinux  = "./RTEditor/";
//QString folderPathWin  = "C:\\";
QString folderPathWin  = ".\\RTEditor\\";
Application * app;

fstream logFile;

int adjustCallCount = 0;
int updateCallCount = 0;

int main(int argc, char ** argv)
{
    fstream commands;
    commands.open("IOfile.txt");
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


/*    for (int i = 0; i < app->getWindow()->getWorkspaceWidget()->actionListRedo.count(); i++)
        commands << app->getWindow()->getWorkspaceWidget()->actionListRedo.at(i).toStdString() << endl;
*/
    commands.close();


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
