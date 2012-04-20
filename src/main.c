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


#define __MAIN__
#include <stdio.h>
#include "Report.h"
#include "Quadruped.h"
#include "Leg.h"

int main(int argc, char* argv[]){
    int plop;
    printf("pointer size = %d bytes\n", sizeof(&plop));
    printf("int size = %d\n", sizeof(int));
    printf("double size = %d\n", sizeof(double));
	
	leg_t* myLeg = Leg_alloc();
	
	Servo_setOffset(myLeg->servos[2], -M_PI/2.0);
	
	myLeg->legSolver->params->A = 10.0;
	myLeg->legSolver->params->B = 10.0;
	myLeg->legSolver->params->C = 10.0;
	
	Leg_updateServoLocations(myLeg);
	Leg_printDetails(myLeg);
	
	printf("try z+5 : %d\n", Leg_tryEndpointChange(myLeg, 
	
	Leg_free(myLeg);
	
    return(0);
}

