#ifndef IM_CHAT_H
#define IM_CHAT_H

#include <gtk/gtk.h>
#include "blist.h"

typedef struct _IMConversation IMConversation;
typedef struct _IMChatPanel IMChatPanel;

struct _IMConversation {
	GtkWidget *window;
	GtkWidget *notebook;
	GSList *chat_buddies;
};

struct _IMChatPanel {
	IMConversation *parent;
	IMBuddy *buddy;
	GtkWidget *pagelabel;
	GtkWidget *textview;
	GtkWidget *toolbar;
	GtkWidget *sendtext;
	guint page_index; /**< The page index in the notebook tabs */

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
 * @return The IMChatPanel created.
 */
IMChatPanel *im_chat_panel_create(IMBuddy *buddy);

#ifdef __cplusplus
}
#endif

#endif
