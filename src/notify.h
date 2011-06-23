#ifndef HYBRID_NOTIFY_H
#define HYBRID_NOTIFY_H

#include <gtk/gtk.h>
#include "account.h"

typedef struct _HybridNotify HybridNotify;

struct _HybridNotify {
	GtkWidget *window;
	GtkWidget *cellview;
	GtkTreeIter iter;
	GtkWidget *textview;
	GtkWidget *action_area;
	HybridAccount *account;
};

enum {
	NOTIFY_ICON_COLUMN,
	NOTIFY_NAME_COLUMN,
	NOTIFY_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a notification box.
 *
 * @param title   The title of the box, if NULL, then set it
 *                to default _("Notification")
 * @param account The account which give out a notification.
 *
 * @return The notification box created.
 */
HybridNotify *hybrid_notify_create(HybridAccount *account, const gchar *title);

/**
 * Set the text to displayed in the text area.
 *
 * @param notify The notification box.
 * @param text   The notification text to set.
 */
void hybrid_notify_set_text(HybridNotify *notify, const gchar *text);

/**
 * Set the name of the account releted to the notification.
 *
 * @param notify The notification box.
 * @param name   The account name.
 */
void hybrid_notify_set_name(HybridNotify *notify, const gchar *name);

#ifdef __cplusplus
}
#endif

#endif
