/**
 * @file Quadruped.c
 * @brief The default leg configuration has the rotation of servo 0 in the XY
 plane and in the XZ plane for 1 and 2.
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


#include "Quadruped.h"

/* ===============================
 *  UTILITY
 * =============================*/
void update_servoAction(quadruped_t* qped){
    if(qped->com != NULL){
        Communication_updateEndpoints(qped->com, qped);
        Communication_updatePWA(qped->com, qped);
        Communication_updatePos(qped->com, qped);
    }
}



/*======================= ALLOC =============================================*/
/** Allocate quadruped memory.
 * @return A pointer to the new quadruped memory.
 */
quadruped_t* Quadruped_alloc(){
    quadruped_t* tqped = calloc(1, sizeof(quadruped_t));
    tqped->dev = Usbdevice_alloc();
    tqped->R = rot_matrix_alloc();
    tqped->invR = rot_matrix_alloc();
    tqped->angles = rot_vector_alloc();
    rot_vector_setAll(tqped->angles, 0.0, 0.0, 0.0);
    tqped->com = NULL;
    
    //TODO remove servoinfo stuff
    tqped->si = (servoinfo_t*)calloc(1, sizeof(servoinfo_t));
    
    Quadruped_updateMatricesFromAngles(tqped, tqped->angles);
    Quadruped_updateServoinfo(tqped);
    
    return(tqped);
}


/*======================= Free ==============================================*/
/** Free qped mem.
 * @param qped The memory to free.
 */
void Quadruped_free(quadruped_t* qped){
    Usbdevice_free(qped->dev);
    Communication_free(qped->com);
    rot_free(qped->R);
    rot_free(qped->invR);
    rot_free(qped->angles);
    free(qped->si);
    free(qped);
}

/*========================== ENABLE COMMUNICATION ===========================*/
/** Use your own allocated com memory*/
void Quadruped_enableCommunication(quadruped_t* qp, communication_t* com){
    qp->com = com;
    update_servoAction(qp);
}

/*==================== STARTUP ==============================================*/
/** Startup function (mainly try to connect usb).
 * @param qped The quadruped data to use.
 * @retval <1 Not connected.
 * @retval >1 Connected, numbers of retries left. 
 */
int Quadruped_startup(quadruped_t* qped){
    int cnt = Usbdevice_connect(qped->dev);
    Quadruped_getServoData(qped);
    return(cnt);
}

/*======================== LOAD STARTING POSISTION ==========================*/
/** Restore the servos to the starting positions stored on the device.
 * @param qped The quadruped to use.
 * @retval 0 Success.
 * @retval -1 Failure.
 */
int Quadruped_loadStartPos(quadruped_t* qped){
   int result = Usbdevice_loadPositions(qped->dev); 
   Quadruped_getServoData(qped);
}

/*================= CONFIG LEG ==============================================*/
/** Set the offset for one of the servos in a leg.
 * @param qped The quadruped data to use.
 * @param legNo The leg number (0..3).
 * @param servoNo The servo to change (0..2).
 * @param offset The offset to set (in radians).
 */
void Quadruped_configureServoOffset(quadruped_t* qped,
    uint8_t legNo, uint8_t servoNo, angle_t offset)
{
    if(legNo < USBDEV_LEGNO && servoNo < LEG_DOF)
        Servo_setOffset(qped->dev->legs[legNo]->servos[servoNo], offset);
}



/*===================== CONFIG SERVO DIRECTION ==============================*/
/** Set the turning direction of a servo.
 * @param qped The quadruped data to use.
 * @param legNo Which leg.
 * @param servoNo Which servo.
 * @param direction The new direction. Higher than 0 gives a positive
 * direction, lower will give a negative direction.
 */
void Quadruped_configureServoDirection(quadruped_t* qped, 
    uint8_t legNo, uint8_t servoNo, int8_t direction)
{
    if(legNo < USBDEV_LEGNO && servoNo < LEG_DOF){
        if(direction > 0) direction = 1;
        else direction = -1;
        Servo_setDirection(qped->dev->legs[legNo]->servos[servoNo], direction);
    }
}



/*=========================== CONFIG LEG LENGTHS ============================*/
/** Set the lengths of the different segments per leg.
 * @param qped The quadruped to use.
 * @param legNo Which leg to set the segments for.
 * @param A Length of segment A in cm.
 * @param B Length of segment B in cm.
 * @param C Length of segment C in cm.
 */
void Quadruped_configureLegLengths(quadruped_t* qped,
    uint8_t legNo, double A, double B, double C)
{
    if(legNo < USBDEV_LEGNO) Leg_setLengths(qped->dev->legs[legNo], A, B, C);
}

/*========================== CONFIG LEG OFFSET ==============================*/
void Quadruped_configureLegOffset(quadruped_t* qped,
    uint8_t legNo, double X, double Y, double Z)
{
    rot_vector_setAll(qped->dev->legs[legNo]->offsetFromCOB, X, Y, Z);
}

/*======================== UPDATE ===========================================*/
/** Update function, should be called often to update accelerometer and 
gamepad data.
 * Every 40 ms or so is recommended. Each update sends an usb control message
 to the robot, so don't go spamming it a million times per second.
 * @param qped The quadruped data to use.
 * @returns the connection status.
 */
int Quadruped_update(quadruped_t* qped){
    int cnt = Usbdevice_getData(qped->dev, qped->buffer);
    return(cnt);
}


/*======================================= GET BUTTON (EDGE) =================*/
/** Query button state.
 * @param qped The quadruped data to use.
 * @param button The playstation controller button to query.
 * @retval 1 Button is pressed.
 * @retval 0 Button is not pressed.
 */
int Quadruped_getPsButton(quadruped_t* qped, pscontroller_button button){
    return(Pscontroller_getButton(&qped->dev->pscon, button));
}

/** Check for a change in the button state.
 * @param qped The quadruped data to use.
 * @param button The playstation controller button to query.
 * @retval 1 There was a rising edge.
 * @retval 0 No change.
 * @retval -1 There was a falling edge.
 */
int Quadruped_getPsButtonEdge(quadruped_t* qped, pscontroller_button button){
    return(Pscontroller_getButtonEdge(&qped->dev->pscon, button));
}


int Quadruped_getPsAxis(quadruped_t* qped, pscontroller_axis axis){
    return(Pscontroller_getAxis(&qped->dev->pscon, axis));
}


/*===================== COMMIT TO DEVICE ====================================*/
int Quadruped_commit(quadruped_t* qped){
    return(Usbdevice_sendServoData(qped->dev));
}

/*====================== GET SERVO DATA FROM DEVICE =========================*/
/** Get the servo positions currently stored on the device.
 * into local storage
 */
int Quadruped_getServoData(quadruped_t* qp){
    int ret = Usbdevice_getServoData(qp->dev, qp->buffer);
    printf("qp_getservodata: %d\n", ret);
    update_servoAction(qp);
    return(ret);
}


/*===================== UPDATE ROT MATRIX ===================================*/
/** Rebuilds the rotation matrix and it's inverse from the provided angles.
 * @param qped The quadruped to use.
 * @param a A 3d vector that holds angles for the rotation around the x y and
 z axis (in that order).
 */
void Quadruped_updateMatricesFromAngles(quadruped_t* qped, rot_vector_t* a){
    rot_matrix_build_from_angles(qped->R, a);
    rot_matrix_invert(qped->R, qped->invR);
}

/*=================== ROTATE =================================================*/
int Quadruped_rotate(quadruped_t* qped, 
    double xaxis, double yaxis, double zaxis)
{
    /*
     * v = delta angles
     * build R and invR from v
     * V = absolute nonrotated position of endpoint
     * newV = invR * endpoint
     * change to newV - V
     * add delta angles to angles and update matrices
     */
    
    //store angles as vector in v for a call to updateMatrices
    rot_vector_t* dangles = rot_vector_alloc();
    rot_vector_setAll(dangles, xaxis, yaxis, zaxis);
    //update Matrices
    Quadruped_updateMatricesFromAngles(qped, dangles);
    
    rot_vector_t* v  = rot_vector_alloc();
    rot_vector_t* tempv = rot_vector_alloc();
    
    int i;
    unsigned int error = 0 ;
    for(i=0;i<USBDEV_LEGNO;i++){
        //grab absolute position
        rot_vector_copy(qped->dev->legs[i]->servoLocations[LEG_DOF], v);
        rot_vector_add(v, qped->dev->legs[i]->offsetFromCOB); //unrotated 
        //rotate backwards
        rot_matrix_dot_vector(qped->invR, v, tempv);
        //get difference 
        rot_vector_minus(tempv, v);
        //and Leg_tryEndpointChange
        error += 
            ( (unsigned int)
                (-Leg_tryEndpointChange(qped->dev->legs[i], tempv) )
            ) << i*4;
    }
    if(error == 0){
        //no errors, commit everything.
        for(i=0;i<USBDEV_LEGNO;i++){
            Leg_commitEndpointChange(qped->dev->legs[i]);
        }
        update_servoAction(qped);
        rot_vector_add(qped->angles, dangles);
        Quadruped_updateMatricesFromAngles(qped, qped->angles);
    }else{
        //errors, do a rollback
        for(i=0;i<USBDEV_LEGNO;i++){
            Leg_performRollback(qped->dev->legs[i]);
        }
    }
    Quadruped_updateMatricesFromAngles(qped, qped->angles);
    rot_free(v);
    rot_free(tempv);
    rot_free(dangles);
    return(error);
}


/*================== CHANGE SINGE LEG ENDPOINT ==============================*/
/** Move a single leg.
 * The provided coordinates are for the endpoint (foot) of the leg. So 
 an increase in Z means servo 0 is going down. This is regardless of 
 the orientation of the mainbody (at least the part we know about).
 Changes are send to the device if they're acceptable.
 * @param qped The quadruped data to use.
 * @param legNo The leg number (0..3).
 * @param X Change in x direction.
 * @param Y Change in y direction.
 * @param Z Change in z direction.
 * @retval 0 success, you can send the newly stored servo data to the
 device
 * @retval -1..3 servo number (-returncode +1) could not make the required 
 angle, nothing was done.
 * @retval -4 There was no solution for the new location, nothing was done.
 */
int Quadruped_changeLegEndpoint(quadruped_t* qped, uint8_t legNo,
    double X, double Y, double Z)
{
    //build delta vector
    rot_vector_t* v = rot_vector_alloc();
    rot_vector_setAll(v, X, Y, Z);
    //rotate to offset main body rotation
    rot_matrix_dot_vector(qped->invR, v, v);
    
    int returnCode = Leg_tryEndpointChange(qped->dev->legs[legNo], v);
    if(returnCode == 0){
        Leg_commitEndpointChange(qped->dev->legs[legNo]);
    }
    
    
    rot_free(v);
    update_servoAction(qped);
    return(returnCode);
}


/*======================= MOVE ALL LEGS======================================*/
/** Move the COB by moving all legs
 * Changes the endpoints of all legs by the same amount. Essentially moving
 the center of the quadruped in the opposite direction.
 Changes are send to the device if they're acceptable.
 * @param qped The quadruped to use.
 * @param X the change in the x direction.
 * @param Y the change in the y direction.
 * @param Z the change in the z direction.
 * @return 4 bytes as an int, each representing the error state of 1 leg.
 */
int Quadruped_changeAllEndpoints(quadruped_t* qped,
    double X, double Y, double Z)
{
    /* TODO this error returning will fail horribly if 
     sizeof(int) < USBDEV_LEGNO
    */
    rot_vector_t* v = rot_vector_alloc();
    rot_vector_setAll(v, X, Y, Z);
    //rotate to offset main body rotation
    rot_matrix_dot_vector(qped->invR, v, v);

    uint8_t i;
    unsigned int error = 0;
    for(i=0;i<USBDEV_LEGNO;i++){
        //error should end up on 0 if all succeeds;
        error += 
            ( (unsigned int)
                (-Leg_tryEndpointChange(qped->dev->legs[i], v) )
            ) << i*4;
    }
    if(error == 0){
        //no errors, commit everything.
        for(i=0;i<USBDEV_LEGNO;i++){
            Leg_commitEndpointChange(qped->dev->legs[i]);
        }
    }else{
        //errors, do a rollback
        for(i=0;i<USBDEV_LEGNO;i++){
            Leg_performRollback(qped->dev->legs[i]);
        }
    }
    update_servoAction(qped);
    rot_free(v);
    return(error);
}


/*======================= SET ALL LEGS======================================*/
/** Set all endpoints to the same coordinates (relative to the leg origin)
 * This will set set all legs to the same height so the quadruped should be
 * level after calling this.
 Changes are send to the device if they're acceptable.
 * @param qped The quadruped to use.
 * @param Z the z coordinate (main body height).
 * @return 4 bytes as an int, each representing the error state of 1 leg.
 */
int Quadruped_setAllEndpointZs(quadruped_t* qped, double Z){
    //for each leg, create a delta and try a change
    rot_vector_t* v = rot_vector_alloc();
    int i;
    unsigned int error = 0;
    for(i=0;i<USBDEV_LEGNO;i++){
        rot_vector_setAll(v,
            0.0,
            0.0,
            Z - qped->dev->legs[i]->legSolver->params->Z);
        error +=
            ( (unsigned int)
                (-Leg_tryEndpointChange(qped->dev->legs[i], v) )
            ) << i*4;
    }
    if(error == 0){
        //no errors, commit everything.
        for(i=0;i<USBDEV_LEGNO;i++){
            Leg_commitEndpointChange(qped->dev->legs[i]);
        }
    }else{
        //errors, do a rollback
        for(i=0;i<USBDEV_LEGNO;i++){
            Leg_performRollback(qped->dev->legs[i]);
        }
    }
    update_servoAction(qped);
    rot_free(v);
    return(error);
}


/*======================= CHANGE SERVO ======================================*/
/** Change the angle of a single servo by an amount.
 * @param qped The quadruped to use.
 * @param l The leg number (0..3).
 * @param s The servo to change (0..2).
 * @param angle The amount to change the servo angle by.
 * @retval 1 success.
 * @retval 0 Angle was out of bounds for this servo. nothing was changed.
 */
int Quadruped_changeSingleServo(
    quadruped_t* qped, uint8_t l, uint8_t s, double angle)
{
    int result = Leg_changeServoAngle(qped->dev->legs[l], s, angle);
    Leg_resyncSolverParams(qped->dev->legs[l]);
    update_servoAction(qped);
    return(result);
}

/*======================= DEBUG LEGS======================================*/
void Quadruped_debugLegs(quadruped_t* qp){
    char i;
    for(i=0;i<USBDEV_LEGNO;i++){
        Leg_printDetails(qp->dev->legs[i]);   
    }
}

/*========================= PRINT SERVO DETAILS ==============================*/
/** Print servodetails to stdout
 * @param qp The quadruped to use.
 * @param legNo The leg the servo is in.
 * @param servoNo The servo from that leg to print.
 */
void Quadruped_printServoDetails(quadruped_t* qp,
  uint8_t legNo, uint8_t servoNo)
{
    Servo_printDetails(qp->dev->legs[legNo]->servos[servoNo], NULL);
}






/*==================================================
 * TO BE DELETED
 * ===============================================*/


/*======================= SET GRAPH POINTERS ================================*/
/** Change location of graph datasets.
 * Make the three filters store both their input and output datasets
 somewhere else (where it's easier for you to read them).<br>
 All [in/out][x/y/z] parameters should be pointers to an array with at room 
 for at least  FILTER_GRAPH_LENGTH doubles.
 * @param qped The quadruped data to use.
 * @param inX input value array for X filter.
 * @param inY input value array for Y filter.
 * @param inZ input value array for Z filter.
 * @param outX output value array for X filter.
 * @param outY output value array for Y filter.
 * @param outZ output value array for Z filter.
 */
void Quadruped_setGraphPointers(quadruped_t* qped,
    double* inX, 
    double* inY, 
    double* inZ, 
    double* outX, 
    double* outY, 
    double* outZ)
{
   Filter_changeGraphPointers(qped->dev->acc->filters[0], inX, outX);
   Filter_changeGraphPointers(qped->dev->acc->filters[1], inY, outY);
   Filter_changeGraphPointers(qped->dev->acc->filters[2], inZ, outZ);
}


/*================== GRAB SERVOINFO =========================================*/
/** Fill the provided servoinfo with current data.
 * Data is angles and pulsewidths for all servos. You need to call this
 * each time you want new data. This is the only function that changes values
 * in the servoinfo structure.
 * @param qp The quadruped to use.
 */
void Quadruped_updateServoinfo(quadruped_t* qp){
    char leg, servo, i;
    i = 0;
    for(leg=0; leg<USBDEV_LEGNO; leg++){
        for(servo=0;servo<LEG_DOF; servo++){
            qp->si->pulsewidths[i] = qp->dev->legs[leg]->servos[servo]->_pw;
            qp->si->angles[i] = qp->dev->legs[leg]->servos[servo]->_angle;
            i++;
        }
    }
}

servoinfo_t* Quadruped_getServoinfoPointer(quadruped_t* qp){
    return(qp->si);
}
