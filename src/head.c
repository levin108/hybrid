#include "head.h"
#include "gtkutils.h"

HybridHead *hybrid_head;

/**
 * Initialize the head cellview, create list store and set the cell renderers.
 */
static void
cell_view_init(HybridHead *head)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreePath *path;

	g_return_if_fail(head != NULL);

	store = gtk_list_store_new(HYBRID_HEAD_COLUMNS,
								GDK_TYPE_PIXBUF,
								G_TYPE_STRING);

	gtk_cell_view_set_model(GTK_CELL_VIEW(head->cellview), GTK_TREE_MODEL(store));

	g_object_unref(store);

	/* buddy icon renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(head->cellview), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(head->cellview), renderer,
			"pixbuf", HYBRID_HEAD_PIXBUF_COLUMN, NULL);
	g_object_set(renderer, "yalign", 0.5, "xpad", 3, "ypad", 0, NULL);

	/* buddy name renderer */
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(head->cellview), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(head->cellview), renderer,
			"markup", HYBRID_HEAD_NAME_COLUMN, NULL);

	g_object_set(renderer, "xalign", 0.0, "yalign", 0.0, "xpad", 6, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_list_store_append(store, &head->iter);
	path = gtk_tree_path_new_from_string("0");
	gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(head->cellview), path);
	gtk_tree_path_free(path);
}

void
hybrid_head_bind_to_account(HybridAccount *account)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GdkPixbuf *pixbuf;
	gchar *text;

	model = gtk_cell_view_get_model(GTK_CELL_VIEW(hybrid_head->cellview));

	if (!account) {
		pixbuf = hybrid_create_default_icon(32);
		text = g_strdup(_("<b>Idle</b>\nNo account was enabled."));

	} else {
		pixbuf = hybrid_create_round_pixbuf(account->icon_data,
							account->icon_data_len, 32);
		text = g_strdup_printf(_("<b>%s</b>\n%s"), 
							account->nickname, account->status_text);
	}

	gtk_list_store_set(GTK_LIST_STORE(model), &hybrid_head->iter,
					HYBRID_HEAD_PIXBUF_COLUMN, pixbuf,
					HYBRID_HEAD_NAME_COLUMN, text,
					-1);

	path = gtk_tree_path_new_from_string("0");
	gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(hybrid_head->cellview), path);
	gtk_tree_path_free(path);
	
	g_object_unref(pixbuf);
	g_free(text);
}

void hybrid_head_init()
{
	hybrid_head = g_new0(HybridHead, 1);
	hybrid_head->cellview = gtk_cell_view_new();

	cell_view_init(hybrid_head);

	hybrid_head_bind_to_account(NULL);
}
