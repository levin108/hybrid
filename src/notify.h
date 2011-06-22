#ifndef HYBRID_NOTIFY_H
#define HYBRID_NOTIFY_H

#include <gtk/gtk.h>

typedef struct _HybridNotify HybridNotify;

struct _HybridNotify {
	GtkWidget *window;
	GtkWidget *textview;
	GtkWidget *action_area;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a notification box.
 *
 * @param title The title of the box, if NULL, then set it
 *              to default _("Notification")
 *
 * @return The notification box created.
 */
HybridNotify *hybrid_notify_create(const gchar *title);

/**
 * Set the text to displayed in the text area.
 *
 * @param notify The notification box.
 * @param text   The notification text to set.
 */
void hybrid_notify_set_text(HybridNotify *notify, const gchar *text);

#ifdef __cplusplus
}
#endif

#endif
