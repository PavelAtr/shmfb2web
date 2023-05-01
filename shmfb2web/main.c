#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "shmfb2web.h"

static int interrupted;

static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
    return 0;
}

void* request_user = NULL;
void* request_user2 = NULL;
char* receive = "";

static int callback_video( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
		case LWS_CALLBACK_RECEIVE:
			request_user = user;
			receive = lws_callback_recieve(wsi, reason, user, in, len);
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			if (user != request_user) return 0;
			lws_callback_server_writable(wsi, reason, user, in, len, receive);
			receive = "";
			break;

		case  LWS_CALLBACK_CLOSED:
			on_connection_close(user);
			break;

		default:
			break;
	}

	return 0;
}

/*static int callback_input( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
		case LWS_CALLBACK_RECEIVE:
			request_user2 = user;
			receive = lws_callback_recieve2(wsi, reason, user, in, len);
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			if (user != request_user2) return 0;
			lws_callback_server_writable2(wsi, reason, user, in, len, receive);
			receive = "";
			break;

		case  LWS_CALLBACK_CLOSED:
			on_connection_close(user);
			break;

		default:
			break;
	}

	return 0;
}*/

static const struct lws_protocol_vhost_options js_mime = {
    NULL,				/* "next" pvo linked-list */
    NULL,				/* "child" pvo linked-list */
    ".js",				/* file suffix to match */
    "text/javascript"		/* mimetype to use */
};

static struct lws_protocols protocols[] =
{
    /* The first protocol must always be the HTTP handler */
	{
	    "http-only",   /* name */
	    callback_http, /* callback */
	    1,             /* per session data. */
	    0,             /* max frame size / rx buffer */
	},
	{
		"video-protocol",
		callback_video,
		1,
		EXAMPLE_RX_BUFFER_BYTES,
	},
/*	{
		"input-protocol",
		callback_input,
		1,
		EXAMPLE_RX_BUFFER_BYTES,
	},*/
	{ NULL, NULL, 0, 0 } /* terminator */
};

struct lws_http_mount mount = {
        NULL,           /* linked-list "next" */
         "/",            /* mountpoint URL */
        PREFIX "/share/shmfb2web/", /* serve from dir */
        "display.html",   /* default filename */
        NULL,
        NULL,
        &js_mime,
        NULL,
        0,
        0,
        0,
        0,
        0,
        0,
        LWSMPRO_FILE,   /* files in a dir */
        1,              /* char count */
        NULL,
};

void sigint_handler(int sig)
{
        interrupted = 1;
}

int main(int argc, const char **argv)
{
        struct lws_context_creation_info info;
        struct lws_context *context;
        const char *p;

	if (parseopts(argc, (char**)argv) == -1) return -1;
        int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_CLIENT | LLL_LATENCY
                        /* for LLL_ verbosity above NOTICE to be built into lws,
                         * lws must have been configured and built with
                         * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
                        /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
                        /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
                        /* | LLL_DEBUG */;

        lws_set_log_level(logs, NULL);

	if (initclients() == -1)
	    return -1;

	if (dbusdest != NULL)
	    if (dbusinit() == -1)
		return -1;

        signal(SIGINT, sigint_handler);

        if ((p = lws_cmdline_option(argc, argv, "-d")))
                logs = atoi(p);


        lwsl_user("Framebuffer http server | visit http://localhost:%d\n", port);

	mount.basic_auth_login_file = authfile;
        memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
        info.port = port;
	info.protocols = protocols;
        info.mounts = &mount;
//	info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

        context = lws_create_context(&info);
        if (!context) {
                lwsl_err("lws init failed\n");
                return 1;
        }

        while (n >= 0 && !interrupted)
	{
                n = lws_service(context, 0);
//		usleep(2000);
	}

        lws_context_destroy(context);

        return 0;
}

