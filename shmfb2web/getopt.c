#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "libwebsockets.h"

int port = 8000;
char* shmfb = "/shmfb";
char* authfile = NULL;
char* dbusdest = NULL;
int width = 1024;
int height = 768;
int delay = 50;
int verbose = 0;

int parseopts(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "a:f:p:W:H:d:i:c:vh")) != -1) {
        switch (opt) {
        case 'f':
            shmfb = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'W':
            width = atoi(optarg);
            break;
        case 'H':
            height = atoi(optarg);
            break;
        case 'd':
            delay = atoi(optarg);
            break;
        case 'c':
            dbusdest = optarg;
            break;
        case 'a':
            authfile = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
	default:
 lwsl_err("\nUsage:\n\
 [-v] verbose\n\
 [-a authbasic_file] path\n\
 [-p port] (def. 8000)\n\
 [-f shmfb] framebuffer (def. /shmfb)\n\
 [-W width] width (def. 1024)\n\
 [-H height] height (def. 768)\n\
 [-d delay] refresh delay in ms (def. 50)\n\
 [-c dbusdest] dbus destination for input events (ex.: org.desktop.wlroots) \n");
            return -1;
        }
    }
    return 0;
}
