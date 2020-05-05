logdraw: log.o gtkUI.o
	gcc `pkg-config --cflags gtk+-3.0` -g -Wall -o logdraw logdraw.o gtkUI.o `pkg-config --libs gtk+-3.0`

log.o: logdraw.c gtkUI.h
	gcc `pkg-config --cflags gtk+-3.0` -g -Wall -c logdraw.c `pkg-config --libs gtk+-3.0` -O0 -I.

gtkUI.o: gtkUI.c gtkUI.h
	gcc `pkg-config --cflags gtk+-3.0` -g -Wall -c gtkUI.c `pkg-config --libs gtk+-3.0`
