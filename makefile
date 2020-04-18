logdraw: log.o gtkUI.o
	gcc `pkg-config --cflags gtk+-3.0` -g -Wall -o logdraw logdraw.o gtkUI.o `pkg-config --libs gtk+-3.0`

log.o: logdraw.c gtkUI.h
	gcc `pkg-config --cflags gtk+-3.0` -c logdraw.c `pkg-config --libs gtk+-3.0` -Wall -g -O0 -I.

gtkUI.o: gtkUI.c gtkUI.h
	gcc `pkg-config --cflags gtk+-3.0` -c gtkUI.c `pkg-config --libs gtk+-3.0`
