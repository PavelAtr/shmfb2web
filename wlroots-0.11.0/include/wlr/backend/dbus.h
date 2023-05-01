/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_BACKEND_DBUS_H
#define WLR_BACKEND_DBUS_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_input_device.h>
#include <backend/dbus.h>

struct wlr_backend *wlr_dbus_backend_create(struct wl_display *display);
int handle_dbus_event(struct wlr_dbus_backend* dbus_backend, char* sigstring);
int pointer_left_down(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2);
int pointer_left_up(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2);
int pointer_right_down(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2);
int pointer_right_up(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2);
int pointer_position(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2);
int keyboard_keydown(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2);
int keyboard_keyup(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2);
int asci2scan(char* key);

#endif
