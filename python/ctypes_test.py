#!/usr/bin/env python2
#"Now go away, or I shall taunt you a second time!"

#string* need to be passed like "plop".encode('ascii')


from ctypes import *
import signal
import sys, os, math
import platform
import pygtk
#pygtk.require(2.0)
import gtk, gobject, cairo

import buttonbar
import grapharea
import qpimage

def handleSigTERM():
    #gtk.main_quit()
    print("quitting...")
    sys.exit(0)


#main screen
#==============
class Screen:

    NUMBER_OF_BUTTONS = 6
    TIMEOUT = 40
    TIMEOUT_GRAPH = 500
    
    def __init__(self, crobot):
        self.timeout_active = False

        self.window = gtk.Window();
        self.window.connect('delete-event', gtk.main_quit)
        self.window.connect('key_press_event', self.do_keypress);

        self.maintable = gtk.Table(2, 3)
        #qpimage
        self.qpimage = qpimage.QpImage()
        self.maintable.attach(self.qpimage, 0,1, 0,1, gtk.FILL,gtk.FILL|gtk.EXPAND) 
        self.qpimage.set_size_request(430, 150)
        #buttonlist
        self.buttontable = gtk.Table(1,1)
        self.maintable.attach(
            self.buttontable, 1,2, 0,1, gtk.FILL|gtk.EXPAND, gtk.FILL|gtk.EXPAND)
        self.connect_button = gtk.Button("connect")
        self.connect_button.connect("clicked", self.connect_click)
        self.buttontable.attach(self.connect_button, 0,1, 0,1, 0,0)
        #grapharea 
        self.graph = grapharea.GraphArea()
        self.graph.set_size_request(100,100)
        self.graph.setBgColor(1,1,1)
        self.maintable.attach(self.graph, 0,2, 1,2, gtk.FILL|gtk.EXPAND,gtk.FILL|gtk.EXPAND)
        #buttonbar
        self.buttonbar = buttonbar.ButtonBar()
        self.maintable.attach(self.buttonbar, 0,2, 2,3, 0,0)

        #launch
        self.window.add(self.maintable)
        self.window.show_all()
        self.graph.do_expose_event(None) #needed to start drawing
        
        self.crobot = crobot
        self.graph.setData([self.crobot.inX, self.crobot.outX])
        self.graph.lines[0].setColor(1,0,0)
        self.graph.lines[0].setColor(1,0,1)
        self.graph.set_maxy(100)

    def connect_to_device(self):
        pass
            
    
    def _attach_analog(self, table):
        table.attach(self.right_analog, 1,3, 1,3, 0,0)        
        table.attach(self.left_analog, 3,5, 1,3, 0,0)        

    def start(self):
        self.connect_click(None)
        gtk.timeout_add(Screen.TIMEOUT_GRAPH, self.graph_timeout)
        gtk.main()

    def connect_click(self, event):
        con = self.crobot.connect()
        if con:
            if self.timeout_active == False:
                print("(re)connected")
                self.update_servoinfo()
                self.timeout_active = True
                gtk.timeout_add(Screen.TIMEOUT, self.timeout)

    def update_servoinfo(self):
        info = self.crobot.getServoinfo()
        for i in range(Crobot.SERVOCOUNT):
            self.qpimage.blocks[i].pw = info.pulsewidths[i]
        self.qpimage.do_expose_event()


    def timeout(self, event=None):
        if self.crobot.update() < 1:
            print('disconnected timeout...')
            self.timeout_active = False
            return(False)
        self.update_buttons()
        self.update_sticks()
        self.update_graph()
        return(True)

    def update_buttons(self):
        for name, button in self.buttonbar.buttons.iteritems():
            button.set(self.crobot.getButtonEdge(button.buttonNo))
            
    def update_sticks(self):
        self.buttonbar.sticks['right'].update(
            self.crobot.getStick(0), self.crobot.getStick(1))
        self.buttonbar.sticks['left'].update(
            self.crobot.getStick(2), self.crobot.getStick(3))

    def update_graph(self):
        self.graph.index += 1
        if self.graph.index >= 300:
            self.graph.index = 0


    def graph_timeout(self):
        self.graph.do_expose_event(None)
        return(True)

    def do_keypress(self, widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval).lower()
        handled = False
        #print(keyname)
        if keyname == 'space':
            handled = True
        elif keyname == 'escape':
            gtk.main_quit()
        elif (keyname == 'plus' or keyname == 'equal' or keyname == 'kp_add'):
            self.change_first_selected_servo(0.1)
        elif (keyname == 'minus' or keyname == 'kp_subtract'):
            self.change_first_selected_servo(-0.1)
        else:
            print("unhandled: " + str(keyname) + " - " + str(event.keyval))

        return(handled)

    def change_first_selected_servo(self, amount):
        if not self.crobot.con: return
        selection = self.qpimage.get_selected()
        if len(selection):
            #print("selection: " + str(selection[0]))
            if selection[0] == qpimage.QpImage.SERVOCOUNT:
                print("passing")
            else:
                l = int(selection[0]) / qpimage.QpImage.LEGSIZE
                s = int(selection[0]) % qpimage.QpImage.LEGSIZE
                result = self.crobot.changeServo(l, s, amount)
                print("move result: " + str(result))
                if result != 1:
                    self.qpimage.blink(selection[0])
                else:
                    self.crobot.commit()
                    self.update_servoinfo()


#crobot library handler
#======================
class Crobot:
        
    SERVOCOUNT = 12

    def __init__(self):
        bits = platform.machine()
        if bits == 'i686' or bits == 'x86':
            bits = '64'
        elif bits == 'x86_64' or bits == 'AMD64':
            bits = '64'
       
        if sys.platform == 'linux2':
            LIBCROBOT = "lib/libcrobot" + bits + ".so.1.0.1"
        elif sys.platform == 'win32':
            LIBCROBOT = "lib/libcrobot" + bits + ".dll"
        print("lib: " + LIBCROBOT)
        #create qped instance
        self.crobotlib = CDLL(LIBCROBOT)
        self.crobotlib.Quadruped_alloc.restype = c_void_p
        self.qped = self.crobotlib.Quadruped_alloc()
        #servoinfo
        self.servoinfop = self.crobotlib.Quadruped_getServoinfoPointer(self.qped)
        self.servoinfo = SERVOINFO.from_address(self.servoinfop)
        #set datasets for filters
        self.inX = (c_double *300)()
        self.p_inX = cast(self.inX, POINTER(c_double))
        self.inY = (c_double *300)()
        self.p_inY = cast(self.inY, POINTER(c_double))
        self.inZ = (c_double *300)()
        self.p_inZ = cast(self.inZ, POINTER(c_double))
        self.outX = (c_double *300)()
        self.p_outX = cast(self.outX, POINTER(c_double))
        self.outY = (c_double *300)()
        self.p_outY = cast(self.outY, POINTER(c_double))
        self.outZ = (c_double *300)()
        self.p_outZ = cast(self.outZ, POINTER(c_double))
        self.con =  self.crobotlib.Quadruped_startup(self.qped)
        print('startup: ' + str(self.con))
        if self.con:
            print("grabbing first servoinfo...")
            self.refreshServoinfo()
        print("setting new graph buffers...")
        self.crobotlib.Quadruped_setGraphPointers(self.qped,
            self.inX,
            self.p_inY,
            self.p_inZ,
            self.p_outX,
            self.p_outY,
            self.p_outZ)

    def connect(self):
        self.con = self.crobotlib.Quadruped_startup(self.qped)
        self.refreshServoinfo()
        return(self.con)

    def __del__(self):
        self.crobotlib.Quadruped_free(self.qped)
    
    def getButton(self, button):
        t = self.crobotlib.Quadruped_getPsButton(self.qped, button)
        #print('button ' + str(button) + ': ' + str(t))
        return(t)

    def getButtonEdge(self, button):
        t = self.crobotlib.Quadruped_getPsButtonEdge(self.qped, button)
        return(t)

    def getStick(self, axis):
        return(self.crobotlib.Quadruped_getPsAxis(self.qped, axis))
    
    def update(self):
        return(self.crobotlib.Quadruped_update(self.qped))

    def changeServo(self, leg, s, value):
        return(self.crobotlib.Quadruped_changeSingleServo(self.qped, leg, s, c_double(value)))

    def getServoinfo(self):
        self.crobotlib.Quadruped_updateServoinfo(self.qped)
        return(self.servoinfo)

    def refreshServoinfo(self):
        self.crobotlib.Quadruped_getServoData(self.qped)
        return(self.getServoinfo())

    def commit(self):
        return(self.crobotlib.Quadruped_commit(self.qped))

class SERVOINFO(Structure):
    _fields_ = [("pulsewidths", c_byte*12),
                ("angles", c_double*12)]


#-------------------------------start -----------------------------------------
if __name__ == "__main__":
    signal.signal(signal.SIGTERM, handleSigTERM) 
    signal.signal(signal.SIGINT, handleSigTERM) 
    try:
        screen = Screen(Crobot())
        screen.start()
    except KeyboardInterrupt:
        handleSigTERM()
