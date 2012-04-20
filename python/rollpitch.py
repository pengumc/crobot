#! /usr/bin/env python2
import pygtk
import gtk, cairo, gobject
import math

#recommended width: >200px
class RollPitch(gtk.DrawingArea):

    K = math.pi/180.0

    
    def __init__(self):
        gtk.DrawingArea.__init__(self)
        self.pitch = -0.5
        self.roll = 0.1
        self.width = 300
        self.height = 300
        self.pitchres = self.height * 20.0/300.0

    def setup_cr(self):
        self.cr = self.window.cairo_create()
        self.width, self.height = self.window.get_size()
        self.pitchres = self.height * 20.0/300.0

    def draw(self, event=None, data=None):
        self.setup_cr()
        self.clear()
        self.draw_pitch(self.width/2.0, self.height/2.0, self.pitch)
        self.draw_base_circle(
            self.width/2.0, #x
            self.height/2.0, #y
            math.sqrt(self.width*self.height)/3.0) #r
        self.draw_roll(self.width/2, self.height/2, self.width/3, self.roll)
        return(True)

    def clear(self):
        self.cr.set_source_rgb(0.0, 0.6, 0.9)
        self.cr.rectangle(0,0,self.width,self.height)
        self.cr.fill()

    def draw_pitch(self, x, y, angle):
        self.cr.set_source_rgb(0.9, 0.6, 0)
        y +=  (angle / RollPitch.K)*0.1*self.pitchres
        self.cr.rectangle(x - self.width/2.0, y, self.width, self.height)
        self.cr.fill()
        self.draw_text(x + self.width/3.0 , y -5, str(int(angle/RollPitch.K)))
        self.draw_pitch_grid(self.width/2,y, self.width/3, 50, self.pitchres)

    def draw_base_circle(self, x, y, r):
        self.cr.set_source_rgb(0,0,0)
        self.cr.set_line_width(2.0)
        self.cr.arc(x, y, r, 0, 2*math.pi)
        self.cr.stroke()

    def draw_pitch_grid(self, x, y, width, count, step):
        self.cr.set_source_rgb(0,0,0)
        self.cr.set_line_width(1)
        x -= width/2.0
        y -= count * step/2
        for i in range(count):
                self.cr.move_to(x, y)
                self.cr.rel_line_to(width, 0)
                y += step
        self.cr.stroke()

    def draw_roll(self, x, y, r, angle):
        self.cr.set_source_rgb(1, 0, 0)
        self.cr.set_line_width(2)
        angle = -angle
        self.cr.move_to(x+math.cos(angle)*r, y+math.sin(angle)*r)
        x = x-math.cos(angle)*r
        y = y-math.sin(angle)*r
        self.cr.line_to(x, y)
        self.cr.stroke()
        self.draw_text(x - 20, y+10, str(int((-angle / RollPitch.K))))

    def draw_text(self, x, y, text):
        self.cr.set_source_rgb(0,0,0)
        self.cr.set_font_size(22)
        self.cr.move_to(x,y)
        self.cr.show_text(text)

class RollPitchExampleScreen:

    def __init__(self):
        self.window = gtk.Window()
        self.window.connect('delete-event', gtk.main_quit)
        self.window.set_size_request(200, 200)
        self.rp = RollPitch()
        self.window.add(self.rp)
        self.window.show_all()
        self.rp.connect("expose_event", self.rp.draw)

    def run(self):
        gtk.main()

if __name__ == "__main__":
    print('start')
    screen = RollPitchExampleScreen()
    screen.run()
    
