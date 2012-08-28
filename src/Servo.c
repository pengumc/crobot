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
/**
 * @addtogroup Servo
 * @{
 * @file Servo.c
 * @date 2012-03
 */

#include "Servo.h"

/*===================== ALLOCATION ==========================================*/
/** Allocate memory for new servo data.
 * Values are automatically initialized with \ref Servo_reset.
 * @return A pointer to the new servo data.
 * 
 * Example: see \ref servo_example
 * 
 */
servo_t* Servo_alloc(){
    servo_t* tempServo = (servo_t*) calloc(1, sizeof(servo_t));
    Servo_reset(tempServo);
    return(tempServo);
}

/*======================== FREE A SERVO =====================================*/
/** Delete servo data memory reservation
 * @param servo The servo data to free.
 * 
 * Example: see \ref servo_example
 */
void Servo_free(servo_t* servo){
    free((void*)servo);
}


/*============================= RESET =======================================*/
/** Reset all values of a servo.
 * The default values are defined in servo.h as 
 * \ref SERVO_DEF_MAX_PULSE "SERVO_DEF_<VALUENAME>".<br>
 * The angle is set to 0 and the pulsewidth to the neutral pulse.
 * @param servo The servo to reset.
 * 
 * Example: see \ref servo_example
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
 * Direction and offset are taken into account.
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
 * Direction and offset are taken into account.
 * @param servo The servo settings to use.
 * @param value The value to convert.
* @returns The pulsewidth this servo would take if you set it's angle to value.
 */
uint8_t Servo_convertToPulsewidth(servo_t* servo, angle_t value){
    value = Angle_normalize(value - servo->offset);
    uint8_t result = (value)
        / servo->K * servo->direction + servo->midPulse;
    //printf("angle2pw: %.2f -> %d\n", value, result);
    return(result);
}


/*=============================== SET ANGLE =================================*/
/** Set the angle of the servo.
 * The value is normalized and checked before being set.
 * @param servo Target servo.
 * @param value The angle to set
 * @retval 0 Angle was out of bounds for this servo. nothing was changed.
 * @retval 1 Success.
 * 
 * Example:
 * @code
 *  servo_t* myServo = Servo_alloc();
 *  int result = Servo_setAngle(myServo, 0.5 * M_PI);
 *  if(result){
     * //success. The angle is now Pi/2 and the pulsewidth is updated as well.
 *  } else {
     * //failure. The pulsewidth that is needed to achieve this angle is out of bounds.
 *  }
 * @endcode
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


/*============================ CHANGE ANGLE =================================*/
/** Change servo angle by value.
 * @param servo Target servo.
 * @param value The amount to change the angle by.
 * @retval 1 Success.
 * @retval 0 Angle was out of bounds for this servo. nothing was changed.
 * 
 * Example: see \ref servo_example
 */
int Servo_changeAngle(servo_t* servo, angle_t value){
    const double a = value + servo->_angle;
    return(Servo_setAngle(servo, a));
}

/*=============================== SET PULSEWIDTH ============================*/
/** Set the pulsewidth of the servo.
 * The value is checked before being set.
 * @param servo Target servo.
 * @param value The pulsewidth to set.
 * @retval 0 Pulsewidth was out of bounds for this servo. nothing was changed.
 * @retval 1 Success.
 * 
 * Example:
 * @code
 *  servo_t* myServo = Servo_alloc();
 *  int result = Servo_setPw(myServo, 55);
 *  if(result){
     * //success. The pulsewidth is now 55 and the angle was updated as well.
 *  } else {
     * //failure. The pulsewidth is out of bounds.
 *  }
 * @endcode
 */
int Servo_setPw(servo_t* servo, uint8_t value){
    int valid = 0;
    if(servo->direction > 0.0){
        if (value >= servo->minPulse && value <= servo->maxPulse) valid = 1;
    }else{
        if (value >= servo->maxPulse && value <= servo->minPulse) valid = 1;
    }   
    if (valid){
        servo->_pw = value;
        servo->_angle = Servo_convertToAngle(servo, value);
        return(1);
    }else{
        return(0);
    }
}


/*============================== CHECK ANGLE ================================*/
/** Test if an angle is reachable by a servo.
 * @param servo The servo settings to use.
 * @param value The value to check.
 * @retval 1 Angle is within bounds.
 * @retval 0 Angle is out of bounds.
 * 
 * Example:
 * @code
 *  servo_t* myServo = Servo_alloc();
 *  int result = Servo_checkAngle(myServo, 0.5 * M_PI); // Pi/2 =  1.57
 *  if(result){
     * // The servo can achieve this angle.
 *  } else {
     * // The pulsewidth needed to reach this angle is out of bounds.
 *  }
 * 
 *  // A servo usually has a range of -0.8 .. 0.8 so an angle of Pi/2 
 *  // would not be possible. When we set an offset of 1.0 however, the
 *  // check should succeed.
 *  Servo_setOffset(myServo, 1.0);
 *  result = Servo_checkAngle(myServo, 0.5 * M_PI);
 *  //success!
 * @endcode
 */
int Servo_checkAngle(servo_t* servo, angle_t value){
    int result;
    angle_t min = Servo_convertToAngle(servo, servo->minPulse);
    angle_t max = Servo_convertToAngle(servo, servo->maxPulse);
    if (Angle_rangeCheck(value, min, max))
    {
        result = 1;
    }else result =0;
    //printf("servo check %.2f: %d\nrange: %.2f ... %.2f\n", value, result, min, max);
    return(result);
}


/*============================== PRINT DETAILS===============================*/
/** Print all values of the servo to stdout
 * @param servo The servo data to print.
 * @param name The name to print with the data.
 * 
 * Example: 
 * @code
 *  servo_t* myServo = Servo_alloc();
 *  Servo_printDetails(myServo, "testServo");
 *  Servo_free(myServo);
 * @endcode
 * Excample output:
 * @code
 * servo (testServo):
 *      angle: 0.00
 *      pulsewidth: 72
 *      minPulse: 48
 *      midPulse: 72
 *      maxPulse: 96
 *      offset: 0.00
 *      K: 0.034
 *      direction: 1.0
 *      range: (-0.82 ... 0.82)
 * @endcode
*/
 
void Servo_printDetails(servo_t* servo, const char* name){
    char s[80*10];
    sprintf(s,"servo %s:\n \
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
    Report_std(s);
}


/*========================== SET OFFSET =====================================*/
/** set offset of servo (for a physical rotation of the servo body).
 * @brief Also reset to midPulse.
 * @param servo The servo settings to use.
 * @param offset The offset to set in radians.
 * 
 * Example: see \ref servo_example
 */
void Servo_setOffset(servo_t* servo, angle_t offset){
    servo->offset = Angle_normalize(offset);
    Servo_setPw(servo, SERVO_DEF_MID_PULSE);
}


/*================================ SET DIRECTION ============================*/
/** Set direction of servo rotation, useful when servo is mirrored.
 * The pulsewidth is reset to SERVO_DEF_MID_PULSE.
 * @param servo The servo settings to use.
 * @param direction The new direction, positive for forward, anything else
 means backwards.
 * 
 * Example: see \ref servo_example
 */
void Servo_setDirection(servo_t* servo, int8_t direction){
    double newdir;
    if(direction > 0) newdir = 1.0;
    else newdir = -1.0;
    if (servo->direction != newdir){
        //flip min and max pulse as well as direction
        uint8_t temp = servo->minPulse;
        servo->minPulse = servo->maxPulse;
        servo->maxPulse = temp;
        servo->direction = newdir;
    }
    Servo_setPw(servo, SERVO_DEF_MID_PULSE);
}
/**@}*/
