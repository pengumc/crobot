#author: Michiel van der Coelen
#date 2012-03-09
#tabsize 4

NAME = crobot
#change bits according to your system
BITS=64
CFLAGS = -Iinclude -Isrc -Iinclude/$(NAME) -Iinclude/gsl$(BITS) -I. -std=c99
#USBLIBS = $(shell libusb-config --libs)
USBLIBS = -lusb
CLIBS = -Llib/gsl$(BITS) -Llib/crobot -lgslcblas -lgsl $(USBLIBS) -lm
CC = gcc
OUTPUTNAME = $(NAME)
LIBUSB_OBJECTS = opendevice.o
OBJECTS = $(LIBUSB_OBJECTS) Report.o Usbdevice.o Leg.o Solver.o Servo.o rotation.o Angle.o main.o

.PHONY:all clean

all:bin/$(OUTPUTNAME)

bin/$(OUTPUTNAME):$(OBJECTS)
	$(CC) -o bin/$(OUTPUTNAME) $(addprefix lib/$(NAME)/, $(OBJECTS)) $(CLIBS)

%.o:src/%.c
	$(CC) $(CFLAGS) -c $< -o lib/$(NAME)/$@




clean:
	rm lib/$(NAME)/*.o
