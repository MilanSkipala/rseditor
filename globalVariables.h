/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

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

#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H
#include <fstream>
#include <QString>
#include "application.h"


#define TOLERANCE_HALF 5 //10 points (=> milimeters)
#define TOLERANCE_ANGLE 5.0 //5 deg.
#define SEARCH_TOLERANCE_HALF 2.5

/**
  CHANGE!!
*/
extern QString folderPathLinux;/// = "/media/sf_Shared_Virtual/RSEditor/";//"/etc/";
extern QString folderPathWin;/// = "C:\\";

extern Application * app;

/*DELETE IN RELEASE MODE*/
extern int adjustCallCount;
extern int updateCallCount;

extern fstream logFile;

#endif // GLOBALVARIABLES_H
