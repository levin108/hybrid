#ifndef HYBRID_CHAT_TEXTVIEW_H
#define HYBRID_CHAT_TEXTVIEW_H

#include <gtk/gtk.h>
#include "account.h"
#include "blist.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a chat box with GtkTextView.
 *
 * @return The chat box created.
 */
GtkWidget *hybrid_chat_textview_create();

/**
 * Append a message to the textview.
 *
 * @param textview The textview.
 * @param name     The sender's name to display.
 * @param message  The message.
 * @param sendout  TRUE if message is sent to buddy, FALSE if message is sent
 *                 from buddy.
 * @param msg_time The time displayed together with the message.
 */
void hybrid_chat_textview_append(GtkWidget *textview, const gchar *name,
								const gchar *message, time_t msg_time, 
								gboolean sendout);
/**
 * Append a notify message to the textview.
 *
 * @param textview The textview.
 * @param text     Content of the notification.
 * @param type     Type of the notification.
 */
void hybrid_chat_textview_notify(GtkWidget *textview, const gchar *text, gint type);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_CHAT_TEXTVIEW_H */
