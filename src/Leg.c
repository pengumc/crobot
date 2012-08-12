/**
 * @file Leg.c
 * @brief leg data is unaware of the main body. The origin point is servo 0,
 servoLocations uses this as origin.
 The plane of rotation is the same as the mainbody plate.
 
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
    tempLeg->endpointRollback = rot_vector_alloc();
    //the solver
    
	//TODO relocate solver to usbdevice (we only need one)
	tempLeg->legSolver = Solver_alloc();
	
	
	return(tempLeg);
}


void Leg_initDefaults(leg_t* leg){
    Servo_setOffset(leg->servos[2], -M_PI/2);
    leg->legSolver->params->A = 10.0;
    leg->legSolver->params->B = 10.0;
    leg->legSolver->params->C = 10.0;
    leg->legSolver->params->X = 20.0;
    leg->legSolver->params->Y = 00.0;
    leg->legSolver->params->Z = -10.0;
    Leg_updateServoLocations(leg);

}


/*==================== FREE ================================================*/
/** Free leg memory.
 * @param leg The leg data to clear up.
 */
void Leg_free(leg_t* leg){
	Solver_free(leg->legSolver);
	rot_free(leg->offsetFromCOB);
    rot_free(leg->endpointRollback);
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
 * @retval 0 Pulsewidth was out of bounds, nothing changed.
 * @retval 1 success
 */
int Leg_setServoPw(leg_t* leg, uint8_t servoNo, uint8_t pw){
    return(Servo_setPw(leg->servos[servoNo], pw));
}



/*================ GET SERVO ANGLE ==========================================*/
/** get stored servo angle.
 * @param leg The leg data to use.
 * @param servoNo The servo to request (0..2).
 * @return The servo angle.
 */
double Leg_getServoAngle(leg_t* leg, uint8_t servoNo){
    return(leg->servos[servoNo]->_angle);
}

/*=============================== CALC COORDS ===============================*/
/** Calculate coordinates of all servos relative to servo 0.
 * The calculated locations are stored in leg->servoLocations.
 * @param solver The solver to use.
 */
void Leg_updateServoLocations(leg_t* leg){
    /*create a vector x that represents the offset to the next servo
     then rotate that vector with the servo angles
     then add the location of the previous servo*/
	
	/*NOTE because of the way the rotation matrix is defined
	the z axis actually rotates when you rotate the xy plane
	so it's better to rotate 1 angle at a time */

	//location for servo 0 is the 0 vector by definition
	rot_vector_setAll(leg->servoLocations[0], 0.0, 0.0, 0.0);

    //allocate some vectors and a rotation matrix
    rot_vector_t* angles = rot_vector_alloc();
    rot_vector_t* x = rot_vector_alloc();
    rot_matrix_t* M = rot_matrix_alloc();
	
	const double alpha = Leg_getServoAngle(leg, 0);
	const double beta = Leg_getServoAngle(leg, 1);
	const double gamma = Leg_getServoAngle(leg, 2);

    //grab the lengths
    solverParams_t params = Solver_getParams(leg->legSolver);
    
	//servo 1
    rot_vector_setAll(x, params.A, 0.0, 0.0);
	rot_vector_setAll(angles, 0.0, 0.0, alpha);
    rot_matrix_build_from_angles(M, angles);
    rot_matrix_dot_vector(M, x, leg->servoLocations[1]); //rotate
    rot_vector_add(leg->servoLocations[1], leg->servoLocations[0]); //add prev
	
	//servo 2
    rot_vector_setAll(x, params.B, 0.0, 0.0);
		 //y first
    rot_vector_setAll(angles, 0.0, beta, 0.0);
    rot_matrix_build_from_angles(M, angles);
    rot_matrix_dot_vector(M, x, leg->servoLocations[2]);
		//next z
	rot_vector_setAll(angles, 0.0, 0.0, alpha);
    rot_matrix_build_from_angles(M, angles);
    rot_matrix_dot_vector(M, leg->servoLocations[2], leg->servoLocations[2]);
		//and add
    rot_vector_add(leg->servoLocations[2], leg->servoLocations[1]);
    
	//endpoint
    rot_vector_setAll(x, params.C, 0.0, 0.0);
		 //y first
    rot_vector_setAll(angles, 0.0, beta+gamma, 0.0);
    rot_matrix_build_from_angles(M, angles);
    rot_matrix_dot_vector(M, x, leg->servoLocations[3]);
		//next z
	rot_vector_setAll(angles, 0.0, 0.0, alpha);
    rot_matrix_build_from_angles(M, angles);
    rot_matrix_dot_vector(M, leg->servoLocations[3], leg->servoLocations[3]);
		//and add
    rot_vector_add(leg->servoLocations[3], leg->servoLocations[2]);

    
    //free vectors and matrix
    rot_free(x);
    rot_free(angles);
    rot_free(M);
	
}

void Leg_resyncSolverParams(leg_t* leg){
    Solver_setXYZ(leg->legSolver, 
        rot_vector_get(leg->servoLocations[3], 0),
        rot_vector_get(leg->servoLocations[3], 1),
        rot_vector_get(leg->servoLocations[3], 2));
}

void Leg_setupRollback(leg_t* leg){
    rot_vector_setAll(leg->endpointRollback,
        leg->legSolver->params->X,
        leg->legSolver->params->Y,
        leg->legSolver->params->Z);
}

void Leg_performRollback(leg_t* leg){
    Solver_setXYZ(leg->legSolver,
        rot_vector_get(leg->endpointRollback, 0),
        rot_vector_get(leg->endpointRollback, 1),
        rot_vector_get(leg->endpointRollback, 2));
    Leg_setupRollback(leg); //clears
}
/*============== TRY ENDPOINT change ========================================*/
/** Check if there's a valid solution for a change.
 * After a successful try you should call Leg_commitEndpointChange to
 finalize the changes. This is to simplify rollbacks when changing multiple
 legs.
 * @param leg The leg data to use.
 * @param delta The change in xyz coords of the endpoint.
 * retval 0 Success, Valid coordinates can be set with 
 Leg_commitEndpointChange.
 * @retval -4 Error, no valid solution, no changes were made. Please note that 
 the lastResult vector in the solver is not valid now.
 * @retval -1..3 A valid solution was found but one of the servo's couldn't
 handle the angle. It's number-1 (negative) is returned.
 */
int Leg_tryEndpointChange(leg_t* leg, rot_vector_t* delta){
    Leg_setupRollback(leg);
    //setup params
    Solver_changeXYZ(leg->legSolver, 
        rot_vector_get(delta, 0),
        rot_vector_get(delta, 1),
        rot_vector_get(delta, 2));
    //DEBUG
    printf("target coords: %.1f, %.1f, %.1f\n", 
        leg->legSolver->params->X,
        leg->legSolver->params->Y,
        leg->legSolver->params->Z);
    //check if the solver can find a solution
	int returnCode = 0;
    if(Solver_solve(leg->legSolver) == 0){
        //rot_vector_print(leg->legSolver->lastResult);
        //successful solve
		int8_t i;
		for(i=0;i<LEG_DOF;i++){
			//still, check the servos to see if they can handle the angle	
            //printf("checking servo %d for angle %.2f\n",
            //    i, rot_vector_get(leg->legSolver->lastResult, i));
			if(Servo_checkAngle(leg->servos[i], 
				rot_vector_get(leg->legSolver->lastResult, i)) == 0)
			{
				returnCode = -(i+1);
                break;
			}
		}
    }else returnCode = -(LEG_DOF+1); //no valid solution 
	
	if(returnCode != 0){
        //no solution, return params to previous state
        printf("failed, rolling back\n");
        /*Solver_changeXYZ(leg->legSolver,
            -rot_vector_get(delta, 0),
            -rot_vector_get(delta, 1),
            -rot_vector_get(delta, 2));
         */
        Leg_performRollback(leg);
    }
    return(returnCode);

}

/*================= COMMIT CHANGE ===========================================*/
/** Commit the previously calculated angles to the servos.
 * @param leg The leg data to use.
 */
void Leg_commitEndpointChange(leg_t* leg){
    if(leg->legSolver->validLastResult){
        char i;
        for(i=0;i<LEG_DOF;i++){
            Servo_setAngle(leg->servos[i], 
               rot_vector_get(leg->legSolver->lastResult, i));
        }
        Leg_updateServoLocations(leg);
    }
}


/*================== PRINT DETAILS ==========================================*/
/** Print leg details.
 * @param leg The leg info to print.
 */
void Leg_printDetails(leg_t* leg){
	uint8_t i;
	char s[80];
	Report_std("============\nLEG DETAILS\n============");
	for(i=0;i<LEG_DOF;i++){
		sprintf(s, "servo %d: %.4f  at %d",
            i, leg->servos[i]->_angle, leg->servos[i]->_pw);
		Report_std(s);
	}
    Report_std("leg offset:");
    rot_vector_print(leg->offsetFromCOB);
	Report_std("servo loc:");
	for(i=0;i<LEG_DOF+1;i++){
		rot_vector_print(leg->servoLocations[i]);
	}
    

    solverParams_t params = Solver_getParams(leg->legSolver);
	sprintf(s, "A: %.1f\nB: %.1f\nC: %.1f", 
        params.A, params.B, params.C);
	Report_std(s);
}


/*====================== CHANGE SERVO =======================================*/
/** Change servo angle by value.
 * @param leg The leg data to use.
 * @param s Target servo no.
 * @param value The amount to change the angle by.
 * @retval 1 Success.
 * @retval 0 Angle was out of bounds for this servo. nothing was changed.
 */
int Leg_changeServoAngle(leg_t* leg, uint8_t s, double value){
    int result = Servo_changeAngle(leg->servos[s], value);
    Leg_updateServoLocations(leg);
    return(result);
}



void Leg_setLengths(leg_t * leg, double A, double B, double C){
    leg->legSolver->params->A = A;
    leg->legSolver->params->B = B;
    leg->legSolver->params->C = C;
    Leg_updateServoLocations(leg);
    leg->legSolver->params->X = rot_vector_get(leg->servoLocations[LEG_DOF], 0);
    leg->legSolver->params->Y = rot_vector_get(leg->servoLocations[LEG_DOF], 1);
    leg->legSolver->params->Z = rot_vector_get(leg->servoLocations[LEG_DOF], 2);
}
