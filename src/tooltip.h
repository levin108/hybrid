#ifndef HYBRID_TOOLTIP_H
#define HYBRID_TOOLTIP_H
#include <gtk/gtk.h>

typedef struct _HybridTooltip HybridTooltip;
typedef struct _HybridTooltipData HybridTooltipData;

/**
 * Hook functiion fot painting the tooltip, this function should
 * be used in expose event of the tooltip window, to paint the details
 * in the tooltip window.
 */
typedef void (*HybridTooltipPaint)(HybridTooltipData *user_data);

/**
 * Hook function for creating the tooltip window, what it shoud do is
 * to calculate the size of the tooltip window, so that we can use
 * the size data to paint the tooltip window.
 */
typedef gboolean (*HybridTooltipCreate)(HybridTooltipData *user_data,
                                    gint *width, gint *height);

/**
 * Hook function for initializing the data in the tooltip window.
 */
typedef gboolean (*HybridTooltipInit)(HybridTooltipData *user_data);

struct _HybridTooltip {
	GtkWidget *widget;
	GtkWidget *window;
	GdkRectangle rect;
	guint source;
};

struct _HybridTooltipData {
	GtkWidget *widget;
	HybridTooltipPaint tooltip_paint;
	HybridTooltipCreate tooltip_create;
	HybridTooltipInit tooltip_init;
	GdkPixbuf *icon;
	GSList *layouts;
	gpointer user_data;
};

#define TOOLTIP_BORDER 10
#define PORTRAIT_WIDTH 96
#define PORTRAIT_MARGIN 1
#define SPACE_IMG_AND_WORD 10

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Setup the tooltip for a given widget, note that the widget should
 * have a GdkWindow associated with, if you're not sure, use the 
 * GTK_WIDGET_NO_WINDOW macro to check it.
 *
 * @param widget         The widget for the tooltip.
 * @param tooltip_create Callback function to create the tooltip
 * @param tooltip_paint  Callback function to paint the tooltip
 * @param tooltip_init   Callback function to initialize the data in the tooltip window.
 * @param user_data      User-specified data;
 */
void hybrid_tooltip_setup(GtkWidget *widget,
                          HybridTooltipCreate tooltip_create,
                          HybridTooltipPaint tooltip_paint,
						  HybridTooltipInit tooltip_init,
						  gpointer user_data);

/**
 * Create an pango layout with markup text, the width and height will
 * be set with the size of the pango layout.
 */
PangoLayout *create_pango_layout(const gchar *markup,
                                 gint *width, gint *height);

/**
 * Add tooltip title to the tooltip data.
 *
 * @param data  The tooltip data.
 * @param title The tooltip title.
 */
void hybrid_tooltip_data_add_title(HybridTooltipData *data, const gchar *title);

/**
 * Add name-value pair to the tooltip data.
 *
 * @param data  The tooltip data.
 * @param name  The attribute name.
 * @param value The attribute value.
 */
void hybrid_tooltip_data_add_pair(HybridTooltipData *data, const gchar *name,
                                  const gchar *value);

/**
 * Add name-value pair to the tooltip data, the value accepts markup string.
 *
 * @param data  The tooltip data.
 * @param name  The attribute name.
 * @param value The attribute value.
 */
void
hybrid_tooltip_data_add_pair_markup(HybridTooltipData *data, const gchar *name,
                                  const gchar *value);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_TOOLTIP_H */
