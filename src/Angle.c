/** 
 * @file Angle.c
 * @date 2012-03
 * @brief angle functions
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
