#ifndef HYBRID_UTIL_H
#define HYBRID_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib/gi18n.h>
#include <locale.h>

/* network headers */
#include <sys/types.h>
#include <sys/socket.h>
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
#ifdef __cplusplus
}
#endif

#endif
