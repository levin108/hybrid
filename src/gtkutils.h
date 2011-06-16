#ifndef Hybird_GTKUTILS_H
#define Hybird_GTKUTILS_H

#include <gdk/gdk.h>

#ifdef __cplusplus
extern "C" {
#endif

gboolean pixbuf_is_opaque(GdkPixbuf *pixbuf);

void pixbuf_make_round(GdkPixbuf *pixbuf);

GdkPixbuf *create_pixbuf(const guchar *pixbuf_data, gint pixbuf_len);

GdkPixbuf *create_pixbuf_at_size(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_width, gint scale_height);

GdkPixbuf *create_round_pixbuf(const guchar *pixbuf_data, gint pixbuf_len,
		gint scale_size);

GdkPixbuf *create_presence_pixbuf(gint presence, gint scale_size);

#ifdef __cplusplus
}
#endif

#endif /* Hybird_GTKUTILS_H */
