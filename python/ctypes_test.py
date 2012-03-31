#! /usr/bin/env python
#"Now go away, or I shall taunt you a second time!"

#string* need to be passed like "plop".encode('ascii')


from ctypes import *
import sys

if sys.platform == 'linux2':
    LIBCROBOT = "lib/libcrobot.so.1.0.1"
elif sys.platform == 'win32':
    LIBCROBOT = "lib/libcrobot.dll"

crobot = CDLL(LIBCROBOT)
crobot.Usbdevice_alloc.restype = c_void_p

buffer = (c_byte *12)()
buffer_p = cast(buffer, POINTER(c_byte))



#-----------------------------------------------------------------------------
if __name__ == "__main__":
    crobot.Report_std(("STARTUP".encode("ascii")))
    myDev = crobot.Usbdevice_alloc()
    if crobot.Usbdevice_connect(myDev):
            print("we're connected!")
            print('received ' + str(crobot.Usbdevice_getData(myDev, buffer_p)) + ' bytes')
            for item in buffer:
                    print(item)
    crobot.Usbdevice_free(myDev)
