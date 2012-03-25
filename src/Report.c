/**
 * @file Report.c
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

#include "Report.h"


/*================================= REPORT STD =============================*/
/** notify the user of something.
 * @param text The text you want the user to see.
 */
void Report_std(const char* text){
    printf("%s\n", text);
}


/*================================= REPORT ERR =============================*/
/** notify the user of an error.
 * @param text The text you want the user to see.
 */
void Report_err(const char* text){
    fprintf(stderr, "%s\n", text);

}
