#include <libwebsockets.h>

extern int port;

extern char* shmfb;

extern char* dbusdest;

extern char* authfile;

extern int width;

extern int height;

extern int delay;

extern int verbose;

extern unsigned int* dummy;

int parseopts(int argc, char *argv[]);

struct client {
    void* user;
    unsigned int X1;
    unsigned int Y1;
    unsigned int X2;
    unsigned int Y2;
    unsigned int* snapshot;
    unsigned int* message;
    unsigned int* compressed;
    unsigned long compsize;
};

int on_connection_open(void* user);

int on_connection_close(void* user);

struct client* clientdraw(void* user);

struct client* getdata(void* user);

int initclients();

char* lws_callback_recieve(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

int lws_callback_server_writable(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len, char* receivestr);

char* lws_callback_recieve2(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

int lws_callback_server_writable2(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len, char* receivestr);

#define EXAMPLE_RX_BUFFER_BYTES 256

int dbusinit();

int dbussend(const char* str);

