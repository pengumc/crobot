#define __MAIN__
#include <stdio.h>
#include "Usbdevice.h"
#include "Report.h"

int main(int argc, char* argv[]){
    int plop;
    printf("pointer size = %d bytes\n", sizeof(&plop));
    printf("unsigned char = %d\n", sizeof(unsigned char));
    printf("int size = %d\n", sizeof(int));
    usbdevice_t* myDev = Usbdevice_alloc();
    printf("sizeof usbdev = %d\n", sizeof(usbdevice_t));
    printf("%s\n", myDev->vendor);
    printf("dev connect: %d\n", Usbdevice_connect(myDev));
    Usbdevice_free(myDev);

    return(0);
}

