#include "gtkutils.h"

#include "util.h"
#include "confirm.h"

/**
 * Destroy the HybridConfirmWindow object when the window
 * was destroyed.
 */
static void
destroy_cb(GtkWidget *widget, HybridConfirmWindow *con)
{
    g_free(con);
}

static void
ok_cb(GtkWidget *widget, HybridConfirmWindow *con)
{
    const gchar *code;

    if (con->ok_func) {
        code = gtk_entry_get_text(GTK_ENTRY(con->entry));
        con->ok_func(con->account, code, con->user_data);
    }

    gtk_widget_destroy(con->window);
}

static void
cancel_cb(GtkWidget *widget, HybridConfirmWindow *con)
{
    if (con->cancel_func) {
        con->cancel_func(con->account, con->user_data);
    }
    gtk_widget_destroy(con->window);
}

HybridConfirmWindow*
hybrid_confirm_window_create(HybridAccount           *account,
                             guchar                  *pic_data,
                             gint                     pic_len,
                             HybridConfirmOkFunc      ok_func,
                             HybridConfirmCancelFunc  cancel_func,
                             gpointer                 user_data)
{
    HybridConfirmWindow *con;
    GtkWidget           *vbox;
    GtkWidget           *table;
    GtkWidget           *action_area;
    GtkWidget           *button;
    GtkWidget           *label;
    GdkPixbuf           *pixbuf;

    con              = g_new0(HybridConfirmWindow, 1);
    con->account     = account;
    con->ok_func     = ok_func;
    con->cancel_func = cancel_func;
    con->user_data   = user_data;

    con->window = hybrid_create_window(_("Confirm Code"), NULL,
                                       GTK_WIN_POS_CENTER, FALSE);
    gtk_widget_set_size_request(con->window, 300, 200);
    gtk_container_set_border_width(GTK_CONTAINER(con->window), 8);
    g_signal_connect(con->window, "destroy", G_CALLBACK(destroy_cb), con);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(con->window), vbox);

    table = gtk_table_new(3, 3, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 5);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label),
                         _("<i>Please input the following code in the picture:</i>"));
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 2, 0, 1);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), _("<i>Code:</i>"));
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);

    /* show code picture. */
    pixbuf     = hybrid_create_pixbuf(pic_data, pic_len);
    con->image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    gtk_table_attach_defaults(GTK_TABLE(table), con->image, 1, 2, 1, 2);

    con->entry = gtk_entry_new();
    g_signal_connect(con->entry, "activate", G_CALLBACK(ok_cb), con);
    gtk_table_attach_defaults(GTK_TABLE(table), con->entry, 0, 2, 2, 3);

    action_area = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5);

    button = gtk_button_new_with_label(_("OK"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked", G_CALLBACK(ok_cb), con);

    button = gtk_button_new_with_label(_("Cancel"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked", G_CALLBACK(cancel_cb), con);

    gtk_widget_show_all(con->window);

    return con;
}
