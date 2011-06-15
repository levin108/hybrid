#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include "util.h"
#include "blist.h"
#include "conv.h"
#include "gtkutils.h"
#include "gtkcellrendererexpander.h"

GSList *group_list = NULL;
GSList *buddy_list = NULL;

IMBlist *blist = NULL;


IMBlist*
im_blist_create()
{
	IMBlist *imb = g_new0(IMBlist, 1);
	return imb;
}

static void
render_column(IMBlist *blist)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	/* expander columns */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(blist->treeview), column);
	gtk_tree_view_column_set_visible(column, FALSE);
	gtk_tree_view_set_expander_column(GTK_TREE_VIEW(blist->treeview), column);

	/* main column */
	blist->column = gtk_tree_view_column_new ();
	gtk_tree_view_append_column(GTK_TREE_VIEW(blist->treeview), blist->column);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(blist->treeview));
	//gtk_tree_view_column_set_sizing(blist->column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	/* group expander */
	renderer = pidgin_cell_renderer_expander_new();
	g_object_set(renderer, "expander-visible", TRUE, NULL);
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
					    "visible", IM_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE,
					    NULL);

	/* contact expander */
	renderer = pidgin_cell_renderer_expander_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
					    "visible", IM_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE,
					    NULL);

	/* portrait */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"pixbuf", IM_BLIST_BUDDY_ICON,
						"visible", IM_BLIST_BUDDY_ICON_COLUMN_VISIBLE,
						NULL);
	g_object_set(renderer, "xalign", 1.0, "xpad", 3, "ypad", 0, NULL);

	/* name */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"markup", IM_BLIST_BUDDY_NAME,
						NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 3, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	/* protocol icon */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"pixbuf", IM_BLIST_PROTO_ICON,
						"visible", IM_BLIST_PROTO_ICON_COLUMN_VISIBLE,
						NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);

	/* status icon */
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"pixbuf", IM_BLIST_STATUS_ICON,
						"visible", IM_BLIST_STATUS_ICON_COLUMN_VISIBLE,
						NULL);

}

static gboolean
button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	return FALSE;
}

static void 
row_activated_cb(GtkTreeView *treeview, GtkTreePath *path,
		GtkTreeViewColumn *col,	gpointer user_data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *id;
	IMBuddy *buddy;

	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter(model, &iter, path);

	gtk_tree_model_get(model, &iter,
			IM_BLIST_BUDDY_ID, &id, -1);

	if (!(buddy = im_blist_find_buddy(id))) {
		im_debug_error("blist", "FATAL ERROR, find buddy \'%s\'", id);
		g_free(id);
		return;
	}

	IMChatPanel *chat = im_chat_panel_create(buddy);

	g_free(id);
}

void 
im_blist_init()
{
	blist = im_blist_create();

	blist->treemodel = gtk_tree_store_new(IM_BLIST_COLUMNS,
			G_TYPE_STRING,
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING,
			GDK_TYPE_PIXBUF,
			GDK_TYPE_PIXBUF,
			G_TYPE_INT,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN);

	blist->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				blist->treemodel));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(blist->treeview), FALSE);

	render_column(blist);

	gtk_tree_sortable_set_sort_column_id(
			GTK_TREE_SORTABLE(blist->treemodel),
			IM_BLIST_BUDDY_STATE, GTK_SORT_DESCENDING);

	g_signal_connect(blist->treeview, "button-press-event",
			G_CALLBACK(button_press_cb), NULL);

	g_signal_connect(blist->treeview, "row-activated",
			G_CALLBACK(row_activated_cb), NULL);

}

IMGroup*
im_blist_add_group(IMAccount *ac, const gchar *id, const gchar *name)
{
	g_return_val_if_fail(name != NULL && blist != NULL, NULL);

	gchar *temp;
	IMGroup *group = g_new0(IMGroup, 1);
	GdkPixbuf *proto_icon = gdk_pixbuf_new_from_file_at_size(
			DATA_DIR"/msn.png", 16, 16, NULL);

	gtk_tree_store_append(blist->treemodel, &group->iter, NULL);

	temp = g_strdup_printf("<b>%s</b>", name);

	gtk_tree_store_set(blist->treemodel, &group->iter,
			IM_BLIST_BUDDY_NAME, temp,
			IM_BLIST_PROTO_ICON, proto_icon,
			IM_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE, TRUE,
			IM_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE, FALSE,
			IM_BLIST_STATUS_ICON_COLUMN_VISIBLE, FALSE,
			IM_BLIST_PROTO_ICON_COLUMN_VISIBLE, TRUE,
			IM_BLIST_BUDDY_ICON_COLUMN_VISIBLE, FALSE,
			-1);

	g_free(temp);

	group->name = g_strdup(name);
	group->id = g_strdup(id);
	group->account = ac;

	group_list = g_slist_append(group_list, group);

	g_object_unref(proto_icon);

	return group;
}

IMBuddy*
im_blist_add_buddy(IMAccount *ac, IMGroup *parent, const gchar *id,
		const gchar *name)
{
	GdkPixbuf *status_icon;
	GdkPixbuf *proto_icon;
	guchar *status_icon_data;
	gsize status_icon_data_length;

	g_return_val_if_fail(blist != NULL, NULL);
	g_return_val_if_fail(parent != NULL, NULL);
	g_return_val_if_fail(id != NULL, NULL);

	status_icon = gdk_pixbuf_new_from_file(DATA_DIR"/available.png", NULL);
	proto_icon = gdk_pixbuf_new_from_file(DATA_DIR"/msn.png", NULL);

	IMBuddy *buddy = g_new0(IMBuddy, 1);
	
	gtk_tree_store_append(blist->treemodel, &buddy->iter, &parent->iter);

	gtk_tree_store_set(blist->treemodel, &buddy->iter,
			IM_BLIST_BUDDY_ID, id,
			IM_BLIST_STATUS_ICON, status_icon,
			IM_BLIST_PROTO_ICON, proto_icon,
			IM_BLIST_BUDDY_NAME, name,
			IM_BLIST_BUDDY_STATE, IM_STATE_OFFLINE,
			IM_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE, FALSE,
			IM_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE, FALSE,
			IM_BLIST_STATUS_ICON_COLUMN_VISIBLE, TRUE,
			IM_BLIST_PROTO_ICON_COLUMN_VISIBLE, TRUE,
			IM_BLIST_BUDDY_ICON_COLUMN_VISIBLE, TRUE,
			-1);

	buddy->id = g_strdup(id);
	buddy->name = g_strdup(name);
	buddy->account = ac;

	buddy_list = g_slist_append(buddy_list, buddy);

	g_file_get_contents(DATA_DIR"/icon.png", (gchar**)&status_icon_data,
			&status_icon_data_length, NULL);
	im_blist_set_buddy_icon(buddy, status_icon_data, status_icon_data_length);

	g_free(status_icon_data);

	g_object_unref(status_icon);
	g_object_unref(proto_icon);

	return buddy;
}

/**
 * Set the name field.
 */
static void
im_blist_set_name_field(IMBuddy *buddy)
{
	gchar *text;
	gchar *mood;
	gchar *tmp;
	const gchar *name;

	g_return_if_fail(buddy != NULL);

	if (buddy->name && *(buddy->name) != '\0') {
		name = buddy->name;

	} else {
		name = buddy->id;
	}

	if (buddy->mood && *(buddy->mood) != '\0') {

		tmp = g_markup_escape_text(buddy->mood, -1);

		mood = g_strdup_printf(
				"\n<small><span color=\"#8f8f8f\">%s</span></small>", 
				tmp);
		g_free(tmp);

	} else {
		mood = g_strdup("");
	}

	tmp = g_markup_escape_text(name, -1);

	text = g_strdup_printf("%s%s", tmp, mood);

	gtk_tree_store_set(blist->treemodel, &buddy->iter,
			IM_BLIST_BUDDY_NAME, text, -1);

	g_free(mood);
	g_free(text);
}

/**
 * Set the buddy state.
 */
static void
im_blist_set_state_field(IMBuddy *buddy)
{
	GdkPixbuf *pixbuf;
	GdkPixbufLoader *loader;
	GError *err = NULL;

	g_return_if_fail(buddy != NULL);

	pixbuf = create_presence_pixbuf(buddy->state, 16);
	
	gtk_tree_store_set(blist->treemodel, &buddy->iter,
			IM_BLIST_BUDDY_STATE, buddy->state,
			IM_BLIST_STATUS_ICON, pixbuf, -1);

	g_object_unref(pixbuf);
	pixbuf = NULL;

	/* set the portrait */
	gint scale_size = 32;

	loader = gdk_pixbuf_loader_new();

	if (buddy->icon_data == NULL || buddy->icon_data_length == 0) {
		/* load the default icon. */
		if (!g_file_get_contents(DATA_DIR"/icon.png", 
					(gchar**)&buddy->icon_data,
					&buddy->icon_data_length, &err)) {

			im_debug_error("blist", "load the default icon:%s", err->message);
			g_error_free(err);
			return;
		}
	}

	pixbuf = create_round_pixbuf(buddy->icon_data, buddy->icon_data_length,
			scale_size);

	/* If buddy is not online, show a grey icon. */
	if (BUDDY_IS_INVISIBLE(buddy) || BUDDY_IS_OFFLINE(buddy)) {
		gdk_pixbuf_saturate_and_pixelate(pixbuf, pixbuf, 0.0, FALSE);
	}

	gtk_tree_store_set(blist->treemodel, &buddy->iter,
			IM_BLIST_BUDDY_ICON, pixbuf, -1);

	g_object_unref(pixbuf);

}

void
im_blist_set_buddy_name(IMBuddy *buddy, const gchar *name)
{
	g_return_if_fail(buddy != NULL);

	g_free(buddy->name);
	buddy->name = g_strdup(name);

	im_blist_set_name_field(buddy);
}

void
im_blist_set_buddy_mood(IMBuddy *buddy, const gchar *mood)
{
	g_return_if_fail(buddy != NULL);

	g_free(buddy->mood);
	buddy->mood = g_strdup(mood);

	im_blist_set_name_field(buddy);
}

void
im_blist_set_buddy_icon(IMBuddy *buddy, const guchar *icon_data, gsize len)
{
	g_return_if_fail(buddy != NULL);

	g_free(buddy->icon_data);
	buddy->icon_data = NULL;

	if (icon_data != NULL) {
		buddy->icon_data = g_memdup(icon_data, len);
		buddy->icon_data_length = len;
	}

	im_blist_set_state_field(buddy);
}

void
im_blist_set_buddy_state(IMBuddy *buddy, gint state)
{
	g_return_if_fail(buddy != NULL);

	buddy->state = state;

	im_blist_set_state_field(buddy);
}

IMGroup*
im_blist_find_group_by_id(const gchar *id)
{
	GSList *pos;
	IMGroup *group;

	g_return_val_if_fail(id != NULL, NULL);

	for (pos = group_list; pos; pos = pos->next) {
		group = (IMGroup*)pos->data;

		if (g_strcmp0(group->id, id) == 0) {
			return group;
		}
	}

	return NULL;
}

IMGroup*
im_blist_find_group_by_name(const gchar *name)
{
	GSList *pos;
	IMGroup *group;

	g_return_val_if_fail(name != NULL, NULL);

	for (pos = group_list; pos; pos = pos->next) {
		group = (IMGroup*)pos->data;

		if (g_strcmp0(group->name, name) == 0) {
			return group;
		}
	}

	return NULL;
}

IMBuddy*
im_blist_find_buddy(const gchar *id)
{
	GSList *pos;
	IMBuddy *buddy;

	g_return_val_if_fail(id != NULL, NULL);

	for (pos = buddy_list; pos; pos = pos->next) {
		buddy = (IMBuddy*)pos->data;

		if (g_strcmp0(buddy->id, id) == 0) {
			return buddy;
		}
	}

	return NULL;
}
