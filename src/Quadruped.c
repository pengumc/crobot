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
    Quadruped_updateMatricesFromAngles(tqped, tqped->angles);
    return(tqped);
}


/*======================= Free ==============================================*/
/** Free qped mem.
 * @param qped The memory to free.
 */
void Quadruped_free(quadruped_t* qped){
    Usbdevice_free(qped->dev);
    rot_free(qped->R);
    rot_free(qped->invR);
    rot_free(qped->angles);
    free(qped);
}


/*==================== STARTUP ==============================================*/
/** Startup function (mainly try to connect usb).
 * @param qped The quadruped data to use.
 * @retval <1 Not connected.
 * @retval >1 Connected, numbers of retries left. 
 */
int Quadruped_startup(quadruped_t* qped){
    int cnt = Usbdevice_connect(qped->dev);
    return(cnt);
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

void Quadruped_configureServoDirection(quadruped_t* qped, 
    uint8_t legNo, uint8_t servoNo, int8_t direction)
{
    if(legNo < USBDEV_LEGNO && servoNo < LEG_DOF){
        if(direction > 0) direction = 1;
        else direction = -1;
        Servo_setDirection(qped->dev->legs[legNo]->servos[servoNo], direction);
    }
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


/*================== CHANGE SINGE LEG ENDPOINT =============================+*/
/** Move a single leg.
 * The provided coordinates are for the endpoint (foot) of the leg. So 
 an increase in Z means servo 0 is going down. This is regardless of 
 the orientation of the mainbody (at least the part we know about).
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
    return(returnCode);
}


/*======================= MOVE ALL LEGS======================================*/
/** Move the COB by moving all legs
 * @param qped
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
    }
    return(error);
}


/*====================== GET SERVO DATA FROM DEVICE =========================*/
/** Get the servo positions currently stored on the device.
 *
 */
int Quadruped_getServoData(quadruped_t* qp){
    int ret = Usbdevice_getServoData(qp->dev, qp->buffer);
    printf("qp_getservodata: %d\n", ret);
    return(ret);
}
