import pygtk
import gtk, gobject, cairo
import os
import math

#playstation analog stick drawing
#==================================
class Psanalog(gtk.DrawingArea):
    #x and y are expected between -128 and 127
    SIZE = 48
    ARCSIZE = 5
    SCALE = SIZE/255.0
    __gsignals__ = {"expose-event":"override"}

    def __init__(self):
        gtk.DrawingArea.__init__(self)
        self.set_size_request(Psanalog.SIZE, Psanalog.SIZE)
        self.X = 0
        self.Y = 0
    
    def do_expose_event(self, event):
        self.cr = self.window.cairo_create()
        self.cr.push_group()
        self.clear()    
        self.draw_stick()
        self.cr.pop_group_to_source()
        self.cr.paint()

    def draw_stick(self):
        self.cr.set_source_rgb(0.0, 0.5, 0.5)
        x = self.X * Psanalog.SCALE + Psanalog.SIZE / 2
        y = self.Y * Psanalog.SCALE + Psanalog.SIZE / 2
        self.cr.arc(x, y, Psanalog.ARCSIZE, 0.0, 2 * math.pi)
        self.cr.stroke()

    def clear(self):
        #empty everything
        self.cr.set_source_rgb(1.0, 1.0, 1.0)
        self.cr.rectangle(0, 0, Psanalog.SIZE, Psanalog.SIZE) 
        self.cr.fill()
        #draw square with cross
        self.cr.set_line_width(2.0)
        self.cr.set_source_rgb(0.0, 0.0, 0.0)
        self.cr.rectangle(2,2, Psanalog.SIZE-4, Psanalog.SIZE-4)
        self.cr.move_to(0, Psanalog.SIZE/2)
        self.cr.rel_line_to(Psanalog.SIZE, 0)
        self.cr.move_to(Psanalog.SIZE/2, 0)
        self.cr.rel_line_to(0, Psanalog.SIZE)
        #stroke
        self.cr.stroke()

    def update(self, x, y):
        self.X = x
        self.Y = y
        self.do_expose_event(None)


#playstation controller button image
#====================================
class Psbutton(gtk.Image):
    
    def __init__(self, pixbuf1, pixbuf2, buttonNo=12):
        gtk.Image.__init__(self)
        self.pb_inactive = pixbuf1
        self.pb_active = pixbuf2
        self.active = False
        self.set_from_pixbuf(self.pb_inactive)
        #button number to use on the robot
        self.buttonNo = buttonNo 

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


#button bar
#===============
#psbuttons.png is assumed to be in the same dir
class ButtonBar (gtk.Table):
   
    BUTTONCOUNT = 16
    BUTTONSIZE = 48

    def __init__(self):
        gtk.Table.__init__(self, ButtonBar.BUTTONCOUNT, 1, False)
        self.count = 0
        self.mainbuf = gtk.gdk.pixbuf_new_from_file(
            os.path.dirname(__file__) + os.sep + 'psbuttons.png')
        self.buttons = {}
        self.sticks = {}
        self.generate_bar()
   
    def generate_bar(self):
        self.add_stick('left')
        self.add_stick('right')
        self.add_button(4,2, 'left', 7, 180)
        self.add_button(4,2, 'up', 4,  90)
        self.add_button(4,2, 'down', 6, 270)
        self.add_button(4,2, 'right', 5, 0)
        self.add_button(0,0, 'cross', 14)
        self.add_button(2,0, 'circle', 13)
        self.add_button(4,0, 'square', 15)
        self.add_button(6,0, 'triangle', 12)
        self.add_button(0,1, 'select', 0)
        self.add_button(2,1, 'start', 3)
        self.add_button(4,1, 'l1', 10)
        self.add_button(6,1, 'r1', 11)
        self.add_button(0,2, 'l2', 8)
        self.add_button(2,2, 'r2', 9)
        
    #x and y are coords in pixbuf
    #active version is assumed to be next to is
    def add_button(self, x, y, name, crobotButtonNo=0, rotation=0):
        size = ButtonBar.BUTTONSIZE
        pixbuf = self.mainbuf.subpixbuf(size*x, size*y, size, size)
        pixbuf = pixbuf.rotate_simple(rotation)
        apixbuf = self.mainbuf.subpixbuf(size*(x+1), size*y, size, size)
        aap= apixbuf.rotate_simple(rotation)
        self.buttons[name] = Psbutton(pixbuf, aap, crobotButtonNo)
        self.attach(self.buttons[name], self.count, self.count+1, 0,1, 0,0)
        self.count = self.count +1

    def add_stick(self, name):
        self.sticks[name] =  Psanalog()
        self.attach(self.sticks[name], self.count, self.count+1, 0,1, 0,0)
        self.count = self.count+1



if __name__ == "__main__":
    window = gtk.Window()
    bar = ButtonBar()
    window.add(bar)
    window.show_all()
    window.connect('delete-event', gtk.main_quit)
    window.set_size_request(48,48)
    gtk.main()
