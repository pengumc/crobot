import pygtk
import gtk, gobject, cairo
import math
from random import random

class QpImage(gtk.DrawingArea):
    
    __gsignals__ = {"expose-event":"override",
                    "configure-event":"override"}
    BLACK = 0
    RED = 1
    GREEN = 2
    MAINBODYSIZE = 0.2
    MAINBODYRATIO = 1.4
    SERVOSIZE = 0.1
    MARGIN = 10
    SERVOCOUNT = 12
    LEGSIZE = 3
    TEXTSIZE = 16

    def __init__(self):
        gtk.DrawingArea.__init__(self)
        self.bgcolor = [0.7, 0.7, 0.7]
        self.cr = None
        self.height = 0
        self.width = 0
        self.mainbodyh =0
        self.mainbodyw = 0
        self.servow = 0
        self.servoh = 0
        self.servocolors = [QpImage.BLACK for i in range(QpImage.SERVOCOUNT)]
        self.servopw = [72 for i in range(QpImage.SERVOCOUNT)]

    def do_expose_event(self, event=None):
        #clear and draw stuff
        self.cr = self.window.cairo_create()
        self.cr.push_group()
        self.cr.set_font_size(QpImage.TEXTSIZE)
        self.clear()
        self.draw_mainbody()
        self.cr.pop_group_to_source()
        self.cr.paint()

    def do_configure_event(self, event=None):
        #grab height/width and setup all sizes
        alloc = self.get_allocation()
        self.width = alloc.width
        self.height = alloc.height
        
        base = self.width
        self.mainbodyw = base * QpImage.MAINBODYSIZE
        self.mainbodyh = self.mainbodyw * QpImage.MAINBODYRATIO
        self.servow = base * QpImage.SERVOSIZE
        self.servoh = self.servow

        #draw after we've set all sizes
        self.do_expose_event()

    def clear(self):
        self.cr.set_source_rgb(*self.bgcolor)
        self.cr.rectangle(0, 0, self.width, self.height) 
        self.cr.fill()
        for i in range(QpImage.SERVOCOUNT):
           self.draw_servo(i, self.servocolors[i], self.servopw[i]) 
            

    def draw_mainbody(self):
        #rectangle at center of area
        self.cr.set_source_rgb(0,0,0)
        self.cr.rectangle(
            self.width/2 - self.mainbodyw/2,
            self.height/2 - self.mainbodyh/2,
            self.mainbodyw,
            self.mainbodyh) 
        self.cr.stroke()

    def draw_servo(self, n, color, pw):
        self.set_color(color)
        x, y = self.get_servoloc_by_number(n)
        self.cr.rectangle(
            x - self.servow/2,
            y - self.servoh/2,
            self.servow,
            self.servoh)
        self.cr.stroke()
        self.set_color(QpImage.BLACK)
        self.cr.move_to(x -20 ,y)
        self.cr.show_text(str(n) + ": " + str(pw))
        self.cr.stroke()

    def set_color(self,colorint):
        if colorint == QpImage.RED:
            self.cr.set_source_rgb(1,0,0)
        if colorint == QpImage.GREEN:
            self.cr.set_source_rgb(0,0.7,0)
        if colorint == QpImage.BLACK:
            self.cr.set_source_rgb(0,0,0)
    
    def get_servoloc_by_number(self,i):
        #returns (x,y) drawlocations for servo i
        direction = 1.0
        legno = i / QpImage.LEGSIZE
        servono = i % QpImage.LEGSIZE
        w = self.width
        if legno == 0:  
            direction = 1.0
            # y = same as top of mainbody + half servo size
            y = self.height/2 - self.mainbodyh/2 + self.servoh/2
        elif legno == 1:
            direction = -1.0
            y = self.height/2 - self.mainbodyh/2 + self.servoh/2
        elif legno == 2:
            y = self.height/2 + self.mainbodyh/2 - self.servoh/2
            direction = 1.0
        elif legno == 3:
            y = self.height/2 + self.mainbodyh/2 - self.servoh/2
            direction = -1.0
            

        #x = to the right of mainbody, one default margin + a margin and a servosize
        # for each additional servo
        x = w*0.5 + direction * (self.mainbodyw/2 + QpImage.MARGIN + servono *(
            QpImage.MARGIN + self.servow) + self.servow/2 )

        return((x,y)) 

if __name__ == "__main__":
    window = gtk.Window()
    qpi = QpImage();
    window.add(qpi);
    window.show_all()
    window.connect('delete-event', gtk.main_quit)
    window.set_size_request(48,48)
    gtk.main()

