/** 
 * @file Leg.h
 */
#ifndef __LEG__
#define __LEG__

#include "rotation.h"
#include "Servo.h"
#include "Solver.h"

/** Number of servos per leg*/
#define LEG_DOF 3

typedef struct{
	servo_t* servos[LEG_DOF]; /**< Servo data for the servo in this leg.*/
	rot_vector_t* servoLocations[LEG_DOF+1]; /**< Location relative to servo0.*/
	rot_vector_t* offsetFromCOB; /**< The vector from COB to servo 0*/
	solver_t* legSolver; /**< The solver that will be used by this leg */
} leg_t;

leg_t* Leg_alloc();
void Leg_free(leg_t* leg);
void Leg_updateServoLocations(leg_t* leg);

#endif

