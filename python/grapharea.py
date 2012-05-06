import pygtk
import gtk, gobject, cairo
from ctypes import *
from random import random
import math


#grapharea
#===========
class GraphArea(gtk.DrawingArea):

    __gsignals__ = {"expose-event":"override"}
    

    def __init__(self, datalist):
        gtk.DrawingArea.__init__(self)
        self.max = 1.0
        self.min = 0.0
        self.index = 0
        self.lines = []
        for data in datalist:
            self.lines.append(GraphLine(data))


    def do_expose_event(self, event):
        self.clear()
        self.drawLines()
        
    def clear(self):
        self.cr = self.window.cairo_create()
        self.width, self.height = self.window.get_size()
        #empty everything
        self.cr.set_source_rgb(0, 1.0, 1.0)
        self.cr.rectangle(0, 0, self.width, self.height) 
        self.cr.fill()


    def drawLines(self):
        self.cr.set_source_rgb(0,0,0)
        self.cr.move_to(0, self.height/2)
        self.cr.rel_line_to(self.width, 0)
        self.cr.stroke()
        self.cr.set_source_rgb(1,0,0)
        for line in self.lines:
            line.drawFromData(self.cr, self.index, self.height, self.width)

    def update(self, index):
        self.index = index
        self.do_expose_event(None)


#GraphLine
#=============
class GraphLine:
    
    def __init__(self, data):
        self.scale = 1
        self.setData(data, len(data))
        self.color = [random(),random(),random()]

    def drawFromData(self, cr, index, height, width):
        cr.set_source_rgb(*self.color)
        xscale = float(width) / float(self.length)
        yscale = height/2
        ybase = height/2
        cr.move_to(0, height/2)
        x = 0
        px = 0
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
    a = GraphArea([data2, data])
    a.lines[0].setColor(1,0,0)
    a.lines[1].setColor(1,0,1)
    window.add(a)
    window.show_all()
    window.connect('delete-event', gtk.main_quit)
    window.set_size_request(48,48)
    gtk.main()
