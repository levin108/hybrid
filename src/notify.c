#include "util.h"
#include "notify.h"
#include "gtkutils.h"

static void
close_cb(GtkWidget *widget, gpointer user_data)
{
	HybridNotify *notify = (HybridNotify*)user_data;

	gtk_widget_destroy(notify->window);
	
	g_free(notify);
}

HybridNotify*
hybrid_notify_create(const gchar *title)
{
	HybridNotify *notify;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *scroll;

	notify = g_new0(HybridNotify, 1);

	notify->window = hybrid_create_window(title ? title : _("Notification"),
						NULL, GTK_WIN_POS_CENTER, FALSE);
	gtk_widget_set_usize(notify->window, 400, 250);

	vbox = gtk_vbox_new(FALSE, 0);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);

	notify->textview = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(notify->textview), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(notify->textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(notify->textview) , GTK_WRAP_CHAR);

	gtk_container_add(GTK_CONTAINER(scroll), notify->textview);

	notify->action_area = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), notify->action_area, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(notify->action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(close_cb), notify);

	gtk_container_add(GTK_CONTAINER(notify->window), vbox);

	gtk_widget_show_all(notify->window);
	
	return notify;
}

void
hybrid_notify_set_text(HybridNotify *notify, const gchar *text)
{
	GtkTextBuffer *buffer;
	GtkTextIter iter;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(notify->textview));

	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, text, strlen(text));
}
