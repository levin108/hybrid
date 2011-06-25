#ifndef HYBRID_GTKUTILS_H
#define HYBRID_GTKUTILS_H

#include <gdk/gdk.h>
#include <gtk/gtk.h>

typedef struct _HybridConfirm HybridConfirm;
typedef void (*confirm_cb)(gpointer user_data);

struct _HybridConfirm {
	GtkWidget *window;
	confirm_cb btn_callback;
	gpointer user_data;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a pixbuf using the pixbuf binary data.
 *
 * @param pixbuf_data The binary data of the pixbuf.
 * @param pixbuf_len  The size of the binary data.
 *
 * @return The pixbuf created.
 */
GdkPixbuf *hybrid_create_pixbuf(const guchar *pixbuf_data, gint pixbuf_len);

/**
 * Create a pixbuf of the default icon with the specified size.
 *
 * @param scale_size The size of the created pixbuf image, if 0, the image 
 *                   will not be scaled.
 *
 * @return The pixbuf created.
 */
GdkPixbuf *hybrid_create_default_icon(gint scale_size);

/**
 * Create a protocol icon pixbuf.
 *
 * @param proto_name The name of the protocol.
 * @param scale_size The scale size of the pixbuf.
 *
 * @return The pixbuf created.
 */
GdkPixbuf *hybrid_create_proto_icon(const gchar *proto_name, gint scale_size);

/**
 * Create a pixbuf using the pixbuf binary data, and scale it to the given size.
 *
 * @param pixbuf_data  The binary data of the pixbuf.
 * @param pixbuf_len   The size of the binary data.
 * @param scale_width  The width of the image to scale to.
 * @param scale_height The heihgt of the image to scale to.
 *
 * @return The pixbuf created.
 */
GdkPixbuf *hybrid_create_pixbuf_at_size(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_width, gint scale_height);

/**
 * Create a pixbuf using the pixbuf binary data with a round corner.
 * The pixbuf's width must be equal to its height.
 *
 * @param pixbuf_data  The binary data of the pixbuf.
 * @param pixbuf_len   The size of the binary data.
 * @param scale_width  The width of the image to scale to.
 * @param scale_height The heihgt of the image to scale to.
 *
 * return The pixbuf created.
 */
GdkPixbuf *hybrid_create_round_pixbuf(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_size);

/**
 * Create the presence status pixbuf with the given scale size.
 *
 * @param presence   The presence status.
 * @param scale_size The size to scale.
 *
 * @return The pixbuf created.
 */
GdkPixbuf *hybrid_create_presence_pixbuf(gint presence, gint scale_size);

/**
 * Create a child menu for the @parent menu. Create an image menu if 
 * icon_path is not NULL, orelse create a text menu.
 *
 * @param parent    The parent menu of the menu to create.
 * @param icon_name The name of icon without suffix and '/', The icon
 *                  must be in share/menus, and has suffix of '.png'.
 */
GtkWidget *hybrid_create_menu(GtkWidget *parent, const gchar *title,
		const gchar *icon_name, gboolean sensitive,
		GCallback callback,
		gpointer user_data);

/**
 * Create a seperator for a menu.
 *
 * @param parent The menu widget.
 */
void hybrid_create_menu_seperator(GtkWidget *parent);

/**
 * Create a GtkWindow.
 *
 * @param title The window title.
 * @param icon  The window icon. If NULL, load the default icon.
 *              It'll decrease the reference value of the icon,
 *              so there's no need to unref the icon manaully.
 * @param pos   The window position.
 * @param resizable True if the user can resize the window.
 *
 * @return The GtkWindow created.
 */
GtkWidget *hybrid_create_window(const gchar *title,	GdkPixbuf *icon,
		GtkWindowPosition pos, gboolean resizable);

/**
 * Create a confirm window.
 *
 * @param title        The title of the confirm window.
 * @param text         The text to show in the window.
 * @param btn_text     The displayed text in the user-defined button.
 * @param btn_callback The callback function of the user-defined button.
 * @param user_data    The user-specified data for the callback function.
 *
 * @return The confirm box created. You don't have to destroy it manually,
 *         it will be destroyed when the box was closed automaticly.
 */
HybridConfirm *hybrid_confirm_show(const gchar *title, const gchar *text,
		const gchar *btn_text, confirm_cb btn_callback, gpointer user_data);


gchar *hybrid_sha1(const gchar *in, gint size);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_GTKUTILS_H */
