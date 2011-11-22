/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#ifndef HYBRID_UTIL_H
#define HYBRID_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <config.h>

#include <sys/stat.h>

/* network headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <glib.h>

#include "debug.h"

#define HYBRID_OK    0
#define HYBRID_ERROR 1

#define PATH_LENGTH 1024
#define URL_LENGTH 1024
#define BUF_LENGTH 4096

typedef struct _HybridStack HybridStack;
typedef struct _HybridStackNode HybridStackNode;

struct _HybridStack {
	HybridStackNode *head;
};

struct _HybridStackNode {
	gpointer *data;
	HybridStackNode *next;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new stack.
 *
 * @return The stack created.
 */
HybridStack *hybrid_stack_create();

/**
 * Push a new element into the stack.
 *
 * @param stack The stack.
 * @param data  The element.
 */
void hybrid_stack_push(HybridStack *stack, gpointer data);

/**
 * Pup out the head element from the stack.
 *
 * @param stack The stack.
 *
 * @return The element node.
 */
gpointer hybrid_stack_pop(HybridStack *stack);


/**
 * Check whether the stack is empty.
 *
 * @param stack The stack.
 *
 * @return TRUE if empty, FALSE if not empty.
 */
gboolean hybrid_stack_empty(HybridStack *stack);

/**
 * Strip the html tags from the string, We check the html tags'
 * validaty, if invalid, just return the input html string.
 *
 * @html The string that may contains html tags.
 *
 * @return The string with no html tags in it, should be freed
 *         with g_free() when no longer needed.
 */
gchar *hybrid_strip_html(const gchar *html);

/**
 * Calculate SHA1 for the input string.
 *
 * @param in   The input string.
 * @param size Size of the input string.
 *
 * @param The SHA1 result.
 */
gchar *hybrid_sha1(const gchar *in, gint size);

/**
 * Base64 encode the input string.
 *
 * @param input The input string.
 * @param size  Size of the input string.
 *
 * @param Base64 encode result.
 */
gchar *hybrid_base64_encode(const guchar *input, gint size);

/**
 * Base64 decode the input string.
 *
 * @param input The input string to decode.
 * @param size  The size of the output string.
 *
 * @param Base64 decode result.
 */
guchar *hybrid_base64_decode(const gchar *input, gint *size);

#ifdef __cplusplus
}
#endif

#endif
