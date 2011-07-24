#ifndef HYBRID_BUDDYREQ_H
#define HYBRID_BUDDYREQ_H
#include <glib.h>

typedef struct _BuddyReqWindow BuddyReqWindow;

#include "account.h"

struct _BuddyReqWindow {
	gchar *buddy_id;
	gchar *buddy_name;

	HybridAccount *account;

	GtkWidget *window;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an add-buddy request window.
 *
 * @param account The account context.
 * @param id      ID of the buddy who sent the request.
 * @param name    Name of the buddy who sent the request. [allow-none]
 *
 * @return The request window created.
 */
BuddyReqWindow *hybrid_buddy_request_window_create(HybridAccount *account, 
		const gchar *id, const gchar *name);

#ifdef __cplusplus
}
#endif

#endif
