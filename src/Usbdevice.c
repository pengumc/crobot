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
    //pscon doesn't need an alloc
    tempDev->acc = Accelerometer_alloc();
    //alloc mem for strings
    tempDev->vendor = calloc(USB_CFG_VENDOR_NAME_LEN+1, sizeof(char));
    tempDev->product = calloc(USB_CFG_DEVICE_NAME_LEN+1, sizeof(char));
    //alloc legs
    unsigned char i;
    for(i=0;i<USBDEV_LEGNO;i++){ tempDev->legs[i] = Leg_alloc();}

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
    Accelerometer_free(usbdevice->acc);
    unsigned char i;
    for(i=0;i<USBDEV_LEGNO;i++){ Leg_free(usbdevice->legs[i]);}
    free(usbdevice);
}


/*============================== INIT =======================================*/
/** Set default values for a usbdevice.
 * Values are defined in i2c_header.h file as well as usbconfig.h
 * @param usbdevice The usbdevice data to use.
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
    usbdevice->connected = 0;

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
            "USB device \"%s\" with vid=0x%x pid=0x%x\n NOT FOUND",
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
    if(usbdevice->handle && usbdevice->connected > 0){
        cnt = usb_control_msg(
            usbdevice->handle,
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | reqType,
            request, wval, wind, buffer,
            BUFLEN_SERVO_DATA,
            USBDEV_TIMEOUT_MS
        );
        if(cnt < 0){
            //horrible error occured, disconnect, abandon all hope
            printf("usb_control_msg: %s\n", usb_strerror());
            usbdevice->connected = 0;
        }else{
            //success full msg
            usbdevice->connected = USBDEV_RETRY;
        }
        
    }else{
        //we're not connected
        usbdevice->connected--;
        return(0);
    }
    //DEBUG
    //printf("usb send: msg = %d, cnt = %d\n", request, cnt);
    //printBuffer(buffer);
    return(cnt);
}


/*=========================READ GENERAL DATA ================================*/
/** Get general data from usbdevice and update everything that uses that data.
 * @param usbdevice The usbdevice data to use.
 * @param buffer The buffer to use. size should be BUFLEN_SERVO_DATA * byte.
 * @return The number of bytes read.
 */
int Usbdevice_getData(usbdevice_t* usbdevice, char* buffer){
    int cnt = _Usbdevice_sendCtrlMsg(usbdevice, CUSTOM_RQ_GET_DATA, 
        USBDEV_READ, 0, 0, buffer);
    if(cnt > 0){
        //store correct values in pscontroller
        //The order of returned values is highly illogical captain...
		Pscontroller_updateData(&usbdevice->pscon, 
			buffer[1], //ss_dpad
			buffer[2], //shoulder_shapes
			buffer[5], buffer[6], buffer[7], buffer[8] //axis
		);
		//and adc
        Accelerometer_updateValues(usbdevice->acc, buffer[3], buffer[4], buffer[0]);
    }
	return(cnt);
}

/*============================ UPDATE SERVOS ================================*/
//called by getservodata
void _Usbdevice_updateServos(usbdevice_t* dev, char* buffer){
    unsigned char leg,servo,i=0;
    for(leg=0;leg<USBDEV_LEGNO;leg++){
        for(servo=0;servo<LEG_DOF;servo++){
            Leg_setServoPw(dev->legs[leg], servo, buffer[i]);
            i++;
        }
        Leg_updateServoLocations(dev->legs[leg]);
    }
}

/*========================= GET SERVO DATA ==================================*/
/** retrieve current servo settings from the usbdevice and update the pc side
servo data.
 * @param usbdevice The device to communicate with.
 * @param buffer A buffer with room for BUFLEN_SERVO_DATA bytes.
 * @return The number of bytes read.
 * @retval -1 Device wasn't ready.
 * @retval -2 Something went horribly wrong (device disconnected?).
 */
int Usbdevice_getServoData(usbdevice_t* usbdevice, char* buffer){
    if(usbdevice->connected <= 0) return(-2);
    int cnt;
    //tell device to get servodata from servocontroller
    cnt = _Usbdevice_sendCtrlMsg(usbdevice, CUSTOM_RQ_LOAD_POS_FROM_I2C,
        USBDEV_READ, 0, 0, buffer);
    if(cnt>0){
        cnt = -1;
        if(buffer[1] == CUSTOM_RQ_LOAD_POS_FROM_I2C){
            printf("received request echo, retrying\n");
		    //nanosleep(&sleepy_time, NULL);
			nsleep(10000000);
            //TODO prevent stack overflow...
            cnt = Usbdevice_getServoData(usbdevice, buffer);
        }
        //device was not ready
        return(cnt);
    }
    
    signed char trying = USBDEV_RETRY;
    while(trying >= 0){
		nsleep(10000000);
        cnt = _Usbdevice_sendCtrlMsg(usbdevice, CUSTOM_RQ_GET_POS,
            USBDEV_READ, 0, 0, buffer);
        //should have received 12 bytes, even on failure
        if(cnt < 1) {
            Report_err("received nothing from GET_POS request.");
            return(-2);
        }else if(cnt == BUFLEN_SERVO_DATA){
            if(buffer[0] == CUSTOM_RQ_GET_POS){
                printf("busy reading...\n");
                //indication the device is still busy
            }else{
                trying = -1;
            }
        }
    }

    //update the servos
    _Usbdevice_updateServos(usbdevice, buffer);
    return(cnt);
}


/*=================== SEND SERVO DATA =======================================*/
/** Send current servo angles to device.
 * @param usbdevice The device to communicate with.
 * @retval 0 Success.
 * @retval -1 Failure (device disconnected?).
 */
int Usbdevice_sendServoData(usbdevice_t* usbdevice){
    uint8_t buffer[USBDEV_LEGNO * LEG_DOF];
    unsigned char l, s, i=0;
    //fill buffer
    for(l=0;l<USBDEV_LEGNO;l++){
        for(s=0;s<LEG_DOF;s++){
            buffer[i] = usbdevice->legs[l]->servos[s]->_pw;
            i++;
        }
    }
    int cnt = _Usbdevice_sendCtrlMsg(usbdevice, CUSTOM_RQ_SET_DATA,
        USBDEV_WRITE, 0, 0, buffer);
    if(cnt != BUFLEN_SERVO_DATA){
        //usb ctrl msg should return number of bytes written in this case
        return(-1);
    }
    //DEBUG
    printf("servo data send: %d\n", cnt);
    printBuffer(buffer);
    return(cnt);
}

void printBuffer(char* buffer){
    printf("buffer = [");
    char i;
    for(i=0;i<BUFLEN_SERVO_DATA;i++){
        printf("%d, ", buffer[i]);
    }
    printf("]\n");
}

void nsleep(long nanoseconds){
	#ifndef __WINDOWSCRAP__
		struct timespec sleepy_time;
        sleepy_time.tv_sec = 0;
		sleepy_time.tv_nsec = nanoseconds;//10000000; //1e6 ns = 1 ms
	#else
		Sleep(nanoseconds/1000000);
	#endif
}
