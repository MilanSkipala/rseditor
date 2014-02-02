#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H
#include <fstream>
#include <QString>
#include "application.h"


#define TOLERANCE_HALF 2.5 //5 points (=> milimeters)
#define TOLERANCE_ANGLE 5.0 //5 deg.
#define SEARCH_TOLERANCE_HALF 2.5

/**
  CHANGE!!
*/
extern QString folderPathLinux;/// = "/media/sf_Shared_Virtual/RTEditor/";//"/etc/";
extern QString folderPathWin;/// = "C:\\";

extern Application * app;

/*DELETE IN RELEASE MODE*/
extern int adjustCallCount;
extern int updateCallCount;

extern fstream logFile;

#endif // GLOBALVARIABLES_H
