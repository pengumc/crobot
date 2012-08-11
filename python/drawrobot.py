import pygtk
import gtk, gobject, cairo



#MAIN VIEW AREA
#--------------
#0-11 servos
#12-15 legs
#16 body
class RobotMainViewArea(gtk.Table):
    SERVOCOUNT = 12
    LEGCOUNT = 4  
    BLINKTIMEMS = 2000  
    #-------------------------------------------------------    
    def __init__(self):
        gtk.Table.__init__(self, 2, 2, False)
        self.selected = -1
        self.servoboxes = [ServoBox(i) for i in range(self.SERVOCOUNT)]
        self.legviews = [
            RobotLegView(i, self.servoboxes[i*3:(i*3)+3], self.click_callback) 
            for i in range(self.LEGCOUNT)]
        self.legviews[0].alignment = RobotLegView.RIGHT
        self.legviews[2].alignment = RobotLegView.RIGHT
        self.attach(self.legviews[1],
            0,1,
            0,1,
            gtk.EXPAND|gtk.FILL,
            gtk.EXPAND|gtk.FILL)
        self.attach(self.legviews[0],
            1,2,
            0,1,
            gtk.EXPAND|gtk.FILL,
            gtk.EXPAND|gtk.FILL)
        self.attach(self.legviews[3],
            0,1,
            1,2,
            gtk.EXPAND|gtk.FILL,
            gtk.EXPAND|gtk.FILL)
        self.attach(self.legviews[2],
            1,2,
            1,2,
            gtk.EXPAND|gtk.FILL,
            gtk.EXPAND|gtk.FILL)
            
    #-------------------------------------------------------
    def click_callback(self, selected):
        self.selected = selected
        if selected < self.SERVOCOUNT + self.LEGCOUNT:
            for legview in self.legviews:
                legview.deselect_all_but(selected)
    #-------------------------------------------------------
    def redraw(self):
        for legview in self.legviews:
            legview.do_expose_event()
    #-------------------------------------------------------
    def blink(self, blockno):
        self.servoboxes[blockno].blinking += 1
        self.redraw()
        if self.servoboxes[blockno].blinking == 1:
            gtk.timeout_add(self.BLINKTIMEMS, self.blink_timeout, blockno)
    def blinknone(self):
        for box in self.servoboxes:
            box.blinking = 0;
        self.redraw()
   #-------------------------------------------------------
    def blink_timeout(self, data):
        self.servoboxes[data].blinking -= 1
        self.redraw()
        if self.servoboxes[data].blinking > 0:
            return(True)
        elif self.servoboxes[data].blinking <= 0:
            return(False)
   #-------------------------------------------------------
    def get_single_selected(self):
        return(self.selected)
    #--------------------------------------------------------------
    def setpw(self, n, pw):
        self.servoboxes[n].data[0] = pw
    #--------------------------------------------------------------
    def setangle(self, n, angle):
        self.servoboxes[n].data[1] = angle
    
                
    
#ROBOT LEG VIEW
#--------------
class RobotLegView(gtk.DrawingArea):

    __gsignals__ = {"expose-event":"override",
                    "configure-event":"override"}

    BGCOLOR_ACTIVE = [0.9, 1 ,0.9]
    BGCOLOR_INACTIVE = [1,1,1]
    LEFT = 0
    RIGHT = 1
    
    #--------------------------------------------------------------
    def __init__(self, n, servoboxes, clickcallback):
        gtk.DrawingArea.__init__(self)
        self.click_callback = clickcallback
        self.servoboxes = servoboxes
        self.alignment = self.LEFT
        self.add_events(gtk.gdk.BUTTON_PRESS_MASK)
        self.connect("button_press_event", self.do_click)
        self.selected = -1
        self.active = False
        self.n = n
        self.scale = 1.0
    #--------------------------------------------------------------
    def do_click(self, widget, event):
        handled = False
        if event.type == gtk.gdk.BUTTON_PRESS:
            if event.button == 1:
                self.selected = RobotMainViewArea.SERVOCOUNT + self.n
                handled = True
                #check which cell, then which box
                #for now just cell 0 
                for box in self.servoboxes:
                    if box.check_boundary(event.x, event.y):
                        self.selected = box.n
                        box.select()
                self.click_callback(self.selected)
            elif event.button == 2:
                pass
        return handled
    #-------------------------------------------------------
    def do_configure_event(self, event):
        alloc = self.get_allocation()
        self.width = alloc.width
        self.height = alloc.height
        #self.align(self.alignment)
        self.do_expose_event()
    #--------------------------------------------------------------        
    def do_expose_event(self, event=None):
        try:
            self.cr = self.window.cairo_create()
        except:
            return
        self.cr.push_group()
        self.clear()
        for box in self.servoboxes:
            box.line(self.cr)
        for box in self.servoboxes:
           box.draw(self.cr)
        self.cr.pop_group_to_source()
        self.cr.paint()
    #--------------------------------------------------------------
    def clear(self):
        try:
            if self.active:
                self.cr.set_source_rgb(*self.BGCOLOR_ACTIVE)
            else:
                self.cr.set_source_rgb(*self.BGCOLOR_INACTIVE)
            self.cr.rectangle(0,0,self.width, self.height)
            self.cr.fill()
        except Exception as e:
            print('robot legview clear exception: ' + str(e))
            return
    #--------------------------------------------------------------
    def deselect_all_but(self, selection):
        for box in self.servoboxes:
            if box.n <> selection:
                box.deselect()
        if selection == self.n + RobotMainViewArea.SERVOCOUNT:
            self.active = True
        else:
            self.active = False
        self.do_expose_event()
    #--------------------------------------------------------------
    def set_xz(self, servo_n, x, y):
        self.servoboxes[servo_n].pos = [
            self.servoboxes[0].pos[0] + x*self.scale,
            self.servoboxes[0].pos[1] + y*self.scale]
    #--------------------------------------------------------------
    def align(self, alignment):
        if alignment == self.RIGHT:
            self.servoboxes[0].pos[0] = self.width - 32
        else:
            self.servoboxes[0].pos[0] = 32
        self.alignment = alignment

#SERVOBOX
#--------------
class ServoBox:
    
    SELECTCOLOR = [0, 0.8, 0]
    FAILCOLOR = [1, 0, 0]
    BGCOLOR = [1,1,1]
    FONTSIZE  = 14
    TXTX = -20  #text start at this offset from x
    TXTY = -9    #text start at this offset from y
    TXTHEIGHT = 12
    LINEWIDTH = 1
    SIZE = 56
    PERLEG = 3
    
    #---------------------------------------------
    def __init__(self, n):
        self.data = [0,0] #pw angle
        self.size = [self.SIZE, self.SIZE] #width height
        self.pos = [32+(n%self.PERLEG)*60 ,100+(n%self.PERLEG)*60] # x y
        self.blinking = 0
        self.selected = False
        self.n = n
    #---------------------------------------------        
    def draw(self, cr):
        cr.set_line_width(self.LINEWIDTH)
        self.clear_block(cr)
        self.textbox(cr)
        cr.move_to(self.pos[0], self.pos[1])
    #---------------------------------------------
    def clear_block(self, cr):
        if self.blinking > 0:
            color = self.FAILCOLOR
        else:
            if self.selected:
                color = self.SELECTCOLOR    
            else:
                 color = self.BGCOLOR
        cr.set_source_rgb(*color)
        cr.rectangle(
            self.pos[0] - self.size[0]/2, self.pos[1] - self.size[1]/2,
            self.size[0], self.size[1])
        cr.fill()
    #--------------------------------------------
    def line(self, cr):
        cr.set_source_rgb(0,0,0)
        if self.n % self.PERLEG <> 0:
            cr.line_to(self.pos[0], self.pos[1])
            cr.stroke()
        cr.move_to(*self.pos)
    #--------------------------------------------
    def textbox(self, cr):
        cr.set_source_rgb(0,0,0)
        cr.rectangle(
            self.pos[0] - self.size[0]/2, self.pos[1] - self.size[1]/2,
            self.size[0], self.size[1])
        cr.set_font_size(self.FONTSIZE)
        cr.move_to(self.pos[0] + self.TXTX, self.pos[1] + self.TXTY)
        text = '# {:2d}'.format(self.n)
        cr.show_text(text)
        cr.move_to(self.pos[0] + self.TXTX, self.pos[1] + self.TXTY + self.TXTHEIGHT)
        text = '{}'.format(self.data[0])
        cr.show_text(text)
        cr.move_to(self.pos[0] + self.TXTX, self.pos[1] + self.TXTY + self.TXTHEIGHT*2)
        text = '{:.2f}'.format(self.data[1])
        cr.show_text(text)
        cr.stroke()
    #--------------------------------------------
    def select(self):
        self.selected = True
    #--------------------------------------------
    def deselect(self):
        self.selected = False
    #--------------------------------------------
    def toggle(self):
        if self.selected:
             self.deselect()
        else:
             self.select()                
    #--------------------------------------------
    def check_boundary(self, x, y):
        hit = False
        if (self.pos[0] - self.size[0]/2 < x < self.pos[0] + self.size[0]/2
            and self.pos[1] - self.size[1]/2 < y < self.pos[1] + self.size[1]/2):
            hit = True
        #print('box {}: {},{}  {}'.format(self.n, x, y, hit))
        return hit
#START MAIN
#----------
if __name__ == "__main__":
    window = gtk.Window()
    a = RobotMainViewArea()
    window.add(a)
    window.show_all()
    window.connect('delete-event', gtk.main_quit)
    window.set_size_request(500,500)
    
    a.servoboxes[0].pos[0] = 100
    
    gtk.main()
