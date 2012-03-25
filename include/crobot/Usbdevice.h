/**
 * @file Usbdevice.h
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

usbdevice_t* Usbdevice_alloc();
void Usbdevice_free(usbdevice_t* usbdevice);

void Usbdevice_init(usbdevice_t* usbdevice);
int Usbdevice_connect(usbdevice_t* usbdevice);

#endif

