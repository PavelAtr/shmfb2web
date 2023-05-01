#include <stdlib.h>
#include <wlr/util/log.h>
#include <util/signal.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_input_device.h>
#include "wlr/backend/dbus.h"
#include <linux/input-event-codes.h>

int pointer_position(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2)
{
    struct wlr_event_pointer_motion_absolute wlr_event = { 0 };
    wlr_event.device = &dbus_backend->pointer_dev;
    wlr_event.time_msec = atoi(time);
    wlr_event.x = atof(param1) / dbus_backend->width;
    wlr_event.y = atof(param2) / dbus_backend->height;
    wlr_signal_emit_safe(&dbus_backend->pointer.events.motion_absolute, &wlr_event);
    wlr_signal_emit_safe(&dbus_backend->pointer.events.frame, &dbus_backend->pointer);
    wlr_log(WLR_DEBUG, "pointer_position %s %s %s %s", time, id, param1, param2);
    return 0;
}

int pointer_left_down(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2)
{
    pointer_position(dbus_backend, time, id, param1, param2);
    struct wlr_event_pointer_button wlr_event = { 0 };
    wlr_event.device = &dbus_backend->pointer_dev;
    wlr_event.time_msec = atoi(time);
    wlr_event.button = BTN_LEFT;
    wlr_event.state = WLR_BUTTON_PRESSED;
    wlr_signal_emit_safe(&dbus_backend->pointer.events.button, &wlr_event);
    wlr_signal_emit_safe(&dbus_backend->pointer.events.frame, &dbus_backend->pointer);
    wlr_log(WLR_DEBUG, "pointer_left_down %s %s %s %s", time, id, param1, param2);
    return 0;
}

int pointer_left_up(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2)
{
    pointer_position(dbus_backend, time, id, param1, param2);
    struct wlr_event_pointer_button wlr_event = { 0 };
    wlr_event.device = &dbus_backend->pointer_dev;
    wlr_event.time_msec = atoi(time);
    wlr_event.button = BTN_LEFT;
    wlr_event.state = WLR_BUTTON_RELEASED;
    wlr_signal_emit_safe(&dbus_backend->pointer.events.button, &wlr_event);
    wlr_signal_emit_safe(&dbus_backend->pointer.events.frame, &dbus_backend->pointer);
    wlr_log(WLR_DEBUG, "pointer_left_up %s %s %s %s", time, id, param1, param2);
    return 0;
}

int pointer_right_down(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2)
{
    pointer_position(dbus_backend, time, id, param1, param2);
    struct wlr_event_pointer_button wlr_event = { 0 };
    wlr_event.device = &dbus_backend->pointer_dev;
    wlr_event.time_msec = atoi(time);
    wlr_event.button = BTN_RIGHT;
    wlr_event.state = WLR_BUTTON_PRESSED;
    wlr_signal_emit_safe(&dbus_backend->pointer.events.button, &wlr_event);
    wlr_signal_emit_safe(&dbus_backend->pointer.events.frame, &dbus_backend->pointer);
    wlr_log(WLR_DEBUG, "pointer_right_down %s %s %s %s", time, id, param1, param2);
    return 0;
}

int pointer_right_up(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2)
{
    pointer_position(dbus_backend, time, id, param1, param2);
    struct wlr_event_pointer_button wlr_event = { 0 };
    wlr_event.device = &dbus_backend->pointer_dev;
    wlr_event.time_msec = atoi(time);
    wlr_event.button = BTN_RIGHT;
    wlr_event.state = WLR_BUTTON_RELEASED;
    wlr_signal_emit_safe(&dbus_backend->pointer.events.button, &wlr_event);
    wlr_signal_emit_safe(&dbus_backend->pointer.events.frame, &dbus_backend->pointer);
    wlr_log(WLR_DEBUG, "pointer_right_up %s %s %s %s", time, id, param1, param2);
    return 0;
}

