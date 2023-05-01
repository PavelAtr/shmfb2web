#include <stdlib.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "shmfb2web.h"

DBusConnection *conn;

int dbusinit()
{

    DBusError err;
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if(!conn) {
        lwsl_err("DBus error %s: %s\n", err.name, err.message);
        return -1;
    }
    char myname[256];
    sprintf(myname, "ru.tinysystem.shmfb2web-%d", port);
    dbus_bus_request_name(conn, myname, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if(dbus_error_is_set(&err)){
        lwsl_err("DBus error %s: %s\n", err.name, err.message);
        dbus_connection_close(conn);
        return -1;
    }
    return 0;
}

int dbussend(const char* str)
{
    DBusMessage *msg;
    msg = dbus_message_new_signal("/ru/tinysystem/shmfb2web", dbusdest, "inputmsg");
    if(msg == NULL){
        fprintf(stderr, "Message is null!\n");
        return(1);
    }
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
    if(!dbus_connection_send(conn, msg, NULL)) lwsl_err("Error sending dbus message!\n");
    dbus_message_unref(msg);
    return 0;
}
