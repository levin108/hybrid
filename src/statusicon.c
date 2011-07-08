#include "statusicon.h"
#include "gtkutils.h"
#include "conv.h"

extern GtkWidget *hybrid_window;
extern GSList *conv_list;

HybridStatusIcon *status_icon;

static void
status_icon_activate_cb(GtkWidget *widget, gpointer user_data)
{

	gtk_window_deiconify(GTK_WINDOW(hybrid_window));

    if (GTK_WIDGET_VISIBLE(hybrid_window)) {

		if (!gtk_window_is_active(GTK_WINDOW(hybrid_window))) {

			gtk_window_present(GTK_WINDOW(hybrid_window));

			return;
		}

		gtk_widget_hide(hybrid_window);

	} else {

		gtk_widget_show(hybrid_window);
	}
}

static void
status_icon_msg_cb(GtkWidget *widget, gpointer user_data)
{
	GdkPixbuf *pixbuf;
	GSList *conv_pos;
	GSList *chat_pos;
	HybridConversation *conv;
	HybridChatWindow *chat;
	gint current_page;

	g_signal_handler_disconnect(G_OBJECT(status_icon->icon),
				status_icon->conn_id);

	pixbuf = hybrid_create_round_pixbuf(NULL, 0, 20);

	gtk_status_icon_set_from_pixbuf(
			GTK_STATUS_ICON(status_icon->icon), pixbuf);

	g_object_unref(pixbuf);

	gtk_status_icon_set_blinking(GTK_STATUS_ICON(status_icon->icon), FALSE);

	status_icon->conn_id = 
		g_signal_connect(G_OBJECT(status_icon->icon), "activate",
		                 G_CALLBACK(status_icon_activate_cb), NULL);

	for (conv_pos = conv_list; conv_pos; conv_pos = conv_pos->next) {

		conv = (HybridConversation*)conv_pos->data;

		/* find the current chat panel. */
		current_page = gtk_notebook_current_page(GTK_NOTEBOOK(conv->notebook));

		for (chat_pos = conv->chat_buddies; chat_pos; chat_pos = chat_pos->next) {

			chat = (HybridChatWindow*)chat_pos->data;
				
			if (chat->unread != 0) {

				gtk_window_present(GTK_WINDOW(conv->window));

				if (current_page == gtk_notebook_page_num(
							GTK_NOTEBOOK(conv->notebook), chat->vbox)) {

					chat->unread = 0;

					hybrid_chat_window_update_tips(chat);
				}
			}
		}
	}
}

void
hybrid_status_icon_init(void)
{
	GdkPixbuf *pixbuf;

	status_icon = g_new0(HybridStatusIcon, 1);


	status_icon->icon = gtk_status_icon_new();

	pixbuf = hybrid_create_round_pixbuf(NULL, 0, 20);

	gtk_status_icon_set_from_pixbuf(
			GTK_STATUS_ICON(status_icon->icon), pixbuf);

	g_object_unref(pixbuf);

	gtk_status_icon_set_tooltip(status_icon->icon, "Hybrid");

	status_icon->conn_id = 
		g_signal_connect(G_OBJECT(status_icon->icon), "activate",
		                 G_CALLBACK(status_icon_activate_cb), NULL);
}

void
hybrid_status_icon_blinking(HybridBuddy *buddy)
{
	GdkPixbuf *pixbuf;

	if (buddy) {

		pixbuf = hybrid_create_round_pixbuf(buddy->icon_data,
									buddy->icon_data_length, 20);

		if (!pixbuf) {
			return;
		}

		gtk_status_icon_set_from_pixbuf(
				GTK_STATUS_ICON(status_icon->icon), pixbuf);

		gtk_status_icon_set_blinking(GTK_STATUS_ICON(status_icon->icon), TRUE);

		g_object_unref(pixbuf);

		g_signal_handler_disconnect(G_OBJECT(status_icon->icon),
					status_icon->conn_id);

		status_icon->conn_id = 
			g_signal_connect(G_OBJECT(status_icon->icon), "activate",
							 G_CALLBACK(status_icon_msg_cb), NULL);

	} else {
		g_signal_handler_disconnect(G_OBJECT(status_icon->icon),
					status_icon->conn_id);

		pixbuf = hybrid_create_round_pixbuf(NULL, 0, 20);

		gtk_status_icon_set_from_pixbuf(
				GTK_STATUS_ICON(status_icon->icon), pixbuf);

		g_object_unref(pixbuf);

		gtk_status_icon_set_blinking(GTK_STATUS_ICON(status_icon->icon), FALSE);

		status_icon->conn_id = 
			g_signal_connect(G_OBJECT(status_icon->icon), "activate",
							 G_CALLBACK(status_icon_activate_cb), NULL);
	}
}
