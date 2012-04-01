/**
 * @file quadruped.h
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

#ifndef __QUADRUPED__
#define __QUADRUPED__

#include "Usbdevice.h"
#include "Filter.h"
#include "Pscontroller.h"

/**Quadruped data.*/
typedef struct QPED{
    usbdevice_t* dev; /**<The usb helper.*/
    uint8_t buffer[BUFLEN_SERVO_DATA]; /**<The usb buffer.*/
} quadruped_t;

quadruped_t* Quadruped_alloc();
void Quadruped_free(quadruped_t* qped);

int Quadruped_update(quadruped_t* qped);

void Quadruped_setGraphPointers(quadruped_t* qped,
    double* inX, 
    double* inY, 
    double* inZ, 
    double* outX, 
    double* outY, 
    double* outZ);

#endif


