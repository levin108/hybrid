#ifndef HYBIRD_GTKUTILS_H
#define HYBIRD_GTKUTILS_H

#include <gdk/gdk.h>

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
GdkPixbuf *hybird_create_pixbuf(const guchar *pixbuf_data, gint pixbuf_len);

/**
 * Create a pixbuf of the default icon with the specified size.
 *
 * @param scale_size The size of the created pixbuf image, if 0, the image 
 *                   will not be scaled.
 *
 * @return The pixbuf created.
 */
GdkPixbuf *hybird_create_default_icon(gint scale_size);

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
GdkPixbuf *hybird_create_pixbuf_at_size(const guchar *pixbuf_data, gint pixbuf_len,
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
GdkPixbuf *hybird_create_round_pixbuf(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_size);

/**
 * Create the presence status pixbuf with the given scale size.
 *
 * @param presence   The presence status.
 * @param scale_size The size to scale.
 *
 * @return The pixbuf created.
 */
GdkPixbuf *hybird_create_presence_pixbuf(gint presence, gint scale_size);

gchar *hybird_sha1(const gchar *in, gint size);

#ifdef __cplusplus
}
#endif

#endif /* HYBIRD_GTKUTILS_H */
