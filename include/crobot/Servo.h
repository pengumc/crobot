/**
 * @file Servo.h
 * @brief servo data and functions.
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


#ifndef __SERVO__
#define __SERVO__

#include <stdlib.h>
#include <stdio.h>
#include "Angle.h"
#include "Report.h"

/** default maximum pulsewidth*/
#define SERVO_DEF_MAX_PULSE 96

/** default minimum pulsewidth*/
#define SERVO_DEF_MIN_PULSE 48

/** (min+max)/2 */
#define SERVO_DEF_MID_PULSE 72

/** default rotation direction for a servo*/
#define SERVO_DEF_DIR 1.0f

/** default servo offset*/
#define SERVO_DEF_OFFSET 0

/** default conversion factor*/
#define SERVO_DEF_K 0.034

/** Struct to hold the servo data.*/
typedef struct{
    uint8_t _pw; /**< Current pulsewidth of servo.*/
    uint8_t midPulse; /**< Neutral pulse for this servo.*/
    uint8_t minPulse; /**< Minimum pulse for this servo.*/
    uint8_t maxPulse; /**< Maximum pulse for this servo.*/
    angle_t offset; /**< physical rotation of the servo.*/
    double K; /**< pulsewidth to angle conversion factor.*/
    double direction; /**< Direction of servo rotation.*/
    angle_t _angle; /**< Current angle of servo.*/
} servo_t;


void Servo_reset(servo_t* servo);
int Servo_setAngle(servo_t* servo, angle_t value);
int Servo_setPw(servo_t* servo, uint8_t value);
angle_t Servo_convertToAngle(servo_t* servo, uint8_t pulsewidth);
uint8_t Servo_convertToPulsewidth(servo_t* servo, angle_t value);
int Servo_checkAngle(servo_t* servo, angle_t value);
void Servo_printDetails(servo_t* servo, const char* name);
void Servo_setOffset(servo_t* servo, angle_t offset);
void Servo_setDirection(servo_t* servo, int8_t direction);
int Servo_changeAngle(servo_t* servo, angle_t value);

servo_t* Servo_alloc();
void Servo_free(servo_t* servo);

#endif

