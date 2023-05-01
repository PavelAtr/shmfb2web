#include <stdio.h>
#include "wlr/backend/dbus.h"
#include <string.h>

int handle_dbus_event(struct wlr_dbus_backend* dbus_backend, char* sigstring)
{
    char time[32];
    char device[32];
    char action[32];
    char id[16];
    char param1[32];
    char param2[32];
    sscanf(sigstring, "%s input: %s %s %s %s %s", time, device, action, id, param1, param2);
    if (strcmp(device, "keyboard") == 0)
    {
	if (strcmp(action, "keydown") == 0) keyboard_keydown(dbus_backend, time, id, param1, param2);
	if (strcmp(action, "keyup") == 0) keyboard_keyup(dbus_backend, time, id, param1, param2);
    }

    if (strcmp(device, "pointer") == 0)
    {
	if (strcmp(action, "downleft") == 0) pointer_left_down(dbus_backend, time, id, param1, param2);
	if (strcmp(action, "downright") == 0) pointer_right_down(dbus_backend, time, id, param1, param2);
	if (strcmp(action, "upleft") == 0) pointer_left_up(dbus_backend, time, id, param1, param2);
	if (strcmp(action, "upright") == 0) pointer_right_up(dbus_backend, time, id, param1, param2);
	if (strcmp(action, "move") == 0) pointer_position(dbus_backend, time, id, param1, param2);
    }

    return 0;
}
