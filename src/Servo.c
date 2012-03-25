/**
 * @file Servo.c
 * @brief servo data and functions
 * @date 2012-03
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

#include "Servo.h"

/*===================== ALLOCATION ==========================================*/
/** Allocate memory for new servo data.
 * Values are initialized with \ref Servo_reset.
 * @return A pointer to the new servo data
 */
servo_t* Servo_alloc(){
    servo_t* tempServo = (servo_t*) calloc(1, sizeof(servo_t));
    Servo_reset(tempServo);
    return(tempServo);
}

/*======================== FREE A SERVO =====================================*/
/** Delete servo data memory reservation
 * @param servo The servo data to free
 */
void Servo_free(servo_t* servo){
    free((void*)servo);
}


/*============================= RESET =======================================*/
/** Reset all values.
 * The default values are defined in as SERVO_DEF_<VALUENAME>.<br>
 * The angle is set to 0 and the pulsewidth to the neutral pulse.
 * @param servo The servo to reset.
 */
void Servo_reset(servo_t* servo){
    servo->maxPulse = SERVO_DEF_MAX_PULSE;
    servo->minPulse = SERVO_DEF_MIN_PULSE;
    servo->midPulse = SERVO_DEF_MID_PULSE;
    servo->direction = SERVO_DEF_DIR;
    servo->offset = SERVO_DEF_OFFSET;
    servo->K = SERVO_DEF_K;
    servo->_angle = 0;
    servo->_pw = SERVO_DEF_MID_PULSE;
    
}


/*============================= CONVERT TO ANGLE ============================*/
/** Convert a pulsewidth to an angle.
 * 
 * @param servo The servo settings to use.
 * @param pulsewidth The value to convert.
 * @returns The angle this servo would take if you set it to pulsewidth.
 */
angle_t Servo_convertToAngle(servo_t* servo, uint8_t pulsewidth){
    return( (pulsewidth - servo->midPulse) 
        * servo->direction * servo->K + servo->offset);
}



/*============================= CONVERT TO PULSEWIDTH =======================*/
/** Convert an angle to a pulsewidth.
 * 
 * @param servo The servo settings to use.
 * @param value The value to convert.
* @returns The pulsewidth this servo would take if you set it's angle to value.
 */
uint8_t Servo_convertToPulsewidth(servo_t* servo, angle_t value){
    return( (uint8_t) ((Angle_normalize(value) - servo->offset)
        / servo->K / servo->direction + servo->midPulse));
}


/*=============================== SET ANGLE =================================*/
/** Set the angle of the servo.
 * The value is normalized and checked before being set.
 * @param servo Target servo.
 * @param value The angle to set
 * @retval 0 Angle was out of bounds for this servo. nothing was changed.
 * @retval 1 Success.
 */
int Servo_setAngle(servo_t* servo, angle_t value){
    if(Servo_checkAngle(servo, value)){
        servo->_angle = Angle_normalize(value);
        servo->_pw = Servo_convertToPulsewidth(servo, value);
        return(1);
    }else{
        return(0);
    }
}


/*=============================== SET PULSEWIDTH ============================*/
/** Set the pulsewidth of the servo.
 * The value is checked before being set.
 * @param servo Target servo.
 * @param value The pulsewidth to set.
 * @retval 0 Pulsewidth was out of bounds for this servo. nothing was changed.
 * @retval 1 Success.
 */
int Servo_setPw(servo_t* servo, uint8_t value){
    if (value >= servo->minPulse && value <= servo->maxPulse){
        servo->_pw = value;
        servo->_angle = Servo_convertToAngle(servo, value);
        return(1);
    }else{
        return(0);
    }
}


/*============================== CHECK ANGLE ================================*/
/** Test if an angle is within bounds for a specific servo
 * @param servo The servo settings to use.
 * @param value The value to check.
 * @retval 1 Angle is within bounds.
 * @retval 0 Angle is out of bounds.
 */
int Servo_checkAngle(servo_t* servo, angle_t value){
    if (Angle_rangeCheck(value,
        Servo_convertToAngle(servo, servo->minPulse),
        Servo_convertToAngle(servo, servo->maxPulse)))
    {
        return(1);
    }else return(0);
}


/*============================== PRINT DETAILS===============================*/
/** Print all values of the servo to stdout
 * @param servo The servo data to print.
 */
void Servo_printDetails(servo_t* servo, const char* name){
    printf("servo %s:\n \
        \tangle: %.2f\n \
        \tpulsewidth: %d\n \
        \tminPulse: %d\n \
        \tmidPulse: %d\n \
        \tmaxPulse: %d\n \
        \toffset: %.2f\n \
        \tK: %.3f\n \
        \tdirection: %.1f\n \
        \trange: (%.2f ... %.2f)\n",
        name, servo->_angle, servo->_pw, servo->minPulse, servo->midPulse,
        servo->maxPulse, servo->offset, servo->K, servo->direction, 
        Servo_convertToAngle(servo, servo->minPulse), 
        Servo_convertToAngle(servo, servo->maxPulse));
}
