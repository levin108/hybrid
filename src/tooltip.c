#include "tooltip.h"
#include "gtkutils.h"
#include "blist.h"
#include "account.h"

HybridTooltip hybrid_tooltip;

static gboolean hybrid_tooltip_show(HybridTooltipData *data);
static void hybrid_tooltip_destroy();

static gboolean
widget_motion_cb(GtkWidget *widget, GdkEvent *event, HybridTooltipData *data)
{
	hybrid_tooltip_destroy();

	hybrid_tooltip.source = g_timeout_add(500, 
	                        (GSourceFunc)hybrid_tooltip_show, data);

	return FALSE;
}

static gboolean
tree_motion_cb(GtkWidget *widget, GdkEventMotion *event, HybridTooltipData *data)
{
	GtkTreePath *path;

	if (hybrid_tooltip.source > 0) {
		if ((event->y >= hybrid_tooltip.rect.y) && 
		   ((event->y - hybrid_tooltip.rect.height) <= hybrid_tooltip.rect.y)) {

			return FALSE;
		}
	}

	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget),
	                              event->x, event->y, &path, NULL, NULL, NULL);

	if (!path) {

		hybrid_tooltip_destroy();

		return FALSE;
	}

	gtk_tree_view_get_cell_area(GTK_TREE_VIEW(widget), path, NULL, &hybrid_tooltip.rect);
	gtk_tree_path_free(path);

	hybrid_tooltip_destroy();

	hybrid_tooltip.source = g_timeout_add(500, 
	                        (GSourceFunc)hybrid_tooltip_show, data);

	return FALSE;
}

static gboolean
widget_leave_cb(GtkWidget *widget, GdkEvent *event, HybridTooltipData *data)
{
	hybrid_tooltip_destroy();

	return FALSE;
}

static gboolean
expose_event_cb(GtkWidget *widget, GdkEventExpose *event,
		HybridTooltipData *data)
{
	if (data->tooltip_paint) {
		data->tooltip_paint(data);
	}

	return FALSE;
}

static void
destroy_cb(HybridTooltipData *data)
{
	g_free(data);
}

/**
 * The default hook function of creating the tooltip window.
 */
static gboolean
create_tooltip_default(HybridTooltipData *tip_data, gint *width, gint *height)
{
	gint w;
	gint h;
	gint layout_width;
	gint layout_height;
	PangoLayout *layout;
	gint text_width = 0;
	gint text_height = TOOLTIP_BORDER * 2;
	GSList *pos;

	w = TOOLTIP_BORDER * 2 + PORTRAIT_WIDTH +
		 PORTRAIT_MARGIN * 2 + SPACE_IMG_AND_WORD;

	h = TOOLTIP_BORDER * 2 + PORTRAIT_WIDTH + PORTRAIT_MARGIN * 2;

	if (tip_data->tooltip_init) {
		if (!tip_data->tooltip_init(tip_data)) {
			return FALSE;
		}
	}

	for (pos = tip_data->layouts; pos; pos = pos->next) {

		layout = (PangoLayout*)pos->data;

		pango_layout_get_size(layout, &layout_width, &layout_height);

		layout_width = PANGO_PIXELS(layout_width);
		layout_height = PANGO_PIXELS(layout_height);

		text_width = MAX(text_width, layout_width);
		text_height += layout_height;
	}

	w += text_width;
	h = MAX(h, text_height);

	if (width) {
		*width = w;
	}

	if (height) {
		*height = h;
	}

	return TRUE;
}

/**
 * The default hook function of painting the tooltip window.
 */
static void
paint_tooltip_default(HybridTooltipData *user_data)
{
	/* paint. */
	GtkStyle *style;
	GdkPixbuf *pixbuf;
	GtkWidget *tipwindow;
	gint window_width;
	gint window_height;
	PangoLayout *layout;
	GSList *pos;

	tipwindow = hybrid_tooltip.window;

	gtk_window_get_size(GTK_WINDOW(tipwindow), &window_width, &window_height);

	style = tipwindow->style;

	/* paint tooltip border */
	gtk_paint_flat_box(style, tipwindow->window, GTK_STATE_NORMAL, GTK_SHADOW_OUT,
	                   NULL, tipwindow, "tooltip", 0, 0, 
					   window_width, window_height);

	/* paint portrait border */
	gtk_paint_flat_box(style, tipwindow->window, GTK_STATE_NORMAL, GTK_SHADOW_OUT,
	                   NULL, tipwindow, "tooltip", TOOLTIP_BORDER, TOOLTIP_BORDER, 
					   PORTRAIT_WIDTH + PORTRAIT_MARGIN * 2, 
					   PORTRAIT_WIDTH + PORTRAIT_MARGIN * 2);

	pixbuf = hybrid_create_round_pixbuf(NULL, 0, PORTRAIT_WIDTH);

	/* paint portrait */
	gdk_draw_pixbuf(GDK_DRAWABLE(tipwindow->window), NULL, user_data->icon, 0, 0,
	                  TOOLTIP_BORDER + PORTRAIT_MARGIN, TOOLTIP_BORDER + PORTRAIT_MARGIN,
					  PORTRAIT_WIDTH, PORTRAIT_WIDTH, GDK_RGB_DITHER_NONE, 0, 0);

	g_object_unref(pixbuf);

	gint width, height;

	gint h = TOOLTIP_BORDER;

	/* paint markups */
	for (pos = user_data->layouts; pos; pos = pos->next) {

		layout = (PangoLayout*)pos->data;
		gtk_paint_layout(style, tipwindow->window, GTK_STATE_NORMAL, FALSE,
		                 NULL, tipwindow, "tooltip", 
						 TOOLTIP_BORDER + PORTRAIT_WIDTH + PORTRAIT_MARGIN * 2 + SPACE_IMG_AND_WORD,
						 h, layout);

		pango_layout_get_size(layout, &width, &height);

		h += PANGO_PIXELS(height);

		g_object_unref(layout);
	}

	g_slist_free(user_data->layouts);
	user_data->layouts = NULL;
}


static void
setup_tooltip_window_position(gpointer data, int w, int h)
{
	int sig;
	int scr_w, scr_h, x, y, dy;
	int mon_num;
	GdkScreen *screen = NULL;
	GdkRectangle mon_size;
	GtkWidget *tipwindow = hybrid_tooltip.window;

	gdk_display_get_pointer(gdk_display_get_default(), &screen, &x, &y, NULL);
	mon_num = gdk_screen_get_monitor_at_point(screen, x, y);
	gdk_screen_get_monitor_geometry(screen, mon_num, &mon_size);

	scr_w = mon_size.width + mon_size.x;
	scr_h = mon_size.height + mon_size.y;

	dy = gdk_display_get_default_cursor_size(gdk_display_get_default()) / 2;

	if (w > mon_size.width)
		w = mon_size.width - 10;

	if (h > mon_size.height)
		h = mon_size.height - 10;

	x -= ((w >> 1) + 4);

	if ((y + h + 4) > scr_h)
		y = y - h - dy - 5;
	else
		y = y + dy + 6;

	if (y < mon_size.y)
		y = mon_size.y;

	if (y != mon_size.y) {
		if ((x + w) > scr_w)
			x -= (x + w + 5) - scr_w;
		else if (x < mon_size.x)
			x = mon_size.x;
	} else {
		x -= (w / 2 + 10);
		if (x < mon_size.x)
			x = mon_size.x;
	}

	gtk_widget_set_size_request(tipwindow, w, h);
	gtk_window_move(GTK_WINDOW(tipwindow), x, y);
	gtk_widget_show(tipwindow);

	/* Hide the tooltip when the widget is destroyed */
	sig = g_signal_connect(G_OBJECT(hybrid_tooltip.widget), "destroy",
	                       G_CALLBACK(hybrid_tooltip_destroy), NULL);
	g_signal_connect_swapped(G_OBJECT(tipwindow), "destroy",
	                         G_CALLBACK(g_source_remove),
							 GINT_TO_POINTER(sig));

	g_signal_connect(G_OBJECT(tipwindow), "expose-event",
	                 G_CALLBACK(expose_event_cb), data);
}

static gboolean
hybrid_tooltip_show(HybridTooltipData *data)
{

	GtkWidget *window;
	gint width;
	gint height;

	window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_TOOLTIP);
	gtk_widget_set_app_paintable(window, TRUE);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_widget_ensure_style(window);
	gtk_widget_realize(window);
	gtk_widget_set_name(window, "gtk-tooltips");

	hybrid_tooltip.widget = data->widget;
	hybrid_tooltip.window = window;
	
	if (!data->tooltip_create) {

		hybrid_tooltip_destroy();

		return FALSE;
	}

	if (!data->tooltip_create(data, &width, &height)) {

		hybrid_tooltip_destroy();

		return FALSE;
	}

	setup_tooltip_window_position(data, width, height);

	return FALSE;
}

static void
hybrid_tooltip_destroy()
{
	if (hybrid_tooltip.source > 0) {
		g_source_remove(hybrid_tooltip.source);
		hybrid_tooltip.source = 0;
	}

	if (hybrid_tooltip.window) {
		gtk_widget_destroy(hybrid_tooltip.window);
		hybrid_tooltip.window = NULL;
	}
}

void
hybrid_tooltip_setup(GtkWidget *widget,
                     HybridTooltipCreate create_tooltip,
                     HybridTooltipPaint paint_tooltip,
					 HybridTooltipInit init_tooltip,
                     gpointer user_data)
{
	HybridTooltipData *data;

	g_return_if_fail(widget != NULL);

	data = g_new0(HybridTooltipData, 1);
	data->widget = widget;
	data->user_data = user_data;

	if (paint_tooltip) {
		data->tooltip_paint = paint_tooltip;

	} else {
		data->tooltip_paint = paint_tooltip_default;
	}

	if (create_tooltip) {
		data->tooltip_create = create_tooltip;

	} else {
		data->tooltip_create = create_tooltip_default;
	}

	data->tooltip_init = init_tooltip;

	gtk_widget_add_events(widget,
	                      GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);

	if (GTK_IS_TREE_VIEW(widget)) {
		g_signal_connect(G_OBJECT(widget), "motion-notify-event",
						 G_CALLBACK(tree_motion_cb), data);

	} else {
		g_signal_connect(G_OBJECT(widget), "motion-notify-event",
						 G_CALLBACK(widget_motion_cb), data);
	}

	g_signal_connect(G_OBJECT(widget), "leave-notify-event",
	                 G_CALLBACK(widget_leave_cb), data);
	g_signal_connect(G_OBJECT(widget), "scroll-event",
	                 G_CALLBACK(widget_leave_cb), data);
	g_signal_connect_swapped(G_OBJECT(widget), "destroy",
	                 G_CALLBACK(destroy_cb), data);
}


PangoLayout*
create_pango_layout(const gchar *markup, gint *width, gint *height)
{
	PangoLayout *layout;
	gint w, h;

	g_return_val_if_fail(markup != NULL, NULL);

	layout = gtk_widget_create_pango_layout(hybrid_tooltip.window, NULL);
	pango_layout_set_markup(layout, markup, -1);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
	pango_layout_set_width(layout, 300000);

	pango_layout_get_size(layout, &w, &h);

	if (width) {
		*width = PANGO_PIXELS(w);
	}

	if (height) {
		*height = PANGO_PIXELS(h);
	}

	return layout;
}

void
hybrid_tooltip_data_add_title(HybridTooltipData *data, const gchar *title)
{
	PangoLayout *layout;
	gchar *markup;
	gchar *escaped_value = NULL;

	g_return_if_fail(data != NULL);
	g_return_if_fail(title != NULL);

	escaped_value = g_markup_escape_text(title, -1);

	markup = g_strdup_printf("<b><span size='x-large'>%s</span></b>",
					escaped_value);

	g_free(escaped_value);

	layout = create_pango_layout(markup, NULL, NULL);

	g_free(markup);

	data->layouts = g_slist_append(data->layouts, layout);
}

void
hybrid_tooltip_data_add_pair(HybridTooltipData *data, const gchar *name,
                                  const gchar *value)
{
	PangoLayout *layout;
	gchar *markup;
	gchar *escaped_name;
	gchar *escaped_value = NULL;

	g_return_if_fail(data != NULL);
	g_return_if_fail(name != NULL);

	escaped_name  = g_markup_escape_text(name, -1);

	if (value) {
		escaped_value = g_markup_escape_text(value, -1);

	} else {
		escaped_value = g_strdup("");
	}

	markup = g_strdup_printf("<b>%s:</b> %s", escaped_name, escaped_value);

	g_free(escaped_name);
	g_free(escaped_value);

	layout = create_pango_layout(markup, NULL, NULL);

	g_free(markup);

	data->layouts = g_slist_append(data->layouts, layout);
}

void
hybrid_tooltip_data_add_pair_markup(HybridTooltipData *data, const gchar *name,
                                  const gchar *value)
{
	PangoLayout *layout;
	gchar *markup;
	gchar *escaped_name;

	g_return_if_fail(data != NULL);
	g_return_if_fail(name != NULL);

	escaped_name  = g_markup_escape_text(name, -1);

	markup = g_strdup_printf("<b>%s:</b> %s", escaped_name, value);

	g_free(escaped_name);

	layout = create_pango_layout(markup, NULL, NULL);

	g_free(markup);

	data->layouts = g_slist_append(data->layouts, layout);
}
