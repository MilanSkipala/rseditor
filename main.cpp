/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

    This file is a part of Rail & Slot Editor.
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

#include "includeHeaders.h"
#include "application.h"
#include "partsRelated.h"
#include "database.h"

QString folderPathLinux  = "/RSEditor/";
QString folderPathWin  = "/RSEditor/"; //\\RSEditor\\ used to be here, but current value works fine
Application * app;

fstream logFile;

int main(int argc, char ** argv)
{
    logFile.open("log.txt",ios_base::out);
    logFile << "application is starting" << endl;

    Application application(argc,argv);
    app = &application;
    application.getWindow()->show();



    int result = application.exec();

    return result;
}
