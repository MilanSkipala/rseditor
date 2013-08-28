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

/**
  turnout:
  A turnout's number expresses how much it will cause a train to diverge when it is thrown.
  The number is calculated by taking the number of units of forward travel for one unit of divergence.
  For example, if after traveling six inches from the point of divergence the train has diverged one inch, then you have crossed a #6 turnout.
  A rule of thumb is, the smaller the number the tighter the radius of the turnout's curve. The image shows, a #6 right turnout and a #4 right turnout.
  Notice that the #4 turnout diverges much faster than the #6.
  ###
  turnout has: angle, radius, length?
*/
