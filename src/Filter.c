/**
 * @file Filter.c
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

#include "Filter.h"
/* ================================= ALLOC ==================================*/
/** Allocate filter memory.
 * @return A pointer to the new filter data.
 */
filter_t* Filter_alloc(){
    filter_t* tempf = calloc(1, sizeof(filter_t));
    tempf->inputData = calloc(FILTER_GRAPH_LENGTH, sizeof(double));
    tempf->outputData = calloc(FILTER_GRAPH_LENGTH, sizeof(double));
    Filter_init(tempf);
    return(tempf);
}


/*==================================== FREE =================================*/
/** Free filter data from memory.
 * @param f The filter data to free.
 */
void Filter_free(filter_t* f){
    free(f->inputData);
    free(f->outputData);
    free(f);
}


/*==================================== INIT =================================*/
/** Set all values to their start values (except Sz/Sw).
 * @param f The filter data to reset.
 */ 
void Filter_init(filter_t* f){
    f->x = 0.0;
    f->_x_last = 0.0;
    f->_P = 0.0;
    f->_P_last = 0.0;
    //clear graphs
    memset((void*)f->inputData, 0, sizeof(double)*FILTER_GRAPH_LENGTH);
    memset((void*)f->outputData, 0, sizeof(double)*FILTER_GRAPH_LENGTH);
    f->dataIndex = 0;
}   


/*============================ STEP =========================================*/
/** Step the filter with an input.
 * @param f The filter data to use.
 * @param input The input to gice the filter.
 */
void Filter_step(filter_t* f, double input){
    double x_temp, P_temp, K;
    
    x_temp = f->_x_last;
    return;
    P_temp = f->_P_last + f->Sw;

    K = (1.0 / (P_temp + f->Sz) + P_temp);
    f->x = x_temp + K * (input - x_temp);
    f->_P = (1.0 - K) * P_temp;

    f->_x_last = f->x;
    f->_P_last = f->_P;
    Filter_addInputPoint(f, input);
    Filter_addOutputPoint(f, f->x);

}


//================================ ADD GRAPHPOINTS ==========================*/
/** Add a value to the dataset.
 * @param f The filter data to use.
 * @param value The value to add.
 * @return The new dataIndex.
 */
uint16_t Filter_addInputPoint(filter_t* f, double value){
    f->dataIndex++;
    if(f->dataIndex >= FILTER_GRAPH_LENGTH) f->dataIndex = 0;
    f->inputData[f->dataIndex] = value;
    return(f->dataIndex);
}

/** Same as Filter_addInputPoint (except for the output set).*/
uint16_t Filter_addOutputPoint(filter_t* f, double value){
    f->dataIndex++;
    if(f->dataIndex >= FILTER_GRAPH_LENGTH) f->dataIndex = 0;
    f->outputData[f->dataIndex] = value;
    return(f->dataIndex);
}


/*=========================== CHANGE GRAPH POINTERS =========================*/
/** Supply your own pointers to double arrays to store data points.
 * @param f The filter to change.
 * @param in The pointer to use for inputData.
 * @param out The pointer to use for outputData.
 */
void Filter_changeGraphPointers(filter_t* f, double* in, double* out){
    free(f->inputData);
    f->inputData = in;
    free(f->outputData);
    f->outputData = out;
}

