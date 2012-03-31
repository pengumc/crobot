#! /usr/bin/env python2.7
#"Now go away, or I shall taunt you a second time!"

#string* need to be passed like "plop".encode('ascii')


from ctypes import *
import signal
import sys
import pygtk
#pygtk.require(2.0)
import gtk, gobject, cairo

def handleSigTERM():
    #gtk.main_quit()
    print("quitting...")
    sys.exit(0)

class Screen:
    
    def __init__(self, crobot):
        self.window = gtk.Window();
        self.window.connect('delete-event', gtk.main_quit)
        self.window.set_size_request(300,300)
        self.da = gtk.DrawingArea()
        self.window.add(self.da)
        self.window.show_all()
        self.crobot = crobot

    def start(self):
        gtk.timeout_add(1000, self.timeout)
        gtk.main()

    def timeout(self):
        self.crobot.Quadruped_update(self.crobot.)
        return(True)
    

class Crobot:
    
    def __init__(self):
        if sys.platform == 'linux2':
            LIBCROBOT = "lib/libcrobot.so.1.0.1"
        elif sys.platform == 'win32':
            LIBCROBOT = "lib/libcrobot.dll"
        self.crobotlib = CDLL(LIBCROBOT)
        self.crobotlib.Quadruped_alloc.restype = c_void_p
        self.qped = self.crobotlib.Quadruped_alloc()
        #self.buf = (c_byte *12)()
        #self.buf_p = cast(self.buf, POINTER(c_byte))
        #self.crobot.Usbdevice_alloc.restype = c_void_p
        #self.dev = self.crobot.Usbdevice_alloc()
        #self.crobot.Usbdevice_connect(self.dev)

    def print_buf(self):
        print('buf{' + 
            str(self.buf[0]) + ', ' +
            str(self.buf[1]) + ', ' +
            str(self.buf[2]) + ', ' +
            str(self.buf[3]) + ', ' +
            str(self.buf[4]) + ', ' +
            str(self.buf[5]) + ', ' +
            str(self.buf[6]) + ', ' +
            str(self.buf[7]) + ', ' +
            str(self.buf[8]) + ', ' +
            str(self.buf[9]) + ', ' +
            str(self.buf[10]) + ', ' +
            str(self.buf[11]) + '}')




#-------------------------------start -----------------------------------------
if __name__ == "__main__":
    signal.signal(signal.SIGTERM, handleSigTERM) 
    signal.signal(signal.SIGINT, handleSigTERM) 
    try:
        screen = Screen(Crobot())
        screen.start()
    except KeyboardInterrupt:
        handleSigTERM()
