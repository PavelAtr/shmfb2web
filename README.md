# shmfb2web
Lightweight web server for shm framebuffer

shmfb2web is intended for translating shm framebuffer in 8888/rgba format to the browser. The pointer and keyboard events are passed as a string to stdout or dbus to the destination.
The implementation is very simple and compact.
Together with wlroots (added fbdev and dbus backends) allows wlroots based composers to render in the browser. The fbdev backend code is based on https://wiki.postmarketos.org/index.php?title=User:Ollieparanoid/Run_wlroots_with_fbdev&mobileaction=toggle_view_desktop.

# Assembly:
cd shmfb2web

PREFIX=/usr make

PREFIX=/usr make install

# Simple start:

dd if=/dev/urandom bs=8192 count=768 > /dev/shm/shmfb

shmfb2wev

firefox http://127.0.0.1:8000


# Launch with sway:

LANG=C WLR_BACKENDS="fbdev,dbus" WLR_SHMDEV="/shmfb" WLR_DBUS_MATCH="type='signal',interface='org.desktop.wlroots'" sway >/dev/null 2>&1 &

sleep 1

shmfb2web -c org.desktop.wlroots >/dev/null 2>&1 &

firefox http://127.0.0.1:8000


# New environment variables added to wlroots

fbdev backend:

WLR_SHMDEV e.g. /shmfb (creates /dev/shm/shmfb)

WLR_SHM_WIDTH default 1024

WLR_SHM_HEIGHT default 768

WLR_FBDEVS e.g. /dev/fb0,/dev/fb1


dbus backend:

WLR_DBUS_MATCH e.g. type='signal',interface='org.desktop.wlroots'
