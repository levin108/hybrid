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

#include "gtkutils.h"
#include "account.h"
#include "debug.h"
/**
 * We copied the following two functions from pidgin,
 * hoping pidgin wouldn't angry,for we follow GPL...
 */
static gboolean 
pixbuf_is_opaque(GdkPixbuf *pixbuf)
{
    int            height, rowstride, i;
    unsigned char *pixels;
    unsigned char *row;

    if (!gdk_pixbuf_get_has_alpha(pixbuf))
        return TRUE;

    height    = gdk_pixbuf_get_height (pixbuf);
    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels    = gdk_pixbuf_get_pixels (pixbuf);

    row = pixels;
    for (i = 3; i < rowstride; i+=4) {
        if (row[i] < 0xfe)
            return FALSE;
    }

    for (i = 1; i < height - 1; i++) {
        row = pixels + (i * rowstride);
        if (row[3] < 0xfe || row[rowstride - 1] < 0xfe) {
            return FALSE;
        }
    }

    row = pixels + ((height - 1) * rowstride);
    for (i = 3; i < rowstride; i += 4) {
        if (row[i] < 0xfe)
            return FALSE;
    }

    return TRUE;
}

static void
pixbuf_make_round(GdkPixbuf *pixbuf) {
    int     width, height, rowstride;
    guchar *pixels;

    if (!gdk_pixbuf_get_has_alpha(pixbuf))
        return;
    width     = gdk_pixbuf_get_width(pixbuf);
    height    = gdk_pixbuf_get_height(pixbuf);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels    = gdk_pixbuf_get_pixels(pixbuf);

    if (width < 6 || height < 6)
        return;
    /* Top left */
    pixels[3]                 = 0;
    pixels[7]                 = 0x80;
    pixels[11]                = 0xC0;
    pixels[rowstride + 3]     = 0x80;
    pixels[rowstride * 2 + 3] = 0xC0;

    /* Top right */
    pixels[width * 4 - 1] = 0;
    pixels[width * 4 - 5] = 0x80;
    pixels[width * 4 - 9] = 0xC0;
    pixels[rowstride + (width * 4) - 1] = 0x80;
    pixels[(2 * rowstride) + (width * 4) - 1] = 0xC0;

    /* Bottom left */
    pixels[(height - 1) * rowstride + 3]  = 0;
    pixels[(height - 1) * rowstride + 7]  = 0x80;
    pixels[(height - 1) * rowstride + 11] = 0xC0;
    pixels[(height - 2) * rowstride + 3]  = 0x80;
    pixels[(height - 3) * rowstride + 3]  = 0xC0;

    /* Bottom right */
    pixels[height * rowstride - 1] = 0;
    pixels[(height - 1) * rowstride - 1] = 0x80;
    pixels[(height - 2) * rowstride - 1] = 0xC0;
    pixels[height * rowstride - 5] = 0x80;
    pixels[height * rowstride - 9] = 0xC0;
}

GdkPixbuf*
hybrid_create_pixbuf(const guchar *pixbuf_data, gint pixbuf_len)
{
    GdkPixbufLoader *loader;
    GdkPixbuf       *pixbuf;
    guchar          *default_pixbuf_data;
    gsize            default_pixbuf_size;

    loader = gdk_pixbuf_loader_new();

    if (!pixbuf_data || pixbuf_len == 0) { /**< Load the default. */
        g_file_get_contents(PIXMAPS_DIR"icons/icon.png",
                (gchar**)&default_pixbuf_data, &default_pixbuf_size, NULL);
        gdk_pixbuf_loader_write(loader, default_pixbuf_data,
                default_pixbuf_size, NULL);
        g_free(default_pixbuf_data);

    } else {
        gdk_pixbuf_loader_write(loader, pixbuf_data, pixbuf_len, NULL);
    }
    gdk_pixbuf_loader_close(loader, NULL);

    pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
    if (pixbuf) {
        g_object_ref(loader);
    }
    g_object_unref(loader);

    return pixbuf;
}

GdkPixbuf*
hybrid_create_default_icon(gint scale_size)
{
    GdkPixbuf *pixbuf;

    if (scale_size) {
        pixbuf = gdk_pixbuf_new_from_file_at_size(PIXMAPS_DIR"icons/icon.png",
                scale_size, scale_size, NULL);

    } else {
        pixbuf = gdk_pixbuf_new_from_file(PIXMAPS_DIR"icons/icon.png", NULL);
    }

    return pixbuf;
}

GdkPixbuf*
hybrid_create_proto_icon(const gchar *proto_name, gint scale_size)
{
    GdkPixbuf *pixbuf;
    gchar     *icon_name;

    g_return_val_if_fail(proto_name != NULL, NULL);
    g_return_val_if_fail(*proto_name != '\0', NULL);

    icon_name = g_strdup_printf(PIXMAPS_DIR"protocols/%s.png",  proto_name);

    pixbuf = gdk_pixbuf_new_from_file_at_size(icon_name,
            scale_size, scale_size, NULL);

    g_free(icon_name);

    return pixbuf;

}

GdkPixbuf*
hybrid_create_pixbuf_at_size(const guchar *pixbuf_data, gint pixbuf_len,
        gint scale_width, gint scale_height)
{
    GdkPixbuf *pixbuf;
    GdkPixbuf *res;

    pixbuf = hybrid_create_pixbuf(pixbuf_data, pixbuf_len);

    res = gdk_pixbuf_scale_simple(pixbuf,
            scale_width, scale_height, GDK_INTERP_BILINEAR);

    g_object_unref(pixbuf);

    return res;
}

GdkPixbuf*
hybrid_create_round_pixbuf(const guchar *pixbuf_data, gint pixbuf_len,
        gint scale_size)
{
    GdkPixbufLoader *loader;
    GdkPixbuf       *pixbuf;
    GdkPixbuf       *newpixbuf;
    gint             orig_width;
    gint             orig_height;
    guchar          *default_pixbuf_data;
    gsize            default_pixbuf_size;

    loader = gdk_pixbuf_loader_new();
    if (!pixbuf_data || pixbuf_len == 0) { /**< Load the default. */
        g_file_get_contents(PIXMAPS_DIR"icons/icon.png",
                (gchar**)&default_pixbuf_data, &default_pixbuf_size, NULL);
        gdk_pixbuf_loader_write(loader, default_pixbuf_data,
                default_pixbuf_size, NULL);
        g_free(default_pixbuf_data);

    } else {
        gdk_pixbuf_loader_write(loader, pixbuf_data, pixbuf_len, NULL);
    }
    gdk_pixbuf_loader_close(loader, NULL);

    pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
    if (pixbuf) {
        g_object_ref(loader);
    }
    g_object_unref(loader);

    if (!pixbuf) {
        hybrid_debug_error("blist", "get pixbuf from loader");
        return NULL;
    }

    orig_width  = gdk_pixbuf_get_width(pixbuf);
    orig_height = gdk_pixbuf_get_height(pixbuf);

    newpixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,    scale_size, scale_size);
    gdk_pixbuf_fill(newpixbuf, 0x00000000);

    gdk_pixbuf_scale(pixbuf, newpixbuf, 0, 0, scale_size, scale_size, 0, 0,
            (double)scale_size/(double)orig_width,
            (double)scale_size/(double)orig_height, GDK_INTERP_BILINEAR);

    g_object_unref(pixbuf);

    if (pixbuf_is_opaque(newpixbuf)) {
        pixbuf_make_round(newpixbuf);
    }

    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, scale_size, scale_size);
    gdk_pixbuf_fill(pixbuf, 0x00000000);
    gdk_pixbuf_copy_area(newpixbuf, 0, 0, scale_size, scale_size,
            pixbuf, 0, 0);

    g_object_unref(newpixbuf);

    return pixbuf;
}

GdkPixbuf*
hybrid_create_presence_pixbuf(gint presence, gint scale_size)
{
    const gchar *name;

    switch (presence) {

    case HYBRID_STATE_OFFLINE:
      name = PIXMAPS_DIR"status/offline.png";
      break;
      
    case HYBRID_STATE_INVISIBLE:
      name = PIXMAPS_DIR"status/invisible.png";
      break;
      
    case HYBRID_STATE_BUSY:
      name = PIXMAPS_DIR"status/busy.png";
      break;
      
    case HYBRID_STATE_AWAY:
      name = PIXMAPS_DIR"status/away.png";
      break;
      
    case HYBRID_STATE_ONLINE:
      name = PIXMAPS_DIR"status/available.png";
      break;
      
    default:
      name = PIXMAPS_DIR"status/offline.png";
      break;
    };

    return gdk_pixbuf_new_from_file_at_size(name,
            scale_size, scale_size, NULL);
}

GtkWidget*
hybrid_create_menu(GtkWidget *parent, const gchar *title,
        const gchar *icon_name, gboolean sensitive,
        GCallback callback,
        gpointer user_data)
{
    GtkWidget *item;
    GdkPixbuf *pixbuf;
    GtkWidget *image;
    gchar     *icon_path;
    
    g_return_val_if_fail(title != NULL, NULL);

    if (icon_name) {
        icon_path = g_strdup_printf(PIXMAPS_DIR"menus/%s.png", icon_name);
        item = gtk_image_menu_item_new_with_label(title);
        pixbuf = gdk_pixbuf_new_from_file_at_size(icon_path, 16, 16, NULL);

        if (pixbuf) {
            image = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
        }

        g_free(icon_path);

    } else {
        item = gtk_image_menu_item_new_with_label(title);
    }

    if (parent) {
        gtk_menu_shell_append(GTK_MENU_SHELL(parent), item);
    }

    if (!sensitive) {
        gtk_widget_set_sensitive(item, FALSE);
        return item;
    }

    if (callback) {
        g_signal_connect(item, "activate", G_CALLBACK(callback), user_data);
    }

    return item;
}


void
hybrid_create_menu_seperator(GtkWidget *parent)
{
    GtkWidget *seperator;

    g_return_if_fail(parent != NULL);

    seperator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(parent), seperator);
}

GtkWidget*
hybrid_create_window(const gchar *title, GdkPixbuf *icon,
        GtkWindowPosition pos, gboolean resizable)
{
    GtkWidget *window;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    if (!icon) {
        icon = hybrid_create_default_icon(0);
    }

    gtk_window_set_icon(GTK_WINDOW(window), icon);

    gtk_window_set_resizable(GTK_WINDOW(window), resizable);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_position(GTK_WINDOW(window), pos);
    g_object_unref(icon);

    return window;
}

/* ================ Confirm Box Start =======================*/
static void
confirm_destroy(GtkWidget *widget, gpointer user_data)
{
    HybridConfirm *confirm = (HybridConfirm*)user_data;

    g_free(confirm);
}

static void
confirm_cancel_btn_cb(GtkWidget *widget, gpointer user_data)
{
    HybridConfirm *confirm;

    confirm = (HybridConfirm*)user_data;

    gtk_widget_destroy(confirm->window);
}

static void
confirm_user_btn_cb(GtkWidget *widget, gpointer user_data)
{
    HybridConfirm *confirm;
    confirm_cb     callback;
    gpointer       cb_user_data;

    confirm = (HybridConfirm*)user_data;
    callback = confirm->btn_callback;
    cb_user_data = confirm->user_data;

    gtk_widget_destroy(confirm->window);

    if (callback) {
        callback(cb_user_data);
    }
}

HybridConfirm*
hybrid_confirm_show(const gchar *title, const gchar *text,
        const gchar *btn_text, confirm_cb btn_callback, gpointer user_data)
{
    GtkWidget     *window;
    GtkWidget     *vbox;
    GtkWidget     *hbox;
    GtkWidget     *action_area;
    GdkPixbuf     *pixbuf;
    GtkWidget     *image;
    GtkWidget     *label;
    GtkWidget     *button;
    HybridConfirm *confirm;

    g_return_val_if_fail(title != NULL, NULL);
    g_return_val_if_fail(text != NULL, NULL);

    confirm               = g_new0(HybridConfirm, 1);
    confirm->btn_callback = btn_callback;
    confirm->user_data    = user_data;

    window = hybrid_create_window(title, NULL, GTK_WIN_POS_CENTER, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);
    g_signal_connect(window, "destroy", G_CALLBACK(confirm_destroy), confirm);

    confirm->window = window;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    pixbuf = hybrid_create_default_icon(64);
    image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 5);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), text);

    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);

    action_area = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 0);

    button = gtk_button_new_with_label(btn_text);
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked",
            G_CALLBACK(confirm_user_btn_cb), confirm);

    button = gtk_button_new_with_label(_("Cancel"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked",
            G_CALLBACK(confirm_cancel_btn_cb), confirm);

    gtk_widget_show_all(confirm->window);

    return confirm;
}
/* ================== Confirm Box End ======================= */

/* ================== Message Box Start ===================== */

/**
 * Callback function of the message box destroy signal.
 */
static void
message_destroy(GtkWidget *widget, HybridMessageBox *box)
{
    g_free(box);
}

static void
message_ok_btn_cb(GtkWidget *widget, HybridMessageBox *box)
{
    gtk_widget_destroy(box->window);
}

HybridMessageBox*
hybrid_message_box_show(HybridMessageType type,
                        const gchar *format, ...)
{
    GtkWidget        *window;
    GtkWidget        *vbox;
    GtkWidget        *hbox;
    GtkWidget        *action_area;
    GdkPixbuf        *pixbuf;
    GtkWidget        *image;
    GtkWidget        *label;
    GtkWidget        *button;
    HybridMessageBox *msg_box;
    va_list           vlist;
    gchar            *message;
    const gchar      *title_str;

    g_return_val_if_fail(format != NULL, NULL);

    msg_box = g_new0(HybridMessageBox, 1);

    if (type == HYBRID_MESSAGE_WARNING) {
        title_str = _("Warning");

    } else if (type == HYBRID_MESSAGE_INFO) {
        title_str = _("Information");

    } else {
        title_str = _("Message");
    }

    window = hybrid_create_window(title_str, NULL, GTK_WIN_POS_CENTER, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);
    g_signal_connect(window, "destroy", G_CALLBACK(message_destroy), msg_box);

    msg_box->window = window;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    if (type == HYBRID_MESSAGE_WARNING) {
        pixbuf = gdk_pixbuf_new_from_file_at_size(
                PIXMAPS_DIR"icons/warning.png", 64, 64, NULL);

    } else {
        pixbuf = gdk_pixbuf_new_from_file_at_size(
                PIXMAPS_DIR"icons/info.png", 64, 64, NULL);
    }
    image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 5);

    label = gtk_label_new(NULL);
    va_start(vlist, format);
    message = g_strdup_vprintf(format, vlist);
    gtk_label_set_markup(GTK_LABEL(label), message);
    g_free(message);
    va_end(vlist);

    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);

    action_area = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 0);

    button = gtk_button_new_with_label(_("OK"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked",
            G_CALLBACK(message_ok_btn_cb), msg_box);

    gtk_widget_show_all(msg_box->window);

    return msg_box;
}

/* ================== Message Box End ======================= */
