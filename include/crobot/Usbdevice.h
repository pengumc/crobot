#ifndef __USBDEVICE__
#define __USBDEVICE__
#define USBCOMMANDLINE

#include <usb.h>
#include <string.h>
#include "opendevice.h"
#include "usbconfig.h"
#include "i2c_header.h"
#include "requests.h"
#include "Report.h"


#define USBDEV_RETRY 3
#define USBDEV_TIMEOUT_MS 500

typedef struct{
    usb_dev_handle* handle;
    int vid;
    int pid;
//    char servoDataBuffer[BUFLEN_SERVO_DATA];
    char* vendor; /**< vendor string. */
    char* product; /**< product string. */
    int connected;    
} usbdevice_t;

typedef enum{
    USBDEV_READ = USB_ENDPOINT_IN,
    USBDEV_WRITE = USB_ENDPOINT_OUT
} usbdevice_reqType;

usbdevice_t* usbdevice_alloc();
void usbdevice_free(usbdevice_t* usbdevice);

void usbdevice_init(usbdevice_t* usbdevice);
int usbdevice_connect(usbdevice_t* usbdevice);

#endif

