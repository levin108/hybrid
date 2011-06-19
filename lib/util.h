#ifndef HYBIRD_UTIL_H
#define HYBIRD_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib/gi18n.h>
#include <locale.h>

/* network headers */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "debug.h"

#define HYBIRD_OK    0
#define HYBIRD_ERROR 1

#define PATH_LENGTH 1024
#define URL_LENGTH 1024
#define BUF_LENGTH 4096

#endif
