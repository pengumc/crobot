import pygtk
import gtk, gobject, cairo
from ctypes import *
from random import random
import math


#grapharea
#===========
class GraphArea(gtk.DrawingArea):

    __gsignals__ = {"expose-event":"override",
                    "configure-event":"override"}
    

    def __init__(self ):
        gtk.DrawingArea.__init__(self)
        self.color = [0, 0.5, 0.5]
        self.index = 0
        self.lines = []

    def setData(self, datalist):
        for data in datalist:
            self.lines.append(GraphLine(data))
        
    def do_configure_event(self, event):
        self.cr = self.window.cairo_create()
        alloc = self.get_allocation()
        self.width = alloc.width
        self.height = alloc.height
        self.clear()
        self.do_expose_event(None)

    def do_expose_event(self, event=None):
        #ask for forgiveness
        try:
            self.cr = self.window.cairo_create()
            self.cr.set_line_width(1);
            self.cr.push_group()
            self.clear()
            self.drawLines()
            self.cr.pop_group_to_source()
            self.cr.paint()
        except AttributeError:
            print("graph: attr error: " + str(self.index))
            return
            self.do_configure_event(None)

    def set_maxy(self, maxAbsY):
        for line in self.lines:
            line.scale = 1.0 / maxAbsY
        
    def clear(self):
        #empty everything
        self.cr.set_source_rgb(*self.color)
        self.cr.rectangle(0, 0, self.width, self.height) 
        self.cr.fill()

    def setBgColor(self, r, g, b):
        self.color = [r, g, b]

    def drawLines(self):
        self.cr.set_source_rgb(0,0,0)
        self.cr.move_to(0, self.height/2)
        self.cr.rel_line_to(self.width-100, 0)
        self.cr.stroke()
        self.cr.set_source_rgb(1,0,0)
        for line in self.lines:
            line.drawFromData(self.cr, self.index, self.height, self.width)

    def update(self, index):
        self.index = index
        self.do_expose_event(None)

    def timeout(self):
        self.index += 1
        if self.index >= 300:
            self.index = 0
        self.do_expose_event(None)
        return(True)


#GraphLine
#=============
class GraphLine:
    
    def __init__(self, data):
        self.scale = 1
        self.setData(data, len(data))
        self.color = [random(),random(),random()]

    def drawFromData(self, cr, index, height, width):
        cr.new_path()
        cr.set_source_rgb(*self.color)
        xscale = float(width-100) / float(self.length)
        yscale = height/2.0 * self.scale
        ybase = height/2
        cr.move_to(0, height/2)
        x = 0
        px = 0
        try:
            py = ybase - self.data[index]*yscale
            for i in range(index+1, self.length):
                x = x + xscale
                y = ybase - self.data[i]*yscale
                cr.move_to(px, py)
                cr.line_to(x, y)
                px = x
                py = y
            for i in range(0, index):
                x = x + xscale
                y = ybase - self.data[i]*yscale
                cr.move_to(px, py)
                cr.line_to(x, y)
                px = x
                py = y
            cr.move_to(px, ybase)
            cr.show_text("{:.2f}".format(self.data[index]))
        except IndexError:
            print("index error: " + str(self.index))
        cr.stroke()

    def setColor(self,r, g, b):
        self.color = [r, g, b];

    def setData(self, data, length):
        self.data = data
        self.length = length

if __name__ == "__main__":
    window = gtk.Window()
    data = (c_double *300)()
    data2 = (c_double*300)()
    for i in range(len(data)):
        data[i] = math.sin(i*(2*math.pi/300))
        data2[i] =random() 
        #print(str(data[i]) +"  " + str(data2[i]))
    a = GraphArea()
    a.setData([data2, data])
    a.set_maxy(2.0)
    a.lines[0].setColor(1,0,0)
    a.lines[1].setColor(1,0,1)
    window.add(a)
    window.show_all()
    a.do_configure_event(None) #TODO should be an event in grapharea
    window.connect('delete-event', gtk.main_quit)
    window.set_size_request(48,48)
    gtk.timeout_add(40, a.timeout)
    gtk.main()
