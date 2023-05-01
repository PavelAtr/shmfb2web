#ifndef BACKEND_DBUS_H
#define BACKEND_DBUS_H

#include <wayland-server-core.h>
#include <wlr/backend/interface.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>

#include <dbus/dbus.h>

struct wlr_dbus_backend {
	struct wlr_backend backend;
	struct wl_display *display;
	struct wl_event_source *input_event;
	struct wl_event_source *input_event_timer;
	DBusConnection* conn;
	struct wlr_keyboard keyboard;
	struct wlr_input_device keyboard_dev;
	struct wlr_pointer pointer;
	struct wlr_input_device pointer_dev;
	int width;
	int height;
};

#endif
