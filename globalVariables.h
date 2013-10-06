#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <QString>
#include "application.h"
/**
  CHANGE!!
*/
extern QString folderPathLinux;/// = "/media/sf_Shared_Virtual/RTEditor/";//"/etc/";
extern QString folderPathWin;/// = "C:\\";

extern Application * app;

/*DELETE IN RELEASE MODE*/
extern int adjustCallCount;
extern int updateCallCount;



#endif // GLOBALVARIABLES_H
