/**
 * @file Leg.c
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

#include "Leg.h"

/*======================== ALLOCATION ======================================*/
/** Allocate memory for a new leg.
 * @return A pointer to a new leg.
 */
leg_t* Leg_alloc(){
	leg_t* tempLeg = (leg_t*) calloc(1, sizeof(leg_t));
    //allocate servos and their positions
    char i;
	for(i = 0; i < LEG_DOF; i++){
		tempLeg->servos[i] = Servo_alloc();
		tempLeg->servoLocations[i] = rot_vector_alloc();
	}
	
	//extra vectors
	tempLeg->servoLocations[LEG_DOF] = rot_vector_alloc();
	tempLeg->offsetFromCOB = rot_vector_alloc();
    //the solver
    //TODO relocate to usbdevice (we only need one)
	tempLeg->legSolver = Solver_alloc();
	return(tempLeg);
}


/*==================== FREE ================================================*/
/** Free leg memory.
 * @param leg The leg data to clear up.
 */
void Leg_free(leg_t* leg){
	Solver_free(leg->legSolver);
	rot_free(leg->offsetFromCOB);
    char i;
	for(i=0; i<LEG_DOF; i++){
		rot_free(leg->servoLocations[i]);
		Servo_free(leg->servos[i]);
	}
	rot_free(leg->servoLocations[LEG_DOF]);
	free(leg);
}


/*================ GET SERVO PW ============================================*/
/** Get the stored pulsewidth of a servo.
 * @param leg The leg data to use.
 * @param servoNo The servo to request (0..2).
 * @return The stored pulsewidth
 */
uint8_t Leg_getServoPw(leg_t* leg, uint8_t servoNo){
    return(leg->servos[servoNo]->_pw);
}


/*================ SET SERVO PW =============================================*/
/** Set the pulsewidth of a servo.
 * @param leg The leg data to use.
 * @param servoNo The servo to change (0..2).
 * @param pw The pulsewidth to set.
 * @retval 1 Pulsewidth was out of bounds, nothing changed.
 * @retval 0 success
 */
int Leg_setServoPw(leg_t* leg, uint8_t servoNo, uint8_t pw){
    return(Servo_setPw(leg->servos[servoNo], pw));
}



/*=============================== CALC COORDS ==============================*/
/** Calculate coordinates of all servos relative to servo 0.
 * The calculated locations are stored in leg->servoLocations.
 * @param solver The solver to use.
 */
void Leg_updateServoLocations(leg_t* leg){
	//location for servo 0 is the 0 vector by defenition
	rot_vector_setAll(leg->servoLocations[0], 0.0, 0.0, 0.0);

    //create some vectors and a rotation matrix
    rot_vector_t* angles = rot_vector_alloc();
    rot_vector_t* x = rot_vector_alloc();
    rot_matrix_t* M = rot_matrix_alloc();
	//general idea: move, rotate system, move again

	//servo 1
	
	//servo 2
	
}
