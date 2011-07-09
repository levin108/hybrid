#include "head.h"
#include "gtkutils.h"
#include "tooltip.h"

HybridHead *hybrid_head;

extern HybridTooltip hybrid_tooltip;

static void
editing_started_cb(GtkCellRenderer *renderer, GtkCellEditable *editable,
		gchar *path_str, gpointer user_data)
{
	GtkEntry *entry;
	const gchar *text;

	text = "test";

	if (GTK_IS_ENTRY(editable)) {
		entry = GTK_ENTRY(editable);

		if (!text) {
			gtk_entry_set_text(entry, "");

		} else {
			gtk_entry_set_text(entry, text);
		}
	}
}

/**
 * Callback funtion for initializing the data in the tooltip window.
 */
static gboolean
tooltip_init(HybridTooltipData *tip_data)
{
	HybridAccount *account;
	HybridModule *module;

	
	if ((account = hybrid_blist_get_current_account())) {

		module = account->proto;

		if (tip_data->icon) {
			g_object_unref(tip_data->icon);
		}

		tip_data->icon = hybrid_create_round_pixbuf(
							account->icon_data,
							account->icon_data_len, PORTRAIT_WIDTH);

		if (module->info->account_tooltip) {
			if (!module->info->account_tooltip(account, tip_data)){

				return FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

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
								G_TYPE_STRING,
								GDK_TYPE_PIXBUF);

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

	g_object_set(renderer, "editable", TRUE, NULL);

	g_signal_connect(G_OBJECT(renderer), "editing-started",
						G_CALLBACK(editing_started_cb), NULL);
	//g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(edited_cb), NULL);

	g_object_set(renderer, "xalign", 0.0, "yalign", 0.0, "xpad", 6, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	/* status icon renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(head->cellview), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(head->cellview), renderer,
			"pixbuf", HYBRID_HEAD_STATUS_ICON_COLUMN, NULL);
	g_object_set(renderer, "yalign", 0.5, "xpad", 10, "ypad", 0, NULL);

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
	GdkPixbuf *status_icon;
	gchar *text;

	model = gtk_cell_view_get_model(GTK_CELL_VIEW(hybrid_head->cellview));

	if (!account) {
		pixbuf = hybrid_create_default_icon(32);
		text = g_strdup(_("No account was enabled."));
		status_icon = NULL;

	} else {
		pixbuf = hybrid_create_round_pixbuf(account->icon_data,
							account->icon_data_len, 32);
		text = g_strdup_printf(_("<b>%s</b> [%s]\n%s"), 
							account->nickname,
							hybrid_get_presence_name(account->state),
							account->status_text);
		status_icon = hybrid_create_presence_pixbuf(account->state, 16);
	}


	gtk_list_store_set(GTK_LIST_STORE(model), &hybrid_head->iter,
					HYBRID_HEAD_PIXBUF_COLUMN, pixbuf,
					HYBRID_HEAD_NAME_COLUMN, text,
					HYBRID_HEAD_STATUS_ICON_COLUMN, status_icon,
					-1);

	path = gtk_tree_path_new_from_string("0");
	gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(hybrid_head->cellview), path);
	gtk_tree_path_free(path);
	
	g_object_unref(pixbuf);
	if (status_icon) {
		g_object_unref(status_icon);
	}
	g_free(text);
}

void hybrid_head_init()
{
	hybrid_head = g_new0(HybridHead, 1);
	hybrid_head->cellview = gtk_cell_view_new();
	hybrid_head->eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(hybrid_head->eventbox), 
	                  hybrid_head->cellview);

	hybrid_tooltip_setup(hybrid_head->eventbox, NULL, NULL, tooltip_init, NULL);

	cell_view_init(hybrid_head);

	hybrid_head_bind_to_account(NULL);
}
