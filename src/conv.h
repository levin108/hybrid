#ifndef HYBRID_CHAT_H
#define HYBRID_CHAT_H

#include <gtk/gtk.h>
#include "blist.h"

typedef struct _HybridConversation HybridConversation;
typedef struct _HybridChatPanel HybridChatPanel;

struct _HybridConversation {
	GtkWidget *window;
	GtkWidget *notebook;
	GSList *chat_buddies;
};

struct _HybridChatPanel {
	HybridConversation *parent;
	HybridBuddy *buddy;
	GtkWidget *pagelabel;
	GtkWidget *textview;
	GtkWidget *toolbar;
	GtkWidget *sendtext;
	GtkWidget *vbox; /**< The Notebook child widget */

	/* tab label */
	GtkWidget *tablabel;
	GtkTreeIter tabiter;

	GtkWidget *tiplabel;
	GtkTreeIter tipiter;
};

/**
 * Notebook tab columns.
 */
enum {
	TAB_STATUS_ICON_COLUMN = 0,
	TAB_NAME_COLUMN,
	TAB_COLUMNS
};

/**
 * Notebook tips columns.
 */
enum {
	BUDDY_ICON_COLUMN = 0,
	BUDDY_NAME_COLUMN,
	BUDDY_STATUS_ICON_COLUMN,
	BUDDY_PROTO_ICON_COLUMN,
	LABEL_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a chat panel.
 *
 * @param buddy The buddy to chat with.
 *
 * @return The HybridChatPanel created.
 */
HybridChatPanel *hybrid_chat_panel_create(HybridBuddy *buddy);

#ifdef __cplusplus
}
#endif

#endif
