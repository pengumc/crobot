/**
 * @file Accelerometer.c
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

#include "Accelerometer.h"


/*====================================== ALLOC ==============================*/
/** Allocate Accelerometer memory.
 * @return A pointer to the new accelerometer data.
 */
accelerometer_t* Accelerometer_alloc(){
    accelerometer_t* tempAcc = calloc(1, sizeof(accelerometer_t));
    tempAcc->tilt_angles = rot_vector_alloc();
    char i;
    for(i=0;i<ACC_FILTER_COUNT;i++){
        tempAcc->filters[i] = Filter_alloc();
    }
    return(tempAcc);
}


/*================================== FREE ===================================*/
/** Free accelerometer data.
 * @param acc The data to free.
 */
void Accelerometer_free(accelerometer_t* acc){
    char i;
    for(i=0;i<ACC_FILTER_COUNT;i++){
        Filter_free(acc->filters[i]);
    }
    rot_free(acc->tilt_angles);
    free(acc);
}


/*========================================= UPDATE VALUES ===================*/
/** Update all values based on new adc data.
 * @param acc The accelerometer data to use.
 * @param x Raw Adc value for the rotation around the x-axis.
 * @param y Raw Adc value for the rotation around the y-axis.
 * @param z Raw Adc value for the rotation around the z-axis.
 */
void Accelerometer_updateValues(accelerometer_t* acc, 
    uint8_t x, uint8_t y, uint8_t z)
{
    acc->adc_values[0] = x;
    acc->adc_values[1] = y;
    acc->adc_values[2] = z;
    _Accelerometer_calcTilt(acc);
    _Accelerometer_updateFilters(acc);
}


/*======================= CALC TILT =========================================*/
/** Recalculate the tilt based on stored adc values.
 * @param acc The accelerometer data to use.
 */
void _Accelerometer_calcTilt(accelerometer_t* acc){
    //some calc
    const double Ax = acc->adc_values[0] - ACC_MID;
    const double Ay = acc->adc_values[1] - ACC_MID;
    const double Az = acc->adc_values[2] - ACC_MID;
    const double x = atan(Ay / sqrt(Ax * Ax + Az * Az));
    const double y = atan(Ax / sqrt(Ay * Ay + Az * Az));
    //const double z = atan(sqrt(Ax * Ax + Ay * Ay) / Az);

    //rot_vector_setAll(acc->tilt_angles, x, y, 0.0);
}

/*========================== UPDATE FILTERS =================================*/
/** Update the filters.
 * @param acc The accelerometer data to use.
 */
void _Accelerometer_updateFilters(accelerometer_t* acc){
    char i;
    for(i=0;i<ACC_FILTER_COUNT;i++){
        Filter_step(acc->filters[i], rot_vector_get(acc->tilt_angles, i)); 
    }
}

