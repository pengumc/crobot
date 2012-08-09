import threading

threadlock = threading.Lock()

#corrected vector
def c_vector(x, y, z):
    return((y, z, x))


class VPythonThread(threading.Thread):
    
    MAIN_H = 14 #mainbody width
    MAIN_W = 8
    MAIN_T = 0.1
    S_WHT = 1.5 #servo indicator with, height, Thickness (it's a cube)
    
    
    def __init__(self):
        threading.Thread.__init__(self)
        self.name = "vpython_thread"
        threadlock.acquire()
        print("vpython: acquired")
        
    def run(self):
        import visual
        print("starting 3d display...")
        self.scene = visual.display()
        self.selection=-1
        self.servos = [visual.box(color=visual.color.green) for i in range(12)]
        self.curves = [visual.curve(radius=0.5) for i in range(len(self.servos))]
        self.endpoints = [visual.box(color=visual.color.cyan) for i in range(4)]
        self.mainbodycurves = [
            visual.curve(radius=0.5,color=visual.color.red) for i in range(4)]
        try:
            threadlock.release()
        except:
            print("error releasing")
        print("vpython: released")
        while 1:
            visual.rate(60)
            e = self.scene.mouse.getclick()
            select = self.scene.mouse.pick
            try:
                index = self.servos.index(select)
                self.selection = index
                for servo in self.servos:
                    servo.color = visual.color.green
                select.color = visual.color.red
            except ValueError:
                pass
            print(self.selection)
            

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
     
    def select(self, obj):
        obj.color = visual.color.red
        
