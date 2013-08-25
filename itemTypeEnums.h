#ifndef ITEMTYPEENUMS_H
#define ITEMTYPEENUMS_H


enum ItemType {
                    C1, //curve
                    E1, //track end
                    J1, //Y junction
                    J2, //junction with 3 ends
                    J3, //X junction
                    S1, //straight
                    X1, //crossing
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
                    //special parts for slot car tracks
                    C2, //curve with crossing
                    CH //straight part with chicane
                   };
#endif // ITEMTYPEENUMS_H
