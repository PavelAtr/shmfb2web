PREFIX ?= /usr/local

all: shmfb2web

main:
	$(CC) $(CFLAGS) -DPREFIX=\"$(PREFIX)\" -c -g -Wall main.c -o main.o

shmfb:
	$(CC) $(CFLAGS) -c -Wall shmfb.c -o shmfb.o `pkg-config libwebsockets --cflags` `pkg-config zlib --cflags`

getopt:
	$(CC) $(CFLAGS) -c -Wall getopt.c -o getopt.o

dbus:
	$(CC) $(CFLAGS) -c -Wall dbus.c -o dbus.o `pkg-config dbus-1 --cflags`

websockets:
	$(CC) $(CFLAGS) -c -Wall websockets.c -o websockets.o `pkg-config libwebsockets --cflags`

example:
	$(CC) $(CFLAGS) -Wall dbus-listener-example.c -o dbus-listener-example `pkg-config dbus-1 --cflags` `pkg-config dbus-1 --libs`


shmfb2web: main shmfb getopt dbus websockets
	$(CC) -o shmfb2web  \
	    main.o shmfb.o \
	    getopt.o \
	    dbus.o \
	    websockets.o \
	    `pkg-config libwebsockets --libs` \
	    `pkg-config zlib --libs` \
	    `pkg-config dbus-1 --libs` \
	    -lrt \
	    $(LDFLAGS)

install:
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	mkdir -p $(DESTDIR)/$(PREFIX)/share/shmfb2web
	install shmfb2web $(DESTDIR)/$(PREFIX)/bin/
	install -m 644 html/display.html $(DESTDIR)/$(PREFIX)/share/shmfb2web/
	install -m 644 html/favicon.ico $(DESTDIR)/$(PREFIX)/share/shmfb2web/

clean:
	rm -f shmfb2web
	rm -f shmfb.o
	rm -f main.o
	rm -f getopt.o
	rm -f dbus.o
	rm -f websockets.o

