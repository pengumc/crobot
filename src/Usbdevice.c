/**
 * @file Usbdevice.c
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

#include "Usbdevice.h"


/*==================================== ALLOC ================================*/
/** Allocate memory for a usbdevice.
 * Also set the default values. No connection is attempted.
 * @return A pointer to a new device.
 */
usbdevice_t* Usbdevice_alloc(){
    usbdevice_t* tempDev = calloc(1, sizeof(usbdevice_t));
    //alloc mem for strings
    tempDev->vendor = calloc(USB_CFG_VENDOR_NAME_LEN+1, sizeof(char));
    tempDev->product = calloc(USB_CFG_DEVICE_NAME_LEN+1, sizeof(char));
    Usbdevice_init(tempDev);
    usb_init();
    return(tempDev);
}


/*================================== FREE ===================================*/
/** Delete a usbdevice from memory.
 * @param usbdevice The device to delete
 */
void Usbdevice_free(usbdevice_t* usbdevice){
    if(usbdevice->handle) usb_close(usbdevice->handle);
    free(usbdevice->vendor);
    free(usbdevice->product);
    free(usbdevice);
}


/*============================== INIT =======================================*/
/** Set default values for a usbdevice.
 * Values are defined in i2c_header.h file as well as usbconfig.h
 */
void Usbdevice_init(usbdevice_t* usbdevice){
    //use vid and pid from usbconfig.h
    unsigned char tempVID[2] = {USB_CFG_VENDOR_ID};
    usbdevice->vid = tempVID[0] + tempVID[1] * 256;
    unsigned char tempPID[2] = {USB_CFG_DEVICE_ID};
    usbdevice->pid = tempPID[0] + tempPID[1] * 256;
    
    //load strings and memcpy them to the usbdevice mem
    char tempVendor[USB_CFG_VENDOR_NAME_LEN+1] = {USB_CFG_VENDOR_NAME,0};
    char tempProduct[USB_CFG_DEVICE_NAME_LEN+1] = {USB_CFG_DEVICE_NAME,0};

    memcpy(usbdevice->vendor, tempVendor, USB_CFG_VENDOR_NAME_LEN+1);
    memcpy(usbdevice->product, tempProduct, USB_CFG_DEVICE_NAME_LEN+1);
	
	Pscontroller_init(&usbdevice->pscon);
}


/*============================ CONNECT =====================================*/
/** Connect the device.
 * Prints to stderr if the device could not be connected.<br>
 * The connected variable is lowered until a succesfull connect
 * On success, the connected variable is set to USBDEV_RETRY 
 * @param usbdevice The device to connect.
 * @returns The new value of connected (so you should try to connect again
if it's not 0).
 */
int Usbdevice_connect(usbdevice_t* usbdevice){
    if (usbOpenDevice(
        &(usbdevice->handle),
        usbdevice->vid, usbdevice->vendor,
        usbdevice->pid, usbdevice->product,
        NULL, NULL, NULL) != 0){

        char s[256];
                
        sprintf(
            s,
            "USB device \"%s\" with vid=0x%x pid=0x%x\n  NOT FOUND",
            usbdevice->product, usbdevice->vid, usbdevice->pid);
        Report_err(s);
        usbdevice->connected = 0;
    }else usbdevice->connected = USBDEV_RETRY;
    return usbdevice->connected;
}



/*================================ SEND CONTROL MESSAGE =====================*/
/** Send a control message to a device.
 * @param usbdevice The device to communicate with.
 * @param request The request to send, see requests.h.
 * @param reqType One of usbdevice_reqType.
 * @param wval Special value to send (1 byte).
 * @param wind Special value to send (1 byte).
 * @param buffer A buffer with room for BUFLEN_SERVO_DATA bytes,
 see i2c_header.h.
 * @return Number of bytes written or read.
 
 */
int _Usbdevice_sendCtrlMsg(
    usbdevice_t* usbdevice, int request, usbdevice_reqType reqType,
    int wval, int wind, char* buffer)
{
    int cnt = 0;
    int i;
    //only send if we're connected
    if(usbdevice->handle && usbdevice->connected){
        cnt = usb_control_msg(
            usbdevice->handle,
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | reqType,
            request, wval, wind, buffer,
            BUFLEN_SERVO_DATA,
            USBDEV_TIMEOUT_MS
        );
        if(cnt < 0){
            char s[256];
            sprintf("usb_control_msg: %s", usb_strerror());
            Report_err(s);
        }else{
            //success full msg
            usbdevice->connected = USBDEV_RETRY;
        }
        
    }else{
        //we're not connected
        usbdevice->connected--;
    }
    usleep(1);
    return(cnt);
}


/*=========================READ GENERAL DATA ================================*/
int Usbdevice_getData(usbdevice_t* usbdevice, char* buffer){
    int cnt = _Usbdevice_sendCtrlMsg(usbdevice, CUSTOM_RQ_GET_DATA, 
        USBDEV_READ, 0, 0, buffer);
    if(cnt > 0){
        //store correct values in pscontroller 
		Pscontroller_updateData(&usbdevice->pscon, 
			buffer[1], //ss_dpad
			buffer[2], //shoulder_shapes
			buffer[5], buffer[6], buffer[7], buffer[8] //axis
		);
		//and adc

    }
	return(cnt);
}
