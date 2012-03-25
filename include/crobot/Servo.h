/**
 * @file Servo.h
 * @brief servo data and functions.
 */
#ifndef __SERVO__
#define __SERVO__

#include <stdlib.h>
#include <stdio.h>
#include "Angle.h"

/** default maximum pulsewidth*/
#define SERVO_DEF_MAX_PULSE 96

/** default minimum pulsewidth*/
#define SERVO_DEF_MIN_PULSE 48

/** (min+max)/2 */
#define SERVO_DEF_MID_PULSE 72

/** default rotation direction for a servo*/
#define SERVO_DEF_DIR 1.0

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

servo_t* Servo_alloc();
void Servo_free(servo_t* servo);

#endif

