#!/usr/bin/env python2
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

class Psbutton(gtk.Image):
    
    def __init__(self, pixbuf1, pixbuf2):
        gtk.Image.__init__(self)
        self.pb_inactive = pixbuf1
        self.pb_active = pixbuf2
        self.active = False
        self.set_from_pixbuf(self.pb_inactive)

    def activate(self):
        if not self.active:
            self.set_from_pixbuf(self.pb_active)
            self.active = True

    def deactivate(self):
        if self.active:
            self.set_from_pixbuf(self.pb_inactive)
            self.active = False

    def set(self, state):
        if state > 0:
            self.deactivate()
        elif state < 0:
            self.activate()


class Screen:

    NUMBER_OF_BUTTONS = 6
    TIMEOUT = 40
    
    def __init__(self, crobot):
        self.timeout_active = False

        self.window = gtk.Window();
        self.window.connect('delete-event', gtk.main_quit)
        self.window.set_size_request(6*48,300)

        self.maintable = gtk.Table(2, 2)

        #controller
        self.pstable = gtk.Table(Screen.NUMBER_OF_BUTTONS, 1)
        self._generate_pixbufs()
        self._gen_buttons()
        self._attach_buttons(self.pstable) 

        #big draw area
        self.bigda = gtk.DrawingArea()
        
        #attach to main table
        self.maintable.attach(self.pstable, 0,1, 1,2, 0,0)
        self.maintable.attach(self.bigda, 0,1, 0,1, gtk.EXPAND|gtk.FILL,0)

        self.window.add(self.maintable)
        self.window.show_all()
        
        self.crobot = crobot

    def connect_to_device(self):
        pass
        #if not self.timeout_active:
            #if self.crobot.connect():
                #self.timeout_active = True;
                #print('connected...')
                #gtk.timeout_add(Screen.TIMEOUT, self.timeout)
            
    def _gen_buttons(self):
        self.psbuttons = {
            'cross' : Psbutton(self.pixbufs['cross'], self.pixbufs['across']),
            'circle' : Psbutton(self.pixbufs['circle'], self.pixbufs['acircle']),
            'triangle' : Psbutton(self.pixbufs['triangle'], self.pixbufs['atriangle']),
            'square' : Psbutton(self.pixbufs['square'], self.pixbufs['asquare']),
            'l1' : Psbutton(self.pixbufs['l1'], self.pixbufs['al1']),
            'r1' : Psbutton(self.pixbufs['r1'], self.pixbufs['ar1']),
            'l2' : Psbutton(self.pixbufs['l2'], self.pixbufs['al2']),
            'r2' : Psbutton(self.pixbufs['r2'], self.pixbufs['ar2']),
            'select' : Psbutton(self.pixbufs['select'], self.pixbufs['aselect']),
            'start' : Psbutton(self.pixbufs['start'], self.pixbufs['astart'])
        }
    
    def _attach_buttons(self, table):
        table.attach(self.psbuttons['l1'], 0,1, 0,1, 0,0)
        table.attach(self.psbuttons['cross'], 1,2, 0,1, 0,0)
        table.attach(self.psbuttons['square'], 2,3, 0,1, 0,0)
        table.attach(self.psbuttons['circle'], 3,4, 0,1, 0,0)
        table.attach(self.psbuttons['triangle'], 4,5, 0,1, 0,0)
        table.attach(self.psbuttons['r1'], 5,6, 0,1, 0,0)
        table.attach(self.psbuttons['l2'], 0,1, 1,2, 0,0)
        table.attach(self.psbuttons['r2'], 5,6, 1,2, 0,0)
        table.attach(self.psbuttons['select'], 0,1, 2,3, 0,0)
        table.attach(self.psbuttons['start'], 5,6, 2,3, 0,0)
        

    def _generate_pixbufs(self):
        mainbuf = gtk.gdk.pixbuf_new_from_file(
            os.path.dirname(__file__) + os.sep + 'psbuttons.png')
        self.pixbufs = dict()
        self.pixbufs['cross'] = mainbuf.subpixbuf(0,0,48,48)
        self.pixbufs['across'] = mainbuf.subpixbuf(48,0,48,48)
        self.pixbufs['circle'] = mainbuf.subpixbuf(48*2,0,48,48)
        self.pixbufs['acircle'] = mainbuf.subpixbuf(48*3,0,48,48)
        self.pixbufs['square'] = mainbuf.subpixbuf(48*4,0,48,48)
        self.pixbufs['asquare'] = mainbuf.subpixbuf(48*5,0,48,48)
        self.pixbufs['triangle'] = mainbuf.subpixbuf(48*6,0,48,48)
        self.pixbufs['atriangle'] = mainbuf.subpixbuf(48*7,0,48,48)

        self.pixbufs['select'] = mainbuf.subpixbuf(0, 48, 48, 48)
        self.pixbufs['aselect'] = mainbuf.subpixbuf(48*1, 48, 48, 48)
        self.pixbufs['start'] = mainbuf.subpixbuf(48*2, 48, 48, 48)
        self.pixbufs['astart'] = mainbuf.subpixbuf(48*3, 48, 48, 48)
        self.pixbufs['l1'] = mainbuf.subpixbuf(48*4, 48, 48, 48)
        self.pixbufs['al1'] = mainbuf.subpixbuf(48*5, 48, 48, 48)
        self.pixbufs['r1'] = mainbuf.subpixbuf(48*6, 48, 48, 48)
        self.pixbufs['ar1'] = mainbuf.subpixbuf(48*7, 48, 48, 48)

        self.pixbufs['l2'] = mainbuf.subpixbuf(48*0, 48*2, 48, 48)
        self.pixbufs['al2'] = mainbuf.subpixbuf(48*1, 48*2, 48, 48)
        self.pixbufs['r2'] = mainbuf.subpixbuf(48*2, 48*2, 48, 48)
        self.pixbufs['ar2'] = mainbuf.subpixbuf(48*3, 48*2, 48, 48)


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
        self.psbuttons['triangle'].set( self.crobot.getButtonEdge(12))
        self.psbuttons['circle'].set(self.crobot.getButtonEdge(13))
        self.psbuttons['cross'].set(self.crobot.getButtonEdge(14))
        self.psbuttons['square'].set(self.crobot.getButtonEdge(15))
        self.psbuttons['select'].set(self.crobot.getButtonEdge(0))
        self.psbuttons['start'].set(self.crobot.getButtonEdge(3))
        self.psbuttons['l1'].set(self.crobot.getButtonEdge(10))
        self.psbuttons['l2'].set(self.crobot.getButtonEdge(8))
        self.psbuttons['r1'].set(self.crobot.getButtonEdge(11))
        self.psbuttons['r2'].set(self.crobot.getButtonEdge(9))

    

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
