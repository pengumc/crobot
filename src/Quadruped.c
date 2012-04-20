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
 * brief Every 40 ms is recommended.
 * @param qped The quadruped data to use.
 * @returns the connection status.
 */
int Quadruped_update(quadruped_t* qped){
    int cnt = Usbdevice_getData(qped->dev, qped->buffer);
    return(cnt);
}

/*======================= SET GRAPH POINTERS ================================*/
/** Change location of graph datasets.
 * @brief Make the three filters store both their input and output datasets
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


/*===================== COMMIT TO DEVICE ====================================*/
int Quadruped_commit(quadruped_t* qped){
    Usbdevice_sendServoData(qped->dev);
}



/*===================== UPDATE ROT MATRIX ===================================*/
void Quadruped_updateMatricesFromAngles(quadruped_t* qped, rot_vector_t* a){
    rot_matrix_build_from_angles(qped->R, a);
    rot_matrix_invert(qped->R, qped->invR);
}


/*================== CHANGE LEG ENDPOINT ===================================+*/
int Quadruped_changeLegEndpoint(quadruped_t* qped, uint8_t legNo,
    double X, double Y, double Z)
{
    //rotate the requested change with the inverse of our current rotation

}


