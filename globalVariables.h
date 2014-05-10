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

//this value is only when no info about item's scale is available (i.e. searching for items in graphicsScene etc.)
#define TOLERANCE_HALF 5 //10 points (=> milimeters)

extern QString folderPathLinux;
extern QString folderPathWin;

extern Application * app;

extern fstream logFile;

#endif // GLOBALVARIABLES_H
