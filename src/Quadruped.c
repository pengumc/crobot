/**
 * @file quadruped.c
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
    return(tqped);
}


/*======================= Free ==============================================*/
/** Free qped mem.
 * @param qped The memory to free.
 */
void Quadruped_free(quadruped_t* qped){
    Usbdevice_free(qped->dev);
    free(qped);
}


/*==================== STARTUP ==============================================*/
int Quadruped_startup(quadruped_t* qped){
    int cnt = Usbdevice_connect(qped->dev);
    return(cnt);
}


/*======================== UPDATE ===========================================*/
int Quadruped_update(quadruped_t* qped){
    int cnt = Usbdevice_getData(qped->dev, qped->buffer);
    return(cnt);
}

/*======================= SET GRAPH POINTERS ================================*/
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
   Filter_changeGraphPointers(qped->dev->acc->filters[2], inY, outZ);
}


int Quadruped_getPsButton(quadruped_t* qped, pscontroller_button button){
    return(Pscontroller_getButton(&qped->dev->pscon, button));
}

int Quadruped_getPsButtonEdge(quadruped_t* qped, pscontroller_button button){
    return(Pscontroller_getButtonEdge(&qped->dev->pscon, button));
}
