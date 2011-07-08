#ifndef HYBRID_CHAT_H
#define HYBRID_CHAT_H

#include <gtk/gtk.h>
#include "blist.h"

typedef struct _HybridConversation HybridConversation;
typedef struct _HybridChatWindow HybridChatWindow;
typedef enum _HybridChatWindowType HybridChatWindowType;
typedef void (*ChatCallback)(HybridAccount *, const gchar *);

struct _HybridConversation {
	GtkWidget *window;
	GtkWidget *notebook;
	GSList *chat_buddies;
};

enum _HybridChatWindowType {
	/*
	 * system panel, double-click on buddy in the buddy list,
	 * then the popuped panel is in this type
	 */
	HYBRID_CHAT_PANEL_SYSTEM,      

	/*
	 * Group chat panel. UNUSED now (6-30)
	 */
	HYBRID_CHAT_PANEL_GROUP_CHAT,

	/*
	 * use-defined panel, should specify the callback function
	 * for the send button click signal.
	 */
	HYBRID_CHAT_PANEL_USER_DEFINED
};

struct _HybridChatWindow {
	HybridConversation *parent;
	HybridChatWindowType type;
	HybridAccount *account;
	gchar *id;

	gchar *title;   /**< only be used when it's user-defined window. */
	GdkPixbuf *icon;/**< only be used when it's user-defined window. */

	gint unread; /* count of the unread message. */

	gpointer data;
	GtkWidget *pagelabel;

	GtkWidget *textview;
	GtkWidget *toolbar;
	GtkWidget *sendtext;
	GtkWidget *vbox; /**< The Notebook child widget */

	/* tab label */
	GtkWidget *tablabel;
	GtkTreeIter tabiter;

	/* tip label */
	GtkWidget *tiplabel;
	GtkTreeIter tipiter;

	/* callback function called when the send button clicked,
	 * only be used when it's user-defined window. */
	ChatCallback callback;
};

#define IS_SYSTEM_CHAT(chat_window)       ((chat_window)->type == HYBRID_CHAT_PANEL_SYSTEM)
#define IS_GROUP_CHAT(chat_window)        ((chat_window)->type == HYBRID_CHAT_PANEL_GROUP_CHAT)
#define IS_USER_DEFINED_CHAT(chat_window) ((chat_window)->type == HYBRID_CHAT_PANEL_USER_DEFINED)

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
 * @param account The account context.
 * @param buddy   ID of the panel.
 * @param type    The type of the chat panel.
 *
 * @return The HybridChatWindow created.
 */
HybridChatWindow *hybrid_chat_window_create(HybridAccount *account,
					const gchar *id, HybridChatWindowType type);

/**
 * Set the title of the chat window, it's only used when 
 * the window is a user-defined window, otherwise this function
 * will be ignored.
 *
 * @param window The user-defined chat window.
 * @param title  The title of the chat window.
 */
void hybrid_chat_window_set_title(HybridChatWindow *window,
					const gchar *title);

/**
 * Set the icon of the chat window, it's only used when
 * the window is a user-defined window, otherwise this function
 * will be ignored.
 *
 * @param window The user-defined chat window.
 * @param pixbuf The icon.
 */
void hybrid_chat_window_set_icon(HybridChatWindow *window,
					GdkPixbuf *pixbuf);

/**
 * Set the callback function for the send button click event,
 * it's only used when the window is a user-defined window,
 * otherwise this function will be ignored.
 *
 * @param window   The user-defined chat window.
 * @param callback The callback function.
 */
void hybrid_chat_window_set_callback(HybridChatWindow *window,
					ChatCallback callback);

void hybrid_conv_got_message(HybridAccount *account,
				const gchar *buddy_id, const gchar *message,
				time_t time);

/**
 * Update the tips title of the chat window, usually used when
 * a new message is received, or messages are being read.
 */
void hybrid_chat_window_update_tips(HybridChatWindow *window);

#ifdef __cplusplus
}
#endif

#endif
