/** 
 * @file Angle.c
 * @date 2012-03
 * @brief angle functions
 */
/* Copyright (c) 2012 Michiel van der Coelen

    This file is part of Crobot

    Crobot is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Crobot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Crobot.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Angle.h"


/*=============================== NORMALIZE =================================*/
/** Fix the range of [value] to -PI..PI.
 * @param value The input value to fix.
 * @return the value winded down to -PI..PI.
 */
angle_t Angle_normalize(angle_t value){
    while(value > M_PI) value -= M_PI * 2;
    while(value < -M_PI) value += M_PI * 2;
    return value;
}


/*================================ RANGE CHECK ==============================*/
/** Check if an angle is between two other angles.
*
 * Angles are in radians.<br>
 * All values are normalized with \ref Angle_normalize before being used.<br>
 * @param test The angle to test.
 * @param min The minimum angle.
 * @param max The maximum angle.
 * @retval 1 Angle lies between min and max.
 * @retval 0 Angle lies outside min and max.
 *
 * Examples:
 * @code
 *  Angle_rangeCheck(0.1, -1.0, 1.0); //will return 1.
 *  Angle_rangeCheck(3.1, 1.57, -1.57); //checks if 3.1 is on the left half of the unity circle. (it is)  
 * @endcode
 *
 * @see Angle_normalize
 */
int Angle_rangeCheck(angle_t test, angle_t min, angle_t max){
    min = Angle_normalize(min);
    max = Angle_normalize(max);
    test = Angle_normalize(test);

    if(min < max) {
        //easy mode
        return((test <= max && test >= min));
    }else{
        angle_t shift = max + M_PI;
        min -= shift;
        test = Angle_normalize(test - shift);
        return((test >= min && test <= M_PI));
    }
}
