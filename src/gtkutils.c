#include <openssl/sha.h>
#include "gtkutils.h"
#include "account.h"
#include "debug.h"
/**
 * We copied the following two functions from pidgin,
 * hoping pidgin wouldn't angry,for we follow GPL...
 */
static gboolean 
pixbuf_is_opaque(GdkPixbuf *pixbuf) {
	int height, rowstride, i;
	unsigned char *pixels;
	unsigned char *row;

	if (!gdk_pixbuf_get_has_alpha(pixbuf))
		return TRUE;

	height = gdk_pixbuf_get_height (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);

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
	int width, height, rowstride;
	guchar *pixels;
	if (!gdk_pixbuf_get_has_alpha(pixbuf))
		return;
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	if (width < 6 || height < 6)
		return;
	/* Top left */
	pixels[3] = 0;
	pixels[7] = 0x80;
	pixels[11] = 0xC0;
	pixels[rowstride + 3] = 0x80;
	pixels[rowstride * 2 + 3] = 0xC0;

	/* Top right */
	pixels[width * 4 - 1] = 0;
	pixels[width * 4 - 5] = 0x80;
	pixels[width * 4 - 9] = 0xC0;
	pixels[rowstride + (width * 4) - 1] = 0x80;
	pixels[(2 * rowstride) + (width * 4) - 1] = 0xC0;

	/* Bottom left */
	pixels[(height - 1) * rowstride + 3] = 0;
	pixels[(height - 1) * rowstride + 7] = 0x80;
	pixels[(height - 1) * rowstride + 11] = 0xC0;
	pixels[(height - 2) * rowstride + 3] = 0x80;
	pixels[(height - 3) * rowstride + 3] = 0xC0;

	/* Bottom right */
	pixels[height * rowstride - 1] = 0;
	pixels[(height - 1) * rowstride - 1] = 0x80;
	pixels[(height - 2) * rowstride - 1] = 0xC0;
	pixels[height * rowstride - 5] = 0x80;
	pixels[height * rowstride - 9] = 0xC0;
}

GdkPixbuf*
hybird_create_pixbuf(const guchar *pixbuf_data, gint pixbuf_len)
{
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	guchar *default_pixbuf_data;
	gsize default_pixbuf_size;

	loader = gdk_pixbuf_loader_new();

	if (!pixbuf_data || pixbuf_len == 0) { /**< Load the default. */
		g_file_get_contents(DATA_DIR"/icon.png",
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
hybird_create_default_icon(gint scale_size)
{
	GdkPixbuf *pixbuf;

	if (scale_size) {
		pixbuf = gdk_pixbuf_new_from_file_at_size(DATA_DIR"/icon.png",
				scale_size, scale_size, NULL);

	} else {
		pixbuf = gdk_pixbuf_new_from_file(DATA_DIR"/icon.png", NULL);
	}

	return pixbuf;
}

GdkPixbuf*
hybird_create_pixbuf_at_size(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_width, gint scale_height)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *res;

	pixbuf = hybird_create_pixbuf(pixbuf_data, pixbuf_len);

	res = gdk_pixbuf_scale_simple(pixbuf,
			scale_width, scale_height, GDK_INTERP_BILINEAR);

	g_object_unref(pixbuf);

	return res;
}

GdkPixbuf*
hybird_create_round_pixbuf(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_size)
{
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	GdkPixbuf *newpixbuf;
	gint orig_width;
	gint orig_height;
	guchar *default_pixbuf_data;
	gsize default_pixbuf_size;

	loader = gdk_pixbuf_loader_new();
	if (!pixbuf_data || pixbuf_len == 0) { /**< Load the default. */
		g_file_get_contents(DATA_DIR"/icon.png",
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
		hybird_debug_error("blist", "get pixbuf from loader");
		return NULL;
	}

	orig_width = gdk_pixbuf_get_width(pixbuf);
	orig_height = gdk_pixbuf_get_height(pixbuf);

	newpixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,	32, 32);
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
hybird_create_presence_pixbuf(gint presence, gint scale_size)
{
	const gchar *name;

	switch (presence) {

		case HYBIRD_STATE_OFFLINE:
			name = DATA_DIR"/offline.png";
			break;
		case HYBIRD_STATE_INVISIBLE:
			name = DATA_DIR"/invisible.png";
			break;
		case HYBIRD_STATE_BUSY:
			name = DATA_DIR"/busy.png";
			break;
		case HYBIRD_STATE_AWAY:
			name = DATA_DIR"/away.png";
			break;
		case HYBIRD_STATE_ONLINE:
			name = DATA_DIR"/available.png";
			break;
		default:
			name = DATA_DIR"/offline.png";
			break;
	};

	return gdk_pixbuf_new_from_file_at_size(name,
			scale_size, scale_size, NULL);
}

GtkWidget*
hybird_create_menu(GtkWidget *parent, const gchar *title,
		const gchar *icon_name, gboolean sensitive,
		void (*callback)(GtkWidget *widget, gpointer user_data),
		gpointer user_data)
{
	GtkWidget *item;
	GdkPixbuf *pixbuf;
	GtkWidget *image;
	gchar *icon_path;

	g_return_val_if_fail(title != NULL, NULL);

	if (icon_name) {
		icon_path = g_strdup_printf(DATA_DIR"/menus/%s.png", icon_name);
		item = gtk_image_menu_item_new_with_label(title);
		pixbuf = gdk_pixbuf_new_from_file_at_size(icon_path, 16, 16, NULL);

		if (pixbuf) {
			image = gtk_image_new_from_pixbuf(pixbuf);
			g_object_unref(pixbuf);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
		}

		g_free(icon_path);

	} else {
		item = gtk_menu_item_new_with_label(title);
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
hybird_create_menu_seperator(GtkWidget *parent)
{
	GtkWidget *seperator;

	g_return_if_fail(parent != NULL);

	seperator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(parent), seperator);
}

GtkWidget*
hybird_create_window(const gchar *title, GdkPixbuf *icon,
		GtkWindowPosition pos, gboolean resizable)
{
	GtkWidget *window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	if (!icon) {
		icon = hybird_create_default_icon(0);
	}

	gtk_window_set_icon(GTK_WINDOW(window), icon);

	gtk_window_set_resizable(GTK_WINDOW(window), resizable);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_window_set_position(GTK_WINDOW(window), pos);
	g_object_unref(icon);

	return window;
}


gchar*
hybird_sha1(const gchar *in, gint size)
{
	SHA_CTX s;
	guchar hash[20];
	gchar *res;
	gint i;
  
	SHA1_Init(&s);
	SHA1_Update(&s, in, size);
	SHA1_Final(hash, &s);

	res = g_malloc0(41);
  
	for (i=0; i < 20; i++) {
		g_snprintf(res + i * 2, 41, "%.2x", (gint)hash[i]);
	}

	return res;
}
