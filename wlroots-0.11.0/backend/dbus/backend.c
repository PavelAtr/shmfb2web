#include <assert.h>
#include <stdlib.h>
#include <wlr/backend/interface.h>
#include <wlr/backend/session.h>
#include <wlr/util/log.h>
#include <util/signal.h>
#include "backend/dbus.h"
#include "wlr/backend/dbus.h"
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_input_device.h>


static struct wlr_dbus_backend *get_dbus_backend_from_backend(
                struct wlr_backend *wlr_backend) {
        return (struct wlr_dbus_backend *)wlr_backend;
}


static void handle_dbus(void *data) {

    struct wlr_dbus_backend* backend = data;
    char* sigvalue;
    DBusMessage * msg;
    DBusMessageIter args;
  
    while ( 1 ) {
	dbus_connection_read_write( backend->conn, 0 );
	msg = dbus_connection_pop_message( backend->conn );
	if ( NULL == msg ) {
	    break;
	}
	if ( dbus_message_iter_init( msg, &args ) &&  DBUS_TYPE_STRING == dbus_message_iter_get_arg_type( &args ) ) {
	    dbus_message_iter_get_basic( &args, &sigvalue );
	    handle_dbus_event(backend, sigvalue);
//	    wlr_log(WLR_ERROR, "%s", sigvalue);
	}
	dbus_message_unref( msg );
    }
    return;
}

static int dbus_timer(void * data)
{
    struct wlr_dbus_backend* backend = data;
    wl_event_source_timer_update(backend->input_event_timer, 50);
    struct wl_event_loop *event_loop =
        wl_display_get_event_loop(backend->display);
    backend->input_event = wl_event_loop_add_idle(event_loop, handle_dbus, backend);
    return 0;
}



static void input_device_destroy(struct wlr_input_device *wlr_device) {
        // Don't free the input device, it's on the stack
}

const struct wlr_input_device_impl dbus_input_device_impl = {
        .destroy = input_device_destroy,
};

static void keyboard_destroy(struct wlr_keyboard *wlr_keyboard) {
        // Don't free the keyboard, it's on the stack
}

const struct wlr_keyboard_impl dbus_keyboard_impl = {
        .destroy = keyboard_destroy,
};

static void pointer_destroy(struct wlr_pointer *wlr_pointer) {
        // Don't free the pointer, it's on the stack
}

const struct wlr_pointer_impl dbus_pointer_impl = {
        .destroy = pointer_destroy,
};

static bool backend_start(struct wlr_backend *wlr_backend) {
        struct wlr_dbus_backend *backend =
                get_dbus_backend_from_backend(wlr_backend);
        wlr_log(WLR_DEBUG, "Initializing dbus");

	DBusError err;
	dbus_error_init( &err );
	backend->conn = dbus_bus_get( DBUS_BUS_SESSION, &err );
	if ( dbus_error_is_set( &err ) ) {
	    wlr_log(WLR_ERROR, "Failed to create dbus connection");
            return false;
	}
	if ( backend->conn == NULL ) {
	    return false;
	}

	char* match = getenv("WLR_DBUS_MATCH");
	char* dbus_match = (match == NULL)? "type='signal',interface='org.desktop.wlroots'": match;

	dbus_bus_add_match( backend->conn,
                      dbus_match,
                      &err );
	dbus_connection_flush( backend->conn );
	if ( dbus_error_is_set( &err ) ) {
	    wlr_log(WLR_ERROR, "Failed to create dbus match");
            return false;
	}

	char* width = getenv("WLR_DBUS_WIDTH");
	backend->width = (width == NULL)? 1024 : atoi(width);
	char* height = getenv("WLR_DBUS_HEIGHT");
	backend->height = (height == NULL)? 768 : atoi(height);

        wlr_input_device_init(&backend->keyboard_dev, WLR_INPUT_DEVICE_KEYBOARD,
                &dbus_input_device_impl, "Dbus keyboard", 0, 0);
        wlr_keyboard_init(&backend->keyboard, &dbus_keyboard_impl);
        backend->keyboard_dev.keyboard = &backend->keyboard;
	wlr_log(WLR_DEBUG, "Dbus keyboard initialized");
        wlr_input_device_init(&backend->pointer_dev, WLR_INPUT_DEVICE_POINTER,
                &dbus_input_device_impl, "Dbus pointer", 0, 0);
        wlr_pointer_init(&backend->pointer, &dbus_pointer_impl);
        backend->pointer_dev.pointer = &backend->pointer;
	wlr_log(WLR_DEBUG, "Dbus pointer initialized");

	wlr_signal_emit_safe(&backend->backend.events.new_input, &backend->pointer_dev);
	wlr_signal_emit_safe(&backend->backend.events.new_input, &backend->keyboard_dev);

	struct wl_event_loop *event_loop =
                wl_display_get_event_loop(backend->display);
        backend->input_event_timer = wl_event_loop_add_timer(event_loop, dbus_timer, backend);
        if (!backend->input_event_timer) {
                wlr_log(WLR_ERROR, "Failed to create dbus event on event loop");
                return false;
        }
	wl_event_source_timer_update(backend->input_event_timer, 50);
        wlr_log(WLR_DEBUG, "dbus successfully initialized, match=%s", dbus_match);

        return true;
}

static void backend_destroy(struct wlr_backend *wlr_backend) {
        if (!wlr_backend) {
                return;
        }
//        struct wlr_dbus_backend *backend =
//                get_dbus_backend_from_backend(wlr_backend);
}

static const struct wlr_backend_impl backend_impl = {
        .start = backend_start,
        .destroy = backend_destroy,
};

struct wlr_backend *wlr_dbus_backend_create(struct wl_display *display) {
        struct wlr_dbus_backend *backend =
                calloc(1, sizeof(struct wlr_dbus_backend));
        if (!backend) {
                wlr_log(WLR_ERROR, "Allocation failed: %s", strerror(errno));
                return NULL;
        }
	backend->display = display;
	wlr_backend_init(&backend->backend, &backend_impl);

	return &backend->backend;
}
