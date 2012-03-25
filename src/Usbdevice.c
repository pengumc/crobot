/**
 * @file Usbdevice.c
 */

#include "Usbdevice.h"


/*==================================== ALLOC ================================*/
/** Allocate memory for a usbdevice.
 * Also set the default values. No connection is attempted.
 * @return A pointer to a new device.
 */
usbdevice_t* usbdevice_alloc(){
    usbdevice_t* tempDev = calloc(1, sizeof(usbdevice_t));
    //alloc mem for strings
    tempDev->vendor = calloc(USB_CFG_VENDOR_NAME_LEN+1, sizeof(char));
    tempDev->product = calloc(USB_CFG_DEVICE_NAME_LEN+1, sizeof(char));
    usbdevice_init(tempDev);
    usb_init();
    return(tempDev);
}


/*================================== FREE ===================================*/
/** Delete a usbdevice from memory.
 * @param usbdevice The device to delete
 */
void usbdevice_free(usbdevice_t* usbdevice){
    if(usbdevice->handle) usb_close(usbdevice->handle);
    free(usbdevice->vendor);
    free(usbdevice->product);
    free(usbdevice);
}


/*============================== INIT =======================================*/
/** Set default values for a usbdevice.
 * Values are defined in i2c_header.h file as well as usbconfig.h
 */
void usbdevice_init(usbdevice_t* usbdevice){
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
int usbdevice_connect(usbdevice_t* usbdevice){
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
        report_err(s);
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
int _usbdevice_sendCtrlMsg(
    usbdevice_t* usbdevice, int request, usbdevice_reqType reqType,
    int wval, int wind, char* buffer)
{
    int cnt, i;
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
            report_err(s);
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


/*=========================READ SERVO DATA ==================================*/
