/**
 * @file Solver.c
 * @date 2012-03
 * @brief Numerical solver for a our specific multiroot problem.
 * All values use a coordinate system with it's origin at the pivot point
 * of servo 0.<br><br>
 * We're taking the easy way out with defining servo positions: assume the
 * endpoint is on the same plane as the rotation axis of servo 0. (same y coord).<br>
 * Any offsets should be handled manually with servo offsets.
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

#include "Solver.h"

/*======================== STEP FUNCTION ====================================*/
int _trig_f2 (const gsl_vector *in, void *params, gsl_vector *out){
    const double A = ((struct PARAMS*)params)->A;
    const double B = ((struct PARAMS*)params)->B;
    const double C = ((struct PARAMS*)params)->C;
    const double X = ((struct PARAMS*)params)->_X;
    const double Z = ((struct PARAMS*)params)->Z;

    const double b = gsl_vector_get(in, 0);
    const double c = gsl_vector_get(in, 1);

    const double x =  A + (B * cos(b)) + (C * cos(c+b)) - X; 
    const double z = (sin(b) *B) + (sin(c+b) * C) -Z;

    gsl_vector_set(out, 0, x);
    gsl_vector_set(out, 1, z);

    return GSL_SUCCESS;
}


/*============================ ALLOCATE =====================================*/
/** Allocate memory for a new solver
 * @return A pointer to a new solver
 */
solver_t* Solver_alloc(){
    solver_t* tempSolver = (solver_t*) calloc(1, sizeof(solver_t));
    tempSolver->lastResult = rot_vector_alloc();
    tempSolver->params = (solverParams_t*) calloc(1, sizeof(solverParams_t));
    tempSolver->_initVector = gsl_vector_alloc(2);
    const gsl_multiroot_fsolver_type* T = gsl_multiroot_fsolver_hybrid;
    tempSolver->_fsolver = gsl_multiroot_fsolver_alloc(T, 2);
    return(tempSolver);
}

/*=============================== FREE ======================================*/
/** Free solver memory allocation
 * @param solver The solver to remove
 */
void Solver_free(solver_t * solver){
    gsl_multiroot_fsolver_free(solver->_fsolver);
    gsl_vector_free(solver->_initVector);
    rot_free(solver->lastResult);
    free((void*) solver->params);
    free((void*) solver);
}


/*============================ GET ALPHA ====================================*/
/** Get the alpha angle from the XY plane
 * Any Y coordinate other than 0 requires servo 0 to rotate by alpha rad.<br>
 * This function calculates that alpha.<br>
 * Target coordinates are taken from solver->params.
 * @param solver The solver to use in calculations.
 * @return The alpha angle.
 */
angle_t Solver_calculateAlpha(solver_t* solver){
    const double X = solver->params->X;
    const double Y = solver->params->Y;
    const double length = sqrt(X * X + Y * Y);
    //change X in params with lenght
    solver->params->_X = length;

    angle_t alpha = asin(Y / length);
    //account for a negative X
    if (X < 0.0){
        alpha = M_PI - alpha;
    }
    return(Angle_normalize(alpha));
}


/*====================================== SOLVE ==============================*/
/** find useful angles for our servos.
 * using the parameter given in solver->params, this solves our multiroot
 * system.
 * @param solver The solver to use.
 * @retval 0 Success.
 * @retval 27 Could not reach.
 * @retval other See gsl solver status codes.
 */
int Solver_solve(solver_t* solver){
    //guess a beta angle
    angle_t betaGuess = 0.01;
    if (solver->params->Z < solver->params->C) betaGuess *= -1.0;
    gsl_vector_set(solver->_initVector, 0, betaGuess);

    //we can already write the alpha angle
    rot_vector_set(solver->lastResult, 0, Solver_calculateAlpha(solver));

    //set our gamma guess
    gsl_vector_set(solver->_initVector, 1, -M_PI_2);
    
    //set the function to use
    gsl_multiroot_function f = {&_trig_f2, 2, solver->params};
    gsl_multiroot_fsolver_set(solver->_fsolver, &f, solver->_initVector);

    //let's do this!
    int iter = 0;
    int status = -1;
    do{
        iter++;
        status = gsl_multiroot_fsolver_iterate(solver->_fsolver);
        if (status) break;
        status = gsl_multiroot_test_residual(solver->_fsolver->f, 1e-3);
    }while (status == GSL_CONTINUE && iter < 100);
    
    //print stuff if we failed
    if(status != GSL_SUCCESS){
        printf("status: %s\niter: %d\n", gsl_strerror(status), iter);
        printf("%.4g, %.4g, %s\n",
            gsl_vector_get(solver->_fsolver->x,0),
            gsl_vector_get(solver->_fsolver->x,1),
            gsl_strerror(status)
        );

    }

    //store result in lastResult
    rot_vector_set(solver->lastResult,
        1, gsl_vector_get(solver->_fsolver->x, 0));
    rot_vector_set(solver->lastResult,
        2, gsl_vector_get(solver->_fsolver->x, 1));

    return(status);
}


/*======================================= SET LENGTHS ======================*/
/** set the leg lenghts to be used. 
 * @param solver The solver to set the leglengths for,
 * @param lengths Pointer to a double[3] holding A B and C.
 */
void Solver_setLengths(solver_t* solver ,double* lengths){
	solver->params->A = lengths[0];
	solver->params->B = lengths[1];
	solver->params->C = lengths[2];
}


/*======================================= SET COORDS ======================*/
/** set the target coordinates to be used in calculating angles.
 * @param solver The solver to set the leglengths for,
 * @param coords Pointer to a double[3] holding X Y and Z.
 */
void Solver_setCoords(solver_t* solver ,double* coords){
	solver->params->X = coords[0];
	solver->params->Y = coords[1];
	solver->params->Z = coords[2];
}


