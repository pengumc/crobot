import threading

#corrected vector
def c_vector(x, y, z):
    return((y, z, x))


class VPythonThread(threading.Thread):
    
    MAIN_H = 14 #mainbody width
    MAIN_W = 8
    MAIN_T = 0.1
    S_WHT = 1 #servo indicator with, height, Thickness (it's a cube)
    
    
    def __init__(self):
        threading.Thread.__init__(self)
        name = "vpython_thread"
        
    def run(self):
        import visual
        self.scene = visual.display()
        self.servos = [visual.box(color=visual.color.green) for i in range(12)]
        self.curves = [visual.curve() for i in range(len(self.servos))]
        self.endpoints = [visual.box(color=visual.color.cyan) for i in range(4)]
        self.mainbodycurves = [visual.curve(color=visual.color.red) for i in range(4)]
        while 1:
            visual.rate(60)

    def set_servo_pos(self, n, x, y, z):
        #print("setting {} to ({:.2f},{:.2f},{:.2f})".format(n, x, y, z))
        self.servos[n].pos = (c_vector(x, y, z))
        if n%3 > 0:
            self.curves[n].pos = [self.servos[n-1].pos, self.servos[n].pos]
        else:
            self.mainbodycurves[0].pos = [self.servos[0].pos, self.servos[6].pos]
            self.mainbodycurves[1].pos = [self.servos[6].pos, self.servos[9].pos]
            self.mainbodycurves[2].pos = [self.servos[9].pos, self.servos[3].pos]
            self.mainbodycurves[3].pos = [self.servos[3].pos, self.servos[0].pos]
            
    def set_endpoint_pos(self, n, x, y, z):
        self.endpoints[n].pos = (c_vector(x, y, z))
        self.curves[n*3].pos = [self.servos[n*3+2].pos, self.endpoints[n].pos]
     