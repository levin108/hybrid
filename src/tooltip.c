#include "tooltip.h"
#include "gtkutils.h"

HybridTooltip hybrid_tooltip;

static gboolean hybrid_tooltip_show(HybridTooltipData *data);
static void hybrid_tooltip_destroy();

static gboolean
widget_motion_cb(GtkWidget *widget, GdkEvent *event, HybridTooltipData *data)
{
	hybrid_tooltip_destroy();

	hybrid_tooltip.source = g_timeout_add(400, (GSourceFunc)hybrid_tooltip_show, data);

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
	/* paint. */
	GtkStyle *style;
	GdkPixbuf *pixbuf;
	GtkWidget *tipwindow;
	gint window_width;
	gint window_height;

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

	gdk_draw_pixbuf(GDK_DRAWABLE(tipwindow->window), NULL, pixbuf, 0, 0,
	                  TOOLTIP_BORDER + PORTRAIT_MARGIN, TOOLTIP_BORDER + PORTRAIT_MARGIN,
					  PORTRAIT_WIDTH, PORTRAIT_WIDTH, GDK_RGB_DITHER_NONE, 0, 0);

	g_object_unref(pixbuf);

	return FALSE;
}

static void
destroy_cb(HybridTooltipData *data)
{
	g_free(data);
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

	window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_TOOLTIP);
	gtk_widget_set_app_paintable(window, TRUE);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_widget_ensure_style(window);
	gtk_widget_realize(window);
	gtk_widget_set_name(window, "gtk-tooltips");

	hybrid_tooltip.widget = data->widget;
	hybrid_tooltip.window = window;
	

	setup_tooltip_window_position(NULL, 300, 200);

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
hybrid_tooltip_setup(GtkWidget *widget, gpointer user_data)
{
	HybridTooltipData *data;

	g_return_if_fail(widget != NULL);

	data = g_new0(HybridTooltipData, 1);
	data->widget = widget;
	data->user_data = user_data;

	gtk_widget_add_events(widget,
	                      GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);

	g_signal_connect(G_OBJECT(widget), "motion-notify-event",
	                 G_CALLBACK(widget_motion_cb), data);
	g_signal_connect(G_OBJECT(widget), "leave-notify-event",
	                 G_CALLBACK(widget_leave_cb), data);
	g_signal_connect(G_OBJECT(widget), "scroll-event",
	                 G_CALLBACK(widget_leave_cb), data);
	g_signal_connect_swapped(G_OBJECT(widget), "destroy",
	                 G_CALLBACK(destroy_cb), data);
}
