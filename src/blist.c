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

HybirdBlist *blist = NULL;

static void hybird_blist_buddy_icon_save(HybirdBuddy *buddy);

HybirdBlist*
hybird_blist_create()
{
	HybirdBlist *imb = g_new0(HybirdBlist, 1);
	return imb;
}

static void
render_column(HybirdBlist *blist)
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
					    "visible", HYBIRD_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE,
					    NULL);

	/* contact expander */
	renderer = pidgin_cell_renderer_expander_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
					    "visible", HYBIRD_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE,
					    NULL);

	/* portrait */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"pixbuf", HYBIRD_BLIST_BUDDY_ICON,
						"visible", HYBIRD_BLIST_BUDDY_ICON_COLUMN_VISIBLE,
						NULL);
	g_object_set(renderer, "xalign", 1.0, "xpad", 3, "ypad", 0, NULL);

	/* name */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"markup", HYBIRD_BLIST_BUDDY_NAME,
						NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 3, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	/* protocol icon */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"pixbuf", HYBIRD_BLIST_PROTO_ICON,
						"visible", HYBIRD_BLIST_PROTO_ICON_COLUMN_VISIBLE,
						NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);

	/* status icon */
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);
	gtk_tree_view_column_pack_start(blist->column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(blist->column, renderer,
						"pixbuf", HYBIRD_BLIST_STATUS_ICON,
						"visible", HYBIRD_BLIST_STATUS_ICON_COLUMN_VISIBLE,
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
	HybirdBuddy *buddy;
	gint depth;

	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter(model, &iter, path);

	depth = gtk_tree_path_get_depth(path);

	if (depth > 1) {

		gtk_tree_model_get(model, &iter,
				HYBIRD_BLIST_OBJECT_COLUMN, &buddy, -1);

		hybird_chat_panel_create(buddy);
	}
}

void 
hybird_blist_init()
{
	blist = hybird_blist_create();

	blist->treemodel = gtk_tree_store_new(HYBIRD_BLIST_COLUMNS,
			G_TYPE_STRING,
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING,
			GDK_TYPE_PIXBUF,
			GDK_TYPE_PIXBUF,
			G_TYPE_INT,
			G_TYPE_POINTER,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN);

	blist->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				blist->treemodel));
	g_object_unref(blist->treemodel);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(blist->treeview), FALSE);

	render_column(blist);

	gtk_tree_sortable_set_sort_column_id(
			GTK_TREE_SORTABLE(blist->treemodel),
			HYBIRD_BLIST_BUDDY_STATE, GTK_SORT_DESCENDING);

	g_signal_connect(blist->treeview, "button-press-event",
			G_CALLBACK(button_press_cb), NULL);

	g_signal_connect(blist->treeview, "row-activated",
			G_CALLBACK(row_activated_cb), NULL);

}

HybirdGroup*
hybird_blist_add_group(HybirdAccount *ac, const gchar *id, const gchar *name)
{
	g_return_val_if_fail(name != NULL && blist != NULL, NULL);

	gchar *temp;
	HybirdGroup *group;
	GdkPixbuf *proto_icon;

	g_return_val_if_fail(ac != NULL, NULL);
	g_return_val_if_fail(id != NULL, NULL);
	g_return_val_if_fail(name != NULL, NULL);

	/*
	 * Make sure we will add a group to an account only
	 * when the account's connection status is CONNECTED 
	 */
	if (!HYBIRD_IS_CONNECTED(ac)) {
		return NULL;
	}

	/*
	 * If current account has a group exists with the specified ID,
	 * then just return the existing group.
	 */
	if ((group = hybird_blist_find_group(ac, id))) {
		return group;
	}

	group = g_new0(HybirdGroup, 1);
	proto_icon  = gdk_pixbuf_new_from_file_at_size(
			DATA_DIR"/msn.png", 16, 16, NULL);

	gtk_tree_store_append(blist->treemodel, &group->iter, NULL);

	temp = g_strdup_printf("<b>%s</b>", name);

	gtk_tree_store_set(blist->treemodel, &group->iter,
			HYBIRD_BLIST_BUDDY_NAME, temp,
			HYBIRD_BLIST_PROTO_ICON, proto_icon,
			HYBIRD_BLIST_OBJECT_COLUMN, group,
			HYBIRD_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE, TRUE,
			HYBIRD_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE, FALSE,
			HYBIRD_BLIST_STATUS_ICON_COLUMN_VISIBLE, FALSE,
			HYBIRD_BLIST_PROTO_ICON_COLUMN_VISIBLE, TRUE,
			HYBIRD_BLIST_BUDDY_ICON_COLUMN_VISIBLE, FALSE,
			-1);

	g_free(temp);

	group->name = g_strdup(name);
	group->id = g_strdup(id);
	group->account = ac;

	group_list = g_slist_append(group_list, group);

	g_object_unref(proto_icon);

	return group;
}

void
hybird_blist_group_destroy(HybirdGroup *group)
{
	if (group) {
		g_free(group->id);
		g_free(group->name);

		g_free(group);
	}
}

HybirdBuddy*
hybird_blist_add_buddy(HybirdAccount *ac, HybirdGroup *parent, const gchar *id,
		const gchar *name)
{
	GdkPixbuf *status_icon;
	GdkPixbuf *proto_icon;
	HybirdBuddy *buddy;

	g_return_val_if_fail(blist != NULL, NULL);
	g_return_val_if_fail(parent != NULL, NULL);
	g_return_val_if_fail(id != NULL, NULL);

	/*
	 * Make sure we will add a buddy to an account only
	 * when the account's connection status is CONNECTED 
	 */
	if (!HYBIRD_IS_CONNECTED(ac)) {
		return NULL;
	}

	/*
	 * If current account has a buddy exists with the specified ID,
	 * then just return the existing buddy.
	 */
	if ((buddy = hybird_blist_find_buddy(ac, id))) {
		return buddy;
	}

	status_icon = create_presence_pixbuf(HYBIRD_STATE_OFFLINE, 32);
	proto_icon = gdk_pixbuf_new_from_file(DATA_DIR"/msn.png", NULL);

	buddy = g_new0(HybirdBuddy, 1);
	
	gtk_tree_store_append(blist->treemodel, &buddy->iter, &parent->iter);

	gtk_tree_store_set(blist->treemodel, &buddy->iter,
			HYBIRD_BLIST_BUDDY_ID, id,
			HYBIRD_BLIST_STATUS_ICON, status_icon,
			HYBIRD_BLIST_PROTO_ICON, proto_icon,
			HYBIRD_BLIST_BUDDY_NAME, name,
			HYBIRD_BLIST_OBJECT_COLUMN, buddy,
			HYBIRD_BLIST_BUDDY_STATE, HYBIRD_STATE_OFFLINE,
			HYBIRD_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE, FALSE,
			HYBIRD_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE, FALSE,
			HYBIRD_BLIST_STATUS_ICON_COLUMN_VISIBLE, TRUE,
			HYBIRD_BLIST_PROTO_ICON_COLUMN_VISIBLE, TRUE,
			HYBIRD_BLIST_BUDDY_ICON_COLUMN_VISIBLE, TRUE,
			-1);

	buddy->id = g_strdup(id);
	buddy->name = g_strdup(name);
	buddy->account = ac;
	buddy->parent = parent;

	buddy_list = g_slist_append(buddy_list, buddy);

	//hybird_blist_set_buddy_icon(buddy, NULL, 0, NULL);

	g_object_unref(status_icon);
	g_object_unref(proto_icon);

	hybird_blist_buddy_to_cache(buddy, HYBIRD_BLIST_CACHE_ADD);

	return buddy;
}

const gchar*
hybird_blist_get_buddy_checksum(HybirdBuddy *buddy)
{
	g_return_val_if_fail(buddy != NULL, NULL);

	return buddy->icon_crc;
}

void
hybird_blist_buddy_destroy(HybirdBuddy *buddy)
{
	if (buddy) {
		g_free(buddy->id);
		g_free(buddy->name);
		g_free(buddy->mood);
		g_free(buddy->icon_name);
		g_free(buddy->icon_data);
		g_free(buddy->icon_crc);
		g_free(buddy);
	}
}

/**
 * Set the name field.
 */
static void
hybird_blist_set_name_field(HybirdBuddy *buddy)
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
			HYBIRD_BLIST_BUDDY_NAME, text, -1);

	g_free(mood);
	g_free(text);
}

/**
 * Set the buddy state.
 */
static void
hybird_blist_set_state_field(HybirdBuddy *buddy)
{
	GdkPixbuf *pixbuf;
	GError *err = NULL;
	guchar *icon_data;
	gsize icon_data_length;

	g_return_if_fail(buddy != NULL);

	pixbuf = create_presence_pixbuf(buddy->state, 16);
	
	gtk_tree_store_set(blist->treemodel, &buddy->iter,
			HYBIRD_BLIST_BUDDY_STATE, buddy->state,
			HYBIRD_BLIST_STATUS_ICON, pixbuf, -1);

	g_object_unref(pixbuf);
	pixbuf = NULL;

	/* set the portrait */
	gint scale_size = 32;

	if (buddy->icon_data == NULL || buddy->icon_data_length == 0) {
		/* 
		 * Load the default icon. Note that we don't set the buddy's
		 * icon_data attribute to the data we got from the default icon,
		 * because if the icon_data attribute is not NULL, it will try
		 * to localize the icon, but we don't want to localize the 
		 * default icon, so if buddy doesn't have a self-defined icon,
		 * keep its icon_data attribute NULL :) .
		 */
		if (!g_file_get_contents(DATA_DIR"/icon.png", 
					(gchar**)&icon_data, &icon_data_length, &err)) {

			hybird_debug_error("blist", "load the default icon:%s", err->message);
			g_error_free(err);
			return;
		}

		pixbuf = create_round_pixbuf(icon_data, icon_data_length, scale_size);

	} else {
		pixbuf = create_round_pixbuf(buddy->icon_data, buddy->icon_data_length,
				scale_size);
	}

	/* If buddy is not online, show a grey icon. */
	if (BUDDY_IS_INVISIBLE(buddy) || BUDDY_IS_OFFLINE(buddy)) {
		gdk_pixbuf_saturate_and_pixelate(pixbuf, pixbuf, 0.0, FALSE);
	}

	gtk_tree_store_set(blist->treemodel, &buddy->iter,
			HYBIRD_BLIST_BUDDY_ICON, pixbuf, -1);

	g_object_unref(pixbuf);

}

void
hybird_blist_set_buddy_name(HybirdBuddy *buddy, const gchar *name)
{
	g_return_if_fail(buddy != NULL);

	g_free(buddy->name);
	buddy->name = g_strdup(name);

	hybird_blist_set_name_field(buddy);
	hybird_blist_buddy_to_cache(buddy, HYBIRD_BLIST_CACHE_UPDATE_NAME);
}

void
hybird_blist_set_buddy_mood(HybirdBuddy *buddy, const gchar *mood)
{
	g_return_if_fail(buddy != NULL);

	g_free(buddy->mood);
	buddy->mood = g_strdup(mood);

	hybird_blist_set_name_field(buddy);
	hybird_blist_buddy_to_cache(buddy, HYBIRD_BLIST_CACHE_UPDATE_MOOD);
}

void
hybird_blist_set_buddy_icon(HybirdBuddy *buddy, const guchar *icon_data,
		gsize len, const gchar *crc)
{
	g_return_if_fail(buddy != NULL);

	g_free(buddy->icon_crc);
	g_free(buddy->icon_data);
	buddy->icon_data = NULL;
	buddy->icon_data_length = len;
	buddy->icon_crc = g_strdup(crc);

	if (icon_data != NULL) {
		buddy->icon_data = g_memdup(icon_data, len);
	}

	hybird_blist_set_state_field(buddy);
	hybird_blist_buddy_icon_save(buddy);
	hybird_blist_buddy_to_cache(buddy, HYBIRD_BLIST_CACHE_UPDATE_ICON);
}

void
hybird_blist_set_buddy_state(HybirdBuddy *buddy, gint state)
{
	g_return_if_fail(buddy != NULL);

	buddy->state = state;

	hybird_blist_set_state_field(buddy);
}

HybirdGroup*
hybird_blist_find_group(HybirdAccount *account, const gchar *id)
{
	GSList *pos;
	HybirdGroup *group;

	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(id != NULL, NULL);

	for (pos = group_list; pos; pos = pos->next) {
		group = (HybirdGroup*)pos->data;

		if (g_strcmp0(group->id, id) == 0 && group->account == account) {
			return group;
		}
	}

	return NULL;
}

HybirdGroup*
hybird_blist_find_group_by_name(HybirdAccount *account, const gchar *name)
{
	GSList *pos;
	HybirdGroup *group;

	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(name != NULL, NULL);

	for (pos = group_list; pos; pos = pos->next) {
		group = (HybirdGroup*)pos->data;

		if (g_strcmp0(group->name, name) == 0 && group->account == account) {
			return group;
		}
	}

	return NULL;
}

HybirdBuddy*
hybird_blist_find_buddy(HybirdAccount *account, const gchar *id)
{
	GSList *pos;
	HybirdBuddy *buddy;

	g_return_val_if_fail(id != NULL, NULL);

	for (pos = buddy_list; pos; pos = pos->next) {
		buddy = (HybirdBuddy*)pos->data;

		if (g_strcmp0(buddy->id, id) == 0 && buddy->account == account) {
			return buddy;
		}
	}

	return NULL;
}

void
hybird_blist_buddy_to_cache(HybirdBuddy *buddy, HybirdBlistCacheType type)
{
	HybirdAccount *ac;
	HybirdConfig *config;
	HybirdBlistCache *cache;
	xmlnode *root;
	xmlnode *node;
	xmlnode *temp;
	gchar *username;
	gchar *protoname;
	gchar *id;
	gchar *name;

	g_return_if_fail(buddy != NULL);

	if (buddy->cache_node) {
		node = buddy->cache_node;
		goto buddy_exist;
	}

	ac = buddy->account;
	config = ac->config;
	cache = config->blist_cache;
	root = cache->root;

	if (!(node = xmlnode_find(root, "accounts"))) {
		hybird_debug_error("blist", "can't find accounts node in blist cache");
		return;
	}

	/* find node named 'account' */
	if ((temp = xmlnode_child(node))) {

		while (temp) {
			if (!xmlnode_has_prop(temp, "username") ||
				!xmlnode_has_prop(temp, "proto")) {
				hybird_debug_error("blist", "invalid blist cache node found");
				temp = xmlnode_next(temp);
				continue;
			}

			username = xmlnode_prop(temp, "username");
			protoname = xmlnode_prop(temp, "proto");

			if (g_strcmp0(ac->username, username) == 0 &&
				g_strcmp0(ac->proto->info->name, protoname) == 0) {
				g_free(username);
				g_free(protoname);
				node = temp;
				goto account_exist;
			}

			temp = xmlnode_next(temp);

			g_free(username);
			g_free(protoname);
		}
	}

	node = xmlnode_new_child(node, "account");
	xmlnode_new_prop(node, "username", ac->username);
	xmlnode_new_prop(node, "proto", ac->proto->info->name);

account_exist:
	if (!(temp = xmlnode_find(node, "buddies"))) {
		node = xmlnode_new_child(node, "buddies");

	} else {
		node = temp;	
	}

	/* Now node point to node named 'buddies'', make it
	 * point to group node.
	 *
	 * check whether there was a group node that we need.
	 */
	if ((temp = xmlnode_child(node))) {

		while (temp) {

			if (!xmlnode_has_prop(temp, "id") ||
				!xmlnode_has_prop(temp, "name")) {
				hybird_debug_error("blist", "invalid blist cache group node found");
				temp = xmlnode_next(temp);
				continue;
			}
			
			id = xmlnode_prop(temp, "id");
			name = xmlnode_prop(temp, "name");

			if (g_strcmp0(id, buddy->parent->id) == 0 &&
				g_strcmp0(name, buddy->parent->name) == 0) {
				g_free(id);
				g_free(name);
				node = temp;
				goto group_exist;
			}

			temp = xmlnode_next(temp);

			g_free(id);
			g_free(name);
		}
	}

	node = xmlnode_new_child(node, "group");
	xmlnode_new_prop(node, "id", buddy->parent->id);
	xmlnode_new_prop(node, "name", buddy->parent->name);

group_exist:
	/* whether there's a buddy node */
	if ((temp = xmlnode_child(node))) {
		
		while (temp) {

			if (!xmlnode_has_prop(temp, "id")) {
				hybird_debug_error("blist", "invalid blist cache buddy node found");
				temp = xmlnode_next(temp);
				continue;
			}
			
			id = xmlnode_prop(temp, "id");

			if (g_strcmp0(id, buddy->id) == 0) {
				g_free(id);
				node = temp;
				goto buddy_exist;
			}

			temp = xmlnode_next(temp);

			g_free(id);
		}
	}

	node = xmlnode_new_child(node, "buddy");

buddy_exist:

	if (type == HYBIRD_BLIST_CACHE_ADD) {

		if (xmlnode_has_prop(node, "id")) {
			xmlnode_set_prop(node, "id", buddy->id);

		} else {
			xmlnode_new_prop(node, "id", buddy->id);
		}
	}

	if (type == HYBIRD_BLIST_CACHE_ADD ||
		type == HYBIRD_BLIST_CACHE_UPDATE_NAME) {

		if (xmlnode_has_prop(node, "name")) {
			xmlnode_set_prop(node, "name", buddy->name);

		} else {
			xmlnode_new_prop(node, "name", buddy->name);
		}
	}

	if (type == HYBIRD_BLIST_CACHE_UPDATE_MOOD) {

		if (xmlnode_has_prop(node, "mood")) {
			xmlnode_set_prop(node, "mood", buddy->mood);

		} else {
			xmlnode_new_prop(node, "mood", buddy->mood);
		}
	}

	if (type == HYBIRD_BLIST_CACHE_UPDATE_ICON) {

		if (xmlnode_has_prop(node, "crc")) {
			xmlnode_set_prop(node, "crc", buddy->icon_crc);

		} else {
			xmlnode_new_prop(node, "crc", buddy->icon_crc);
		}

		if (xmlnode_has_prop(node, "icon")) {
			xmlnode_set_prop(node, "icon", buddy->icon_name);

		} else {
			xmlnode_new_prop(node, "icon", buddy->icon_name);
		}
	}

	/* Set the buddy's xml cache node property. */
	buddy->cache_node = node;

	hybird_blist_cache_flush();
}


/**
 * Save the buddy's icon to the local file.The naming method is:
 * SHA1(buddy_id + '_' + proto_name).type
 */
static void
hybird_blist_buddy_icon_save(HybirdBuddy *buddy)
{
	gchar *name;
	gchar *hashed_name;
	HybirdAccount *account;
	HybirdModule *module;
	HybirdConfig *config;

	g_return_if_fail(buddy != NULL);

	if (buddy->icon_data == NULL || buddy->icon_data_length == 0) {
		return;
	}

	account = buddy->account;
	module = account->proto;
	config = account->config;


	if (!buddy->icon_name) {
		name = g_strdup_printf("%s_%s", module->info->name, buddy->id);
		hashed_name = hybird_sha1(name, strlen(name));
		g_free(name);

		buddy->icon_name = g_strdup_printf("%s.jpg", hashed_name);

		name = g_strdup_printf("%s/%s.jpg", config->icon_path, hashed_name);
		g_free(hashed_name);

	} else {
		name = g_strdup_printf("%s/%s", config->icon_path, buddy->icon_name);
	}

	g_file_set_contents(name, (gchar*)buddy->icon_data,
			buddy->icon_data_length, NULL);

	g_free(name);

}
