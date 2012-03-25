#include "Leg.h"

/*======================== ALLOCATION ======================================*/
/** Allocate memory for a new leg.
 * @return A pointer to a new leg.
 */
leg_t* Leg_alloc(){
	leg_t* tempLeg = (leg_t*) calloc(1, sizeof(leg_t));
	for(int i = 0; i < LEG_DOF; i++){
		tempLeg->servos[i] = Servo_alloc();
		tempLeg->servoLocations[i] = rot_vector_alloc();
	}
	
	//vector for endpoint
	tempLeg->servoLocations[LEG_DOF] = rot_vector_alloc();
	
	tempLeg->offsetFromCOB = rot_vector_alloc();
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
	for(int i=0; i<LEG_DOF; i++){
		rot_free(leg->servoLocations[i]);
		Servo_free(leg->servos[i]);
	}
	rot_free(leg->servoLocations[LEG_DOF]);
	free(leg);
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
