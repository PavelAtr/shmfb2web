#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>
#include "shmfb2web.h"
#include <sys/time.h>
#include <sys/sysinfo.h>

struct payload
{
        unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + EXAMPLE_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
        size_t len;
} received_payload;

char receivestr[256];

char* lws_callback_recieve(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {


    memcpy( &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], in, len );
    received_payload.len = len;
    memcpy(receivestr, &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], received_payload.len);
    receivestr[received_payload.len] = '\0';
    lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );

    return receivestr;
}

int lws_callback_server_writable(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len, char* receivestr) {
    unsigned char transmitstr[256];

    if (strcmp(receivestr, "draw") == 0)
    {
        struct client* buf =  clientdraw(user);
        if (buf == NULL) {
            sprintf((char*)transmitstr, "cmddraw %d %d %d %d", 0, 0, 0, 0);
            lws_write( wsi, transmitstr, strlen((char*)transmitstr), LWS_WRITE_TEXT);
        } else {
            sprintf((char*)transmitstr, "cmddraw %d %d %d %d", buf->X1, buf->Y1, buf->X2, buf->Y2);
            lws_write( wsi, transmitstr, strlen((char*)transmitstr), LWS_WRITE_TEXT);
        }
    }

    if (strcmp(receivestr, "getdata") == 0)
    {
        struct client* buf =  getdata(user);
        if (buf == NULL) {
            lws_write( wsi, (unsigned char*)dummy, sizeof(unsigned int), LWS_WRITE_BINARY);
        } else {
            lws_write( wsi, (unsigned char*)buf->compressed, buf->compsize, LWS_WRITE_BINARY);
        }
    }

    char str[512];
    struct timeval tv;
    struct timezone tz;

    if(strstr(receivestr, "input: ") != NULL)
    {

	gettimeofday(&tv,&tz);
	struct sysinfo info;
	sysinfo(&info);

	sprintf(str, "%lu %s", info.uptime * 1000 + tv.tv_usec / 1000, receivestr);

        if (dbusdest == NULL)
            lwsl_user("%s\n", str);
	else
	    dbussend(str);
    }

    if (strcmp(receivestr, "config") == 0)
    {
        if (on_connection_open(user) == -1) lwsl_err("problem connection open\n");
        sprintf((char*)transmitstr, "delay %d", delay);
        if (verbose) lwsl_user("%s\n", transmitstr);
        lws_write( wsi, transmitstr, strlen((char*)transmitstr), LWS_WRITE_TEXT );
    }

    return 0;
}
