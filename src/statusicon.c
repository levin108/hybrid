#include "statusicon.h"
#include "gtkutils.h"

extern GtkWidget *hybrid_window;

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

void
hybrid_status_icon_init(void)
{
	status_icon = g_new0(HybridStatusIcon, 1);

	status_icon->icon = gtk_status_icon_new_from_file(
							PIXMAPS_DIR"icons/icon.png");

	gtk_status_icon_set_tooltip(status_icon->icon, "Hybrid");

	status_icon->conn_id = 
		g_signal_connect(G_OBJECT(status_icon->icon), "activate",
		                 G_CALLBACK(status_icon_activate_cb), NULL);
}

void
hybrid_status_icon_blinking(HybridBuddy *buddy)
{
	GdkPixbuf *pixbuf;

	g_return_if_fail(buddy != NULL);

	pixbuf = hybrid_create_pixbuf_at_size(buddy->icon_data,
								buddy->icon_data_length, 24, 24);

	if (!pixbuf) {
		return;
	}

	gtk_status_icon_set_from_pixbuf(
			GTK_STATUS_ICON(status_icon->icon), pixbuf);

	gtk_status_icon_set_blinking(GTK_STATUS_ICON(status_icon->icon), TRUE);

	g_object_unref(pixbuf);
}
