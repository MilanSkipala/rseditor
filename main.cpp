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
    logFile.open("log.txt",ios_base::out);
    logFile << "application is starting" << endl;
    //QApplication app(argc,argv);
    Application application(argc,argv);
    app = &application;
    application.getWindow()->show();
    int result = application.exec();
    int a = adjustCallCount;
    int u = updateCallCount;
    return result;
}


/**
  23-01-2014
  why does the deletion works and connection is problematic (180 deg. differences in angles with no neighbours)
  -make "O" track in 1 direction, then delete some item to make such piece of track in which counter-direction parts
  will appear -> angles on the left side are correct after deletion
  -when such track is created by connection of two fragments (1 straight, 1 corner), angles are ok, add some item, then delete it
  -now angles are wrong. The reason why it happens is that there is 180 diff. between [1] of straight and [1] of curved in "O" track,
  which causes that it works (and additionally - it is not correct to have there such difference)
  -connectFragments fixes differences in angles, so after deletion and rebuilding the error occurs

  solution:
  -rebuild fragment in the way that all items are in correct direction
*/

/*
 *DB - instead of "-8" and "(*(*this->productLines->find(*this->currentProductLine))).getScaleEnum()" use another variable
 *?generateModel? - text positioning should be dependent on ScaleEnum
 *horrible performance when there are some items in scene and height profile is turned on
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




