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

#ifndef ITEMTYPEENUMS_H
#define ITEMTYPEENUMS_H


enum ItemType {
                    C1, //curve
                    E1, //track end
                    J1, //Y right turnout
                    J2, //Y left turnout
                    J3, //junction with 3 ends
                    J4, //X junction
                    J5, // |X| junction
                    S1, //straight
                    X1, //crossing (slot track: looks like two crossed pieces)

                    T1, //turntable
                    T2, //transfer table - 2 lanes
                    T3,
                    T4,
                    T5,
                    T6,
                    T7,
                    T8,
                    T9,
                    T10, //transfer table - 10 lanes

                    //special parts for slot tracks
                    C2, //curved crossing
                    CB, //banked curve
                    HC, //straight part with chicane
                    //H1, //hairpin curve
                    HS, //straight bottleneck section
                    HE, //straight with end of hairpin
                    SC, //straight with lap counter
                    JM, //jump
                    X2, //crossing on one straight piece
                    //X3 //"bridged" crossing - exists in db and sbw, but when it is being inserted into workspacewidget, program creates two S1 items with different height profiles

                    UNKNOWN



                   };
#endif // ITEMTYPEENUMS_H
