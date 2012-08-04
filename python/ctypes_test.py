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
import drawrobot #import qpimage
import configuration

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
    #--------------------------------------------------------------------------    
    def __init__(self, crobot):
        self.timeout_active = False
        self.window = gtk.Window();
        self.window.connect('delete-event', gtk.main_quit)
        self.window.connect('key_press_event', self.do_keypress);
        self.window.add_events(gtk.gdk.SCROLL_MASK)
        self.window.connect("scroll-event", self.do_scroll)
        self.maintable = gtk.Table(2, 3)
        #drawrobot
        self.robotdrawing = drawrobot.RobotMainViewArea()
        self.maintable.attach(self.robotdrawing, 0,1, 0,1, gtk.FILL,gtk.FILL|gtk.EXPAND) 
        self.robotdrawing.set_size_request(500,500)
        #buttonlist
        self.buttontable = gtk.Table(1,2)
        self.maintable.attach(
            self.buttontable, 1,2, 0,1, gtk.FILL|gtk.EXPAND, gtk.FILL|gtk.EXPAND)
        self.connect_button = gtk.Button("connect")
        self.connect_button.connect("clicked", self.connect_click)
        self.buttontable.attach(self.connect_button, 0,1, 0,1,gtk.FILL,gtk.FILL)
        self.debug_button = gtk.Button("debug info")
        self.debug_button.connect("clicked", self.debug_click)
        self.buttontable.attach(self.debug_button, 0,1, 1,2, gtk.FILL,gtk.FILL)
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
        #crobot lib
        self.crobot = crobot
        self.graph.setData([self.crobot.inX, self.crobot.outX])
        self.graph.lines[0].setColor(1,0,0)
        self.graph.lines[0].setColor(1,0,1)
        self.graph.set_maxy(100)
        self.configure()
    #--------------------------------------------------------------------------
    def msgbox(self, text):
        box = gtk.MessageDialog(self.window,
            gtk.DIALOG_DESTROY_WITH_PARENT, gtk.MESSAGE_INFO,
            gtk.BUTTONS_OK, text)
        box.run()
        box.destroy()
    #--------------------------------------------------------------------------
    def connect_to_device(self):
        con = self.crobot.connect()
        if con:
            if self.timeout_active == False:
                print("(re)connected")
                self.connect_button.set_label('connect (1)')
                self.update_servoinfo()
                self.timeout_active = True
                gtk.timeout_add(Screen.TIMEOUT, self.timeout)
        else:
            self.connect_button.set_label('connect (0)')
    #--------------------------------------------------------------------------            
    def _attach_analog(self, table):
        table.attach(self.right_analog, 1,3, 1,3, 0,0)        
        table.attach(self.left_analog, 3,5, 1,3, 0,0)        
    #--------------------------------------------------------------------------
    def start(self):
        self.connect_click(None)
        gtk.timeout_add(Screen.TIMEOUT_GRAPH, self.graph_timeout)
        gtk.main()
    #--------------------------------------------------------------------------
    def connect_click(self, event):
        self.connect_to_device()
    #--------------------------------------------------------------------------
    def debug_click(self, event):
        self.crobot.printLegs();
        for i in range(4):
            for j in range(3):
                print('servo {},{}'.format(i,j))
                self.crobot.printServoDetails(i, j)

    #--------------------------------------------------------------------------
    def update_servoinfo(self):
        info = self.crobot.getServoinfo()
        for i in range(Crobot.SERVOCOUNT):
            self.robotdrawing.servoboxes[i].data[0] = info.pulsewidths[i]
            self.robotdrawing.servoboxes[i].data[1] = info.angles[i]
        self.robotdrawing.redraw()
    #--------------------------------------------------------------------------
    def timeout(self, event=None):
        if self.crobot.update() < 1:
            print('disconnected timeout...')
            self.timeout_active = False
            self.connect_button.set_label('connect (0)')
            return(False)
        self.update_buttons()
        self.update_sticks()
        self.update_graph()
        return(True)
    #--------------------------------------------------------------------------
    def update_buttons(self):
        for name, button in self.buttonbar.buttons.iteritems():
            button.set(self.crobot.getButtonEdge(button.buttonNo))
    #--------------------------------------------------------------------------            
    def update_sticks(self):
        self.buttonbar.sticks['right'].update(
            self.crobot.getStick(0), self.crobot.getStick(1))
        self.buttonbar.sticks['left'].update(
            self.crobot.getStick(2), self.crobot.getStick(3))
    #--------------------------------------------------------------------------
    def update_graph(self):
        self.graph.index += 1
        if self.graph.index >= 300:
            self.graph.index = 0
    #--------------------------------------------------------------------------
    def graph_timeout(self):
        self.graph.do_expose_event(None)
        return(True)
    #--------------------------------------------------------------------------
    def do_scroll(self, widget, event):
        handled = False
        if event.direction == gtk.gdk.SCROLL_DOWN:
            handled = True
            self.change_selected(-0.1)
        if event.direction == gtk.gdk.SCROLL_UP:
            handled = True
            self.change_selected(0.1)
        return(handled)
    #--------------------------------------------------------------------------
    def do_keypress(self, widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval).lower()
        handled = False
        if keyname == 'space':
            handled = True
        elif keyname == 'escape':
            gtk.main_quit()
        elif (keyname == 'plus' or keyname == 'equal' or keyname == 'kp_add'):
            self.change_selected(0.1)
        elif (keyname == 'minus' or keyname == 'kp_subtract'):
            self.change_selected(-0.1)
        else:
            #print("unhandled: " + str(keyname) + " - " + str(event.keyval))
            pass
        return(handled)
    #--------------------------------------------------------------------------
    def change_selected(self, amount):
        #if not self.crobot.con: return
        selection = self.robotdrawing.selected
        if selection == -1: return
        if selection < drawrobot.RobotMainViewArea.SERVOCOUNT:
            l = int(selection) / drawrobot.ServoBox.PERLEG
            s = int(selection) % drawrobot.ServoBox.PERLEG
            result = self.crobot.changeServo(l, s, amount)
            print("move result: " + str(result))
            if result != 1:
                self.robotdrawing.blink(selection)
            else:
                self.crobot.commit()
                self.update_servoinfo()
        elif selection < (drawrobot.RobotMainViewArea.SERVOCOUNT
             + drawrobot.RobotMainViewArea.LEGCOUNT):
             l = int(selection) - drawrobot.RobotMainViewArea.SERVOCOUNT
             result = self.crobot.changeLeg(l, 0, 0, amount)
             print('move result: ' + str(result))
             if result == 0:
                self.crobot.commit()
                self.update_servoinfo()
    #--------------------------------------------------------------------------
    def configure(self):
        self.config = configuration.Configuration()
        if  not self.config.load(): exit(-1)
        for leg in self.config.legs:
            for servo in leg.servos:
                self.crobot.setServoOffset(leg.n, servo["n"], float(servo["offset"]))
                ori = servo["orientation"]
                if ori == "xy":
                    self.crobot.setServoDirection(leg.n, servo["n"], -1);
                elif ori == "zx":
                    self.crobot.setServoDirection(leg.n, servo["n"], -1);
            self.crobot.setLegLengths(
                leg.n,
                float(leg.sections["A"]["length"]),
                float(leg.sections["B"]["length"]),
                float(leg.sections["C"]["length"]))
        
    #--------------------------------------------------------------------------
      


#crobot library handler
#======================
class Crobot:
        
    SERVOCOUNT = 12
    #--------------------------------------------------------------------------
    def __init__(self):
        bits = platform.machine()
        print("system reported: " + bits)
        if bits == 'i686' or bits == 'x86':
            bits = '32'
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
    #--------------------------------------------------------------------------
    def connect(self):
        self.con = self.crobotlib.Quadruped_startup(self.qped)
        self.refreshServoinfo()
        return(self.con)
    #--------------------------------------------------------------------------
    def __del__(self):
        self.crobotlib.Quadruped_free(self.qped)
    #--------------------------------------------------------------------------    
    def getButton(self, button):
        t = self.crobotlib.Quadruped_getPsButton(self.qped, button)
        #print('button ' + str(button) + ': ' + str(t))
        return(t)
    #--------------------------------------------------------------------------
    def getButtonEdge(self, button):
        t = self.crobotlib.Quadruped_getPsButtonEdge(self.qped, button)
        return(t)
    #--------------------------------------------------------------------------
    def getStick(self, axis):
        return(self.crobotlib.Quadruped_getPsAxis(self.qped, axis))
    #--------------------------------------------------------------------------    
    def update(self):
        return(self.crobotlib.Quadruped_update(self.qped))
    #--------------------------------------------------------------------------
    def changeServo(self, leg, s, value):
        return(self.crobotlib.Quadruped_changeSingleServo(self.qped, leg, s, c_double(value)))
    #--------------------------------------------------------------------------
    def getServoinfo(self): #put C info into the shared mem
        self.crobotlib.Quadruped_updateServoinfo(self.qped)
        return(self.servoinfo)
    #--------------------------------------------------------------------------
    def refreshServoinfo(self): #ask the device for actual info
        self.crobotlib.Quadruped_getServoData(self.qped)
        return(self.getServoinfo())
    #--------------------------------------------------------------------------
    def commit(self):
        return(self.crobotlib.Quadruped_commit(self.qped))
    #--------------------------------------------------------------------------
    def setLegLengths(self, legno, A, B, C):
        self.crobotlib.Quadruped_configureLegLengths(
            self.qped, legno, c_double(A), c_double(B), c_double(C))
    #--------------------------------------------------------------------------
    def setServoOffset(self, legno, servono, offset):
        self.crobotlib.Quadruped_configureServoOffset(
            self.qped, c_byte(int(legno)), c_byte(int(servono)), c_double(offset))
    #--------------------------------------------------------------------------
    def changeLeg(self, legno, dX, dY, dZ):
        result = self.crobotlib.Quadruped_changeLegEndpoint(
            self.qped, legno, c_double(dX), c_double(dY), c_double(dZ))
        return(result)
    #--------------------------------------------------------------------------
    def printLegs(self):
        self.crobotlib.Quadruped_debugLegs(self.qped)
    #--------------------------------------------------------------------------
    def setServoDirection(self, legno, servono, direction):
        self.crobotlib.Quadruped_configureServoDirection(self.qped, 
            c_byte(int(legno)), c_byte(int(servono)), c_int8(int(direction)))
        
    #--------------------------------------------------------------------------
    def printServoDetails(self, legno, servono):
        self.crobotlib.Quadruped_printServoDetails(self.qped,
            c_byte(int(legno)), c_byte(int(servono)))
    #--------------------------------------------------------------------------


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
