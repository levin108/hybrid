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

#ifndef HYBRID_CHAT_H
#define HYBRID_CHAT_H

#include <gtk/gtk.h>
#include "logs.h"

#include "account.h"
#include "blist.h"

typedef struct _HybridConversation HybridConversation;
typedef struct _HybridChatWindow   HybridChatWindow;
typedef struct _HybridChatTextOps  HybridChatTextOps;
typedef enum _HybridChatWindowType HybridChatWindowType;

typedef void (*ChatCallback)(HybridAccount *, const gchar *);

typedef GtkWidget* (*text_create)(void);
typedef void (*text_append)(GtkWidget *, HybridAccount *,
							HybridBuddy *,	const gchar *, time_t);
typedef void (*text_notify)(GtkWidget *, const gchar *, gint);

struct _HybridChatTextOps{
	text_create create;
	text_append append;
	text_notify notify;
};

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

	HybridLogs *logs; /* log context. */

	gpointer data;
	GtkWidget *pagelabel;

	GtkWidget *textview;
	GtkWidget *toolbar;
	GtkWidget *sendtext;
	GtkWidget *vbox; /**< The Notebook child widget */

	/* label to show how many words left that can be input. */
	GtkWidget *words_left_label;

	/* tab label */
	GtkWidget *tablabel;
	GtkTreeIter tabiter;

	/* tip label */
	GtkWidget *tiplabel;
	GtkTreeIter tipiter;

	/* callback function called when the send button clicked,
	 * only be used when it's user-defined window. */
	ChatCallback callback;

	guint typing_source;
	gboolean is_typing;

	/* event source of the inputing timeout event. */
	guint input_source;
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

enum {
	MSG_NOTIFICATION_INPUT,
	MSG_NOTIFICATION_ERROR,
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
 * Find a chat window for buddy with the given buddy id.
 *
 * @param buddy_id ID of the buddy.
 *
 * @param chat NULL if not found.
 */
HybridChatWindow *hybrid_conv_find_chat(const gchar *buddy_id);

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
 * Set the chat text ops.
 *
 * @param ops The ops methods.
 */
void hybrid_conv_set_chat_text_ops(HybridChatTextOps *ops);

/**
 * Got a status message to display in the receiving window.
 *
 * @param account  The account.
 * @param buddy_id ID of the buddy to which the chat window belongs.
 * @param text     Content of the status message.
 * @param type     Tyep of the status.
 */
void hybrid_conv_got_status(HybridAccount *account, const gchar *buddy_id,
		const gchar *text, gint type);

/**
 * Got an buddy's inputing message.
 *
 * @param account   The account.
 * @param buddy_id  ID of the buddy to which the chat window belongs.
 * @param auto_stop If TRUE it will set the inputing state to be stoped automaticly. 
 */
void hybrid_conv_got_inputing(HybridAccount *account, const gchar *buddy_id,
		gboolean auto_stop);

/**
 * Got an buddy's stopping inputing message.
 *
 * @param account   The account.
 * @param buddy_id  ID of the buddy to which the chat window belongs.
 */
void hybrid_conv_stop_inputing(HybridAccount *account, const gchar *buddy_id);

/**
 * Clear the inputing message.
 *
 * @param account The account.
 * @param buddy_id ID of the buddy to which the chat window belongs.
 */
void hybrid_conv_clear_inputing(HybridAccount *account, const gchar *buddy_id);

/**
 * Update the tips title of the chat window, usually used when
 * a new message is received, or messages are being read.
 */
void hybrid_chat_window_update_tips(HybridChatWindow *window);

#ifdef __cplusplus
}
#endif

#endif
