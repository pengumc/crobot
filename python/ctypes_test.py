#! /usr/bin/env python2.7
#"Now go away, or I shall taunt you a second time!"

#string* need to be passed like "plop".encode('ascii')


from ctypes import *
import signal
import sys, os
import pygtk
#pygtk.require(2.0)
import gtk, gobject, cairo

def handleSigTERM():
    #gtk.main_quit()
    print("quitting...")
    sys.exit(0)

class Screen:

    NUMBER_OF_BUTTONS = 4
    TIMEOUT = 40
    
    def __init__(self, crobot):
        self.timeout_active = False

        self.window = gtk.Window();
        self.window.connect('delete-event', gtk.main_quit)
        self.window.set_size_request(300,300)
        self.pstable = gtk.Table(Screen.NUMBER_OF_BUTTONS, 1)
        self._generate_pixbufs()
        self._attach_buttons(self.pstable) 

        self.window.add(self.pstable)
        self.window.show_all()
        
        self.crobot = crobot

    def connect_to_device(self):
        pass
        #if not self.timeout_active:
            #if self.crobot.connect():
                #self.timeout_active = True;
                #print('connected...')
                #gtk.timeout_add(Screen.TIMEOUT, self.timeout)
            

    def _attach_buttons(self, table):
        self.images = []
        for i in range(Screen.NUMBER_OF_BUTTONS):
            self.images.append(gtk.Image())
            table.attach(self.images[i], i, i+1, 0, 1, 0, 0)
        self.images[0].set_from_pixbuf(self.button_bufs['circle'])
        self.images[1].set_from_pixbuf(self.button_bufs['square'])
        self.images[2].set_from_pixbuf(self.button_bufs['cross'])
        self.images[3].set_from_pixbuf(self.button_bufs['triangle'])

    def _generate_pixbufs(self):
        mainbuf = gtk.gdk.pixbuf_new_from_file(
            os.path.dirname(__file__) + os.sep + 'psbuttons.png')
        self.button_bufs = dict()
        self.button_bufs['cross'] = mainbuf.subpixbuf(0,0,48,48)
        self.button_bufs['across'] = mainbuf.subpixbuf(48,0,48,48)
        self.button_bufs['circle'] = mainbuf.subpixbuf(48*2,0,48,48)
        self.button_bufs['acircle'] = mainbuf.subpixbuf(48*3,0,48,48)
        self.button_bufs['square'] = mainbuf.subpixbuf(48*4,0,48,48)
        self.button_bufs['asquare'] = mainbuf.subpixbuf(48*5,0,48,48)
        self.button_bufs['triangle'] = mainbuf.subpixbuf(48*6,0,48,48)
        self.button_bufs['atriangle'] = mainbuf.subpixbuf(48*7,0,48,48)
        self.button_bufs['select'] = mainbuf.subpixbuf(0, 48, 48, 48)
        self.button_bufs['aselect'] = mainbuf.subpixbuf(48*1, 48, 48, 48)

    def start(self):
        self.timeout_active = True
        gtk.timeout_add(Screen.TIMEOUT, self.timeout)
        gtk.main()

    def timeout(self):
        if self.crobot.update() < 1:
            print('disconnected...')
            self.timeout_active = False
            return(False)
        self.update_buttons()
        return(True)

    def update_buttons(self):
        circle = self.crobot.getButtonEdge(13)
        if circle == -1:
            self.images[0].set_from_pixbuf(self.button_bufs['acircle'])
        elif circle == 1:
            self.images[0].set_from_pixbuf(self.button_bufs['circle'])

        square = self.crobot.getButtonEdge(15)
        if square == -1:
            self.images[1].set_from_pixbuf(self.button_bufs['asquare'])
        elif square == 1:
            self.images[1].set_from_pixbuf(self.button_bufs['square'])
        
        cross = self.crobot.getButtonEdge(14)
        if cross == -1:
            self.images[2].set_from_pixbuf(self.button_bufs['across'])
        elif cross == 1:
            self.images[2].set_from_pixbuf(self.button_bufs['cross'])
        
        triangle = self.crobot.getButtonEdge(12)
        if triangle == -1:
            self.images[3].set_from_pixbuf(self.button_bufs['atriangle'])
        elif triangle == 1:
            self.images[3].set_from_pixbuf(self.button_bufs['triangle'])

    

class Crobot:
    
    def __init__(self):
        if sys.platform == 'linux2':
            LIBCROBOT = "lib/libcrobot.so.1.0.1"
        elif sys.platform == 'win32':
            LIBCROBOT = "lib/libcrobot.dll"
        self.crobotlib = CDLL(LIBCROBOT)
        self.crobotlib.Quadruped_alloc.restype = c_void_p
        self.qped = self.crobotlib.Quadruped_alloc()
        self.con =  self.crobotlib.Quadruped_startup(self.qped)
        print('startup: ' + str(self.con))
            
    def getButton(self, button):
        t = self.crobotlib.Quadruped_getPsButton(self.qped, button)
        #print('button ' + str(button) + ': ' + str(t))
        return(t)

    def getButtonEdge(self, button):
        t = self.crobotlib.Quadruped_getPsButtonEdge(self.qped, button)
        return(t)
    
    def update(self):
        return(self.crobotlib.Quadruped_update(self.qped))





#-------------------------------start -----------------------------------------
if __name__ == "__main__":
    signal.signal(signal.SIGTERM, handleSigTERM) 
    signal.signal(signal.SIGINT, handleSigTERM) 
    try:
        screen = Screen(Crobot())
        screen.start()
    except KeyboardInterrupt:
        handleSigTERM()
