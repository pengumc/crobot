#! /usr/bin/env python
#"Now go away, or I shall taunt you a second time!"

#string* need to be passed like "plop".encode('ascii')

LIBCROBOT = "lib/libcrobot.so.1.0.1"

from ctypes import *
crobot = CDLL(LIBCROBOT)
crobot.Usbdevice_alloc.restype = c_void_p






#-----------------------------------------------------------------------------
if __name__ == "__main__":
    crobot.Report_std(("STARTUP".encode("ascii")))
    myDev = crobot.Usbdevice_alloc()
    crobot.Usbdevice_connect(myDev)
    crobot.Usbdevice_free(myDev)
