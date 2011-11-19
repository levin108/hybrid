/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include "head.h"
#include "gtkutils.h"
#include "tooltip.h"

HybridHead *hybrid_head;

extern HybridTooltip hybrid_tooltip;

/**
 * Callback funtion for initializing the data in the tooltip window.
 */
static gboolean
tooltip_init(HybridTooltipData *tip_data)
{
	HybridAccount		*account;
	HybridModule		*module;
	HybridIMOps			*ops;
	
	if ((account = hybrid_blist_get_current_account())) {

		module = account->proto;
		ops	   = module->info->im_ops;

		if (tip_data->icon) {
			g_object_unref(tip_data->icon);
		}

		tip_data->icon = hybrid_create_round_pixbuf(
							account->icon_data,
							account->icon_data_len, PORTRAIT_WIDTH);

		if (ops->account_tooltip) {
			if (!ops->account_tooltip(account, tip_data)){

				return FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Hide the edit box and show the info panel.
 */
static void
hide_edit_box(void)
{
	gtk_widget_show(hybrid_head->eventbox);
	gtk_widget_hide(hybrid_head->editbox);
}

/**
 * Hide the info panel and show the edit box.
 */
static void
show_edit_box(gint edit_type)
{
	const gchar *markup;
	HybridAccount *account;

	/*
	 * Be sure there's at least one account was enable.
	 */
	if (!(account = hybrid_blist_get_current_account())) {
		return;
	}

	hybrid_head->edit_account = account;
	hybrid_head->edit_state = edit_type;

	switch (edit_type) {
		case HYBRID_HEAD_EDIT_NAME:

			markup = _("<b>Please input the name:</b>");

			if (account->nickname) {
				gtk_entry_set_text(GTK_ENTRY(hybrid_head->edit_entry),
						account->nickname);
			}

			break;
		case HYBRID_HEAD_EDIT_STATUS:

			markup = _("<b>Please input the status:</b>");
			
			if (account->status_text) {
				gtk_entry_set_text(GTK_ENTRY(hybrid_head->edit_entry),
						account->status_text);
			}

			break;
		default:
			return;
	}


	gtk_label_set_markup(GTK_LABEL(hybrid_head->edit_label), markup);

	gtk_widget_hide(hybrid_head->eventbox);
	gtk_widget_show(hybrid_head->editbox);

	gtk_widget_grab_focus(hybrid_head->edit_entry);
}

static void
modify_photo_menu_cb(GtkWidget *widget, gpointer user_data)
{
	GtkWidget			*filechooser;
	gint				 response;
	const gchar			*filename;
	HybridAccount		*account;
	HybridModule		*module;
	HybridIMOps			*ops;

	hybrid_head->edit_account = hybrid_blist_get_current_account();

	if (!hybrid_head->edit_account) {
		return;
	}

	filechooser = gtk_file_chooser_dialog_new(
						_("Choose the avatar file to upload"),
						NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
						_("Cancel"), 2, _("OK"), 1, NULL);
	response	= gtk_dialog_run(GTK_DIALOG(filechooser));

	if (!(account = hybrid_head->edit_account)) {
		return;
	}

	if (response == 1) {

		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));

		if (filename) {
			module = account->proto;
			ops	   = module->info->im_ops;
			
			if (ops->modify_photo) {
				ops->modify_photo(account, filename);
			}
		}
	}
	
	gtk_widget_destroy(filechooser);
}

static void
modify_name_menu_cb(GtkWidget *widget, gpointer user_data)
{
	show_edit_box(HYBRID_HEAD_EDIT_NAME);
}

static void
modify_status_menu_cb(GtkWidget *widget, gpointer user_data)
{
	show_edit_box(HYBRID_HEAD_EDIT_STATUS);
}

static gboolean
button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
#if 0
	/* Show the modify-name panel for left click. */
	if (event->button == 1) {

		show_edit_box(HYBRID_HEAD_EDIT_STATUS);

		return TRUE;
	}
#endif

	/* Popup menus for right click. */
	if (event->button == 3) {

		GtkWidget *menu;

		menu = gtk_menu_new();

		hybrid_create_menu(menu, _("Modify Photo"), "rename", TRUE,
						G_CALLBACK(modify_photo_menu_cb), NULL);

		hybrid_create_menu(menu, _("Modify Name"), "rename", TRUE,
						G_CALLBACK(modify_name_menu_cb), NULL);

		hybrid_create_menu(menu, _("Modify Status"), "rename", TRUE,
						G_CALLBACK(modify_status_menu_cb), NULL);

		gtk_widget_show_all(menu);
		
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				(event != NULL) ? event->button : 0,
				gdk_event_get_time((GdkEvent*)event));

		return TRUE;
	}

	return FALSE;
}

static void
entry_activate_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccount		*account;
	HybridModule		*module;
	HybridIMOps			*ops;
	const gchar			*text;

	hide_edit_box();

	account = hybrid_head->edit_account;
	module  = account->proto;
	ops		= module->info->im_ops;
	text    = gtk_entry_get_text(GTK_ENTRY(hybrid_head->edit_entry));

	if (!account) {
		return;
	}

	if (hybrid_head->edit_state == HYBRID_HEAD_EDIT_NAME) {
		/* No change to the nickname. */
		if (g_strcmp0(text, account->nickname) == 0) {
			return;
		}

		if (ops->modify_name) {
			if (!ops->modify_name(account, text)) {
				return;
			}

			account = hybrid_blist_get_current_account();
			hybrid_account_set_nickname(account, text);
			hybrid_head_bind_to_account(account);
		}
	}

	if (hybrid_head->edit_state == HYBRID_HEAD_EDIT_STATUS) {
		/* No change to the status text. */
		if (g_strcmp0(text, account->status_text) == 0) {
			return;
		}

		if (ops->modify_status) {
			if (!ops->modify_status(account, text)) {
				return;
			}

			account = hybrid_blist_get_current_account();
			hybrid_account_set_status_text(account, text);
			hybrid_head_bind_to_account(account);
		}
	}
}

static gboolean
focus_out_cb(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	hide_edit_box();

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
		text = g_strdup_printf(_("<b>%s</b> [%s]\n<small>%s</small>"), 
							account->nickname,
							hybrid_get_presence_name(account->state),
							account->status_text ? account->status_text : "");
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
	GtkWidget *align;

	hybrid_head = g_new0(HybridHead, 1);
	hybrid_head->vbox = gtk_vbox_new(FALSE, 0);
	hybrid_head->eventbox = gtk_event_box_new();
	hybrid_head->editbox = gtk_vbox_new(TRUE, 4);

	hybrid_head->cellview = gtk_cell_view_new();
	gtk_container_add(GTK_CONTAINER(hybrid_head->eventbox), 
	                  hybrid_head->cellview);

	hybrid_head->edit_label = gtk_label_new(NULL);
	hybrid_head->edit_entry = gtk_entry_new();

	align = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(align), hybrid_head->edit_label);
	gtk_box_pack_start(GTK_BOX(hybrid_head->editbox),
						align, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hybrid_head->editbox),
						hybrid_head->edit_entry, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hybrid_head->vbox), hybrid_head->eventbox,
						FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hybrid_head->vbox), hybrid_head->editbox,
						FALSE, FALSE, 0);

	gtk_container_set_border_width(GTK_CONTAINER(hybrid_head->editbox), 3);

	g_signal_connect(G_OBJECT(hybrid_head->eventbox),
				     "button_press_event",
					 GTK_SIGNAL_FUNC(button_press_cb), NULL);

	g_signal_connect(G_OBJECT(hybrid_head->edit_entry),
				     "focus-out-event",
					 GTK_SIGNAL_FUNC(focus_out_cb), NULL);

	g_signal_connect(G_OBJECT(hybrid_head->edit_entry),
				     "activate",
					 GTK_SIGNAL_FUNC(entry_activate_cb), NULL);

	hybrid_tooltip_setup(hybrid_head->eventbox, NULL, NULL, tooltip_init, NULL);

	cell_view_init(hybrid_head);

	hybrid_head_bind_to_account(NULL);
}
