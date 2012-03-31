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

int main(int argc, char* argv[]){
    int plop;
    printf("pointer size = %d bytes\n", sizeof(&plop));
    printf("unsigned char = %d\n", sizeof(unsigned char));
    printf("int size = %d\n", sizeof(int));
    
    
    return(0);
}

