/** 
 * @file Pscontroller.h
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

#ifndef __PSCONTROLLER__
#define __PSCONTROLLER__

#include <stdint.h>
#include <stdlib.h>

#define PS_DEFAULT_ANALOG 128
#define CHK(x,y) (x&(1<<y)) 

//buttons
typedef enum{
	PS_SELECT = 0,
	PS_START = 3,
	PS_UP = 4,
	PS_RIGHT = 5,
	PS_DOWN = 6,
	PS_LEFT = 7,
	//substract 8 from these to get the bit numbers
	PS_L2 = 8,
	PS_R2 = 9,
	PS_L1 = 10,
	PS_R1 = 11,
	PS_TRIANGLE = 12,
	PS_CIRCLE = 13,
	PS_CROSS = 14,
	PS_SQUARE = 15,
} pscontroller_button;

//axis
typedef enum{
	PS_RX = 0,
	PS_RY = 1,
	PS_LX = 2,
	PS_LY = 3
} pscontroller_axis;

typedef struct {
  uint8_t ss_dpad;
  uint8_t shoulder_shapes;
  uint8_t previous_ss_dpad;
  uint8_t previous_shoulder_shapes;
  uint8_t rx;
  uint8_t ry;
  uint8_t lx;
  uint8_t ly;
  uint8_t rx_mid;
  uint8_t ry_mid;
  uint8_t lx_mid;
  uint8_t ly_mid;
}pscontroller_t ;

pscontroller_t* Pscontroller_alloc();
void Pscontroller_init(pscontroller_t* pscon);
void Pscontroller_free(pscontroller_t* pscontroller);

void Pscontroller_saveMid(pscontroller_t* pscontroller);

int Pscontroller_isAnalogOn(pscontroller_t* pscontroller);

int Pscontroller_getButton(pscontroller_t* pscontroller,
	pscontroller_button button);

int8_t Pscontroller_getAxis(pscontroller_t* pscontroller,
	pscontroller_axis axis);

int Pscontroller_getButtonEdge(pscontroller_t* pscontroller,
	pscontroller_button button);
	
void Pscontroller_updateData(pscontroller_t* pscontroller, 
    uint8_t ss_dpad, uint8_t shoulder_shapes,
    uint8_t rx, uint8_t ry, uint8_t lx, uint8_t ly);



#endif

