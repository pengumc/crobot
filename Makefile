#author: Michiel van der Coelen
#date 2012-03-09
#tabsize 4

NAME = crobot
#change bits according to your system
BITS=64
#c99 std prevents usleep?
CFLAGS = -fPIC -Iinclude -Isrc -Iinclude/$(NAME) -Iinclude/gsl$(BITS) -I. -std=c99
#USBLIBS = $(shell libusb-config --libs)
USBLIBS = -lusb
CLIBS = -Llib/gsl$(BITS) -Llib/crobot -lgslcblas -lgsl $(USBLIBS) -lm
CC = gcc
OUTPUTNAME = $(NAME)
LIBUSB_OBJECTS = opendevice.o
OBJECTS = $(LIBUSB_OBJECTS) Quadruped.o Accelerometer.o Filter.o Pscontroller.o Report.o Usbdevice.o Leg.o Solver.o Servo.o rotation.o Angle.o main.o
UNAME:= $(shell uname -s)

.PHONY:all clean library

all:bin/$(OUTPUTNAME)

bin/$(OUTPUTNAME):$(OBJECTS)
	$(CC) -o bin/$(OUTPUTNAME) $(addprefix lib/$(NAME)/, $(OBJECTS)) $(CLIBS)

%.o:src/%.c
	$(CC) $(CFLAGS) -c $< -o lib/$(NAME)/$@

clean:
	rm lib/$(NAME)/*.o

library:lib/lib$(OUTPUTNAME).so.1.0.1

lib/lib$(OUTPUTNAME).so.1.0.1:$(OBJECTS)
ifeq ($(UNAME),Linux)
	$(CC) $(CLIBS) -shared -Wl,-soname,lib$(OUTPUTNAME).so.1 -o lib/lib$(OUTPUTNAME).so.1.0.1 $(addprefix lib/$(NAME)/, $(OBJECTS))
endif
ifeq ($(UNAME),MINGW32_NT-6.1)
	$(CC) -shared -Wall -o lib/lib$(OUTPUTNAME).dll $(addprefix lib/$(NAME)/, $(OBJECTS)) $(CLIBS)
endif
