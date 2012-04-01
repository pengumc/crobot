/** 
 * @file Pscontroller.c
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

#include "Pscontroller.h"
/*============================== ALLOC ======================================*/
/** Allocate memory for controller data.
 * @return Pointer to new controller data.
 */
pscontroller_t* Pscontroller_alloc(){
	pscontroller_t* tempController = calloc(1, sizeof(pscontroller_t));
	Pscontroller_init(tempController);
	return(tempController);
}


/*================================ INIT =====================================*/
/** (re)set all values to default.
 * @param pscon The controller to reset.
 */
void Pscontroller_init(pscontroller_t* pscon){
	pscon->ss_dpad = 0x00;
	pscon->previous_ss_dpad = 0x00;
	pscon->shoulder_shapes = 0x00;
	pscon->previous_shoulder_shapes = 0x00;
	pscon->rx = PS_DEFAULT_ANALOG;
	pscon->ry = PS_DEFAULT_ANALOG;
	pscon->lx = PS_DEFAULT_ANALOG;
	pscon->ly = PS_DEFAULT_ANALOG;
	pscon->rx_mid = PS_DEFAULT_ANALOG;
	pscon->ry_mid = PS_DEFAULT_ANALOG;
	pscon->lx_mid = PS_DEFAULT_ANALOG;
	pscon->ly_mid = PS_DEFAULT_ANALOG;
}


/*============================== FREE =======================================*/
/** Free controller data from memory.
 * @param pscontroller Data to free.
 */
void Pscontroller_free(pscontroller_t* pscontroller){
	free(pscontroller);
}


/*============================= UPDATE DATA =================================*/
/** Update controller state.
 * @param pscontroller The controller data to change.
 * @param ss_dpad Byte representing button states for start, select and the
 4 dpad buttons.<br>
 bit 7->{LEFT, DOWN, RIGHT, UP, START, IGNORE, IGNORE, SELECT}<-bit 0.
 * @param shoulder_shapes Byte representing the button states for the cross,
  triangle, square, circle and the 4 shoulder buttons.<br>
 bit 7->{SQUARE, CROSS, CIRCLE, TRIANGLE, R1, L1, R2, L2}<-bit 0.
 * @param rx Byte representing the right analog x axis.
 * @param ry Byte representing the right analog y axis.
 * @param lx Byte representing the left analog x axis.
 * @param ly Byte representing the left analog y axis.
 */
void Pscontroller_updateData(pscontroller_t* pscontroller, 
    uint8_t ss_dpad, uint8_t shoulder_shapes,
    uint8_t rx, uint8_t ry, uint8_t lx, uint8_t ly)
{
    pscontroller->previous_ss_dpad = pscontroller->ss_dpad;
    pscontroller->ss_dpad = ss_dpad;

    pscontroller->previous_shoulder_shapes = pscontroller->shoulder_shapes;
    pscontroller->shoulder_shapes = shoulder_shapes;

    pscontroller->rx = rx;
    pscontroller->ry = ry;
    pscontroller->lx = lx;
    pscontroller->ly = ly;
}


/*======================== SET MID ==========================================*/
/** Save the current analog stick states as resting positions.
 * @param pscontroller The controller data to use.
 */
void Pscontroller_saveMid(pscontroller_t* pscontroller){
    pscontroller->rx_mid = pscontroller->rx;
    pscontroller->ry_mid = pscontroller->ry;
    pscontroller->lx_mid = pscontroller->lx;
    pscontroller->ly_mid = pscontroller->ly;
}


/*=============================== ANAGLOG ON? ===============================*/
/** deprecated.*/
int Pscontroller_isAnalogOn(pscontroller_t* pscontroller){
	const int mid = pscontroller->rx_mid
		+ pscontroller->ry_mid
		+ pscontroller->lx_mid
		+ pscontroller->ly_mid;
	return(1);
}


/*============================ GET BUTTON ===================================*/
/** Get button state.
 * @param pscontroller The controller to use.
 * @param button The button to retreive.
 * @retval 1 Button is pressed.
 * @retval 0 Button is up.
 */
int Pscontroller_getButton(pscontroller_t* pscontroller,
	pscontroller_button button)
{
	if(button < PS_L2){
		//check ss_dpad
		if(CHK(pscontroller->ss_dpad, button) == 0) return(1);
		else return(0);
	}else{
		//check shoulder_shapes
		if(CHK(pscontroller->shoulder_shapes, button - PS_L2) == 0) return(1);
		else return(0);
	}
}


/*============================= GET AXIS ====================================*/
/** Get one of the analog axis.
 * @param pscontroller The date to get the axis from.
 * @param axis The axis to get.
 * @return The value of the axis (-128..127) with 0 = axis_mid.
 */
int8_t Pscontroller_getAxis(pscontroller_t* pscontroller,
	pscontroller_axis axis)
{
	switch(axis){
		case PS_RX:
			return(pscontroller->rx - pscontroller->rx_mid);
		case PS_RY:
			return(pscontroller->ry - pscontroller->ry_mid);
		case PS_LX:
			return(pscontroller->lx - pscontroller->lx_mid);
		case PS_LY:
			return(pscontroller->ly - pscontroller->ly_mid);
	}
}


/*====================== GET BUTTON EDGE ====================================*/
/** Check for rising of falling edge of a button.
 * @param pscontroller The data to check.
 * @param button The button to check.
 * @retval 1  rising edge.
 * @retval 0  no chance since last update
 * @retval -1 falling edge.
 */
int Pscontroller_getButtonEdge(pscontroller_t* pscontroller,
	pscontroller_button button)
{
	if(button < PS_L2){
        printf("checking ss_dpad\n");
		//check ss_dpad
        printf("0x%X vs 0x%X\n", pscontroller->ss_dpad, pscontroller->previous_ss_dpad);
		if(CHK(pscontroller->ss_dpad, button) 
			> CHK(pscontroller->previous_ss_dpad, button))
		{
			return(1);
		}
		else if (CHK(pscontroller->ss_dpad, button) 
			< CHK(pscontroller->previous_ss_dpad, button))
		{
			return(-1);
		}
		else
		{
			return(0);
		}
	}else{
		//check shoulder_shapes
		if(CHK(pscontroller->shoulder_shapes, button - PS_L2) 
			> CHK(pscontroller->previous_shoulder_shapes, button - PS_L2))
		{
			return(1);
		}
		else if (CHK(pscontroller->shoulder_shapes, button - PS_L2) 
			< CHK(pscontroller->previous_shoulder_shapes, button - PS_L2))
		{
			return(-1);
		}
		else
		{
			return(0);
		}
	}
}
