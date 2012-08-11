import pygtk
import gtk, gobject, cairo
import math
from random import random

class QpImage(gtk.DrawingArea):
    # a good size = 440 x 200 


    __gsignals__ = {"expose-event":"override",
                    "configure-event":"override"}
    BLACK = 0
    RED = 1
    GREEN = 2
    MAINBODYSIZE = 0.2 
    MAINBODYRATIO = 1.4 #h/w of main rectangle
    SERVOSIZE = 0.1
    MARGIN = 10 #flat pixel margin between servos
    SERVOCOUNT = 12
    LEGSIZE = 3 #servos per leg
    LEGCOUNT = 4
    TEXTSIZE = 12
    BLINKTIMEMS = 1000 #ms to flash red

    def __init__(self):
        gtk.DrawingArea.__init__(self)
        #self.bgcolor = [0.9, 0.9, 0.9]
        self.bgcolor = [1,1,1]
        self.cr = None
        self.height = 0
        self.width = 0
        self.mainbodyh =0
        self.mainbodyw = 0
        self.servow = 0
        self.servoh = 0
        #0-11 servos, 12-15 legs, 16 mainbody
        self.blocks = [ServoBlock(i) for i in range(QpImage.SERVOCOUNT+QpImage.LEGCOUNT+1)]
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK)
        self.connect("button_press_event", self.do_click)

    def redraw(self):
        self.do_expose_event()
        
    def setpw(self, n, pw):
        self.blocks[n].pw = pw
        
    def setangle(self, n, angle):
        self.blocks[n].angle = angle
        
    def do_click(self, widget, event):
        handled = False
        #normal click = select item (clear everything else)
        #ctrl click = toggle item (keep others, except main)
        #selecting mainbody also clears all others
        if event.type == gtk.gdk.BUTTON_PRESS:
            if event.button == 1:
                handled = True
                item = self.getItemByXY(event.x, event.y)
                if item < 0: return(handled) #misclick, ignore

                if(event.state & gtk.gdk.CONTROL_MASK):
                    if item == QpImage.SERVOCOUNT:
                        self.clearSelected()
                    else:
                        self.blocks[QpImage.SERVOCOUNT].off()
                    self.blocks[item].toggle_select()
                else:
                    self.clearSelected()
                    self.blocks[item].on()
                self.do_expose_event()
        return(handled)

    def select_block(self, blockno):
        self.clearSelected()
        self.blocks[blockno].on()
        self.do_expose_event()

    def blink(self, blockno):
        self.blocks[blockno].blinking += 1
        self.do_expose_event()
        if self.blocks[blockno].blinking == 1:
            gtk.timeout_add(QpImage.BLINKTIMEMS, self.blink_timeout, blockno)
    
    def blinknone(self):
        for block in self.blocks:
            block.blinking = 0
        self.do_expose_event()

    def blink_timeout(self, data):
        self.blocks[data].blinking -= 1
        self.do_expose_event()
        if self.blocks[data].blinking > 0:
            return(True)
        elif self.blocks[data].blinking <= 0:
            return(False)

    def clearSelected(self):
        for block in self.blocks:
            block.off()

    def get_selected(self):
        result = []
        for block in self.blocks:
            if block.selected: result.append(block.n)
        return(result)

    def get_single_selected(self):
        for block in self.blocks:
            if block.selected: return(block.n)
        return(-1)
        
    def getItemByXY(self, x, y):
        #check mainbody first
        block = self.blocks[-1]
        if block.x - block.w/2 < x < block.x + block.w/2:
            if block.y - block.h/2 < y < block.y + block.h/2:
                return(block.n)
        #next check the rest
        for block in self.blocks[:-1]:
            if block.x - block.w/2 < x < block.x + block.w/2:
                if block.y - block.h/2 < y < block.y + block.h/2:
                    return(block.n)
        return(-1)

    def do_expose_event(self, event=None):
        #clear and draw stuff
        self.cr = self.window.cairo_create()
        self.cr.push_group()
        self.cr.set_font_size(QpImage.TEXTSIZE)
        self.clear()
        #self.draw_mainbody()
        self.draw_blocks()
        self.cr.pop_group_to_source()
        self.cr.paint()

    def do_configure_event(self, event=None):
        #grab height/width and setup all sizes
        alloc = self.get_allocation()
        self.width = alloc.width
        self.height = alloc.height
        base = self.width
        #mainbody
        self.mainbodyw = base * QpImage.MAINBODYSIZE
        self.mainbodyh = self.mainbodyw * QpImage.MAINBODYRATIO
        mainbodyindex = QpImage.SERVOCOUNT+QpImage.LEGCOUNT
        self.blocks[mainbodyindex].w = self.mainbodyw
        self.blocks[mainbodyindex].h = self.mainbodyh
        self.blocks[mainbodyindex].x = self.width/2
        self.blocks[mainbodyindex].y = self.height/2
        #servos
        self.servow = base * QpImage.SERVOSIZE
        self.servoh = self.servow
        for i in range(QpImage.SERVOCOUNT):
            self.blocks[i].x, self.blocks[i].y = self.get_servoloc_by_number(i)
            self.blocks[i].w = self.servow
            self.blocks[i].h = self.servoh
        #legs
        legwidth  = self.width/2 - 4
        legheight = self.mainbodyh * 0.7
        #legbasex = self.width / 2
        #legbasey = self.height /2
        for i in range(self.LEGCOUNT):
            self.blocks[QpImage.SERVOCOUNT + i].x = self.blocks[i*self.LEGSIZE + 1].x
            self.blocks[QpImage.SERVOCOUNT + i].y = self.blocks[i*self.LEGSIZE + 1].y
            self.blocks[QpImage.SERVOCOUNT + i].w = legwidth
            self.blocks[QpImage.SERVOCOUNT + i].h = legheight
            self.blocks[QpImage.SERVOCOUNT + i].silent = True
        #draw after we've set all sizes
        self.do_expose_event()

    def clear(self):
        self.cr.set_source_rgb(*self.bgcolor)
        self.cr.rectangle(0, 0, self.width, self.height) 
        self.cr.fill()

    def draw_blocks(self):
        for block in self.blocks[QpImage.SERVOCOUNT:QpImage.SERVOCOUNT + QpImage.LEGCOUNT]:
            block.draw(self.cr)
        for block in self.blocks[:QpImage.SERVOCOUNT]:
            block.draw(self.cr)
        for block in self.blocks[QpImage.SERVOCOUNT + QpImage.LEGCOUNT:]:
            block.draw(self.cr)

    def toggle_select(self):
        if self.selected:
            self.selected = False
            self.color = QpImage.BLACK
        else:
            self.selected = True
            self.color = QpImage.GREEN

    def get_servoloc_by_number(self,i):
        #returns (x,y) drawlocations for servo i
        direction = 1.0
        legno = i / QpImage.LEGSIZE
        servono = i % QpImage.LEGSIZE
        w = self.width
        if legno == 3:  
            direction = -1.0
            # y = same as top of mainbody + half servo size
            y = self.height/2 + self.mainbodyh/2 - self.servoh/2
        elif legno == 2:
            direction = 1.0
            y = self.height/2 + self.mainbodyh/2 - self.servoh/2
        elif legno == 1:
            y = self.height/2 - self.mainbodyh/2 + self.servoh/2
            direction = -1.0
        elif legno == 0:
            y = self.height/2 - self.mainbodyh/2 + self.servoh/2
            direction = 1.0
        #x = to the right of mainbody, one default margin + half servowidth +
        #a margin and a servosize for each additional servo
        x = w*0.5 + direction * (self.mainbodyw/2 + QpImage.MARGIN + servono *(
            QpImage.MARGIN + self.servow) + self.servow/2 )

        return((x,y)) 

class ServoBlock:
    
    def __init__(self, n, x=0, y=0):
        self.x = x
        self.y = y
        self.color = QpImage.BLACK
        self.selected = False
        self.w = 0 
        self.h = 0
        self.n = n
        self.pw = 72
        self.angle = 0.0
        self.blinking = 0
        self.silent = False

        
    def draw(self, cr):
        #colored blocks with text at roughly the servo locations
        self.set_color(cr, self.color)
        cr.set_line_width(2.0)
        x = self.x
        y = self.y
        cr.rectangle(
            x - self.w/2,
            y - self.h/2,
            self.w,
            self.h)
        if not self.silent:
            cr.stroke()
            self.set_color(cr, QpImage.BLACK)
            cr.move_to(x -18 ,y-5)
            cr.show_text(str(self.n) + ":  " + str(self.pw))
            cr.move_to(x -18, y+10)
            cr.show_text("{:.2f}".format(self.angle))
            cr.stroke()
        else:
            if self.selected:
                cr.fill()
            else:
                cr.set_line_width(0.5)
                cr.stroke()
    
    def toggle_select(self):
        if self.selected:
            self.off()
        else:
            self.on()

    def off(self):
        self.selected = False
        self.color =QpImage.BLACK
    def on(self):
        self.selected = True
        self.color = QpImage.GREEN

    def set_color(self, cr, colorint):
        if colorint == QpImage.RED or self.blinking > 0:
            cr.set_source_rgb(1,0,0)
        elif colorint == QpImage.GREEN:
            cr.set_source_rgb(0,0.7,0)
        elif colorint == QpImage.BLACK:
            cr.set_source_rgb(0,0,0)

if __name__ == "__main__":
    window = gtk.Window()
    qpi = QpImage();
    window.add(qpi);
    window.show_all()
    window.connect('delete-event', gtk.main_quit)
    window.set_size_request(48,48)
    gtk.main()

