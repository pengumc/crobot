#include "Solver.h"
#include "rotation.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    if(argc<4){
        printf("not enough arguments, provide x y z. lengths are 1 10 10\n");
        return(1);
    }    
    unsigned char i;
    for(i=0;i<argc;i++){
        printf("argument: %s\n", argv[i]);
    }
    solver_t* mySolver = Solver_alloc();
    mySolver->params->A = 1.0;
    mySolver->params->B = 10.0;
    mySolver->params->C = 10.0;
    mySolver->params->X = atof(argv[1]);
    mySolver->params->Y = atof(argv[2]);
    mySolver->params->Z = atof(argv[3]);
    
    printf("solver status: %d\n", Solver_solve(mySolver));
    rot_vector_print(mySolver->lastResult);

    Solver_free(mySolver);

}
