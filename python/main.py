import signal
import sys, os, math
import platform
import pygtk
import gtk, gobject, cairo
import time
import threading

import buttonbar
import grapharea
#import drawrobot
import qpimage
import configuration
from crobotlib import Crobot
import display3d

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
    SERVOCOUNT = 12
    DOF = 3
    LEGCOUNT = 4
    SPEED = 0.5
    STICKTHRES = 50
    #--------------------------------------------------------------------------    
    def __init__(self, crobot):
        self.timeout_active = False
        self.window = gtk.Window();
        self.window.connect('destroy', gtk.main_quit)
        self.window.connect('key_press_event', self.do_keypress);
        self.window.add_events(gtk.gdk.SCROLL_MASK)
        self.window.connect("scroll-event", self.do_scroll)
        self.maintable = gtk.Table(2, 3)
        #=============
        #self.robotdisp = drawrobot.RobotMainViewArea()
        self.robotdisp = qpimage.QpImage()
        self.maintable.attach(self.robotdisp, 0,1, 0,1, gtk.FILL,gtk.FILL|gtk.EXPAND) 
        self.robotdisp.set_size_request(440,200)
        #=============
        #buttonlist
        self.buttontable = gtk.Table(1,4)
        self.maintable.attach(
        self.buttontable, 1,2, 0,1, gtk.FILL|gtk.EXPAND, gtk.FILL|gtk.EXPAND)
        self.connect_button = gtk.Button("connect")
        self.connect_button.connect("clicked", self.connect_click)
        self.buttontable.attach(self.connect_button, 0,1, 0,1,gtk.FILL,gtk.FILL)
        self.debug_button = gtk.Button("debug info")
        self.debug_button.connect("clicked", self.debug_click)
        self.buttontable.attach(self.debug_button, 0,1, 1,2, gtk.FILL,gtk.FILL)
        self.load_button = gtk.Button("Load from eeprom")
        self.load_button.connect("clicked", self.load_click)
        self.buttontable.attach(self.load_button, 0,1, 2,3, gtk.FILL,gtk.FILL)
        self.launch3d_button = gtk.Button("Launch 3D")
        self.launch3d_button.connect("clicked", self.launch3d_click)
        self.buttontable.attach(self.launch3d_button, 0,1, 3,4, gtk.FILL,gtk.FILL)
        #buttonbar
        self.buttonbar = buttonbar.ButtonBar()
        self.maintable.attach(self.buttonbar, 0,2, 2,3, 0,0)
        #launch
        self.window.add(self.maintable)
        self.window.show_all()
        #crobot lib
        self.crobot = crobot
        self.configure()
        self.crobot.enable_com()
        #3D
        self.vp = display3d.VPythonThread()
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
                self.connect_button.set_label('connect ({})'.format(con))
                self.timeout_active = True
                gtk.timeout_add(Screen.TIMEOUT, self.timeout)
        self.connect_button.set_label('connect ({})'.format(con))
    #--------------------------------------------------------------------------            
    def _attach_analog(self, table):
        table.attach(self.right_analog, 1,3, 1,3, 0,0)        
        table.attach(self.left_analog, 3,5, 1,3, 0,0)        
    #--------------------------------------------------------------------------
    def start(self):
        self.connect_click(None)
        self.update_servoinfo()
        gtk.main()
    #--------------------------------------------------------------------------
    def connect_click(self, event):
        self.connect_to_device()
    #--------------------------------------------------------------------------
    def debug_click(self, event):
        self.crobot.printLegs();
        #for i in range(4):
        #    for j in range(3):
        #        print('servo {},{}'.format(i,j))
        #        self.crobot.printServoDetails(i, j)
        for i in range(Crobot.SERVOCOUNT):
            print("servo {}: {} {:.2f} -- {:.2f}, {:.2f}, {:.2f}".format(
                i, self.crobot.com.pulsewidths[i], self.crobot.com.angles[i],
                self.crobot.com.servopos.x[i],self.crobot.com.servopos.y[i],
                self.crobot.com.servopos.z[i]))
            if i%3 == 2:
                print("endpoint {}: {:.2f}, {:.2f}, {:.2f}".format(
                    i/3,
                    self.crobot.com.endpoints.x[i/3],
                    self.crobot.com.endpoints.y[i/3],
                    self.crobot.com.endpoints.z[i/3]))
    #--------------------------------------------------------------------------
    def load_click(self, event):
        self.crobot.load_from_eeprom()
        self.update_servoinfo()
    #--------------------------------------------------------------------------
    def launch3d_click(self, event):
        if not self.vp.isAlive():
            self.vp.start()
            #workaround. the vp thread won't start until after this function
            #returns for some reason....
            gtk.timeout_add(500, self.wait_vpython)
    def wait_vpython(self):
        result = display3d.threadlock.acquire(False) 
        if result:
            self.update_servoinfo()
        return(result)
    #--------------------------------------------------------------------------
    def timeout(self, event=None):
        if self.crobot.update_sensor_data() < 1:
            print('disconnected timeout...')
            self.timeout_active = False
            self.connect_button.set_label('connect (0)')
            return(False)
        self.update_buttons()
        self.update_sticks()
        self.controller_actions()
        return(True)
    #--------------------------------------------------------------------------
    def update_buttons(self):
        for name, button in self.buttonbar.buttons.iteritems():
            edge = self.crobot.getButtonEdge(button.buttonNo)
            button.set(edge)
            if edge == 1:
                self.psbutton_action(name)
    #--------------------------------------------------------------------------            
    def update_sticks(self):
        self.buttonbar.sticks['right'].update(
            self.crobot.getStick(0), self.crobot.getStick(1))
        self.buttonbar.sticks['left'].update(
            self.crobot.getStick(2), self.crobot.getStick(3))
    #--------------------------------------------------------------------------
    def controller_actions(self):
        leftx = self.buttonbar.sticks['left'].X
        if abs(leftx) > self.STICKTHRES:
            self.change_selected((float(leftx)/128.0*self.SPEED, 0, 0))
        lefty = self.buttonbar.sticks['left'].Y
        if abs(lefty) > self.STICKTHRES:
            self.change_selected((0, -float(lefty)/128.0*self.SPEED, 0))
        righty = self.buttonbar.sticks['right'].Y
        if abs(righty) > self.STICKTHRES:
            self.change_selected((0, 0, -float(righty)/128.0*self.SPEED))
    #--------------------------------------------------------------------------
    def psbutton_action(self, name):
        print(name)
        if name == 'select':
            self.robotdisp.select(self.SERVOCOUNT + self.LEGCOUNT)
        elif name == 'r1':
            self.robotdisp.select(self.SERVOCOUNT + 0)
        elif name == 'r2':
            self.robotdisp.select(self.SERVOCOUNT + 2)
        elif name == 'l1':
            self.robotdisp.select(self.SERVOCOUNT + 1)
        elif name == 'l2':
            self.robotdisp.select(self.SERVOCOUNT + 3)
    #--------------------------------------------------------------------------
    def do_scroll(self, widget, event):
        handled = False
        if event.direction == gtk.gdk.SCROLL_DOWN:
            handled = True
            self.change_selected(-0.2)
        if event.direction == gtk.gdk.SCROLL_UP:
            handled = True
            self.change_selected(0.2)
        return(handled)
    #--------------------------------------------------------------------------
    def do_keypress(self, widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval).lower()
        handled = False
        if keyname == 'space':
            handled = True
        elif keyname == 'escape':
            self.window.destroy()
            gtk.main_quit()
        elif (keyname == 'plus' or keyname == 'equal' or keyname == 'kp_add'):
            self.change_selected(0.1)
        elif (keyname == 'minus' or keyname == 'kp_subtract'):
            self.change_selected(-0.1)
        elif (keyname == 'up' or keyname == 'w'):
            self.change_selected((0, self.SPEED, 0))
        elif (keyname == 'down' or keyname == 's'):
            self.change_selected((0, -self.SPEED, 0))
        elif (keyname == 'left' or keyname == 'a'):
            self.change_selected((self.SPEED, 0.0, 0))
        elif (keyname == 'right' or keyname == 'd'):
            self.change_selected((-self.SPEED, 0, 0))
        else:
            #print("unhandled: " + str(keyname) + " - " + str(event.keyval))
            pass
        return(handled)
    #--------------------------------------------------------------------------
    def update_servoinfo(self):
        info = self.crobot.com
        for i in range(Crobot.SERVOCOUNT):
            self.robotdisp.setpw(i, info.pulsewidths[i])
            self.robotdisp.setangle(i, info.angles[i])
        self.robotdisp.redraw()
        if self.vp.isAlive(): self.update_servoinfo_3d()
    #--------------------------------------------------------------------------
    def update_servoinfo_3d(self):
        info = self.crobot.com
        for i in range(Crobot.SERVOCOUNT):
            self.vp.set_servo_pos(i, 
                info.servopos.x[i],
                info.servopos.y[i],
                info.servopos.z[i])
        for i in range(Crobot.LEGCOUNT):
            self.vp.set_endpoint_pos(i,
                info.endpoints.x[i],
                info.endpoints.y[i],        
                info.endpoints.z[i])
    #--------------------------------------------------------------------------
    def change_selected(self, amount):
        try:
            value = amount[0]
        except TypeError:
            value = amount
            amount = (0,0,value)
        #if not self.crobot.con: return
        selection = self.robotdisp.get_single_selected()
        if selection == -1: return
        if selection < self.SERVOCOUNT:
            l = int(selection) / self.DOF
            s = int(selection) % self.DOF
            result = self.crobot.changeServo(l, s, value)
            print("move result: " + str(result))
            if result != 1:
                self.robotdisp.blink(selection)
            else:
                self.crobot.commit()
                self.robotdisp.blinknone()
                self.update_servoinfo()
        elif selection < (self.SERVOCOUNT + self.LEGCOUNT):
             l = int(selection) - self.SERVOCOUNT
             result = self.crobot.changeLeg(l, *amount)
             print('move result: ' + str(result))
             if result == 0:
                self.crobot.commit()
                self.robotdisp.blinknone()
                self.update_servoinfo()
        elif selection == self.SERVOCOUNT + self.LEGCOUNT:
            result = self.crobot.changeAllLegs(*amount)
            if result == 0:
                self.crobot.commit()
                self.robotdisp.blinknone()
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
            self.crobot.set_leg_offset(
                leg.n, 
                float(leg.offset["x"]),
                float(leg.offset["y"]),
                float(leg.offset["z"]))
    #--------------------------------------------------------------------------


      
#-------------------------------start -----------------------------------------
if __name__ == "__main__":
    signal.signal(signal.SIGTERM, handleSigTERM) 
    signal.signal(signal.SIGINT, handleSigTERM) 
    gobject.threads_init()
    bits = platform.machine()
    print("machine: " + bits)
    if bits == 'i686' or bits == 'x86':
        bits = '32'
    elif bits == 'x86_64' or bits == 'AMD64':
        bits = '64'
    if sys.platform == 'linux2':
        LIBCROBOT = os.path.dirname(__file__) + "/../lib/libcrobot" + bits + ".so.1.0.1"
    elif sys.platform == 'win32':
        LIBCROBOT = os.path.dirname(__file__) + "/../lib/libcrobot" + bits + ".dll"
    print("lib: " + LIBCROBOT)
    try:
        screen = Screen(Crobot(LIBCROBOT))
        screen.start()
    except KeyboardInterrupt:
        handleSigTERM()
