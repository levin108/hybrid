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
 * @param buddy    The buddy who sent or to receive the message.
 * @param message  The message.
 * @param sendout  TRUE if message is sent to buddy, FALSE if message is sent
 *                 from buddy.
 */
void hybrid_chat_textview_append(GtkWidget *textview, HybridBuddy *buddy,
								const gchar *message, gboolean sendout);
#ifdef __cplusplus
}
#endif

#endif /* HYBRID_CHAT_TEXTVIEW_H */
