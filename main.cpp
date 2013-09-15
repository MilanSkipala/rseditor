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
/*
 *DB - instead of "-8" and "(*(*this->productLines->find(*this->currentProductLine))).getScaleEnum()" use another variable
 *?generateModel? - text positioning should be dependent on ScaleEnum
 *
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
