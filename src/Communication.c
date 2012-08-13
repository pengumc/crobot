/**
 * @file Communication.c
 * @brief data transfer functions for external use
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

communication_t* Communication_alloc(){
    communication_t* com =
        (communication_t*) calloc(1, sizeof(communication_t));
    return(com);
}

void Communication_free(communication_t* com){
    free(com);
}

void Communication_connectGraphData(communication_t* com,
    accelerometer_t* acc)
{
    //we're not actively using the graph data, we only write to it
    //so we can freely give away the pointers.
    com->graphs->inX = qped->dev->acc->filter[0]->inputData;
    com->graphs->inY = qped->dev->acc->filter[1]->inputData;
    com->graphs->inZ = qped->dev->acc->filter[2]->inputData;
    com->graphs->outX = qped->dev->acc->filter[0]->outputData;
    com->graphs->outY = qped->dev->acc->filter[1]->outputData;
    com->graphs->outZ = qped->dev->acc->filter[2]->outputData;
}

void Communication_updatePWA(communication_t* com, quadruped_t* qp){
    char leg, servo, i;
    i = 0;
    for(leg=0; leg<USBDEV_LEGNO; leg++){
        for(servo=0;servo<LEG_DOF; servo++){
            com->pulsewidths[i] = qp->dev->legs[leg]->servos[servo]->_pw;
            com->angles[i] = qp->dev->legs[leg]->servos[servo]->_angle;
            i++;
        }
    }
}

void Communication_updatePos(communication_t* com, quadruped_t* qp){
    int i,j;
    int index = 0;
    rot_vector_t* v = rot_vector_alloc();
    for(i=0;i<USBDEV_LEGNO;i++){
        for(j=0;j<LEG_DOF;j++){
            rot_vector_copy(qp->dev->legs[i]->servoLocations[j], v);
            rot_vector_add(v, qp->dev->legs[i]->offsetFromCOB);
            com->servopos.x[index] = rot_vector_get(v, 0);
            com->servopos.y[index] = rot_vector_get(v, 1);
            com->servopos.z[index] = rot_vector_get(v, 2);
            index ++;
        }
    }
    rot_free(v);
}           

void Communication_updateEndpoints(communication_t* com, quadruped_t* qp){
    int i;
    rot_vector_t* v = rot_vector_alloc();
    for(i=0;i<USBDEV_LEGNO;i++){
        rot_vector_copy(qp->dev->legs[i]->servoLocations[LEG_DOF], v);
        rot_vector_add(v, qp->dev->legs[i]->offsetFromCOB);
        com->endpoints.x[i] = rot_vector_get(v, 0);
        com->endpoints.y[i] = rot_vector_get(v, 1);
        com->endpoints.z[i] = rot_vector_get(v, 2);
    }
    rot_free(v);
}

