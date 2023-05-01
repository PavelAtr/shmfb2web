#include <stdlib.h>
#include <wlr/util/log.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include "wlr/backend/dbus.h"

int keyboard_keydown(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2)
{
    struct wlr_event_keyboard_key wlr_event = { 0 };
    wlr_event.time_msec = atoi(time);
    wlr_event.keycode = asci2scan(param1);
    wlr_event.state = WLR_KEY_PRESSED;
    wlr_event.update_state = true;
    wlr_keyboard_notify_key(&dbus_backend->keyboard, &wlr_event);
    wlr_log(WLR_DEBUG, "keyboard_keydown %s %s %s %s", time, id, param1, param2);
    return 0;
}

int keyboard_keyup(struct wlr_dbus_backend* dbus_backend, char* time, char* id, char* param1, char* param2)
{
    struct wlr_event_keyboard_key wlr_event = { 0 };
    wlr_event.time_msec =atoi(time);
    wlr_event.keycode = asci2scan(param1);
    wlr_event.state = WLR_KEY_RELEASED;
    wlr_event.update_state = true;
    wlr_keyboard_notify_key(&dbus_backend->keyboard, &wlr_event);
    wlr_log(WLR_DEBUG, "keyboard_keyup %s %s %s %s", time, id, param1, param2);
    return 0;
}

