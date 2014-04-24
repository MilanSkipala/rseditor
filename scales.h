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

#ifndef SCALES_H
#define SCALES_H


//all scales are defined by track gauge in milimeters, 1 pixel of gauge equals to 1 milimeter
/*
//rail scales
#define SCALE_G 45
#define SCALE_L 38
#define SCALE_O 32
#define SCALE_S 22
#define SCALE_OO 16
#define SCALE_HO 16
#define SCALE_TT 12
#define SCALE_N 9
#define SCALE_Z 6
#define SCALE_ZZ 5
#define SCALE_T 3

//slot car scales - values equal to distance between two tracks
#define SCALE_24 10
#define SCALE_32 8
#define SCALE_43 6
#define SCALE_HO 5
*/

enum ScaleEnum {
            SCALE_G = 45,
            SCALE_L = 38,
            SCALE_O = 32,
            SCALE_S = 22,
            SCALE_OO = 16,
            SCALE_HO = 16,
            SCALE_TT = 12,
            SCALE_N = 9,
            SCALE_Z = 6,
            SCALE_ZZ = 5,
            SCALE_T = 3,

            SCALE_24 = 60,//100,
            SCALE_32 = 40,//80,
            SCALE_43 = 25,//60,
            SCALE_HOSLOT = 17,//50,

            SCALE_undef = 1

};


#endif // SCALES_H
