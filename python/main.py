import signal
import sys, os, math
import platform
import pygtk
import gtk, gobject, cairo
import threading

import buttonbar
import grapharea
import drawrobot #import qpimage
import configuration
from crobotlib import Crobot

def handleSigTERM():
    #gtk.main_quit()
    print("quitting...")
    sys.exit(0)

def c_vector(x, y, z):
    return((y, z, x))


class VPythonThread(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
        
    def run(self):
        import visual
        self.scene = visual.display()
        self.b = visual.box()
        while 1:
            visual.rate(30)
            self.b.rotate(angle=0.01, axis=c_vector(1,1,0), orig=self.b.pos)
            


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
        self.window.connect('destroy', gtk.main_quit)
        self.window.connect('key_press_event', self.do_keypress);
        self.window.add_events(gtk.gdk.SCROLL_MASK)
        self.window.connect("scroll-event", self.do_scroll)
        self.maintable = gtk.Table(2, 3)
        #drawrobot
        self.robotdrawing = drawrobot.RobotMainViewArea()
        self.maintable.attach(self.robotdrawing, 0,1, 0,1, gtk.FILL,gtk.FILL|gtk.EXPAND) 
        self.robotdrawing.set_size_request(500,500)
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
        self.vp = VPythonThread()
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
        if not self.vp.isAlive(): self.vp.start()
    #--------------------------------------------------------------------------
    def timeout(self, event=None):
        if self.crobot.update_sensor_data() < 1:
            print('disconnected timeout...')
            self.timeout_active = False
            self.connect_button.set_label('connect (0)')
            return(False)
        self.update_buttons()
        self.update_sticks()
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
            self.window.destroy()
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
    def update_servoinfo(self):
        info = self.crobot.com
        for i in range(Crobot.SERVOCOUNT):
            self.robotdrawing.servoboxes[i].data[0] = info.pulsewidths[i]
            self.robotdrawing.servoboxes[i].data[1] = info.angles[i]
        self.robotdrawing.redraw()
        
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
                self.robotdrawing.blinknone()
                self.update_servoinfo()
        elif selection < (drawrobot.RobotMainViewArea.SERVOCOUNT
             + drawrobot.RobotMainViewArea.LEGCOUNT):
             l = int(selection) - drawrobot.RobotMainViewArea.SERVOCOUNT
             result = self.crobot.changeLeg(l, 0, 0, amount)
             print('move result: ' + str(result))
             if result == 0:
                self.crobot.commit()
                self.robotdrawing.blinknone()
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
        LIBCROBOT = "lib/libcrobot" + bits + ".so.1.0.1"
    elif sys.platform == 'win32':
        LIBCROBOT = "lib/libcrobot" + bits + ".dll"
    print("lib: " + LIBCROBOT)
    try:
        screen = Screen(Crobot(LIBCROBOT))
        screen.start()
    except KeyboardInterrupt:
        handleSigTERM()
