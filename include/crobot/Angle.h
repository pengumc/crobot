/**
 * @file Angle.h
 * @brief angles are always in radians
 */
#ifndef __ANGLE__
#define __ANGLE__

#include <math.h> 

#ifndef M_PI
/** defenition of PI if it's not defined in math.h*/
#define M_PI		3.14159265358979323846
#endif
#include <stdint.h>
#include <stdlib.h>

/** variable type of an angle*/
typedef double angle_t;


angle_t Angle_normalize(angle_t value);
int Angle_rangeCheck(angle_t test, angle_t min, angle_t max);


#endif

