#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#define _GNU_SOURCE
#include <string.h>
#include <zlib.h>
#include "shmfb2web.h"

char *strchrnul(const char *s, int c);

#define MAXCLIENTS	8

struct client* clients[MAXCLIENTS];
int shmfd;
unsigned int* shmmem;
unsigned int* tmpbuffer;
unsigned int* dummy;

int initclients() {
//    clients = malloc(sizeof(struct client*) * MAXCLIENTS);
    for (int i = 0; i < MAXCLIENTS; i++)
	clients[i] = NULL;
    if(verbose) lwsl_user("initclients\n");
    int shmfd = shm_open(shmfb, O_RDONLY, S_IRWXU);
    if (shmfd < 0) {
	lwsl_err("Error shm_open(%s)\n", shmfb);
	return -1;
    }
    shmmem = mmap(NULL, width * height * sizeof(unsigned int), PROT_READ, MAP_SHARED, shmfd, 0);

    if (shmmem == MAP_FAILED) {
	lwsl_err("Error mmap %s failed\n", shmfb);
	return -1;
    }
    tmpbuffer = malloc(width * height * sizeof(unsigned int));
    dummy = malloc(sizeof(unsigned int));
    dummy[0] = 0;

    return 0;
}

int getclient(void* user) {
    for (int i = 0; i < MAXCLIENTS; i++)
    {
	if (clients[i] != NULL) {
	    if (clients[i]->user == user) {
		return i;
	    }
	}
    }
    return -1;
}

struct client* clientdraw(void* user) {


    int i = getclient(user);
    if (i == -1) {
	dummy[0] = shmmem[0];
	return NULL;
    }

    unsigned int X1 = width;
    unsigned int Y1 = height;
    unsigned int X2 = 0;
    unsigned int Y2 = 0;


	for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
	    int ind = y * width + x;
	    tmpbuffer[ind] = shmmem[ind];
	}
    }


    for (int y = 0; y < height; y++) {
	for (int x = 0; x < width; x++) {
	    int ind = y * width + x;
	    if (tmpbuffer[ind] != clients[i]->snapshot[ind]) {
		if (x <= X1) X1 = x;
		if (x >= X2) X2 = x;
		if (y <= Y1) Y1 = y;
		if (y >= Y2) Y2 = y;
		clients[i]->snapshot[ind] = tmpbuffer[ind];
	    }
	}
    }

    if (X1 == width && X2 == 0 && Y1 == height && Y2 == 0)
    {
        clients[i]->X1 = 0;
        clients[i]->Y1 = 0;
        clients[i]->X2 = 0;
        clients[i]->Y2 = 0;
	clients[i]->message[0] = shmmem[0];
    } else {
        clients[i]->X1 = X1;
        clients[i]->Y1 = Y1;
        clients[i]->X2 = X2;
        clients[i]->Y2 = Y2;
    }

    int c = 0;
    for (int y = Y1; y <= Y2; y++) {
	for (int x = X1; x <= X2; x++) {
	    int ind = y * width + x;
	    clients[i]->message[c] = clients[i]->snapshot[ind];
	    c++;
	}
    }

    unsigned long d;
    compress((Bytef *)clients[i]->compressed, &d, (const Bytef *)clients[i]->message, 
    (clients[i]->X2 - clients[i]->X1 + 1) * (clients[i]->Y2 - clients[i]->Y1 + 1) * sizeof(unsigned int));
    clients[i]->compsize = d;

    if (verbose) lwsl_user("clientdraw %u %u %u %u z=%lu\n", clients[i]->X1, clients[i]->Y1, clients[i]->X2, clients[i]->Y2, d);

    return clients[i];
}

struct client* getdata(void* user) {
    int i = getclient(user);
    if (i == -1) {
	dummy[0] = shmmem[0];
	return NULL;
    }

    if (verbose) lwsl_user("getdata %u %u %u %u\n", clients[i]->X1, clients[i]->Y1, clients[i]->X2, clients[i]->Y2);
    return clients[i];
}

int newclient(void* user) {
    for (int i = 0; i < MAXCLIENTS; i++)
    {

	if (clients[i] == NULL) {
	    clients[i] = malloc(sizeof(struct client));
	    clients[i]->user = user;
	    clients[i]->snapshot = malloc(width * height * sizeof(unsigned int));
	    clients[i]->message = malloc(width * height * sizeof(unsigned int));
	    clients[i]->compressed = malloc(width * height * sizeof(unsigned int) * 2);
	    clients[i]->X1 = 0;
	    clients[i]->Y1 = 0;
	    clients[i]->X2 = 0;
	    clients[i]->Y2 = 0;
	    for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
		    clients[i]->snapshot[x * y] = 0;
		    clients[i]->message[x * y] = 0;
		}
	    }
	    if (verbose) lwsl_user("newclient\n");
	    return i;
	}
    }
    return -1;
}


int on_connection_open(void* user) {
    if (newclient(user) == -1) return -1;
    if (verbose) lwsl_user("on_connection_open\n");
    return 0;
}

int clearbuffers(void* user) {
    int i = getclient(user);
    if (i == -1) return -1;
//    if (clients[i]->snapshot != NULL) free(clients[i]->snapshot);
//    if (clients[i]->message != NULL) free(clients[i]->message);
//    if (clients[i] != NULL) free(clients[i]);
    clients[i] = NULL;
    if (verbose) lwsl_user("clearbuffers\n");
    return 0;
}

int on_connection_close(void* user) {
    clearbuffers(user);
    if (verbose) lwsl_user("on_connection_close\n");
    return 0;
}
