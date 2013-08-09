#include "includeHeaders.h"
#include "application.h"
#include "partsRelated.h"
#include "database.h"

QString folderPathLinux  = "/media/sf_Shared_Virtual/RTEditor/";
QString folderPathWin  = "C:\\";
Application * app;


int main(int argc, char ** argv)
{
    //QApplication app(argc,argv);
    Application application(argc,argv);
    app = &application;
    application.getWindow()->show();
    return application.exec();
}
